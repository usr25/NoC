#include <cstdio>
#include <iostream>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <sstream>
#include <vector>
#include <random>
#include <math.h>
#include "unistd.h"

/*
Use:
This is a highly specialized tool, I doubt it'll be of much help to anyone but me
-It reads the file last.txt
-Generates 2 new files A.txt, B.txt
-Tells the engine which version to use
-The winner is selected to become the next last.txt
-Depending on how wide the margin is, a ponderate mean will be calculated between the values

WARNING: All paths have to end with '/', I will change this later, if need be
*/
bool ONE = false;
int ITER = 4;

int iterations = 0;
int var_count = 6;
int curr_var = 0;

std::string rounds("1");

std::string tablebases("-/");
std::string dir("-/");


const std::string concurrency("1");
const std::string cutechess("cutechess-cli");
const std::string tc(".1+.01");
const std::string resign("-resign movecount=10 score=700");
const std::string sprt("-sprt elo0=5 elo1=5 alpha=0.05 beta=0.05");
const std::string extra_flags("-repeat -games 2 -recover -tbpieces 5");

std::default_random_engine generator;

const std::string help_msg("trainer [OPTIONS]\n\n  -o: Tune the variables ONE by ONE, do this with new engines to find a better starting point\n  -c: Number of variables to tune with -o option\n  -i: Total number of iterations\n  -r: Rounds per iteration, the number of games is double the rounds\n  -d: Dir where the engines and value files are placed\n  -t: Dir where the tb are placed\nExample: $trainer -o -c 20 -r 700\n\nWARNING: Ensure the dir names end with \'/\'. Call the engine \'train\'");

class Game
{
    std::string cmd;
    std::string a, b;

public:
    Game(std::string, std::string);
    void play();
};

/* This is pretty basic but functional enough, it simulates annealing generating big values at the beggining
 * and smaller ones towards the latter iterations
 */
int new_value(int v){
    float var = std::sqrt(std::abs((float)v)) / (1.5 + std::sqrt(iterations / 3));
    std::normal_distribution<float> dist((float)v, std::max(var, 0.55f));
    return std::round(dist(generator));
}

/* Call to cutechess
 */
std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe)
        throw std::runtime_error("popen() failed!");

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();

    return result;
}

/* Runs the command and parses the output to get the line which mentions the elo difference
 */
std::string run(std::string cmd){
    std::string out = exec(cmd.c_str());
    std::stringstream stream(out);
    std::string to;

    if (!out.empty()){
        while(std::getline(stream, to)){
            if (to.compare(0, 3, "Elo") == 0)
                return to;
        }
    }
}

/* Generates the new value files to evaluate (A, B)
 */
void gen_new_files(){
    std::string line;
    std::ifstream last(dir + "last.txt");
    std::vector<int> vals;
    vals.clear();
    if (last.is_open()){
        while(getline(last, line)){
            std::string::size_type pos = line.find('#');
            std::string ns;
            if (pos != std::string::npos)
                ns = line.substr(0, pos);
            else
                ns = line;
            if (ns.length() > 0){
                vals.push_back(std::stoi(ns));
            }
        }
        last.close();
    }

    std::ofstream a(dir + "A.txt");
    std::ofstream b(dir + "B.txt");
    if (!a.is_open() || !b.is_open())
        std::runtime_error("Couldn't open A or B");

    for (std::vector<int>::iterator i = vals.begin(); i != vals.end(); ++i){
        int v = *i;
        a << new_value(v) << std::endl;
        b << new_value(v) << std::endl;
    }
    a.close();
    b.close();
}

/* Generates the new files but on ONE variable mode
 */
void gen_new_files_one(){
    std::string line;
    std::ifstream last(dir + "last.txt");
    std::vector<int> vals;
    vals.clear();
    if (last.is_open()){
        while(getline(last, line)){
            std::string::size_type pos = line.find('#');
            std::string ns;
            if (pos != std::string::npos)
                ns = line.substr(0, pos);
            else
                ns = line;
            if (ns.length() > 0)
                vals.push_back(std::stoi(ns));
        }
        last.close();
    }

    std::ofstream a(dir + "A.txt");
    std::ofstream b(dir + "B.txt");
    if (!a.is_open() || !b.is_open())
        std::runtime_error("Couldn't open A or B");

    for (int i = 0; i < vals.size(); ++i){
        if (i == curr_var){
            int testVal = std::abs(new_value(vals[i]) - vals[i]);
            testVal = std::max(1, testVal);
            a << vals[i] + testVal << std::endl;
            b << vals[i] - testVal << std::endl;
        }else{
            a << vals[i] << std::endl;
            b << vals[i] << std::endl;
        }
    }
    a.close();
    b.close();
}

/* Merges A.txt and B.txt into last.txt based on the result
 */
void gen_new_last(float interp){

    //1 means A played better, 0 means B, 0.5 is a draw
    std::vector<int> v;
    std::ofstream last(dir + "last.txt");
    std::ifstream a(dir + "A.txt");
    std::ifstream b(dir + "B.txt");

    if (!last.is_open() || ! a.is_open() || !b.is_open())
        std::runtime_error("Couldn't open a file");
    std::string al;
    std::string bl;
    while(getline(a, al) && getline(b, bl)){
        int a_val = std::stoi(al);
        int b_val = std::stoi(bl);
        int weighted = std::round(a_val * interp + b_val * (1 - interp));
        last << weighted << std::endl;
    }
    last.close();
    a.close();
    b.close();
}

/* Parses the string with the information on the elo diff and returns the difference
 */
float analyze_results(std::string s){

    std::cout << s << std::endl;
    std::string ns = s.erase(0, 16);
    std::string::size_type pos = ns.find('+');
    std::string float_str;

    float num;
    if (ns.compare(0, 3, "inf") == 0)
        num = 1000;
    else if (ns.compare(0, 4, "-inf") == 0)
        num = -1000;
    else if (pos != std::string::npos){
        float_str = ns.substr(0, pos);
        num = std::stof(float_str);
    }
    else
        std::runtime_error("ERROR parsing str: " + s);

    return num;
}

/* Make a new Game object
 */
Game::Game (const std::string _a, const std::string _b){
    a = _a;
    b = _b;

    std::stringstream command_stream;
    command_stream  << cutechess
                    << " -engine cmd=\'" << dir << a << " " << dir << "A.txt\'"
                    << " -engine cmd=\'" << dir << b << " " << dir << "B.txt\'"
                    << " -each proto=uci tc=" << tc
                    << " -rounds " << rounds
                    << " -tb " << tablebases
                    << " -concurrency " << concurrency
                    << " " << resign
                    //<< " " << sprt
                    << " " << extra_flags
                    << " | tail -20";
    cmd = command_stream.str();

    std::cout << cmd << std::endl;
}


void Game::play(){

    std::vector<std::string> v;
    std::string result = run(cmd);;
    float res = analyze_results(result);

    //If the elo diff is more than 30 the engine is considered superior and all values are unchanged
    if (res >= 25)
        gen_new_last(1);
    else if (res <= -25)
        gen_new_last(0);
    else
        gen_new_last((res / 50) + .5f);
    //This interpolation method may cause problems with small numbers due to the rounding (the value won't be changed unless the diff is too big)
    //Another possible interpolation is sqrt(abs(x)), since it pushes values away from 0
    //The cubic root also seems like a good option, but it grows too fast near 0
}

/* Main loop
 */
void loop(){
    Game g("train", "train");
    for (iterations = 0; iterations < ITER; ++iterations)
    {
        std::cout << "Iter: " << iterations << "/" << ITER << std::endl;
        curr_var = 0;
        if (!ONE)
        {
            gen_new_files();
            g.play();
        }
        else
        {
            for (curr_var = 0; curr_var < var_count; ++curr_var)
            {
                gen_new_files_one();
                g.play();
                std::cout << "  Var: " << curr_var << "/" << var_count << std::endl;
            }
        }
    }
}

int main(const int argc, char** argv){

    int c;
    while((c = getopt(argc, argv, "ohi:r:d:t:c:")) != -1)
    {
        switch(c)
        {
            case 'h':
            std::cout << help_msg << std::endl;
            return 0;

            case 'o':
            ONE = true;
            break;

            case 'i':
            ITER = std::atoi(optarg);
            break;

            case 'r':
            rounds = optarg;
            break;

            case 'd':
            dir = optarg;
            break;

            case 'c':
            var_count = std::atoi(optarg);
            break;

            case 't':
            tablebases = optarg;
            break;
        }
    }

    loop();

    return 0;
}
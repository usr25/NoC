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

int ITER = 5;
std::string rounds("1");

std::string tablebases("-/");
std::string values_dir("-/");
std::string engines_dir("-/");

const std::string concurrency("1");
const std::string cutechess("cutechess-cli");
const std::string tc(".1+.01");
const std::string extra_flags("-repeat -games 2 -recover -tbpieces 5");

std::default_random_engine generator;
int iterations = 0;

const std::string help_msg("trainer [OPTIONS]\n  -i: Total number of iterations\n  -r: Rounds per iterations, the games are double the rounds\n  -e: Dir where the engines are placed\n  -v: Dir where the values are placed\n  -t: Dir where the tb are placed\nExample: $trainer -t 20 -r 600\n\nWARNING: Ensure the dir names end with \'/\'. Call the engine \'Train\'");

class Game
{
    std::string cmd;
    std::string a, b;

public:
    Game(std::string, std::string);
    void play();
};

//This is pretty basic, try somethin better like big values at the beggining
int new_value(int v){
    std::normal_distribution<float> dist((float)v, 
        std::sqrt(std::abs((float)v)) / (2 + std::sqrt(iterations / 3)));
    return std::round(dist(generator));
}

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
}

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

void gen_new_files(){
    std::string line;
    std::ifstream last(values_dir + "last.txt");
    std::vector<int> vals;
    vals.clear();
    if (last.is_open())
    {
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

    std::ofstream a(values_dir + "A.txt");
    std::ofstream b(values_dir + "B.txt");
    if (!a.is_open() || !b.is_open())
        std::runtime_error("Couldn't open A or B");

    for (std::vector<int>::iterator i = vals.begin(); i != vals.end(); ++i)
    {
        int v = *i;
        a << new_value(v) << std::endl;
        b << new_value(v) << std::endl;
    }
    a.close();
    b.close();
}

void gen_new_last(float interp){

    //1 means A played better, 0 means B, 0.5 is a draw
    iterations++;

    std::vector<int> v;
    std::ofstream last(values_dir + "last.txt");
    std::ifstream a(values_dir + "A.txt");
    std::ifstream b(values_dir + "B.txt");

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

Game::Game (const std::string _a, const std::string _b){
    a = _a;
    b = _b;

    std::stringstream command_stream;
    command_stream  << cutechess
                    << " -engine cmd=\'" << engines_dir << a << " " << values_dir << "A.txt\'"
                    << " -engine cmd=\'" << engines_dir << b << " " << values_dir << "B.txt\'"
                    << " -each proto=uci tc=" << tc
                    << " -rounds " << rounds
                    << " -tb " << tablebases
                    << " -concurrency " << concurrency
                    << " " << extra_flags
                    << " | tail -20";
    cmd = command_stream.str();

    std::cout << cmd << std::endl;
}


void Game::play(){

    std::vector<std::string> v;
    std::string result = run(cmd);;
    float res = analyze_results(result);

    std::cout << res << std::endl;

    //If the elo diff is more than 50 the engine is considered superior and all values are unchanged
    if (res > 50)
        gen_new_last(1);
    else if (res < -50)
        gen_new_last(0);
    else
        gen_new_last((res / 100) + .5f);
}

int main(const int argc, char** argv){

    int c;
    while((c = getopt(argc, argv, "hi:r:v:e:t:")) != -1)
    {
        switch(c)
        {
            case 'h':
            std::cout << help_msg << std::endl;
            return 0;

            case 'i':
            ITER = std::atoi(optarg);
            break;

            case 'r':
            rounds = optarg;
            break;

            case 'v':
            values_dir = optarg;
            break;

            case 'e':
            engines_dir = optarg;
            break;

            case 't':
            tablebases = optarg;
            break;
        }
    }


    Game g("Train", "Train");
    for (int i = 0; i < ITER; ++i)
    {
        std::cout << "Iter: " << i << "/" << ITER << std::endl;
        gen_new_files();
        g.play();
    }

    return 0;
}
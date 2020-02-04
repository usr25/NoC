/* train.c
 * Automatically tweaks the evaluation parametres to better fit a set of
 * labled positions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>

#include "../include/global.h"
#include "../include/train.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/hash.h"
#include "../include/search.h"
#include "../include/evaluation.h"

#define NUM_VARS 14


typedef struct
{
    int successful;
    int valueMG;
    int valueEG;
} ReadInt;

typedef struct
{
    char fen[150];
    double result;
} FenResult;


static ReadInt getNext(FILE* fp);
static void setArray(const int* arr);
static void saveArray(const int* arr);
static void optimize(void);
static void loadFensIntoMem(void);
static double error(void);

/* static vars */
static int num_thr = 2; //Number of threads
static int num_pos = 100; //To count the number of positions run $ wc -l /../fen.csv
static int val_lim = 1400; //To ensure that no value gets too high

static char fenFile[200];  //File with the fens and results
static char saveFile[200]; //File where the optimum values will be placed
static char valFile[200];  //File that contains the initial values

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static int vals[2*NUM_VARS];     //The values of the variables
static FenResult* positions;   //Array that holds the fens+results in memory, this is much faster than reading from the hard drive
static double* acc;            //Accumulators for the different threads

static char* help_msg = 
"Module to tune engine evaluation parametres\n"
"Place Values.txt and fen.csv in the same dir, pass the path as the fst argument\n"
"The params are:\n"
" -h: display help msg\n"
" -l: variable limit\n"
" -c: number of threads\n"
" -p: number of positions\n"
"Eg.: ./t /home/usr/chess -c 3 -p 400000";


static const int isNumeric(char c)
{
    return c == '-' || (c >= '0' && c <= '9');
}

static const double sigmoid(const double v)
{
    return 1.0 / (1.0 + exp(-v * 0.00518078)); //This factor is ln(10) * .85 / 400
}

void setVariables(const int argc, char** const argv)
{
    for (int i = 0; i < argc; ++i)
    {
        if (strncmp(argv[i], "-c", 2) == 0) { //Number of concurrent threads
            num_thr = atoi(argv[i+1]);
            if (num_thr <= 0){
                printf("[-] Invalid arg %s\n", argv[i+1]);
                exit(EXIT_FAILURE);
            }
        } else if (strncmp(argv[i], "-p", 2) == 0) { //Number of positions
            num_pos = atoi(argv[i+1]);
            if (num_pos <= 0){
                printf("[-] Invalid arg %s\n", argv[i+1]);
                exit(EXIT_FAILURE);
            }
        } else if (strncmp(argv[i], "-l", 2) == 0) { //Limit of the var
            val_lim = atoi(argv[i+1]);
            if (val_lim <= 0){
                printf("[-] Invalid arg %s\n", argv[i+1]);
                exit(EXIT_FAILURE);
            }
        } else if (strncmp(argv[i], "-h", 2) == 0) { //Show help message
            printf("%s\n", help_msg);
            exit(EXIT_SUCCESS);
        }
    }

    positions = malloc(num_pos * sizeof(FenResult));
    acc = malloc(num_thr * sizeof(double));
}

/* Reads all the info from path and parses the values that will be used
 */
void readValues(const char* path)
{
    sprintf(fenFile,  "%s/fen.csv",     path);
    sprintf(valFile,  "%s/Values.txt",  path);
    sprintf(saveFile, "%s/opt.txt",     path);

    FILE* fp;

    fp = fopen(valFile, "r");
    if (fp == NULL)
    {
        printf("[-] Error opening \'%s\'\n", valFile);
        exit(EXIT_FAILURE);
    }

    ReadInt r;
    int i = 0;
    while((r = getNext(fp)).successful)
    {
        vals[i++] = r.valueMG;
        vals[i++] = r.valueEG;
    }

    setArray(vals);

    fclose(fp);
}

void txlTrain(void)
{
    loadFensIntoMem();
    optimize();

    free(positions);
    free(acc);
}
/* Parses all the lines in the file to easily assign the values to the variables
 */
static ReadInt getNext(FILE* fp)
{
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    ReadInt r = (ReadInt){.successful = 0};

    if ((read = getline(&line, &len, fp)) != -1)
    {
        if (read > 1 && isNumeric(line[0]))
        {
            r.successful = 1;
            r.valueMG = atoi(line);
            r.valueEG = atoi(strchr(line, ',')+1);
        }
    }

    if (line) free(line);

    return r;
}

/* Sets the relevant variables to their respective values
 */
static void setArray(const int* arr)
{
    V_QUEEN[0]  = arr[0];
    V_QUEEN[1]  = arr[1];
    V_ROOK[0] = arr[2];
    V_ROOK[1] = arr[3];
    V_BISH[0] = arr[4];
    V_BISH[1] = arr[5];
    V_KNIGHT[0] = arr[6];
    V_KNIGHT[1] = arr[7];

    CONNECTED_ROOKS[0] = arr[8];
    CONNECTED_ROOKS[1] = arr[9];
    ROOK_OPEN_FILE [0] = arr[10];
    ROOK_OPEN_FILE [1] = arr[11];
    SAFE_KING      [0] = arr[12];
    SAFE_KING      [1] = arr[13];

    BISH_PAIR      [0] = arr[14];
    BISH_PAIR      [1] = arr[15];
    KNIGHT_PAWNS   [0] = arr[16];
    KNIGHT_PAWNS   [1] = arr[17];
    N_KING_OPEN_FILE[0] = arr[18];
    N_KING_OPEN_FILE[1] = arr[19];

    PAWN_CHAIN     [0] = arr[20];
    PAWN_CHAIN     [1] = arr[21];
    PAWN_PROTECTION[0] = arr[22];
    PAWN_PROTECTION[1] = arr[23];
    ATTACKED_BY_PAWN[0] = arr[24];
    ATTACKED_BY_PAWN[1] = arr[25];
    N_DOUBLED_PAWNS[0] = arr[26];
    N_DOUBLED_PAWNS[1] = arr[27];
}

/* Saves the array into the harddrive, so in case it crashes no data is lost
 */
static void saveArray(const int* arr)
{
    FILE* fp;
    fp = fopen(saveFile, "w");
    if (fp == NULL)
    {
        printf("[-] Error opening \'%s\' in write mode\n", saveFile);
        printf("[-] Printing the current values...\n");

        for (int i = 0; i < NUM_VARS; ++i)
            printf("%d, %d\n", arr[2*i],arr[2*i+1]);

        exit(EXIT_FAILURE);
    }
    char numAsStr[20];
    for (int i = 0; i < NUM_VARS; ++i)
    {
        sprintf(numAsStr, "%d, %d\n", arr[2*i],arr[2*i+1]);
        fputs(numAsStr, fp);
    }
    fclose(fp);
}

/* Load all the fens and results into memory
 */
static void loadFensIntoMem(void)
{
    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(fenFile, "r");
    if (fp == NULL)
    {
        printf("[-] Error opening \'%s\'\n", fenFile);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < num_pos; ++i)
    {
        if ((read = getline(&line, &len, fp)) != -1)
        {
            positions[i] = (FenResult) {};
            if (read > 1)
            {
                for (int j = 0; j < 200; ++j)
                {
                    if (line[j] == '\n')
                    {
                        printf("[-] Shouldn't be here!\n");
                        positions[i].fen[0] = '\0';
                        break;
                    }
                    if (line[j] == ',')
                    {
                        switch(line[j+1])
                        {
                            case 'w':
                                positions[i].result = 1.0;
                                break;
                            case 'b':
                                positions[i].result = 0.0;
                                break;
                            case 'd':
                                positions[i].result = 0.5;
                                break;
                            default:
                                printf("[-] UNSUPPORTED char %c\n", line[j+1]);
                        }
                        positions[i].fen[j] = '\0';
                        break;
                    }

                    positions[i].fen[j] = line[j];
                }
            }
        }
    }

    fclose(fp);
    if (line) free(line);
}

static int assign;

/* Multithreaded function to calculate the error, called from each thread
 */
static void* mthError(void* var)
{
    const int limit = num_pos / num_thr;
    pthread_mutex_lock(&mutex);
    const int threadOffset = assign;
    assign++;
    pthread_mutex_unlock(&mutex);

    int _ignore;
    int qv, adjustedQV;
    double error;

    double localAcc = 0;
    Board b;
    for (int i = 0; i < limit; ++i)
    {
        b = genFromFen(positions[num_thr*i+threadOffset].fen, &_ignore);
        qv = qsearch(b, MINS_INF, PLUS_INF, 7);
        adjustedQV = b.stm? qv : -qv;
        error = positions[num_thr*i+threadOffset].result - sigmoid(adjustedQV);
        localAcc += error * error;
    }

    acc[threadOffset] = localAcc;

    return NULL;
}

static double error(void)
{
    //Set up the multithread vars
    assign = 0;

    setArray(vals);
    pthread_t thread_id[num_thr];

    //Launch the threads
    for (int i = 0; i < num_thr; ++i)
        pthread_create(&thread_id[i], NULL, mthError, NULL);
    //Wait for them to end
    for (int i = 0; i < num_thr; ++i)
        pthread_join(thread_id[i], NULL);

    //Collect the results
    double res = 0;
    for (int i = 0; i < num_thr; ++i)
        res += acc[i];

    return res / num_pos;
}

/* This finds a local minimum
 * It is guaranteed to terminate since the search space is finite, and simple enough
 * so that there are no bugs
 */
static void optimize(void)
{
    double bestVal = error();
    int improved = 1, iter = 0;

    printf("Start training with %d threads, %d variables, and %d positions\n", num_thr, NUM_VARS, num_pos);
    printf("Init E: %.12f\n", bestVal);

    while(improved)
    {
        iter++;
        improved = 0;
        for (int var = 0; var < 2*NUM_VARS; ++var)
        {
            if (vals[var] >= val_lim || vals[var] <= -val_lim) continue;

            int best = vals[var];
            vals[var]++;
            initEval(); //To ensure that the change have been applied for some variables (such as passed pawns)
            double newVal = error();

            while (newVal < bestVal)
            {
                improved = 1;
                best = vals[var];
                bestVal = newVal;
                vals[var]++;
                initEval();
                newVal = error();
            }
            if(!improved)
            {
                vals[var] -= 2;
                initEval();
                newVal = error();
                while(newVal < bestVal)
                {
                    improved = 1;
                    bestVal = newVal;
                    best = vals[var];
                    vals[var]--;
                    initEval();
                    newVal = error();
                }
            }

            vals[var] = best;
        }

        //Output the error and save the vals
        printf("E: %.12f\n", bestVal);
        saveArray(vals);
    }

    saveArray(vals);
    printf("Optimum E: %.12f\n", bestVal);
}

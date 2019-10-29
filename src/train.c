#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "../include/global.h"
#include "../include/train.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/hash.h"
#include "../include/search.h"
#include "../include/evaluation.h"

#define NUM_VARS 12
#define NUM_POS 1000 //To count the number of positions run $ wc -l /../fen.csv


typedef struct
{
    int successful;
    int value;
} ReadInt;

typedef struct
{
    char fen[150];
    double result;
} FenResult;

/* Place the relevant files (valFile, fenFile) in the same dir, saveFile will be placed here
 */
int vals[NUM_VARS];
char fenFile[200];  //File with the fens and results
char saveFile[200]; //File where the optimum values will be placed
char valFile[200];  //File that contains the initial values

FenResult positions[NUM_POS]; //Array that holds the fens+results in memory, this is much faster than reading from the hard drive

static ReadInt getNext(FILE* fp);
static void setArray(const int* arr);
static void saveArray(const int* arr);
static void optimize(void);
static void loadFensIntoMem(void);
static double error(const int* arr);

static const int isNumeric(char c)
{
    return c == '-' || (c >= '0' && c <= '9');
}

static const double sigmoid(const double v)
{
    return 1.0 / (1.0 + exp(-v * 0.0069077)); //This factor is ln(10) * 1.2 / 400
}

/* Reads all the info from path and parses the values that will be used
 */
void readValues(const char* path)
{
    sprintf(fenFile, "%s/fen.csv", path);
    sprintf(valFile, "%s/Values.txt", path);
    sprintf(saveFile, "%s/opt.txt", path);

    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

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
        vals[i] = r.value;
        ++i;
    }

    setArray(vals);

    fclose(fp);
    if (line)
        free(line);
}

void txlTrain(void)
{
    loadFensIntoMem();
    optimize();
}
/* Parses all the lines in the file to easily assign the values to the variables
 */
static ReadInt getNext(FILE* fp)
{
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    ReadInt r = (ReadInt){.successful = 0, .value = 0};

    if ((read = getline(&line, &len, fp)) != -1)
    {
        if (read > 1 && isNumeric(line[0]))
        {
            r.successful = 1;
            r.value = atoi(line);
        }
    }

    return r;
}

/* Sets the relevant variables to their respective values
 */
static void setArray(const int* arr)
{
    V_QUEEN     = arr[0];
    V_ROOK      = arr[1];
    V_BISH      = arr[2];
    V_KNIGHT    = arr[3];
    V_PASSEDP   = arr[4];

    CONNECTED_ROOKS = arr[5];
    ROOK_OPEN_FILE  = arr[6];
    SAFE_KING       = arr[7];

    TWO_BISH        = arr[8];
    KNIGHT_PAWNS    = arr[9];
    N_KING_OPEN_FILE= arr[10];
    N_CLOSE_TO_KING = arr[11];
}

/* Saves the array into memory, so in case it crashes no data is lost
 */
static void saveArray(const int* arr)
{
    FILE* fp;
    fp = fopen(saveFile, "w");
    if (fp == NULL)
    {
        printf("[-] Error opening \'%s\' as w\n", saveFile);
        exit(EXIT_FAILURE);
    }
    char numAsStr[6];
    for (int i = 0; i < NUM_VARS; ++i)
    {
        sprintf(numAsStr, "%d", arr[i]);
        fputs(numAsStr, fp);
        fputc('\n', fp);
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

    for (int i = 0; i < NUM_POS; ++i)
    {
        if ((read = getline(&line, &len, fp)) != -1)
        {
            positions[i] = (FenResult) {};
            if (read > 1)
            {
                for (int j = 0; j < 150; ++j)
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
}

static double error(const int* arr)
{
    setArray(arr);
    int _a;
    int qv, adjustedQV, index;
    double error;
    double acc[3] = {}; //Instead of having one acc we have 3 to avoid possible precision errors
    Board b;

    for (int i = 0; i < NUM_POS / 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            index = 3 * i + j;
            b = genFromFen(positions[index].fen, &_a);
            qv = qsearch(b, MINS_INF, PLUS_INF);
            adjustedQV = b.turn? qv : -qv;
            error = positions[index].result - sigmoid(adjustedQV);
            acc[j] += error * error;
        }
    }

    return (acc[0] + acc[1] + acc[2]) / NUM_POS;
}

/* This finds a local minimum
 * It is guaranteed to terminate since the search space is finite, and simple enough
 * so that there are no bugs
 */
static void optimize(void)
{
    double bestVal = error(vals);
    int improved = 1, iter = 0;

    printf("Initial Error: %.10f\n", bestVal);


    while(improved)
    {
        iter++;
        improved = 0;
        for (int var = 0; var < NUM_VARS; ++var)
        {
            vals[var]++;
            double newVal = error(vals);
            if (newVal < bestVal)
            {
                improved = 1;
                bestVal = newVal;
            }
            else
            {
                vals[var] -= 2;
                newVal = error(vals);
                if (newVal < bestVal)
                {
                    improved = 1;
                    bestVal = newVal;
                }
                else //Reset the variable if nothing improves
                    vals[var]++;
            }
        }

        if ((iter & 15) == 0) //Output the error and save the vals every 16 iters
        {
            printf("E: %.10f\n", bestVal);
            saveArray(vals);
        }
    }

    saveArray(vals);
    printf("Optimum E: %.10f\n", error(vals));
}
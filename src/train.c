#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/train.h"
#include "../include/board.h"
#include "../include/evaluation.h"

const int isNumeric(char c)
{
    return c == '-' || (c >= '0' && c <= '9');
}

typedef struct
{
    int successful;
    int value;
} ReadO;

ReadO getNext(FILE* fp)
{
    char* line = NULL;
    size_t len = 0;
    ssize_t read;
    ReadO r = (ReadO){.successful = 0, .value = 0};

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

/* Reads all the info from path and parses the values
 */
void initTrain(const char* path)
{
    FILE* fp;
    char* line = NULL;
    size_t len = 0;
    ssize_t read;

    fp = fopen(path, "r");
    if (fp == NULL)
    {
        printf("[-] Error opening \'%s\'\n", path);
        exit(EXIT_FAILURE);
    }

    ReadO r;

    if ((r = getNext(fp)).successful) V_QUEEN = r.value;
    if ((r = getNext(fp)).successful) V_ROOK = r.value;
    if ((r = getNext(fp)).successful) V_BISH = r.value;
    if ((r = getNext(fp)).successful) V_KNIGHT = r.value;
    if ((r = getNext(fp)).successful) V_PASSEDP = r.value;
    if ((r = getNext(fp)).successful) CONNECTED_ROOKS = r.value;
    if ((r = getNext(fp)).successful) ROOK_OPEN_FILE = r.value;
    if ((r = getNext(fp)).successful) SAFE_KING = r.value;

    fclose(fp);
    if (line)
        free(line);
}
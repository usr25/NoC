#include <stdio.h>

#include "../include/global.h"
#include "../include/board.h"

void drawPosition(Board b, const int drawCoords){
    unsigned long long pos = POW2[63];
    int i, j;
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            if (pos & b.wPawns)
                printf(" P ");
            else if (pos & b.wKing)
                printf(" K ");
            else if (pos & b.wQueen)
                printf(" Q ");
            else if (pos & b.wRook)
                printf(" R ");
            else if (pos & b.wBish)
                printf(" B ");
            else if (pos & b.wKnight)
                printf(" N ");
            else if (pos & b.bPawns)
                printf(" p ");
            else if (pos & b.bKing)
                printf(" k ");
            else if (pos & b.bQueen)
                printf(" q ");
            else if (pos & b.bRook)
                printf(" r ");
            else if (pos & b.bBish)
                printf(" b ");
            else if (pos & b.bKnight)
                printf(" n ");
            else
                printf(" - ");

            pos >>= 1;
        }
        if (drawCoords)
            printf("  %d\n", 8 - i);
        else
            printf("\n");
    }
    printf("\n");
    if (drawCoords)
        printf(" a  b  c  d  e  f  g  h\n");
}
void drawBitboard(unsigned long long b)
{
    unsigned long long pos = POW2[63];
    for (int i = 64; i > 0; --i)
    {
        if (i % 8 == 0) printf("\n");
        printf("%d", pos & b?1:0);
        pos >>= 1;

    }
    printf("\n");
}

int getNextMove()
{
    int fcol, tcol;
    int frow, trow;

    printf("From col:"); scanf("%d", &fcol);
    printf("From row:"); scanf("%d", &frow);
    printf("To col:"); scanf("%d", &tcol);
    printf("To row:"); scanf("%d", &trow);

    return fcol - (int)'a' + frow - 1;
}
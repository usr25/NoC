
#include <stdio.h>

#include "../include/board.h"

void drawPosition(Board b, int drawCoords){
    unsigned long long pos = 1LLU << 63;
    int i, j;
    for (i = 0; i < SIDE; ++i)
    {
        for (j = 0; j < SIDE; ++j)
        {
            if (pos & b.white){
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

            }else if (pos & b.black){
                if (pos & b.bPawns)
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
            }else{
                printf(" - ");
            }

            pos >>= 1;
        }
        if (drawCoords)
            printf("  %d\n", SIDE - i);
        else
            printf("\n");
    }
    printf("\n");
    if (drawCoords)
        printf(" a  b  c  d  e  f  g  h\n");
    
}
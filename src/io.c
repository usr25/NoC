#include <stdio.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/io.h"

void drawPosition(Board b, const int drawCoords){
    uint64_t pos = POW2[63];
    int i, j;
    for (i = 0; i < 8; ++i)
    {
        for (j = 0; j < 8; ++j)
        {
            if (pos & b.piece[1][PAWN])
                printf(" P ");
            else if (pos & b.piece[1][KING])
                printf(" K ");
            else if (pos & b.piece[1][QUEEN])
                printf(" Q ");
            else if (pos & b.piece[1][ROOK])
                printf(" R ");
            else if (pos & b.piece[1][BISH])
                printf(" B ");
            else if (pos & b.piece[1][KNIGHT])
                printf(" N ");

            else if (pos & b.piece[0][PAWN])
                printf(" p ");
            else if (pos & b.piece[0][KING])
                printf(" k ");
            else if (pos & b.piece[0][QUEEN])
                printf(" q ");
            else if (pos & b.piece[0][ROOK])
                printf(" r ");
            else if (pos & b.piece[0][BISH])
                printf(" b ");
            else if (pos & b.piece[0][KNIGHT])
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
void drawBitboard(uint64_t b)
{
    uint64_t pos = POW2[63];
    for (int i = 64; i > 0; --i)
    {
        if (i % 8 == 0) printf("\n");
        printf("%d", pos & b?1:0);
        pos >>= 1;

    }
    printf("\n");
}
void drawMove(Move m)
{
    char a = 'a' + (m.from % 8);
    char a1 = '1' + (m.from / 8);
    char b = 'a' + (m.to % 8);
    char b1 = '1' + (m.to / 8);

    printf("%c%c%c%c", a, a1, b, b1);
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
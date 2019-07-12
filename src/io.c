#include <stdio.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/io.h"

char pieces[6] = {'k', 'q', 'r', 'b', 'n', 'p'};

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
    char a = 'h' - (m.from % 8);
    char a1 = '1' + (m.from / 8);
    char b = 'h' - (m.to % 8);
    char b1 = '1' + (m.to / 8);

    printf("%c%c%c%c", a, a1, b, b1);

    if (m.promotion)
        printf("=%c", pieces[m.promotion]);
    else if (m.castle & 1)
        printf(" O-O");
    else if (m.castle & 2)
        printf(" O-O-O");
}
void moveToText(Move m, char* mv)
{
    mv[0] = (char)('h' - (m.from % 8));
    mv[1] = (char)('1' + (m.from / 8));
    mv[2] = (char)('h' - (m.to % 8));
    mv[3] = (char)('1' + (m.to / 8));

    if (m.promotion)
        mv[4] = pieces[m.promotion];
}

void generateFen(Board b, char* c)
{
    int counter = 0;
    for (int i = 7; i >= 0; --i)
    {
        int blankSquares = 0;

        for (int j = 7; j >= 0; --j)
        {
            uint64_t pos = 1ULL << ((i << 3) + j);
            if (b.color[WHITE] & pos)
            {
                if (blankSquares){
                    c[counter++] = '0' + blankSquares;
                    blankSquares = 0;
                }

                if (b.piece[WHITE][KING] & pos)
                    c[counter++] = 'K';
                else if (b.piece[WHITE][QUEEN] & pos)
                    c[counter++] = 'Q';
                else if (b.piece[WHITE][ROOK] & pos)
                    c[counter++] = 'R';
                else if (b.piece[WHITE][BISH] & pos)
                    c[counter++] = 'B';
                else if (b.piece[WHITE][KNIGHT] & pos)
                    c[counter++] = 'N';
                else if (b.piece[WHITE][PAWN] & pos)
                    c[counter++] = 'P';
            }
            else if(b.color[BLACK] & pos)
            {
                if(blankSquares){
                    c[counter++] = '0' + blankSquares;
                    blankSquares = 0;
                }

                if (b.piece[BLACK][KING] & pos)
                    c[counter++] = 'k';
                else if (b.piece[BLACK][QUEEN] & pos)
                    c[counter++] = 'q';
                else if (b.piece[BLACK][ROOK] & pos)
                    c[counter++] = 'r';
                else if (b.piece[BLACK][BISH] & pos)
                    c[counter++] = 'b';
                else if (b.piece[BLACK][KNIGHT] & pos)
                    c[counter++] = 'n';
                else if (b.piece[BLACK][PAWN] & pos)
                    c[counter++] = 'p';
            }
            else
                blankSquares++;
        }
        if (blankSquares){
            c[counter++] = '0' + blankSquares;
            blankSquares = 0;
        }

        if (i != 0)
            c[counter++] = '/';
    }

    c[counter++] = ' ';
    c[counter++] = (b.posInfo & 1) ? 'w' : 'b';
    c[counter++] = ' ';

    if(b.posInfo & 0b11110)
    {
        if(b.posInfo & WCASTLEK) c[counter++] = 'K';
        if(b.posInfo & WCASTLEQ) c[counter++] = 'Q';
        if(b.posInfo & BCASTLEK) c[counter++] = 'k';
        if(b.posInfo & BCASTLEQ) c[counter++] = 'q';
    }
    else
        c[counter++] = '-';

    c[counter++] = ' ';

    if (b.enPass)
    {
        c[counter++] = 'h' - (b.enPass % 8);
        c[counter++] = '1' + (b.enPass / 8);
    }
    else
        c[counter++] = '-';
}
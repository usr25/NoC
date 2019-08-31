/* board.c
 * Generates the board in which the game is played along with useful functionality
 */

/*
 Black pieces
 Black pawns
 -
 -
 -
 -
 White pawns
 White pieces
*/
#include <stdio.h>
#include <assert.h>
#include "../include/global.h"
#include "../include/board.h"

#define INITIAL_WPIECES 0xffff
#define INITIAL_WPAWN 0xff00
#define INITIAL_WKING 0x8
#define INITIAL_WQUEEN 0x10
#define INITIAL_WROOK 0x81
#define INITIAL_WBISH 0x24
#define INITIAL_WKNIGHT 0x42

#define INITIAL_BPIECES 0xffff000000000000
#define INITIAL_BPAWN 0xff000000000000
#define INITIAL_BKING 0x800000000000000
#define INITIAL_BQUEEN 0x1000000000000000
#define INITIAL_BROOK 0x8100000000000000
#define INITIAL_BBISH 0x2400000000000000
#define INITIAL_BKNIGHT 0x4200000000000000

//Starting: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

int textToPiece(char piece);
int color(char piece);

//TODO: 50 moves in the fen are ignored, the moves are set to 0
Board genFromFen(char* const fen, int* counter)
{
    Board b = (Board) {.enPass = 0, .castleInfo = 0};
    int i, num, shift, col, piece;
    uint64_t pos = POW2[63];

    for (i = 0; pos != 0; ++i)
    {
        shift = 1;
        col = color(fen[i]);
        piece = textToPiece(fen[i]);

        if (piece != NO_PIECE)
        {
            b.color[col] |= pos;
            b.piece[col][piece] |= pos;
        }
        else if (fen[i] == '/')
        {
            shift = 0;
        }
        else
        {
            num = fen[i] - '0';
            pos >>= num;
            shift = 0;
        }

        pos >>= shift;
    }

    b.color[AV_WHITE] = ~b.color[WHITE];
    b.color[AV_BLACK] = ~b.color[BLACK];
    b.allPieces = b.color[WHITE] | b.color[BLACK];

    i++;

    b.turn = fen[i] == 'w';

    i++;

    int castleInfo = 0;
    while(fen[++i] != ' ' && fen[i] != '\0')
    {
        switch (fen[i])
        {
            case 'K':
                castleInfo |= WCASTLEK;
                break;
            case 'Q':
                castleInfo |= WCASTLEQ;
                break;
            case 'k':
                castleInfo |= BCASTLEK;
                break;
            case 'q':
                castleInfo |= BCASTLEQ;
                break;
        }
    }
    b.castleInfo = castleInfo;

    i++;
    if (fen[i] != '-')
    {
        b.enPass = getIndex(fen[i], fen[i+1]) - (2 * b.turn - 1) * 8;
        i++;
    }

    i+=2;

    b.fifty = 0;

    if (fen[i] >= '0' && fen[i] <= '9')
    {
        while (fen[i] >= '0' && fen[i] <= '9')
        {
            /*
            b.fifty *= 10;
            b.fifty += fen[i] - '0';
            */
            ++i;
        }

        ++i;

        while (fen[i] >= '0' && fen[i] <= '9')
        {
            b.fifty *= 10;
            b.fifty += fen[i] - '0';
            ++i;
        }
    }

    assert(POPCOUNT(b.piece[WHITE][KING]) == 1);
    assert(POPCOUNT(b.piece[BLACK][KING]) == 1);

    if (b.enPass < 8 || b.enPass >= 56)
        b.enPass = 0;
    if (b.castleInfo > 0xf)
        b.castleInfo &= 0xf;

    *counter = i;
    return b;
}

const Board defaultBoard()
{
    Board b = (Board) {};
    
    b.piece[BLACK][KING] = INITIAL_BKING;
    b.piece[BLACK][QUEEN] = INITIAL_BQUEEN;
    b.piece[BLACK][ROOK] = INITIAL_BROOK;
    b.piece[BLACK][BISH] = INITIAL_BBISH;
    b.piece[BLACK][KNIGHT] = INITIAL_BKNIGHT;
    b.piece[BLACK][PAWN] = INITIAL_BPAWN;

    b.piece[WHITE][KING] = INITIAL_WKING;
    b.piece[WHITE][QUEEN] = INITIAL_WQUEEN;
    b.piece[WHITE][ROOK] = INITIAL_WROOK;
    b.piece[WHITE][BISH] = INITIAL_WBISH;
    b.piece[WHITE][KNIGHT] = INITIAL_WKNIGHT;
    b.piece[WHITE][PAWN] = INITIAL_WPAWN;

    b.color[BLACK] = INITIAL_BPIECES;
    b.color[WHITE] = INITIAL_WPIECES;
    b.color[AV_BLACK] = ~ INITIAL_BPIECES;
    b.color[AV_WHITE] = ~ INITIAL_WPIECES;

    b.castleInfo =  0xf;
    b.allPieces = INITIAL_WPIECES | INITIAL_BPIECES;
    b.turn = WHITE;

    return b;
}

int equal(const Board* a, const Board* b)
{
    int data = 
        (a->castleInfo == b->castleInfo) && (a->allPieces == b->allPieces);
    
    int pieces = 1;

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 6 && pieces; ++j)
        {
            pieces &= a->piece[i][j] == b->piece[i][j];
        }
    }

    int other = a->turn == b->turn && a->enPass == b->enPass;

    return data && pieces && other;
}

Board duplicate(const Board b)
{
    Board a = b;
    return a;
}

int textToPiece(char piece)
{
    switch(piece)
    {
        case 'P':
        case 'p':
            return PAWN;
        case 'K':
        case 'k':
            return KING;
        case 'Q':
        case 'q':
            return QUEEN;
        case 'R':
        case 'r':
            return ROOK;
        case 'B':
        case 'b':
            return BISH;
        case 'N':
        case 'n':
            return KNIGHT;

        default:
            return NO_PIECE;
    }
}
int color(char piece)
{
    return 'A' <= piece && piece <= 'Z';
}
int getIndex(const char row, const char col)
{
    return ((col - '1') << 3) + (7 + 'a' - row);
}
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
#include "../include/global.h"
#include "../include/board.h"

#include <stdio.h>

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

Board generateFromFen(char* const fen, char* const toPlay, char* const castle)
{
    Board b = (Board) {};
    int i, num, shift;
    uint64_t pos = POW2[63];
    
    for (i = 0; pos != 0; ++i)
    {
        shift = 1;
        switch(fen[i])
        {
            case 'K':
                b.color[WHITE] |= pos;
                b.piece[1][KING] |= pos;
                break;
            case 'Q':
                b.color[WHITE] |= pos;
                b.piece[1][QUEEN] |= pos;
                break;
            case 'R':
                b.color[WHITE] |= pos;
                b.piece[1][ROOK] |= pos;
                break;
            case 'B':
                b.color[WHITE] |= pos;
                b.piece[1][BISH] |= pos;
                break;
            case 'N':
                b.color[WHITE] |= pos;
                b.piece[1][KNIGHT] |= pos;
                break;
            case 'P':
                b.color[WHITE] |= pos;
                b.piece[1][PAWN] |= pos;
                break;

            case 'k':
                b.color[BLACK] |= pos;
                b.piece[0][KING] |= pos;
                break;
            case 'q':
                b.color[BLACK] |= pos;
                b.piece[0][QUEEN] |= pos;
                break;
            case 'r':
                b.color[BLACK] |= pos;
                b.piece[0][ROOK] |= pos;
                break;
            case 'b':
                b.color[BLACK] |= pos;
                b.piece[0][BISH] |= pos;
                break;
            case 'n':
                b.color[BLACK] |= pos;
                b.piece[0][KNIGHT] |= pos;
                break;
            case 'p':
                b.color[BLACK] |= pos;
                b.piece[0][PAWN] |= pos;
                break;
            case '/': 
                shift = 0;
                break;

            default:
                num = fen[i] - (int)'0';
                pos >>= num;
                shift = 0;
        }

        pos >>= shift;
    }

    b.color[AV_WHITE] = ALL ^ b.color[WHITE];
    b.color[AV_BLACK] = ALL ^ b.color[BLACK];

    b.allPieces = b.color[WHITE] | b.color[BLACK];

    int castleInfo = 0;

    for (int i = 0, brk = 1; brk && i < 4; ++i)
    {
        switch (castle[i])
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
            default:
                brk = 0;
                break;
        }
    }

    b.posInfo = castleInfo | (toPlay[0] == 'w');

    return b;
}

Board defaultBoard()
{
    Board b = (Board) {};
    
    b.piece[0][KING] = INITIAL_BKING;
    b.piece[0][QUEEN] = INITIAL_BQUEEN;
    b.piece[0][ROOK] = INITIAL_BROOK;
    b.piece[0][BISH] = INITIAL_BBISH;
    b.piece[0][KNIGHT] = INITIAL_BKNIGHT;
    b.piece[0][PAWN] = INITIAL_BPAWN;

    b.piece[1][KING] = INITIAL_WKING;
    b.piece[1][QUEEN] = INITIAL_WQUEEN;
    b.piece[1][ROOK] = INITIAL_WROOK;
    b.piece[1][BISH] = INITIAL_WBISH;
    b.piece[1][KNIGHT] = INITIAL_WKNIGHT;
    b.piece[1][PAWN] = INITIAL_WPAWN;

    b.color[BLACK] = INITIAL_BPIECES;
    b.color[WHITE] = INITIAL_WPIECES;
    b.color[AV_BLACK] = ALL ^ INITIAL_BPIECES;
    b.color[AV_WHITE] = ALL ^ INITIAL_WPIECES;

    b.posInfo =  0b11111;
    b.allPieces = INITIAL_WPIECES | INITIAL_BPIECES;

    return b;
}


int capturePiece(Board* b, const uint64_t pos, const int colorToCapture)
{
    int targetPiece = pieceAt(b, pos, colorToCapture);

    if (targetPiece != NO_PIECE)
        b->piece[colorToCapture][targetPiece] ^= pos;

    return targetPiece;
}

int pieceAt(Board* const b, const uint64_t pos, const int color)
{
    if (pos & b->piece[color][PAWN])     return PAWN;
    else if (pos & b->piece[color][ROOK]) return ROOK;
    else if (pos & b->piece[color][BISH]) return BISH;
    else if (pos & b->piece[color][KNIGHT]) return KNIGHT;
    else if (pos & b->piece[color][QUEEN]) return QUEEN;
    else if (pos & b->piece[color][KING]) return KING;
    
    return NO_PIECE;
}

int equal(Board* a, Board* b)
{
    int data = 
        (a->posInfo == b->posInfo) && (a->allPieces == b->allPieces);
    
    int pieces = 1;

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 6 && pieces; ++j)
        {
            pieces &= a->piece[i][j] == b->piece[i][j];
        }
    }

    return data && pieces;
}

Board duplicate(Board b)
{
    
    Board a = (Board) {};

    for (int i = 0; i < 6; ++i)
    {
        a.piece[0][i] = b.piece[0][i];
        a.piece[1][i] = b.piece[1][i];
    }
    for (int i = 0; i < 4; ++i)
        a.color[i] = b.color[i];

    a.posInfo = b.posInfo;
    a.allPieces = b.allPieces;

    return a;

}

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

#include "../include/board.h"


#define INITIAL_WPIECES 0xffff
#define INITIAL_WPAWNS 0xff00
#define INITIAL_WKING 0x8
#define INITIAL_WQUEEN 0x10
#define INITIAL_WROOK 0x81
#define INITIAL_WBISH 0x24
#define INITIAL_WKNIGHT 0x42

#define INITIAL_BPIECES 0xffff000000000000
#define INITIAL_BPAWNS 0xff000000000000
#define INITIAL_BKING 0x800000000000000
#define INITIAL_BQUEEN 0x1000000000000000
#define INITIAL_BROOK 0x8100000000000000
#define INITIAL_BBISH 0x2400000000000000
#define INITIAL_BKNIGHT 0x4200000000000000

//Starting: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

Board generateFromFen(char* const fen, char* const toPlay)
{
    Board b = (Board) {};
    int i, num, shift;
    unsigned long long pos = 1ULL << 63;
    
    for (i = 0; pos != 0; ++i)
    {
        shift = 1;
        switch(fen[i])
        {
            case 'K':
                b.white |= pos;
                b.wKing |= pos;
                break;
            case 'Q':
                b.white |= pos;
                b.wQueen |= pos;
                break;
            case 'R':
                b.white |= pos;
                b.wRook |= pos;
                break;
            case 'B':
                b.white |= pos;
                b.wBish |= pos;
                break;
            case 'N':
                b.white |= pos;
                b.wKnight |= pos;
                break;
            case 'P':
                b.white |= pos;
                b.wPawns |= pos;
                break;

            case 'k':
                b.black |= pos;
                b.bKing |= pos;
                break;
            case 'q':
                b.black |= pos;
                b.bQueen |= pos;
                break;
            case 'r':
                b.black |= pos;
                b.bRook |= pos;
                break;
            case 'b':
                b.black |= pos;
                b.bBish |= pos;
                break;
            case 'n':
                b.black |= pos;
                b.bKnight |= pos;
                break;
            case 'p':
                b.black |= pos;
                b.bPawns |= pos;
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
    int whiteToPlay = toPlay[0] == 'w';

    b.avWhite = ALL ^ b.white;
    b.avBlack = ALL ^ b.black;

    return b;
}

Board defaultBoard()
{
    Board b = (Board) {
    .white = INITIAL_WPIECES, .avWhite = ALL ^ INITIAL_WPIECES, .wPawns = INITIAL_WPAWNS, 
    .wKing = INITIAL_WKING, .wQueen = INITIAL_WQUEEN, .wRook = INITIAL_WROOK, .wBish = INITIAL_WBISH, .wKnight = INITIAL_WKNIGHT,
    
    .black = INITIAL_BPIECES, .avBlack = ALL ^ INITIAL_BPIECES, .bPawns = INITIAL_BPAWNS, 
    .bKing = INITIAL_BKING, .bQueen = INITIAL_BQUEEN, .bRook = INITIAL_BROOK, .bBish = INITIAL_BBISH, .bKnight = INITIAL_BKNIGHT
    };

    return b;
}
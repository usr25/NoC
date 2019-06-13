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

Board generateFromFen(char const* fen)
{
    return (Board) {};
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
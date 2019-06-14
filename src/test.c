#include <stdio.h>

#include "../include/board.h"

int validStartingPos(Board b)
{
    int correctNumOfPieces = 
        __builtin_popcountll(b.wPawns) == 8 &&
        __builtin_popcountll(b.bPawns) == 8 &&
        __builtin_popcountll(b.wKing) == 1 &&
        __builtin_popcountll(b.bKing) == 1 &&
        __builtin_popcountll(b.wQueen) == 1 &&
        __builtin_popcountll(b.bQueen) == 1 &&
        __builtin_popcountll(b.wRook) == 2 &&
        __builtin_popcountll(b.bRook) == 2 &&
        __builtin_popcountll(b.wBish) == 2 &&
        __builtin_popcountll(b.bBish) == 2 &&
        __builtin_popcountll(b.wKnight) == 2 &&
        __builtin_popcountll(b.bKnight) == 2;

    int piecesAddUp = __builtin_popcountll(b.white) == 16 && __builtin_popcountll(b.black) == 16;
    int availableAddUp = __builtin_popcountll(b.avWhite) == 48 && __builtin_popcountll(b.avBlack) == 48;
    int avAreCorrect = (b.white & b.avWhite) == 0 && (b.black & b.avBlack) == 0;
    int whiteAreInAvBlack = (b.white & b.avBlack) == b.white;
    int blackAreInAvWhite = (b.black & b.avWhite) == b.black;

    return 
    correctNumOfPieces && piecesAddUp && availableAddUp && avAreCorrect && whiteAreInAvBlack && blackAreInAvWhite;
}

int validPieces(Board b)
{
    int validKings = (__builtin_popcountll(b.wKing) == 1) && (__builtin_popcountll(b.bKing) == 1);
    int totNumPieces = (__builtin_popcountll(b.white) <= 16) && (__builtin_popcountll(b.black) <= 16);
    int validNumPawns = (__builtin_popcountll(b.wPawns) <= 8) && (__builtin_popcountll(b.bPawns) <= 8);

    return validKings && totNumPieces && validNumPawns;
}
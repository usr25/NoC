#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/io.h"

#include <stdio.h>

unsigned long long posKingMoves(Board* b, int color)
{   
    if (color)
        return getKingMoves(__builtin_ctzll(b->wKing)) & b->avWhite;
    else
        return getKingMoves(__builtin_ctzll(b->bKing)) & b->avBlack;
}

unsigned long long posKnightMoves(Board* b, int color, int index)
{
    unsigned long long pos = (color)?b->wKnight:b->bKnight;
    while(index--) pos &= (pos - 1);

    if (color)
        return getKnightMoves(__builtin_ctzll(pos)) & b->avWhite;
    else
        return getKnightMoves(__builtin_ctzll(pos)) & b->avBlack;
}

unsigned long long posPawnMoves(Board* b, int color, int index)
{
    unsigned long long pos = (color)?b->wPawns:b->bPawns;
    while(index--) pos &= (pos - 1);

    int i = __builtin_ctzll(pos);
    unsigned long long forward;
    if (color){
        forward = getWhitePawnMoves(i) & (b->avWhite ^ b->black);
        
        if (i < 16 && ((1ULL << (8 + i)) & b->pieces))
            return getWhitePawnCaptures(i) & b->black;
        
        return forward | (getWhitePawnCaptures(i) & b->black);
    }
    else{
        forward = getBlackPawnMoves(i) & (b->avBlack ^ b->white);
        
        if (i > 47 && ((1ULL << (i - 8)) & b->pieces))
            return getBlackPawnCaptures(i) & b->white;
        
        return forward | (getBlackPawnCaptures(i) & b->white);
    }
}

unsigned long long posRookMoves(Board* b, int color, int index)
{
    unsigned long long pos = (color)?b->wRook:b->bRook;
    while(index--) pos &= (pos - 1);
    int i = __builtin_ctzll(pos);

    unsigned long long inteUp = getUpMoves(i) & b->pieces;
    unsigned long long inteDown = getDownMoves(i) & b->pieces;
    unsigned long long inteRight = getRightMoves(i) & b->pieces;
    unsigned long long inteLeft = getLeftMoves(i) & b->pieces;

    unsigned long long res = 0;
    int obstacle;


    if (inteUp){
        obstacle = __builtin_ctzll(inteUp);
        res |= getUpMoves(i) ^ getUpMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getUpMoves(i);
    
    if (inteDown){
        obstacle = 63 - __builtin_clzll(inteDown);
        res |= getDownMoves(i) ^ getDownMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getDownMoves(i);
    
    if (inteRight){
        obstacle = 63 - __builtin_clzll(inteRight);
        res |= getRightMoves(i) ^ getRightMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getRightMoves(i);
    
    if (inteLeft){
        obstacle = __builtin_ctzll(inteLeft);
        res |= getLeftMoves(i) ^ getLeftMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getLeftMoves(i);


    return res;
}

unsigned long long posBishMoves(Board* b, int color, int index)
{
    unsigned long long pos = (color)?b->wBish:b->bBish;
    while(index--) pos &= (pos - 1);
    int i = __builtin_ctzll(pos);

    unsigned long long inteUpRight = getUpRightMoves(i) & b->pieces;
    unsigned long long inteUpLeft = getUpLeftMoves(i) & b->pieces;
    unsigned long long inteDownRight = getDownRightMoves(i) & b->pieces;
    unsigned long long inteDownLeft = getDownLeftMoves(i) & b->pieces;

    unsigned long long res = 0;
    int obstacle;


    if (inteUpRight){
        obstacle = __builtin_ctzll(inteUpRight);
        res |= getUpRightMoves(i) ^ getUpRightMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getUpRightMoves(i);
    
    if (inteUpLeft){
        obstacle = __builtin_ctzll(inteUpLeft);
        res |= getUpLeftMoves(i) ^ getUpLeftMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getUpLeftMoves(i);
    
    if (inteDownRight){
        obstacle = 63 - __builtin_clzll(inteDownRight);
        res |= getDownRightMoves(i) ^ getDownRightMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getDownRightMoves(i);
    
    if (inteDownLeft){
        obstacle = 63 - __builtin_clzll(inteDownLeft);
        res |= getDownLeftMoves(i) ^ getDownLeftMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getDownLeftMoves(i);


    return res;
}
unsigned long long posQueenMoves(Board* b, int color, int index)
{
    unsigned long long pos = (color)?b->wQueen:b->bQueen;
    while(index--) pos &= (pos - 1);
    int i = __builtin_ctzll(pos);

    unsigned long long inteUp = getUpMoves(i) & b->pieces;
    unsigned long long inteDown = getDownMoves(i) & b->pieces;
    unsigned long long inteRight = getRightMoves(i) & b->pieces;
    unsigned long long inteLeft = getLeftMoves(i) & b->pieces;

    unsigned long long inteUpRight = getUpRightMoves(i) & b->pieces;
    unsigned long long inteUpLeft = getUpLeftMoves(i) & b->pieces;
    unsigned long long inteDownRight = getDownRightMoves(i) & b->pieces;
    unsigned long long inteDownLeft = getDownLeftMoves(i) & b->pieces;

    unsigned long long res = 0;
    int obstacle;

    if (inteUp){
        obstacle = __builtin_ctzll(inteUp);
        res |= getUpMoves(i) ^ getUpMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getUpMoves(i);
    
    if (inteDown){
        obstacle = 63 - __builtin_clzll(inteDown);
        res |= getDownMoves(i) ^ getDownMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getDownMoves(i);
    
    if (inteRight){
        obstacle = 63 - __builtin_clzll(inteRight);
        res |= getRightMoves(i) ^ getRightMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getRightMoves(i);
    
    if (inteLeft){
        obstacle = __builtin_ctzll(inteLeft);
        res |= getLeftMoves(i) ^ getLeftMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getLeftMoves(i);
    

    if (inteUpRight){
        obstacle = __builtin_ctzll(inteUpRight);
        res |= getUpRightMoves(i) ^ getUpRightMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getUpRightMoves(i);
    
    if (inteUpLeft){
        obstacle = __builtin_ctzll(inteUpLeft);
        res |= getUpLeftMoves(i) ^ getUpLeftMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getUpLeftMoves(i);
    
    if (inteDownRight){
        obstacle = 63 - __builtin_clzll(inteDownRight);
        res |= getDownRightMoves(i) ^ getDownRightMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getDownRightMoves(i);
    
    if (inteDownLeft){
        obstacle = 63 - __builtin_clzll(inteDownLeft);
        res |= getDownLeftMoves(i) ^ getDownLeftMoves(obstacle);
        res ^= color ? (1ULL << obstacle) & b->white : (1ULL << obstacle) & b->black;
    } else res |= getDownLeftMoves(i);


    return res;
}
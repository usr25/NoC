/* perft.c
 * Generates the perft for any given position
 */

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/movegen.h"
#include "../include/hash.h"
#include "../include/io.h"
#include "../include/nnue.h"
#include "../include/perft.h"


uint64_t perftRecursive(Board b, const int depth)
{
    if (depth == 0) return 1;

    Move moves[NMOVES];
    History h;
    uint64_t tot = 0;

    const int numMoves = legalMoves(&b, moves) >> 1;

    const int fifty = b.fifty;
    if (depth == 1)
        tot = numMoves;
    else
    {
        for (int i = 0; i < numMoves; ++i)
        {
            makeMove(&b, moves[i], &h);
            assert(boardIsOK(&b));
            assert((moves[i].piece == PAWN || moves[i].capture > 0) || b.fifty == fifty+1);
            assert(!(moves[i].piece == PAWN || moves[i].capture > 0) || b.fifty == 0);
            undoMove(&b, moves[i], &h);
            makePermaMove(&b, moves[i]);
            assert(boardIsOK(&b));
            assert((moves[i].piece == PAWN || moves[i].capture > 0) || b.fifty == fifty+1);
            assert(!(moves[i].piece == PAWN || moves[i].capture > 0) || b.fifty == 0);
            tot += perftRecursive(b, depth - 1);
            undoMove(&b, moves[i], &h);
            assert(boardIsOK(&b));
            assert(b.fifty == fifty);
        }
    }

    return tot;
}

uint64_t perft(Board b, const int depth, const int divide)
{
    if (depth == 0) return 1;

    Move moves[NMOVES];
    History h;
    uint64_t tot = 0, temp;

    const int numMoves = legalMoves(&b, moves) >> 1;

    if (depth == 1)
        return numMoves;

    for (int i = 0; i < numMoves; ++i)
    {
        assert(moveIsValidBasic(&b, &moves[i]));
        makeMove(&b, moves[i], &h);

        temp = perftRecursive(b, depth-1);

        if (divide)
        {
            drawMove(moves[i]);
            printf(": %lu\n", temp);
        }

        tot += temp;

        undoMove(&b, moves[i], &h);
    }

    return tot;
}

uint64_t perftMovegen(Board b, const int depth, const int divide) {

    if (depth == 0) return 1;

    MoveGen mg = newMG(&b, 0, (Move){.from = -1});
    Move m;
    History h;
    uint64_t tot = 0, temp = 0;

    int i = 0, t = 0, sNMoves = 0;
    while (1) {
        assert(mg.nmoves >= 0);
        m = next(&mg, &b);
        if (mg.state == Exhausted) break;
        if (mg.currmove == 1){
            t++;
            sNMoves += mg.nmoves;
        }
        assert(m.from != -1);
        makeMove(&b, m, &h);

        temp = perftMovegen(b, depth-1, 0);

        if (divide)
        {
            drawMove(m);
            printf(": %lu\n", temp);
        }

        tot += temp;

        undoMove(&b, m, &h);
        i++;
    }

    assert(t <= 2);
    assert(sNMoves == i && sNMoves == mg.tot);

    return tot;
}

/* This is an especial version of the perft to ensure that the zobrist
 * hash update works
 */
int hashPerft(Board b, const int depth, const uint64_t prevHash)
{
    if (depth == 0) return 1;

    Move moves[NMOVES];
    History h;

    const int numMoves = legalMoves(&b, moves) >> 1;

    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, moves[i], &h);
        uint64_t newHash = makeMoveHash(prevHash, &b, moves[i], h);

        assert(newHash == hashPosition(&b));

        if (newHash != hashPosition(&b) || !hashPerft(b, depth - 1, newHash))
            return 0;

        undoMove(&b, moves[i], &h);
    }

    return 1;
}

static NNUE dummy;

void initDummy(void)
{
    dummy = (NNUE) {};
    dummy.ftBiases = (int16_t*)malloc(sizeof(int16_t)*kHalfDimensionFT);
    dummy.ftWeights = (int16_t*)malloc(sizeof(int16_t)*kHalfDimensionFT*kInputDimensionsFT);
    CHECK_MALLOC(dummy.ftBiases);
    CHECK_MALLOC(dummy.ftWeights);

    for (int i = 0; i < kHalfDimensionFT; ++i)
    {
        dummy.ftBiases[i] += (int16_t)((i*23+i-81) & 65535);
        for (int j = 0; j < kInputDimensionsFT; ++j)
            dummy.ftWeights[i*kInputDimensionsFT+j] = (int16_t)((j*23+i+1+j) & 65535);
    }
}

void freeDummy(void)
{
    freeNNUE(&dummy);
}

int nnuePerft(Board b, const int depth, int16_t* test)
{
    if (depth == 0) return 1;

    Move moves[NMOVES];
    History h;

    const int numMoves = legalMoves(&b, moves) >> 1;
    int16_t* nInputBef = malloc(sizeof(int16_t)*kDimensionFT);
    int16_t* nInputAft = malloc(sizeof(int16_t)*kDimensionFT);
    CHECK_MALLOC(nInputBef);
    CHECK_MALLOC(nInputAft);

    inputLayer(&dummy, &b, WHITE, nInputBef);
    inputLayer(&dummy, &b, BLACK, nInputBef+kHalfDimensionFT);

    if (test == NULL)
    {
        test = malloc(sizeof(int16_t)*kDimensionFT);
        CHECK_MALLOC(test);
        memcpy(test, nInputBef, sizeof(int16_t)*kDimensionFT);
    }

    for (int i = 0; i < numMoves; ++i)
    {
        NNUEChangeList q = (NNUEChangeList){.idx = 0};
        determineChanges(moves[i], &q, b.stm);
        assert(q.idx < 5);

        makeMove(&b, moves[i], &h);

        inputLayer(&dummy, &b, WHITE, nInputAft);
        inputLayer(&dummy, &b, BLACK, nInputAft + 256);
/*
        applyChanges(&dummy, &b, &q, WHITE, test);
        applyChanges(&dummy, &b, &q, BLACK, test + 256);
*/
        for (int j = 0; j < 512; ++i)
        {
            if (test[j] != nInputAft[j])
                return 0;
        }

        if (!nnuePerft(b, depth - 1, test))
            return 0;

        undoMove(&b, moves[i], &h);

        assert(q.idx < 5);

        for (int j = 0; j < q.idx; ++j)
            q.changes[j].appears ^= 1;
/*
        applyChanges(&dummy, &b, &q, WHITE, test);
        applyChanges(&dummy, &b, &q, BLACK, test+256);
*/
        for (int i = 0; i < 512; ++i)
        {
            if (test[i] != nInputBef[i])
                return 0;
        }
    }

    return 1;
}
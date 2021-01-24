#ifdef NNUE_SPARSE

#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/nnue.h"
#include "../include/nnuearch.h"

//static const int dimensions[5] = {41024, 512, 32, 32, 1};

#define mask_t int16_t

static clipped_t clippedInput[kDimensionFT];
static clipped_t hiddenLayer1[kDimensionHidden];
static clipped_t hiddenLayer2[kDimensionHidden];

const int getIdx(const int i, const int j, const int dim)
{
    return j*kDimensionHidden+i;
}

static void propagateInput(const int16_t* __restrict__ input, const int stm,
    clipped_t* __restrict__ nextLayer,
    const weight_t* ws, const int32_t* bs)
{
    assert(stm == 1 || stm == 0);

    int32_t tmp[kDimensionHidden];
    for (int i = 0; i < kDimensionHidden; ++i)
        tmp[i] = bs[i];

    const int offset = (1^stm)*kHalfDimensionFT;
    const int offset2 = kHalfDimensionFT ^ offset;

    for (int i = 0; i < kDimensionFT; ++i)
        clippedInput[i] = clip(input[i]);

    int idx, ni;
    for (int i = 0; i < kHalfDimensionFT; ++i)
    {
        idx = i + offset;
        if (clippedInput[idx])
            for (int j = 0; j < kDimensionHidden; ++j)
                tmp[j] += clippedInput[idx]*ws[kDimensionHidden*i+j];

        idx = i + offset2;
        ni = i + kHalfDimensionFT;
        if (clippedInput[idx])
            for (int j = 0; j < kDimensionHidden; ++j)
                tmp[j] += clippedInput[idx]*ws[kDimensionHidden*ni+j];
    }

    for (unsigned i = 0; i < kDimensionHidden; i++)
        nextLayer[i] = clip64(tmp[i]);
}

static void propagate(const clipped_t* __restrict__ prevLayer, const int prevSize,
    clipped_t* __restrict__ nextLayer, const int nextSize,
    const weight_t* ws, const int32_t* bs)
{
    int32_t tmp[kDimensionHidden];
    for (int i = 0; i < kDimensionHidden; ++i)
        tmp[i] = bs[i];

    for (int i = 0; i < prevSize; ++i)
    {
        if (prevLayer[i])
            for (int j = 0; j < kDimensionHidden; ++j)
                tmp[j] += prevLayer[i]*ws[kDimensionHidden*i+j];
    }

    for (unsigned i = 0; i < kDimensionHidden; i++)
        nextLayer[i] = clip64(tmp[i]);
}

static int32_t output(const clipped_t* __restrict__ prevLayer,
    const weight_t* __restrict__ ws, int32_t out)
{
    for (int i = 0; i < kDimensionHidden; ++i)
        out += prevLayer[i]*ws[i];

    return out;
}

int evaluate(const NNUE* nn, const Board* b, int16_t* nInput)
{
    inputLayer(nn, b, WHITE, nInput);
    inputLayer(nn, b, BLACK, nInput + kHalfDimensionFT);

    propagateInput(nInput, b->stm, hiddenLayer1, nn->weights1, nn->biases1);
    propagate(hiddenLayer1, dimensions[2], hiddenLayer2, dimensions[3], nn->weights2, nn->biases2);

    int32_t out = output(hiddenLayer2, nn->outputW, *(nn->outputB));

    return out / FV_SCALE;
}

//#define TEST_ACC

#ifdef TEST_ACC
static int32_t testInput[kDimensionFT];
#endif
int evaluateAcc(const NNUE* nn, const Board* const b, const int16_t* nInput)
{
    #ifdef TEST_ACC
    inputLayer(nn, b, WHITE, testInput);
    inputLayer(nn, b, BLACK, testInput+kHalfDimensionFT);

    for (int i = 0; i < kDimensionFT; ++i)
        assert(testInput[i] == nInput[i]);
    #endif

    propagateInput(nInput, b->stm, hiddenLayer1, nn->weights1, nn->biases1);
    propagate(hiddenLayer1, dimensions[2], hiddenLayer2, dimensions[3], nn->weights2, nn->biases2);

    const int32_t out = output(hiddenLayer2, nn->outputW, *(nn->outputB));

    return out / FV_SCALE;
}
#endif
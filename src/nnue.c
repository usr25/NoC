#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/nnue.h"

#define CHECK_READ(read,correct) if (read != correct) {fprintf(stderr, "Unsuccessful read in %s %d\n", __FILE__, __LINE__); \
                                exit(5);}

void readHeaders(FILE* f, NNUE* nn);
void readParams(FILE* f, NNUE* nn);
void readWeights(FILE* f, weight_t* nn, const int dims, const int isOutput);
void showNNUE(const NNUE* nn);

const int dimensions[5] = {41024, 512, 32, 32, 1};
const int kHalfDimensionFT = 256;
const int kInputDimensionsFT = 41024;

const uint32_t FTHeader = 0x5d69d7b8;
const uint32_t NTHeader = 0x63337156;
const uint32_t NNUEHash = 0x3e5aa6eeU;
const uint32_t ArchSize = 177;

const int FV_SCALE = 16;

//TODO: Make the nnue a static variable here

enum {
  PS_W_PAWN   =  1,
  PS_B_PAWN   =  1 * 64 + 1,
  PS_W_KNIGHT =  2 * 64 + 1,
  PS_B_KNIGHT =  3 * 64 + 1,
  PS_W_BISHOP =  4 * 64 + 1,
  PS_B_BISHOP =  5 * 64 + 1,
  PS_W_ROOK   =  6 * 64 + 1,
  PS_B_ROOK   =  7 * 64 + 1,
  PS_W_QUEEN  =  8 * 64 + 1,
  PS_B_QUEEN  =  9 * 64 + 1,
  PS_END      = 10 * 64 + 1
};

const uint32_t PieceToIndex[2][16] = {
  { 0, PS_B_PAWN, PS_B_KNIGHT, PS_B_BISHOP, PS_B_ROOK, PS_B_QUEEN, 0, 0,
    0, PS_W_PAWN, PS_W_KNIGHT, PS_W_BISHOP, PS_W_ROOK, PS_W_QUEEN, 0, 0 },
  { 0, PS_W_PAWN, PS_W_KNIGHT, PS_W_BISHOP, PS_W_ROOK, PS_W_QUEEN, 0, 0,
    0, PS_B_PAWN, PS_B_KNIGHT, PS_B_BISHOP, PS_B_ROOK, PS_B_QUEEN, 0, 0 }
};

const int clip(const int v)
{
    return v<0? 0 : (v>127? 127 : v);
}

NNUE loadNNUE(const char* path)
{
    assert(sizeof(uint32_t) == 4);
    assert(dimensions[2] == dimensions[3]);
    assert(dimensions[2] == 32);
    assert(kHalfDimensionFT == dimensions[1] / 2);

    NNUE nn = (NNUE) {.i = 0};

    FILE* f = fopen(path, "r");

    if (!f)
    {
        fprintf(stderr, "Can't open nnue file: %s\n", path);
        exit(5);
    }

    #ifdef NNUE_DEBUG
        printf("Loading NNUE %s\n", path);
    #endif

    nn.ftBiases = (int16_t*)malloc(sizeof(int16_t)*kHalfDimensionFT);
    nn.ftWeights = (int16_t*)malloc(sizeof(int16_t)*kHalfDimensionFT*kInputDimensionsFT);
    CHECK_MALLOC(nn.ftBiases);
    CHECK_MALLOC(nn.ftWeights);

    readHeaders(f, &nn);
    readParams(f, &nn);

    fclose(f);

    #ifdef NNUE_DEBUG
    if (0)
        showNNUE(&nn);
    #endif

    printf("%s NNUE loaded\n", path);

    return nn;
}

void readHeaders(FILE* f, NNUE* nn)
{
    uint32_t version, hash, size;
    int successfulRead = 1;

    successfulRead &= fread(&version, sizeof(uint32_t), 1, f);
    successfulRead &= fread(&hash, sizeof(uint32_t), 1, f);
    successfulRead &= fread(&size, sizeof(uint32_t), 1, f);

    assert(version == NNUEVersion);
    assert(hash == NNUEHash);
    assert(size == ArchSize);
    CHECK_READ(successfulRead, 1);

    char* architecture = (char*)malloc(ArchSize);
    CHECK_MALLOC(architecture);

    successfulRead = fread(architecture, sizeof(char), ArchSize, f);
    CHECK_READ(successfulRead, ArchSize);

    #ifdef NNUE_DEBUG
        printf("Version: %u\n", version);
        printf("Hash: %u\n", hash);
        printf("Size: %u\n", size);
        printf("Architecture: %s\n", architecture);
    #endif
}

//Code copied from evaluate_nnue
void readParams(FILE* f, NNUE* nn)
{
    uint32_t header;
    int successfulRead = 1;

    //First read the feature transformer

    successfulRead = fread(&header, sizeof(uint32_t), 1, f);
    CHECK_READ(successfulRead, 1);
    assert(header == FTHeader);

    #ifdef NNUE_DEBUG
        printf("Header_FT: %u\n", header);
    #endif

    successfulRead = fread(nn->ftBiases, sizeof(nn->ftBiases[0]), kHalfDimensionFT, f);
    CHECK_READ(successfulRead, kHalfDimensionFT);

    successfulRead = fread(nn->ftWeights, sizeof(nn->ftWeights[0]), kHalfDimensionFT*kInputDimensionsFT, f);
    CHECK_READ(successfulRead, kHalfDimensionFT*kInputDimensionsFT);


    //Now read the network

    successfulRead = fread(&header, sizeof(uint32_t), 1, f);
    CHECK_READ(successfulRead, 1);
    assert(header == NTHeader);

    #ifdef NNUE_DEBUG
        printf("Header_Network: %u\n", header);
    #endif

    successfulRead = fread(nn->biases1, sizeof(nn->biases1[0]), dimensions[2], f);
    CHECK_READ(successfulRead, dimensions[3]);
    readWeights(f, nn->weights1, dimensions[1], 0);

    successfulRead = fread(nn->biases2, sizeof(nn->biases2[0]), dimensions[3], f);
    CHECK_READ(successfulRead, dimensions[3]);
    readWeights(f, nn->weights2, dimensions[2], 0);

    successfulRead = fread(nn->outputB, sizeof(nn->outputB[0]), dimensions[4], f);
    CHECK_READ(successfulRead, dimensions[4]);
    readWeights(f, nn->outputW, 1, 1);

    int ignore, cnt = 0;
    while (fread(&ignore, sizeof(int), 1, f))
        cnt++;

    if (cnt)
    {
        fprintf(stderr, "NNUE file hasn't been read completely, %d ints remeain\n", cnt);
        exit(10);
    }
}

//This is depends on sparse/regular
const int getIdx(const int i, const int j, const int dim)
{
    return i*dim+j;
}

void readWeights(FILE* f, weight_t* ws, const int dims, const int isOutput)
{
    for (int i = 0; i < 32; ++i)
    {
        for (int j = 0; j < dims; ++j)
        {
            int8_t a;
            int s = fread(&a, 1, 1, f);
            CHECK_READ(s, 1);
            //Output is the same whether it is sparse or regular
            int idx = isOutput? i*dims+j : getIdx(i,j,dims);
            ws[idx] = (weight_t)a;
        }
    }
}

//TODO: make this a "save nn into binary file" function
void showNNUE(const NNUE* nn)
{
    //./noc | tail --lines=+9 | sha256sum
    //./cfish | head --lines=-2 | tail --lines=+4 | sha256sum

    //Input will be needed
    //FT
    for (int i = 0; i < kHalfDimensionFT; ++i)
    {
        printf("%d, ", nn->ftBiases[i]);
    }
    printf("\n");

    for (int i = 0; i < kHalfDimensionFT*kInputDimensionsFT; ++i)
    {
        printf("%d, ", nn->ftWeights[i]);
    }
    printf("\n");

    //First layer
    for (int i = 0; i < 32; ++i)
    {
        printf("%d, ", nn->biases1[i]);
    }
    printf("\n");
    for (int i = 0; i < 512*32; ++i)
    {
        printf("%d, ", nn->weights1[i]);
    }
    printf("\n");
    
    //Second layer
    for (int i = 0; i < 32; ++i)
    {
        printf("%d, ", nn->biases2[i]);
    }
    printf("\n");
    for (int i = 0; i < 32*32; ++i)
    {
        printf("%d, ", nn->weights2[i]);
    }
    printf("\n");

    //Output layer
    printf("%d\n", nn->outputB[0]);
    for (int i = 0; i < 1*32; ++i)
    {
        printf("%d, ", nn->outputW[i]);
    }
    printf("\n");
}

void freeNNUE(NNUE* nn)
{
    free(nn->ftBiases);
    free(nn->ftWeights);
}

unsigned makeIndex(const int c, const int sq, const int pc, const int ksq)
{
    return sq + PieceToIndex[c][pc] + PS_END * ksq;
}

//Sqrs aren't the same in this engine than in sf
//also, flip it if it is the opposing side
int toSf(const int c, const int sq)
{
    int col = sq & 7;
    int row = sq >> 3;
    return ((row << 3) + (7 - col))^(c==WHITE? 0 : 0x3f);
}

//Calculates the input layer for a given color (king-piece, king is of color)
int32_t* inputLayer(const NNUE* nn, const Board b, const int color, int32_t* inp)
{
    assert(POPCOUNT(b.allPieces) <= 32);
    unsigned actives[30];
    int numActives = 0;

    //memcpy(inp, nn->ftBiases, sizeof(int16_t)*kHalfDimensionFT);

    int ksq = toSf(color, LSB_INDEX(b.piece[color][KING]));

    for (int c = BLACK; c <= WHITE; ++c)
    {
        for (int piece = QUEEN; piece <= PAWN; ++piece)
        {
            int sfPc = (c==WHITE? 6 : 14) - piece;
            uint64_t bb = b.piece[c][piece];
            while (bb)
            {
                int sq = toSf(color, LSB_INDEX(bb));
                int idx = makeIndex(color, sq, sfPc, ksq);
                actives[numActives++] = idx;
                REMOVE_LSB(bb);
            }
        }
    }

    assert(numActives == POPCOUNT(b.allPieces)-2);

    for (int i = 0; i < kHalfDimensionFT; ++i)
    {
        inp[i] = nn->ftBiases[i];
    }

    for (int i = 0; i < numActives; ++i)
    {
        int offset = kHalfDimensionFT * actives[i];
        for (int j = 0; j < kHalfDimensionFT; ++j)
            inp[j] += nn->ftWeights[offset+j];
    }
    for (int i = 0; i < kHalfDimensionFT; ++i)
    {
        inp[i] = clip(inp[i]);
    }

    return inp;
}

void propagate(const int32_t* prevLayer, const int prevSize, int32_t* nextLayer, const int nextSize, const weight_t* ws, const int32_t* bs)
{
    for (int i = 0; i < nextSize; ++i)
    {
        int sum = bs[i];
        for (int j = 0; j < prevSize; ++j)
        {
            sum += prevLayer[j]*(int8_t)ws[i*prevSize+j];
        }
        nextLayer[i] = clip(sum/64);
    }
}

int32_t output(const int32_t* prevLayer, const int prevSize, const weight_t* ws, int32_t out)
{
    for (int i = 0; i < prevSize; ++i)
        out += ws[i] * prevLayer[i];

    return out;
}

static int32_t nInput[512];
static int32_t hiddenLayer1[32];
static int32_t hiddenLayer2[32];

int evaluateNNUE(const NNUE* nn, const Board b)
{
    inputLayer(nn, b, b.stm, nInput);
    inputLayer(nn, b, 1^b.stm, nInput + kHalfDimensionFT);

    propagate(nInput, 2*kHalfDimensionFT, hiddenLayer1, 32, nn->weights1, nn->biases1);
    propagate(hiddenLayer1, 32, hiddenLayer2, 32, nn->weights2, nn->biases2);

    int32_t out = output(hiddenLayer2, 32, nn->outputW, *(nn->outputB));

    return out / FV_SCALE;
}

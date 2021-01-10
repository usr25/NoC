#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>

#include "../include/global.h"
#include "../include/nnue.h"

#define CHECK_READ(read,correct) if (read != correct) {fprintf(stderr, "Unsuccessful read in %s %d\n", __FILE__, __LINE__); \
                                exit(5);}

//#define TEST_NNUE_PARSING

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

    #ifdef DEBUG
        printf("Loading NNUE %s\n", path);
    #endif

    nn.ftBiases = (uint16_t*)malloc(sizeof(uint16_t)*kHalfDimensionFT);
    nn.ftWeights = (uint16_t*)malloc(sizeof(uint16_t)*kHalfDimensionFT*kInputDimensionsFT);
    CHECK_MALLOC(nn.ftBiases);
    CHECK_MALLOC(nn.ftWeights);

    readHeaders(f, &nn);
    readParams(f, &nn);

    fclose(f);

    #ifdef TEST_NNUE_PARSING
    showNNUE(&nn);
    #endif

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

    #ifdef DEBUG
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
    #ifdef DEBUG
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
    #ifdef DEBUG
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

    int j;
    int i = 0;
    while (fread(&j, sizeof(int), 1, f))
        i++;

    if (i)
    {
        fprintf(stderr, "NNUE file hasn't been read completely, %d ints remeain\n", i);
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
            int idx = isOutput? i*dims+j : getIdx(i,j,dims);
            ws[idx] = (weight_t)a;
        }
    }
}

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

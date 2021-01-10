//It is different in sparse/regular
#define weight_t int8_t

typedef struct
{
    int i;

    int16_t* ftBiases;
    int16_t* ftWeights;

    weight_t weights1[32*512];
    weight_t weights2[32*32];
    weight_t outputW[1*32];

    int32_t biases1[32];
    int32_t biases2[32];
    int32_t outputB[1];
} NNUE;

static const unsigned int NNUEVersion = 0x7AF32F16u;

NNUE loadNNUE(const char* path);
void freeNNUE(NNUE* nn);
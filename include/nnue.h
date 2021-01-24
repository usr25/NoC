//It is different in sparse/regular
#define weight_t int8_t
#define clipped_t int8_t

typedef struct
{
    int16_t* ftBiases;
    int16_t* ftWeights;

    weight_t weights1[32*512];
    weight_t weights2[32*32];
    weight_t outputW[1*32];

    int32_t biases1[32];
    int32_t biases2[32];
    int32_t outputB[1];
} NNUE;

typedef struct
{
    int piece;
    int color;
    int sqr;
    int appears;
} NNUEChange;

typedef struct
{
    NNUEChange changes[4];
    int idx;
} NNUEChangeList;

enum
{
    kHalfDimensionFT = 256,
    kDimensionFT = 512,
    kDimensionHidden = 32,
    kInputDimensionsFT = 41024
};

enum {
    FV_SCALE = 16,
    SHIFT = 6,
};

static const int dimensions[5] = {41024, 512, 32, 32, 1};
static const unsigned int NNUEVersion = 0x7AF32F16u;

void initNNUE(const char* path);
NNUE loadNNUE(const char* path);
void freeNNUE(NNUE* nn);
void inputLayer(const NNUE* nn, const Board* const b, const int color, int16_t* inp);
void determineChanges(const Move m, NNUEChangeList* list, const int color);
int evaluateNNUE(const Board* b, const int useAcc);

void initNNUEAcc(const Board* b);
void updateDo(NNUEChangeList* q, const Move m, const Board* const b);
void updateUndo(NNUEChangeList* q, const Board* const b);
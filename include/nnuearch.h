const int getIdx(const int i, const int j, const int dim);
int evaluateAcc(const NNUE* nn, const Board* const b, const int16_t* nInput);
int evaluate(const NNUE* nn, const Board* const b, int16_t* nInput);

inline static const clipped_t clip64(const int32_t v)
{
    if (v <= 0) return 0;
    if (v >= 127 << SHIFT) return 127;
    return v >> SHIFT;
}

inline static const clipped_t clip(const int16_t v)
{
    return v >= 127? 127 : (v <= 0? 0 : v);
}
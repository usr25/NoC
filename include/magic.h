void genMagics(void);
void initMagics(void);

//This arent optimal magics space wise, older hardware with smaller caches may suffer
uint64_t bishMagicMoves[64] [512];
uint64_t rookMagicMoves[64][4096];

uint64_t bishMagic[64];
uint64_t rookMagic[64];

__attribute__((hot)) static inline uint64_t getRookMagicMoves(const int index, const uint64_t allPieces)
{
    //allPieces &= getStraInt(index);
    //allPieces *= rookMagic[index];
    //64 - 12 == 52, worst case scenario. To make it variable use POPCOUNT(mask)
    return rookMagicMoves[index][((allPieces & getStraInt(index)) * rookMagic[index]) >> 52];
}
__attribute__((hot)) static inline uint64_t getBishMagicMoves(const int index, const uint64_t allPieces)
{
    //allPieces &= getDiagInt(index);
    //allPieces *= bishMagic[index];
    //64 - 9 == 55, worst case scenario
    return bishMagicMoves[index][((allPieces & getDiagInt(index)) * bishMagic[index]) >> 55];
}
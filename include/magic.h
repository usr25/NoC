void genMagics(void);
void initMagics(void);

//This arent optimal magics space wise, older hardware with smaller caches may suffer
uint64_t bishMagicMoves[64] [512];
uint64_t rookMagicMoves[64][4096];

uint64_t bishMagic[64];
uint64_t rookMagic[64];

/* How it works:
 * 1- Get all the relevant bits for the attack using a mask getMoveTypeInt(sqr) & allPieces, it is not neccessary to include the bits of the blocking pieces in the mask
 * 2- Multiply * magicType[sqr]
 * 3- Bitshift the result 64 - 12 (55) for the rook and 64 - 9 (52) for the bishop
 * 4- Refer to the respective array and return getTypeMoves[sqr][multShifted]
 * The return WILL include the blocking sqrs, so that the mask b.color[opp] can be applied
 * to include the opp pieces for captures
 */

static inline uint64_t getRookMagicMoves(const int sqr, const uint64_t allPieces)
{
    //64 - 12 == 52, worst case scenario. To make it variable use POPCOUNT(mask)
    return rookMagicMoves[sqr][((allPieces & getStraInt(sqr)) * rookMagic[sqr]) >> 52];
}
static inline uint64_t getBishMagicMoves(const int sqr, const uint64_t allPieces)
{
    //64 - 9 == 55, worst case scenario
    return bishMagicMoves[sqr][((allPieces & getDiagInt(sqr)) * bishMagic[sqr]) >> 55];
}
static inline uint64_t getQueenMagicMoves(const int sqr, const uint64_t allPieces)
{
    //64 - 9 == 55, worst case scenario
    return getBishMagicMoves(sqr, allPieces) | getRookMagicMoves(sqr, allPieces);
}
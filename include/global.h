#define ALL 0xffffffffffffffffULL

#define BOOL(x) ((x)?"true":"false")
#define REMOVE_LSB(bb) bb &= bb - 1
#define POPCOUNT(ll) __builtin_popcountll(ll)
#define LSB_INDEX(ll) __builtin_ctzll(ll)
#define MSB_INDEX(ll) 63 - __builtin_clzll(ll)

#define NO_PIECE -1
#define KING 0
#define QUEEN 1
#define ROOK 2
#define BISH 3
#define KNIGHT 4
#define PAWN 5

//1 ^ color to change the color
//2 | color to change to AV
//3 - color to change to opposite AV
#define BLACK 0
#define WHITE 1
#define AV_BLACK 2
#define AV_WHITE 3

#define BCASTLEK 2
#define BCASTLEQ 4
#define WCASTLEK 8
#define WCASTLEQ 16

#define C_MASK_WK 0x6ULL
#define C_MASK_WQ 0x70ULL
#define C_MASK_BK 0x600000000000000ULL
#define C_MASK_BQ 0x7000000000000000ULL

#include <inttypes.h>

typedef struct 
{
    int numMoves;
    int check;
}Check;

uint64_t POW2[64];
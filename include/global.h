#define ALL 0xffffffffffffffff

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

#define BLACK 0
#define WHITE 1
#define AV_BLACK 2
#define AV_WHITE 3

#define WCASTLEK 16
#define WCASTLEQ 8
#define BCASTLEK 4
#define BCASTLEQ 2

#include <inttypes.h>

uint64_t POW2[64];
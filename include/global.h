#define ALL 0xffffffffffffffff

#define BOOL(x) ((x)?"true":"false")
#define REMOVE_LSB(bb) bb &= bb - 1
#define POPCOUNT(ll) __builtin_popcountll((ll))
#define LSB_INDEX(ll) __builtin_ctzll((ll))
#define MSB_INDEX(ll) 63 - __builtin_clzll((ll))

//The LSB is used to indicate the color
#define KING 2
#define QUEEN 4
#define ROOK 8
#define BISH 16
#define KNIGHT 32
#define PAWN 64

#define WCASTLEK 16
#define WCASTLEQ 8
#define BCASTLEK 4
#define BCASTLEQ 2

#include <inttypes.h>

uint64_t POW2[64];
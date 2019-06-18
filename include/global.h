#define ALL 0xffffffffffffffff
#define BOOL(x) ((x)?"true":"false")
#define POPCOUNT(ll) __builtin_popcountll((ll))
#define LSB_INDEX(ll) __builtin_ctzll((ll))
#define MSB_INDEX(ll) 63 - __builtin_clzll((ll))

#include <inttypes.h>
uint64_t POW2[64];
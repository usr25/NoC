#define ALL 0xffffffffffffffffULL //(~0ULL)

#define ENGINE_AUTHOR "usr"
#define ENGINE_NAME "NoC"

#define USE_TB

#define REMOVE_LSB(bb) bb &= bb - 1
#define POPCOUNT(ll) __builtin_popcountll(ll)
#define LSB_INDEX(ll) __builtin_ctzll(ll)
#define MSB_INDEX(ll) (63 - __builtin_clzll(ll))

#define NO_PIECE -1
#define KING    0
#define QUEEN   1
#define ROOK    2
#define BISH    3
#define KNIGHT  4
#define PAWN    5

//1 ^ color to change the color
//2 | color to change to AV
//3 - color to change to opposite AV
#define BLACK 0
#define WHITE 1
#define AV_BLACK 2
#define AV_WHITE 3

#define BCASTLEK 0b1
#define BCASTLEQ 0b10
#define WCASTLEK 0b100
#define WCASTLEQ 0b1000

#define C_MASK_WK 0x6ULL
#define C_MASK_WQ 0x70ULL
#define C_MASK_BK 0x600000000000000ULL
#define C_MASK_BQ 0x7000000000000000ULL

#define NMOVES 200

//Relevant positions on the board
#define EVEN_TILES  0x55aa55aa55aa55aaULL
#define ODD_TILES   0xaa55aa55aa55aa55ULL
#define EDGE        0xff818181818181ffULL
#define EDGE_RANKS  0xff000000000000ffULL

#define VQUEEN 950
#define VROOK 525
#define VBISH 335
#define VKNIGHT 320
#define VPAWN 100

#define PLUS_MATE    99999
#define MINS_MATE   -99999
#define PLUS_INF   9999999
#define MINS_INF  -9999999

#ifndef uint64_t
#define uint64_t unsigned long long
#endif

inline int min(const int a, const int b) {return (a < b)? a : b;}
inline int max(const int a, const int b) {return (a > b)? a : b;}

uint64_t POW2[64];
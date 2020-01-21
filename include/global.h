#define ALL 0xffffffffffffffffULL //(~0ULL)

#define ENGINE_AUTHOR "Jorge"
#define ENGINE_NAME "NoC 1.20"

//#define USE_TB
//#define TRAIN

#define REMOVE_LSB(bb) bb &= bb - 1
#define POPCOUNT(ll) __builtin_popcountll(ll)
#define LSB_INDEX(ll) __builtin_ctzll(ll)
#define MSB_INDEX(ll) (63 ^ __builtin_clzll(ll))

#define WHITE_PAWN_ATT(bb) (((bb) << 9) & 0xfefefefefefefefe) | (((bb) << 7) & 0x7f7f7f7f7f7f7f7f)
#define BLACK_PAWN_ATT(bb) (((bb) >> 9) & 0x7f7f7f7f7f7f7f7f) | (((bb) >> 7) & 0xfefefefefefefefe)

#define BASE_64(a, b) (((a) << 6) + (b))
#define RANGE_64(a) ((a) >= 0 && (a) < 64)

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

#define BCASTLEK    0b1
#define BCASTLEQ   0b10
#define WCASTLEK  0b100
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

#define PLUS_MATE   1159998
#define MINS_MATE  -1159998
#define PLUS_INF   99999999
#define MINS_INF  -99999999

#define MAX_PLY 50

#ifndef uint64_t
#define uint64_t unsigned long long
#endif

const inline int min(const int a, const int b) {return (a < b)? a : b;}
const inline int max(const int a, const int b) {return (a > b)? a : b;}

uint64_t POW2[64];
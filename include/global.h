#define ALL 0xffffffffffffffffULL //(~0ULL)

#define REMOVE_LSB(bb) bb &= bb - 1
#define POPCOUNT(ll) __builtin_popcountll(ll)
#define LSB_INDEX(ll) __builtin_ctzll(ll)
#define MSB_INDEX(ll) 63 - __builtin_clzll(ll)

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

#define BCASTLEK 0b10
#define BCASTLEQ 0b100
#define WCASTLEK 0b1000
#define WCASTLEQ 0b10000

#define C_MASK_WK 0x6ULL
#define C_MASK_WQ 0x70ULL
#define C_MASK_BK 0x600000000000000ULL
#define C_MASK_BQ 0x7000000000000000ULL

//Relevant positions on the board
#define EVEN_TILES  0x55aa55aa55aa55aaULL
#define ODD_TILES   0xaa55aa55aa55aa55ULL
#define EDGE        0xff818181818181ffULL
#define EDGE_RANKS  0xff000000000000ffULL

#define uint64_t unsigned long long

uint64_t POW2[64];
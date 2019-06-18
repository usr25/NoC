#define ALL 0xffffffffffffffff

#define WCASTLEK 16
#define WCASTLEQ 8
#define BCASTLEK 4
#define BCASTLEQ 2

#define WHITETOPLAY 1

//The LSB is used to indicate the color
#define KING 2
#define QUEEN 4
#define ROOK 8
#define BISH 16
#define KNIGHT 32
#define PAWN 64

#define SH_WQUEEN 0
#define SH_WROOK 4
#define SH_WBISH 8
#define SH_WKNIGHT 12
#define SH_WPAWNS 16
#define SH_BQUEEN 20
#define SH_BROOK 24
#define SH_BBISH 28
#define SH_BKNIGHT 32
#define SH_BPAWN 36

#define NUM_WQUEEN  0xf
#define NUM_WROOK   0xf0
#define NUM_WBISH   0xf00
#define NUM_WKNIGHT 0xf000
#define NUM_WPAWNS  0xf0000
#define NUM_BQUEEN  0xf00000
#define NUM_BROOK   0xf000000
#define NUM_BBISH   0xf0000000
#define NUM_BKNIGHT 0xf00000000
#define NUM_BPAWN   0xf000000000

typedef struct
{
    unsigned long long white;
    unsigned long long avWhite;

    unsigned long long wPawns;
    unsigned long long wKing;
    unsigned long long wQueen;
    unsigned long long wBish;
    unsigned long long wRook;
    unsigned long long wKnight;

    
    unsigned long long black;
    unsigned long long avBlack;

    unsigned long long bPawns;
    unsigned long long bKing;
    unsigned long long bQueen;
    unsigned long long bBish;
    unsigned long long bRook;
    unsigned long long bKnight;

    unsigned long long pieces;
    unsigned long long numPieces;

    int posInfo;
} Board;

static inline int numWQueen(Board* b)   {return (b->numPieces & NUM_WQUEEN) >> SH_WQUEEN;}
static inline int numWRook(Board* b)    {return (b->numPieces & NUM_WROOK) >> SH_WROOK;}
static inline int numWBish(Board* b)    {return (b->numPieces & NUM_WBISH) >> SH_WBISH;}
static inline int numWKnight(Board* b)  {return (b->numPieces & NUM_WKNIGHT) >> SH_WKNIGHT;}
static inline int numWPawns(Board* b)   {return (b->numPieces & NUM_WPAWNS) >> SH_WPAWNS;}

static inline int numBQueen(Board* b)   {return (b->numPieces & NUM_BQUEEN) >> SH_BQUEEN;}
static inline int numBRook(Board* b)    {return (b->numPieces & NUM_BROOK) >> SH_BROOK;}
static inline int numBBish(Board* b)    {return (b->numPieces & NUM_BBISH) >> SH_BBISH;}
static inline int numBKnight(Board* b)  {return (b->numPieces & NUM_BKNIGHT) >> SH_BKNIGHT;}
static inline int numBPawns(Board* b)   {return (b->numPieces & NUM_BPAWN) >> SH_BPAWN;}


static inline void incrWQueen(Board* b)     {b->numPieces += 1ULL << SH_WQUEEN;}
static inline void incrWRook(Board* b)      {b->numPieces += 1ULL << SH_WROOK;}
static inline void incrWBish(Board* b)      {b->numPieces += 1ULL << SH_WBISH;}
static inline void incrWKnight(Board* b)    {b->numPieces += 1ULL << SH_WKNIGHT;}
static inline void incrWPawns(Board* b)     {b->numPieces += 1ULL << SH_WPAWNS;}

static inline void incrBQueen(Board* b)     {b->numPieces += 1ULL << SH_BQUEEN;}
static inline void incrBRook(Board* b)      {b->numPieces += 1ULL << SH_BROOK;}
static inline void incrBBish(Board* b)      {b->numPieces += 1ULL << SH_BBISH;}
static inline void incrBKnight(Board* b)    {b->numPieces += 1ULL << SH_BKNIGHT;}
static inline void incrBPawns(Board* b)     {b->numPieces += 1ULL << SH_BPAWN;}


Board generateFromFen(char* const fen, char* const toPlay, char* const castle);
Board defaultBoard();

unsigned int index(unsigned long long bitboard, int prev);

unsigned int pieceAt(Board* const b, const unsigned int coord);
unsigned int whitePieceAt(Board* const b, const unsigned int coord);
unsigned int blackPieceAt(Board* const b, const unsigned int coord);
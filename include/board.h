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


typedef struct
{
    uint64_t white;
    uint64_t avWhite;

    uint64_t wPawns;
    uint64_t wKing;
    uint64_t wQueen;
    uint64_t wBish;
    uint64_t wRook;
    uint64_t wKnight;

    
    uint64_t black;
    uint64_t avBlack;

    uint64_t bPawns;
    uint64_t bKing;
    uint64_t bQueen;
    uint64_t bBish;
    uint64_t bRook;
    uint64_t bKnight;

    uint64_t pieces;
    uint64_t numPieces;

    int posInfo;
} Board;

static inline int numWKing(uint64_t pieces)   {return (pieces >> SH_WQUEEN) & 0xf;}
static inline int numWQueen(uint64_t pieces)   {return (pieces >> SH_WQUEEN) & 0xf;}
static inline int numWRook(uint64_t pieces)    {return (pieces >> SH_WROOK) & 0xf;}
static inline int numWBish(uint64_t pieces)    {return (pieces >> SH_WBISH) & 0xf;}
static inline int numWKnight(uint64_t pieces)  {return (pieces >> SH_WKNIGHT) & 0xf;}
static inline int numWPawn(uint64_t pieces)   {return (pieces >> SH_WPAWNS) & 0xf;}

static inline int numBKing(uint64_t pieces)   {return (pieces >> SH_BQUEEN) & 0xf;}
static inline int numBQueen(uint64_t pieces)   {return (pieces >> SH_BQUEEN) & 0xf;}
static inline int numBRook(uint64_t pieces)    {return (pieces >> SH_BROOK) & 0xf;}
static inline int numBBish(uint64_t pieces)    {return (pieces >> SH_BBISH) & 0xf;}
static inline int numBKnight(uint64_t pieces)  {return (pieces >> SH_BKNIGHT) & 0xf;}
static inline int numBPawn(uint64_t pieces)   {return (pieces >> SH_BPAWN) & 0xf;}


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

unsigned int index(uint64_t bitboard, int prev);

unsigned int pieceAt(Board* const b, const unsigned int coord);
unsigned int whitePieceAt(Board* const b, const unsigned int coord);
unsigned int blackPieceAt(Board* const b, const unsigned int coord);
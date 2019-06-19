#define WHITETOPLAY 1

#define SH_WQUEEN 0
#define SH_WROOK 4
#define SH_WBISH 8
#define SH_WKNIGHT 12
#define SH_WPAWN 16
#define SH_BQUEEN 20
#define SH_BROOK 24
#define SH_BBISH 28
#define SH_BKNIGHT 32
#define SH_BPAWN 36


typedef struct
{
    uint64_t white;
    uint64_t avWhite;

    uint64_t wPawn;
    uint64_t wKing;
    uint64_t wQueen;
    uint64_t wBish;
    uint64_t wRook;
    uint64_t wKnight;

    
    uint64_t black;
    uint64_t avBlack;

    uint64_t bPawn;
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
static inline int numWPawn(uint64_t pieces)   {return (pieces >> SH_WPAWN) & 0xf;}

static inline int numBKing(uint64_t pieces)   {return (pieces >> SH_BQUEEN) & 0xf;}
static inline int numBQueen(uint64_t pieces)   {return (pieces >> SH_BQUEEN) & 0xf;}
static inline int numBRook(uint64_t pieces)    {return (pieces >> SH_BROOK) & 0xf;}
static inline int numBBish(uint64_t pieces)    {return (pieces >> SH_BBISH) & 0xf;}
static inline int numBKnight(uint64_t pieces)  {return (pieces >> SH_BKNIGHT) & 0xf;}
static inline int numBPawn(uint64_t pieces)   {return (pieces >> SH_BPAWN) & 0xf;}


static inline void incrWQueen(Board* b)     {b->numPieces += POW2[SH_WQUEEN];}
static inline void incrWRook(Board* b)      {b->numPieces += POW2[SH_WROOK];}
static inline void incrWBish(Board* b)      {b->numPieces += POW2[SH_WBISH];}
static inline void incrWKnight(Board* b)    {b->numPieces += POW2[SH_WKNIGHT];}
static inline void incrWPawn(Board* b)     {b->numPieces += POW2[SH_WPAWN];}

static inline void incrBQueen(Board* b)     {b->numPieces += POW2[SH_BQUEEN];}
static inline void incrBRook(Board* b)      {b->numPieces += POW2[SH_BROOK];}
static inline void incrBBish(Board* b)      {b->numPieces += POW2[SH_BBISH];}
static inline void incrBKnight(Board* b)    {b->numPieces += POW2[SH_BKNIGHT];}
static inline void incrBPawn(Board* b)     {b->numPieces += POW2[SH_BPAWN];}

static inline void decrWQueen(Board* b)     {b->numPieces -= POW2[SH_WQUEEN];}
static inline void decrWRook(Board* b)      {b->numPieces -= POW2[SH_WROOK];}
static inline void decrWBish(Board* b)      {b->numPieces -= POW2[SH_WBISH];}
static inline void decrWKnight(Board* b)    {b->numPieces -= POW2[SH_WKNIGHT];}
static inline void decrWPawn(Board* b)     {b->numPieces -= POW2[SH_WPAWN];}

static inline void decrBQueen(Board* b)     {b->numPieces -= POW2[SH_BQUEEN];}
static inline void decrBRook(Board* b)      {b->numPieces -= POW2[SH_BROOK];}
static inline void decrBBish(Board* b)      {b->numPieces -= POW2[SH_BBISH];}
static inline void decrBKnight(Board* b)    {b->numPieces -= POW2[SH_BKNIGHT];}
static inline void decrBPawn(Board* b)     {b->numPieces -= POW2[SH_BPAWN];}


Board generateFromFen(char* const fen, char* const toPlay, char* const castle);
Board defaultBoard();

unsigned int index(uint64_t bitboard, int prev);

unsigned int pieceAt(Board* const b, const unsigned int coord);
unsigned int whitePieceAt(Board* const b, const uint64_t coord);
unsigned int blackPieceAt(Board* const b, const uint64_t coord);
unsigned int captureBlackPiece(Board* b, const uint64_t pos);
unsigned int captureWhitePiece(Board* b, const uint64_t pos);

int equal(Board* a, Board* b);
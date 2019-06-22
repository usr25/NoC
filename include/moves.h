typedef struct 
{
    unsigned int pieceThatMoves;
    unsigned int pieceCaptured;
    int from;
    int to;
    int color;
    int castle;
}Move;

typedef struct
{
    uint64_t allPieces;
    int posInfo;
}History;

uint64_t posKingMoves(Board* b, const int color);
uint64_t posKnightMoves(Board* b, const int color, const int lsb);
uint64_t posPawnMoves(Board* b, const int color, const int lsb);
uint64_t posRookMoves(Board* b, const int color, const int lsb);
uint64_t posBishMoves(Board* b, const int color, const int lsb);
uint64_t posQueenMoves(Board* b, const int color, const int lsb);

int canCastle(Board* b, const int color);
Move castleKSide(const int color);
Move castleQSide(const int color);

int isInCheck(Board* b, const int kingsColor);
int checkInPosition(Board* b, const int lsb, const int kingsColor);
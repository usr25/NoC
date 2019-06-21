
typedef struct 
{
    unsigned int pieceThatMoves;
    unsigned int pieceCaptured;
    int from;
    int to;
    int color;
}Move;

typedef struct
{
    int castle;
    int enPass;
}History;

uint64_t posKingMoves(Board* b, const int color);
uint64_t posKnightMoves(Board* b, const int color, const int lsb);
uint64_t posPawnMoves(Board* b, const int color, const int lsb);
uint64_t posRookMoves(Board* b, const int color, const int lsb);
uint64_t posBishMoves(Board* b, const int color, const int lsb);
uint64_t posQueenMoves(Board* b, const int color, const int lsb);

int isInCheck(Board* b, const int kingsColor);
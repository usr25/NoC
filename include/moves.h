
typedef struct 
{
    unsigned int pieceThatMoves;
    unsigned int pieceCaptured;
    int from;
    int to;
    int color;
}Move;

uint64_t posWhiteKingMoves(Board* b);
uint64_t posBlackKingMoves(Board* b);
uint64_t posKingMoves(Board* b, const int color);
uint64_t posKnightMoves(Board* b, const int color, int index);
uint64_t posPawnMoves(Board* b, const int color, int index);
uint64_t posRookMoves(Board* b, const int color, int index);
uint64_t posBishMoves(Board* b, const int color, int index);
uint64_t posQueenMoves(Board* b, const int color, int index);

int isInCheck(Board* b, const int kingsColor);
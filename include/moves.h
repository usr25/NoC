uint64_t posKingMoves(Board* b, int color);
uint64_t posKnightMoves(Board* b, int color, int index);
uint64_t posPawnMoves(Board* b, int color, int index);
uint64_t posRookMoves(Board* b, int color, int index);
uint64_t posBishMoves(Board* b, int color, int index);
uint64_t posQueenMoves(Board* b, int color, int index);

int isInCheck(Board* b, int kingsColor);
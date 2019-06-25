int allMoves(Board* b, Move* list, const int color);
int legalMoves(Board* b, Move* list, const int color);
int moveIsValid(Board* b, Move move, History h);
uint64_t pinnedPieces(Board* b, const int color);
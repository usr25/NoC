int legalMoves(Board* b, Move* list);
int legalMovesQuiesce(Board* b, Move* list);
uint64_t pinnedPieces(const Board* b, const int color);
int movesCheck(const Board* b, Move* list, const int color, const uint64_t forbidden, const uint64_t pinned);
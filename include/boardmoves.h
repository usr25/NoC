void makeMove(Board* b, Move* move, const int color);
void undoMove(Board* b, Move* move, const int color);
int allMoves(Board* b, Move* list, uint64_t prevMovEnPass, const int color);
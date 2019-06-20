void makeMoveWhite(Board* b, Move* move, int* history);
void makeMoveBlack(Board* b, Move* move, int* history);
void undoMoveWhite(Board* b, Move* move, int* history);
void undoMoveBlack(Board* b, Move* move, int* history);
int allMovesWhite(Board* b, Move* list, uint64_t prevMovEnPass);
int allMovesBlack(Board* b, Move* list, uint64_t prevMovEnPass);
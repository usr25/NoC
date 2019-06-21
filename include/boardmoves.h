int pieceAt(Board* const b, const uint64_t coord, const int color);
int capturePiece(Board* b, const uint64_t pos, const int colorToCapture);
void makeMove(Board* b, Move* move);
void undoMove(Board* b, Move move);
int allMoves(Board* b, Move* list, uint64_t prevMovEnPass, const int color);
int pieceAt(Board* const b, const uint64_t coord, const int color);
int capturePiece(Board* b, const uint64_t pos, const int colorToCapture);
void makeMove(Board* b, Move move, History* h);
void undoMove(Board* b, Move move, History* h);
int allMoves(Board* b, Move* list, const int color);
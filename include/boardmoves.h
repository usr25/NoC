int pieceAt(Board* const b, const uint64_t coord, const int color);
int capturePiece(Board* b, const uint64_t pos, const int colorToCapture);
void makeMove(Board* b, const Move move, History* h);
void undoMove(Board* b, const Move move, History* h);
void makePermaMove(Board* b, const Move move);
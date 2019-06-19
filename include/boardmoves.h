unsigned int makeMoveWhite(Board* b, const unsigned int piece, const int from, const int to);
unsigned int makeMoveBlack(Board* b, const unsigned int piece, const int from, const int to);
void undoMoveWhite(Board* b, const int pieceMoved, const int pieceCaptured, const int from, const int to);
void undoMoveBlack(Board* b, const int pieceMoved, const int pieceCaptured, const int from, const int to);
void drawPosition(const Board b, const int drawCoords);
void drawBitboard(uint64_t b);
void drawMove(const Move m);
void moveToText(const Move m, char* mv);
void generateFen(const Board b, char* c);

char pieces[6];
void loop(void);
Board gen_(char* beg, Repetition* rep);
Move evalPos(char* beg);
void infoString(const Move m, const int depth, const uint64_t nodes);
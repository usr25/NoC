void initSort(void);
void initKM(void);
void initHistory(void);
void addKM(const Move m, const int depth);
void addHistory(const int from, const int to, const int n, const int stm);
void decHistory(const int from, const int to, const int n, const int stm);
__attribute__((hot)) void assignScores(Board* b, Move* list, const int numMoves, const Move bestFromPos, const int depth);
__attribute__((hot)) void assignScoresQuiesce(Board* b, Move* list, const int numMoves);
int compMoves(const Move* m1, const Move* m2);
void sort(Move* start, Move* end);
void moveToFst(Move* list, int idx);

extern int history[2][4096];
void initSort(void);
void initKM(void);
void addKM(const Move m, const int depth);
__attribute__((hot)) void assignScores(Board* b, Move* list, const int numMoves, const Move bestFromPos, const int depth);
__attribute__((hot)) void assignScoresQuiesce(Board* b, Move* list, const int numMoves);
int compMoves(const Move* m1, const Move* m2);
void sort(Move* start, Move* end);
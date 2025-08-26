typedef struct
{
    int depth;
    clock_t timeToMove;
    clock_t extraTime;
    clock_t maxTime;

} SearchParams;

typedef struct
{
    Move lastMove;
    int lastScore;
    int consecutiveMove;
    int consecutiveScore;
} SearchData;

Move bestTime(Board b, Repetition rep, SearchParams sp);
__attribute__((hot)) int qsearch(Board b, int alpha, const int beta, const int d);
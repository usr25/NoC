typedef struct
{
    int depth;
    clock_t timeToMove;
    clock_t extraTime;

} SearchParams;


Move bestTime(Board b, Repetition rep, SearchParams sp);
__attribute__((hot)) int qsearch(Board b, int alpha, const int beta, const int d);
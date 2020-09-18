enum MGState
{
    Uninitialized,
    BestMove,
    Tactical,
    Quiet,
    Check,
    Exhausted
};

typedef struct
{
    Move moves[NMOVES];
    enum MGState state;

    uint64_t forbidden;
    uint64_t pinned;
    int bestMoveIdx;
    int nmoves;
    int currmove;
    int tot;
    int qsearch;
} MoveGen;

MoveGen newMG(const Board* b, const int qsearch, const Move bestM);
Move next(MoveGen* mg, const Board* b);
int collect(Move* list, const Board* b);
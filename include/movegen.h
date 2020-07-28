enum MGState
{
    Uninitialized,
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
    int nmoves;
    int currmove;
    int tot;
} MoveGen;

MoveGen newMG(const Board* b);
Move next(MoveGen* mg, const Board* b);
int collect(Move* list, const Board* b);
#define WHITETOPLAY 1

typedef struct
{
    uint64_t piece[2][6];
    
    uint64_t color[4];

    uint64_t allPieces;

    int turn;
    int posInfo;
    int enPass; //0 by default, otherwise it is the index of the pawn that just moved
} Board;


Board generateFromFenEP(char* const fen, char* const toPlay, char* const castle, char* const enPass);
Board generateFromFen(char* const fen, char* const toPlay, char* const castle);
Board defaultBoard();

int equal(Board* a, Board* b);
Board duplicate(Board b);

int getIndex(char row, char col);
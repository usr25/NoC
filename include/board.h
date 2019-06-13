#define LEN 8
#define AREA 64

/*
Should offer better performance, but I want to start simple
#define LEN 16

#define WHITE   0
#define BLACK   1
#define WPAWN   2
#define WKING   3
#define WQUEEN  4
#define WROOK   5
#define WBISH   6
#define WKNIGHT 7
#define BPAWN   8
#define BKING   9
#define BQUEEN  10
#define BROOK   11
#define BBISH   12
#define BKNIGHT 13
#define AVWHITE 14
#define AVBLACK 15

unsigned long long board[NUMPIECES];
*/

#define ALL 0xffffffffffffffff
#define SIDE 8

typedef struct
{
    unsigned long long white;
    unsigned long long avWhite;

    unsigned long long wPawns;
    unsigned long long wKing;
    unsigned long long wQueen;
    unsigned long long wBish;
    unsigned long long wRook;
    unsigned long long wKnight;

    
    unsigned long long black;
    unsigned long long avBlack;

    unsigned long long bPawns;
    unsigned long long bKing;
    unsigned long long bQueen;
    unsigned long long bBish;
    unsigned long long bRook;
    unsigned long long bKnight;
} Board;

Board generateFromFen(char const* fen);
Board defaultBoard();
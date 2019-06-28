typedef struct 
{
    int pieceThatMoves;
    int from;
    int to;
    int castle;
    int promotion;
    int enPass;
}Move;

typedef struct
{
    int pieceCaptured;

    //Data regarding the Board struct, to make it easier to save
    uint64_t allPieces;
    int posInfo;
    int enPass;
}History;

typedef struct
{
    uint64_t tiles;
    int num;
}AttacksOnK;

uint64_t posKingMoves(Board* b, const int color);
uint64_t posKnightMoves(Board* b, const int color, const int lsb);
uint64_t posPawnMoves(Board* b, const int color, const int lsb);
uint64_t posRookMoves(Board* b, const int color, const int lsb);
uint64_t posBishMoves(Board* b, const int color, const int lsb);
uint64_t posQueenMoves(Board* b, const int color, const int lsb);

int canCastle(Board* b, const int color, const uint64_t forbidden);
int canCastleCheck(Board* b, const int color); //Slower version if forbidden hasnt been calculated
Move castleKSide(const int color);
Move castleQSide(const int color);

uint64_t controlledKingPawnKnight(Board* b, const int inverse);
uint64_t allSlidingAttacks(Board* b, const int color, const uint64_t obstacles);
AttacksOnK getCheckTiles(Board* b, const int color);
int isInCheck(Board* b, const int kingsColor);
int checkInPosition(Board* b, const int lsb, const int kingsColor);
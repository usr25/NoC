/* Struct that holds all the information necessary to undo moves
 * piece -> Int representing the piece that moves
 * from -> Index from where the piece moves
 * to -> Index where the piece moves
 * capture -> Int representing the piece captured (0/-1 is no piece)
 * castle -> Int representing if it is a castle, only when KING moves (1 -> Kingside, 2 -> Queenside)
 * promotion -> Int representing the piece to which a PAWN promotes
 * enPass -> Index of where the opp pawn is located when capturing En Passand
 * score -> Score used for the MVV - LVA ordering and other orderings
 */

typedef struct 
{
    int piece;
    int from;
    int to;
    int capture;

    int castle;
    int promotion;
    int enPass;

    int score;
}Move;

/* Struct to make it easier to undo moves
 * allPieces -> bb representing all the pieces in the board
 * castleInfo -> Castle info
 * enPass -> A pawn moved 2 squares and thus allowed En Passand
 */

typedef struct
{
    uint64_t allPieces;
    int castleInfo;
    int enPass;
    int fifty;
}History;

/* Struct that holds the information about the number of attackers
 * tiles -> bb Where all the 1s are tiles where a piece placed will stop a check
 * num -> number of attackers
 */
typedef const struct
{
    uint64_t tiles;
    int num;
}AttacksOnK;

uint64_t posKingMoves(Board* b, const int color);
uint64_t posRookMoves(Board* b, const int color, const int lsb);
uint64_t posBishMoves(Board* b, const int color, const int lsb);
uint64_t posQueenMoves(Board* b, const int color, const int lsb);
uint64_t posKnightMoves(Board* b, const int color, const int lsb);
uint64_t posPawnMoves(Board* b, const int color, const int lsb);

uint64_t diagonal(const int lsb, const uint64_t allPieces);
uint64_t straight(const int lsb, const uint64_t allPieces);

int canCastle(Board* b, const int color, const uint64_t forbidden);
Move castleKSide(const int color);
Move castleQSide(const int color);

uint64_t controlledKingPawnKnight(Board* b, const int inverse);
uint64_t allSlidingAttacks(Board* b, const int color, const uint64_t obstacles);
AttacksOnK getCheckTiles(Board* b, const int color);

int isInCheck(Board* b, const int kingsColor);
int slidingCheck(Board* b, const int kingsColor);
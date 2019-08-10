/* Holds all the information about the board
 * piece -> Array holding the bb of each type of piece indexed by [color][piece]
 * color -> Array holding the bb of sqrs occupied by a color and the available sqrs
 * allPieces -> bb with all the pieces 
 * turn -> Int holding the turn of the player that is to move
 * castleInfo -> Int that holds the availability of the 4 diff castles
 * enPass -> Index of the pawn that moved 2 sqrs in the last turn, otherwise 0
 */

typedef struct
{
    uint64_t piece[2][6];
    uint64_t color[4];

    uint64_t allPieces;

    int turn;
    int castleInfo;
    int enPass;
    int fifty;
} Board;

int textToPiece(char piece);

Board genFromFen(char* const fen, int* counter);
Board defaultBoard(void);

int equal(Board* a, Board* b);
Board duplicate(const Board b);

int getIndex(const char row, const char col);
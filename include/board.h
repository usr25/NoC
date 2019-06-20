#define WHITETOPLAY 1

typedef struct
{
    uint64_t piece[2][12];
    
    uint64_t color[4];

    uint64_t allPieces;

    int posInfo;
} Board;



Board generateFromFen(char* const fen, char* const toPlay, char* const castle);
Board defaultBoard();

int index(uint64_t bitboard, int prev);

int pieceAt(Board* const b, const uint64_t coord, const int color);
int capturePiece(Board* b, const uint64_t pos, const int colorToCapture);

int equal(Board* a, Board* b);
Board duplicate(Board* b);
/*
Black pieces
Black pawns
-
-
-
-
White pawns
White pieces
*/
#include "../include/global.h"
#include "../include/board.h"

#include <stdio.h>

#define INITIAL_WPIECES 0xffff
#define INITIAL_WPAWNS 0xff00
#define INITIAL_WKING 0x8
#define INITIAL_WQUEEN 0x10
#define INITIAL_WROOK 0x81
#define INITIAL_WBISH 0x24
#define INITIAL_WKNIGHT 0x42

#define INITIAL_BPIECES 0xffff000000000000
#define INITIAL_BPAWNS 0xff000000000000
#define INITIAL_BKING 0x800000000000000
#define INITIAL_BQUEEN 0x1000000000000000
#define INITIAL_BROOK 0x8100000000000000
#define INITIAL_BBISH 0x2400000000000000
#define INITIAL_BKNIGHT 0x4200000000000000

//Starting: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1

Board generateFromFen(char* const fen, char* const toPlay, char* const castle)
{
    Board b = (Board) {};
    int i, num, shift;
    uint64_t pos = POW2[63];
    
    for (i = 0; pos != 0; ++i)
    {
        shift = 1;
        switch(fen[i])
        {
            case 'K':
                b.white |= pos;
                b.wKing |= pos;
                break;
            case 'Q':
                b.white |= pos;
                b.wQueen |= pos;
                incrWQueen(&b);
                break;
            case 'R':
                b.white |= pos;
                b.wRook |= pos;
                incrWRook(&b);
                break;
            case 'B':
                b.white |= pos;
                b.wBish |= pos;
                incrWBish(&b);
                break;
            case 'N':
                b.white |= pos;
                b.wKnight |= pos;
                incrWKnight(&b);
                break;
            case 'P':
                b.white |= pos;
                b.wPawn |= pos;
                incrWPawn(&b);
                break;

            case 'k':
                b.black |= pos;
                b.bKing |= pos;
                break;
            case 'q':
                b.black |= pos;
                b.bQueen |= pos;
                incrBQueen(&b);
                break;
            case 'r':
                b.black |= pos;
                b.bRook |= pos;
                incrBRook(&b);
                break;
            case 'b':
                b.black |= pos;
                b.bBish |= pos;
                incrBBish(&b);
                break;
            case 'n':
                b.black |= pos;
                b.bKnight |= pos;
                incrBKnight(&b);
                break;
            case 'p':
                b.black |= pos;
                b.bPawn |= pos;
                incrBPawn(&b);
                break;
            case '/': 
                shift = 0;
                break;

            default:
                num = fen[i] - (int)'0';
                pos >>= num;
                shift = 0;
        }

        pos >>= shift;
    }

    b.avWhite = ALL ^ b.white;
    b.avBlack = ALL ^ b.black;

    b.pieces = b.black | b.white;

    int castleInfo = 0;

    for (int i = 0, brk = 1; brk && i < 4; ++i)
    {
        switch (castle[i])
        {
            case 'K':
                castleInfo |= WCASTLEK;
                break;
            case 'Q':
                castleInfo |= WCASTLEQ;
                break;
            case 'k':
                castleInfo |= BCASTLEK;
                break;
            case 'q':
                castleInfo |= BCASTLEQ;
                break;
            default:
                brk = 0;
                break;
        }
    }

    b.posInfo = castleInfo | (toPlay[0] == 'w');

    return b;
}

Board defaultBoard()
{
    Board b = (Board) {
    .white = INITIAL_WPIECES, .avWhite = ALL ^ INITIAL_WPIECES, .wPawn = INITIAL_WPAWNS, 
    .wKing = INITIAL_WKING, .wQueen = INITIAL_WQUEEN, .wRook = INITIAL_WROOK, .wBish = INITIAL_WBISH, .wKnight = INITIAL_WKNIGHT,
    
    .black = INITIAL_BPIECES, .avBlack = ALL ^ INITIAL_BPIECES, .bPawn = INITIAL_BPAWNS, 
    .bKing = INITIAL_BKING, .bQueen = INITIAL_BQUEEN, .bRook = INITIAL_BROOK, .bBish = INITIAL_BBISH, .bKnight = INITIAL_BKNIGHT,
    
    .posInfo = 0b11111, .pieces = INITIAL_WPIECES | INITIAL_BPIECES, .numPieces = 0x8222182221ULL
    };

    return b;
}

/*
prev is incremented by 1, so the first call has to be made with prev = -1
It begins from h1, so that the wking index = 1 and bking index = 59
*/
unsigned int index(const uint64_t bitboard, int prev)
{
    uint64_t pos = POW2[++prev];
    
    while(prev < 64 && !(pos & bitboard))
    {
        pos <<= 1;
        ++prev;
    }

    return prev;
}
//Get the piece in the respective coord
unsigned int pieceAt(Board* const b, const unsigned int coord)
{
    if (POW2[coord] & b->white)
        return whitePieceAt(b, coord);
    else if (POW2[coord] & b->black)
        return blackPieceAt(b, coord);
    return 0;
}

/*
Specialized cases in which it is already assumed that there is a piece of 
the respective color.
The ifs are improved based on the average num of the piece type there are on
the board
*/

unsigned int captureBlackPiece(Board* b, const uint64_t pos)
{
    unsigned int targetPiece = blackPieceAt(b, pos);
    switch (targetPiece)
    {
        case KING:
        b->bKing ^= pos;
        //decr
        break;
        case QUEEN:
        b->bQueen ^= pos;
        decrBQueen(b);
        break;
        case ROOK:
        b->bRook ^= pos;
        decrBRook(b);
        break;
        case BISH:
        b->bBish ^= pos;
        decrBBish(b);
        break;
        case KNIGHT:
        b->bKnight ^= pos;
        decrBKnight(b);
        break;
        case PAWN:
        b->bPawn ^= pos;
        decrBPawn(b);
        break;
    }
    return targetPiece;
}

unsigned int captureWhitePiece(Board* b, const uint64_t pos)
{
    unsigned int targetPiece = whitePieceAt(b, pos);
    switch (targetPiece)
    {
        case KING:
        b->wKing ^= pos;
        //decr
        break;
        case QUEEN:
        b->wQueen ^= pos;
        decrWQueen(b);
        break;
        case ROOK:
        b->wRook ^= pos;
        decrWRook(b);
        break;
        case BISH:
        b->wBish ^= pos;
        decrWBish(b);
        break;
        case KNIGHT:
        b->wKnight ^= pos;
        decrWKnight(b);
        break;
        case PAWN:
        b->wPawn ^= pos;
        decrWPawn(b);
        break;
    }

    return targetPiece;
}

//TODO: finish implementing using the coords
unsigned int whitePieceAt(Board* const b, const uint64_t pos)
{
    unsigned int res = 0;

    if (pos & b->wPawn)     res = PAWN;
    else if (pos & b->wRook) res = ROOK;
    else if (pos & b->wBish) res = BISH;
    else if (pos & b->wKnight) res = KNIGHT;
    else if (pos & b->wQueen) res = QUEEN;
    else if (pos & b->wKing) res = KING;

    return res;
}
unsigned int blackPieceAt(Board* const b, const uint64_t pos)
{
    unsigned int res = 0;

    if (pos & b->bPawn)     res = PAWN;
    else if (pos & b->bRook) res = ROOK;
    else if (pos & b->bBish) res = BISH;
    else if (pos & b->bKnight) res = KNIGHT;
    else if (pos & b->bQueen) res = QUEEN;
    else if (pos & b->bKing) res = KING;
    
    return res;
}

int equal(Board* a, Board* b)
{
    int data = 
        (a->posInfo == b->posInfo) && (a->pieces == b->pieces) && (a->numPieces == b->numPieces);
    int white = 
        (a->white == b->white) && (a->avWhite == b->avWhite) && (a->wPawn == b->wPawn) && (a->wKing == b->wKing) && 
        (a->wQueen == b->wQueen) && (a->wRook == b->wRook) && (a->wBish == b->wBish) && (a->wKnight == b->wKnight);
    int black = 
        (a->black == b->black) && (a->avBlack == b->avBlack) && (a->bPawn == b->bPawn) && (a->bKing == b->bKing) && 
        (a->bQueen == b->bQueen) && (a->bRook == b->bRook) && (a->bBish == b->bBish) && (a->bKnight == b->bKnight);

    /*
    printf("Data: %d\n", data);
    printf("White: %d\n", white);
    printf("Black: %d\n", black);
    */

    return data && white && black;
}
/* allmoves.c
 * Its job is to generate all possible moves for a given position and color
 * legalMoves is the main function.
 */

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/memoization.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/magic.h"

#define SEVENTH_RANK 0xff000000000000
#define SECOND_RANK 0xff00

#define NOT_SEVENTH_RANK 0xff00ffffffffffff
#define NOT_SECOND_RANK 0xffffffffffff00ff

__attribute__((hot)) int movesKingFree(Board* b, Move* list, const int color, const uint64_t forbidden);
__attribute__((hot)) int movesPinnedPiece(Board* b, Move* list, const int color, const uint64_t forbidden, const uint64_t pinned);
__attribute__((hot)) int movesCheck(Board* b, Move* list, const int color, const uint64_t forbidden, const uint64_t pinned);

/* Generates all the legal moves for a given position and color
 */
int legalMoves(Board* b, Move* list)
{
    //Squares attacked by opp pieces, to detect checks and castling
    uint64_t forbidden = allSlidingAttacks(b, 1 ^ b->turn, b->allPieces ^ b->piece[b->turn][KING]) | controlledKingPawnKnight(b, 1 ^ b->turn);

    //All the pinned pieces for the side to move
    uint64_t pinned = pinnedPieces(b, b->turn);

    if (forbidden & b->piece[b->turn][KING])
        return (movesCheck(b, list, b->turn, forbidden, pinned) << 1) | 1;
    else if (pinned)
        return movesPinnedPiece(b, list, b->turn, forbidden, pinned) << 1;
    else
        return movesKingFree(b, list, b->turn, forbidden) << 1;
}

/* Detects if there is a check given by the queen / bish / rook. To detect discoveries or illegal moves
 */
static inline int moveIsValidSliding(Board* b, const Move m, History h)
{
    makeMove(b, m, &h);
    int chk = slidingCheck(b, 1 ^ b->turn);
    undoMove(b, m, &h);
    return !chk;
}

/* Returns a bitboard with a 1 for every pinned piece, works similarly to isInCheck
 *   1- Trace moves from the king as if it were a queen but separating each direction
 *   2- Only pay attention to the lines that the first intersection is with a piece of the king's color
 *   3- Retrace from that piece in the direction and detect if there is a Rook / Bish / Queen
 */
uint64_t pinnedPieces(Board* b, const int color)
{
    uint64_t res = 0;

    const int k = LSB_INDEX(b->piece[color][KING]);
    const int opp = 1 ^ color;

    const uint64_t stra = (b->piece[opp][QUEEN] | b->piece[opp][ROOK]) & getStraMoves(k);
    const uint64_t diag = (b->piece[opp][QUEEN] | b->piece[opp][BISH]) & getDiagMoves(k);
    uint64_t obst, retrace;

    if (stra && (obst = b->color[color] & getRookMagicMoves(k, b->allPieces)))
    {
        const uint64_t inteUp = getUpMovesInt(k) & obst;
        const uint64_t inteDown = getDownMovesInt(k) & obst;
        const uint64_t inteRight = getRightMovesInt(k) & obst;
        const uint64_t inteLeft = getLeftMovesInt(k) & obst;

        if (inteUp)
        {
            retrace = getUpMoves(LSB_INDEX(inteUp)) & b->allPieces;
            if (retrace & -retrace & stra)
                res |= inteUp;
        }
        if (inteDown)
        {
            retrace = getDownMoves(LSB_INDEX(inteDown)) & b->allPieces;
            if (retrace && (POW2[MSB_INDEX(retrace)] & stra))
                res |= inteDown;
        }
        if (inteRight)
        {
            retrace = getRightMoves(LSB_INDEX(inteRight)) & b->allPieces;
            if (retrace && (POW2[MSB_INDEX(retrace)] & stra))
                res |= inteRight;
        }
        if (inteLeft)
        {
            retrace = getLeftMoves(LSB_INDEX(inteLeft)) & b->allPieces;
            if (retrace & -retrace & stra)
                res |= inteLeft;
        }
    }
    if (diag && (obst = b->color[color] & getBishMagicMoves(k, b->allPieces)))
    {
        const uint64_t inteUpRight = getUpRightMovesInt(k) & obst;
        const uint64_t inteUpLeft = getUpLeftMovesInt(k) & obst;
        const uint64_t inteDownRight = getDownRightMovesInt(k) & obst;
        const uint64_t inteDownLeft = getDownLeftMovesInt(k) & obst;

        if (inteUpRight)
        {
            retrace = getUpRightMoves(LSB_INDEX(inteUpRight)) & b->allPieces;
            if (retrace & -retrace & diag)
                res |= inteUpRight;
        }
        if (inteUpLeft)
        {
            retrace = getUpLeftMoves(LSB_INDEX(inteUpLeft)) & b->allPieces;
            if (retrace & -retrace & diag)
                res |= inteUpLeft;
        }
        if (inteDownRight)
        {
            retrace = getDownRightMoves(LSB_INDEX(inteDownRight)) & b->allPieces;
            if (retrace && (POW2[MSB_INDEX(retrace)] & diag))
                res |= inteDownRight;
        }
        if (inteDownLeft)
        {
            retrace = getDownLeftMoves(LSB_INDEX(inteDownLeft)) & b->allPieces;
            if (retrace && (POW2[MSB_INDEX(retrace)] & diag))
                res |= inteDownLeft;
        }
    }

    return res;
}

/* Generates all legal moves if the king isnt in check nor is there a pinned piece
 * No move will leave the king in check except for enPassand, since the discoveries are harder to detect
 */
int movesKingFree(Board* b, Move* list, const int color, const uint64_t forbidden)
{
    const int opp = 1 ^ b->turn;
    const uint64_t oppPieces = b->color[opp];
    int from, to, numMoves = 0;
    uint64_t temp, tempMoves, tempCaptures;

    const int castle = canCastle(b, color, forbidden);
    if (castle & 1)
        list[numMoves++] = castleKSide(color);
    if (castle & 2)
        list[numMoves++] = castleQSide(color);


    //Promoting pawns
    temp = b->piece[color][PAWN] & (color? SEVENTH_RANK : SECOND_RANK);
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        if (color)
        {
            tempMoves = getWhitePawnMoves(from) & ~b->allPieces;
            tempCaptures = getWhitePawnCaptures(from) & b->color[BLACK];
        }
        else
        {
            tempMoves = getBlackPawnMoves(from) & ~b->allPieces;
            tempCaptures = getBlackPawnCaptures(from) & b->color[WHITE];
        }

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            int capt = pieceAt(b, POW2[to], opp); 

            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = QUEEN, .capture = capt, .score = 850};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = KNIGHT, .capture = capt, .score = 351};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = ROOK, .capture = capt, .score = 350};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = BISH, .capture = capt, .score = 150};

            REMOVE_LSB(tempCaptures);
        }
        while (tempMoves)
        {
            to = LSB_INDEX(tempMoves);

            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = QUEEN, .score = 800};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = KNIGHT, .score = 301};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = ROOK, .score = 300};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = BISH, .score = 100};

            REMOVE_LSB(tempMoves);
        }
    }


    //Pawns that arent going to promote
    temp = b->piece[color][PAWN] & (color? NOT_SEVENTH_RANK : NOT_SECOND_RANK);
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        if (color)
        {
            //Checks if there is a piece ahead, if so, the pawn cant move
            tempMoves = ((256ULL << from) & b->allPieces)? 0 : getWhitePawnMoves(from) & ~b->allPieces;
            tempCaptures = getWhitePawnCaptures(from) & oppPieces;
        }
        else
        {
            tempMoves = ((1ULL << (from - 8)) & b->allPieces)? 0 : getBlackPawnMoves(from) & ~b->allPieces;
            tempCaptures = getBlackPawnCaptures(from) & oppPieces;
        }

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
            REMOVE_LSB(tempCaptures);
        }
        while (tempMoves)
        {
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = LSB_INDEX(tempMoves)};
            REMOVE_LSB(tempMoves);
        }

        if (b->enPass - from == 1 && (from & 7) != 7 && (b->piece[opp][PAWN] & POW2[b->enPass]))
        {
            History h;
            Move m = (Move) {.piece = PAWN, .from = from, .to = from + 1 + (2 * color - 1) * 8, .enPass = b->enPass, .score = 101};

            if (moveIsValidSliding(b, m, h)) list[numMoves++] = m;
        }
        else if (b->enPass - from == -1 && (from & 7) && (b->piece[opp][PAWN] & POW2[b->enPass]))
        {
            History h;
            Move m = (Move) {.piece = PAWN, .from = from, .to = from - 1 + (2 * color - 1) * 8, .enPass = b->enPass, .score = 101};

            if (moveIsValidSliding(b, m, h)) list[numMoves++] = m;
        }
    }

    temp = b->piece[color][QUEEN];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        tempMoves = posQueenMoves(b, color, from);
        tempCaptures = tempMoves & oppPieces;
        tempMoves ^= tempCaptures;

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            list[numMoves++] = (Move) {.piece = QUEEN, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
            REMOVE_LSB(tempCaptures);
        }
        while(tempMoves)
        {
            list[numMoves++] = (Move) {.piece = QUEEN, .from = from, .to = LSB_INDEX(tempMoves)};
            REMOVE_LSB(tempMoves);
        }
    }


    temp = b->piece[color][ROOK];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        tempMoves = posRookMoves(b, color, from);
        tempCaptures = tempMoves & oppPieces;
        tempMoves ^= tempCaptures;

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            list[numMoves++] = (Move) {.piece = ROOK, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
            REMOVE_LSB(tempCaptures);
        }
        while(tempMoves)
        {
            list[numMoves++] = (Move) {.piece = ROOK, .from = from, .to = LSB_INDEX(tempMoves)};
            REMOVE_LSB(tempMoves);
        }
    }


    temp = b->piece[color][BISH];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        tempMoves = posBishMoves(b, color, from);
        tempCaptures = tempMoves & oppPieces;
        tempMoves ^= tempCaptures;

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            list[numMoves++] = (Move) {.piece = BISH, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
            REMOVE_LSB(tempCaptures);
        }
        while(tempMoves)
        {
            list[numMoves++] = (Move) {.piece = BISH, .from = from, .to = LSB_INDEX(tempMoves)};
            REMOVE_LSB(tempMoves);
        }
    }


    temp = b->piece[color][KNIGHT];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        tempMoves = posKnightMoves(b, color, from);
        tempCaptures = tempMoves & oppPieces;
        tempMoves ^= tempCaptures;

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            list[numMoves++] = (Move) {.piece = KNIGHT, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
            REMOVE_LSB(tempCaptures);
        }
        while(tempMoves)
        {
            list[numMoves++] = (Move) {.piece = KNIGHT, .from = from, .to = LSB_INDEX(tempMoves)};
            REMOVE_LSB(tempMoves);
        }
    }

    from = LSB_INDEX(b->piece[color][KING]);
    tempMoves = getKingMoves(from) & b->color[color | 2] & ~forbidden;
    tempCaptures = tempMoves & oppPieces;
    tempMoves ^= tempCaptures;
    while (tempCaptures)
    {
        to = LSB_INDEX(tempCaptures);
        list[numMoves++] = (Move) {.piece = KING, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
        REMOVE_LSB(tempCaptures);
    }
    while(tempMoves)
    {
        list[numMoves++] = (Move) {.piece = KING, .from = from, .to = LSB_INDEX(tempMoves)};
        REMOVE_LSB(tempMoves);
    }

    return numMoves;
}

/* Generates all legal moves when there is a pinned piece
 * Notice that a pinned piece can only move in the direction it is pinned
 */
int movesPinnedPiece(Board* b, Move* list, const int color, const uint64_t forbidden, const uint64_t pinned)
{
    const int opp = 1 ^ b->turn;
    const uint64_t oppPieces = b->color[opp];
    int from, to, numMoves = 0;
    uint64_t temp, tempMoves, tempCaptures;

    const int castle = canCastle(b, color, forbidden);
    if (castle & 1)
        list[numMoves++] = castleKSide(color);
    if (castle & 2)
        list[numMoves++] = castleQSide(color);

    from = LSB_INDEX(b->piece[color][KING]);
    tempMoves = getKingMoves(from) & b->color[color | 2] & ~forbidden;

    const uint64_t pinStra = getStraMoves(from);
    const uint64_t pinDiag = getDiagMoves(from);

    tempCaptures = tempMoves & oppPieces;
    tempMoves ^= tempCaptures;
    while (tempCaptures)
    {
        to = LSB_INDEX(tempCaptures);
        list[numMoves++] = (Move) {.piece = KING, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
        REMOVE_LSB(tempCaptures);
    }
    while(tempMoves)
    {
        list[numMoves++] = (Move) {.piece = KING, .from = from, .to = LSB_INDEX(tempMoves)};
        REMOVE_LSB(tempMoves);
    }

    //Promoting pawns
    temp = b->piece[color][PAWN] & (color? SEVENTH_RANK : SECOND_RANK);
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        if (b->turn)
        {
            tempMoves = getWhitePawnMoves(from) & ~b->allPieces;
            tempCaptures = getWhitePawnCaptures(from) & oppPieces;
        }
        else
        {
            tempMoves = getBlackPawnMoves(from) & ~b->allPieces;
            tempCaptures = getBlackPawnCaptures(from) & oppPieces;
        }
        if (pinned & POW2[from])
        {
            if (pinStra & POW2[from])
            {
                tempMoves &= pinStra;
                tempCaptures = 0;
            }
            else
            {
                tempMoves = 0;
                tempCaptures &= pinDiag;
            }
        }

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            REMOVE_LSB(tempCaptures);

            int capt = pieceAt(b, POW2[to], opp);
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = QUEEN, .capture = capt, .score = 850};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = KNIGHT, .capture = capt, .score = 351};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = ROOK, .capture = capt, .score = 350};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = BISH, .capture = capt, .score = 150};
        }
        while (tempMoves)
        {
            to = LSB_INDEX(tempMoves);
            REMOVE_LSB(tempMoves);

            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = QUEEN, .score = 800};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = KNIGHT, .score = 301};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = ROOK, .score = 300};
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = BISH, .score = 100};
        }
    }

    temp = b->piece[color][PAWN] & (color? NOT_SEVENTH_RANK : NOT_SECOND_RANK);
    while(temp)
    {
        History h;

        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        if (b->turn)
        {
            //Checks if there is a piece ahead, so the pawn cant move
            tempMoves = (256ULL << from) & b->allPieces ? 0 : getWhitePawnMoves(from) & ~b->allPieces;
            tempCaptures = getWhitePawnCaptures(from) & oppPieces;
        }
        else
        {
            tempMoves = (1ULL << (from - 8)) & b->allPieces ? 0 : getBlackPawnMoves(from) & ~b->allPieces;
            tempCaptures = getBlackPawnCaptures(from) & oppPieces;
        }

        if (pinned & POW2[from])
        {
            if (pinStra & POW2[from])
            {
                tempMoves &= pinStra;
                tempCaptures = 0;
            }
            else
            {
                tempMoves = 0;
                tempCaptures &= pinDiag;
            }
        }

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
            REMOVE_LSB(tempCaptures);
        }
        while (tempMoves)
        {
            list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = LSB_INDEX(tempMoves)};
            REMOVE_LSB(tempMoves);
        }

        if (b->enPass - from == 1 && (from & 7) != 7 && (b->piece[opp][PAWN] & POW2[b->enPass]))
        {
            Move m = (Move) {.piece = PAWN, .from = from, .to = from + 1 + (2 * color - 1) * 8, .enPass = b->enPass, .score = 101};

            if (moveIsValidSliding(b, m, h)) list[numMoves++] = m;
        }
        else if (b->enPass - from == -1 && (from & 7) && (b->piece[opp][PAWN] & POW2[b->enPass]))
        {
            Move m = (Move) {.piece = PAWN, .from = from, .to = from - 1 + (2 * color - 1) * 8, .enPass = b->enPass, .score = 101};

            if (moveIsValidSliding(b, m, h)) list[numMoves++] = m;
        }
    }

    temp = b->piece[color][QUEEN];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        uint64_t movesR = posRookMoves(b, color, from);
        uint64_t movesB = posBishMoves(b, color, from);

        if (pinned & POW2[from])
        {
            movesR &= ((pinStra & POW2[from]) != 0) * pinStra ; //To avoid branching
            movesB &= ((pinDiag & POW2[from]) != 0) * pinDiag;
        }

        tempMoves = movesR | movesB;
        tempCaptures = tempMoves & oppPieces;
        tempMoves ^= tempCaptures;

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            list[numMoves++] = (Move) {.piece = QUEEN, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
            REMOVE_LSB(tempCaptures);
        }
        while(tempMoves)
        {
            list[numMoves++] = (Move) {.piece = QUEEN, .from = from, .to = LSB_INDEX(tempMoves)};
            REMOVE_LSB(tempMoves);
        }
    }

    temp = b->piece[color][ROOK];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posRookMoves(b, color, from);

        if (pinned & POW2[from])
            tempMoves &= ((pinStra & POW2[from]) != 0) * pinStra;

        tempCaptures = tempMoves & oppPieces;
        tempMoves ^= tempCaptures;

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            list[numMoves++] = (Move) {.piece = ROOK, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
            REMOVE_LSB(tempCaptures);
        }
        while(tempMoves)
        {
            list[numMoves++] = (Move) {.piece = ROOK, .from = from, .to = LSB_INDEX(tempMoves)};
            REMOVE_LSB(tempMoves);
        }
    }


    temp = b->piece[color][BISH];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);
        tempMoves = posBishMoves(b, color, from);

        if (pinned & POW2[from]) 
            tempMoves &= ((pinDiag & POW2[from]) != 0) * pinDiag;

        tempCaptures = tempMoves & oppPieces;
        tempMoves ^= tempCaptures;

        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            list[numMoves++] = (Move) {.piece = BISH, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
            REMOVE_LSB(tempCaptures);
        }
        while(tempMoves)
        {
            list[numMoves++] = (Move) {.piece = BISH, .from = from, .to = LSB_INDEX(tempMoves)};
            REMOVE_LSB(tempMoves);
        }
    }


    temp = b->piece[color][KNIGHT];
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        if (!(pinned & POW2[from])) //Fun fact: A pinned knight cant move
        {
            tempMoves = posKnightMoves(b, color, from);
            tempCaptures = tempMoves & oppPieces;
            tempMoves ^= tempCaptures;

            while (tempCaptures)
            {
                to = LSB_INDEX(tempCaptures);
                list[numMoves++] = (Move) {.piece = KNIGHT, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
                REMOVE_LSB(tempCaptures);
            }
            while(tempMoves)
            {
                list[numMoves++] = (Move) {.piece = KNIGHT, .from = from, .to = LSB_INDEX(tempMoves)};
                REMOVE_LSB(tempMoves);
            }
        }
    }

    return numMoves;
}

/* Generates all legal moves when the king is in check
 * Notice that when the king is in check no pinned piece can move and if the number of attackers
 * is greater than 1 the only option is to move the king
 */
int movesCheck(Board* b, Move* list, const int color, const uint64_t forbidden, const uint64_t pinned)
{
    const int opp = 1 ^ b->turn;
    const uint64_t oppPieces = b->color[opp];
    int from, to, numMoves = 0;
    uint64_t temp, tempMoves, tempCaptures;
    History h;
    Move m;

    const AttacksOnK att = getCheckTiles(b, color);
    const uint64_t interfere = att.tiles; //A piece placed here will stop the check
    const uint64_t pinnedMask = ~pinned;

    from = LSB_INDEX(b->piece[color][KING]);
    tempMoves = getKingMoves(from) & b->color[color | 2] & ~forbidden;
    while(tempMoves)
    {
        to = LSB_INDEX(tempMoves);
        list[numMoves++] = (Move) {.piece = KING, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
        REMOVE_LSB(tempMoves);
    }

    //If is more than one attacker the only option is to move the king
    if (att.num == 1)
    {
        //Promoting pawns
        temp = b->piece[color][PAWN] & pinnedMask & (color? SEVENTH_RANK : SECOND_RANK);
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);

            if (color)
            {
                tempMoves = getWhitePawnMoves(from) & ~b->allPieces & interfere;
                tempCaptures = getWhitePawnCaptures(from) & oppPieces & interfere;
            }
            else
            {
                tempMoves = getBlackPawnMoves(from) & ~b->allPieces & interfere;
                tempCaptures = getBlackPawnCaptures(from) & oppPieces & interfere;
            }

            while (tempCaptures)
            {
                to = LSB_INDEX(tempCaptures);
                REMOVE_LSB(tempCaptures);
                int capt = pieceAt(b, POW2[to], opp);

                list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = QUEEN, .capture = capt, .score = 850};
                list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = KNIGHT, .capture = capt, .score = 351};
                list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = ROOK, .capture = capt, .score = 350};
                list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = BISH, .capture = capt, .score = 100};
            }
            while (tempMoves)
            {
                to = LSB_INDEX(tempMoves);
                REMOVE_LSB(tempMoves);

                list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = QUEEN, .score = 800};
                list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = KNIGHT, .score = 301};
                list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = ROOK, .score = 300};
                list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = BISH, .score = 100};
            }
        }

        temp = b->piece[color][PAWN] & pinnedMask & (color? NOT_SEVENTH_RANK : NOT_SECOND_RANK);
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);

           if (color)
            {
                //Checks if there is a piece ahead, if so, the pawn cant move
                tempMoves = ((256ULL << from) & b->allPieces)? 0 : getWhitePawnMoves(from) & ~b->allPieces & interfere;
                tempCaptures = getWhitePawnCaptures(from) & oppPieces & interfere;
            }
            else
            {
                tempMoves = ((1ULL << (from - 8)) & b->allPieces)? 0 : getBlackPawnMoves(from) & ~b->allPieces & interfere;
                tempCaptures = getBlackPawnCaptures(from) & oppPieces & interfere;
            }

            while (tempCaptures)
            {
                to = LSB_INDEX(tempCaptures);
                list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
                REMOVE_LSB(tempCaptures);
            }
            while (tempMoves)
            {
                list[numMoves++] = (Move) {.piece = PAWN, .from = from, .to = LSB_INDEX(tempMoves)};
                REMOVE_LSB(tempMoves);
            }

            if (b->enPass - from == 1 && (from & 7) != 7 && (b->piece[opp][PAWN] & POW2[b->enPass]))
            {
                m = (Move) {.piece = PAWN, .from = from, .to = from + 1 + (2 * color - 1) * 8, .enPass = b->enPass, .score = 101};
                
                if (moveIsValidSliding(b, m, h)) list[numMoves++] = m;
            }
            else if (b->enPass - from == -1 && (from & 7) && (b->piece[opp][PAWN] & POW2[b->enPass]))
            {
                m = (Move) {.piece = PAWN, .from = from, .to = from - 1 + (2 * color - 1) * 8, .enPass = b->enPass, .score = 101};
                
                if (moveIsValidSliding(b, m, h)) list[numMoves++] = m;
            }
        }

        temp = b->piece[color][QUEEN] & pinnedMask;
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);

            tempMoves = posQueenMoves(b, color, from) & interfere;
            tempCaptures = tempMoves & oppPieces;
            tempMoves ^= tempCaptures;

            while (tempCaptures)
            {
                to = LSB_INDEX(tempCaptures);
                list[numMoves++] = (Move) {.piece = QUEEN, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
                REMOVE_LSB(tempCaptures);
            }
            while(tempMoves)
            {
                list[numMoves++] = (Move) {.piece = QUEEN, .from = from, .to = LSB_INDEX(tempMoves)};
                REMOVE_LSB(tempMoves);
            }
        }

        temp = b->piece[color][ROOK] & pinnedMask;
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);

            tempMoves = posRookMoves(b, color, from) & interfere;
            tempCaptures = tempMoves & oppPieces;
            tempMoves ^= tempCaptures;

            while (tempCaptures)
            {
                to = LSB_INDEX(tempCaptures);
                list[numMoves++] = (Move) {.piece = ROOK, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
                REMOVE_LSB(tempCaptures);
            }
            while(tempMoves)
            {
                list[numMoves++] = (Move) {.piece = ROOK, .from = from, .to = LSB_INDEX(tempMoves)};
                REMOVE_LSB(tempMoves);
            }
        }

        temp = b->piece[color][BISH] & pinnedMask;
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);

            tempMoves = posBishMoves(b, color, from) & interfere;
            tempCaptures = tempMoves & oppPieces;
            tempMoves ^= tempCaptures;

            while (tempCaptures)
            {
                to = LSB_INDEX(tempCaptures);
                list[numMoves++] = (Move) {.piece = BISH, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
                REMOVE_LSB(tempCaptures);
            }
            while(tempMoves)
            {
                list[numMoves++] = (Move) {.piece = BISH, .from = from, .to = LSB_INDEX(tempMoves)};
                REMOVE_LSB(tempMoves);
            }
        }

        temp = b->piece[color][KNIGHT] & pinnedMask;
        while(temp)
        {
            from = LSB_INDEX(temp);
            REMOVE_LSB(temp);

            tempMoves = posKnightMoves(b, color, from) & interfere;
            tempCaptures = tempMoves & oppPieces;
            tempMoves ^= tempCaptures;

            while (tempCaptures)
            {
                to = LSB_INDEX(tempCaptures);
                list[numMoves++] = (Move) {.piece = KNIGHT, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
                REMOVE_LSB(tempCaptures);
            }
            while(tempMoves)
            {
                list[numMoves++] = (Move) {.piece = KNIGHT, .from = from, .to = LSB_INDEX(tempMoves)};
                REMOVE_LSB(tempMoves);
            }
        }
    }

    return numMoves;
}
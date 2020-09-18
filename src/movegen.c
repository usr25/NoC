#include "../include/global.h"
#include "../include/memoization.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/magic.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/movegen.h"

#include <assert.h>

#define SEVENTH_RANK 0xff000000000000
#define SECOND_RANK 0xff00

#define NOT_SEVENTH_RANK 0xff00ffffffffffff
#define NOT_SECOND_RANK 0xffffffffffff00ff


void genTacticals(MoveGen* mg, const Board* b);

void addPawn(MoveGen* mg, const Board* b, const int onlyTacticals);
void addKnight(MoveGen* mg, const Board* b, const int onlyTacticals);
void addKing(MoveGen* mg, const Board* b, const int onlyTacticals);
void addSlidings(MoveGen* mg, const Board* b, const int piece, const int onlyTacticals);

void genQuiets(MoveGen* mg, const Board* b);

MoveGen newMG(const Board* b, const int qsearch, const Move bestM) {
    MoveGen mg = (MoveGen){.qsearch = qsearch, .nmoves = 0, .currmove = 0, .tot = 0, .state = Uninitialized, .bestMoveIdx = -1};

    mg.forbidden = allSlidingAttacks(b, 1 ^ b->stm, b->allPieces ^ b->piece[b->stm][KING]) | controlledKingPawnKnight(b, 1 ^ b->stm);
    mg.pinned = pinnedPieces(b, b->stm);

    if (mg.forbidden & b->piece[b->stm][KING]) {
        mg.nmoves = movesCheck(b, mg.moves, b->stm, mg.forbidden, mg.pinned);
        mg.state = Quiet;
    } else {
        genTacticals(&mg, b);
        if (mg.state == Uninitialized) {
            if (!mg.qsearch)
                genQuiets(&mg, b);
            else
                mg.state = Exhausted;
        }
    }

    assert(mg.state != Uninitialized);
    return mg;
}


void genTacticals(MoveGen* mg, const Board* b) {

    mg->nmoves = 0;

    addPawn(mg, b, 1);
    addKnight(mg, b, 1);
    for (int i = BISH; i >= QUEEN; --i)
    {
        addSlidings(mg, b, i, 1);
    }
    addKing(mg, b, 1);

    mg->state = mg->nmoves? Tactical : Uninitialized;
}

void addPawn(MoveGen* mg, const Board* b, const int onlyTacticals) {
    Move* p = mg->moves + mg->nmoves;
    //Promoting
    uint64_t temp = b->piece[b->stm][PAWN] & (b->stm? SEVENTH_RANK : SECOND_RANK); 
    int from, to;
    uint64_t tempCaptures, tempMoves;
    const int opp = 1^b->stm;
    const uint64_t oppPieces = b->color[opp];

    const int k = LSB_INDEX(b->piece[b->stm][KING]);
    const uint64_t pinStra = getStraMoves(k);
    const uint64_t pinDiag = getDiagMoves(k);

    //Promoting pawns
    while (temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        if (b->stm)
        {
            tempMoves = getWhitePawnMoves(from) & ~b->allPieces;
            tempCaptures = getWhitePawnCaptures(from) & b->color[BLACK];
        }
        else
        {
            tempMoves = getBlackPawnMoves(from) & ~b->allPieces;
            tempCaptures = getBlackPawnCaptures(from) & b->color[WHITE];
        }

        if (mg->pinned & POW2[from])
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
            int capt = pieceAt(b, POW2[to], opp); 

            if (onlyTacticals){
                *p++ = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = QUEEN, .capture = capt, .score = 650};
            } else {
                for (int i = ROOK; i <= KNIGHT; ++i)
                    *p++ = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = i, .capture = capt, .score = 550};
            }

            REMOVE_LSB(tempCaptures);
        }
        while (tempMoves)
        {
            to = LSB_INDEX(tempMoves);

            if (onlyTacticals){
                *p++ = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = QUEEN, .score = 650};
            } else {
                for (int i = ROOK; i <= KNIGHT; ++i)
                    *p++ = (Move) {.piece = PAWN, .from = from, .to = to, .promotion = i, .score = 550};
            }

            REMOVE_LSB(tempMoves);
        }
    }

    //Pawns which aren't going to promote
    temp = b->piece[b->stm][PAWN] & (b->stm? NOT_SEVENTH_RANK : NOT_SECOND_RANK);
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        if (b->stm)
        {
            //Checks if there is a piece ahead, if so, the pawn can't move
            tempMoves = ((256ULL << from) & b->allPieces)? 0 : getWhitePawnMoves(from) & ~b->allPieces;
            tempCaptures = getWhitePawnCaptures(from) & oppPieces;
        }
        else
        {
            tempMoves = ((1ULL << (from - 8)) & b->allPieces)? 0 : getBlackPawnMoves(from) & ~b->allPieces;
            tempCaptures = getBlackPawnCaptures(from) & oppPieces;
        }

        if (mg->pinned & POW2[from])
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

        if (onlyTacticals)
        {
            while (tempCaptures)
            {
                to = LSB_INDEX(tempCaptures);
                *p++ = (Move) {.piece = PAWN, .from = from, .to = to, .capture = pieceAt(b, POW2[to], opp)};
                REMOVE_LSB(tempCaptures);
            }
        }
        else
        {
            while (tempMoves)
            {
                *p++ = (Move) {.piece = PAWN, .from = from, .to = LSB_INDEX(tempMoves)};
                REMOVE_LSB(tempMoves);
            }
        }

        //EnPassand moves
        if (onlyTacticals)
        {
            if (b->enPass - from == 1 && (from & 7) != 7 && (b->piece[opp][PAWN] & POW2[b->enPass]))
            {
                Move m = (Move) {.piece = PAWN, .from = from, .to = from + 1 + (2 * b->stm - 1) * 8, .enPass = b->enPass, .score = 101};

                if (moveIsValidSliding(*b, m)) *p++ = m;
            }
            else if (b->enPass - from == -1 && (from & 7) && (b->piece[opp][PAWN] & POW2[b->enPass]))
            {
                Move m = (Move) {.piece = PAWN, .from = from, .to = from - 1 + (2 * b->stm - 1) * 8, .enPass = b->enPass, .score = 101};

                if (moveIsValidSliding(*b, m)) *p++ = m;
            }
        }
    }

    mg->nmoves += p - (mg->moves + mg->nmoves);
}

void addKnight(MoveGen* mg, const Board* b, const int onlyTacticals) {
    Move* p = mg->moves + mg->nmoves;
    uint64_t temp = b->piece[b->stm][KNIGHT] & ~mg->pinned;
    int from, to;
    uint64_t tempCaptures, tempMoves;
    while(temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        tempMoves = posKnightMoves(b, b->stm, from);
        tempCaptures = tempMoves & b->color[1^b->stm];
        tempMoves ^= tempCaptures;

        if (onlyTacticals)
        {
            while (tempCaptures)
            {
                to = LSB_INDEX(tempCaptures);
                *p++ = (Move) {.piece = KNIGHT, .from = from, .to = to, .capture = pieceAt(b, POW2[to], 1^b->stm)};
                REMOVE_LSB(tempCaptures);
            }
        }
        else
        {
            while (tempMoves)
            {
                *p++ = (Move) {.piece = KNIGHT, .from = from, .to = LSB_INDEX(tempMoves)};
                REMOVE_LSB(tempMoves);
            }
        }
    }

    mg->nmoves += p - (mg->moves + mg->nmoves);
}

void addKing(MoveGen* mg, const Board* b, const int onlyTacticals) {
    Move* p = mg->moves + mg->nmoves;

    const int castle = canCastle(b, b->stm, mg->forbidden);
    if (onlyTacticals)
    {
        if (castle & 1)
            *p++ = castleKSide(b->stm);
        if (castle & 2)
            *p++ = castleQSide(b->stm);
    }

    int from = LSB_INDEX(b->piece[b->stm][KING]);
    int to;
    uint64_t tempMoves = getKingMoves(from) & b->color[b->stm | 2] & ~mg->forbidden;
    uint64_t tempCaptures = tempMoves & b->color[1^b->stm];
    tempMoves ^= tempCaptures;
    if (onlyTacticals)
    {
        while (tempCaptures)
        {
            to = LSB_INDEX(tempCaptures);
            *p++ = (Move) {.piece = KING, .from = from, .to = to, .capture = pieceAt(b, POW2[to], 1^b->stm)};
            REMOVE_LSB(tempCaptures);
        }
    }
    else
    { 
        while(tempMoves)
        {
            *p++ = (Move) {.piece = KING, .from = from, .to = LSB_INDEX(tempMoves)};
            REMOVE_LSB(tempMoves);
        }
    }

    mg->nmoves += p - (mg->moves + mg->nmoves);
}

void addSlidings(MoveGen* mg, const Board* b, const int piece, const int onlyTacticals) {
    Move* p = mg->moves + mg->nmoves;
    uint64_t temp = b->piece[b->stm][piece];
    int from, to;
    const uint64_t oppPieces = b->color[1^b->stm];

    const int k = LSB_INDEX(b->piece[b->stm][KING]);
    const uint64_t pinStra = getStraMoves(k);
    const uint64_t pinDiag = getDiagMoves(k);

    uint64_t tempMoves, tempCaptures;

    while (temp)
    {
        from = LSB_INDEX(temp);
        REMOVE_LSB(temp);

        uint64_t stra = 0, diag = 0;
        switch(piece)
        {
            case BISH:
                diag = posBishMoves(b, b->stm, from);
            break;
            case ROOK:
                stra = posRookMoves(b, b->stm, from);
            break;
            case QUEEN:
                diag = posBishMoves(b, b->stm, from);
                stra = posRookMoves(b, b->stm, from);
            break;
            default:
                assert(0);
        }

        if (mg->pinned & POW2[from])
        {
            stra &= ((pinStra & POW2[from]) != 0) * pinStra; //To avoid branching
            diag &= ((pinDiag & POW2[from]) != 0) * pinDiag;
        }

        assert(!(piece == ROOK && diag));
        assert(!(piece == BISH && stra));

        tempMoves = stra | diag;
        tempCaptures = tempMoves & oppPieces;
        tempMoves ^= tempCaptures;

        if (onlyTacticals)
        {
            while (tempCaptures)
            {
                to = LSB_INDEX(tempCaptures);
                *p++ = (Move) {.piece = piece, .from = from, .to = to, .capture = pieceAt(b, POW2[to], 1^b->stm)};
                REMOVE_LSB(tempCaptures);
            }
        }
        else
        {
            while(tempMoves)
            {
                *p++ = (Move) {.piece = piece, .from = from, .to = LSB_INDEX(tempMoves)};
                REMOVE_LSB(tempMoves);
            }
        }
    }

    mg->nmoves += p - (mg->moves + mg->nmoves);
}

void genQuiets(MoveGen* mg, const Board* b) {
    mg->nmoves = 0;
    mg->currmove = 0;

    addPawn(mg, b, 0);
    addKnight(mg, b, 0);
    for (int i = BISH; i >= QUEEN; --i)
    {
        addSlidings(mg, b, i, 0);
    }
    addKing(mg, b, 0);

    mg->state = mg->nmoves? Quiet : Exhausted;
}

Move next(MoveGen* mg, const Board* b) {
    assert(mg->state != Uninitialized);
    //We have finished the current batch of moves
    if (mg->currmove >= mg->nmoves) {
        if (mg->state == Tactical && !mg->qsearch){
            genQuiets(mg, b);
            if (mg->nmoves){
                mg->tot++;
                return mg->moves[mg->currmove++];
            }
        } else {
            mg->state = Exhausted;
        }
        return (Move) {.from = -1};
    } else {
        mg->tot++;
        return mg->moves[mg->currmove++];
    }
}

int collect(Move* list, const Board* b)
{
    MoveGen mg = (MoveGen) {.nmoves = 0, .currmove = 0, .tot = 0, .state = Uninitialized};
    mg.forbidden = allSlidingAttacks(b, 1 ^ b->stm, b->allPieces ^ b->piece[b->stm][KING]) | controlledKingPawnKnight(b, 1 ^ b->stm);
    mg.pinned = pinnedPieces(b, b->stm);

    if (mg.forbidden & b->piece[b->stm][KING])
        return movesCheck(b, list, b->stm, mg.forbidden, mg.pinned);

    Move* p = list;
    genTacticals(&mg, b);
    for (int i = 0; i < mg.nmoves; ++i)
        *p++ = mg.moves[i];
    genQuiets(&mg, b);
    for (int i = 0; i < mg.nmoves; ++i)
        *p++ = mg.moves[i];

    return p - list;
}
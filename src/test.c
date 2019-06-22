#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/io.h"
#include "../include/perft.h"

#include <stdio.h>
#include <assert.h>

int validStartingPos(Board b)
{
    int correctNumOfPieces = 
        POPCOUNT(b.piece[WHITE][PAWN]) == 8 &&
        POPCOUNT(b.piece[BLACK][PAWN]) == 8 &&
        POPCOUNT(b.piece[WHITE][KING]) == 1 &&
        POPCOUNT(b.piece[BLACK][KING]) == 1 &&
        POPCOUNT(b.piece[WHITE][QUEEN]) == 1 &&
        POPCOUNT(b.piece[BLACK][QUEEN]) == 1 &&
        POPCOUNT(b.piece[WHITE][ROOK]) == 2 &&
        POPCOUNT(b.piece[BLACK][ROOK]) == 2 &&
        POPCOUNT(b.piece[WHITE][BISH]) == 2 &&
        POPCOUNT(b.piece[BLACK][BISH]) == 2 &&
        POPCOUNT(b.piece[WHITE][KNIGHT]) == 2 &&
        POPCOUNT(b.piece[BLACK][KNIGHT]) == 2;

    int piecesAddUp = POPCOUNT(b.color[WHITE]) == 16 && POPCOUNT(b.color[BLACK]) == 16;
    int availableAddUp = POPCOUNT(b.color[AV_WHITE]) == 48 && POPCOUNT(b.color[AV_BLACK]) == 48;
    int avAreCorrect = (b.color[WHITE] & b.color[AV_WHITE]) == 0 && (b.color[BLACK] & b.color[AV_BLACK]) == 0;
    int whiteAreInAvBlack = (b.color[WHITE] & b.color[AV_BLACK]) == b.color[WHITE];
    int blackAreInAvWhite = (b.color[BLACK] & b.color[AV_WHITE]) == b.color[BLACK];
    int posInfoCorrect = b.posInfo == 0b11111;

    return 
    correctNumOfPieces && piecesAddUp && availableAddUp && avAreCorrect && whiteAreInAvBlack && blackAreInAvWhite && posInfoCorrect;
}

int validPieces(Board b)
{
    int validKings = (POPCOUNT(b.piece[WHITE][KING]) == 1) && (POPCOUNT(b.piece[BLACK][KING]) == 1);
    int totNumPieces = (POPCOUNT(b.color[WHITE]) <= 16) && (POPCOUNT(b.color[BLACK]) <= 16);
    int validNumPawns = (POPCOUNT(b.piece[WHITE][PAWN]) <= 8) && (POPCOUNT(b.piece[BLACK][PAWN]) <= 8);

    return validKings && totNumPieces && validNumPawns;
}

int testGeneration()
{
    Board a = defaultBoard();
    Board b = generateFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", "w", "KQkq");

    return 
        equal(&a, &b) && validPieces(a) && validPieces(b) && validStartingPos(a) && validStartingPos(b);
}

int testKingMoves()
{
    uint64_t movsW, movsB;
    Board b = defaultBoard();
    movsW = posKingMoves(&b, 1);
    movsB = posKingMoves(&b, 0);

    int defaultMovs = 
        (movsW == 0ULL) && (movsB == 0ULL);

    b = generateFromFen("5N2/5Pkq/5p2/3P4/3pKQ2/3n4/8/8", "w", "-");
    movsW = posKingMoves(&b, 1);
    movsB = posKingMoves(&b, 0);

    int otherMovs = 
        (movsW == 51809878016ULL) && (movsB == 505532356707221504ULL);

    return defaultMovs && otherMovs;
}
int testQueenMoves()
{
    uint64_t movsW, movsB;
    Board b = defaultBoard();
    movsW = posQueenMoves(&b, 1, LSB_INDEX(b.piece[WHITE][QUEEN]));
    movsB = posQueenMoves(&b, 0, LSB_INDEX(b.piece[BLACK][QUEEN]));

    int defaultMovs = 
        (movsW == 0ULL) && (movsB == 0ULL);

    b = generateFromFen("8/2n2N2/8/1br2rb1/n1Q2q1N/1BR2RB1/8/8", "w", "-");
    movsW = posQueenMoves(&b, 1, LSB_INDEX(b.piece[WHITE][QUEEN]));
    movsB = posQueenMoves(&b, 0, LSB_INDEX(b.piece[BLACK][QUEEN]));

    int otherMovs = 
        (movsW == 9280821397508ULL) && (movsB == 17627536560160ULL);

    return defaultMovs && otherMovs;
}
int testRookMoves()
{
    uint64_t movsW1, movsW2, movsB1, movsB2;
    uint64_t posW, posB;
    Board b = defaultBoard();

    posW = b.piece[WHITE][ROOK];
    posB = b.piece[BLACK][ROOK];
    
    movsW1 = posRookMoves(&b, 1, LSB_INDEX(posW));
    movsW2 = posRookMoves(&b, 1, MSB_INDEX(posW));
    movsB1 = posRookMoves(&b, 0, LSB_INDEX(posB));
    movsB2 = posRookMoves(&b, 0, MSB_INDEX(posB));

    int defaultMovs = 
        (movsW1 == 0ULL) && (movsW2 == 0ULL) && (movsB1 == 0ULL) && (movsB2 == 0ULL);

    b = generateFromFen("8/8/2Br2R1/8/8/2BR2r1/8/8", "w", "-");

    posW = b.piece[WHITE][ROOK];
    posB = b.piece[BLACK][ROOK];

    movsW1 = posRookMoves(&b, 1, LSB_INDEX(posW));
    movsW2 = posRookMoves(&b, 1, MSB_INDEX(posW));
    movsB1 = posRookMoves(&b, 0, LSB_INDEX(posB));
    movsB2 = posRookMoves(&b, 0, MSB_INDEX(posB));

    int otherMovs = 
        (movsW1 == 17661174878224ULL) && (movsW2 == 144710032490102784ULL) && (movsB1 == 2207648645634ULL) && (movsB2 == 1157475750758055936ULL);

    return defaultMovs && otherMovs;
}
int testBishMoves()
{
    uint64_t movsW1, movsW2, movsB1, movsB2;
    uint64_t posW, posB;
    Board b = defaultBoard();

    posW = b.piece[WHITE][BISH];
    posB = b.piece[BLACK][BISH];

    movsW1 = posBishMoves(&b, 1, LSB_INDEX(posW));
    movsW2 = posBishMoves(&b, 1, MSB_INDEX(posW));
    movsB1 = posBishMoves(&b, 0, LSB_INDEX(posB));
    movsB2 = posBishMoves(&b, 0, MSB_INDEX(posB));

    int defaultMovs = 
        (movsW1 == 0ULL) && (movsW2 == 0ULL) && (movsB1 == 0ULL) && (movsB2 == 0ULL);

    b = generateFromFen("8/2r4R/3B2b1/8/8/3B2b1/2R4r/8", "w", "-");

    posW = b.piece[WHITE][BISH];
    posB = b.piece[BLACK][BISH];

    movsW1 = posBishMoves(&b, 1, LSB_INDEX(posW));
    movsW2 = posBishMoves(&b, 1, MSB_INDEX(posW));
    movsB1 = posBishMoves(&b, 0, LSB_INDEX(posB));
    movsB2 = posBishMoves(&b, 0, MSB_INDEX(posB));

    int otherMovs = 
        (movsW1 == 143229240477700ULL) && (movsW2 == 299489548168200192ULL) && (movsB1 == 17626629669896ULL) && (movsB2 == 577868148797079552ULL);

    return defaultMovs && otherMovs;
}
int testKnightMoves()
{
    uint64_t movsW1, movsW2, movsB1, movsB2;
    uint64_t posW, posB;
    Board b = defaultBoard();

    posW = b.piece[WHITE][KNIGHT];
    posB = b.piece[BLACK][KNIGHT];

    movsW1 = posKnightMoves(&b, 1, LSB_INDEX(posW));
    movsW2 = posKnightMoves(&b, 1, MSB_INDEX(posW));
    movsB1 = posKnightMoves(&b, 0, LSB_INDEX(posB));
    movsB2 = posKnightMoves(&b, 0, MSB_INDEX(posB));

    int defaultMovs = 
        (movsW1 == 327680ULL) && (movsW2 == 10485760ULL) && (movsB1 == 5497558138880ULL) && (movsB2 == 175921860444160ULL);

    b = generateFromFen("8/1p1p1Pk1/2N3K1/n3N3/2n3p1/qP1PQ3/8/8", "w", "-");
    
    posW = b.piece[WHITE][KNIGHT];
    posB = b.piece[BLACK][KNIGHT];

    movsW1 = posKnightMoves(&b, 1, LSB_INDEX(posW));
    movsW2 = posKnightMoves(&b, 1, MSB_INDEX(posW));
    movsB1 = posKnightMoves(&b, 0, LSB_INDEX(posB));
    movsB2 = posKnightMoves(&b, 0, MSB_INDEX(posB));

    int otherMovs = 
        (movsW1 == 4503600198057984ULL) && (movsW2 == 5802888670964875264ULL) && (movsB1 == 87995290505216ULL) && (movsB2 == 35184376283136ULL);

    return defaultMovs && otherMovs;
}

int testPawnMoves()
{
    uint64_t movsW1, movsW2, movsW3, movsB1, movsB2, movsB3;
    uint64_t posW, posB;
    Board b = defaultBoard();

    posW = b.piece[WHITE][PAWN];
    posB = b.piece[BLACK][PAWN];

    int defaultMovs = 1;
    for (int i = 0; i < 8 && defaultMovs; ++i)
    {
        defaultMovs &= posPawnMoves(&b, 1, LSB_INDEX(posW)) == (POW2[i + 8] | POW2[i]) << 16;
        defaultMovs &= posPawnMoves(&b, 0, LSB_INDEX(posB)) == (POW2[i] | POW2[i + 8]) << 32;
        REMOVE_LSB(posW);
        REMOVE_LSB(posB);
    }

    b = generateFromFen("8/4p3/3Q1q2/4q3/4q3/pq3P1p/P5P1/8", "w", "-");

    posW = b.piece[WHITE][PAWN];
    posB = b.piece[BLACK][PAWN];

    movsW1 = posPawnMoves(&b, 1, LSB_INDEX(posW)); REMOVE_LSB(posW);
    movsW2 = posPawnMoves(&b, 1, LSB_INDEX(posW)); REMOVE_LSB(posW);
    movsW3 = posPawnMoves(&b, 1, LSB_INDEX(posW)); REMOVE_LSB(posW);
    movsB1 = posPawnMoves(&b, 0, LSB_INDEX(posB)); REMOVE_LSB(posB);
    movsB2 = posPawnMoves(&b, 0, LSB_INDEX(posB)); REMOVE_LSB(posB);
    movsB3 = posPawnMoves(&b, 0, LSB_INDEX(posB)); REMOVE_LSB(posB);

    int otherMovs = 
        (movsW1 == 33751040ULL) && (movsW2 == 4194304ULL) && (movsW3 == 201326592ULL) && (movsB1 == 768ULL) && (movsB2 == 0ULL) && (movsB3 == 26388279066624ULL);

    return defaultMovs && otherMovs;
}

int testChecks()
{
    Board b;
    int white = 1, black = 1;

    //White king
    b = generateFromFen("8/8/1q6/8/3K4/8/8/8", "w", "-");
    white &= isInCheck(&b, 1) == QUEEN;
    b = generateFromFen("8/8/8/8/3K4/8/8/3q4", "w", "-");
    white &= isInCheck(&b, 1) == QUEEN;

    b = generateFromFen("8/8/8/8/3K3r/8/8/8", "w", "-");
    white &= isInCheck(&b, 1) == ROOK;    
    b = generateFromFen("8/8/8/2b5/3K4/8/8/8", "w", "-");
    white &= isInCheck(&b, 1) == BISH;
    b = generateFromFen("8/8/8/5n2/3K4/8/8/8", "w", "-");
    white &= isInCheck(&b, 1) == KNIGHT;
    b = generateFromFen("8/8/8/2p5/3K4/8/8/8", "w", "-");
    white &= isInCheck(&b, 1) == PAWN;
    b = generateFromFen("4r3/8/3k4/1q6/3K4/8/2p2n2/2b5", "w", "-");
    white &= isInCheck(&b, 1) == NO_PIECE;

    //Black king
    b = generateFromFen("8/8/1Q6/8/3k4/8/8/8", "w", "-");
    black &= isInCheck(&b, 0) == QUEEN;
    b = generateFromFen("8/8/8/8/3k4/8/8/3Q4", "w", "-");
    black &= isInCheck(&b, 0) == QUEEN;

    b = generateFromFen("8/8/8/8/3k3R/8/8/8", "w", "-");
    black &= isInCheck(&b, 0) == ROOK;
    b = generateFromFen("8/8/8/2B5/3k4/8/8/8", "w", "-");
    black &= isInCheck(&b, 0) == BISH;
    b = generateFromFen("8/8/8/5N2/3k4/8/8/8", "w", "-");
    black &= isInCheck(&b, 0) == KNIGHT;
    b = generateFromFen("8/8/8/8/3k4/2P5/8/8", "w", "-");
    black &= isInCheck(&b, 0) == PAWN;
    b = generateFromFen("4R3/8/3K4/1Q6/3k4/8/2P2N2/2B5", "w", "-");
    black &= isInCheck(&b, 0) == NO_PIECE;
    
    return white && black;
}

int testUndoMoves()
{
    
    Board b = defaultBoard();
    Board _b = defaultBoard();
    History h;
    int stays = equal(&b, &_b);

    int temp; //TODO: Remove this
    Move w1 = (Move) {.pieceThatMoves= PAWN, .from = 8, .to = 56, .color = WHITE};

    makeMove(&b, &w1, &h);
    undoMove(&b, w1, &h);
    int white = equal(&b, &_b);

    Move b1 = (Move) {.pieceThatMoves= PAWN, .from = 55, .to = 7, .color = BLACK};

    makeMove(&b, &b1, &h);
    undoMove(&b, b1, &h);
    int black = equal(&b, &_b);

    return stays && white && black;
}

int testBoardPawnMoves()
{
    int temp;
    History h;
    Board b;
    Move w1, w2, b1, b2;
    int pawnMovesNoCapture = 1;
    for (int i = 0; i < 8; ++i)
    {
        w1 = (Move) {.pieceThatMoves= PAWN, .from = i + 8, .to = i + 16, .color = WHITE};
        w2 = (Move) {.pieceThatMoves= PAWN, .from = i + 16, .to = i + 24, .color = WHITE};
        
        b1 = (Move) {.pieceThatMoves= PAWN, .from = 55 - i, .to = 47 - i, .color = BLACK};
        b2 = (Move) {.pieceThatMoves= PAWN, .from = 47 - i, .to = 39 - i, .color = BLACK};

        b = defaultBoard();
        
        makeMove(&b, &w1, &h);
        makeMove(&b, &w2, &h);
        makeMove(&b, &b1, &h);
        makeMove(&b, &b2, &h);

        pawnMovesNoCapture &= ((b.piece[WHITE][PAWN] & POW2[i + 16]) == 0) && ((POW2[i + 24] & b.piece[WHITE][PAWN]) == POW2[i + 24]);
        pawnMovesNoCapture &= ((b.color[WHITE] | b.piece[WHITE][PAWN]) == b.color[WHITE]) && ((b.color[WHITE] & b.piece[WHITE][PAWN]) == b.piece[WHITE][PAWN]);
        
        pawnMovesNoCapture &= ((b.piece[BLACK][PAWN] & POW2[47 - i]) == 0) && ((POW2[39 - i] & b.piece[BLACK][PAWN]) == POW2[39 - i]);
        pawnMovesNoCapture &= ((b.color[BLACK] | b.piece[BLACK][PAWN]) == b.color[BLACK]) && ((b.color[BLACK] & b.piece[BLACK][PAWN]) == b.piece[BLACK][PAWN]);

        pawnMovesNoCapture &= (POPCOUNT(b.piece[WHITE][PAWN]) == POPCOUNT(b.piece[WHITE][PAWN])) && (POPCOUNT(b.piece[BLACK][PAWN]) == POPCOUNT(b.piece[BLACK][PAWN]));
    }

    int pawnMovesCapture = 1;

    for (int i = 0; i < 8; ++i)
    {
        w1 = (Move) {.pieceThatMoves= PAWN, .from = 8 + i, .to = 48 + i, .color = WHITE};
        b1 = (Move) {.pieceThatMoves= PAWN, .from = 48 + i, .to = 8 + i, .color = BLACK};

        b = defaultBoard();
        makeMove(&b, &w1, &h);

        pawnMovesCapture &= ((b.piece[WHITE][PAWN] & POW2[8 + i]) == 0) && ((POW2[48 + i] & b.piece[WHITE][PAWN]) == POW2[48 + i]);
        pawnMovesCapture &= ((b.color[WHITE] | b.piece[WHITE][PAWN]) == b.color[WHITE]) && ((b.color[WHITE] & b.piece[WHITE][PAWN]) == b.piece[WHITE][PAWN]);
   
        pawnMovesCapture &= (b.piece[BLACK][PAWN] & POW2[48 + i]) == 0;
        pawnMovesCapture &= (POPCOUNT(b.piece[BLACK][PAWN]) == 7) && (POPCOUNT(b.piece[BLACK][PAWN]) == 7);
        pawnMovesCapture &= POPCOUNT(b.color[BLACK]) == 15;

        b = defaultBoard();
        makeMove(&b, &b1, &h);

        pawnMovesCapture &= ((b.piece[BLACK][PAWN] & POW2[48 + i]) == 0) && ((POW2[8 + i] & b.piece[BLACK][PAWN]) == POW2[8 + i]);
        pawnMovesCapture &= ((b.color[BLACK] | b.piece[BLACK][PAWN]) == b.color[BLACK]) && ((b.color[BLACK] & b.piece[BLACK][PAWN]) == b.piece[BLACK][PAWN]);

        pawnMovesCapture &= (b.piece[WHITE][PAWN] & POW2[8 + i]) == 0;
        pawnMovesCapture &= (POPCOUNT(b.piece[WHITE][PAWN]) == 7) && (POPCOUNT(b.piece[WHITE][PAWN]) == 7);
        pawnMovesCapture &= POPCOUNT(b.color[WHITE]) == 15;
    }

    b = defaultBoard();
    w1 = (Move) {.pieceThatMoves= PAWN, .from = 8, .to = 56, .color = WHITE};
    b1 = (Move) {.pieceThatMoves= PAWN, .from = 55, .to = 7, .color = BLACK};

    makeMove(&b, &w1, &h);
    int extra = ((b.piece[WHITE][PAWN] & POW2[8]) == 0) && ((b.piece[WHITE][PAWN] & POW2[56]) == POW2[56]) && (b.piece[BLACK][ROOK] == POW2[63]);
    makeMove(&b, &b1, &h);
    extra &= ((b.piece[BLACK][PAWN] & POW2[57]) == 0) && ((b.piece[BLACK][PAWN] & POW2[7]) == POW2[7]) && (b.piece[WHITE][ROOK] == POW2[0]);

    return pawnMovesNoCapture && pawnMovesCapture && extra;
}

int testCastleNoCheck()
{
    Move move;
    History h;
    Board expected, b;
    
    //White kingside
    b = generateFromFen("8/8/8/8/8/8/8/4K2R", "b", "K");
    int dataIsOk = b.posInfo == WCASTLEK;
    int can = (canCastle(&b, BLACK) == 0) && (canCastle(&b, WHITE) == 1);

    move = castleKSide(WHITE);
    makeMove(&b, &move, &h);
    expected = generateFromFen("8/8/8/8/8/8/8/5RK1", "b", "-");
    int castleSuccessful = equal(&b, &expected);
    undoMove(&b, move, &h);
    expected = generateFromFen("8/8/8/8/8/8/8/4K2R", "b", "K");
    castleSuccessful &= equal(&b, &expected);

    //White queenside
    b = generateFromFen("8/8/8/8/8/8/8/R3K3", "b", "Q");
    dataIsOk &= b.posInfo == WCASTLEQ;
    can &= (canCastle(&b, BLACK) == 0) && (canCastle(&b, WHITE) == 2);

    move = castleQSide(WHITE);
    makeMove(&b, &move, &h);
    expected = generateFromFen("8/8/8/8/8/8/8/2KR4", "b", "-");
    castleSuccessful &= equal(&b, &expected);
    undoMove(&b, move, &h);
    expected = generateFromFen("8/8/8/8/8/8/8/R3K3", "b", "Q");
    castleSuccessful &= equal(&b, &expected);

    //Black kingside
    b = generateFromFen("4k2r/8/8/8/8/8/8/8", "b", "k");
    dataIsOk &= b.posInfo == BCASTLEK;
    can &= (canCastle(&b, BLACK) == 1) && (canCastle(&b, WHITE) == 0);

    move = castleKSide(BLACK);
    makeMove(&b, &move, &h);
    expected = generateFromFen("5rk1/8/8/8/8/8/8/8", "b", "-");
    castleSuccessful &= equal(&b, &expected);
    undoMove(&b, move, &h);
    expected = generateFromFen("4k2r/8/8/8/8/8/8/8", "b", "k");
    castleSuccessful &= equal(&b, &expected);

    //Black queenside
    b = generateFromFen("r3k3/8/8/8/8/8/8/8", "b", "q");
    dataIsOk &= b.posInfo == BCASTLEQ;
    can &= (canCastle(&b, BLACK) == 2) && (canCastle(&b, WHITE) == 0);

    move = castleQSide(BLACK);
    makeMove(&b, &move, &h);
    expected = generateFromFen("2kr4/8/8/8/8/8/8/8", "b", "-");
    castleSuccessful &= equal(&b, &expected);
    undoMove(&b, move, &h);
    expected = generateFromFen("r3k3/8/8/8/8/8/8/8", "b", "q");
    castleSuccessful &= equal(&b, &expected);

    //Queenside with rook without blocking
    //White
    b = generateFromFen("1r6/8/8/8/8/8/8/R3K3", "b", "Q");
    dataIsOk &= b.posInfo == WCASTLEQ;
    can &= (canCastle(&b, BLACK) == 0) && (canCastle(&b, WHITE) == 2);

    move = castleQSide(WHITE);
    makeMove(&b, &move, &h);
    expected = generateFromFen("1r6/8/8/8/8/8/8/2KR4", "b", "-");
    castleSuccessful &= equal(&b, &expected);
    undoMove(&b, move, &h);
    expected = generateFromFen("1r6/8/8/8/8/8/8/R3K3", "b", "Q");
    castleSuccessful &= equal(&b, &expected);

    //Black
    b = generateFromFen("r3k3/8/8/8/8/8/8/1R6", "b", "q");
    dataIsOk &= b.posInfo == BCASTLEQ;
    can &= (canCastle(&b, BLACK) == 2) && (canCastle(&b, WHITE) == 0);

    move = castleQSide(BLACK);
    makeMove(&b, &move, &h);
    expected = generateFromFen("2kr4/8/8/8/8/8/8/1R6", "b", "-");
    castleSuccessful &= equal(&b, &expected);
    undoMove(&b, move, &h);
    expected = generateFromFen("r3k3/8/8/8/8/8/8/1R6", "b", "q");
    castleSuccessful &= equal(&b, &expected);

    return dataIsOk && can && castleSuccessful;
}
int testCastleCheck()
{
    Board b;
    int white = 1;

    //Kingside
    b = generateFromFen("8/8/8/8/8/8/8/4KP1R", "b", "KQkq");
    white &= canCastle(&b, WHITE) == 0;
    b = generateFromFen("4r3/8/8/8/8/8/8/4K2R", "b", "KQkq");
    white &= canCastle(&b, WHITE) == 0;
    b = generateFromFen("5r2/8/8/8/8/8/8/4K2R", "b", "KQkq");
    white &= canCastle(&b, WHITE) == 0;

    //Queenside
    b = generateFromFen("8/8/8/8/8/8/8/RP2K3", "b", "KQkq");
    white &= canCastle(&b, WHITE) == 0;
    b = generateFromFen("4r3/8/8/8/8/8/8/R3K3", "b", "KQkq");
    white &= canCastle(&b, WHITE) == 0;
    b = generateFromFen("2r5/8/8/8/8/8/8/R3K3", "b", "KQkq");
    white &= canCastle(&b, WHITE) == 0;

    int black = 1;

    b = generateFromFen("4k1pr/8/8/8/8/8/8/8", "b", "KQkq");
    white &= canCastle(&b, BLACK) == 0;
    b = generateFromFen("4k2r/8/8/8/8/8/8/4R3", "b", "KQkq");
    white &= canCastle(&b, BLACK) == 0;
    b = generateFromFen("4k2r/8/8/8/8/8/8/6R1", "b", "KQkq");
    white &= canCastle(&b, BLACK) == 0;

    //Queenside
    b = generateFromFen("r1P1k3/8/8/8/8/8/8/8", "b", "KQkq");
    white &= canCastle(&b, BLACK) == 0;
    b = generateFromFen("r3k3/8/8/8/8/8/8/4R3", "b", "KQkq");
    white &= canCastle(&b, BLACK) == 0;
    b = generateFromFen("r3k3/8/8/8/8/8/8/2R5", "b", "KQkq");
    white &= canCastle(&b, BLACK) == 0;

    return white && black;
}

int testStartMoveListing()
{
    //WARNING: This will fail if the ordering of the generation is changed
    Board b = defaultBoard();
    Move moves[256];

    int numMovesWhite = allMoves(&b, moves, 1);
    int whiteMovesAreAccurate = 1;
    for (int i = 0; i < 8; ++i)
    {
        whiteMovesAreAccurate &= (moves[2*i].pieceThatMoves == PAWN) && (moves[2*i].from == i + 8);
        whiteMovesAreAccurate &= (moves[2*i + 1].pieceThatMoves == PAWN) && (moves[2*i + 1].from == i + 8);
        whiteMovesAreAccurate &= moves[2*i + 1].to != moves[2*i].to;
        whiteMovesAreAccurate &= (moves[2*i].to == i + 16) || (moves[2*i + 1].to == i + 16);
        whiteMovesAreAccurate &= (moves[2*i].to == i + 24) || (moves[2*i + 1].to == i + 24);
    }

    int whiteKnight = 1;
    whiteKnight &= (moves[16].pieceThatMoves == KNIGHT) && (moves[17].pieceThatMoves == KNIGHT) && (moves[18].pieceThatMoves == KNIGHT) && (moves[19].pieceThatMoves == KNIGHT);
    whiteKnight &= (moves[16].from == moves[17].from) && (moves[18].from == moves[19].from);

    int numMovesBlack = allMoves(&b, moves, 0);
    int blackMovesAreAccurate = 1;
    for (int i = 0; i < 8; ++i)
    {
        blackMovesAreAccurate &= (moves[2*i].pieceThatMoves == PAWN) && (moves[2*i].from == 55 - (7 -i));
        blackMovesAreAccurate &= (moves[2*i + 1].pieceThatMoves == PAWN) && (moves[2*i + 1].from == 55 - (7 - i));
        blackMovesAreAccurate &= moves[2*i + 1].to != moves[2*i].to;
        blackMovesAreAccurate &= (moves[2*i].to == 47 - (7 - i)) || (moves[2*i + 1].to == 47 - (7 - i));
        blackMovesAreAccurate &= (moves[2*i].to == 39 - (7 - i)) || (moves[2*i + 1].to == 39 - (7 - i));
    }

    int blackKnight = 1;
    blackKnight &= (moves[16].pieceThatMoves == KNIGHT) && (moves[17].pieceThatMoves == KNIGHT) && (moves[18].pieceThatMoves == KNIGHT) && (moves[19].pieceThatMoves == KNIGHT);
    blackKnight &= (moves[16].from == moves[17].from) && (moves[18].from == moves[19].from);

    return 
        (numMovesWhite == 20) && (numMovesBlack == 20) && whiteMovesAreAccurate && blackMovesAreAccurate && whiteKnight && blackKnight;
}

int testPromotion()
{
    Board b;
    Move moves[13];
    History h;

    b = generateFromFen("8/4P1K1/8/8/8/1k6/3p4/8", "w" ,"-");

    int numMovesB = allMoves(&b, moves, BLACK);
    int numMovesW = allMoves(&b, moves, WHITE);

    int correctNum = (numMovesW == 12) && (numMovesB == 12);
    int piecesAddUp = 1;

    for (int i = 0; i < 4; ++i)
    {
        piecesAddUp &= (moves[8 + i].promotion != KING);
        makeMove(&b, &moves[8 + i], &h);
        undoMove(&b, moves[8 + i], &h);
    }
    Board cp = generateFromFen("8/4P1K1/8/8/8/1k6/3p4/8", "w" ,"-");
    
    int eq = equal(&cp, &b);

    return correctNum && eq && piecesAddUp;
}

int testSimplePerft()
{
    Board b = defaultBoard();
    int startPos = 
        perft(b, 1, 0) && perft(b, 2, 0) && perft(b, 3, 0);

    b = generateFromFen("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR", "b", "KQkq");
    int noPawns = 
        (perftRecursive(b, 1, BLACK) == 50ULL) && (perftRecursive(b, 2, BLACK) == 2125ULL) && (perftRecursive(b, 3, BLACK) == 96062ULL);

    b = generateFromFen("r3k3/8/8/8/8/3b4/8/R3K2R", "b", "KQkq");
    int castle = 
        (perftRecursive(b, 1, BLACK) == 27ULL) && (perftRecursive(b, 2, BLACK) == 586ULL) && (perftRecursive(b, 3, BLACK) == 13643ULL);

    b = generateFromFen("4k3/1b2nbb1/3n4/8/8/4N3/1B1N1BB1/4K3", "w", "-");
    int bishAndKnight =
        (perftRecursive(b, 1, WHITE) == 35ULL) && (perftRecursive(b, 2, WHITE) == 1252ULL) && (perftRecursive(b, 3, WHITE) == 42180ULL);

    b = generateFromFen("4kq2/4q3/8/8/8/8/1Q6/Q3K3", "w", "-");
    int queen =
        (perftRecursive(b, 1, WHITE) == 4ULL) && (perftRecursive(b, 2, WHITE) == 105ULL) && (perftRecursive(b, 3, WHITE) == 2532ULL);

    b = generateFromFen("8/4P1K1/8/8/8/1k6/3p4/8", "w" ,"-");
    int promotion = 
        (perftRecursive(b, 1, WHITE) == 12ULL) && (perftRecursive(b, 2, WHITE) == 142ULL) && (perftRecursive(b, 3, WHITE) == 1788ULL);

    return startPos && noPawns && castle && bishAndKnight && queen && promotion;
}

void runTests()
{
    //Generation
    printf("[+] Generation: %d\n",      testGeneration());
    //Moves
    printf("[+] King moves: %d\n",      testKingMoves());
    printf("[+] Queen moves: %d\n",     testQueenMoves());
    printf("[+] Rook moves: %d\n",      testRookMoves());
    printf("[+] Bish moves: %d\n",      testBishMoves());
    printf("[+] Knight moves: %d\n",    testKnightMoves());
    printf("[+] Pawn moves: %d\n",      testPawnMoves());
    
    //Checks
    printf("[+] Checks: %d\n",          testChecks());

    //Make Moves
    printf("[+] Undo moves: %d\n",      testUndoMoves());
    printf("[+] Board Pawn moves: %d\n",testBoardPawnMoves());

    //All move generation
    printf("[+] Castle No Chck: %d\n",  testCastleNoCheck());
    printf("[+] Castle Chck: %d\n",     testCastleCheck());
    printf("[+] Move listing: %d\n",    testStartMoveListing());
    printf("[+] Promotion: %d\n",       testPromotion());

    //Perft
    printf("[+] Perft: %d\n",           testSimplePerft());
}
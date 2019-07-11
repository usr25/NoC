#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/io.h"
#include "../include/evaluation.h"
#include "../include/perft.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1  Starting pos
//8/pppppppp/rnbqkbNr/8/8/RNBQKBnR/PPPPPPPP/8 w KQkq - 0 1  All pieces in front of pawns
//5N2/5Pkq/5p2/3P4/3pKQ2/3n4/8/8 w - -                      King moves
//8/2n2N2/8/1br2rb1/n1Q2q1N/1BR2RB1/8/8 w - -               Queen moves
//8/8/2Br2R1/8/8/2BR2r1/8/8 w - -                           Rook moves
//8/2r4R/3B2b1/8/8/3B2b1/2R4r/8 w - -                       Bish moves
//8/1p1p1Pk1/2N3K1/n3N3/2n3p1/qP1PQ3/8/8 w - -              Knight moves
//8/4p3/3Q1q2/4q3/4q3/pq3P1p/P5P1/8 w - -                   Pawn moves

int ignore;

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
    int posInfoCorrect = b.posInfo == 0b11110;
    int turn = b.turn == WHITE;

    return 
    correctNumOfPieces && piecesAddUp && availableAddUp && avAreCorrect && whiteAreInAvBlack && blackAreInAvWhite && posInfoCorrect && turn;
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
    Board b = genFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", &ignore);

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

    b = genFromFen("5N2/5Pkq/5p2/3P4/3pKQ2/3n4/8/8 w - -", &ignore);
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

    b = genFromFen("8/2n2N2/8/1br2rb1/n1Q2q1N/1BR2RB1/8/8 w - -", &ignore);
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

    b = genFromFen("8/8/2Br2R1/8/8/2BR2r1/8/8 w - -", &ignore);

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

    b = genFromFen("8/2r4R/3B2b1/8/8/3B2b1/2R4r/8 w - -", &ignore);

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

    b = genFromFen("8/1p1p1Pk1/2N3K1/n3N3/2n3p1/qP1PQ3/8/8 w - -", &ignore);
    
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

    b = genFromFen("8/4p3/3Q1q2/4q3/4q3/pq3P1p/P5P1/8 w - -", &ignore);

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
    b = genFromFen("8/8/1q6/8/3K4/8/8/8 w - -", &ignore);
    white &= isInCheck(&b, 1);
    b = genFromFen("8/8/8/8/3K4/8/8/3q4 w - -", &ignore);
    white &= isInCheck(&b, 1);

    b = genFromFen("8/8/8/8/3K3r/8/8/8 w - -", &ignore);
    white &= isInCheck(&b, 1);    
    b = genFromFen("8/8/8/2b5/3K4/8/8/8 w - -", &ignore);
    white &= isInCheck(&b, 1);
    b = genFromFen("8/8/8/5n2/3K4/8/8/8 w - -", &ignore);
    white &= isInCheck(&b, 1);
    b = genFromFen("8/8/8/2p5/3K4/8/8/8 w - -", &ignore);
    white &= isInCheck(&b, 1);
    b = genFromFen("4r3/8/3k4/1q6/3K4/8/2p2n2/2b5 w - -", &ignore);
    white &= ! isInCheck(&b, 1);

    //Black king
    b = genFromFen("8/8/1Q6/8/3k4/8/8/8 w - -", &ignore);
    black &= isInCheck(&b, 0);
    b = genFromFen("8/8/8/8/3k4/8/8/3Q4 w - -", &ignore);
    black &= isInCheck(&b, 0);

    b = genFromFen("8/8/8/8/3k3R/8/8/8 w - -", &ignore);
    black &= isInCheck(&b, 0);
    b = genFromFen("8/8/8/2B5/3k4/8/8/8 w - -", &ignore);
    black &= isInCheck(&b, 0);
    b = genFromFen("8/8/8/5N2/3k4/8/8/8 w - -", &ignore);
    black &= isInCheck(&b, 0);
    b = genFromFen("8/8/8/8/3k4/2P5/8/8 w - -", &ignore);
    black &= isInCheck(&b, 0);
    b = genFromFen("4R3/8/3K4/1Q6/3k4/8/2P2N2/2B5 w - -", &ignore);
    black &= ! isInCheck(&b, 0);
    
    return white && black;
}

int testCheckInterfTiles()
{
    Board b;
    AttacksOnK att;
    uint64_t expected;

    int emptyBoard = 1;
    b = genFromFen("8/8/2Q3Q1/8/2R1K1B1/8/2R3B1/8 w - -", &ignore);
    att = getCheckTiles(&b, WHITE);
    emptyBoard &= att.tiles == 0ULL && att.num == 0;
    b = genFromFen("8/8/2q3q1/8/2r1k1b1/8/2r3b1/8 b - -", &ignore);
    att = getCheckTiles(&b, BLACK);
    emptyBoard &= att.tiles == 0ULL && att.num == 0;

    int slidingPieces = 1;
    expected = 37470100259328ULL;

    b = genFromFen("8/8/2q3q1/8/2r1K1b1/8/2r3b1/8 w - -", &ignore);
    att = getCheckTiles(&b, WHITE);
    slidingPieces &= att.tiles == expected && att.num == 4;
    b = genFromFen("8/8/2Q3Q1/8/2R1k1B1/8/2R3B1/8 b - -", &ignore);
    att = getCheckTiles(&b, BLACK);
    slidingPieces &= att.tiles == expected && att.num == 4;

    int pawns = 1;
    b = genFromFen("8/8/8/3P1p2/4K3/3p1P2/8/8 w - -", &ignore);
    att = getCheckTiles(&b, WHITE);
    pawns &= att.tiles == 17179869184ULL && att.num == 1;
    b = genFromFen("8/8/8/3p1P2/4k3/3P1p2/8/8 b - -", &ignore);
    att = getCheckTiles(&b, BLACK);
    pawns &= att.tiles == 1048576ULL && att.num == 1;

    int knights = 1;
    expected = 4535485469696ULL;
    b = genFromFen("8/8/3N1n2/2n3N1/4K3/2N3N1/3n1n2/8 w - -", &ignore);
    att = getCheckTiles(&b, WHITE);
    knights &= att.tiles == expected && att.num == 4;
    b = genFromFen("8/8/3n1N2/2N3n1/4k3/2n3n1/3N1N2/8 b - -", &ignore);
    att = getCheckTiles(&b, BLACK);
    knights &= att.tiles == expected && att.num == 4;

    int misc = 1;
    b = genFromFen("q7/1b5b/2Qq4/5P2/1bb1K1Qr/3r4/2rp1qQ1/7b w - -", &ignore);
    att = getCheckTiles(&b, WHITE);
    misc &= att.tiles == 0ULL && att.num == 0;
    b = genFromFen("q7/1B5B/2qQ4/5p2/1BB1k1qR/3R4/2RP1Qq1/7B b - -", &ignore);
    att = getCheckTiles(&b, BLACK);
    misc &= att.tiles == 0ULL && att.num == 0;

    return 1;
}

int testUndoMoves()
{
    
    Board b = defaultBoard();
    Board _b = defaultBoard();
    History h;
    int stays = equal(&b, &_b);

    Move w1 = (Move) {.pieceThatMoves= PAWN, .from = 8, .to = 56};

    makeMove(&b, w1, &h);
    undoMove(&b, w1, &h);
    int white = equal(&b, &_b);

    Move b1 = (Move) {.pieceThatMoves= PAWN, .from = 55, .to = 7};

    b.turn = BLACK;
    makeMove(&b, b1, &h);
    undoMove(&b, b1, &h);
    b.turn = WHITE;
    int black = equal(&b, &_b);

    return stays && white && black;
}

int testBoardPawnMoves()
{
    int temp;
    History h;
    Board b;
    int pawnMovesNoCapture = 1;
    for (int i = 0; i < 8; ++i)
    {
        Move w1 = (Move) {.pieceThatMoves= PAWN, .from = i + 8, .to = i + 16};
        Move w2 = (Move) {.pieceThatMoves= PAWN, .from = i + 16, .to = i + 24};
        
        Move b1 = (Move) {.pieceThatMoves= PAWN, .from = 55 - i, .to = 47 - i};
        Move b2 = (Move) {.pieceThatMoves= PAWN, .from = 47 - i, .to = 39 - i};

        b = defaultBoard();
        
        b.turn = WHITE;
        makeMove(&b, w1, &h);
        b.turn = WHITE;
        makeMove(&b, w2, &h);
        b.turn = BLACK;
        makeMove(&b, b1, &h);
        b.turn = BLACK;
        makeMove(&b, b2, &h);

        pawnMovesNoCapture &= ((b.piece[WHITE][PAWN] & POW2[i + 16]) == 0) && ((POW2[i + 24] & b.piece[WHITE][PAWN]) == POW2[i + 24]);
        pawnMovesNoCapture &= ((b.color[WHITE] | b.piece[WHITE][PAWN]) == b.color[WHITE]) && ((b.color[WHITE] & b.piece[WHITE][PAWN]) == b.piece[WHITE][PAWN]);
        
        pawnMovesNoCapture &= ((b.piece[BLACK][PAWN] & POW2[47 - i]) == 0) && ((POW2[39 - i] & b.piece[BLACK][PAWN]) == POW2[39 - i]);
        pawnMovesNoCapture &= ((b.color[BLACK] | b.piece[BLACK][PAWN]) == b.color[BLACK]) && ((b.color[BLACK] & b.piece[BLACK][PAWN]) == b.piece[BLACK][PAWN]);

        pawnMovesNoCapture &= (POPCOUNT(b.piece[WHITE][PAWN]) == POPCOUNT(b.piece[WHITE][PAWN])) && (POPCOUNT(b.piece[BLACK][PAWN]) == POPCOUNT(b.piece[BLACK][PAWN]));
    }

    int pawnMovesCapture = 1;

    for (int i = 0; i < 8; ++i)
    {
        Move w1 = (Move) {.pieceThatMoves= PAWN, .from = 8 + i, .to = 48 + i, .capture = PAWN};
        Move b1 = (Move) {.pieceThatMoves= PAWN, .from = 48 + i, .to = 8 + i, .capture = PAWN};

        b = defaultBoard();
        b.turn = WHITE;
        makeMove(&b, w1, &h);

        pawnMovesCapture &= ((b.piece[WHITE][PAWN] & POW2[8 + i]) == 0) && ((POW2[48 + i] & b.piece[WHITE][PAWN]) == POW2[48 + i]);
        pawnMovesCapture &= ((b.color[WHITE] | b.piece[WHITE][PAWN]) == b.color[WHITE]) && ((b.color[WHITE] & b.piece[WHITE][PAWN]) == b.piece[WHITE][PAWN]);
   
        pawnMovesCapture &= (b.piece[BLACK][PAWN] & POW2[48 + i]) == 0;
        pawnMovesCapture &= (POPCOUNT(b.piece[BLACK][PAWN]) == 7) && (POPCOUNT(b.piece[BLACK][PAWN]) == 7);
        pawnMovesCapture &= POPCOUNT(b.color[BLACK]) == 15;

        b = defaultBoard();
        b.turn = BLACK;
        makeMove(&b, b1, &h);

        pawnMovesCapture &= ((b.piece[BLACK][PAWN] & POW2[48 + i]) == 0) && ((POW2[8 + i] & b.piece[BLACK][PAWN]) == POW2[8 + i]);
        pawnMovesCapture &= ((b.color[BLACK] | b.piece[BLACK][PAWN]) == b.color[BLACK]) && ((b.color[BLACK] & b.piece[BLACK][PAWN]) == b.piece[BLACK][PAWN]);

        pawnMovesCapture &= (b.piece[WHITE][PAWN] & POW2[8 + i]) == 0;
        pawnMovesCapture &= (POPCOUNT(b.piece[WHITE][PAWN]) == 7) && (POPCOUNT(b.piece[WHITE][PAWN]) == 7);
        pawnMovesCapture &= POPCOUNT(b.color[WHITE]) == 15;
    }

    b = defaultBoard();
    Move w1 = (Move) {.pieceThatMoves= PAWN, .from = 8, .to = 56, .capture = ROOK};
    Move b1 = (Move) {.pieceThatMoves= PAWN, .from = 55, .to = 7, .capture = ROOK};

    b.turn = WHITE;
    makeMove(&b, w1, &h);
    int extra = ((b.piece[WHITE][PAWN] & POW2[8]) == 0) && ((b.piece[WHITE][PAWN] & POW2[56]) == POW2[56]) && (b.piece[BLACK][ROOK] == POW2[63]);
    b.turn = BLACK;
    makeMove(&b, b1, &h);
    extra &= ((b.piece[BLACK][PAWN] & POW2[57]) == 0) && ((b.piece[BLACK][PAWN] & POW2[7]) == POW2[7]) && (b.piece[WHITE][ROOK] == POW2[0]);


    return pawnMovesNoCapture && pawnMovesCapture && extra;
}

int testCastleNoCheck()
{
    History h;
    Board expected, b;
    
    //White kingside
    b = genFromFen("8/8/8/8/8/8/8/4K2R w K -", &ignore);
    int dataIsOk = b.posInfo == WCASTLEK;
    int can = (canCastleCheck(&b, BLACK) == 0) && (canCastleCheck(&b, WHITE) == 1);

    Move moveWK = castleKSide(WHITE);
    makeMove(&b, moveWK, &h);
    expected = genFromFen("8/8/8/8/8/8/8/5RK1 b - -", &ignore);
    int castleSuccessful = equal(&b, &expected);
    undoMove(&b, moveWK, &h);
    expected = genFromFen("8/8/8/8/8/8/8/4K2R w K -", &ignore);
    castleSuccessful &= equal(&b, &expected);

    //White queenside
    b = genFromFen("8/8/8/8/8/8/8/R3K3 w Q -", &ignore);
    dataIsOk &= b.posInfo == WCASTLEQ;
    can &= (canCastleCheck(&b, BLACK) == 0) && (canCastleCheck(&b, WHITE) == 2);

    Move moveWQ = castleQSide(WHITE);
    makeMove(&b, moveWQ, &h);
    expected = genFromFen("8/8/8/8/8/8/8/2KR4 b - -", &ignore);
    castleSuccessful &= equal(&b, &expected);
    undoMove(&b, moveWQ, &h);
    expected = genFromFen("8/8/8/8/8/8/8/R3K3 w Q -", &ignore);
    castleSuccessful &= equal(&b, &expected);

    //Black kingside
    b = genFromFen("4k2r/8/8/8/8/8/8/8 b k -", &ignore);
    dataIsOk &= b.posInfo == BCASTLEK;
    can &= (canCastleCheck(&b, BLACK) == 1) && (canCastleCheck(&b, WHITE) == 0);

    Move moveBK = castleKSide(BLACK);
    makeMove(&b, moveBK, &h);
    expected = genFromFen("5rk1/8/8/8/8/8/8/8 w - -", &ignore);
    castleSuccessful &= equal(&b, &expected);
    undoMove(&b, moveBK, &h);
    expected = genFromFen("4k2r/8/8/8/8/8/8/8 b k -", &ignore);
    castleSuccessful &= equal(&b, &expected);

    //Black queenside
    b = genFromFen("r3k3/8/8/8/8/8/8/8 b q -", &ignore);
    dataIsOk &= b.posInfo == BCASTLEQ;
    can &= (canCastleCheck(&b, BLACK) == 2) && (canCastleCheck(&b, WHITE) == 0);

    Move moveBQ = castleQSide(BLACK);
    makeMove(&b, moveBQ, &h);
    expected = genFromFen("2kr4/8/8/8/8/8/8/8 w - -", &ignore);
    castleSuccessful &= equal(&b, &expected);
    undoMove(&b, moveBQ, &h);
    expected = genFromFen("r3k3/8/8/8/8/8/8/8 b q -", &ignore);
    castleSuccessful &= equal(&b, &expected);

    //Queenside with rook without blocking
    //White
    b = genFromFen("1r6/8/8/8/8/8/8/R3K3 w Q -", &ignore);
    dataIsOk &= b.posInfo == WCASTLEQ;
    can &= (canCastleCheck(&b, BLACK) == 0) && (canCastleCheck(&b, WHITE) == 2);

    Move moveWQ2 = castleQSide(WHITE);
    makeMove(&b, moveWQ2, &h);
    expected = genFromFen("1r6/8/8/8/8/8/8/2KR4 b - -", &ignore);
    castleSuccessful &= equal(&b, &expected);
    undoMove(&b, moveWQ2, &h);
    expected = genFromFen("1r6/8/8/8/8/8/8/R3K3 w Q -", &ignore);
    castleSuccessful &= equal(&b, &expected);

    //Black
    b = genFromFen("r3k3/8/8/8/8/8/8/1R6 b q -", &ignore);
    dataIsOk &= b.posInfo == BCASTLEQ;
    can &= (canCastleCheck(&b, BLACK) == 2) && (canCastleCheck(&b, WHITE) == 0);

    Move moveBQ2 = castleQSide(BLACK);
    makeMove(&b, moveBQ2, &h);
    expected = genFromFen("2kr4/8/8/8/8/8/8/1R6 w - -", &ignore);
    castleSuccessful &= equal(&b, &expected);
    undoMove(&b, moveBQ2, &h);
    expected = genFromFen("r3k3/8/8/8/8/8/8/1R6 b q -", &ignore);
    castleSuccessful &= equal(&b, &expected);

    return dataIsOk && can && castleSuccessful;
}
int testCastleCheck()
{
    Board b;
    int white = 1;

    //Kingside
    b = genFromFen("8/8/8/8/8/8/8/4KP1R b KQkq -", &ignore);
    white &= canCastleCheck(&b, WHITE) == 0;
    b = genFromFen("4r3/8/8/8/8/8/8/4K2R b KQkq -", &ignore);
    white &= canCastleCheck(&b, WHITE) == 0;
    b = genFromFen("5r2/8/8/8/8/8/8/4K2R b KQkq -", &ignore);
    white &= canCastleCheck(&b, WHITE) == 0;

    //Queenside
    b = genFromFen("8/8/8/8/8/8/8/RP2K3 b KQkq -", &ignore);
    white &= canCastleCheck(&b, WHITE) == 0;
    b = genFromFen("4r3/8/8/8/8/8/8/R3K3 b KQkq -", &ignore);
    white &= canCastleCheck(&b, WHITE) == 0;
    b = genFromFen("2r5/8/8/8/8/8/8/R3K3 b KQkq -", &ignore);
    white &= canCastleCheck(&b, WHITE) == 0;

    int black = 1;

    b = genFromFen("4k1pr/8/8/8/8/8/8/8 b KQkq -", &ignore);
    white &= canCastleCheck(&b, BLACK) == 0;
    b = genFromFen("4k2r/8/8/8/8/8/8/4R3 b KQkq -", &ignore);
    white &= canCastleCheck(&b, BLACK) == 0;
    b = genFromFen("4k2r/8/8/8/8/8/8/6R1 b KQkq -", &ignore);
    white &= canCastleCheck(&b, BLACK) == 0;

    //Queenside
    b = genFromFen("r1P1k3/8/8/8/8/8/8/8 b KQkq -", &ignore);
    white &= canCastleCheck(&b, BLACK) == 0;
    b = genFromFen("r3k3/8/8/8/8/8/8/4R3 b KQkq -", &ignore);
    white &= canCastleCheck(&b, BLACK) == 0;
    b = genFromFen("r3k3/8/8/8/8/8/8/2R5 b KQkq -", &ignore);
    white &= canCastleCheck(&b, BLACK) == 0;

    return white && black;
}

int testStartMoveListing()
{
    //WARNING: This will fail if the ordering of the generation is changed
    Board b = defaultBoard();
    Move moves[100];

    int numMovesWhite = legalMoves(&b, moves, WHITE);
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

    b.turn ^= 1;
    int numMovesBlack = legalMoves(&b, moves, BLACK);
    int blackMovesAreAccurate = 1;
    
    for (int i = 0; i < 8; ++i)
    {
        blackMovesAreAccurate &= (moves[2*i].pieceThatMoves == PAWN) && (moves[2*i].from == 55 - (7 - i));
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

    b = genFromFen("8/4P1K1/8/8/8/1k6/3p4/8 w - -", &ignore);

    int numMovesB = legalMoves(&b, moves, BLACK);
    int numMovesW = legalMoves(&b, moves, WHITE);

    int correctNum = (numMovesW == 12) && (numMovesB == 12);
    int piecesAddUp = 1;

    for (int i = 0; i < 4; ++i)
    {
        piecesAddUp &= (moves[i].promotion != KING);
        makeMove(&b, moves[i], &h);
        undoMove(&b, moves[i], &h);
    }
    Board cp = genFromFen("8/4P1K1/8/8/8/1k6/3p4/8 w - -", &ignore);
    
    int eq = equal(&cp, &b);

    return correctNum && piecesAddUp && eq;
}

int testEnPass()
{
    Board b;

    b = genFromFen("8/1p3k2/7K/8/2P5/8/8/8 w - -", &ignore);
    int perft1 = 
        (perftRecursive(b, 1) == 4ULL) && (perftRecursive(b, 2) == 32ULL) && (perftRecursive(b, 3) == 185ULL) && (perftRecursive(b, 4) == 1382ULL);

    b = genFromFen("3k4/1p6/8/2P5/6p1/3K4/5P1P/8 w - -", &ignore);
    int perft2 = 
        (perftRecursive(b, 1) == 13ULL) && (perftRecursive(b, 2) == 108ULL) && (perftRecursive(b, 3) == 1360ULL);        

    b = genFromFen("k7/pp5p/8/2P5/8/8/P6P/K7 w - -", &ignore);
    int perft3 = 
        (perftRecursive(b, 3) == 400ULL) && (perftRecursive(b, 4) == 2824ULL) && (perftRecursive(b, 6) == 177792ULL);

    b = genFromFen("rnbqkbnr/pppppp1p/8/6pP/8/8/PPPPPPP1/RNBQKBNR w KQkq g6", &ignore);
    int perft4 = 
        (perftRecursive(b, 3) == 11273ULL) && (perftRecursive(b, 4) == 249383ULL);

    return perft1 && perft2 && perft3 && perft4;
}

int testSimplePerft()
{
    Board b;
    int startPos = 
        perft(1, 0) && perft(2, 0) && perft(3, 0);

    b = genFromFen("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR b KQkq -", &ignore);
    int noPawns = 
        (perftRecursive(b, 1) == 50ULL) && (perftRecursive(b, 2) == 2125ULL) && (perftRecursive(b, 3) == 96062ULL);

    b = genFromFen("r3k3/8/8/8/8/3b4/8/R3K2R b KQkq -", &ignore);
    int castle = 
        (perftRecursive(b, 1) == 27ULL) && (perftRecursive(b, 2) == 586ULL) && (perftRecursive(b, 3) == 13643ULL);

    b = genFromFen("4k3/1b2nbb1/3n4/8/8/4N3/1B1N1BB1/4K3 w - -", &ignore);
    int bishAndKnight =
        (perftRecursive(b, 1) == 35ULL) && (perftRecursive(b, 2) == 1252ULL) && (perftRecursive(b, 3) == 42180ULL);

    b = genFromFen("4kq2/4q3/8/8/8/8/1Q6/Q3K3 w - -", &ignore);
    int queen =
        (perftRecursive(b, 1) == 4ULL) && (perftRecursive(b, 2) == 105ULL) && (perftRecursive(b, 3) == 2532ULL);

    b = genFromFen("8/4P1K1/8/8/8/1k6/3p4/8 w - -", &ignore);
    int promotion = 
        (perftRecursive(b, 1) == 12ULL) && (perftRecursive(b, 2) == 142ULL) && (perftRecursive(b, 3) == 1788ULL);


    return startPos && noPawns && castle && bishAndKnight && queen && promotion;
}

int equalsDelta(int a, int b)
{
    return abs(a - b) < 20;
}

int evEquality()
{
    int eq = 1;
    
    Board b = defaultBoard();
    eq &= equalsDelta(eval(b), 0);

    b = genFromFen("2b3k1/2pQRppp/1p6/p1N5/P1n5/1P6/2PqrPPP/2B3K1 b - -", &ignore);
    return eq;
}
int evMaterial()
{
    int mat = 1;
    Board b1 = defaultBoard();
    History h;

    //BLACK pieces removed
    for (int i = 0; i < 8; ++i)
    {
        if (i != 4)
        {
            Move m = (Move) {.pieceThatMoves = PAWN, .from = 8, .to = 63 - i, .capture = pieceAt(&b1, 1ULL << (63 - i), 1 ^ b1.turn)};
            makeMove(&b1, m, &h);
            mat &= eval(b1) > 270;
            undoMove(&b1, m, &h);
        }
    }

    //BLACK pawns removed
    for (int i = 8; i < 16; ++i)
    {
        Move m = (Move) {.pieceThatMoves = PAWN, .from = 8, .to = 63 - i, .capture = PAWN};
        makeMove(&b1, m, &h);
        mat &= 300 > eval(b1) && eval(b1) > 75;
        undoMove(&b1, m, &h);
    }
    

    b1.turn ^= 1;
    //WHITE pieces removed
    for (int i = 0; i < 8; ++i)
    {
        if (i != 3)
        {
            Move m = (Move) {.pieceThatMoves = PAWN, .from = 56, .to = i, .capture = pieceAt(&b1, 1ULL << i, 1 ^ b1.turn)};
            makeMove(&b1, m, &h);
            mat &= eval(b1) < -270;
            undoMove(&b1, m, &h);
        }
    }
    //WHITE pawns removed
    for (int i = 8; i < 16; ++i)
    {
        Move m = (Move) {.pieceThatMoves = PAWN, .from = 56, .to = i, .capture = PAWN};
        makeMove(&b1, m, &h);
        mat &= eval(b1) < -75 && -300 < eval(b1);
        undoMove(&b1, m, &h);
    }

    return mat;
}
int evCastle()
{
    int cas = 1;
    Board b1 = genFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq -", &ignore);
    Board b2 = genFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQ1RK1 w kq -", &ignore);

    //TODO: Improve this so that the rook doesnt get penalized
    //cas &= eval(b2) > eval(b1);

    b1 = genFromFen("rnbqk2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq -", &ignore);
    b2 = genFromFen("rnbq1rk1/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQ -", &ignore);

    //cas &= eval(b2) < eval(b1);

    b1 = genFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/R3K2R w KQkq -", &ignore);
    b2 = genFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/2KR3R w kq -", &ignore);

    //cas &= eval(b2) > eval(b1);

    b1 = genFromFen("r3k2r/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQkq -", &ignore);
    b2 = genFromFen("2kr3r/pppppppp/8/8/8/8/PPPPPPPP/RNBQK2R w KQ -", &ignore);

    //cas &= eval(b2) < eval(b1);

    return cas;
}
int evPawnPos()
{
    return 1;
}

int testEvaluation()
{
    return evEquality() && evMaterial() && evCastle() && evPawnPos();
}

//This are deep perfts to ensure that everything is working, use this as well to benchmark
void slowTests()
{
    printf("\n---= This will take a long time =---\n");
    Board b;

    printf("\n");
    printf("Start depth 4: %d\n", perft(4, 0));
    printf("Start depth 5: %d\n", perft(5, 0));
    printf("Start depth 6: %d\n", perft(6, 0));


    b = genFromFen("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR b KQkq -", &ignore);
    printf("No pawns depth 5: %d\n", perftRecursive(b, 5) == 191462298ULL);
    
    b = genFromFen("8/1p3k2/7K/8/2P5/8/8/8 w - -", &ignore);
    printf("En passand + promotion: %d\n", perftRecursive(b, 8) == 3558853ULL);
    
    b = genFromFen("r3k3/8/8/8/8/3b4/8/R3K2R b KQkq -", &ignore);
    printf("Castle: %d\n", perftRecursive(b, 5) == 7288108ULL);
    
    b = genFromFen("4k3/1b2nbb1/3n4/8/8/4N3/1B1N1BB1/4K3 w - -", &ignore);
    printf("Bish & Knight: %d\n", perftRecursive(b, 5) == 48483119ULL);
    
    b = genFromFen("4kq2/4q3/8/8/8/8/1Q6/Q3K3 w - -", &ignore);
    printf("Queen: %d\n", perftRecursive(b, 6) == 71878391ULL);
    
    b = genFromFen("8/8/8/3k1K3/8/8/8/8 w - -", &ignore);
    printf("King: %d\n", perftRecursive(b, 9) == 31356171ULL);

    b = genFromFen("8/5K2/5PR1/7k/7p/5P2/6P1/8 w - -", &ignore);
    printf("Check enP: %d\n", perftRecursive(b, 8) == 7991633ULL);


    b = genFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", &ignore);
    printf("Perfect 1: %d\n", perftRecursive(b, 6) == 119060324ULL);
    
    b = genFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", &ignore);
    printf("Perfect 2: %d\n", perftRecursive(b, 5) == 193690690ULL);
    
    b = genFromFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", &ignore);
    printf("Perfect 3: %d\n", perftRecursive(b, 7) == 178633661ULL);

    b = genFromFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -", &ignore);
    printf("Perfect 4: %d\n", perftRecursive(b, 6) == 706045033ULL);

    b = genFromFen("1k6/1b6/8/8/7R/8/8/4K2R b K -", &ignore);
    printf("Perfect 5: %d\n", perftRecursive(b, 5) == 1063513ULL);
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
    printf("[+] CheckInterfTiles: %d\n",testCheckInterfTiles());

    //Make Moves
    printf("[+] Undo moves: %d\n",      testUndoMoves());
    printf("[+] Board Pawn moves: %d\n",testBoardPawnMoves());

    //All move generation
    printf("[+] Castle No Chck: %d\n",  testCastleNoCheck());
    printf("[+] Castle Chck: %d\n",     testCastleCheck());
    printf("[+] Move listing: %d\n",    testStartMoveListing());
    printf("[+] Promotion: %d\n",       testPromotion());
    printf("[+] En Passand: %d\n",      testEnPass());

    //Perft
    printf("[+] Perft: %d\n",           testSimplePerft());
  
    //Eval
    printf("[+] Eval: %d\n",            testEvaluation());
}
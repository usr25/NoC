#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/io.h"

#include <stdio.h>

int validStartingPos(Board b)
{
    int correctNumOfPieces = 
        POPCOUNT(b.wPawn) == 8 &&
        POPCOUNT(b.bPawn) == 8 &&
        POPCOUNT(b.wKing) == 1 &&
        POPCOUNT(b.bKing) == 1 &&
        POPCOUNT(b.wQueen) == 1 &&
        POPCOUNT(b.bQueen) == 1 &&
        POPCOUNT(b.wRook) == 2 &&
        POPCOUNT(b.bRook) == 2 &&
        POPCOUNT(b.wBish) == 2 &&
        POPCOUNT(b.bBish) == 2 &&
        POPCOUNT(b.wKnight) == 2 &&
        POPCOUNT(b.bKnight) == 2;

    int countIsCorrect = 
        POPCOUNT(b.wPawn) == numWPawn(b.numPieces) &&
        POPCOUNT(b.bPawn) == numBPawn(b.numPieces) &&
        POPCOUNT(b.wKing) == numWKing(b.numPieces) &&
        POPCOUNT(b.bKing) == numBKing(b.numPieces) &&
        POPCOUNT(b.wQueen) == numWQueen(b.numPieces) &&
        POPCOUNT(b.bQueen) == numBQueen(b.numPieces) &&
        POPCOUNT(b.wRook) == numWRook(b.numPieces) &&
        POPCOUNT(b.bRook) == numBRook(b.numPieces) &&
        POPCOUNT(b.wBish) == numWBish(b.numPieces) &&
        POPCOUNT(b.bBish) == numBBish(b.numPieces) &&
        POPCOUNT(b.wKnight) == numWKnight(b.numPieces) &&
        POPCOUNT(b.bKnight) == numBKnight(b.numPieces);

    int piecesAddUp = POPCOUNT(b.white) == 16 && POPCOUNT(b.black) == 16;
    int availableAddUp = POPCOUNT(b.avWhite) == 48 && POPCOUNT(b.avBlack) == 48;
    int avAreCorrect = (b.white & b.avWhite) == 0 && (b.black & b.avBlack) == 0;
    int whiteAreInAvBlack = (b.white & b.avBlack) == b.white;
    int blackAreInAvWhite = (b.black & b.avWhite) == b.black;
    int posInfoCorrect = b.posInfo == 0b11111;

    return 
    correctNumOfPieces && piecesAddUp && availableAddUp && avAreCorrect && whiteAreInAvBlack && blackAreInAvWhite && posInfoCorrect;
}

int validPieces(Board b)
{
    int validKings = (POPCOUNT(b.wKing) == 1) && (POPCOUNT(b.bKing) == 1);
    int totNumPieces = (POPCOUNT(b.white) <= 16) && (POPCOUNT(b.black) <= 16);
    int validNumPawns = (POPCOUNT(b.wPawn) <= 8) && (POPCOUNT(b.bPawn) <= 8);

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
    movsW = posQueenMoves(&b, 1, 0);
    movsB = posQueenMoves(&b, 0, 0);

    int defaultMovs = 
        (movsW == 0ULL) && (movsB == 0ULL);

    b = generateFromFen("8/2n2N2/8/1br2rb1/n1Q2q1N/1BR2RB1/8/8", "w", "-");
    movsW = posQueenMoves(&b, 1, 0);
    movsB = posQueenMoves(&b, 0, 0);

    int otherMovs = 
        (movsW == 9280821397508ULL) && (movsB == 17627536560160ULL);

    return defaultMovs && otherMovs;
}
int testRookMoves()
{
    uint64_t movsW1, movsW2, movsB1, movsB2;
    Board b = defaultBoard();
    movsW1 = posRookMoves(&b, 1, 0);
    movsW2 = posRookMoves(&b, 1, 1);
    movsB1 = posRookMoves(&b, 0, 0);
    movsB2 = posRookMoves(&b, 0, 1);

    int defaultMovs = 
        (movsW1 == 0ULL) && (movsW2 == 0ULL) && (movsB1 == 0ULL) && (movsB2 == 0ULL);

    b = generateFromFen("8/8/2Br2R1/8/8/2BR2r1/8/8", "w", "-");
    movsW1 = posRookMoves(&b, 1, 0);
    movsW2 = posRookMoves(&b, 1, 1);
    movsB1 = posRookMoves(&b, 0, 0);
    movsB2 = posRookMoves(&b, 0, 1);

    int otherMovs = 
        (movsW1 == 17661174878224ULL) && (movsW2 == 144710032490102784ULL) && (movsB1 == 2207648645634ULL) && (movsB2 == 1157475750758055936ULL);

    return defaultMovs && otherMovs;
}
int testBishMoves()
{
    uint64_t movsW1, movsW2, movsB1, movsB2;
    Board b = defaultBoard();
    movsW1 = posBishMoves(&b, 1, 0);
    movsW2 = posBishMoves(&b, 1, 1);
    movsB1 = posBishMoves(&b, 0, 0);
    movsB2 = posBishMoves(&b, 0, 1);

    int defaultMovs = 
        (movsW1 == 0ULL) && (movsW2 == 0ULL) && (movsB1 == 0ULL) && (movsB2 == 0ULL);

    b = generateFromFen("8/2r4R/3B2b1/8/8/3B2b1/2R4r/8", "w", "-");
    movsW1 = posBishMoves(&b, 1, 0);
    movsW2 = posBishMoves(&b, 1, 1);
    movsB1 = posBishMoves(&b, 0, 0);
    movsB2 = posBishMoves(&b, 0, 1);

    int otherMovs = 
        (movsW1 == 143229240477700ULL) && (movsW2 == 299489548168200192ULL) && (movsB1 == 17626629669896ULL) && (movsB2 == 577868148797079552ULL);

    return defaultMovs && otherMovs;
}
int testKnightMoves()
{
    uint64_t movsW1, movsW2, movsB1, movsB2;
    Board b = defaultBoard();
    movsW1 = posKnightMoves(&b, 1, 0);
    movsW2 = posKnightMoves(&b, 1, 1);
    movsB1 = posKnightMoves(&b, 0, 0);
    movsB2 = posKnightMoves(&b, 0, 1);

    int defaultMovs = 
        (movsW1 == 327680ULL) && (movsW2 == 10485760ULL) && (movsB1 == 5497558138880ULL) && (movsB2 == 175921860444160ULL);

    b = generateFromFen("8/1p1p1Pk1/2N3K1/n3N3/2n3p1/qP1PQ3/8/8", "w", "-");
    movsW1 = posKnightMoves(&b, 1, 0);
    movsW2 = posKnightMoves(&b, 1, 1);
    movsB1 = posKnightMoves(&b, 0, 0);
    movsB2 = posKnightMoves(&b, 0, 1);

    int otherMovs = 
        (movsW1 == 4503600198057984ULL) && (movsW2 == 5802888670964875264ULL) && (movsB1 == 87995290505216ULL) && (movsB2 == 35184376283136ULL);

    return defaultMovs && otherMovs;
}

int testPawnMoves()
{
    uint64_t movsW1, movsW2, movsW3, movsB1, movsB2, movsB3;
    Board b = defaultBoard();
    int defaultMovs = 1;
    for (int i = 0; i < 8 && defaultMovs; ++i)
    {
        defaultMovs &= posPawnMoves(&b, 1, i) == (POW2[i + 8] | POW2[i]) << 16;
        defaultMovs &= posPawnMoves(&b, 0, i) == (POW2[i] | POW2[i + 8]) << 32;
    }

    b = generateFromFen("8/4p3/3Q1q2/4q3/4q3/pq3P1p/P5P1/8", "w", "-");

    movsW1 = posPawnMoves(&b, 1, 0);
    movsW2 = posPawnMoves(&b, 1, 1);
    movsW3 = posPawnMoves(&b, 1, 2);
    movsB1 = posPawnMoves(&b, 0, 0);
    movsB2 = posPawnMoves(&b, 0, 1);
    movsB3 = posPawnMoves(&b, 0, 2);

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
    white &= isInCheck(&b, 1) == 0;

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
    black &= isInCheck(&b, 0) == 0;
    
    return white && black;
}

int testUndoMoves()
{
    
    Board b = defaultBoard();
    Board _b = defaultBoard();
    int stays = equal(&b, &_b);

    int temp; //TODO: Remove this
    Move w1 = (Move) {.pieceThatMoves= PAWN, .from = 8, .to = 56, .color = 1};

    makeMoveWhite(&b, &w1, &temp);
    undoMoveWhite(&b, &w1, &temp);
    int white = equal(&b, &_b);

    Move b1 = (Move) {.pieceThatMoves= PAWN, .from = 55, .to = 7, .color = 0};

    makeMoveBlack(&b, &b1, &temp);
    undoMoveBlack(&b, &b1, &temp);
    int black = equal(&b, &_b);

    return stays && white && black;
}

int testBoardPawnMoves()
{
    int temp; //TODO: implement history
    Board b;
    Move w1, w2, b1, b2;
    int pawnMovesNoCapture = 1;
    for (int i = 0; i < 8; ++i)
    {
        w1 = (Move) {.pieceThatMoves= PAWN, .from = i + 8, .to = i + 16, .color = 1};
        w2 = (Move) {.pieceThatMoves= PAWN, .from = i + 16, .to = i + 24, .color = 1};
        
        b1 = (Move) {.pieceThatMoves= PAWN, .from = 55 - i, .to = 47 - i, .color = 0};
        b2 = (Move) {.pieceThatMoves= PAWN, .from = 47 - i, .to = 39 - i, .color = 0};

        b = defaultBoard();
        
        makeMoveWhite(&b, &w1, &temp);
        makeMoveWhite(&b, &w2, &temp);
        makeMoveBlack(&b, &b1, &temp);
        makeMoveBlack(&b, &b2, &temp);

        pawnMovesNoCapture &= ((b.wPawn & POW2[i + 16]) == 0) && ((POW2[i + 24] & b.wPawn) == POW2[i + 24]);
        pawnMovesNoCapture &= ((b.white | b.wPawn) == b.white) && ((b.white & b.wPawn) == b.wPawn);
        
        pawnMovesNoCapture &= ((b.bPawn & POW2[47 - i]) == 0) && ((POW2[39 - i] & b.bPawn) == POW2[39 - i]);
        pawnMovesNoCapture &= ((b.black | b.bPawn) == b.black) && ((b.black & b.bPawn) == b.bPawn);

        pawnMovesNoCapture &= (POPCOUNT(b.wPawn) == numWPawn(b.numPieces)) && (POPCOUNT(b.bPawn) == numBPawn(b.numPieces));
    }

    int pawnMovesCapture = 1;

    for (int i = 0; i < 8; ++i)
    {
        w1 = (Move) {.pieceThatMoves= PAWN, .from = i + 8, .to = i + 48, .color = 1};
        b1 = (Move) {.pieceThatMoves= PAWN, .from = i + 48, .to = i + 8, .color = 0};

        b = defaultBoard();
        makeMoveWhite(&b, &w1, &temp);

        pawnMovesCapture &= ((b.wPawn & POW2[8 + i]) == 0) && ((POW2[48 + i] & b.wPawn) == POW2[48 + i]);
        pawnMovesCapture &= ((b.white | b.wPawn) == b.white) && ((b.white & b.wPawn) == b.wPawn);

        pawnMovesCapture &= (b.bPawn & POW2[48 + i]) == 0;
        pawnMovesCapture &= (POPCOUNT(b.bPawn) == 7) && (numBPawn(b.numPieces) == 7);
        pawnMovesCapture &= POPCOUNT(b.black) == 15;
    
        b = defaultBoard();
        makeMoveBlack(&b, &b1, &temp);

        pawnMovesCapture &= ((b.bPawn & POW2[48 + i]) == 0) && ((POW2[8 + i] & b.bPawn) == POW2[8 + i]);
        pawnMovesCapture &= ((b.black | b.bPawn) == b.black) && ((b.black & b.bPawn) == b.bPawn);

        pawnMovesCapture &= (b.wPawn & POW2[8 + i]) == 0;
        pawnMovesCapture &= (POPCOUNT(b.wPawn) == 7) && (numWPawn(b.numPieces) == 7);
        pawnMovesCapture &= POPCOUNT(b.white) == 15;
    }

    b = defaultBoard();
    w1 = (Move) {.pieceThatMoves= PAWN, .from = 8, .to = 56, .color = 1};
    b1 = (Move) {.pieceThatMoves= PAWN, .from = 55, .to = 7, .color = 0};

    makeMoveWhite(&b, &w1, &temp);
    int extra = ((b.wPawn & POW2[8]) == 0) && ((b.wPawn & POW2[56]) == POW2[56]) && (b.bRook == POW2[63]);
    makeMoveBlack(&b, &b1, &temp);
    extra &= ((b.bPawn & POW2[57]) == 0) && ((b.bPawn & POW2[7]) == POW2[7]) && (b.wRook == POW2[0]);

    return pawnMovesNoCapture && pawnMovesCapture && extra;
}

int testStartMoveListing()
{
    //This may fail if the ordering of the generation is changed
    Board b = defaultBoard();
    Move moves[256];

    int sum = 1152;

    int numMovesWhite = allMovesWhite(&b, moves, 0);
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

    int numMovesBlack = allMovesBlack(&b, moves, 0);
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
    printf("[+] Move listing: %d\n",    testStartMoveListing());
}
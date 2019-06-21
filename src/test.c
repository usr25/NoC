#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/io.h"
#include "../include/perft.h"

#include <stdio.h>

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
    int stays = equal(&b, &_b);

    int temp; //TODO: Remove this
    Move w1 = (Move) {.pieceThatMoves= PAWN, .from = 8, .to = 56, .color = 1};

    makeMove(&b, &w1, 1);
    undoMove(&b, w1, 1);
    int white = equal(&b, &_b);

    Move b1 = (Move) {.pieceThatMoves= PAWN, .from = 55, .to = 7, .color = 0};

    makeMove(&b, &b1, 0);
    undoMove(&b, b1, 0);
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
        
        makeMove(&b, &w1, 1);
        makeMove(&b, &w2, 1);
        makeMove(&b, &b1, 0);
        makeMove(&b, &b2, 0);

        pawnMovesNoCapture &= ((b.piece[WHITE][PAWN] & POW2[i + 16]) == 0) && ((POW2[i + 24] & b.piece[WHITE][PAWN]) == POW2[i + 24]);
        pawnMovesNoCapture &= ((b.color[WHITE] | b.piece[WHITE][PAWN]) == b.color[WHITE]) && ((b.color[WHITE] & b.piece[WHITE][PAWN]) == b.piece[WHITE][PAWN]);
        
        pawnMovesNoCapture &= ((b.piece[BLACK][PAWN] & POW2[47 - i]) == 0) && ((POW2[39 - i] & b.piece[BLACK][PAWN]) == POW2[39 - i]);
        pawnMovesNoCapture &= ((b.color[BLACK] | b.piece[BLACK][PAWN]) == b.color[BLACK]) && ((b.color[BLACK] & b.piece[BLACK][PAWN]) == b.piece[BLACK][PAWN]);

        pawnMovesNoCapture &= (POPCOUNT(b.piece[WHITE][PAWN]) == POPCOUNT(b.piece[WHITE][PAWN])) && (POPCOUNT(b.piece[BLACK][PAWN]) == POPCOUNT(b.piece[BLACK][PAWN]));
    }

    int pawnMovesCapture = 1;

    for (int i = 0; i < 8; ++i)
    {
        w1 = (Move) {.pieceThatMoves= PAWN, .from = 8 + i, .to = 48 + i, .color = 1};
        b1 = (Move) {.pieceThatMoves= PAWN, .from = 48 + i, .to = 8 + i, .color = 0};

        b = defaultBoard();
        makeMove(&b, &w1, 1);

        pawnMovesCapture &= ((b.piece[WHITE][PAWN] & POW2[8 + i]) == 0) && ((POW2[48 + i] & b.piece[WHITE][PAWN]) == POW2[48 + i]);
        pawnMovesCapture &= ((b.color[WHITE] | b.piece[WHITE][PAWN]) == b.color[WHITE]) && ((b.color[WHITE] & b.piece[WHITE][PAWN]) == b.piece[WHITE][PAWN]);
   
        pawnMovesCapture &= (b.piece[BLACK][PAWN] & POW2[48 + i]) == 0;
        pawnMovesCapture &= (POPCOUNT(b.piece[BLACK][PAWN]) == 7) && (POPCOUNT(b.piece[BLACK][PAWN]) == 7);
        pawnMovesCapture &= POPCOUNT(b.color[BLACK]) == 15;

        b = defaultBoard();
        makeMove(&b, &b1, 0);

        pawnMovesCapture &= ((b.piece[BLACK][PAWN] & POW2[48 + i]) == 0) && ((POW2[8 + i] & b.piece[BLACK][PAWN]) == POW2[8 + i]);
        pawnMovesCapture &= ((b.color[BLACK] | b.piece[BLACK][PAWN]) == b.color[BLACK]) && ((b.color[BLACK] & b.piece[BLACK][PAWN]) == b.piece[BLACK][PAWN]);

        pawnMovesCapture &= (b.piece[WHITE][PAWN] & POW2[8 + i]) == 0;
        pawnMovesCapture &= (POPCOUNT(b.piece[WHITE][PAWN]) == 7) && (POPCOUNT(b.piece[WHITE][PAWN]) == 7);
        pawnMovesCapture &= POPCOUNT(b.color[WHITE]) == 15;
    }

    b = defaultBoard();
    w1 = (Move) {.pieceThatMoves= PAWN, .from = 8, .to = 56, .color = 1};
    b1 = (Move) {.pieceThatMoves= PAWN, .from = 55, .to = 7, .color = 0};

    makeMove(&b, &w1, 1);
    int extra = ((b.piece[WHITE][PAWN] & POW2[8]) == 0) && ((b.piece[WHITE][PAWN] & POW2[56]) == POW2[56]) && (b.piece[BLACK][ROOK] == POW2[63]);
    makeMove(&b, &b1, 0);
    extra &= ((b.piece[BLACK][PAWN] & POW2[57]) == 0) && ((b.piece[BLACK][PAWN] & POW2[7]) == POW2[7]) && (b.piece[WHITE][ROOK] == POW2[0]);

    return pawnMovesNoCapture && pawnMovesCapture && extra;
}

int testSimplePerft()
{
    Board b = defaultBoard();
    int startPos = 
        perft(b, 1, 0) && perft(b, 2, 0) && perft(b, 3, 0);

    b = generateFromFen("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR", "w", "KQkq");
    int other = 
        (perftRecursive(b, 1, 0) == 50ULL) && (perftRecursive(b, 2, 0) == 2125ULL) && (perftRecursive(b, 3, 0) == 96062ULL);

    return startPos && other;
}

int testStartMoveListing()
{
    
    //This may fail if the ordering of the generation is changed
    Board b = defaultBoard();
    Move moves[256];

    int sum = 1152; //TODO: Implement this

    int numMovesWhite = allMoves(&b, moves, 0ULL, 1);
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

    int numMovesBlack = allMoves(&b, moves, 0ULL, 0);
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

    //Perft
    printf("[+] Perft: %d\n",           testSimplePerft());
}
/* test.c
 * File which holds some tests to quickly ensure everything is working
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
//TODO: Add support #include <assert.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/allmoves.h"
#include "../include/io.h"
#include "../include/evaluation.h"
#include "../include/hash.h"
#include "../include/search.h"
#include "../include/perft.h"

#define PATH "/home/j/Desktop/Chess/Engine/positions.fen"

//rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1  Starting pos
//8/pppppppp/rnbqkbNr/8/8/RNBQKBnR/PPPPPPPP/8 w KQkq - 0 1  All pieces in front of pawns
//5N2/5Pkq/5p2/3P4/3pKQ2/3n4/8/8 w - -                      King moves
//8/2n2N2/8/1br2rb1/n1Q2q1N/1BR2RB1/8/8 w - -               Queen moves
//8/8/2Br2R1/8/8/2BR2r1/8/8 w - -                           Rook moves
//8/2r4R/3B2b1/8/8/3B2b1/2R4r/8 w - -                       Bish moves
//8/1p1p1Pk1/2N3K1/n3N3/2n3p1/qP1PQ3/8/8 w - -              Knight moves
//8/4p3/3Q1q2/4q3/4q3/pq3P1p/P5P1/8 w - -                   Pawn moves

//8/5k2/P6K/8/8/8/4p3/8 w - -                               2x faster than sf10 at depth 9(!)

int ignore;

int compMove(char* fen, char* target, int depth, int len)
{
    char mv[6] = "";
    Board b = genFromFen(fen, &ignore);
    //drawPosition(b, 0);
    Move best = bestTime(b, 0, (Repetition){.index = 0}, depth);

    moveToText(best, mv);
    //printf("%s\n", mv);
    return strncmp(mv, target, len) == 0;
}

int testHashingInPos(char* fen)
{
    int a;
    Board b = genFromFen(fen, &a);
    Move list[NMOVES];
    History h;

    const int numMoves = legalMoves(&b, list) >> 1;

    uint64_t initialHash = hashPosition(&b);

    int works = 1;
    for (int i = 0; i < numMoves; ++i)
    {
        makeMove(&b, list[i], &h);
        const uint64_t updateHash = makeMoveHash(initialHash, &b, list[i], h);
        const uint64_t correcHash = hashPosition(&b);
        undoMove(&b, list[i], &h);

        works &= updateHash == correcHash;
    }

    return works;
}

int testHashing()
{
    int updating = 1;

    updating &= testHashingInPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0");
    updating &= testHashingInPos("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq - 0");

    updating &= testHashingInPos("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR w KQkq -");
    updating &= testHashingInPos("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR b KQkq -");

    updating &= testHashingInPos("8/1p3k2/7K/8/2P5/8/8/8 w - -");
    updating &= testHashingInPos("8/1p3k2/7K/8/2P5/8/8/8 b - -");

    updating &= testHashingInPos("8/5K2/5PR1/7k/7p/5P2/6P1/8 w - -");
    updating &= testHashingInPos("8/5K2/5PR1/7k/7p/5P2/6P1/8 b - -");

    updating &= testHashingInPos("r3k3/8/8/8/8/3b4/8/R3K2R w KQkq -");
    updating &= testHashingInPos("r3k3/8/8/8/8/3b4/8/R3K2R b KQkq -");

    int a;
    Board b = genFromFen("2k5/6p1/q7/7P/7p/8/Q5P1/3K4 w - -", &a);
    Move m = (Move) {.piece = PAWN, .from = 9, .to = 25};
    History h;

    uint64_t start = hashPosition(&b);
    makeMove(&b, m, &h);
    uint64_t after = makeMoveHash(start, &b, m, h);


    b = genFromFen("2k5/6p1/q7/7P/6Pp/8/Q7/3K4 b - -", &a);
    int enPass = hashPosition(&b) != after && after != start;

    return updating && enPass;
}

int testRookMate()
{
    int white = 1, black = 1;
    int depth = 3;

    white &= compMove("8/3R4/8/1k6/5K2/8/8/8 w - -", "d7c7", depth, 4);
    white &= compMove("8/3R4/1k6/8/5K2/8/8/8 w - -", "d7d1", depth, 4);
    white &= compMove("8/2R5/1k6/8/5K2/8/8/8 w - -", "c7c1", depth, 4);
    white &= compMove("8/8/1k6/8/5K2/8/8/2R5 w - -", "f4e5", depth, 4);
    white &= compMove("8/8/1k1K4/8/8/8/8/2R5 w - -", "c1b1", depth, 4);
    white &= compMove("8/8/1k1K4/2R5/8/8/8/8 w - -", "c5c1", depth, 4);
    white &= compMove("6R1/8/8/5K2/7k/8/8/8 w - -", "g8g7", depth, 4);
    white &= compMove("8/8/8/5K2/7k/8/8/6R1 w - -", "g1g8", depth, 4);
    white &= compMove("3R4/8/8/8/8/2k5/4K3/8 w - -", "d8d7", depth, 4);
    white &= compMove("3R4/8/8/8/8/8/4K3/2k5 w - -", "d8c8", depth, 4);

    black &= compMove("8/3r4/8/1K6/5k2/8/8/8 b - -", "d7c7", depth, 4);
    black &= compMove("8/3r4/1K6/8/5k2/8/8/8 b - -", "d7d1", depth, 4);
    black &= compMove("8/2r5/1K6/8/5k2/8/8/8 b - -", "c7c1", depth, 4);
    black &= compMove("8/8/1K6/8/5k2/8/8/2r5 b - -", "f4e5", depth, 4);
    black &= compMove("8/8/1K1k4/8/8/8/8/2r5 b - -", "c1b1", depth, 4);
    black &= compMove("8/8/1K1k4/2r5/8/8/8/8 b - -", "c5c1", depth, 4);
    black &= compMove("6r1/8/8/5k2/7K/8/8/8 b - -", "g8g7", depth, 4);
    black &= compMove("8/8/8/5k2/7K/8/8/6r1 b - -", "g1g8", depth, 4);
    black &= compMove("3r4/8/8/8/8/2K5/4k3/8 b - -", "d8d7", depth, 4);
    black &= compMove("3r4/8/8/8/8/8/4k3/2K5 b - -", "d8c8", depth, 4);

    return white && black;
}

void clear(char* str)
{
    for (int i = 0; i < 256; ++i)
        str[i] = '\0';
}
int upTo(FILE* fp, char* buf, char target)
{
    clear(buf);
    int i = 0;
    char ch = ' ';

    while (ch != EOF){
        ch = fgetc(fp);
        if (ch == target)
            break;
        else
            buf[i] = ch;
        i++;
    }

    return (ch == EOF);
}

void parseFensFromFilePerft(void)
{
    FILE* fp = fopen(PATH, "r");
    if (fp == NULL)
    {
        printf("[-] Cant open file %s\n", PATH);
        return;
    }

    Board b;
    uint64_t tot = 0;
    char buff[256] = "";
    int hasEnded = 0;

    int depth, perft, cnt = 0;
    //fen,depth,perft(TODO:,move)
    //Dont put spaces, except in the fen
    while (!hasEnded)
    {
        hasEnded = upTo(fp, buff, ',');
        if (hasEnded) break;
        b = genFromFen(buff, &ignore);
        hasEnded = upTo(fp, buff, ',');
        depth = atoi(buff);
        hasEnded = upTo(fp, buff, '\n');
        perft = atoi(buff);

        uint64_t temp = perftRecursive(b, depth);
        tot += temp;
        if (temp != perft)
            printf("Error at %d\n", cnt + 1);
        if (cnt % 10 == 0)
            printf("%d\n", cnt);
        cnt++;
    }

    fclose(fp);
    printf("Evaluated %d positions and %llu nodes\n", cnt, tot);
}
void parseFensFromFileEva(void)
{
    FILE* fp = fopen(PATH, "r");
    if (fp == NULL)
    {
        printf("[-] Cant open file %s\n", PATH);
        return;
    }

    Board b;
    char buff[256] = "";
    int hasEnded = 0, depth = 10, cnt = 0;

    //fen,depth,perft(TODO:,move)
    //Dont put spaces, except in the fen
    while (!hasEnded)
    {
        hasEnded = upTo(fp, buff, ',');
        if (hasEnded) break;
        b = genFromFen(buff, &ignore);
        hasEnded = upTo(fp, buff, '\n');

        Repetition rep = (Repetition) {.index = 0};
        drawMove(bestTime(b, 0, rep, depth));
        printf("\n");
        if (cnt % 10 == 0)
            printf("cnt: %d\n", cnt);
        cnt++;
    }

    fclose(fp);
}

void slowEval(void)
{
    printf("\n---= This will take a long time =---\n");

    int depth = 8;
    int white = 1, black = 1;
    Board b;

    printf("[+] Eval equal position: ");
    b = defaultBoard();
    drawMove(bestTime(b, 0, (Repetition){}, depth));
    printf(" ");
    b.turn ^= 1;
    drawMove(bestTime(b, 0, (Repetition){}, depth));
    printf("\n");

    depth = 10;

    white &= compMove("5b2/7p/3p2bk/2p2pN1/2P2P2/P1QPqB1P/7K/8 w - -", "g5f7", depth, 4); //Knight sac to mate

    white &= compMove("k7/pp6/8/4Q3/8/2r5/K7/2q5 w - -", "e5b8", depth, 4);      //Queen sac to draw
    black &= compMove("2Q5/k7/2R5/8/4q3/8/PP6/K7 b - -", "e4b1", depth, 4);

    white &= compMove("k7/pp6/8/5Q2/8/8/K7/2q5 w - -", "f5f8", depth, 4);        //Queen mate
    black &= compMove("k7/2Q5/8/8/5q2/8/PP6/K7 b - -", "f4f1", depth, 4);

    white &= compMove("k1q5/pp6/8/3K4/2N5/Q7/8/8 w - -", "c4b6", depth, 4);      //Knight fork pin
    black &= compMove("k7/8/8/r1n5/8/8/PP6/K1Q5 b - -", "c5b3", depth, 4);

    white &= compMove("8/pp3r1k/8/3K2P1/2N5/8/6R1/8 w - -", "g5g6", depth, 4);   //Pawn fork
    black &= compMove("6r1/8/8/3k2p1/2N5/5R1K/8/8 b - -", "g5g4", depth, 4);

    white &= compMove("5k2/2P2q2/8/8/4R3/5K2/8/8 w - -", "e4f4", depth, 4);      //Pins
    black &= compMove("5k2/4r3/8/8/5Q2/5K2/2p5/8 b - -", "e7f7", depth, 4);

    white &= compMove("8/k1P5/2K5/8/8/8/8/8 w - -", "c7c8r", depth, 4);          //Rook promotion
    black &= compMove("8/8/8/8/8/2k5/K1p5/8 b - -", "c2c1r", depth, 4);

    white &= compMove("1q4k1/2pN3R/8/6B1/5K2/8/2p5/4q3 w - -", "d7f6", depth, 4);//Mate
    black &= compMove("1Q4Q1/2P5/8/8/1b3k2/8/r3n3/1K6 b - -", "e2c3", depth, 4);

    white &= compMove("5Q2/7k/1K6/5pP1/4B1b1/8/8/8 w - f6", "g5f6", depth, 4);   //EnPass mate
    black &= compMove("5Q2/5p1k/1K6/6P1/4B1b1/8/8/8 b - -", "f7f5", depth, 4);

    black &= compMove("8/2r5/K7/2q4k/5p2/8/8/8 b - -", "c7a7", depth, 4);
    white &= compMove("8/2R5/k7/2Q4K/5P2/8/8/8 w - -", "c7a7", depth, 4);

    black &= ! compMove("r1bq1rk1/p1pnppbp/1pnp2p1/8/3PP3/3Q1NPP/PPP2PB1/RNB2RK1 b - -", "c6d4", depth, 4);
    black &= ! compMove("7r/p3k3/1p2p2n/2p2p1R/2P5/4K2B/7P/8 b - -", "h8g8", depth, 4);

    white &= compMove("6k1/5pp1/6p1/8/7Q/2r5/K7/2r5 w - -", "h4d8", depth, 4); //Repetition draw
    black &= compMove("2R5/k7/2R5/7q/8/6P1/5PP1/6K1 b -", "h5d1", depth, 4);

    //Need depth >= 7
    //white &= compMove("rnbqkbnr/pp2pppp/4P3/2pp4/3N4/8/PPPP1PPP/RNBQKB1R w KQkq -", "f1b5", depth, 4);
    //black &= compMove("r1bqk2r/pp3ppp/2n2n2/3pp1B1/1b6/1BNP4/PPP1NPPP/R2QK2R b KQkq -", "d5d4", depth, 4);

    depth = 10;

    white &= compMove("8/ppp5/8/PPP5/8/8/5K1k/8 w - -", "b5b6", depth, 4); //Pawn breaks
    black &= compMove("7K/5k2/8/8/ppp5/8/PPP5/8 b - -", "b4b3", depth, 4);

    //white &= compMove("3r1nk1/1b1R4/8/8/3B4/4K3/8/8 w - -", "d7g7", depth, 4); //Windmill

    printf("[+] White Eval: %d\n", white);
    printf("[+] Black Eval: %d\n", black);
}

//This are deep perfts to ensure that everything is working, use this as well to benchmark
void slowTests(void)
{
    printf("\n---= This will take a long time =---\n");
    Board b;

    printf("\n");
    printf("Start depth 4: %d\n", perft(defaultBoard(), 4, 0) == 197281ULL);
    printf("Start depth 5: %d\n", perft(defaultBoard(), 5, 0) == 4865609ULL);
    printf("Start depth 6: %d\n", perft(defaultBoard(), 6, 0) == 119060324ULL);


    b = genFromFen("rnbqkbnr/8/8/8/8/8/8/RNBQKBNR b KQkq -", &ignore);
    printf("No pawns depth 5: %d\n", perft(b, 5, 0) == 191462298ULL);

    b = genFromFen("8/1p3k2/7K/8/2P5/8/8/8 w - -", &ignore);
    printf("En passand + promotion: %d\n", perft(b, 8, 0) == 3558853ULL);

    b = genFromFen("8/5k2/P4P1K/8/8/8/4p3/8 w - -", &ignore);
    printf("Promotion: %d\n", perft(b, 8, 0) == 38555705ULL);

    b = genFromFen("r3k3/8/8/8/8/3b4/8/R3K2R b KQkq -", &ignore);
    printf("Castle: %d\n", perft(b, 5, 0) == 7288108ULL);

    b = genFromFen("4k3/1b2nbb1/3n4/8/8/4N3/1B1N1BB1/4K3 w - -", &ignore);
    printf("Bish & Knight: %d\n", perft(b, 5, 0) == 48483119ULL);

    b = genFromFen("4kq2/4q3/8/8/8/8/1Q6/Q3K3 w - -", &ignore);
    printf("Queen: %d\n", perft(b, 6, 0) == 71878391ULL);

    b = genFromFen("8/8/8/3k1K3/8/8/8/8 w - -", &ignore);
    printf("King: %d\n", perft(b, 9, 0) == 31356171ULL);

    b = genFromFen("8/5K2/5PR1/7k/7p/5P2/6P1/8 w - -", &ignore);
    printf("Check enP: %d\n", perft(b, 8, 0) == 7991633ULL);


    b = genFromFen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -", &ignore);
    printf("Perfect 1: %d\n", perft(b, 6, 0) == 119060324ULL);

    b = genFromFen("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -", &ignore);
    printf("Perfect 2: %d\n", perft(b, 5, 0) == 193690690ULL);

    b = genFromFen("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", &ignore);
    printf("Perfect 3: %d\n", perft(b, 7, 0) == 178633661ULL);

    b = genFromFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -", &ignore);
    printf("Perfect 4: %d\n", perft(b, 6, 0) == 706045033ULL);

    b = genFromFen("1k6/1b6/8/8/7R/8/8/4K2R b K -", &ignore);
    printf("Perfect 5: %d\n", perft(b, 5, 0) == 1063513ULL);

    b = genFromFen("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq -", &ignore);
    printf("Complex: %d\n", perft(b, 6, 0) == 706045033ULL);
}

void runTests(void)
{
    //Rook mate
    printf("[+] Rook Mate: %d\n",       testRookMate());
    //Hash
    printf("[+] Hash: %d\n",            testHashing());
}

void testGav(void)
{
    int pawn = 1, rook = 1, queen = 1;
    pawn &= compMove("8/k1P5/2K5/8/8/8/8/8 w - -", "c7c8r", 1, 5);
    pawn &= compMove("3k4/8/4P3/4K3/8/8/8/8 w - -", "e5d6", 1, 5);
    pawn &= compMove("8/8/8/5k2/8/8/4PK2/8 w - -", "f2f3", 1, 5);
    pawn &= compMove("8/8/4k3/8/4PK2/8/8/8 b - -", "e6f6", 1, 5); //To draw
    pawn &= compMove("8/8/8/8/8/6k1/P1K5/8 b - -", "g3f4", 1, 5); //To draw

    queen &= compMove("1Q6/8/2K5/8/k7/8/8/8 w - -", "b8b2", 1, 4);
    queen &= compMove("K7/8/2k5/8/1q6/8/8/8 b - -", "b4b7", 1, 4); //Gives checkmate
    queen &= compMove("8/8/2K5/8/8/1Q6/k7/8 b - -", "a2b3", 1, 4); //To draw
    queen &= compMove("1K6/8/3Q4/8/8/4k3/8/8 b - -", "e3e4", 1, 4); //Go for longest distance to mate

    rook &= compMove("8/8/8/8/8/6r1/2k5/K7 b - -", "g3a3", 1, 4); //Gives checkmate
    rook &= compMove("8/8/8/8/2K5/k7/1R6/8 w - -", "b2h2", 1, 4); //May fail if ordering changes
    rook &= compMove("7K/8/8/8/3k4/8/4R3/8 b - -", "d4d3", 1, 4); //Go for longest distance to mate
    rook &= compMove("8/8/8/3k4/5r2/4K3/8/8 w - -", "e3f4", 1, 4); //To draw

    printf("Pawn  TB: %d\n", pawn);
    printf("Rook  TB: %d\n", rook);
    printf("Queen TB: %d\n", queen);
}

void chooseTest(int mode)
{
    switch (mode)
    {
        case 0:
            runTests();
            break;
        case 1:
            slowTests();
            break;
        case 2:
            slowEval();
            break;
        case 3:
            parseFensFromFilePerft();
            break;
        case 4:
            parseFensFromFileEva();
            break;
        case 5:
            testGav();
            break;
        default:
            printf("Choose mode [0..5]\n");
    }
}
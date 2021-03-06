void initMemo(void);

extern uint64_t kingMoves[64];
extern uint64_t knightMoves[64];

extern uint64_t whitePawnMoves[64];
extern uint64_t whitePawnCaptures[64];
extern uint64_t blackPawnMoves[64];
extern uint64_t blackPawnCaptures[64];

/* ...Int are the intersection tiles, since it isnt really necessary to calculate the intersection
 * against a piece that is on the edge because the possible sqrs will stay the same
 */
extern uint64_t rightMoves[64];        extern uint64_t rightMovesInt[64];
extern uint64_t leftMoves[64];         extern uint64_t leftMovesInt[64];
extern uint64_t upMoves[64];           extern uint64_t upMovesInt[64];
extern uint64_t downMoves[64];         extern uint64_t downMovesInt[64];
extern uint64_t uprightMoves[64];      extern uint64_t uprightMovesInt[64];
extern uint64_t downrightMoves[64];    extern uint64_t downrightMovesInt[64];
extern uint64_t upleftMoves[64];       extern uint64_t upleftMovesInt[64];
extern uint64_t downleftMoves[64];     extern uint64_t downleftMovesInt[64];

extern uint64_t straMoves[64];         extern uint64_t straMovesInt[64];
extern uint64_t diagMoves[64];         extern uint64_t diagMovesInt[64];

extern uint64_t vert[8];
extern uint64_t horiz[8];

extern uint64_t pawnLanes[8];
extern uint64_t wPassedPawn[64];
extern uint64_t bPassedPawn[64];

extern uint64_t king2[64];

#define getKingMoves(i) kingMoves[i]
#define getKnightMoves(i) knightMoves[i]

#define getRightMoves(i) rightMoves[i]
#define getLeftMoves(i) leftMoves[i]
#define getUpMoves(i) upMoves[i]
#define getDownMoves(i) downMoves[i]
#define getUpRightMoves(i) uprightMoves[i]
#define getUpLeftMoves(i) upleftMoves[i]
#define getDownRightMoves(i) downrightMoves[i]
#define getDownLeftMoves(i) downleftMoves[i]

#define getRightMovesInt(i) rightMovesInt[i]
#define getLeftMovesInt(i) leftMovesInt[i]
#define getUpMovesInt(i) upMovesInt[i]
#define getDownMovesInt(i) downMovesInt[i]
#define getUpRightMovesInt(i) uprightMovesInt[i]
#define getUpLeftMovesInt(i) upleftMovesInt[i]
#define getDownRightMovesInt(i) downrightMovesInt[i]
#define getDownLeftMovesInt(i) downleftMovesInt[i]

#define getStraMoves(i) straMoves[i]
#define getDiagMoves(i) diagMoves[i]

#define getStraInt(i) straMovesInt[i]
#define getDiagInt(i) diagMovesInt[i]

#define getWhitePawnMoves(i) whitePawnMoves[i]
#define getWhitePawnCaptures(i) whitePawnCaptures[i]
#define getBlackPawnMoves(i) blackPawnMoves[i]
#define getBlackPawnCaptures(i) blackPawnCaptures[i]

#define getVert(i) vert[i]
#define getHoriz(i) horiz[i]

#define getPawnLanes(i) pawnLanes[i]
#define getWPassedPawn(i) wPassedPawn[i]
#define getBPassedPawn(i) bPassedPawn[i]

#define getKing2(i) king2[i]
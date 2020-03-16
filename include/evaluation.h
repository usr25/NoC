void initEval(void);
__attribute__((hot)) int eval(const Board* b);
int insuffMat(const Board* b);

extern int V_QUEEN[2];
extern int V_ROOK[2];
extern int V_BISH[2];
extern int V_KNIGHT[2];
extern int V_PAWN[2];

extern int CONNECTED_ROOKS[2];
extern int ROOK_OPEN_FILE[2];
extern int SAFE_KING[2];
extern int BISH_PAIR[2];
extern int KNIGHT_PAWNS[2];
extern int N_KING_OPEN_FILE[2];
extern int PAWN_CHAIN[2];
extern int PAWN_PROTECTION_BISH[2];
extern int PAWN_PROTECTION_KNIG[2];
extern int ATTACKED_BY_PAWN[2];
extern int N_DOUBLED_PAWNS[2];
extern int N_ISOLATED_PAWN[2];
extern int QUEEN_CHECKS[2];

extern int BISH_MOB[2];
extern int KNIG_MOB[2];
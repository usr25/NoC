void initEval(void);
__attribute__((hot)) int eval(const Board* b);
int insuffMat(const Board* b);

extern int V_QUEEN;
extern int V_ROOK;
extern int V_BISH;
extern int V_KNIGHT;
extern int V_PAWN;

extern int V_PASSEDP;
extern int CONNECTED_ROOKS;
extern int BISH_PAIR;
extern int SAFE_KING;
extern int ROOK_OPEN_FILE;

extern int ROOK_XR;
extern int BISH_XR;
extern int QUEEN_XR;
extern int ROOK_MOB;
extern int BISH_MOB;
extern int QUEEN_MOB;

extern int KNIGHT_PAWNS;
extern int N_KING_OPEN_FILE;
extern int N_CLOSE_TO_KING;
extern int N_DOUBLED_PAWNS;
extern int PAWN_CHAIN;
extern int PAWN_PROTECTION;
extern int ATTACKED_BY_PAWN;

extern int BISHOP_MOBILITY;
extern int ATTACKED_BY_PAWN_LATER;
extern int N_PIECE_SLOW_DEV;
extern int STABLE_KING;
extern int N_ISOLATED_PAWN;
extern int N_TARGET_PAWN;
extern int CLEAN_PAWN;

extern int TEMPO;
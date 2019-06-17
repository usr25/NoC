static inline unsigned long long getKingMoves(unsigned int index);
static inline unsigned long long getKnightMoves(unsigned int index);

static inline unsigned long long getQueenMoves(unsigned int index);
static inline unsigned long long getRookMoves(unsigned int index);
static inline unsigned long long getBishMoves(unsigned int index);

static inline unsigned long long getRightMoves(unsigned int index);
static inline unsigned long long getLeftMoves(unsigned int index);
static inline unsigned long long getUpMoves(unsigned int index);
static inline unsigned long long getDownMoves(unsigned int index);

static inline unsigned long long getUpRightMoves(unsigned int index);
static inline unsigned long long getUpLeftMoves(unsigned int index);
static inline unsigned long long getDownRightMoves(unsigned int index);
static inline unsigned long long getDownLeftMoves(unsigned int index);

static inline unsigned long long getWhitePawnMoves(unsigned int index);
static inline unsigned long long getWhitePawnCaptures(unsigned int index);
static inline unsigned long long getBlackPawnMoves(unsigned int index);
static inline unsigned long long getBlackPawnCaptures(unsigned int index);

void initialize();
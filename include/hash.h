#define NUM_ENTRIES 0x1000000 //16_777_216
#define MOD_ENTRIES 0xffffff //NUM_ENTRIES - 1, since it is a pow of 2, mod can be done using & MOD_ENTRIES

#define COLOR_OFFSET 383 //The first (almost) half of the table is for the black pieces
#define PIECE_OFFSET 64 //Indeces for all the tiles for each piece, in order k,q,r,b,n,p
#define TURN_OFFSET 780 //Index in the array of a turn change
#define EPAS_OFFSET 768 //Index in the array of enPass for a given col arr[E_O + (eP & 7)]
#define CAST_OFFSET 776 //Index in the array of the first castle. arr[C_O + color * 2 | castle >> 1]

/* Struct to hold the information about a position for the transposition table
 * key -> Hash of the position
 * val -> Evaluation assigned to the position
 * depth -> depth at which the entry was created
 * Move -> Best move for that position (Hasnt been implemented)
 */
typedef struct
{
    uint64_t key;
    int val;
    int depth;
    Move m;
} Eval;

/* Struct that holds all the information for a 3fold repetition
 * hashTable -> Array that holds the hashes of the position since the last capture / pawn move
 * index -> Index of the last hash inserted + 1 (to add a hash r.hashTable[r.index++] = ...)
 */
typedef struct
{
    uint64_t hashTable[128]; //To avoid problems with the 50-move rule
    int index;
} Repetition;

void initializeTable(void);
int isThreeRep(Repetition* r, uint64_t hash);
uint64_t hashPosition(Board* b);
uint64_t makeMoveHash(uint64_t prev, Board* b, const Move m, const History h);


Eval table[NUM_ENTRIES];
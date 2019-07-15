#define NUM_ENTRIES 0x1000000 //262144
#define MOD_ENTRIES 0xffffff //NUM_ENTRIES - 1, since it is a pow of 2, mod can be done using & MOD_ENTRIES

#define COLOR_OFFSET 383 //The first (almost) half of the table is for the black pieces
#define PIECE_OFFSET 64 //Indeces for all the tiles for each piece, in order k,q,r,b,n,p
#define TURN_OFFSET 780 //Index in the array of a turn change
#define EPAS_OFFSET 768 //Index in the array of enPass for a given col arr[E_O + (eP & 7)]
#define CAST_OFFSET 776 //Index in the array of the first castle. arr[C_O + color * 2 | castle >> 1]

typedef struct
{
    uint64_t key;
    int val;
    int depth;
    Move m; //This isnt used so
} Eval;

Eval table[NUM_ENTRIES];

void initializeTable();
uint64_t hashPosition(Board* b);
uint64_t makeMoveHash(uint64_t prev, Board* b, Move m, History h);
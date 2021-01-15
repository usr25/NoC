#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/memoization.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/io.h"
#include "../include/magic.h"

uint64_t bishMagic[64] = {0x40a004010410300, 0x400405002202140c, 0x4204540022000040, 0x60a082042004242, 0x200424440410a701, 0x3608030c03400010, 0x1412204402010, 0x424050020104, 0x4400644022080, 0x4000480025140820, 0x22009010404104a0, 0x1004082024200060, 0x452c0061000002, 0x104408014808020, 0x8645008020580, 0x20490210410c10, 0x1484804010140, 0x8004610810108022, 0x20008c1218001104, 0x183400808042003, 0x8002080010840011, 0x90014008108a8400, 0x41400240884022, 0x2068080011044204, 0x2212030500240, 0x4401040008164050, 0x908080001000860, 0x20080423004048, 0x101001011004000, 0x432008000102021, 0x4209000101000, 0x2500085000280504, 0x80101044200a0080, 0x5480082000030404, 0x1000100840034804, 0x221040400880120, 0x10802040008d010, 0x10019008322020, 0x50830a01043802, 0x804008a408011100, 0x4800100b008440, 0x4000840836200400, 0x2100202e801000, 0x1101400630930800, 0x1200c002121c00c0, 0x20c8021000b01168, 0x20602081a060008, 0x400140081580c108, 0x210080208031000, 0x401148020020c0, 0x8801008240830, 0x4010000202022050, 0x2080084044090008, 0x4006001044108, 0x98010808004000, 0x209080064005001, 0x202002023040080, 0xa11088600900c200, 0xa014100a0808, 0x841040000c41030, 0x40000020002882, 0x200480650114082, 0x2101204880f80, 0x1104040048001c80};
uint64_t rookMagic[64] = {0x80002040008010, 0x40200040001000, 0x401000400204060, 0x50042640502800, 0x1a00020008200104, 0x20009200050400, 0x1001201002401468, 0x450000804021000a, 0x2004004400820102, 0x4200104020004a10, 0x4000440010a0001c, 0x1044800800049000, 0x104418070a04, 0x2800824004082006, 0x110100044081e001, 0x1921000200040, 0x100404800110080, 0x840000402200800, 0x10000801200434, 0x44000810024008, 0x1842808102029, 0x8104004000818040, 0x12004904200, 0x90022001c01008, 0x40065a201000c004, 0x1040980004012004, 0x1200080020042000, 0x40820008100450, 0x400460a008004200, 0x105032008008001, 0x2064005001020, 0x6400904200010094, 0x308001010048001, 0x904604000080902, 0x1080c04002040d, 0x1020400205100004, 0x1418140418008040, 0x42018045000103, 0x401c10004401101, 0x4001100800a00042, 0x8050014009001, 0x8140010030010080, 0x800480104c801400, 0xc100901000a, 0xc00020004004001, 0x8c000200010300, 0x4840048000411001, 0x9088002848810002, 0x2420108006400012, 0x224003000400092, 0xd410402800200128, 0x180040042002018, 0x1010300800600350, 0x82008040440208, 0x4080810250420400, 0x2a0010000204080, 0x406008420104302, 0x1414182080c001, 0x802401005082101, 0xa0040402489a020e, 0x800211002000106, 0x22010d8422084809, 0x42000210008720d4, 0x8041002412};

uint64_t bishMagicMoves[64] [512];
uint64_t rookMagicMoves[64][4096];

const uint64_t indexToBitboard(const int index, const int bits, uint64_t m);
uint64_t findMagic(int sqr, int isBishop);
static void populateRookMagics(void);
static void populateBishMagics(void);

/* Generate a random uint64_t
 */
static inline uint64_t random_uint64()
{
    return (((uint64_t)rand()) << 33) | rand();
}

/* Populate the arrays
 */
void initMagics(void)
{
    populateRookMagics();
    populateBishMagics();
}

/* Generate all the magics and populate the arrays
 * It shouldnt be called since the magics are hard-coded
 */
void genMagics(void)
{
    srand(11); //'Random' number I picked by hand, used to ensure determinism

    for (int i = 0; i < 64; ++i)
    {
        rookMagic[i] = findMagic(i, 0);
        bishMagic[i] = findMagic(i, 1);
    }

    initMagics();
}

/* Generate the respective bb from a mask, this is used to enumerate all
 * possible states
 */
const uint64_t indexToBitboard(const int index, const int bits, uint64_t m)
{
    int j;
    uint64_t result = 0ULL;
    for(int i = 0; i < bits; i++)
    {
        j = LSB_INDEX(m);
        REMOVE_LSB(m);
        if(index & (1 << i)) result |= (1ULL << j);
    }
    return result;
}

/* Generate the magic for a given sqr and piece type
 * This algorithm is NOT guaranteed to find a solution since it works by trial and error
 */
uint64_t findMagic(int sqr, int isBishop)
{
    uint64_t attacks[4096], colArray[4096], magic;
    int i, collision, arrInd;

    const uint64_t mask = isBishop? getDiagInt(sqr) : getStraInt(sqr);
    const int n = POPCOUNT(mask);

    const int sh = isBishop? 55 : 52;

    for (i = 0; i < (1 << n); i++)
        attacks[i] = indexToBitboard(i, n, mask);

    for (int k = 0; k < 0xfffff; k++)
    {
        magic = random_uint64() & random_uint64() & random_uint64(); //Magics with low popcounts are better

        if (POPCOUNT((mask * magic) & 0xFF00000000000000ULL) < 6) continue; //If the magic doesn't map at lease 6 bits to the upper row of the bitboard, it wont work
        for (i = 0; i < 4096; i++) colArray[i] = 0ULL; //Initialize the collision array

        //Go through all the possible states and see if it produces a collision, if it doesn't it is a valid magic for the given square
        for (i = 0, collision = 0; !collision && i < (1 << n); i++)
        {
            arrInd = (attacks[i] * magic) >> sh; // >> (64 - n) for better magics space wise
            if (!colArray[arrInd]) colArray[arrInd] = 1;
            else collision = 1;
        }

        if (!collision) //There haven't been any colisions, so the magic works
            return magic;
    }

    printf("[-] ERROR, increase the limit of the loop or change the seed\n");
    return 0ULL;
}

/* Map the respective bitboards to the magics for the rook movements
 */
static void populateRookMagics(void)
{
    for (int i = 0; i < 64; ++i)
    {
        const uint64_t mask = getStraInt(i);
        const uint64_t n = POPCOUNT(mask);

        for (int j = 0; j < 4096; ++j)
        {
            const uint64_t state = indexToBitboard(j, n, mask);
            const int index = (state * rookMagic[i]) >> 52;

            rookMagicMoves[i][index] = straight(i, state);
        }
    }
}
/* Map the respective bitboards to the magics for the bishop movements
 */
static void populateBishMagics(void)
{
    for (int i = 0; i < 64; ++i)
    {
        const uint64_t mask = getDiagInt(i);
        const uint64_t n = POPCOUNT(mask);

        for (int j = 0; j < 512; ++j)
        {
            const uint64_t state = indexToBitboard(j, n, mask);
            const int index = (state * bishMagic[i]) >> 55;

            bishMagicMoves[i][index] = diagonal(i, state);
        }
    }
}
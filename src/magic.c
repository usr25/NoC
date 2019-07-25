#include <stdio.h>
#include <stdlib.h>

#include "../include/global.h"
#include "../include/memoization.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/io.h"
#include "../include/magic.h"

uint64_t indexToBitboard(int index, int bits, uint64_t m);
uint64_t findMagic(int sqr, int isBishop);
void populateRookMagics(void);
void populateBishMagics(void);

/* Generate a random uint64_t
 */
static inline uint64_t random_uint64() 
{
  return (((uint64_t)rand()) << 33) | rand();
}

/* Generate all the magics and populate the arrays
 */
void genMagics(void)
{
    for (int i = 0; i < 64; ++i)
    {
        rookMagic[i] = findMagic(i, 0);
        bishMagic[i] = findMagic(i, 1);
    }
    populateRookMagics();
    populateBishMagics();
}

/* Generate the respective bb from a mask, this is used to enumerate all
 * possible states
 */
uint64_t indexToBitboard(int index, int bits, uint64_t m) 
{
    int j;
    uint64_t result = 0ULL;
    for(int i = 0; i < bits; i++) {
        j = LSB_INDEX(m);
        REMOVE_LSB(m);
        if(index & (1 << i)) result |= (1ULL << j);
    }
    return result;
}

/* Generate the magic for a given sqr and piece type
 * This algorithm is NOT guaranteed to finish
 */
uint64_t findMagic(int sqr, int isBishop)
{
    uint64_t attacks[4096], colArray[4096], magic;
    int i, k, collision, arrInd;

    const uint64_t mask = isBishop ? getDiagInt(sqr) : getStraInt(sqr);
    const int n = POPCOUNT(mask);

    int sh = isBishop? 55 : 52;

    for (i = 0; i < (1 << n); i++)
        attacks[i] = indexToBitboard(i, n, mask);
    
    for (k = 0; k < 0xfffff; k++) 
    {
        magic = random_uint64() & random_uint64() & random_uint64();
        
        if(POPCOUNT((mask * magic) & 0xFF00000000000000ULL) < 6) continue;
        for (i = 0; i < 4096; i++) colArray[i] = 0ULL;
        
        for (i = 0, collision = 0; !collision && i < (1 << n); i++)
        {
            arrInd = (attacks[i] * magic) >> sh; // >> (64 - n) for better magics space wise
            if (!colArray[arrInd]) colArray[arrInd] = 1;
            else collision = 1;
        }

        if (!collision) //There hasnt been any colision, so the magic works
            return magic;
    }
    printf("[-] ERROR, increase k or change the seed\n");
    return 0ULL;
}

void populateRookMagics(void)
{
    for (int i = 0; i < 64; ++i)
    {
        const uint64_t mask = getStraInt(i);
        const uint64_t n = POPCOUNT(mask);

        for (int j = 0; j < 4096; ++j)
        {
            uint64_t state = indexToBitboard(j, n, mask);
            int index = (state * rookMagic[i]) >> 52;

            rookMagicMoves[i][index] = straight(i, state);
        }
    }
}
void populateBishMagics(void)
{
    for (int i = 0; i < 64; ++i)
    {
        const uint64_t mask = getDiagInt(i);
        const uint64_t n = POPCOUNT(mask);

        for (int j = 0; j < 512; ++j)
        {
            uint64_t state = indexToBitboard(j, n, mask);
            int index = (state * bishMagic[i]) >> 55;

            bishMagicMoves[i][index] = diagonal(i, state);
        }
    }
}
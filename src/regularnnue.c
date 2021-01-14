#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "../include/global.h"
#include "../include/board.h"
#include "../include/moves.h"
#include "../include/boardmoves.h"
#include "../include/nnue.h"

//This is depends on sparse/regular
const int getIdx(const int i, const int j, const int dim)
{
    return i*dim+j;
}
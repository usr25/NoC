/* 
 * File:   main.c
 *
 * Main file, in charge of calling the mainloop
 */

#include <stdio.h>
#include <stdlib.h>

#include "../include/chess.h"

int main(int argc, char const *argv[])
{
    printf("%d\n", DIMS);
    printf("%d\n", DIMS2);
    printf("%d\n", val());
    
    return (EXIT_SUCCESS);
}
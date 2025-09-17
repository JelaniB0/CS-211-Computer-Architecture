/*
 * Programming Assignment 2
 * CS 211 Spring 2025 (Sections 5-8)
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "pa2.h"

/**
 * converts the input ASCII string into a 128-bit number that we can use
 */
struct bv128 parse_addend(char bit_width_str[], char addend_str[]) 
{
    // use the standard library function strtol() to convert from string to int
    int bit_width = strtol(bit_width_str, NULL, 0);
    if(bit_width <= 0 || bit_width > 128)
        return (struct bv128){0, 0, ERROR_INVALID_ARGUMENTS};

    // use your conversion function to convert the string to a bv128 type
    return str_to_bv128(addend_str, strlen(addend_str), bit_width);
}

/*
 * standardized output for when something goes wrong with the program or its inputs
 */
void print_error(enum error_code ec)
{
    switch(ec)
    {
        case ERROR_NONE:              printf("error: %s\n", "none");              break;
        case ERROR_INVALID_ARGUMENTS: printf("error: %s\n", "invalid arguments"); break;
        case ERROR_MALFORMED_ADDEND:  printf("error: %s\n", "malformed addend");  break;
        case ERROR_ADDEND_OVERFLOW:   printf("error: %s\n", "addend overflow");   break;
        case ERROR_SUM_OVERFLOW:      printf("error: %s\n", "sum overflow");      break;
        default:                      printf("error: %s\n", "invalid enum");
    }
} 

/*
 * Main expects four arguments:
 *   ./pa2 <addend0_bit_width> <addend0_representation> <addend1_bit_width> <addend1_representation>
 * 
 * Example uses:  
 *   ./pa2 4 0x3 4 0xa
 *   ./pa2 16 0x3eda 32 0x00f33d4d
 *   ./pa2 128 0xfedcba9876543210 128 0xffffffffffffffffffffffffffffffff
 */
int main(int argc, char **argv)
{
    if(argc != 5) {
        print_error(ERROR_INVALID_ARGUMENTS);
        return EXIT_FAILURE;
    }

    // parse addend 0
    struct bv128 addend_0 = parse_addend(argv[1], argv[2]);
    if(addend_0.error != ERROR_NONE)
    {
        print_error(addend_0.error);
        return EXIT_FAILURE;
    }

    // parse addend 1
    struct bv128 addend_1 = parse_addend(argv[3], argv[4]);
    if(addend_1.error != ERROR_NONE)
    {
        print_error(addend_1.error);
        return EXIT_FAILURE;
    }

    // perform the 128-bit add operation
    struct bv128 sum = add_bv128(addend_0, addend_1);
    if(sum.error != ERROR_NONE)
    {
        print_error(sum.error);
        return EXIT_FAILURE;
    }
    else
    {
        print_bv128(sum);
        return EXIT_SUCCESS;
    }
}

/*
 * Programming Assignment 2
 * CS 211 Spring 2025 (Sections 5-8)
 */
#include <stdio.h>

#include "pa2.h"

/******************************************************************************
 * Provided functions: consult these as examples for how to work with the data
 ******************************************************************************/

/** prints out the entire 128-bit value as an ASCII-encoded hexadecimal string
 *
 *  note: this relies on your value_to_hexdigit() implementation, so if you get
 *  that wrong, this printout will be wrong too!
 */
void print_bv128(struct bv128 val)
{
    char string[32];
    for(int nibble_idx = 31; nibble_idx >= 0; nibble_idx--)
    {
        int nibble = 0;
        if(nibble_idx >= 16)
            nibble = (val.hi >> ((nibble_idx - 16) * 4)) & 0xf;
        else
            nibble = (val.lo >> (nibble_idx * 4)) & 0xf;
        
        // should never run into this since a nibble should not exceed unsigned value 15
        string[31 - nibble_idx] = is_hexvalue(nibble) ? value_to_hexdigit(nibble) : '?';
    }

    printf("0x%.32s\n", string);
}

/** check if the provided ASCII character is a valid hexadecimal digit */
int is_hexdigit(char c)
{
	/* chars represent their codepoints, and the codepoints can be compared just
	like any other number. refer to the ASCII table for more info */
    return 
		(c >= '0' && c <= '9') ||
		(c >= 'a' && c <= 'f') ||
		(c >= 'A' && c <= 'F');
}


/******************************************************************************
 * Your Assignment:
 *     Implement the assigned functions declared in pa2.h. 
 *     You may NOT:
 *      - Use any compiler-supported 128-bit data types (e.g., __int128_t)
 *      - Use macros or functions from any standard library (e.g., <string.h>, <stdio.h>)
 *      - Use dynamic memory allocation (malloc/free)
 *      - Use static or global variables
 *     
 *     Make sure to follow the detailed specifications from pa2.h, including setting 
 *     the error conditions correctly where necessary.
 *     
 *     Consult the Google Doc for further information on everything.
 ******************************************************************************/

/**
 * This is a complement to is_hexdigit(): checks whether the number v is
 * representable as a single hexadecimal digit (true) 
 */
int is_hexvalue(int v) 
{
	return (v >= 0 && v <= 15); //should be good
}

/**
 * Convert a single ASCII character that encodes a hexadecimal digit to its
 * decimal value (e.g., 'a' -> 10)
 *
 * You could use a switch statement, but there is a much easier way based on the
 * arrangement of ASCII codepoints, which you might figure out by consulting
 * is_hexdigit() above 
 */
int hexdigit_to_value(char c)
{
    //works tho i can prob use an array too. 
    int num; //probably fine? If not just resort back to normal int. 
    if(c >= '0' && c <= '9'){ //done. 
        num = c - '0'; //probably fine? 
        return num;
    }else if((c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')){
        if (c == 'a' || c == 'A')  {
            return 10; 
        }
        if (c == 'b' || c == 'B')  {
            return 11; 
        }
        if (c == 'c' || c == 'C')  {
            return 12; 
        }
        if (c == 'd' || c == 'D')  {
            return 13; 
        }
        if (c == 'e' || c == 'E')  {
            return 14; 
        }
        if (c == 'f' || c == 'F')  {
            return 15; 
        }
    }
}

/**
 * Convert the value of a hexadecimal digit (i.e., 0-15) into its ASCII
 * character representation (e.g., 10 -> 'a')
 *
 * This is the reverse of the previous function, and there is an easier
 * way to do it using ASCII codepoints rather than a giant switch statement.
 */
char value_to_hexdigit(int v)
{
    //works though i can prob use an array too. 
    if (v >= 0 && v <= 9){
        char c = v + '0';
        return c;
    }else if(v >= 10 && v <= 15){
        if (v == 10){
            return 'a';
        }if (v == 11){
            return 'b';
        }if (v == 12){
            return 'c';
        }if (v == 13){
            return 'd';
        }if (v == 14){
            return 'e';
        }if (v == 15){
            return 'f';
        }
    }
}

/**
 * Return the value of a number's sign bit given the bit width. Follow the
 * detailed specification given in pa2.h.
 *
 * Hint: using < or > will not work! 
 */
int get_sign_bit_value(struct bv128 bv, int bit_width)
{
    int sign_bit_index = bit_width - 1;
    if (bit_width > 64) {
        int sign_bit = (bv.hi >> (sign_bit_index - 64)) & 1;
        return sign_bit ? 1 : 0; 
    }
    int sign_bit = (bv.lo >> sign_bit_index) & 1;
    return sign_bit ? 1 : 0; 
}

/**
 * Convert an ASCII string containing hexadecimal digits (example: "0x5afb") to
 * a bv128 struct. 
 *
 * see pa2.h for the detailed specifications of this function
 */
struct bv128 str_to_bv128(char str[], int str_len, int bit_width)
{
    //do this later. 
    //Step One: Check for badly formed hex string
    //if hex string structure is invalid.
    if (!(str[0] == '0' && ((str[1] == 'x') ||(str[1] == 'X')))){ //should work now probably. 
        return (struct bv128){0, 0, ERROR_MALFORMED_ADDEND};
    }else{
        for(int i = 2; i < str_len; i++){
            if(!is_hexdigit(str[i])){
                return (struct bv128){0, 0, ERROR_MALFORMED_ADDEND};
            }
        }
    }

    //str_len is hex digit length and bit width is bit length so if str_len * 4 > bit_width it's too large. 

    int numOfBits = 0;
    // if(bit_width < 4){
    //     numOfBits = bit_width;
    // }else{
    //     numOfBits = (str_len-2)*4;
    // }
    int numOfMostSigBit = 1;
    int mostSigBitAmount = hexdigit_to_value(str[2]);
    for(int i = 3; i >= 0; i--){
        if (((mostSigBitAmount >> i) & 1) == 1){
            numOfMostSigBit = i+1;
            break;
        }
    }
    numOfBits = numOfMostSigBit + (str_len-3)*4;

        // if (divisor == 0){
        //     numOfBits+=4;
        // }
        // while (divisor != 0){
        //     numOfBits += 1;
        //     divisor = divisor/2;
        // }
    if (numOfBits > bit_width){
        return (struct bv128){0, 0, ERROR_ADDEND_OVERFLOW};
    }
    
    /*
     * STEP 2: hex string to binary conversion
     */
    uint64_t hi = 0; // bits [127:64]
    uint64_t lo = 0; // bits [63:0]
   
    uint64_t start = 0; //starts at bit 0. 
    for (int i = str_len - 1; i >= 2; i--) {
        uint64_t val = hexdigit_to_value(str[i]); //Goes from hex to number. 
        if (start < 64) {
            lo |= (val << start);
        } else {
            hi |= (val << (start - 64)); //cut down my code so much through this method instead of checking and converting bits 1 by 1. 
        }
        start += (uint64_t)4;
    }

    /*
     * STEP 3: sign extension to a 128-bit two's complement number
     */
    int sign_bit_idx = bit_width - 1;
    int originalidx = sign_bit_idx;
    if (bit_width < 128) {  // Sign extension only applies if not full 128 bit. 
        if (sign_bit_idx >= 64) {
            int sign_bit_value = (hi >> (sign_bit_idx - 64)) & 1;
            if (sign_bit_value) {
                hi |= (~0ULL << (sign_bit_idx - 63)); //Sign extension for high part only. 
            }
        } else {
            int sign_bit_value = (lo >> sign_bit_idx) & 1;
            if (sign_bit_value) {
                lo |= (~0ULL << sign_bit_idx); // Sign extension for low part only. 
                hi = ~0ULL; //Fills hi part with only 1s as part of extension. 
            }
        }
    }
    return (struct bv128){hi, lo, ERROR_NONE}; 
}



/**
 * perform a 128-bit add on a + b 
 *
 * see pa2.h for the detailed specification of this function
 */
struct bv128 add_bv128(struct bv128 a, struct bv128 b)
{
    //overflow is if sign flips (so 2 positives get a negative or 2 negatives get a positive, meaning value is too larg, so error sum overflow. )
    uint64_t hisum = 0;
    uint64_t lowsum = 0;
    int signOfa = get_sign_bit_value(a, 128);
    int signOfb = get_sign_bit_value(b, 128);

    
    lowsum = a.lo + b.lo;
    hisum = a.hi + b.hi;

    if (lowsum < a.lo ){
        hisum+=1;
    }

    int signOfSumToCorrect = get_sign_bit_value((struct bv128){hisum, lowsum, ERROR_NONE}, 128);


    struct bv128 sumOfTwo = (struct bv128){hisum, lowsum, ERROR_NONE};
    if (signOfa == signOfb && signOfSumToCorrect != signOfa){
        return (struct bv128){0, 0, ERROR_SUM_OVERFLOW};
    }
    return (struct bv128){hisum, lowsum, ERROR_NONE}; //should be done. 
}

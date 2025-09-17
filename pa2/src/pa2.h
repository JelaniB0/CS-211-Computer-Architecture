/*
 * Programming Assignment 2
 * CS 211 Spring 2025 (Sections 5-8)
 */
#ifndef PA2_H
#define PA2_H

#include <stdint.h>

#define TRUE (1 == 1)
#define FALSE (1 != 1)

enum error_code 
{
    ERROR_NONE = 0, // No error

	/** One or more of the provided arguments was invalid for some reason
	 *  (wrong order, wrong number, etc.). Different from MALFORMED_ADDEND */
    ERROR_INVALID_ARGUMENTS = 1, 

    ERROR_MALFORMED_ADDEND = 2, // One or both of the addends was improper (bad hexdigit, etc.).
    ERROR_ADDEND_OVERFLOW = 3, // One or both of the addends overflowed the provided width. 
    ERROR_SUM_OVERFLOW = 4 // The sum overflowed the provided width.
};

/**
 * We are storing our 128 bit number as two unsigned 64 bit numbers. Don’t be
 * confused by the fact that they are unsigned, our numbers are very much
 * signed, since we are storing them in two’s complement format. These two
 * uint64_t's are simply containers for our bits.
 *
 * You must represent the 128 bit number in big-endian format using the 'lo'
 * field for the least significant bits and the 'hi' field for the most
 * significant bits. Thus, for a 128-bit number called 'val': 
 *    - hi[63:56] = val[127:120] 
 *    - hi[7:0] = val[71:64] 
 *    - lo[63:56] = val[63:56]
 *    - lo[7:0] = val[7:0]
 *
 * The error code enum is there to handle the possibility of errors, such as
 * invalid arguments or overflow. If these occur, you should set this value in
 * your result.
 */
struct bv128
{
    uint64_t hi; // bits [127:64]
    uint64_t lo; // bits [63:0]
    enum error_code error;
};

/******************************************************************************
 * Provided Functions: 
 *     Consult these as examples for how to work with the data
 ******************************************************************************/

// Prints a given number in our struct form.
void print_bv128(struct bv128 val);

/**
 * Accepts: a single ASCII character, which encodes one hexadecimal digit from
 * one of the addends.
 *
 * Returns: TRUE or FALSE: whether or not that character encodes a valid
 * hexdigit (0-9, a-f, A-F) 
 */
int is_hexdigit(char c);

/******************************************************************************
 * Your Assignment:
 *     Implement the following functions in pa2.c
 ******************************************************************************/
/**
 * Accepts: a value from [0-15] representing a single hexadecimal number
 * 
 * Returns: TRUE or FALSE: whether this is a valid number in base 16
 *
 * Errors reported: None
 *
 * Assumptions: None
 */
int is_hexvalue(int v);

/**
 * Accepts: a single ASCII character, which is a digit from one of the hex
 * strings you are given as inputs.
 *
 * Returns: the integer value (0-15) of that hexdigit.
 *
 * Errors reported: None
 *
 * Assumptions: 
 *     - Assume the input is a valid hexdigit value
 */
int hexdigit_to_value(char c);

/**
 * Reverse of the previous function:
 *
 * Accepts: the integer value of a hexadecimal digit
 *
 * Returns: the ASCII character representing that hexdigit
 *
 * Errors reported: None
 *
 * Assumptions:
 *     - Assume the input is a valid hexdigit's ASCII character
 */
char value_to_hexdigit(int c);

/**
 * Accepts: a 128-bit number in the form of 'struct bv_128', and the given
 * bit-width.
 *
 * Returns: 1 if the number is negative or 0 if it’s positive [the value of the
 * leftmost bit (the sign bit) in the number].
 *
 * Errors reported: None
 *
 * Assumptions: 
 *     - Ignore bv.error: return the correct value regardless of what it is
 *     - Assume 0 < bit_width <= 128
 */
int get_sign_bit_value(struct bv128 bv, int bit_width);

/**
 * Accepts: an ASCII string as a char[], the length of that string, and the bit
 * width of the two's complement number that the string describes. It is your
 * job to determine the number's sign and exetend the number to fit within the
 * 128-bit two's complement value. 
 *
 * A valid string must start with "0x" or "0X", and the remaining digits must be
 * one or more valid hexadecimal digits (case insensitive). 
 *
 * Example valid strings:
 *     - 0xf
 *     - 0x0000000000000001
 *     - 0XFe3
 *
 * Example invalid strings:
 *     - f
 *     - 0xg
 *
 * Returns: the ASCII string converted to a two's complement 128-bit number, in
 * the format of our 128-bit-width struct. For example, the hex string "0xf"
 * should return 15, so hi = 0 (000...000) and lo = 15 (000...0001111).
 *
 * Errors reported: 
 *     There are three possible cases for an error:
 *         - If the string is invalid, return zero and ERROR_MALFORMED_ADDEND.
 *         - If the string is valid, but the value does not actually fit in the given 
 *               number of bits, return zero and ERROR_ADDEND_OVERFLOW.
 *         - If everything is valid, return the value and ERROR_NONE.
 *
 * Assumptions: 
 *     - You may safely assume that `str_len` always gives the correct length of whatever 
 *       string is provided in `str`. However `str` may be an invalid string.
 *     - Assume 0 < bit_width <= 128
 */
struct bv128 str_to_bv128(char str[], int str_len, int bit_width);

/**
 * Accepts: the two 128-bit two's complement addends, a and b, in their struct
 * form. 
 *
 * Returns: the sum (a+b) and an error flag with the appropriate value (see below)
 *
 * Errors reported: 
 *     There are two possible cases for an error:
 *         - ERROR_NONE (if the sum is representable) zero
 *         - ERROR_SUM_OVERFLOW (if the sum overflows)
 *
 * Assumptions: 
 *     - Ignore the error fields of a and b (perform the sum regardless).
 *
 * Hint: You cannot just use '+' for this: it won't work. Think about the two's
 * complement representation of the the 128-bit number and what it means to have
 * positive and negative overflow.
 */
struct bv128 add_bv128(struct bv128 a, struct bv128 b);

#endif /* PA2_H */
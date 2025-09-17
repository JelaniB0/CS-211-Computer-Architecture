#ifndef PA5_H
#define PA5_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <sys/types.h>

#include "pa5_cache.h" // Maybe use for memoutput

/*****************************************************************************
 *                      PROVIDED UTILITY FUNCTIONS                           *
 *****************************************************************************/

// extract the immediate from an instruction
uint64_t extract_immediate(uint32_t inst);

// extract some range of bits from an instruction (indices are right to left)
uint16_t extract_bits(uint32_t inst, uint8_t msb_idx, uint8_t lsb_idx);

/*****************************************************************************
 *                   RECOMMENDED UTILITY FUNCTIONS                           *
 *                                                                           *
 * We recommend you implement these to make the fetch() and memory() stage   *
 * cleaner. These will not be graded, but feel free to define assert-based   *
 * tests for them if you like.                                               *
 *****************************************************************************/
/*
 * use the function pointer rd_fn to read the RISC-V instruction from memory at
 * the given program counter 
 */
uint32_t read_instruction(uint64_t pc, uint8_t (*rd_fn)(uint64_t addr));

// use the function pointer rd_fn to read the dword at the given memory address
uint64_t read_dword(uint64_t addr, uint8_t (*rd_fn)(uint64_t addr));

// use the function pointer wr_fn to write the dword "data" to the given memory address
void write_dword(uint64_t addr, uint64_t data, void (*wr_fn)(uint64_t addr, uint8_t data));

/*****************************************************************************
 *                                   FETCH                                   *
 *****************************************************************************/
/*
 * simulates the fetch stage
 *
 * inputs:
 *     pc: the program counter for this cycle
 *     mem_read_byte: a function pointer to a function that allows reading a single byte from memory
 * outputs:
 *     out: the outputs of the fetch stage (defined in types.h)
 * assumptions:
 *     - the pc will be a multiple of 4 bytes
 */
void pa5_fetch(uint64_t pc, uint8_t (*mem_read_byte)(uint64_t addr), struct fetch_outputs *out);

/*****************************************************************************
 *                                  DECODE                                   *
 *****************************************************************************/
/*
 * decode the ALU operation from the instruction
 *
 * inputs:
 *     inst: the instruction to decode
 * outputs:
 *     out: the ALU operation (enums are defined in types.h)
 */
enum ALU_OP get_aluop(uint32_t inst);

/*
 * decode the branch condition from the instruction
 *
 * inputs:
 *     inst: the instruction to decode
 * outputs:
 *     out: the branch condition (enums are defined in types.h)
 */
enum BR_COND get_br_cond(uint32_t inst);

/*
 * simulates the decode stage
 *
 * inputs:
 *     in: the outputs of the fetch stage (that are inputs to the decode stage)
 *     gprs: x0 - x31 (the general-purpose registers) READ-ONLY
 * outputs:
 *     out: the outputs of the decode stage (defined in types.h)
 * assumptions:
 *     - there will always be NUM_GPRS elements in the gprs array
 */
void pa5_decode(const struct fetch_outputs *in,
    const uint64_t *gprs,
    struct decode_outputs *out);

/*****************************************************************************
 *                        ADDRESS GENERATION + EXECUTE                       *
 *****************************************************************************/
/*
 * simulates the agex stage
 *
 * inputs:
 *     in: the outputs of the decode stage
 * outputs:
 *     out: the outputs of the agex stage (defined in types.h)
 */
void pa5_agex(const struct decode_outputs *in, struct agex_outputs *out);

/*****************************************************************************
 *                                 MEMORY                                    *
 *****************************************************************************/
/*
 * simulates the memory stage
 *
 * inputs:
 *     in: the outputs of the agex stage
 *     mem_read_byte: a function pointer to a function that allows reading a single byte from memory
 *     mem_write_byte: a function pointer to a function that allows writing a single byte to memory
 * outputs:
 *     out: the outputs of the memory stage (defined in types.h)
 * assumptions:
 *     - the memory address to access will always be a multiple of 8 bytes
 */
void pa5_mem(const struct agex_outputs *in,
    uint8_t (*mem_read_byte)(uint64_t addr),
    void (*mem_write_byte)(uint64_t addr, uint8_t data),
    struct mem_outputs *out,
    bool use_cache);

/*****************************************************************************
 *                               WRITEBACK                                   *
 *****************************************************************************/
/*
 * simulates the writeback stage
 *
 * inputs:
 *     in: the outputs of the memory stage
 *     gprs: x0 - x31 (the general-purpose registers)
 * outputs:
 *     out: the outputs of the writeback stage (defined in types.h)
 * assumptions: 
 *     - there will always be NUM_GPRS elements in the gprs array
 */
 void pa5_writeback(const struct mem_outputs *in, uint64_t *gprs, struct writeback_outputs *out);

#endif /* PA5_H */

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>

/*****************************************************************************
 *               RV64I CONSTANTS (defined in the specifications)             *
 *****************************************************************************/

#define RV_OPCODE_OP      51  /* 0b0110011 -  Register-register arithmetic instructions */
#define RV_OPCODE_OP_IMM  19  /* 0b0010011 -  Immediate arithmetic instructions */
#define RV_OPCODE_LOAD    3   /* 0b0000011 -  Load instructions (LB, LH, LW, etc.) */
#define RV_OPCODE_STORE   35  /* 0b0100011 -  Store instructions (SB, SH, SW, etc.) */
#define RV_OPCODE_BRANCH  99  /* 0b1100011 -  Branch instructions (BEQ, BNE, etc.) */

#define FUNCT3_ADD_SUB  0   /* 0b000 */
#define FUNCT3_SLT      2   /* 0b010 */
#define FUNCT3_SLTU     3   /* 0b011 */
#define FUNCT3_XOR      4   /* 0b100 */
#define FUNCT3_OR       6   /* 0b110 */
#define FUNCT3_AND      7   /* 0b111 */

#define FUNCT3_BEQ   0   /* 0b000 */
#define FUNCT3_BNE   1   /* 0b001 */
#define FUNCT3_BLT   4   /* 0b100 */
#define FUNCT3_BGE   5   /* 0b101 */
#define FUNCT3_BLTU  6   /* 0b110 */
#define FUNCT3_BGEU  7   /* 0b111 */

#define FUNCT7_ADD 0   /* 0b0000000 */
#define FUNCT7_SUB 32  /* 0b0100000 */

enum GPR_IDX // Correspond from instruction read to hex and map the two. Set these to wtvr values are read in from array.
{
    // For human readability?
    X0_ZERO = 0, // zero (immutable)
    X1_RA, // ra (return address)
    X2_SP, // sp (stack pointer)
    X3_GP, // gp (global pointer)
    X4_TP, // tp (thread pointer)
    X5_T0, X6_T1, X7_T2, // temporary registers
    X8_S0_FP, // callee-save register (frame pointer)
    X9_S1, // callee-save register
    X10_A0, X11_A1, X12_A2, X13_A3, X14_A4, X15_A5, X16_A6, X17_A7, // function argument registers
    X18_S2, X19_S3, X20_S4, X21_S5, X22_S6, X23_S7, X24_S8, X25_S9, X26_S10, X27_S11, // callee-save registers
    X28_T3, X29_T4, X30_T5, X31_T6, // temporary registers Changed to X31_T6 instead of X21_T6. 

    NUM_GPRS
};

/*****************************************************************************
 *                            CONTROL SIGNAL ENUMS                           *
 *****************************************************************************/

/* the different types of operations supported by the ALU. used for:
 *   - R-type (depends on funct)
 *   - Load/Store address calculation (add)
 *   - Branch target calculation (add)
 */
enum ALU_OP // We don't use shifts, not defined. 
{
    ALU_OP_ADD,
    ALU_OP_SUB,
    ALU_OP_XOR,
    ALU_OP_OR,
    ALU_OP_AND,

    ALU_OP_NONE
};

// branch condition selection
enum BR_COND
{
    BR_COND_EQ,
    BR_COND_NEQ,
    BR_COND_LT,
    BR_COND_LTU,
    BR_COND_GE,
    BR_COND_GEU,
    BR_COND_ALWAYS,
    BR_COND_NEVER,

    BR_COND_NONE
};

// writeback mux selection
enum WB_MUX_SEL
{
    WB_SEL_ALU_RESULT,
    WB_SEL_MEM_READ_DATA,

    WB_SEL_NONE
};

// A mux selection
enum A_MUX_SEL
{
    A_SEL_PC,
    A_SEL_RS1,

    A_SEL_NONE
};

// B mux selection
enum B_MUX_SEL
{
    B_SEL_RS2,
    B_SEL_IMM,

    B_SEL_NONE
};

// PC mux selection
enum PC_MUX_SEL
{
    PC_SEL_PC_PLUS_4,
    PC_SEL_ALU_RESULT,

    PC_SEL_NONE
};

/*****************************************************************************
 *                             CPU STAGE OUTPUTS                             *
 *****************************************************************************/

// the outputs of the fetch stage
struct fetch_outputs
{
    uint32_t inst; // the instruction register that should contain the fetched instruction
    uint64_t pc; // the program counter for this cycle, to be passed on to later CPU stages
};

// the outputs of the decode stage
struct decode_outputs
{
    // control signals for future stages
    enum GPR_IDX rd_idx; // the 5-bit destination register index
    enum ALU_OP alu_op; // the operation that the ALU should perform during the agex stage
    enum BR_COND br_cond; // the branch condition to check for during the agex stage
    enum WB_MUX_SEL wb_sel; // which field to write back to the destination register (if any)
    enum A_MUX_SEL a_sel; // which field to select as the A input to the ALU
    enum B_MUX_SEL b_sel; // which field to select as the B input to the ALU
    bool reg_write_en; // whether to write the destination register during the writeback stage
    bool mem_read_en; // whether to read memory during the memory stage
    bool mem_write_en; // whether to write memory during the memory stage

    // data forwarded to future stages
    uint64_t rs1; // the data of source register 1
    uint64_t rs2; // the data of source register 2
    uint64_t imm; // correctly sign/zero-extended immediate value for (I/S/B/U/J-type instructions)
    uint64_t pc; // the pc forwarded onward to the next stage
};

// the outputs of the address generation + execution stage
struct agex_outputs
{
    // new control signals for future stages
    enum PC_MUX_SEL pc_sel; // which value to update the PC with during the writeback stage

    // control signals forwarded to future stages
    enum GPR_IDX rd_idx; // 5-bit destination register index
    enum WB_MUX_SEL wb_sel; // which field to write back to the destination register (if any)
    bool reg_write_en; // whether to write the destination register during the writeback stage
    bool mem_read_en; // whether to read memory during the memory stage
    bool mem_write_en; // whether to write memory during the memory stage

    // data forwarded to future stages
    uint64_t alu_result; // the result of ALU calculations
    uint64_t rs2; // the data from rs2, used for register file write or store operations
    uint64_t pc; // the pc forwarded to the next stage
};

// the outputs of the memory stage
struct mem_outputs
{
    // control signals forwarded to future stages
    enum GPR_IDX rd_idx; // 5-bit destination register index
    enum WB_MUX_SEL wb_sel; // which field to write back to the destination register (if any)
    enum PC_MUX_SEL pc_sel; // which value to update the PC with during the writeback stage
    bool reg_write_en; // whether to write the destination register during the writeback stage

    // data forwarded to future stages
    uint64_t alu_result; // the result of ALU calculations
    uint64_t mem_rdata; // the data read from memory
    uint64_t pc; // the pc forwarded to the next stage
};

// the outputs of the writeback stage
struct writeback_outputs
{
    uint64_t next_pc; // the new pc value used for the next fetch stage operation
};

/*****************************************************************************
 *                                CACHE TYPES                                *
 *****************************************************************************/

// cache constants
#define CACHE_BLOCK_SIZE_BYTES 64
#define CACHE_NUM_SETS 16
#define NUM_BITS_BYTE_OFFSET_IN_BLOCK (nbits_required(CACHE_BLOCK_SIZE_BYTES))
#define NUM_BITS_SET_INDEX (nbits_required(CACHE_NUM_SETS))

// states a single state cache block can be in
enum CACHE_BLOCK_STATE
{
    STATE_INVALID,
    STATE_CLEAN,
    STATE_DIRTY
};

// cache block definition
struct cache_block
{
    enum CACHE_BLOCK_STATE state;
    uint64_t tag;
    uint8_t data[CACHE_BLOCK_SIZE_BYTES]; // Data stored in the cache. Copy of data from main memory which was fetched into cache. 
};

#endif /* TYPES_H */
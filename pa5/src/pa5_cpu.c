#include "pa5_cpu.h"

/*****************************************************************************
 *                      PROVIDED UTILITY FUNCTIONS                           *
 *****************************************************************************/
// extract the immediate from an instruction
uint64_t extract_immediate(uint32_t inst)
{
    uint32_t opcode = extract_bits(inst, 6, 0);
    uint64_t imm = 0;

    switch(opcode)
    {
        case RV_OPCODE_OP: return 0;

        case RV_OPCODE_OP_IMM:
        case RV_OPCODE_LOAD:
            imm = extract_bits(inst, 31, 20);
            return imm >> 11 ? (-1ul << 11) | imm : imm;

        case RV_OPCODE_STORE:
            imm = (extract_bits(inst, 31, 25) << 5) | extract_bits(inst, 11, 7);
            return imm >> 11 ? (-1ul << 11) | imm : imm;

        case RV_OPCODE_BRANCH:
            imm = (extract_bits(inst, 31, 31) << 12) | (extract_bits(inst, 7, 7) << 11) | (extract_bits(inst, 30, 25) << 5) | (extract_bits(inst, 11, 8) << 1);
            return imm >> 12 ? (-1ul << 12) | imm : imm;

        default:
            printf("error: cannot extract immediate from instruction of unknown itype\n");
            return 0;
    }
}

// extract some range of bits from an instruction (indices are right to left)
uint16_t extract_bits(uint32_t inst, uint8_t msb_idx, uint8_t lsb_idx)
{
    return (inst >> lsb_idx) & ((1u << (msb_idx - lsb_idx + 1)) - 1);
}

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
uint32_t read_instruction(uint64_t pc, uint8_t (*rd_fn)(uint64_t addr)) 
{
    uint32_t inst = 0; 
    for (int i = 0; i < 4; i++){ //0-3
        inst |= (uint32_t)rd_fn(pc + i)<<(8 * i); //rd_fn at most 8 bytes so, must load 4 times.
    }
    return inst;
}

// use the function pointer rd_fn to read the dword at the given memory address
uint64_t read_dword(uint64_t addr, uint8_t (*rd_fn)(uint64_t addr))
{
    uint64_t dwordret = 0; 
    for (int i = 0; i < 8; i++){ //0-7
        dwordret |= (uint64_t)rd_fn(addr + i)<<(8 * i); // Read each byte from memory. 
    }
    return dwordret;
}

// use the function pointer wr_fn to write the dword "data" to the given memory address
void write_dword(uint64_t addr, uint64_t data, void (*wr_fn)(uint64_t addr, uint8_t data))
{
    for (int i = 0; i < 8; i++){ //0-7
        uint8_t byteExtract = (data >> (8*i)) & 0xFF;
        wr_fn(addr + i, byteExtract);
    }
}

/*****************************************************************************
 *                                                                           *
 *                   REQUIRED + GRADED FUNCTIONS                             *
 *                                                                           *
 *****************************************************************************/

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
void pa5_fetch(uint64_t pc, uint8_t (*mem_read_byte)(uint64_t addr), struct fetch_outputs *out) // Ptr to struct. // Done 4-27-2025 1:10 AM
{
    out->pc = pc;
    out->inst = read_instruction(pc, mem_read_byte); //mem_read_byte == rd_fn in read instruction.
}

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
enum ALU_OP get_aluop(uint32_t inst) 
{
    uint8_t funct3 = extract_bits(inst, 14, 12);
    uint8_t funct7 = extract_bits(inst, 31, 25); 
    uint8_t opcode = extract_bits(inst, 6, 0);
    if (opcode == RV_OPCODE_OP){
        if (funct3 == FUNCT3_ADD_SUB){ // If add sub, automatically assume R-Type
            if (funct7 == FUNCT7_ADD){
                return ALU_OP_ADD;
            } 
            return ALU_OP_SUB;
        } else if (funct3 == FUNCT3_XOR){
            return ALU_OP_XOR;
        } else if (funct3 == FUNCT3_OR){
            return ALU_OP_OR;
        } else if (funct3 == FUNCT3_AND){
            return ALU_OP_AND;
        }
    } else if (opcode == RV_OPCODE_OP_IMM){
        if (funct3 == FUNCT3_ADD_SUB) {
            return ALU_OP_ADD;
        } else if (funct3 == FUNCT3_XOR){
            return ALU_OP_XOR;
        } else if (funct3 == FUNCT3_OR){
            return ALU_OP_OR;
        } else if (funct3 == FUNCT3_AND){
            return ALU_OP_AND;
        }
    } else {
        return ALU_OP_ADD; // For branch, load, save
    }
    return ALU_OP_NONE;
}

/*
 * decode the branch condition from the instruction
 *
 * inputs:
 *     inst: the instruction to decode
 * outputs:
 *     out: the branch condition (enums are defined in types.h)
 */
enum BR_COND get_br_cond(uint32_t inst)
{
    uint8_t funct3 = extract_bits(inst, 14, 12); // Both helpers completed 2:20 am 4/27/2025
    uint8_t opcode = extract_bits(inst, 6, 0);
    if (opcode != RV_OPCODE_BRANCH){
        return BR_COND_NEVER;
    } else if (funct3 == FUNCT3_BEQ){
        return BR_COND_EQ;
    } else if (funct3 == FUNCT3_BNE){
        return BR_COND_NEQ;
    } else if (funct3 == FUNCT3_BLT){
        return BR_COND_LT;
    } else if (funct3 == FUNCT3_BGE){
        return BR_COND_GE;
    } else if (funct3 == FUNCT3_BLTU){
        return BR_COND_LTU;
    }else if (funct3 == FUNCT3_BGEU){
        return BR_COND_GEU;
    }
    // Don't include branch always, no jump or ret. May check back on this later. 
    return BR_COND_NEVER; // Idk where to incorporate always
}

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
void pa5_decode(const struct fetch_outputs *in, const uint64_t *gprs, struct decode_outputs *out) // Do Tomorrow. Gprs is array of size 32, index 0 - 31. // Done as of Now 4/27/2025 5:29 PM. 
{
    // General Reminders: OP/OP_32 deal with 2 register and arithmetic betweetn 2 registers, are R-type? OP_imm are I type and deal with a register/imm arithmetic. 
    out->alu_op = get_aluop(in->inst);
    out->br_cond = get_br_cond(in->inst);
    uint8_t opcode = extract_bits(in->inst, 6, 0);
    if (opcode == RV_OPCODE_OP) {
        uint8_t rs1 = extract_bits(in->inst, 19, 15);
        uint8_t rs2 = extract_bits(in->inst, 24, 20);
        uint8_t rd = extract_bits(in->inst, 11, 7);
        for (uint64_t  i = X0_ZERO; i < NUM_GPRS; i++){
            if (rd == i){
                out->rd_idx = (enum GPR_IDX)i;
                break;
            }
        }
        out->rs1 = gprs[rs1]; // Data
        out->rs2 = gprs[rs2]; // Data
        out->a_sel = A_SEL_RS1;
        out->b_sel = B_SEL_RS2;
        out->wb_sel = WB_SEL_ALU_RESULT; // All enums are forwarded signals. 
        out->mem_read_en = false;
        out->mem_write_en = false;
        out->reg_write_en = true;
    } else if (opcode == RV_OPCODE_OP_IMM) {
        uint8_t rs1 = extract_bits(in->inst, 19, 15);
        out->imm = extract_immediate(in->inst);
        uint8_t rd = extract_bits(in->inst, 11, 7);
        for (uint8_t i = X0_ZERO; i < NUM_GPRS; i++){
            if (rd == i){
                out->rd_idx = (enum GPR_IDX)i;
                break;
            }
        }
        out->rs1 = gprs[rs1]; // Data
        out->a_sel = A_SEL_RS1;
        out->b_sel = B_SEL_IMM;
        out->wb_sel = WB_SEL_ALU_RESULT; // All enums are forwarded signals. 
        out->mem_read_en = false;
        out->mem_write_en =false;
        out->reg_write_en = true;
    } else if (opcode == RV_OPCODE_LOAD) {
        uint8_t rs1 = extract_bits(in->inst, 19, 15);
        out->imm = extract_immediate(in->inst);
        uint8_t rd = extract_bits(in->inst, 11, 7);  // Only has rs1 and imm
        for (uint8_t i = X0_ZERO; i < NUM_GPRS; i++){
            if (rd == i){
                out->rd_idx = (enum GPR_IDX)i;
                break;
            }
        }
        out->rs1 = gprs[rs1]; // Data
        out->a_sel = A_SEL_RS1;
        out->b_sel = B_SEL_IMM;
        out->wb_sel = WB_SEL_MEM_READ_DATA; // All enums are forwarded signals. 
        out->mem_read_en = true;
        out->mem_write_en = false;
        out->reg_write_en = true;
    } else if (opcode == RV_OPCODE_STORE) {
        uint8_t rs1 = extract_bits(in->inst, 19, 15);
        uint8_t rs2 = extract_bits(in->inst, 24, 20);
        out->imm = extract_immediate(in->inst); 
        uint8_t rd = rs2; // Save rd/rs2 value to rs1 + imm -> address. 
        for (uint8_t i = X0_ZERO; i < NUM_GPRS; i++){
            if (rd == i){
                out->rd_idx = (enum GPR_IDX)i;
                break;
            }
        }
        out->rs1 = gprs[rs1]; // Data
        out->rs2 = gprs[rs2]; // Data
        out->a_sel = A_SEL_RS1;
        out->b_sel = B_SEL_IMM;
        out->wb_sel = WB_SEL_NONE; // All enums are forwarded signals. 
        out->mem_read_en = false;
        out->mem_write_en = true;
        out->reg_write_en = false;
    } else if (opcode == RV_OPCODE_BRANCH) {
        uint8_t rs1 = extract_bits(in->inst, 19, 15);
        uint8_t rs2 = extract_bits(in->inst, 24, 20);
        out->imm = extract_immediate(in->inst); // Add imm to pc if branch successful. Compare rs1 and rs2 for branch. 
        // uint64_t rd = extract_bits(in->inst, 11, 7); **branch has no rd**
        // for (int i = X0_ZERO; i < NUM_GPRS; i++){ // Assigns rd to correct register. 
        //     if (rd == i){
        //         out->rd_idx = (enum GPR_IDX)i;
        //         break;
        //     }
        // }
        out->rs1 = gprs[rs1]; // Data
        out->rs2 = gprs[rs2]; // Data
        out->a_sel = A_SEL_PC;
        out->b_sel = B_SEL_IMM;
        out->wb_sel = WB_SEL_NONE; // All enums are forwarded signals. 
        out->mem_read_en = false; // 0
        out->mem_write_en = false;
        out->reg_write_en = false;
    }
    out->pc = in->pc;
}

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
void pa5_agex(const struct decode_outputs *in, struct agex_outputs *out) // Done as of now, 4/29/2025 9:37 PM
{   // A Sel and B sel are inputs into operations. 
    if (in->br_cond == BR_COND_NEVER) {
        if (in->alu_op == ALU_OP_ADD){ // Automatically ADD for branch. 
            if (in->a_sel == A_SEL_RS1 && in->b_sel == B_SEL_RS2){
                out->alu_result = in->rs1 + in->rs2;
                out->wb_sel = WB_SEL_ALU_RESULT;
            } else if (in->a_sel == A_SEL_RS1 && in->b_sel == B_SEL_IMM){
                out->alu_result = in->rs1 + in->imm;
                if (in->mem_read_en == true) {
                    out->wb_sel = WB_SEL_MEM_READ_DATA; // Load
                } else if (in->mem_write_en == false && in->mem_read_en == false) {
                    out->wb_sel = WB_SEL_ALU_RESULT; // Immediate
                } else {
                    out->wb_sel = WB_SEL_NONE; // Store
                }
            }
        } else if (in->alu_op == ALU_OP_SUB){
            out->alu_result = in->rs1 - in->rs2;
            out->wb_sel = WB_SEL_ALU_RESULT;
        } else if (in->alu_op == ALU_OP_XOR){
            if (in->a_sel == A_SEL_RS1 && in->b_sel == B_SEL_RS2){
                out->alu_result = in->rs1 ^ in->rs2;
                out->wb_sel = WB_SEL_ALU_RESULT;
            } else if (in->a_sel == A_SEL_RS1 && in->b_sel == B_SEL_IMM){
                out->alu_result = in->rs1 ^ in->imm;
                out->wb_sel = WB_SEL_ALU_RESULT;
            }
        } else if (in->alu_op == ALU_OP_OR){
            if (in->a_sel == A_SEL_RS1 && in->b_sel == B_SEL_RS2){
                out->alu_result = in->rs1 | in->rs2;
                out->wb_sel = WB_SEL_ALU_RESULT;
            } else if (in->a_sel == A_SEL_RS1 && in->b_sel == B_SEL_IMM){
                out->alu_result = in->rs1 | in->imm;
                out->wb_sel = WB_SEL_ALU_RESULT;
            }
        } else if (in->alu_op == ALU_OP_AND){
            if (in->a_sel == A_SEL_RS1 && in->b_sel == B_SEL_RS2){
                out->alu_result = in->rs1 & in->rs2;
                out->wb_sel = WB_SEL_ALU_RESULT;
            } else if (in->a_sel == A_SEL_RS1 && in->b_sel == B_SEL_IMM){
                out->alu_result = in->rs1 & in->imm;
                out->wb_sel = WB_SEL_ALU_RESULT;
            }
        }
        out->pc_sel = PC_SEL_PC_PLUS_4;
    } else if (in->br_cond == BR_COND_EQ) {
        out->alu_result = in->pc + in->imm;
        if (in->rs1 == in->rs2){
            out->pc_sel = PC_SEL_ALU_RESULT; // In the case condition passes. 
        } else {
            out->pc_sel = PC_SEL_PC_PLUS_4;
        }
    } else if (in->br_cond == BR_COND_GE) {
        out->alu_result = in->pc + in->imm;
        if ((int64_t)in->rs1 >= (int64_t)in->rs2){
            out->pc_sel = PC_SEL_ALU_RESULT; // In the case condition passes. 
        } else {
            out->pc_sel = PC_SEL_PC_PLUS_4;
        }
    } else if (in->br_cond == BR_COND_GEU) {
        out->alu_result = in->pc + in->imm;
        if (in->rs1 >= in->rs2){
            out->pc_sel = PC_SEL_ALU_RESULT; // In the case condition passes.
        } else {
            out->pc_sel = PC_SEL_PC_PLUS_4;
        }
    } else if (in->br_cond == BR_COND_LT) {
        out->alu_result = in->pc + in->imm;
        if ((int64_t)in->rs1 < (int64_t)in->rs2){
            out->pc_sel = PC_SEL_ALU_RESULT; // In the case condition passes. 
        } else {
            out->pc_sel = PC_SEL_PC_PLUS_4;
        }
    } else if (in->br_cond == BR_COND_LTU) {
        out->alu_result = in->pc + in->imm;
        if (in->rs1 < in->rs2){
            out->pc_sel = PC_SEL_ALU_RESULT; // In the case condition passes. 
        } else {
            out->pc_sel = PC_SEL_PC_PLUS_4;
        }
    } else if (in->br_cond == BR_COND_NEQ) {
        out->alu_result = in->pc + in->imm;
        if (in->rs1 != in->rs2){
            out->pc_sel = PC_SEL_ALU_RESULT; // In the case condition passes. 
        } else {
            out->pc_sel = PC_SEL_PC_PLUS_4;
        }
    } else if (in->br_cond == BR_COND_ALWAYS) { // Always true, reps ret for example. 
            out->alu_result = in->pc + in->imm;  
            out->pc_sel = PC_SEL_ALU_RESULT; // In the case condition passes. 
            out->pc = out->alu_result;
    }
    out->pc = in->pc;
    out->wb_sel = in->wb_sel;
    out->mem_read_en =  in->mem_read_en;
    out->mem_write_en = in->mem_write_en;
    out->reg_write_en = in->reg_write_en;
    out->rd_idx = in->rd_idx; // register destination idx remains the same. 
    out->rs2 = in->rs2; // rs2 remains the same for now. 
}

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
void pa5_mem(const struct agex_outputs *in, // Use dword functions here w mem read byte as inputs to both for function ptrs. Ask about cpu design for signals, if modified or remain the same.
    uint8_t (*mem_read_byte)(uint64_t addr), // use rs2 for dword and dwrite? 
    void (*mem_write_byte)(uint64_t addr, uint8_t data), // Load -> read data from memory Store -> write data to memory. Ops -> pass along. 
    struct mem_outputs *out, // pc is address.
    bool use_cache)
{   // Cache used when reading and writing memory ie enabled, not used when not enabled or working with registers.
    if (use_cache == true){ // Consider cases when it misses. (How do we indicate when a miss occurs?) 
        if (in->mem_read_en == true) { // Command line that enables or disables cache. 
            out->mem_rdata = cache_read(in->alu_result, mem_read_byte, mem_write_byte); // **DWORD USED FOR MEMORY ONLY, USE ALU RESULT NOT PC** 
        } if (in->mem_write_en == true) {
            cache_write(in->alu_result, in->rs2, mem_read_byte, mem_write_byte); // Need to implement cache write and read. 
        }
    } else { // Read data from memory/write  if cache disabled. 
        if (in->mem_read_en == true) {
            out->mem_rdata = read_dword(in->alu_result, mem_read_byte);
        } if (in->mem_write_en == true) {
            write_dword(in->alu_result, in->rs2, mem_write_byte);
        }
    }
    out->pc_sel = in->pc_sel;
    out->wb_sel = in->wb_sel;
    out->rd_idx = in->rd_idx;
    out->reg_write_en = in->reg_write_en;
    out->alu_result = in->alu_result;
    out->pc = in->pc; // Finished 4/29/2025 9:40 PM -> may return to edit/debug more. 
}

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
void pa5_writeback(const struct mem_outputs *in, uint64_t *gprs, struct writeback_outputs *out) // Finished as of 4/29/2025 10:47 pm, will check back on it soon. 
{
    if (in->reg_write_en == true) {
        if (in->wb_sel == WB_SEL_ALU_RESULT){
            gprs[in->rd_idx] = in->alu_result;
        } else if (in->wb_sel == WB_SEL_MEM_READ_DATA) {
            gprs[in->rd_idx] = in->mem_rdata;
        } 
    }
    if (in->pc_sel == PC_SEL_ALU_RESULT){
        out->next_pc = in->alu_result; // increment pc by imm. 
    } else if (in->pc_sel == PC_SEL_PC_PLUS_4 || in->pc_sel == PC_SEL_NONE) {
        out->next_pc = in->pc + 4; // increment pc by 4 for next instruction for fetch. 
    } 
}

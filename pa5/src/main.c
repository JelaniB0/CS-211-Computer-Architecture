#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>

#include "simulator/sim.h"

/*
 * Example assertion-based tests for the functions you need to implement in `pa5_cache.c` 
 *
 * TODO: extend this with your own test cases!
 */

void test_cache(uint8_t (*mem_read_byte)(uint64_t addr), void (*mem_write_byte)(uint64_t addr, uint8_t data))
{
    /*************************************************************************
     * test cache indexing
     *************************************************************************/
    // test extracting bits from the address for indexing into the cache and tag matching
    uint64_t address = 0xfff;
    assert(get_byte_offset_within_cache_block(address) == 0x3f);
    assert(get_cache_set_index(address) == 0xf);
    assert(get_cache_block_tag(address) == 0x3);

    // initialize the cache to some known state
    for(int i = 0; i < CACHE_NUM_SETS; i++)
    {
        struct cache_block *block = &g_cache[i];
        block->state = STATE_CLEAN;
        block->tag = 0x0;
        for(int byte_in_block = 0; byte_in_block < CACHE_BLOCK_SIZE_BYTES; byte_in_block++)
            block->data[byte_in_block] = (uint8_t)(byte_in_block + i * CACHE_BLOCK_SIZE_BYTES);
    }

    /*************************************************************************
     * test cache_read/cache_write
     *************************************************************************/
    address = 0;
    cache_write(address, 0x1234, mem_read_byte, mem_write_byte);
    assert(cache_read(address, mem_read_byte, mem_write_byte) == 0x1234);
}

/*
 * Example assertion-based tests for the functions you need to implement in `pa5_cpu.c` 
 *
 * TODO: extend this with your own test cases!
 */
 void test_cpu(struct simulation_state *sim_state)
{
    /*************************************************************************
     * test fetch
     *************************************************************************/
    uint64_t pc = 123901234; // random address
    sim_state->mem_write_byte(pc, 0x78); 
    sim_state->mem_write_byte(pc + 1, 0x56);
    sim_state->mem_write_byte(pc + 2, 0x34);
    sim_state->mem_write_byte(pc + 3, 0x12);

    struct fetch_outputs f_out;
    pa5_fetch(pc, sim_state->mem_read_byte, &f_out);
    assert(f_out.pc == pc);
    assert(f_out.inst == 0x12345678);

    /*************************************************************************
     * test decode
     *************************************************************************/
    struct decode_outputs d_out;

    // addi   s2, zero, -1
    f_out.pc = 0x1055c; // random address
    f_out.inst = 0xfff00913;
    memset(sim_state->gprs, 0, sizeof(sim_state->gprs)); // clear all registers
    pa5_decode(&f_out, sim_state->gprs, &d_out);
    assert(d_out.rd_idx == X18_S2);
    assert(d_out.alu_op == ALU_OP_ADD);
    assert(d_out.br_cond == BR_COND_NEVER);
    assert(d_out.wb_sel == WB_SEL_ALU_RESULT);
    assert(d_out.a_sel == A_SEL_RS1);
    assert(d_out.b_sel == B_SEL_IMM);
    assert(d_out.reg_write_en == true);
    assert(d_out.mem_read_en == false);
    assert(d_out.mem_write_en == false);
    assert(d_out.rs1 == 0);
    // assert(d_out.rs2 == /* DONT CARE */);
    assert(d_out.imm == 0xffffffffffffffff);
    assert(d_out.pc == 0x1055c);

    /*************************************************************************
     * test agex
     *************************************************************************/
    struct agex_outputs a_out;

    // bltu x10, x11, 10
    d_out.rd_idx = X0_ZERO;
    d_out.alu_op = ALU_OP_ADD;
    d_out.br_cond = BR_COND_LTU;
    d_out.wb_sel = WB_SEL_NONE;
    d_out.a_sel = A_SEL_PC;
    d_out.b_sel = B_SEL_IMM;
    d_out.reg_write_en = false;
    d_out.mem_read_en = false;
    d_out.mem_write_en = false;
    d_out.rs1 = 0x1000000000000000; // arbitrary value
    d_out.rs2 = 0x8111111111111111; // arbitrary value
    d_out.imm = 10; // random number
    d_out.pc = 0x98765432;
    pa5_agex(&d_out, &a_out);
    assert(a_out.pc_sel == PC_SEL_ALU_RESULT);
    assert(a_out.rd_idx == X0_ZERO);
    assert(a_out.wb_sel == WB_SEL_NONE);
    assert(a_out.reg_write_en == false);
    assert(a_out.mem_read_en == false);
    assert(a_out.mem_write_en == false);
    assert(a_out.alu_result == 0x9876543c); // branch target
    assert(a_out.rs2 == 0x8111111111111111);
    // printf("[DEBUG] pc set to branch target: %lx\n", a_out.pc);
    assert(a_out.pc == 0x98765432);

    /*************************************************************************
     * test mem
     *************************************************************************/
    struct mem_outputs m_out;
    uint64_t addr = 0xa28c94fb823d45c0; // random number
    uint64_t data = 0xfedcba9876543210; // random data

    // initialize memory with the data for a load operation
    sim_state->mem_write_byte(addr + 0, (data >> 0x00) & 0xff);
    sim_state->mem_write_byte(addr + 1, (data >> 0x08) & 0xff);
    sim_state->mem_write_byte(addr + 2, (data >> 0x10) & 0xff);
    sim_state->mem_write_byte(addr + 3, (data >> 0x18) & 0xff);
    sim_state->mem_write_byte(addr + 4, (data >> 0x20) & 0xff);
    sim_state->mem_write_byte(addr + 5, (data >> 0x28) & 0xff);
    sim_state->mem_write_byte(addr + 6, (data >> 0x30) & 0xff);
    sim_state->mem_write_byte(addr + 7, (data >> 0x38) & 0xff);

    // ld x22, 0(addr)
    a_out.pc_sel = PC_SEL_PC_PLUS_4;
    a_out.rd_idx = X22_S6;
    a_out.wb_sel = WB_SEL_MEM_READ_DATA;
    a_out.reg_write_en = true;
    a_out.mem_read_en = true;
    a_out.mem_write_en = false;
    a_out.alu_result = addr;
    a_out.rs2 = 0x1111111111111111; // random number
    a_out.pc = 0x1122334455667788; // random number
    pa5_mem(&a_out, sim_state->mem_read_byte, sim_state->mem_write_byte, &m_out, sim_state->use_cache);
    assert(m_out.rd_idx == X22_S6);
    assert(m_out.wb_sel == WB_SEL_MEM_READ_DATA);
    assert(m_out.pc_sel == PC_SEL_PC_PLUS_4);
    assert(m_out.reg_write_en == true);
    assert(m_out.alu_result == addr);
    assert(m_out.mem_rdata == data);
    assert(m_out.pc == 0x1122334455667788);

    /*************************************************************************
     * test writeback
     *************************************************************************/
    struct writeback_outputs w_out;
    memset(sim_state->gprs, 0, sizeof(sim_state->gprs)); // clear all registers

    // sd x10, 0(rs1)
    m_out.rd_idx = X10_A0;
    m_out.wb_sel = WB_SEL_NONE;
    m_out.pc_sel = PC_SEL_PC_PLUS_4;
    m_out.reg_write_en = false;
    m_out.alu_result = 42;
    m_out.mem_rdata = 196;
    m_out.pc = 0x9876543287654320;
    pa5_writeback(&m_out, sim_state->gprs, &w_out);
    assert(w_out.next_pc == 0x9876543287654324);
    for(int i = 0; i < NUM_GPRS; i++)
        assert(sim_state->gprs[i] == 0);
}


int main(int argc, char **argv)
{
    if(argc < 2)
    {
        puts("usage:");
        puts("    simulate a program........................: ./pa5 <program to simulate, e.g., /tests/file.txt> [optional : -with-cache]");
        puts("    run assertion-based tests for the datapath: ./pa5 -test-datapath [optional : -with-cache]");
        puts("    run assertion-based tests for the cache...: ./pa5 -test-cache");
        return EXIT_FAILURE;
    }

    const char* filename = NULL;
    bool use_cache = false, 
         run_datapath_test = false,
         run_cache_test = false;

    for (int i = 1; i < argc; i++)
    {
        if(strcmp(argv[i], "-with-cache") == 0) use_cache = true;
        else if(strcmp(argv[i], "-test-datapath") == 0) run_datapath_test = true;
        else if(strcmp(argv[i], "-test-cache") == 0) run_cache_test = true;
        else if(strncmp(argv[i], "-", 1) == 0)
        {
            printf("error: unrecognized command-block switch: \"%s\"\n", argv[i]);
            return EXIT_FAILURE;
        }
        else filename = argv[i];
    }

    if(run_datapath_test) 
    {
        struct simulation_state sim_state;
        sim_state.use_cache = use_cache;
        sim_mem_init(&sim_state.mem_read_byte, &sim_state.mem_write_byte);
        
        test_cpu(&sim_state);
    }
    else if(run_cache_test)
    {
        uint8_t (*mem_read_byte)(uint64_t addr);
        void (*mem_write_byte)(uint64_t addr, uint8_t data);
        sim_mem_init(&mem_read_byte, &mem_write_byte);

        test_cache(mem_read_byte, mem_write_byte);
    }
    else 
        return run_simulator(filename, use_cache);

    return EXIT_SUCCESS;
}

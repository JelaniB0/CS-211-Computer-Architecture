#ifndef SIMULATE_H
#define SIMULATE_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>

#include "sim_mem.h"
#include "../pa5_cpu.h"

#define SIM_BUF_LEN 257

enum MACHINE_STATE
{
      STATE_HALT = 0
    , STATE_RUNNING
    , STATE_ERROR
};

struct simulation_state
{
    uint64_t gprs[NUM_GPRS]; // the general-purpose registers (x0 - x31)
    uint64_t pc; // the program counter register
    uint32_t ir; // the instruction register
    uint64_t num_instructions_executed;
    enum MACHINE_STATE state;
    uint8_t (*mem_read_byte)(uint64_t addr);
    void (*mem_write_byte)(uint64_t addr, uint8_t data);
    bool use_cache;
};

void sim_init(const char *filename, bool use_cache, struct simulation_state *sim_state) ;
void sim_fini(void);
void sim_print_regs(struct simulation_state *sim_state);
void sim_print_mem(uint64_t addr, uint64_t num_bytes, struct simulation_state *sim_state);
void sim_step(struct simulation_state *sim_state);

int run_simulator(const char *filename, bool use_cache);

#endif /* SIMULATE_H */

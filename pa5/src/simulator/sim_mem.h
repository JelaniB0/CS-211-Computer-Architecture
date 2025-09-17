#ifndef SIM_MEM_H
#define SIM_MEM_H

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>

/*
 * the simulator will stop after trying to allocate more than 2 MiB of memory
 * this is helpful for debugging infinite loops
 *
 * you may want to change this if you are testing gigantic allocations
 */
#define MAX_NUM_BYTES_ALLOCATABLE (2 * 1024 * 1024)

/*****************************************************************************
 *                              MAIN MEMORY
 *****************************************************************************/

/* initialize and de-initialize the simulator's memory interface */
void sim_mem_init(uint8_t (**mem_read_byte)(uint64_t addr), void (**mem_write_byte)(uint64_t addr, uint8_t data));
void sim_mem_fini(void);
int sim_mem_is_initialized(void);

#endif /* SIM_MEM_H */

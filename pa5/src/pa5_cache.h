#ifndef PA5_CACHE_H // Can't use anything in cpu here. 
#define PA5_CACHE_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include <assert.h>

#include "types.h"

// cache declaration
extern struct cache_block g_cache[CACHE_NUM_SETS];

/*
 * calculates which byte position in a cache block the address points to (i.e.,
 * extracts the byte offset into the cache block)
 */
uint64_t get_byte_offset_within_cache_block(uint64_t address);

/*
 * calculates which cache set the address belongs to (i.e., extracts the set
 * index field of the address)
 */
uint64_t get_cache_set_index(uint64_t address);

/*
 * returns the cache block tag (i.e., extracts the tag field of the address)
 */
uint64_t get_cache_block_tag(uint64_t address);

/*
 * copies a cache block's worth of data from main memory into the cache
 * 
 * inputs:
 *     block: the cache block to fill
 *     address: the memory address that the original ld/sd instruction attempted to access
 *     mem_read_byte: a function for reading one byte from memory at a specific address
 * assumptions:
 *     address will be a multiple of 8 bytes
 */
 void cache_block_fill(struct cache_block *block, uint64_t address, uint8_t (*mem_read_byte)(uint64_t addr));

/*
 * copies a cache block's worth of data from the cache into main memory
 * 
 * inputs:
 *     block: the cache block to write back to memory
 *     address: the memory address that the original ld/sd instruction attempted to access
 *     mem_write_byte: a function for writing one byte to memory at a specific address
 * assumptions:
 *     address will be a multiple of 8 bytes
 */
 void cache_block_writeback(struct cache_block *block, uint64_t address, void (*mem_write_byte)(uint64_t addr, uint8_t data));

/*
 * reads a dword (64 bits) from the cache, performing any necessary writeback or
 * fill operations to ensure that dirty data is not lost and that the correct
 * data is accessed.
 *
 * inputs:
 *     address: the address of the dword being read
 *     mem_read_byte: a function pointer to a function that allows reading a single byte from memory
 *     mem_write_byte: a function pointer to a function that allows writing a single byte to memory
 * outputs:
 *     out: the desired dword (either coming from cache or memory)
 * assumptions:
 *     address will be a multiple of 8 bytes
 */
uint64_t cache_read(uint64_t address, uint8_t (*mem_read_byte)(uint64_t addr), void (*mem_write_byte)(uint64_t addr, uint8_t data));

/*
 * writes a dword (64 bits) to the cache, performing any necessary writeback or
 * fill operations to ensure that dirty data is not lost and that the correct
 * data is accessed.
 *
 * inputs:
 *     address: the address of the dword being written
 *     data: the dword to be written
 *     mem_read_byte: a function pointer to a function that allows reading a single byte from memory
 *     mem_write_byte: a function pointer to a function that allows writing a single byte to memory
 * assumptions: 
 *     address will be a multiple of 8 bytes
 */
void cache_write(uint64_t address, uint64_t data, uint8_t (*mem_read_byte)(uint64_t addr), void (*mem_write_byte)(uint64_t addr, uint8_t data));

#endif /* PA5_CACHE_H */

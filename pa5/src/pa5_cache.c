#include "pa5_cache.h"

/*****************************************************************************
 *                      PROVIDED UTILITY FUNCTIONS                           *
 *****************************************************************************/
/*
 * checks whether x is a power of 2 (exculding zero)
 */
bool is_po2(uint64_t x)
{
    return (x != 0) && ((x & (x - 1)) == 0);
}

/*
 * computes how many bits are required to represent the given unsigned number 
 */
int64_t nbits_required(uint64_t x)
{
    int64_t r = 0;
    while(x >>= 1)
        r++;
    return r;
}

/*****************************************************************************
 *                   RECOMMENDED UTILITY FUNCTIONS                           *
 *                                                                           *
 * We recommend you implement these to make the fetch() and memory() stage   *
 * cleaner. These will not be graded, but we provide some example assertion- *
 * based tests for them in `main.c`                                          *
 *****************************************************************************/
/*
 * calculates which byte position in a cache block the address points to (i.e.,
 * extracts the byte offset into the cache block)
 */
// Bits needed to extract offset -> 6.  
uint64_t get_byte_offset_within_cache_block(uint64_t address)
{
    if(is_po2(CACHE_BLOCK_SIZE_BYTES) == true){ // I guess we can't assume cache block size is power of 2. 
        uint64_t bitMask = (1ULL << NUM_BITS_BYTE_OFFSET_IN_BLOCK) - 1;
        return address & bitMask; // Bit offset. 
    }
    return 0; // In case block size not a power of 2 for whatever reason. 
}

/*
 * calculates which cache set the address belongs to (i.e., extracts the set
 * index field of the address)
 */
uint64_t get_cache_set_index(uint64_t address)
{
    return (address >> NUM_BITS_BYTE_OFFSET_IN_BLOCK) & ((1ULL << NUM_BITS_SET_INDEX) - 1); // Grabbing bits for cache set index.

}

/*
 * returns the cache block tag (i.e., extracts the tag field of the address)
 */
uint64_t get_cache_block_tag(uint64_t address) 
{
    return address >> (NUM_BITS_BYTE_OFFSET_IN_BLOCK + NUM_BITS_SET_INDEX); // Grabbing bits for block tag. 
}

uint64_t originalAddress(uint64_t setIndex, uint64_t tag) { // We need to write back to original memory address. 
    return (tag << (NUM_BITS_BYTE_OFFSET_IN_BLOCK + NUM_BITS_SET_INDEX)) | (setIndex << NUM_BITS_BYTE_OFFSET_IN_BLOCK); 
}
/*****************************************************************************
 *                   REQUIRED + GRADED FUNCTIONS                             *
 *****************************************************************************/
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
//use for cache misses
void cache_block_fill(struct cache_block *block, uint64_t address, uint8_t (*mem_read_byte)(uint64_t addr)) // Relatively short, finished 5/2/2025 1:03 AM.
{
    uint64_t alignAddr = address & ~(CACHE_BLOCK_SIZE_BYTES - 1); // Make sure address correctly aligned, only possible thing I can imagine causing error for now. 
    for (int i = 0; i < CACHE_BLOCK_SIZE_BYTES; i++){ // Access specific mem location corresponding to each byte. 
        block->data[i] = mem_read_byte(alignAddr + i);
    }
    block->state = STATE_CLEAN; // Updating data of cache block. 
    block->tag = get_cache_block_tag(address);
}

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
void cache_block_writeback(struct cache_block *block, uint64_t address, void (*mem_write_byte)(uint64_t addr, uint8_t data)) // Finished 1:31 PM 5/3/2025
{
    uint64_t originalAddr = originalAddress(get_cache_set_index(address), block->tag);
    for (uint64_t i = 0; i < CACHE_BLOCK_SIZE_BYTES; i++){ //0-7
        mem_write_byte(originalAddr + i, block->data[i]); 
    }
    block->state = STATE_CLEAN; // Updating data of cache block. 
}

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
uint64_t cache_read(uint64_t address, uint8_t (*mem_read_byte)(uint64_t addr), void (*mem_write_byte)(uint64_t addr, uint8_t data)) // Finished 4:20 PM 5/3/2025
{
    uint64_t setIndex = get_cache_set_index(address);
    uint64_t cacheOffset = get_byte_offset_within_cache_block(address); // Use for data
    uint64_t addressTag = get_cache_block_tag(address);
    // **Create a struct called block**
    struct cache_block *block;
    block = &g_cache[setIndex]; // Affects actual address of whatever cache block is at set index. 
    switch(block->state)
    {
        case STATE_CLEAN:
            if (addressTag == block->tag){
                break;
            } else { // Miss Case
                cache_block_fill(block, address, mem_read_byte);
            }
            break;
        case STATE_DIRTY:
            if (addressTag == block->tag){
                break;
            } else {
                cache_block_writeback(block, address, mem_write_byte); // Need to write data from cache back to original memory address first.       
                cache_block_fill(block, address, mem_read_byte);
            }
            break;
        case STATE_INVALID: // Block is invalid, no data stored in that cache block so there is no tag. 
            cache_block_fill(block, address, mem_read_byte);
            break;
        default:
            printf("error: unrecognized cache block state: %d\n", block->state);
            return 0;
    }
    uint64_t readData = 0;
    for (int i = 0; i < 8; i++){
        readData |= ((uint64_t)block->data[cacheOffset + i]) << (8 * i); // Similar to read_dword in pa5cpu
    }
    // printf("[DEBUG]Read Data: 0x%lx\n", readData); // Works now. 
    return readData;
}

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
void cache_write(uint64_t address, uint64_t data, uint8_t (*mem_read_byte)(uint64_t addr), void (*mem_write_byte)(uint64_t addr, uint8_t data)) // Finished at 4:20 PM 5/3/2025
{
    uint64_t setIndex = get_cache_set_index(address);
    uint64_t cacheOffset = get_byte_offset_within_cache_block(address); // Use for data
    uint64_t addressTag = get_cache_block_tag(address);
    struct cache_block *block;
    block = &g_cache[setIndex];
    switch(block->state)
    {
        case STATE_CLEAN:
            if (addressTag == block->tag){
                break;
            } else { // Miss Case
                cache_block_fill(block, address, mem_read_byte);
            }
            break;
        case STATE_DIRTY: 
            if (addressTag == block->tag){
                break;
            }
                cache_block_writeback(block, address, mem_write_byte); // Need to write data from cache back to original memory address first.       
                cache_block_fill(block, address, mem_read_byte);
            break;
        case STATE_INVALID:
            cache_block_fill(block, address, mem_read_byte);
            break;
        default:
            printf("error: unrecognized cache block state: %d\n", block->state);
            return;
    } // Same as write_dword in pa5cpu. 
    for (int i = 0; i < 8; i++){ //0-7
        block->data[cacheOffset + i] = (data >> (8 * i)) & 0xFF; // Writes data to cache data. 
    }
    block->state = STATE_DIRTY; // State always dirty after write. 
}

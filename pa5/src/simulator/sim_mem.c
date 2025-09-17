#include "sim_mem.h"

/*****************************************************************************
 *          Simple naive hash table implementation for the simulator         *
 *****************************************************************************/
#define HT_NUM_ENTRIES 8192

struct ht_kv_pair_node
{
    uint64_t key;
    uint8_t val;
    struct ht_kv_pair_node *next;
};

struct ht_kv_pair_node **g_ht = NULL;
uint64_t g_num_bytes_allocated = 0;

#define HASH(x) ((x) % HT_NUM_ENTRIES)

void ht_init(void)
{
    g_ht = (struct ht_kv_pair_node **)malloc(sizeof(struct ht_kv_pair_node*) * HT_NUM_ENTRIES);
    if (g_ht == NULL)
    {
        printf("[SIMULATOR ERROR] malloc fail when trying to initialize the simulated main memory\n");
        return;
    }

    for(int i = 0; i < HT_NUM_ENTRIES; i++)
        g_ht[i] = NULL;
    g_num_bytes_allocated = 0;
}

void ll_free(struct ht_kv_pair_node *node)
{
    if(node == NULL)
        return;

    if(node->next != NULL)
        ll_free(node->next);
    free(node);
}

void ht_fini(void)
{
    if(g_ht == NULL)
        return;

    for(int idx = 0; idx < HT_NUM_ENTRIES; idx++)
        ll_free(g_ht[idx]);
    free(g_ht);

    g_num_bytes_allocated = 0;
}

struct ht_kv_pair_node *ht_find(uint64_t key)
{
    uint64_t idx = HASH(key);
    struct ht_kv_pair_node **node = &g_ht[idx];
    while(*node != NULL)
    {
        if((*node)->key == key)
            return *node;
        node = &((*node)->next);
    }


    g_num_bytes_allocated++;
    *node = (struct ht_kv_pair_node *)malloc(sizeof(struct ht_kv_pair_node));
    if(*node == NULL || g_num_bytes_allocated >= MAX_NUM_BYTES_ALLOCATABLE)
    {
        printf("[SIMULATOR ERROR] out of simulated memory! total simulated bytes allocated: %" PRIu64 "\n", g_num_bytes_allocated);
        return NULL;
    }
    (*node)->key = key;
    (*node)->val = 0;
    (*node)->next = NULL;
    return *node;
}

/*****************************************************************************
 *                  Memory interface to the simulator                        *
 *****************************************************************************/
/* get a byte of memory at the supplied 64-bit address */
static uint8_t sim_mem_read_byte(uint64_t address);

/* write a byte to the supplied memory location */
static void sim_mem_write_byte(uint64_t address, uint8_t data);

void sim_mem_init(uint8_t (**mem_read_byte)(uint64_t addr), void (**mem_write_byte)(uint64_t addr, uint8_t data))
{
    if(sim_mem_is_initialized())
    {
        printf("error: memory already initialized\n");
        *mem_read_byte = NULL;
        *mem_write_byte = NULL;
        return;
    }

    ht_init();

    *mem_read_byte = &sim_mem_read_byte;
    *mem_write_byte = &sim_mem_write_byte;
}

void sim_mem_fini(void)
{
    ht_fini();
}

int sim_mem_is_initialized(void)
{
    return g_ht != NULL;
}

static uint8_t sim_mem_read_byte(uint64_t address)
{
    struct ht_kv_pair_node *node = ht_find(address);
    if(node == NULL)
        return 0;
    return node->val;
}

static void sim_mem_write_byte(uint64_t address, uint8_t data)
{
    struct ht_kv_pair_node *node = ht_find(address);
    if(node == NULL)
        return;
    node->val = data;
}

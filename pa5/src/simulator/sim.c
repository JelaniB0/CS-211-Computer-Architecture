#include "sim.h"
#include <stdlib.h>

/* declaration for the cache (exposed in pa5_cache.h) */
struct cache_block g_cache[CACHE_NUM_SETS];

void sim_init(const char *filename, bool use_cache, struct simulation_state *sim_state)
{
    sim_state->use_cache = use_cache;

    // clear all registers
    memset(sim_state, 0, sizeof(struct simulation_state));

    // initialize subcomponents
    sim_mem_init(&sim_state->mem_read_byte, &sim_state->mem_write_byte);

    // open the test file supplied
    FILE *f = fopen(filename, "r");
    if(f == NULL)
    {
        printf("[SIMULATOR ERROR] failed to load input file %s", filename);
        sim_state->state = STATE_ERROR;
        sim_mem_fini();
        return;
    }

    // initialize the simulator state based on the input file line-by-line
    #define SIM_INIT_BUF_SIZE 1024
    char line[SIM_INIT_BUF_SIZE];
    for(uint64_t line_num = 0; fgets(line, SIM_INIT_BUF_SIZE, f) != NULL; line_num++)
    {
        if(!strncmp(line, "sp", 2))
        {
            char *str = line + 2;
            char *end;
            sim_state->gprs[X2_SP] = strtoull(str, &end, 16);
            if(sim_state->gprs[X2_SP] == 0 && end == str)
            {
                printf("[SIMULATOR ERROR] invalid sp on input line \"%s\":%" PRIu64 " = \"%s\"\n", filename, line_num, line);
                goto sim_init_cleanup_on_error;
            }
            else if(sim_state->gprs[X2_SP] == ULLONG_MAX)
                printf("[WARN] sp == ULLONG_MAX on input line \"%s\":%" PRIu64 " = \"%s\"\n", filename, line_num, line);
        }
        else if(!strncmp(line, "pc", 2))
        {
            char *str = line + 2;
            char *end;
            sim_state->pc = strtoull(str, &end, 16);
            if(sim_state->pc == 0 && end == str)
            {
                printf("[SIMULATOR ERROR] invalid pc on input line \"%s\":%" PRIu64 " = \"%s\"\n", filename, line_num, line);
                goto sim_init_cleanup_on_error;
            }
            else if(sim_state->pc == ULLONG_MAX)
                printf("[WARN] pc == ULLONG_MAX on input line \"%s\":%" PRIu64 " = \"%s\"\n", filename, line_num, line);
        }
        else if(!strncmp(line, "0x", 2))
        {
            char *addr_end;
            uint64_t addr = strtoull(line, &addr_end, 16);
            if(addr == 0 && addr_end == line)
            {
                printf("[SIMULATOR ERROR] invalid address on input line \"%s\":%" PRIu64 " = \"%s\"\n", filename, line_num, line);
                goto sim_init_cleanup_on_error;
            }

            char *len_end;
            uint64_t len = strtoull(addr_end, &len_end, 16);
            if(len == 0 && len_end == addr_end)
            {
                printf("[SIMULATOR ERROR] invalid byte length on input line \"%s\":%" PRIu64 " = \"%s\"\n", filename, line_num, line);
                goto sim_init_cleanup_on_error;
            }
            if(len == 0 || len > 8)
            {
                printf("[SIMULATOR ERROR] invalid line in input file\"%s\":%" PRIu64 " = \"%s\"\n", filename, line_num, line);
                printf("length in bytes must be between 1 and 8\n");
                goto sim_init_cleanup_on_error;
            }

            char *val_end;
            uint64_t val = strtoull(len_end, &val_end, 16);
            if(val == 0 && val_end == len_end)
            {
                printf("[SIMULATOR ERROR] invalid value on input line \"%s\":%" PRIu64 " = \"%s\"\n", filename, line_num, line);
                goto sim_init_cleanup_on_error;
            }

            for(uint64_t i = 0; i < len; i++)
                sim_state->mem_write_byte(addr + i, (val >> (i * 8)) & 0xff);
        }
    }
    fclose(f);

    // set the simulator state to ready
    sim_state->state = STATE_RUNNING;
    return;

sim_init_cleanup_on_error:
    sim_state->state = STATE_ERROR;
    fclose(f);
    sim_mem_fini();
    return;
}

void sim_fini(void)
{
    sim_mem_fini();
}

const char* machine_state_to_string(enum MACHINE_STATE state) {
    switch (state)
    {
        case STATE_HALT:    return "HALT";
        case STATE_RUNNING: return "RUNNING";
        case STATE_ERROR:   return "ERROR";
        default:            return "UNKNOWN";
    }
}

void sim_print_regs(struct simulation_state *sim_state)
{
    const char *reg_names[32][2] = {
        {"x0", "zero"}, {"x1", "ra"},  {"x2", "sp"},   {"x3", "gp"},
        {"x4", "tp"},   {"x5", "t0"},  {"x6", "t1"},   {"x7", "t2"},
        {"x8", "s0"},   {"x9", "s1"},  {"x10", "a0"},  {"x11", "a1"},
        {"x12", "a2"},  {"x13", "a3"}, {"x14", "a4"},  {"x15", "a5"},
        {"x16", "a6"},  {"x17", "a7"}, {"x18", "s2"},  {"x19", "s3"},
        {"x20", "s4"},  {"x21", "s5"}, {"x22", "s6"},  {"x23", "s7"},
        {"x24", "s8"},  {"x25", "s9"}, {"x26", "s10"}, {"x27", "s11"},
        {"x28", "t3"},  {"x29", "t4"}, {"x30", "t5"},  {"x31", "t6"}
    };

    printf("=== Simulation State ===\n");
    for (int i = 0; i < NUM_GPRS; i++)
        printf("%3s (%4s):   0x%016"PRIx64" (%"PRId64")\n", reg_names[i][0], reg_names[i][1], sim_state->gprs[i], sim_state->gprs[i]);
    printf("pc:    0x%016"PRIx64"\n", sim_state->pc);
    printf("ir:    0x%08"PRIx32"\n", sim_state->ir);
    printf("ninst: 0x%016"PRIx64"\n", sim_state->num_instructions_executed);
    printf("state: %s\n", machine_state_to_string(sim_state->state));
    printf("========================\n");
}

void sim_print_mem(uint64_t addr, uint64_t num_bytes, struct simulation_state *sim_state)
{
    const int wrap = 16;
    int start_padding = addr % wrap;
    int total = start_padding + num_bytes;
    int rows = (total + wrap - 1) / wrap;

    uint64_t byte_idx = 0;
    for (int row = 0; row < rows; ++row)
    {
        printf("0x%016lx : ", (addr / wrap + row) * wrap);
        for (int col = 0; col < wrap; ++col)
        {
            int cur_idx = row * wrap + col;
            if(cur_idx < start_padding)
                printf("   ");
            else if(byte_idx < num_bytes)
            {
                printf("%02x ", sim_state->mem_read_byte(addr + byte_idx));
                byte_idx++;
            }
            else
                printf("   ");
        }
        printf("\n");
    }
}

void sim_step(struct simulation_state *sim_state)
{
    printf("Stepping - pc = %016"PRIx64"\n", sim_state->pc);

    struct fetch_outputs f_out;
    struct decode_outputs d_out;
    struct agex_outputs a_out;
    struct mem_outputs m_out;
    struct writeback_outputs w_out;

    /* execute each CPU stage */
    pa5_fetch(sim_state->pc, sim_state->mem_read_byte, &f_out);

    // check for simulation termination
    if(f_out.inst == 0x00100073) // ebreak
    {
        sim_state->state = STATE_HALT;
        return;
    }

    pa5_decode(&f_out, sim_state->gprs, &d_out);
    pa5_agex(&d_out, &a_out);
    pa5_mem(&a_out, sim_state->mem_read_byte, sim_state->mem_write_byte, &m_out, sim_state->use_cache);
    pa5_writeback(&m_out, sim_state->gprs, &w_out);

    /* update the PC with the final output */
    sim_state->pc = w_out.next_pc;

    /* accounting and debugging */
    sim_state->num_instructions_executed++;
}

void print_help(void)
{
    printf("\t\"help\" - prints this help information \n");
    printf("\t\"run\" - execute instructions indefinitely\n");
    printf("\t\"step <optional: N>\" - runs N instructions (N=1 if omitted)\n");
    printf("\t\"regs\" - print all register values\n");
    printf("\t\"mem <addr> <N>\" - print out N bytes of memory at the given address (N=32 if omitted)\n");
    printf("\t\"quit\" or \"exit\" - exits the simulation\n");
}

const char *sim_state_to_str(enum MACHINE_STATE state)
{
    switch(state)
    {
        case STATE_ERROR:   return "ERR";
        case STATE_HALT:    return "HLT";
        case STATE_RUNNING: return "RUN";
        default:            return "UNK";
    }
}

int run_simulator(const char *filename, bool use_cache) {
    // disable buffering on stdout
    setbuf(stdout, NULL);

    /* parse the input file and initialize simulator state for step-through */
    struct simulation_state sim_state;
    sim_init(filename, use_cache, &sim_state);
    if(sim_state.state == STATE_ERROR) return EXIT_FAILURE;

    /* launch an interactive shell */
    assert(sim_state.state == STATE_RUNNING);
    printf("[INFO] test file \"%s\" successfully loaded into simulated memory\n", filename);
    printf("[INFO] starting interactive shell. type \"help\" for help; ctrl+c to exit\n");

    char buffer[SIM_BUF_LEN];
    bool is_sim_active = true;
    while(is_sim_active)
    {
        fprintf(stdout, "sim [state = %s]> ", sim_state_to_str(sim_state.state));
        fflush(stdout);
        fgets(buffer, sizeof(buffer), stdin);

        char *p = memchr(buffer, '\n', SIM_BUF_LEN);
        if(p == NULL)
        {
            printf("\ncommand too long! keep it within %d bytes or increase the line buffer size in " __FILE__ "\n", SIM_BUF_LEN);
            continue;
        }

        // parse the command
        //     sim> help
        //     sim> run
        //     sim> next - runs until control reaches PC + 4
        //     sim> step - runs one instruction
        //     sim> regs - dump registers
        //     sim> mem <addr> <count> - print bytes in memory
        if(!strncmp(buffer, "run", 3))
        {
            while(sim_state.state == STATE_RUNNING)
                sim_step(&sim_state);
        }
        else if(!strncmp(buffer, "step", 4))
        {
            uint64_t N;
            int n_matches = sscanf(buffer, "step %" SCNx64, &N);
            if(n_matches != 1)
                N = 1;

            while(sim_state.state == STATE_RUNNING && N > 0)
            {
                sim_step(&sim_state);
                N--;
            }
        }
        else if(!strncmp(buffer, "regs", 4))
            sim_print_regs(&sim_state);
        else if(!strncmp(buffer, "mem", 3))
        {
            uint64_t addr;
            uint64_t N;
            int n_matches = sscanf(buffer, "mem %" SCNx64 " %" SCNx64, &addr, &N);
            if(n_matches != 2)
                print_help();
            else
                sim_print_mem(addr, N, &sim_state);
        }
        else if(!strncmp(buffer, "quit", 4) || !strncmp(buffer, "exit", 4))
            is_sim_active = false;
        else
        {
            if(strncmp(buffer, "help", 4))
                printf("\nerror: unrecognized command: \"%s\". Available commands:\n", buffer);
            print_help();
        }

        switch(sim_state.state)
        {
            case STATE_ERROR:   printf("warn: simulator is in an error state (cannot step; inspect state or exit only)\n"); break;
            case STATE_HALT:    printf("warn: simulator is in a halted state (cannot step; inspect state or exit only)\n"); break;
            case STATE_RUNNING: break;
            default:            printf("warn: simulator is in an unknown state (cannot step; inspect state or exit only)\n"); break;
        }
    }

    printf("Simulation terminated with final state:\n");
    sim_print_regs(&sim_state);

    /* cleanup and return */
    sim_fini();

    return EXIT_SUCCESS;
}

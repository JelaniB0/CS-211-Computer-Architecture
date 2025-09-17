SOURCE       := main.c pa2.c
SRC_DIR      := src
BUILD_DIR    := build
EXECUTABLE   := pa2

CFLAGS_ARCH  := -march=rv64i -mabi=lp64 -misa-spec=2.2
CFLAGS_DEBUG := -O0 -g
CFLAGS       := $(CFLAGS_ARCH) $(CFLAGS_DEBUG) -Isrc -std=c11

LP64_PREFIX  := /common/users/shared/cs211_s25_5678/toolchain_glibc2
LP64_GCC     := $(LP64_PREFIX)/bin/riscv64-unknown-linux-gnu-gcc
PA2_SO       := libpa2.so

GRUN_PATH    := /common/system/riscv64i
PREFIX       := /common/system/riscvi/bin/riscv64-unknown-elf-

.PHONY: default clean grade test run debug
.SUFFIXES: # no default implicit rules bogging us down

default: | $(BUILD_DIR) $(EXECUTABLE)

debug: $(EXECUTABLE)
	$(GRUN_PATH)/grun $(EXECUTABLE)
	$(info Great! Now, open another terminal and connect to this exact same ilab machine ($(shell uname -n)). Navigate to the PA2 repo, and issue the command "/common/system/riscv64i/gdb pa2". Use Ctrl+C to exit back to the shell.)

clean:
	rm -f $(EXECUTABLE)
	rm -rf $(BUILD_DIR)
	rm -f $(PA2_SO)

grade:
	$(LP64_GCC) $(CFLAGS_ARCH) src/pa2.c -fPIE -shared -o $(PA2_SO)

test:
	qemu-riscv64 -L $(LP64_PREFIX)/sysroot/ ./cs211_autograder -s

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(SOURCE:%.c=$(BUILD_DIR)/%.d)

# C compilation (C source -> binary object file)
$(SOURCE:%.c=$(BUILD_DIR)/%.o): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(PREFIX)gcc -c $(CFLAGS) -MMD -MF $(@:%.o=%.d) $< -o $@

# Linking the object files ({all objects} -> executable binary)
$(EXECUTABLE): $(SOURCE:%.c=$(BUILD_DIR)/%.o)
	$(PREFIX)gcc $(CFLAGS) $^ -o $@

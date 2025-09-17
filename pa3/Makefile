SOURCE             := main.c pa3.c pa3_test.c
SRC_DIR            := src
BUILD_DIR          := build
EXE_CROSS          := pa3
EXE_ASAN           := pa3.asan
EXE_VALGRIND       := pa3.valgrind
EXE_NATIVE         := pa3.native

CS211_DIR          := /common/users/shared/cs211_s25_5678
TOOLCHAN_DIR       := $(CS211_DIR)/toolchain_glibc3
VALGRIND_DIR       := $(CS211_DIR)/valgrind_glibc2

CFLAGS_ARCH_RV64I  := -march=rv64i -mabi=lp64 -misa-spec=2.2
CFLAGS_DEBUG       := -O0 -g -Wall -Wextra -Werror -Wshadow -pedantic -fno-omit-frame-pointer
CFLAGS_COMMON      := $(CFLAGS_DEBUG) -Isrc -std=c11

CFLAGS_CROSS       := $(CFLAGS_COMMON) $(CFLAGS_ARCH_RV64I)
CFLAGS_ASAN        := $(CFLAGS_COMMON) $(CFLAGS_ARCH_RV64I) -fsanitize=address
CFLAGS_VALGRIND    := $(CFLAGS_COMMON) $(CFLAGS_ARCH_RV64I)
CFLAGS_NATIVE      := $(CFLAGS_COMMON)

LDFLAGS_CROSS      := -Wl,-rpath=$(TOOLCHAN_DIR)/sysroot/lib
LDFLAGS_ASAN       := -Wl,-rpath=$(TOOLCHAN_DIR)/sysroot/lib -fsanitize=address
LDFLAGS_VALGRIND   := -Wl,-rpath=$(TOOLCHAN_DIR)/sysroot/lib
LDFLAGS_NATIVE     :=

PATCHELF           := patchelf
INTERPRETER_CROSS  := $(TOOLCHAN_DIR)/sysroot/lib/ld-linux-riscv64-lp64.so.1

GRUN_PATH          := /common/system/riscv64i
PREFIX_CROSS       := $(TOOLCHAN_DIR)/bin/riscv64-unknown-linux-gnu-
PREFIX_NATIVE      :=

.PHONY: default clean native debug sanitize valgrind
.SUFFIXES: # no default implicit rules bogging us down

default: | $(BUILD_DIR) $(EXE_CROSS)

native: | $(BUILD_DIR) $(EXE_NATIVE)

debug: $(EXE_CROSS)
	$(GRUN_PATH)/grun $(EXE_CROSS)
	$(info Great! Now, open another terminal and connect to this exact same ilab machine ($(shell uname -n)). Navigate to the PA3 repo, and issue the command "/common/system/riscv64i/gdb pa3". Use Ctrl+C to exit back to the shell.)

sanitize: $(BUILD_DIR) $(EXE_ASAN)
	ASAN_OPTIONS=detect_leaks=0 QEMU_RESERVED_VA=256G QEMU_GUEST_BASE=0x14000 qemu-riscv64 -L $(TOOLCHAN_DIR)/sysroot $(EXE_ASAN)

valgrind: $(BUILD_DIR) $(EXE_VALGRIND)
	QEMU_RESERVED_VA=256G QEMU_GUEST_BASE=0x14000 qemu-riscv64 -E LD_LIBRARY_PATH=$(TOOLCHAN_DIR)/sysroot/lib/,VALGRIND_LIB=$(VALGRIND_DIR)/libexec/valgrind/ -L $(TOOLCHAN_DIR)/sysroot/ $(VALGRIND_DIR)/bin/valgrind --track-origins=yes -s $(TOOLCHAN_DIR)/sysroot/lib/ld-linux-riscv64-lp64.so.1 ./$(EXE_VALGRIND)

clean:
	rm -f $(EXE_CROSS) $(EXE_NATIVE) $(EXE_ASAN) $(EXE_VALGRIND)
	rm -rf $(BUILD_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

#
# Build a cross-compiled version of the binary
#
-include $(SOURCE:%.c=$(BUILD_DIR)/%.d)

# C compilation (C source -> binary object file)
$(SOURCE:%.c=$(BUILD_DIR)/%.o): $(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(PREFIX_CROSS)gcc $(CFLAGS_CROSS) -c -MMD -MF $(@:%.o=%.d) $< -o $@

# Linking the object files ({all objects} -> executable binary)
$(EXE_CROSS): $(SOURCE:%.c=$(BUILD_DIR)/%.o)
	$(PREFIX_CROSS)gcc $(LDFLAGS_CROSS) $^ -o $@
	$(PATCHELF) --set-interpreter $(INTERPRETER_CROSS) $@

#
# Build a cross-compiled version of the binary without address sanitizer
#
-include $(SOURCE:%.c=$(BUILD_DIR)/%.d.asan)

# C compilation (C source -> binary object file)
$(SOURCE:%.c=$(BUILD_DIR)/%.o.asan): $(BUILD_DIR)/%.o.asan: $(SRC_DIR)/%.c
	$(PREFIX_CROSS)gcc $(CFLAGS_ASAN) -c -MMD -MF $(@:%.o.asan=%.d.asan) $< -o $@

# Linking the object files ({all objects} -> executable binary)
$(EXE_ASAN): $(SOURCE:%.c=$(BUILD_DIR)/%.o.asan)
	$(PREFIX_CROSS)gcc $(LDFLAGS_ASAN) $^ -o $@

#
# Build a cross-compiled version of the unpatched binary without address sanitizer
#
-include $(SOURCE:%.c=$(BUILD_DIR)/%.d.valgrind)

# C compilation (C source -> binary object file)
$(SOURCE:%.c=$(BUILD_DIR)/%.o.valgrind): $(BUILD_DIR)/%.o.valgrind: $(SRC_DIR)/%.c
	$(PREFIX_CROSS)gcc $(CFLAGS_VALGRIND) -c -MMD -MF $(@:%.o.valgrind=%.d.valgrind) $< -o $@

# Linking the object files ({all objects} -> executable binary)
$(EXE_VALGRIND): $(SOURCE:%.c=$(BUILD_DIR)/%.o.valgrind)
	$(PREFIX_CROSS)gcc $(LDFLAGS_VALGRIND) $^ -o $@

#
# Build a native version of the binary
#
-include $(SOURCE:%.c=$(BUILD_DIR)/%.d.native)

# C compilation (C source -> binary object file)
$(SOURCE:%.c=$(BUILD_DIR)/%.o.native): $(BUILD_DIR)/%.o.native: $(SRC_DIR)/%.c
	$(PREFIX_NATIVE)gcc $(CFLAGS_NATIVE) -c -MMD -MF $(@:%.o.native=%.d.native) $< -o $@

# Linking the object files ({all objects} -> executable binary)
$(EXE_NATIVE): $(SOURCE:%.c=$(BUILD_DIR)/%.o.native)
	$(PREFIX_NATIVE)gcc $(LDFLAGS_NATIVE) $^ -o $@

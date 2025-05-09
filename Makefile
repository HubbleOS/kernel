DIR_X86 = arch/x86
DIR_ARM64 = arch/arm64

.PHONY: all arch_x86 arch_arm64 

arch_x86:
	make -C ${DIR_X86}

arch_arm64:
	make -C ${DIR_ARM64}

# # Compiler for .c files
# CC := gcc
# CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -Werror -Wshadow -Wunused-variable -Wuninitialized -Wconversion -Wdeprecated-declarations -Wformat -Wswitch -Wvla -Wunreachable-code
# INCLUDES := -Iinclude -Isrc

# # Compiler for .s files
# AS := as
# ASFLAGS := -arch arm64

# BUILD_DIR := build

# SRC_DIR := src
# TARGET := $(BUILD_DIR)/main

# SRCS := $(shell find $(SRC_DIR) -name '*.c')
# ASM_SRCS := $(shell find $(SRC_DIR) -name '*.s')

# OBJS := $(SRCS:%.c=$(BUILD_DIR)/%.o)
# ASM_OBJS := $(ASM_SRCS:%.s=$(BUILD_DIR)/%.o)

# ALL_OBJS := $(OBJS) $(ASM_OBJS)

# TEST_DIR := test
# TEST_TARGET := $(BUILD_DIR)/testProgram

# TEST_SRCS := $(shell find $(TEST_DIR) -name '*.c')
# TEST_OBJS := $(filter-out $(BUILD_DIR)/src/kernel/main.o, $(OBJS) $(TEST_SRCS:%.c=$(BUILD_DIR)/%.o))

# .PHONY: all build run clean test

# all: build run

# build: $(TARGET)

# $(TARGET): $(ALL_OBJS)
# 	@mkdir -p $(BUILD_DIR)
# 	$(CC) $(CFLAGS) $(INCLUDES) $(ALL_OBJS) $(LDFLAGS) -o $(TARGET)

# # Компиляция .c файлов
# $(BUILD_DIR)/%.o: %.c
# 	@mkdir -p $(dir $@)
# 	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

# # Компиляция .s файлов
# $(BUILD_DIR)/%.o: %.s
# 	@mkdir -p $(dir $@)
# 	$(AS) $(ASFLAGS) -o $@ $<

# test: $(TEST_TARGET)
# 	@$(TEST_TARGET)

# $(TEST_TARGET): $(TEST_OBJS)
# 	@mkdir -p $(BUILD_DIR)
# 	$(CC) $(CFLAGS) $(INCLUDES) $(TEST_OBJS) $(LDFLAGS) -o $(TEST_TARGET)

# run: build
# 	@$(TARGET)

# clean:
# 	@rm -rf $(BUILD_DIR)

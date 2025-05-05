# Cross-compiler prefix
CROSS = i686-elf-

# Tools
AS = $(CROSS)as
CC = $(CROSS)gcc
LD = $(CROSS)gcc
QEMU = qemu-system-x86_64

# Flags
CFLAGS = -ffreestanding -c -lgcc
LDFLAGS = -nostdlib -T linker.ld -ffreestanding -O2 -lgcc

INCLUDES := -Iinclude

# Directories
SRC_DIRS = boot kernel
BUILD_DIR = build
BIN_DIR = bin

# Source files
SRC_FILES = $(foreach dir,$(SRC_DIRS),$(wildcard $(dir)/*.[csS]))
OBJ_FILES = $(patsubst %.c,$(BUILD_DIR)/%.o,$(patsubst %.s,$(BUILD_DIR)/%.o,$(patsubst %.S,$(BUILD_DIR)/%.o,$(SRC_FILES))))

# Output ELF
OUTPUT = $(BIN_DIR)/kernel.bin

# Default target
all: run

# Build only
build: $(OUTPUT)

# Link
$(OUTPUT): $(OBJ_FILES) linker.ld | $(BIN_DIR)
	@echo "Linking: $@"
	$(LD) $(LDFLAGS) $(OBJ_FILES) -o $@

# Compile/Assemble rules
$(BUILD_DIR)/%.o: %.c | create-dir
	@echo "Compiling C: $< -> $@"
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

$(BUILD_DIR)/%.o: %.s | create-dir
	@echo "Assembling ASM: $< -> $@"
	$(AS) $< -o $@

$(BUILD_DIR)/%.o: %.S | create-dir
	@echo "Assembling preprocessed ASM: $< -> $@"
	$(CC) $(CFLAGS) $(INCLUDES) -x assembler-with-cpp $< -o $@

# Create directory structure in build/ and bin/
create-dir:
	@echo "Creating build and bin directories..."
	@mkdir -p $(addprefix $(BUILD_DIR)/,$(SRC_DIRS))
	@mkdir -p $(BIN_DIR)

ISO_DIR = isodir
ISO_BOOT = $(ISO_DIR)/boot
ISO_GRUB = $(ISO_BOOT)/grub
ISO_KERNEL = $(ISO_BOOT)/kernel.bin
ISO_IMAGE = $(BIN_DIR)/myos.iso
GRUB_CFG = grub/grub.cfg

iso: $(ISO_IMAGE)

$(ISO_IMAGE): $(OUTPUT) $(GRUB_CFG)
	@echo "Creating ISO directory structure..."
	mkdir -p $(ISO_GRUB)
	cp $(OUTPUT) $(ISO_KERNEL)
	cp $(GRUB_CFG) $(ISO_GRUB)/grub.cfg
	grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR)


# Run in QEMU
run: $(ISO_IMAGE)
	@echo "Running ISO in QEMU..."
	$(QEMU) -cdrom $(ISO_IMAGE)


# Clean build
clean:
	@echo "Cleaning build and bin directories..."
	rm -rf $(BUILD_DIR) $(BIN_DIR)

.PHONY: all run clean build create-dir

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

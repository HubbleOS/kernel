# Cross compiler
CROSS = x86_64-elf-

# Tools
AS = $(CROSS)as
CC = $(CROSS)gcc
LD = $(CROSS)ld
OBJCOPY = $(CROSS)objcopy

# Flags
CFLAGS = -ffreestanding -m64 -O2 -Wall -Wextra -c
LDFLAGS = -nostdlib -T kernel/linker.ld # !!!!!!!!!!!!!!

INCLUDES := -Iinclude

# Directories
SRC_DIRS = kernel
BUILD_DIR = build

# Source files
SRC_FILES = $(shell find $(SRC_DIRS) -type f -name "*.c")
OBJ_FILES = $(patsubst %.c,$(BUILD_DIR)/%.o,$(SRC_FILES))

# Output ELF
OUTPUT = $(BUILD_DIR)/kernel.elf

# Default target
all: run

run: $(OUTPUT)
	@echo "Build successful."

# Build object files
build: $(OBJ_FILES)

# Compile rule
$(BUILD_DIR)/%.o: %.c
	@echo "Compiling C: $< -> $@"
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

# Linking
$(OUTPUT): $(OBJ_FILES)
	@echo "Linking ELF: $@"
	$(LD) $(LDFLAGS) $^ -o $@
	$(LD) $(LDFLAGS) $(BUILD_DIR)/kernel.elf

# Clean
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD_DIR)

.PHONY: all run build clean

# Bootloader


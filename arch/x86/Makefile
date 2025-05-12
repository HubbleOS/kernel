# Cross-compiler prefix
CROSS = x86_64-elf-

# Tools
AS = $(CROSS)as
CC = $(CROSS)gcc
LD = $(CROSS)gcc
QEMU = qemu-system-x86_64

# Flags
CFLAGS = -ffreestanding -c -lgcc
# -LDFLAGS = -nostdlib -T linker.ld -ffreestanding -O2 -lgcc
LDFLAGS = -nostdlib -T linker.ld -ffreestanding -O2 

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
	# i686-elf-grub-mkrescue -o $(ISO_IMAGE) $(ISO_DIR)
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

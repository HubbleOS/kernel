# Root Makefile

# Allow override from command line
ARCH ?= x86
BUILD_DIR := build
ARCH_DIR := arch/$(ARCH)

.PHONY: all clean run build

all: build

build:
	$(MAKE) -C $(ARCH_DIR) BUILD_DIR=$(abspath $(BUILD_DIR))

run:
	$(MAKE) -C $(ARCH_DIR) run BUILD_DIR=$(abspath $(BUILD_DIR))

clean:
	$(MAKE) -C $(ARCH_DIR) clean BUILD_DIR=$(abspath $(BUILD_DIR))

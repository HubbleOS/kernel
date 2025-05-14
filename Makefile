.PHONY: all build-x86

BUILD_DIR := build

all: build-x86

build-x86:
	make -C arch/x86 BUILD_DIR=$(BUILD_DIR)

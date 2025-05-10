#!/bin/bash

make

# Збірка ядра

x86_64-elf-gcc -Iinclude -ffreestanding -m64 -O2 -Wall -c kernel/font.c -o build/font.o
x86_64-elf-gcc -Iinclude -ffreestanding -m64 -O2 -Wall -c kernel/kernel.c -o build/kernel.o
x86_64-elf-gcc -Iinclude -ffreestanding -m64 -O2 -Wall -c kernel/textout.c -o build/textout.o


x86_64-elf-ld -nostdlib -T kernel/linker.ld -o build/kernel.elf build/kernel.o build/font.o build/textout.o


# Витяг у flat binary
x86_64-elf-objcopy -O binary build/kernel.elf iso/kernel.bin

qemu-system-x86_64 \
              -drive if=pflash,format=raw,readonly=on,file=ovmf/OVMF_CODE.fd \
              -drive if=pflash,format=raw,file=ovmf/OVMF_VARS.fd \
              -hda fat:rw:iso \
              -m 512 \
              -vga std \
              -serial stdio
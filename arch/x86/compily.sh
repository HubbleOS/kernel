#!/bin/bash

make

# Збірка ядра
nasm -f elf64 ./kernel/stack.s -o ./build/entry.o
x86_64-elf-gcc -Iinclude -mno-red-zone -ffreestanding -m64 -O2 -Wall -c kernel/font.c -o build/font.o
x86_64-elf-gcc -Iinclude -mno-red-zone -ffreestanding -m64 -O2 -Wall -c kernel/kernel.c -o build/kernel.o
x86_64-elf-gcc -Iinclude -mno-red-zone -ffreestanding -m64 -O2 -Wall -c kernel/textout.c -o build/textout.o
x86_64-elf-gcc -Iinclude -mno-red-zone -ffreestanding -m64 -O2 -Wall -c kernel/cli.c -o build/cli.o
x86_64-elf-gcc -Iinclude -mno-red-zone -ffreestanding -m64 -O2 -Wall -c kernel/heap.c -o build/heap.o

x86_64-elf-ld -nostdlib -T kernel/linker.ld -o  build/kernel.elf build/entry.o build/kernel.o build/font.o build/cli.o build/textout.o build/heap.o


# Витяг у flat binary
x86_64-elf-objcopy -O binary build/kernel.elf iso/kernel.bin

# qemu-system-x86_64 \
#               -M pc \
#               -drive if=pflash,format=raw,readonly=on,file=ovmf/OVMF_CODE.fd \
#               -drive if=pflash,format=raw,file=ovmf/OVMF_VARS.fd \
#               -hda fat:rw:iso \
#               -m 512 \
#               -serial stdio \
qemu-system-x86_64 \
  -cpu max -m 1024M -machine accel=tcg \
  -no-reboot -d int,guest_errors \
  -drive if=pflash,format=raw,readonly=on,file=ovmf/OVMF_CODE.fd \
  -drive if=pflash,format=raw,file=ovmf/OVMF_VARS.fd \
  -hda fat:rw:iso \
  -serial stdio \
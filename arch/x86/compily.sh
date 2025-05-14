#!/bin/bash

gcc -Iinclude -Ignu-efi/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c boot.c -o boot.o
ld -shared -Bsymbolic -L gnu-efi/x86_64/lib -L gnu-efi/x86_64/gnuefi -T gnu-efi/gnuefi/elf_x86_64_efi.lds gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o boot.o -o boot.so -lgnuefi -lefi
objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 boot.so iso/EFI/BOOT/BOOTx64.EFI

rm *.o *.so

x86_64-elf-gcc -Iinclude -ffreestanding -m64 -O2 -Wall -c kernel/utils/font.c -o build/utils/font.o
x86_64-elf-gcc -Iinclude -ffreestanding -m64 -O2 -Wall -c kernel/kernel.c -o build/kernel.o
x86_64-elf-gcc -Iinclude -ffreestanding -m64 -O2 -Wall -c kernel/textout.c -o build/textout.o
x86_64-elf-gcc -Iinclude -ffreestanding -m64 -O2 -Wall -c kernel/cli.c -o build/cli.o

x86_64-elf-ld -nostdlib -T kernel/linker.ld -o build/kernel.elf build/kernel.o build/utils/font.o build/cli.o build/textout.o

# Витяг у flat binary
x86_64-elf-objcopy -O binary build/kernel.elf iso/kernel.bin

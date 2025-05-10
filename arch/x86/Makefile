run: boot.o boot.so boot.efi
	make clean

boot.o:
	gcc -Iinclude -I gnu-efi/inc -fpic -ffreestanding -fno-stack-protector -fno-stack-check -fshort-wchar -mno-red-zone -maccumulate-outgoing-args -c boot.c -o boot.o

boot.so:
	ld -shared -Bsymbolic -L gnu-efi/x86_64/lib -L gnu-efi/x86_64/gnuefi -T gnu-efi/gnuefi/elf_x86_64_efi.lds gnu-efi/x86_64/gnuefi/crt0-efi-x86_64.o boot.o -o boot.so -lgnuefi -lefi

boot.efi:
	objcopy -j .text -j .sdata -j .data -j .rodata -j .dynamic -j .dynsym  -j .rel -j .rela -j .rel.* -j .rela.* -j .reloc --target efi-app-x86_64 --subsystem=10 boot.so iso/EFI/BOOT/BOOTx64.EFI

clean:
	rm *.o *.so
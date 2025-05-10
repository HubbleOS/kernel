FROM ubuntu:22.04

RUN apt update && apt install -y \
	build-essential bison flex libgmp3-dev libmpc-dev libmpfr-dev texinfo \
	curl git xorriso mtools qemu-system-aarch64

# binutils
RUN curl -LO https://ftp.gnu.org/gnu/binutils/binutils-2.42.tar.gz && \
	tar -xf binutils-2.42.tar.gz && \
	mkdir binutils-build && cd binutils-build && \
	../binutils-2.42/configure --target=aarch64-elf --prefix=/usr/local/cross \
	--with-sysroot --disable-nls --disable-werror && \
	make -j$(nproc) && make install

# gcc
RUN curl -LO https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.gz && \
	tar -xf gcc-13.2.0.tar.gz && \
	mkdir gcc-build && cd gcc-build && \
	../gcc-13.2.0/configure --target=aarch64-elf --prefix=/usr/local/cross \
	--disable-nls --enable-languages=c --without-headers && \
	make all-gcc -j2 && make all-target-libgcc -j2 && \
	make install-gcc && make install-target-libgcc

ENV PATH="/usr/local/cross/bin:$PATH"
WORKDIR /src
CMD ["/bin/bash"]

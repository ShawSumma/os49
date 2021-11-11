
CLANG = clang

all: tmp/os49.iso

run: tmp/os49.iso
	qemu-system-x86_64 -hda tmp/os49.iso

BINTHING ?= limine-eltorito-efi.bin

tmp/os49.iso: tmp/os49.bin
	mkdir -p tmp/iso
	mkdir -p tmp/iso/boot
	cp limine.cfg tmp/iso/limine.cfg
	cp tmp/os49.bin tmp/iso/os49.bin
	cp limine/limine.sys tmp/iso/boot/limine.sys
	cp limine/limine-eltorito-efi.bin tmp/iso/limine-eltorito-efi.bin
	cp limine/limine.sys tmp/iso/boot/limine.sys
	cp limine/limine-cd.bin tmp/iso/limine-cd.bin
	xorriso -as mkisofs -b limine-cd.bin \
			-no-emul-boot -boot-load-size 4 -boot-info-table \
			--efi-boot limine-eltorito-efi.bin \
			-efi-boot-part --efi-boot-image --protective-msdos-label \
			./isoroot -o tmp/os49.iso
	./limine/limine-install-linux-x86_64 tmp/os49.iso

tmp/os49.bin: kernel.c
	mkdir -p tmp
	$(CLANG) -g3 -c kernel.c -o tmp/kernel.o -std=gnu11 -ffreestanding -O2 -fPIC 
	$(CLANG) -g3 -T linker.ld -o tmp/os49.bin -ffreestanding -O2 -nostdlib tmp/kernel.o -lgcc

.dummy:

clean:
	rm -fr tmp
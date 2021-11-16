
default: all

CLANG ?= clang
GIT ?= git
OPT ?= 0

VM_CFILES = paka/minivm/vm/vm.c paka/minivm/vm/state.c paka/minivm/vm/gc.c
VM_OBJS = $(VM_CFILES:%.c=%.o)

OS_CFILES = src/kernel.c src/io.c src/mem.c src/math.c
OS_OBJS = $(OS_CFILES:%.c=%.o)

OBJS = $(OS_OBJS) $(VM_OBJS)
INCLUDE = -Istivale -Ipaka/minivm

LANG = PAKA

OS_CFLAGS = -DOS_LANG_$(LANG)
VM_CFLAGS = -DVM_OS

QFLAGS = -nographic -serial mon:stdio

ifeq ($(LANG),paka)
GIT_REPO ?= https://github.com/shawsumma/paka.git
tmp/lang.bc: tmp/lang
	$(MAKE) -C tmp/lang 
	cp tmp/lang/bin/stage3 $@ 
else
GIT_REPO ?= https://github.com/shawsumma/litter.git
tmp/lang.bc: tmp/lang
	$(MAKE) -C tmp/lang/paka
	./tmp/lang/paka/bin/minivm ./tmp/lang/paka/bin/stage3  ./tmp/lang/src/mlatu.paka -o $@
endif

tmp/lang: 
	mkdir -p tmp
	rm -rf tmp/lang
	$(GIT) clone $(GIT_REPO) --recursive --depth 1 "tmp/lang" || true

all: tmp/os49.iso

run: tmp/os49.iso
	qemu-system-x86_64 $(QFLAGS) -cdrom tmp/os49.iso

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
			tmp/iso -o tmp/os49.iso
	./limine/limine-install-linux-x86_64 tmp/os49.iso

tmp/os49.bin: $(OBJS)
	mkdir -p tmp
	$(CLANG) -g3 -T linker.ld -o tmp/os49.bin -ffreestanding -O$(OPT) -nostdlib $(OBJS) -lgcc

$(VM_OBJS): $(@:%.o=%.c)
	$(CLANG) -g3 -c $(@:%.o=%.c) -o $@ -std=gnu11 -ffreestanding -O$(OPT) -fPIC -fno-builtin $(INCLUDE) $(CFLAGS) $(VM_CFLAGS)

$(OS_OBJS): $(@:%.o=%.c) tmp/lang.bc
	$(CLANG) -g3 -c $(@:%.o=%.c) -o $@ -std=gnu11 -ffreestanding -O$(OPT) -fPIC -fno-builtin $(INCLUDE) $(CFLAGS) $(OS_CFLAGS)

.dummy:

clean:
	rm -fr tmp $(OBJS)
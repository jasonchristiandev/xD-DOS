DEBUG = 0

CC = gcc
LD = ld
OBJCOPY = objcopy
MAKE = make
QEMU = qemu-system-x86_64

SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
ISO_DIR = iso
LIMINE_DIR = limine_bin
FONTS_DIR = fonts

FONT = font.psf
LINKER_SCRIPT = linker.lds
LIMINE_CONF = limine.conf
KERNEL = kernel.elf
ISO_IMAGE = xD-DOS.iso

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS)) $(BUILD_DIR)/font_data.o

CFLAGS = -fno-stack-protector -ffreestanding -mno-red-zone \
		 -fms-extensions -nostdlib -Wall -Wextra -Wno-variadic-macros \
		 -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast \
		 -Wno-compare-distinct-pointer-types -std=c11 -O2 \
		 -mno-mmx -mno-sse -mno-sse2 -I$(INCLUDE_DIR)

FONT_NAME = cp850-8x16.psf
FONT_URL = https://raw.githubusercontent.com/ercanersoy/PSF-Fonts/master/$(FONT_NAME)
LIMINE_VERSION = v11.x-binary
LIMINE_URL = https://github.com/limine-bootloader/limine.git

.PHONY: all run clean

all: $(LIMINE_DIR)/limine $(ISO_IMAGE)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(INCLUDE_DIR)/limine.h: limine-protocol/include/limine.h
	@cp $< $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(BUILD_DIR) $(INCLUDE_DIR)/limine.h
	@echo " [CC] $<"
ifeq ($(DEBUG),1)
	@$(CC) $(CFLAGS) -c $< -o $@ -DDEBUG
else
	@$(CC) $(CFLAGS) -c $< -o $@
endif

$(BUILD_DIR)/font_data.o: $(FONTS_DIR)/$(FONT) $(BUILD_DIR)
	@echo " [OBJCOPY] $<"
	objcopy -I binary -O elf64-x86-64 -B i386 \
		--rename-section .data=.data,alloc,load,readonly,data,contents \
		$< $@
	@objcopy $@ $@ \
		--redefine-sym _binary_fonts_font_psf_start=_binary_font_psf_start \
		--redefine-sym _binary_fonts_font_psf_end=_binary_font_psf_end \
		--redefine-sym _binary_fonts_font_psf_size=_binary_font_psf_size

$(BUILD_DIR)/$(KERNEL): $(OBJS) $(LINKER_SCRIPT) $(BUILD_DIR)
	@echo " [LD] Linking..."
	@echo "     $(OBJS)"
	@$(LD) -T $(LINKER_SCRIPT) $(OBJS) -o $(BUILD_DIR)/$(KERNEL)

$(FONTS_DIR):
	@mkdir -p $(FONTS_DIR)

$(FONTS_DIR)/$(FONT): $(FONTS_DIR)
	@echo " [FETCH] Fetching $(FONT)..."
	curl -sL $(FONT_URL) -o $@ || wget -qO $@ $(FONT_URL)

$(LIMINE_DIR)/limine:
	@if [ ! -f "$(LIMINE_DIR)/limine" ]; then \
		echo " [FETCH] Fetching Limine binaries..."; \
		git clone --depth 1 -b $(LIMINE_VERSION) $(LIMINE_URL) $(LIMINE_DIR); \
		$(MAKE) -C $(LIMINE_DIR); \
	fi

$(ISO_IMAGE): $(BUILD_DIR)/$(KERNEL) $(LIMINE_CONF) $(LIMINE_DIR)/limine
	@echo " [ISO] Creating $(ISO_IMAGE)..."
	mkdir -p $(ISO_DIR)/boot
	mkdir -p $(ISO_DIR)/EFI/BOOT
	
	cp $(BUILD_DIR)/$(KERNEL) $(ISO_DIR)/boot/
	cp $(LIMINE_CONF) $(ISO_DIR)/boot/
	
	cp $(LIMINE_DIR)/limine-bios.sys $(ISO_DIR)/boot/
	cp $(LIMINE_DIR)/limine-bios-cd.bin $(ISO_DIR)/boot/
	cp $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_DIR)/boot/
	
	xorriso -as mkisofs -b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$(ISO_DIR) -o $(ISO_IMAGE)

	$(LIMINE_DIR)/limine bios-install $(ISO_IMAGE)
	@echo " [ISO] $(ISO_IMAGE) success."

run: $(ISO_IMAGE)
	$(QEMU) -cdrom $(ISO_IMAGE) -m 256M -M q35 -serial mon:stdio | tee qemu.log

distclean:
	rm -rf $(BUILD_DIR) $(ISO_IMAGE) $(ISO_DIR) $(LIMINE_DIR) $(FONTS_DIR)

clean:
	rm -rf $(BUILD_DIR) $(ISO_IMAGE) $(ISO_DIR)
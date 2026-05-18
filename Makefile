CC = gcc
LD = ld
MAKE = make

SRC = src
INCLUDE = include
BUILD = build
ISO_DIR = iso
LIMINE_DIR = limine_bin

LINKER_SCRIPT = linker.lds
LIMINE_CONF = limine.conf
KERNEL = kernel.elf
ISO_IMAGE = xD-DOS.iso

SRCS = $(wildcard $(SRC)/*.c)
OBJS = $(patsubst $(SRC)/%.c, $(BUILD)/%.o, $(SRCS))

CFLAGS = -ffreestanding -fno-exceptions -mno-red-zone \
		 -Wall -Wextra -Wpedantic -std=c11 -O2 -I$(INCLUDE)

LIMINE_VERSION = v11.x-binary
LIMINE_URL = https://github.com/limine-bootloader/limine.git

.PHONY: all run clean

all: $(LIMINE_DIR)/limine $(ISO_IMAGE)

$(BUILD):
	@mkdir -p $(BUILD)

$(BUILD)/%.o: $(SRC)/%.c $(BUILD)
	@echo " [CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/$(KERNEL): $(OBJS) $(LINKER_SCRIPT) $(BUILD)
	@echo " [LD] Linking..."
	echo "$(OBJS)"
	@$(LD) -T $(LINKER_SCRIPT) $(OBJS) -o $(BUILD)/$(KERNEL)

$(LIMINE_DIR)/limine:
	@if [ ! -f "$(LIMINE_DIR)/limine" ]; then \
		echo " [SETUP_LIMINE] Fetching Limine binaries..."; \
		git clone --depth 1 -b $(LIMINE_VERSION) $(LIMINE_URL) $(LIMINE_DIR); \
		$(MAKE) -C $(LIMINE_DIR); \
	fi

$(ISO_IMAGE): $(BUILD)/$(KERNEL) $(LIMINE_CONF) $(LIMINE_DIR)/limine
	@echo " [ISO] Creating $(ISO_IMAGE)..."
	mkdir -p $(ISO_DIR)/boot
	mkdir -p $(ISO_DIR)/EFI/BOOT
	
	cp $(BUILD)/$(KERNEL) $(ISO_DIR)/boot/
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
	qemu-system-x86_64 -cdrom $(ISO_IMAGE) -m 256M -M q35

distclean:
	rm -rf $(BUILD) $(ISO_IMAGE) $(ISO_DIR) $(LIMINE_DIR)

clean:
	rm -rf $(BUILD) $(ISO_IMAGE) $(ISO_DIR)
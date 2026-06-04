include settings.mk

all: $(LIMINE_DIR)/limine $(FONT_PSF) $(ISO_IMAGE)

$(FONTS_DIR):
	@mkdir -p $(FONTS_DIR)

$(FONT_PSF): $(FONTS_DIR)
	@echo " [FETCH] Fetching $(FONT_NAME)..."
	curl -sL $(FONT_URL) -o $@ || wget -qO $@ $(FONT_URL)

$(LIMINE_DIR)/limine:
	@if [ ! -f "$(LIMINE_DIR)/limine" ]; then \
		echo " [FETCH] Fetching Limine binaries..."; \
		git clone --depth 1 -b $(LIMINE_VERSION) $(LIMINE_URL) $(LIMINE_DIR); \
		$(MAKE) -C $(LIMINE_DIR); \
	fi

$(KERNEL_ELF): $(LIBC_A)
	make -C $(KERNEL_DIR)

$(LIBC_A):
	make -C $(LIBC_DIR)

$(ISO_IMAGE): $(KERNEL_ELF) $(LIMINE_CONF) $(LIMINE_DIR)/limine
	@echo " [ISO] Creating $(ISO_IMAGE)..."
	mkdir -p $(ISO_DIR)/boot
	mkdir -p $(ISO_DIR)/EFI/BOOT
	
	cp $(KERNEL_ELF) $(ISO_DIR)/boot/
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
	$(QEMU) -cdrom $(ISO_IMAGE) \
		-m 256M \
		-M q35 \
		-serial mon:stdio \
		-vga none \
		-device VGA,xres=640,yres=480 \
		-d int,cpu_reset \
		-no-reboot \
		-D qemu.log \
	| tee serial.log

clean:
	rm -rf $(ISO_DIR) qemu.log serial.log $(ISO_IMAGE)
	make -C $(KERNEL_DIR) clean
	make -C $(LIBC_DIR) clean

distclean: clean
	rm -rf $(LIMINE_DIR) $(FONTS_DIR)
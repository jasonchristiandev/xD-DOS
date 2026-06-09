include settings.mk

all: $(LIMINE_DIR)/limine $(FONT_PSF) $(ISO_IMAGE)

FORCE:

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

$(KERNEL_ELF): FORCE $(LIBC_A)
	@$(MAKE) -C $(KERNEL_DIR)

$(LIBC_A): FORCE
	@$(MAKE) -C $(LIBC_DIR)

$(ISO_IMAGE): $(KERNEL_ELF) $(LIMINE_CONF) $(LIMINE_DIR)/limine
	@echo " [ISO] Creating $(ISO_IMAGE)..."
	@mkdir -p $(ISO_DIR)/boot
	@mkdir -p $(ISO_DIR)/EFI/BOOT
	
	@cp $(KERNEL_ELF) $(ISO_DIR)/boot/
	@cp $(LIMINE_CONF) $(ISO_DIR)/boot/
	
	@cp $(LIMINE_DIR)/limine-bios.sys $(ISO_DIR)/boot/
	@cp $(LIMINE_DIR)/limine-bios-cd.bin $(ISO_DIR)/boot/
	@cp $(LIMINE_DIR)/limine-uefi-cd.bin $(ISO_DIR)/boot/
	
	@xorriso -as mkisofs -b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-uefi-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		$(ISO_DIR) -o $(ISO_IMAGE)

	@$(LIMINE_DIR)/limine bios-install $(ISO_IMAGE)
	@echo " [ISO] $(ISO_IMAGE) success."

run: $(ISO_IMAGE)
	$(QEMU) -cdrom $(ISO_IMAGE) \
		-m 256M \
		-M q35 \
		-serial mon:stdio \
		-vga none \
		-device VGA,xres=640,yres=480 \
		-d int,cpu_reset \
		-D qemu.log \
	| tee serial.log

check-target:
ifndef TARGET_VOLUME
	$(error TARGET_VOLUME is not set!)
endif
	@# Check if partition is mounted at root
	@MOUNT=$$(lsblk -no MOUNTPOINT $(TARGET_VOLUME) | head -n 1); \
	if [ "$$MOUNT" = "/" ]; then \
		echo "CRITICAL: Cannot format root partition"; exit 1; \
	fi

grub-install: $(ISO_IMAGE) check-target
	@echo "[INSTALL] Wiping $(TARGET_VOLUME)!"
	@read -p "[INSTALL] Continue? [y/N]: " confirm && [ "$$confirm" = "y" ]
	
	@echo "[INSTALL] Formatting $(TARGET_VOLUME)..."
	sudo umount /dev/nvme0n1p7 || /bin/true
	sudo mkfs.vfat -F 32 -n $(PARTITION_LABEL) $(TARGET_VOLUME)
	
	@echo "[INSTALL] Copying files to $(TARGET_VOLUME)..."
	sudo mkdir -p $(TMP_MOUNT_DIR)
	sudo mount $(TARGET_VOLUME) $(TMP_MOUNT_DIR)
	sudo mkdir -p $(TMP_MOUNT_DIR)/boot
	sudo mkdir -p $(TMP_MOUNT_DIR)/EFI/BOOT
	
	sudo mkdir -p $(TMP_MOUNT_DIR)/boot
	sudo cp $(KERNEL_ELF) $(TMP_MOUNT_DIR)/boot/

	sudo cp $(LIMINE_DIR)/BOOTX64.EFI $(TMP_MOUNT_DIR)/EFI/BOOT/BOOTX64.EFI
	sudo cp $(LIMINE_CONF) $(TMP_MOUNT_DIR)/limine.conf

	$(LIMINE_DIR)/limine bios-install $(ISO_IMAGE)
	sudo umount $(TMP_MOUNT_DIR)
	@echo " [INSTALL] Install success. Be sure to update your GRUB configuration before rebooting."

clean:
	rm -rf $(ISO_DIR) qemu.log serial.log $(ISO_IMAGE)
	make -C $(KERNEL_DIR) clean
	make -C $(LIBC_DIR) clean

distclean: clean
	rm -rf $(LIMINE_DIR) $(FONTS_DIR)
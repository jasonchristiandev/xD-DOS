include settings.mk

all: $(KERNEL_ELF)
	@echo " [ISO] Creating ISO structure..."
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_DIR)/disk/boot/grub
	@cp $(KERNEL_ELF) $(BUILD_DIR)/disk/boot/
	@cp $(GRUB_CFG) $(BUILD_DIR)/disk/boot/grub/grub.cfg
	@grub-mkrescue -o $(ISO_IMAGE) $(BUILD_DIR)/disk

FORCE:

$(FONTS_DIR):
	@mkdir -p $(FONTS_DIR)

$(FONT_PSF): $(FONTS_DIR)
	@echo " [FETCH] Fetching $(FONT_NAME)..."
	curl -sL $(FONT_URL) -o $@ || wget -qO $@ $(FONT_URL)

$(KERNEL_ELF): FORCE $(LIBC_A) $(FONT_PSF)
	@$(MAKE) -C $(KERNEL_DIR)

$(LIBC_A): FORCE
	@$(MAKE) -C $(LIBC_DIR)

run: all
	@rm -f disk/NvVars
	@sync
	$(QEMU) -nodefaults \
		-cdrom $(ISO_IMAGE) \
		-bios /usr/share/ovmf/OVMF.fd \
		-fw_cfg name=opt/org.tianocore/IPv4NetworkStack,string=n \
		-fw_cfg name=opt/org.tianocore/IPv6NetworkStack,string=n \
		-device virtio-rng-pci \
		-m 256M \
		-M q35,accel=kvm \
		-cpu host,-svm \
		-serial mon:stdio \
		-device VGA,xres=640,yres=480 \
		-d int,cpu_reset \
		-D qemu.log \
	| sed -u -e '/BdsDxe/d' -e 's/\x1b\[001;001H//g' | tee serial.log

check-target:
ifndef TARGET_VOLUME
	$(error TARGET_VOLUME is not set!)
endif
	@# Check if partition is mounted at root
	@MOUNT=$$(lsblk -no MOUNTPOINT $(TARGET_VOLUME) | head -n 1); \
	if [ "$$MOUNT" = "/" ]; then \
		echo "CRITICAL: Cannot format root partition"; exit 1; \
	fi

install: $(KERNEL_ELF) check-target
	@echo " [INSTALL] Wiping $(TARGET_VOLUME)!"
	@read -p "[INSTALL] Continue? [y/N]: " confirm && [ "$$confirm" = "y" ]

	@echo " [INSTALL] Formatting $(TARGET_VOLUME)..."
	@sudo umount $(TARGET_VOLUME) || true
	@sudo dd if=/dev/zero of=$(TARGET_VOLUME) bs=1M status=progress || true

ifndef PARTITION_UUID
	@sudo mkfs.vfat -F 32 -n $(PARTITION_LABEL) $(TARGET_VOLUME)
else
	@sudo mkfs.vfat -i $(PARTITION_UUID) -F 32 -n $(PARTITION_LABEL) $(TARGET_VOLUME)
endif

	@echo " [INSTALL] Copying files to $(TARGET_VOLUME)..."
	@sudo mkdir -p $(TMP_MOUNT_DIR)
	@sudo mount $(TARGET_VOLUME) $(TMP_MOUNT_DIR)
	@sudo mkdir -p $(TMP_MOUNT_DIR)/boot
	@sudo mkdir -p $(TMP_MOUNT_DIR)/EFI/BOOT

	@sudo cp $(KERNEL_ELF) $(TMP_MOUNT_DIR)/boot/
	@sudo cp $(BOOTX64_EFI) $(TMP_MOUNT_DIR)/EFI/BOOT/

	@sudo umount $(TMP_MOUNT_DIR)
	@echo " [INSTALL] Install success."

clean:
	rm -rf $(BUILD_DIR) qemu.log serial.log
	make -C $(KERNEL_DIR) clean
	make -C $(LIBC_DIR) clean

distclean: clean
	rm -rf $(FONTS_DIR)
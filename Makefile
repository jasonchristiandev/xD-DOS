include settings.mk

all: $(BOOTX64_EFI) $(KERNEL_ELF)
	@echo " [DISK] Creating boot structure..."

	@mkdir -p $(DISK_DIR)/boot
	@mkdir -p $(DISK_DIR)/EFI/BOOT
	@cp $(KERNEL_ELF) $(DISK_DIR)/boot/
	
	@echo " [DISK] Success."

FORCE:

$(GNU_EFI_DIR):
	@echo " [FETCH] Fetching gnu-efi..."
	git clone $(GNU_EFI_URL) $(GNU_EFI_DIR)

$(GNU_EFI): $(GNU_EFI_DIR)
	@echo " [MAKE] Building gnu-efi..."
	@$(MAKE) -C $(GNU_EFI_DIR) CFLAGS="" CPPFLAGS=""

$(FONTS_DIR):
	@mkdir -p $(FONTS_DIR)

$(FONT_PSF): $(FONTS_DIR)
	@echo " [FETCH] Fetching $(FONT_NAME)..."
	curl -sL $(FONT_URL) -o $@ || wget -qO $@ $(FONT_URL)

$(KERNEL_ELF): FORCE $(LIBC_A) $(FONT_PSF)
	@$(MAKE) -C $(KERNEL_DIR)

$(BOOTX64_EFI): FORCE $(GNU_EFI)
	@$(MAKE) -C $(BOOTLOADER_DIR)

$(LIBC_A): FORCE
	@$(MAKE) -C $(LIBC_DIR)

run: all
	$(QEMU) -nodefaults \
		-drive file=fat:rw:disk,format=raw,media=disk \
		-drive if=pflash,format=raw,readonly=on,file=./OVMF_CODE.fd \
		-drive if=pflash,format=raw,readonly=off,file=./OVMF_VARS.fd \
		-fw_cfg name=opt/org.tianocore/IPv4NetworkStack,string=n \
		-fw_cfg name=opt/org.tianocore/IPv6NetworkStack,string=n \
		-device virtio-rng-pci \
		-fda /dev/null \
		-m 256M \
		-M q35,accel=kvm \
		-cpu host,-svm \
		-serial mon:stdio \
		-vga none \
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
	@echo "[INSTALL] Wiping $(TARGET_VOLUME)!"
	@read -p "[INSTALL] Continue? [y/N]: " confirm && [ "$$confirm" = "y" ]
	
	@echo "[INSTALL] Formatting $(TARGET_VOLUME)..."
	@sudo umount $(TARGET_VOLUME) || true
	@sudo dd if=/dev/zero of=$(TARGET_VOLUME) bs=1M status=progress || true

ifndef PARTITION_UUID
	@sudo mkfs.vfat -F 32 -n $(PARTITION_LABEL) $(TARGET_VOLUME)
else
	@sudo mkfs.vfat -i $(PARTITION_UUID) -F 32 -n $(PARTITION_LABEL) $(TARGET_VOLUME)
endif
	
	@echo "[INSTALL] Copying files to $(TARGET_VOLUME)..."
	@sudo mkdir -p $(TMP_MOUNT_DIR)
	@sudo mount $(TARGET_VOLUME) $(TMP_MOUNT_DIR)
	@sudo mkdir -p $(TMP_MOUNT_DIR)/boot
	@sudo mkdir -p $(TMP_MOUNT_DIR)/EFI/BOOT
	
	@sudo mkdir -p $(TMP_MOUNT_DIR)/boot
	@sudo cp $(KERNEL_ELF) $(TMP_MOUNT_DIR)/boot/

	@sudo umount $(TMP_MOUNT_DIR)
	@echo " [INSTALL] Install success."

clean:
	rm -rf $(DISK_DIR) qemu.log serial.log
	make -C $(KERNEL_DIR) clean
	make -C $(LIBC_DIR) clean
	make -C $(BOOTLOADER_DIR) clean

distclean: clean
	rm -rf $(FONTS_DIR) $(GNU_EFI_DIR)
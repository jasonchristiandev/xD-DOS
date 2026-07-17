ROOT_DIR = $(abspath ..)
include ../settings.mk

CFLAGS += -fshort-wchar
CFLAGS += -I/usr/include/efi
CFLAGS += -I/usr/include/efi/x86_64
CFLAGS += -fPIC

SRCS = $(wildcard $(SRC_DIR)/*.c)
ASMS = $(wildcard $(ASM_DIR)/*.s)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(SRCS)) $(patsubst $(ASM_DIR)/%.s, $(BUILD_DIR)/%.s.o, $(ASMS))
LIBS = $(LIBGNUEFI_A) $(LIBEFI_A)

.PHONY: all run clean

all: $(BOOTX64_EFI)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo " [CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERBOSE=$(VERBOSE)

$(BUILD_DIR)/%.s.o: $(ASM_DIR)/%.s | $(BUILD_DIR)
	@echo " [ASM] $<"
	@$(ASM) -f elf64 $< -o $@

$(BOOTX64_EFI): $(OBJS) $(LIBS) | $(BUILD_DIR)
	@echo " [LD] Linking..."
	@echo "     $(OBJS)"
	@$(LD) -nostdlib -znocombreloc -T $(ELF_X86_64_EFI_LDS) -shared \
		-Bsymbolic -entry=_start -z max-page-size=0x1000 \
		$(CRT0_EFI_X86_64_O) $(OBJS) $(LIBS) -o $(BUILD_DIR)/bootloader.so
	@$(OBJCOPY) -j .text -j .sdata -j .data -j .rodata -j .dynamic \
		-j .dynsym -j .rel -j .rela -j .reloc \
		-I elf64-x86-64 \
		-O pei-x86-64 --subsystem=10 $(BUILD_DIR)/bootloader.so $(BOOTX64_EFI)

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
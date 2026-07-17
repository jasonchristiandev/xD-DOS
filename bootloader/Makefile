ROOT_DIR = $(abspath ..)
include ../settings.mk

CFLAGS += -I/usr/include/efi
CFLAGS += -I/usr/include/efi/x86_64

SRCS = $(wildcard $(SRC_DIR)/*.c)
ASMS = $(wildcard $(ASM_DIR)/*.s)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(SRCS)) $(patsubst $(ASM_DIR)/%.s, $(BUILD_DIR)/%.s.o, $(ASMS))
OBJS += $(CRT0_EFI_X86_64_O)
LIBS = $(LIBGNUEFI_A) $(LIBEFI_A)

.PHONY: all run clean

all: $(BOOTX64_EFI)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo " [CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@ -DDEBUG=$(DEBUG)

$(BUILD_DIR)/%.s.o: $(ASM_DIR)/%.s | $(BUILD_DIR)
	@echo " [ASM] $<"
	@$(ASM) -f elf64 $< -o $@

$(BOOTX64_EFI): $(OBJS) $(LIBS) $(LINKER_SCRIPT) | $(BUILD_DIR)
	@echo " [LD] Linking..."
	@echo "     $(OBJS)"
	@$(LD) -T $(ELF_X86_64_EFI_LDS) $(OBJS) $(LIBS) -o $(BOOTX64_EFI)

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
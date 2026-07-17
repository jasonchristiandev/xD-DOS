ROOT_DIR = $(abspath ..)
include ../settings.mk

CFLAGS += -I$(LIBC_DIR)/$(INCLUDE_DIR)
CFLAGS += -nostdinc

SRCS = $(wildcard $(SRC_DIR)/*.c)
ASMS = $(wildcard $(ASM_DIR)/*.s)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(SRCS)) $(patsubst $(ASM_DIR)/%.s, $(BUILD_DIR)/%.s.o, $(ASMS))
OBJS += $(BUILD_DIR)/font_data.o
LIBS = $(LIBC_A)

.PHONY: all run clean

all: $(KERNEL_ELF)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo " [CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERBOSE=$(VERBOSE)

$(BUILD_DIR)/%.s.o: $(ASM_DIR)/%.s | $(BUILD_DIR)
	@echo " [ASM] $<"
	@$(ASM) -f elf64 $< -o $@

FONT_STEM = $(subst -,_,$(subst .,_,$(notdir $(FONT_PSF))))

$(BUILD_DIR)/font_data.o: $(FONT_PSF) | $(BUILD_DIR)
	@echo " [OBJCOPY] $<"
	@cd $(dir $(FONT_PSF)); \
	objcopy -I binary -O elf64-x86-64 -B i386 \
		--rename-section .data=.data,alloc,load,readonly,data,contents \
		$(notdir $(FONT_PSF)) $(abspath $@)
	@objcopy $@ $@ \
		--redefine-sym _binary_$(FONT_STEM)_start=_binary_font_psf_start \
		--redefine-sym _binary_$(FONT_STEM)_end=_binary_font_psf_end \
		--redefine-sym _binary_$(FONT_STEM)_size=_binary_font_psf_size

$(KERNEL_ELF): $(OBJS) $(LIBS) $(LINKER_SCRIPT) | $(BUILD_DIR)
	@echo " [LD] Linking..."
	@echo "     $(OBJS)"
	@$(LD) -T $(LINKER_SCRIPT) $(OBJS) $(LIBS) -o $(KERNEL_ELF)

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
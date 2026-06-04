ROOT_DIR = $(abspath ..)
include ../settings.mk

LINKER_SCRIPT = linker.lds

SRCS = $(wildcard $(SRC_DIR)/*.c)
ASMS = $(wildcard $(ASM_DIR)/*.s)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS)) $(patsubst $(ASM_DIR)/%.s, $(BUILD_DIR)/%.o, $(ASMS))
OBJS += $(BUILD_DIR)/font_data.o
LIBS = $(LIBC_A)

CFLAGS = -fno-stack-protector -ffreestanding -mno-red-zone \
		 -fms-extensions -nostdlib -nostdinc -Wall -Wextra \
		 -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast \
		 -Wno-compare-distinct-pointer-types -Wno-enum-conversion \
		 -Wno-variadic-macros -Wno-enum-compare -std=c11 -O2 \
		 -mno-mmx -mno-sse -mno-sse2 -MMD -MP -I$(INCLUDE_DIR) \
		 -I$(LIBC_DIR)/$(INCLUDE_DIR) -I$(LIMINE_PROTOCOL)/include

.PHONY: all run clean

all: $(KERNEL_ELF)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c $(LIMINE_H) | $(BUILD_DIR)
	@echo " [CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@ -DDEBUG=$(DEBUG)

$(BUILD_DIR)/%.o: $(ASM_DIR)/%.s | $(BUILD_DIR)
	@echo " [ASM] $<"
	@$(ASM) -f elf64 $< -o $@

FONT_STEM = $(subst -,_,$(subst .,_,$(notdir $(FONT_PSF))))

$(BUILD_DIR)/font_data.o: $(FONT_PSF) | $(BUILD_DIR)
	@echo " [OBJCOPY] $<"
	cd $(dir $(FONT_PSF)); \
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
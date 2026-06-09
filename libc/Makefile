ROOT_DIR = $(abspath ..)
include ../settings.mk

# LINKER_SCRIPT = linker.lds

SRCS = $(wildcard $(SRC_DIR)/*.c)
ASMS = $(wildcard $(ASM_DIR)/*.s)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(SRCS)) $(patsubst $(ASM_DIR)/%.s, $(BUILD_DIR)/%.s.o, $(ASMS))

CFLAGS = -fno-stack-protector -ffreestanding -mno-red-zone \
		 -fms-extensions -nostdlib -nostdinc -Wall -Wextra \
		 -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast \
		 -Wno-compare-distinct-pointer-types -Wno-enum-conversion \
		 -Wno-variadic-macros -Wno-enum-compare -std=c11 -O2 \
		 -mno-mmx -mno-sse -mno-sse2 -MMD -MP -I$(INCLUDE_DIR) \
		 -I$(KERNEL_DIR)/$(INCLUDE_DIR) -I$(LIMINE_PROTOCOL)/include

.PHONY: all run clean

all: $(LIBC_A)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo " [CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@ -DDEBUG=$(DEBUG)

$(BUILD_DIR)/%.s.o: $(ASM_DIR)/%.s | $(BUILD_DIR)
	@echo " [ASM] $<"
	@$(ASM) -f elf64 $< -o $@

$(LIBC_A): $(OBJS)
	@echo " [AR] Archiving..."
	@echo "     $(OBJS)"
	@ar rcs $@ $(OBJS)

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
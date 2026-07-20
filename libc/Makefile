ROOT_DIR = $(abspath ..)
include ../settings.mk

CFLAGS += -I$(KERNEL_DIR)/$(INCLUDE_DIR)
CFLAGS += -nostdinc -mno-mmx -mno-sse -mno-sse2 -MMD -MP

SRCS = $(wildcard $(SRC_DIR)/*.c)
ASMS = $(wildcard $(ASM_DIR)/*.s)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(SRCS)) $(patsubst $(ASM_DIR)/%.s, $(BUILD_DIR)/%.s.o, $(ASMS))

.PHONY: all run clean

all: $(LIBC_A)

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.c.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	@echo " [CC] $<"
	@$(CC) $(CFLAGS) -c $< -o $@ -DVERBOSE=$(VERBOSE)

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
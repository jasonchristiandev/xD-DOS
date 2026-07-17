ROOT_DIR = $(abspath ..)
include ../settings.mk

SRCS = $(wildcard $(SRC_DIR)/*.c)
ASMS = $(wildcard $(ASM_DIR)/*.s)
OBJS = $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.c.o, $(SRCS)) $(patsubst $(ASM_DIR)/%.s, $(BUILD_DIR)/%.s.o, $(ASMS))
LIBS = 

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
	@$(LD) -T $(LINKER_SCRIPT) $(OBJS) $(LIBS) -o $(BOOTX64_EFI)

clean:
	rm -rf $(BUILD_DIR)

-include $(OBJS:.o=.d)
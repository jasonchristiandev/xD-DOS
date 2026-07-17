export

VERBOSE ?= 0

CC ?= gcc
ASM ?= nasm
LD ?= ld
OBJCOPY ?= objcopy
MAKE ?= make
QEMU ?= qemu-system-x86_64

ROOT_DIR ?= $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
DEPS_DIR = $(ROOT_DIR)/deps
SRC_DIR = src
ASM_DIR = asm
INCLUDE_DIR = include
BUILD_DIR = build
BOOTLOADER_DIR = $(ROOT_DIR)/bootloader
KERNEL_DIR = $(ROOT_DIR)/kernel
LIBC_DIR = $(ROOT_DIR)/libc
DISK_DIR = $(ROOT_DIR)/disk
FONTS_DIR = $(ROOT_DIR)/fonts
TMP_MOUNT_DIR = /mnt/xddos_tmp

BOOTX64_EFI = $(BOOTLOADER_DIR)/$(BUILD_DIR)/BOOTX64.EFI
KERNEL_ELF = $(KERNEL_DIR)/$(BUILD_DIR)/kernel.elf
LIBC_A = $(LIBC_DIR)/$(BUILD_DIR)/libc.a
FONT_PSF = $(FONTS_DIR)/font.psf
LINKER_SCRIPT = linker.lds

FONT_NAME = cp850-8x16.psf
FONT_URL = https://raw.githubusercontent.com/ercanersoy/PSF-Fonts/master/$(FONT_NAME)

PARTITION_LABEL = XDDOS

CFLAGS = -fno-stack-protector -ffreestanding -mno-red-zone \
		 -fms-extensions -nostdlib -Wall -Wextra \
		 -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast \
		 -Wno-compare-distinct-pointer-types -Wno-enum-conversion \
		 -Wno-variadic-macros -Wno-enum-compare -std=c11 -O2 \
		 -fno-strict-aliasing -I$(INCLUDE_DIR)
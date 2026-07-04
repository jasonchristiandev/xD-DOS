export

DEBUG ?= 0

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
KERNEL_DIR = $(ROOT_DIR)/kernel
LIBC_DIR = $(ROOT_DIR)/libc
ISO_DIR = $(ROOT_DIR)/iso
LIMINE_DIR = $(ROOT_DIR)/limine-bin
FONTS_DIR = $(ROOT_DIR)/fonts
TMP_MOUNT_DIR = /mnt/xddos_tmp

LIMINE_PROTOCOL = $(DEPS_DIR)/limine-protocol
UACPI_DIR = $(DEPS_DIR)/uACPI

KERNEL_ELF = $(KERNEL_DIR)/$(BUILD_DIR)/kernel.elf
LIBC_A = $(LIBC_DIR)/$(BUILD_DIR)/libc.a
ISO_IMAGE = $(ROOT_DIR)/xD-DOS.iso
LIMINE_H = $(LIMINE_PROTOCOL)/include/limine.h
FONT_PSF = $(FONTS_DIR)/font.psf
LIMINE_CONF = $(ROOT_DIR)/limine.conf
LINKER_SCRIPT = linker.lds

FONT_NAME = cp850-8x16.psf
FONT_URL = https://raw.githubusercontent.com/ercanersoy/PSF-Fonts/master/$(FONT_NAME)
LIMINE_VERSION = v11.x-binary
LIMINE_URL = https://github.com/limine-bootloader/limine.git

PARTITION_LABEL = XDDOS

CFLAGS = -fno-stack-protector -ffreestanding -mno-red-zone \
		 -fms-extensions -nostdlib -nostdinc -Wall -Wextra \
		 -Wno-pointer-to-int-cast -Wno-int-to-pointer-cast \
		 -Wno-compare-distinct-pointer-types -Wno-enum-conversion \
		 -Wno-variadic-macros -Wno-enum-compare -std=c11 -O2 \
		 -mno-mmx -mno-sse -mno-sse2 -MMD -MP -I$(INCLUDE_DIR)
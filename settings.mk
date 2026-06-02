DEBUG ?= 0

CC = gcc
LD = ld
OBJCOPY = objcopy
MAKE = make
QEMU = qemu-system-x86_64

ROOT_DIR = $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
KERNEL_DIR = $(ROOT_DIR)/kernel
ISO_DIR = $(ROOT_DIR)/iso
LIBC_DIR = $(ROOT_DIR)/libc
LIMINE_DIR = $(ROOT_DIR)/limine-bin
FONTS_DIR = $(ROOT_DIR)/fonts
LIMINE_PROTOCOL = $(ROOT_DIR)/limine-protocol

KERNEL_ELF = $(KERNEL_DIR)/$(BUILD_DIR)/kernel.elf
ISO_IMAGE = $(ROOT_DIR)/xD-DOS.iso
LIMINE_H = $(LIMINE_PROTOCOL)/include/limine.h
FONT_PSF = $(FONTS_DIR)/font.psf
LIMINE_CONF = $(ROOT_DIR)/limine.conf

FONT_NAME = cp850-8x16.psf
FONT_URL = https://raw.githubusercontent.com/ercanersoy/PSF-Fonts/master/$(FONT_NAME)
LIMINE_VERSION = v11.x-binary
LIMINE_URL = https://github.com/limine-bootloader/limine.git

export CC LD OBJCOPY MAKE QEMU DEBUG ROOT_DIR SRC_DIR INCLUDE_DIR BUILD_DIR KERNEL_DIR
export ISO_DIR LIBC_DIR LIMINE_DIR FONTS_DIR LIMINE_PROTOCOL KERNEL_ELF ISO_IMAGE LIMINE_H FONT_PSF LINKER_SCRIPT
#Makefile

CC = gcc
CFLAGS = -fno-pie -no-pie -nostdlib -m32 -fno-builtin -O0 -fno-common

LFLAGS = -m elf_i386

LIB = $(TARGET_DIR)/lib/apilib.lib
LS = ../../../scripts/app.lds

TARGET_DIR = ../../../bin/app
DIST_DIR = $(TARGET_DIR)/dist
TMP_DIR = $(TARGET_DIR)/tmp/color2

APP = $(DIST_DIR)/color2.hrb
APP_SRC = color2.c

INCLUDE = -I ../lib/include

$(APP): $(APP_SRC) $(LIB)
	mkdir -p $(DIST_DIR)
	mkdir -p $(TMP_DIR)
	$(CC) -c $(CFLAGS) $(INCLUDE) -o $(TMP_DIR)/$(APP_SRC:.c=.o) $(APP_SRC)
	ld $(LFLAGS) -o $(APP) -e HariMain -T $(LS) $(TMP_DIR)/$(APP_SRC:.c=.o) $(LIB)

$(LIB):
	cd ../lib; make

clean:
	rm -rf $(TMP_DIR)
	rm -f $(APP)

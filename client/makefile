CC=gcc

SRC_DIR=src
BUILD_DIR=build

SDL3_BUILD_DIR := ../SDL3_git/build
SDL3_INSTALL_DIR := ../SDL3-x64


CFLAGS= -Wall -MMD -g -Wextra -I../SDL3-x64/include -I../SDL3-ttf-x64/include $(pkg-config --cflags harfbuzz)
LDFLAGS=-lcrypto -L../SDL3-x64/lib -L../SDL3-ttf-x64/lib -lSDL3 -lSDL3_ttf -lfreetype -lpng -lz -lbrotlidec -lbrotlicommon -L/usr/lib/x86_64-linux-gnu $(shell pkg-config --libs harfbuzz) -lbz2 -lsodium -lm #-static# -lpthread


#-fsanitize=thread

EXEC=res

SRC=$(wildcard $(SRC_DIR)/*.c)
OBJ=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))
DEPS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.d,$(SRC))


all: $(EXEC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

build_version:
	cd includes; ./generate_version.sh

sdl3_build:
	mkdir -p $(SDL3_BUILD_DIR)
	cd $(SDL3_BUILD_DIR) && \
		cmake -DSDL_STATIC=ON -DSDL_SHARED=OFF -DCMAKE_INSTALL_PREFIX=../$(SDL3_INSTALL_DIR) ..
	cd $(SDL3_BUILD_DIR) && $(MAKE) && $(MAKE) install

$(EXEC):$(OBJ) 
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)

$(OBJ):$(BUILD_DIR) build_version sdl3_build

-include $(DEPS)

.PHONY:clean mrproper clangd

clean:
	rm -rfd build
	rm -rfd $(SDL3_BUILD_DIR)

mrproper: clean
	rm -rf $(EXEC)
	rm -rf compile_commands.json
	rm -rf .clangd
	rm -rfd savedFiles
	rm -rfd $(SDL3_INSTALL_DIR)


clangd: clean
	bear make
	

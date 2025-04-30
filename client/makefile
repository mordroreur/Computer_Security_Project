CC=gcc

SRC_DIR=src
BUILD_DIR=build


CFLAGS= -Wall -MMD -g -Wextra -I../SDL3-x64/include -I../SDL3-ttf-x64/include $(pkg-config --cflags harfbuzz)
LDFLAGS=-lcrypto -L../SDL3-x64/lib -L../SDL3-ttf-x64/lib -lSDL3 -lSDL3_ttf -lfreetype -lpng -lz -lbrotlidec -lbrotlicommon -L/usr/lib/x86_64-linux-gnu $(shell pkg-config --libs harfbuzz) -lbz2 -lsodium -lm #-static# -lpthread


#-fsanitize=thread

EXEC=res

SRC=$(wildcard $(SRC_DIR)/*.c)
OBJ=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))
DEPS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.d,$(SRC))


all:$(BUILD_DIR) build_version $(EXEC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

build_version:
	cd includes; ./generate_version.sh

$(EXEC):$(OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -o $@ -c $< $(CFLAGS)


-include $(DEPS)

.PHONY:clean mrproper clangd

clean:
	rm -rfd build

mrproper: clean
	rm -rf $(EXEC)
	rm -rf compile_commands.json
	rm -rf .clangd
	rm -rfd savedFiles

clangd: clean
	bear make

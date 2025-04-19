CC=gcc

SRC_DIR=src
BUILD_DIR=build


CFLAGS= -Wall -MMD -g -Wextra -I../SDL3-x64/include
LDFLAGS=-L../SDL3-x64/lib -lSDL3 -lm # -lpthread -D_REENTRANT  -L../SDL3-x64/lib


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

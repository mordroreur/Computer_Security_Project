CC=x86_64-w64-mingw32-gcc # Use mingw-w64 cross-compiler for 64-bit Windows

SRC_DIR=../src
BUILD_DIR=build

CFLAGS=-Wall -MMD -Wextra -I'../includes/' -I'../../openssl/usr/local/include'
LDFLAGS= 	-lmingw32 \
			-L'../../openssl/usr/local/lib64' -lssl -lcrypto \
			-lws2_32 -lcrypt32 \
			-pthread



# -lmingw32 : for opening a Window
# -L'../../openssl/usr/local/lib64' -lssl -lcrypto : for getting openssl function
# -lws2_32 -lcrypt32 : All the function needed by at least openssl
# -mwindows : for no console windows
# -lm 

EXEC=res.exe # Windows executable file


SRC=$(wildcard $(SRC_DIR)/*.c)
OBJ=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))
DEPS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.d,$(SRC))


all:$(BUILD_DIR) $(EXEC)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

build_version:
	cd ../includes; ./generate_version.sh

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

clangd: clean
	bear make


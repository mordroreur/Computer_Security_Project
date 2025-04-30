CC=x86_64-w64-mingw32-gcc # Use mingw-w64 cross-compiler for 64-bit Windows

SRC_DIR=../src
BUILD_DIR=build

CFLAGS=-Wall -MMD -Wextra -I./SDL3-x64-win/include -I./SDL3-ttf-x64-win/include -I'../includes/' -I'../../openssl/usr/local/include' -I'../../libsodium/include'
LDFLAGS=-L./SDL3-x64-win/lib -lSDL3 \
		-L./SDL3-ttf-x64-win/lib -lSDL3_ttf \
		-L./freetype-win/lib -lfreetype\
		-L'../../openssl/usr/local/lib64' -lssl -lcrypto \
		-L'../../libsodium/lib' -lsodium\
		-lws2_32 -lcrypt32 \
		-pthread \
		-lmingw32 -lwinmm -lole32 -loleaut32 -lsetupapi -limm32 -lversion -luuid -lgdi32 \
		-lm -static -static-libgcc -static-libstdc++




			



# -mwindows
# -L./SDL3-x64/lib -lSDL3 -lmingw32 -lwinmm -lole32 -lm -mwindows -static  # -mwindows
# -lmingw32 for Windows support
# -mwindows for no console windows

EXEC=res.exe # Windows executable file


SRC=$(wildcard $(SRC_DIR)/*.c)
OBJ=$(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRC))
DEPS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.d,$(SRC))


all:$(BUILD_DIR) build_version $(EXEC)

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
	rm -rfd savedFiles

clangd: clean
	bear make


APP_NAME = Study-with-this.app
APP_DIR  = $(APP_NAME)
APP_MACOS = $(APP_DIR)/Contents/MacOS
APP_FW    = $(APP_DIR)/Contents/Frameworks
APP_RES   = $(APP_DIR)/Contents/Resources

UNAME_S := $(shell uname -s 2>/dev/null || echo Unknown)

# default = linux
CC := gcc
CFLAGS := -Wall -g
RPATH :=
PLATFORM_SRC := src/platform/platform_posix.c

# windows detection
ifeq ($(OS),Windows_NT)
	PLATFORM_SRC := src/platform/platform_win.c
else
	# uname based detection
	ifneq (,$(filter MINGW% MSYS% CYGWIN%,$(UNAME_S)))
		PLATFORM_SRC := src/platform/platform_win.c
	endif
endif

# now for macos, use clang compiler and relative path
ifeq ($(UNAME_S),Darwin)
	CC := clang
	RPATH := -Wl,-rpath,@executable_path/../Frameworks
endif

PKG_CONFIG_FLAGS := sdl2 SDL2_ttf SDL2_mixer
INCLUDES := -I./include $(shell pkg-config --cflags $(PKG_CONFIG_FLAGS))
LIBS := $(shell pkg-config --libs $(PKG_CONFIG_FLAGS)) -lm

OBJFILES := main.o cJSON.o settings.o pomodoro.o graphics.o music.o platform.o
TARGET = study-with-this

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(OBJFILES) -o $(TARGET) $(LIBS) $(RPATH)

main.o: src/main.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

settings.o: src/settings.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

cJSON.o: src/cJSON.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

pomodoro.o: src/pomodoro.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

graphics.o: src/graphics.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

music.o: src/music.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

platform.o: $(PLATFORM_SRC)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: app bundle fixup verify clean

app: $(TARGET)
ifeq ($(UNAME_S),Darwin)
	$(MAKE) bundle
	$(MAKE) fixup
	$(MAKE) verify
endif

bundle: macos/Info.plist resources/icon.icns resources/bell1.mp3
	@mkdir -p "$(APP_MACOS)" "$(APP_FW)" "$(APP_RES)"
	@cp "$(TARGET)" "$(APP_MACOS)/$(TARGET)"

	# Bundle metadata + resources
	@cp "macos/Info.plist" "$(APP_DIR)/Contents/Info.plist"
	@cp "resources/icon.icns" "$(APP_RES)/icon.icns"

	# Copy the three direct deps in otool -L:
	@cp /opt/homebrew/opt/sdl2/lib/libSDL2*.dylib "$(APP_FW)/"
	@cp /opt/homebrew/opt/sdl2_ttf/lib/libSDL2_ttf*.dylib "$(APP_FW)/"
	@cp /opt/homebrew/opt/sdl2_mixer/lib/libSDL2_mixer*.dylib "$(APP_FW)/"

fixup:
	@bash scripts/macos_bundle_deps.sh "$(APP_MACOS)/$(TARGET)" "$(APP_FW)"

verify:
	@echo "== otool -L on app binary =="
	@otool -L "$(APP_MACOS)/$(TARGET)"
	@echo "== plist sanity =="
	@plutil -lint "$(APP_DIR)/Contents/Info.plist"

clean:
	rm -f $(OBJFILES) $(TARGET)
	rm -rf $(APP_DIR)

APP_NAME = Study-with-this.app
APP_DIR  = $(APP_NAME)
APP_MACOS = $(APP_DIR)/Contents/MacOS
APP_FW    = $(APP_DIR)/Contents/Frameworks
APP_RES   = $(APP_DIR)/Contents/Resources
PKG_CONFIG_FLAGS := sdl2 SDL2_ttf SDL2_mixer

UNAME_S := $(shell uname -s 2>/dev/null || echo Unknown)

# default = linux
CC := gcc
CFLAGS := -Wall -g
RPATH :=
PLATFORM_SRC := src/platform/platform_posix.c
SDL_CFLAGS := $(shell pkg-config --cflags $(PKG_CONFIG_FLAGS))
SDL_LIBS   := $(shell pkg-config --libs   $(PKG_CONFIG_FLAGS))
OTHER_LIBS :=

# windows flag
IS_WINDOWS   := 0

# windows detection (either via OS=Windows_NT or uname based)
ifeq ($(OS),Windows_NT)
	IS_WINDOWS := 1

endif
ifneq (,$(filter MINGW% MSYS% CYGWIN%,$(UNAME_S)))
	IS_WINDOWS := 1
endif

# macOS specific behaviour
ifeq ($(UNAME_S),Darwin)
	CC := clang
	RPATH := -Wl,-rpath,@executable_path/../Frameworks
endif

ifeq ($(IS_WINDOWS),1)
	PLATFORM_SRC := src/platform/platform_win.c

	CFLAGS += -DSDL_MAIN_HANDLED

	SDL_CFLAGS := $(filter-out -Dmain=SDL_main,$(SDL_CFLAGS))
	SDL_LIBS   := $(filter-out -lSDL2main,$(SDL_LIBS))
	OTHER_LIBS := -lole32 -lshell32 -luuid

	# for resources. get mingw directory and copy required dlls from there.
	APP_DIR    := dist
	MINGW_BIN  := $(shell dirname $(shell which gcc))
	DLLS       := SDL2.dll SDL2_ttf.dll SDL2_mixer.dll

	# for icon
	APP_ICON_RC  := resources/appicon.rc
   	APP_ICON_RES := appicon.res
endif

INCLUDES := -I./include $(SDL_CFLAGS)
LIBS     := $(SDL_LIBS) -lm $(OTHER_LIBS)

OBJFILES := main.o cJSON.o settings.o pomodoro.o graphics.o music.o platform.o
TARGET = study-with-this

all: $(TARGET)

# link rules should be different between Windows vs others. What a headache!
ifeq ($(IS_WINDOWS),1)
$(TARGET): $(OBJFILES) $(APP_ICON_RES)
	$(CC) $(OBJFILES) $(APP_ICON_RES) -o $(TARGET) $(LIBS) $(RPATH)

$(APP_ICON_RES): $(APP_ICON_RC)
	windres $< -O coff -o $@
else
$(TARGET): $(OBJFILES)
	$(CC) $(OBJFILES) -o $(TARGET) $(LIBS) $(RPATH)
endif

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

.PHONY: app bundle dist fixup verify clean

app: $(TARGET)
ifeq ($(UNAME_S),Darwin)
	$(MAKE) bundle
	$(MAKE) fixup
	$(MAKE) verify
endif

# macos
bundle: macos/Info.plist resources/icon.icns
	@mkdir -p "$(APP_MACOS)" "$(APP_FW)" "$(APP_RES)"
	@cp "$(TARGET)" "$(APP_MACOS)/$(TARGET)"

	# Bundle metadata + resources
	@cp "macos/Info.plist" "$(APP_DIR)/Contents/Info.plist"
	@cp "resources/icon.icns" "$(APP_RES)/icon.icns"

	# Copy the three direct deps in otool -L:
	@cp /opt/homebrew/opt/sdl2/lib/libSDL2*.dylib "$(APP_FW)/"
	@cp /opt/homebrew/opt/sdl2_ttf/lib/libSDL2_ttf*.dylib "$(APP_FW)/"
	@cp /opt/homebrew/opt/sdl2_mixer/lib/libSDL2_mixer*.dylib "$(APP_FW)/"

dist: $(TARGET)
ifeq ($(IS_WINDOWS),1)
	rm -rf $(APP_DIR)
	mkdir -p $(APP_DIR)
	cp $(TARGET).exe $(APP_DIR)
	for f in $(DLLS); do \
		cp "$(MINGW_BIN)/$$f" $(APP_DIR)/; \
	done
else
	@echo "dist only works on Windows."
endif

fixup:
	@bash scripts/macos_bundle_deps.sh "$(APP_MACOS)/$(TARGET)" "$(APP_FW)"

verify:
	@echo "== otool -L on app binary =="
	@otool -L "$(APP_MACOS)/$(TARGET)"
	@echo "== plist sanity =="
	@plutil -lint "$(APP_DIR)/Contents/Info.plist"

clean:
	rm -f $(OBJFILES) $(TARGET) $(APP_ICON_RES)
	rm -rf $(APP_DIR)

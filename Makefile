APP_NAME = Study-with-this.app
APP_DIR  = $(APP_NAME)
APP_MACOS = $(APP_DIR)/Contents/MacOS
APP_FW    = $(APP_DIR)/Contents/Frameworks
APP_RES   = $(APP_DIR)/Contents/Resources

UNAME_S := $(shell uname -s)
CC = gcc
CFLAGS = -Wall -g

ifeq ($(UNAME_S),Darwin)
    # Use clang if macos
    CC = clang
    # Add an rpath for the binary to search inside the app bundle
    RPATH = -Wl,-rpath,@executable_path/../Frameworks
else
    RPATH =
endif

PKG_CONFIG_FLAGS := sdl2 SDL2_ttf SDL2_mixer
INCLUDES := -I./include $(shell pkg-config --cflags $(PKG_CONFIG_FLAGS))
LIBS := $(shell pkg-config --libs $(PKG_CONFIG_FLAGS)) -lm

OBJFILES = main.o cJSON.o settings.o pomodoro.o graphics.o music.o
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

.PHONY: app bundle fixup verify

app: $(TARGET) bundle fixup verify

bundle:
	@mkdir -p "$(APP_MACOS)" "$(APP_FW)"
	@cp "$(TARGET)" "$(APP_MACOS)/$(TARGET)"

   	# Bundle metadata + resources
	@cp "macos/Info.plist" "$(APP_DIR)/Contents/Info.plist"
	@cp "resources/icon.icns" "$(APP_RES)/icon.icns"


	# Copy the three direct deps you already saw in otool -L:
	@cp /opt/homebrew/opt/sdl2/lib/libSDL2-2.0.0.dylib "$(APP_FW)/"
	@cp /opt/homebrew/opt/sdl2_ttf/lib/libSDL2_ttf-2.0.0.dylib "$(APP_FW)/"
	@cp /opt/homebrew/opt/sdl2_mixer/lib/libSDL2_mixer-2.0.0.dylib "$(APP_FW)/"

fixup:
	@bash scripts/macos_bundle_deps.sh "$(APP_MACOS)/$(TARGET)" "$(APP_FW)"

verify:
	@echo "== otool -L on app binary =="
	@otool -L "$(APP_MACOS)/$(TARGET)"
	@echo "== plist sanity =="
	@plutil -lint "$(APP_DIR)/Contents/Info.plist"

clean:
	rm -f $(OBJFILES) $(TARGET)

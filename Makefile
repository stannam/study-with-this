UNAME_S := $(shell uname -s)
CC = gcc
CFLAGS = -Wall -g

# Use clang if macos
ifeq ($(UNAME_S),Darwin)
    CC = clang
endif

PKG_CONFIG_FLAGS := sdl2 SDL2_ttf SDL2_mixer
INCLUDES := -I./include $(shell pkg-config --cflags $(PKG_CONFIG_FLAGS))
LIBS := $(shell pkg-config --libs $(PKG_CONFIG_FLAGS)) -lm

OBJFILES = main.o cJSON.o settings.o pomodoro.o graphics.o music.o
TARGET = study-with-this

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(OBJFILES) -o $(TARGET) $(LIBS)

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

clean:
	rm -f $(OBJFILES) $(TARGET)

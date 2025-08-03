#include "music.h"
#include <SDL.h>
#include <SDL_mixer.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define MAX_HISTORY_SIZE 100   // upper bound for played music memory

static Mix_Chunk *alarm_chunk     = NULL;
static Mix_Music *current_music   = NULL;
static char     **lofi_paths      = NULL;
static int       *recent_history  = NULL;
static int        history_size    = 0;
static int        history_index   = 0;
static int        lofi_count      = 0;
static int        current_volume  = MIX_MAX_VOLUME/2;
static int        previous_volume = MIX_MAX_VOLUME/2;
static int        alarm_channel   = -1;
static int        current_index   = 0;                  // index of the last‐played track
static bool       muted           = false;

// Helper: case‐insensitive extension check
static bool has_ext(const char *fname, const char *ext) {
    size_t fl = strlen(fname), el = strlen(ext);
    if (fl < el+1) return false;
    return strcasecmp(fname + fl - el, ext) == 0;
}

// Helper: make sure if the song has been played recently.
static bool is_recent(int index) {
    for (int i = 0; i < history_size; i++) {
        if (recent_history[i] == index) return true;
    }
    return false;
}

const char* get_current_lofi_name(void) {
    const char *full = lofi_paths[current_index];
    const char *base = strrchr(full, '/');
    return base ? base + 1 : full;
}

int init_audio(const Settings *settings) {
    srand((unsigned)time(NULL));                   // seeding the random number generator choosing a lofi music

    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        fprintf(stderr, "Mix_OpenAudio Error: %s\n", Mix_GetError());
        return 0;
    }
    Mix_HookMusicFinished(play_lofi);

    // Load alarm chunk
    alarm_chunk = Mix_LoadWAV(settings->alarm_sound);
    if (!alarm_chunk) {
        fprintf(stderr, "Mix_LoadWAV Error: %s\n", Mix_GetError());
        printf("[DEBUG] Full alarm path: %s\n", settings->alarm_sound);
        return 0;
    }

    // Scan music directory for files with mp3, wav, ogg
    DIR *d = opendir(settings->music_directory);
    if (!d) {
        fprintf(stderr, "Could not open music directory: %s\n", settings->music_directory);
        return 0;
    }
    struct dirent *ent;
    // First pass: count files
    while ((ent = readdir(d))) {
        if (ent->d_type == DT_REG &&
            (has_ext(ent->d_name, ".mp3")
             || has_ext(ent->d_name, ".wav")
             || has_ext(ent->d_name, ".ogg"))) {
            lofi_count++;
        }
    }
    if (lofi_count == 0) {
        fprintf(stderr, "No audio files found in: %s\n", settings->music_directory);
        closedir(d);
        return 0;
    }
    rewinddir(d);

    // initialize played music memory of size a third of lofi_count
    history_size = lofi_count / 3;
    if (history_size > MAX_HISTORY_SIZE) history_size = MAX_HISTORY_SIZE;
    if (history_size < 1) history_size = 1;

    recent_history = calloc(history_size, sizeof(int));
    for (int i = 0; i < history_size; i++) recent_history[i] = -1;  // initialize all slots with -1

    // Allocate and fill path list
    lofi_paths = malloc(lofi_count * sizeof(char*));
    int idx = 0;
    while ((ent = readdir(d))) {
        if (ent->d_type == DT_REG &&
            (has_ext(ent->d_name, ".mp3")
             || has_ext(ent->d_name, ".wav")
             || has_ext(ent->d_name, ".ogg"))) {
            size_t len = strlen(settings->music_directory) + 1 + strlen(ent->d_name) + 1;
            char *full = malloc(len);
            snprintf(full, len, "%s/%s", settings->music_directory, ent->d_name);
            lofi_paths[idx++] = full;
        }
    }
    closedir(d);

    // Set initial volume
    current_volume = previous_volume = MIX_MAX_VOLUME/2;
    Mix_VolumeMusic(current_volume);
    Mix_VolumeChunk(alarm_chunk, current_volume);
    muted = false;

    current_music = NULL;
    return 1;
}

// called when the program terminates. cleans up the audio
void cleanup_audio(void) {
    stop_lofi();
    if (alarm_chunk) { Mix_FreeChunk(alarm_chunk); alarm_chunk = NULL; }

    for (int i = 0; i < lofi_count; i++) {
        free(lofi_paths[i]);
    }
    free(lofi_paths);
    lofi_paths = NULL;
    lofi_count = 0;

    free(recent_history);
    recent_history = NULL;

    Mix_CloseAudio();
}

void play_lofi(void) {
    if (muted) return;
    stop_lofi();

    // Pick random track
    do {
        current_index = rand() % lofi_count;
    } while (is_recent(current_index));

    recent_history[history_index] = current_index;
    history_index = (history_index + 1) % history_size;

    Mix_Music *m = Mix_LoadMUS(lofi_paths[current_index]);
    if (!m) {
        fprintf(stderr, "Mix_LoadMUS Error (%s): %s\n", lofi_paths[current_index], Mix_GetError());
        return;
    }
    current_music = m;

    if (Mix_PlayMusic(current_music, 0) == -1) {
        fprintf(stderr, "Mix_PlayMusic Error: %s\n", Mix_GetError());
        return;
    }

    return;
}

void stop_lofi(void) {
    if (current_music) {
        Mix_HaltMusic();
        Mix_FreeMusic(current_music);
        current_music = NULL;
    }
}

bool is_lofi_playing(void) {
    return Mix_PlayingMusic() != 0;
}

int play_alarm(void) {
    if (muted) return -1;
    stop_lofi();
    alarm_channel = Mix_PlayChannel(-1, alarm_chunk, 0);
    if (alarm_channel < 0) fprintf(stderr, "Mix_PlayChannel Error. alarm_channel < 0: %s\n", Mix_GetError());
    return alarm_channel;
}

int get_alarm_channel(void) {
    return alarm_channel;
}

bool is_alarm_playing(void) {
    return (alarm_channel >= 0) && Mix_Playing(alarm_channel);
}

void set_volume(int level) {
    if (level < 0) level = 0;
    if (level > MIX_MAX_VOLUME) level = MIX_MAX_VOLUME;
    current_volume = level;
    if (!muted) {
        Mix_VolumeMusic(current_volume);
        Mix_VolumeChunk(alarm_chunk, current_volume);
    }
}

void adjust_volume(int delta){
    int new_vol = current_volume + delta;
    if (new_vol < 0) new_vol = 0;
    if (new_vol > MIX_MAX_VOLUME) new_vol = MIX_MAX_VOLUME;
    set_volume(new_vol);
}

int get_volume_percent(void){
    if (muted) return 0;
    return (current_volume  * 100 / MIX_MAX_VOLUME);
}

void toggle_mute(void) {
    muted = !muted;
    if (muted) {
        Mix_VolumeMusic(0);
        Mix_VolumeChunk(alarm_chunk, 0);
    } else {
        Mix_VolumeMusic(current_volume);
        Mix_VolumeChunk(alarm_chunk, current_volume);
    }
}

bool is_muted(void) {
    return muted;
}

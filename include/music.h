#ifndef MUSIC_H
#define MUSIC_H

#include <stdbool.h>
#include "settings.h"

// Initialize SDL_mixer and load audio assets.
// Returns 1 on success, 0 on failure.
int init_audio(const Settings *settings);

// Clean up all audio resources.
void cleanup_audio(void);

// Start playing a random lo-fi track on loop.
void play_lofi(void);

// Stop the currently playing lo-fi track.
void stop_lofi(void);

// Play the alarm sound once.
int play_alarm(void);
int get_alarm_channel(void);

// Set playback volume (0–128).
void set_volume(int level);

// Adjust the volume by `delta` (positive or negative).
void adjust_volume(int delta);

// Toggle mute on/off (affects both lo-fi and alarm).
void toggle_mute(void);

// Return current volume 0-100 (0 if mute)
int get_volume_percent(void);

// Check mute state.
bool is_muted(void);

// Check alarm playing state.
bool is_alarm_playing(void);

// Check lofi playing state.
bool is_lofi_playing(void);

/// Returns basename of the currently loaded lo-fi track
const char* get_current_lofi_name(void);

#endif

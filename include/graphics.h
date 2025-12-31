#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL.h>
#include <time.h>      // for time_t, struct tm
#include "settings.h"
#include "pomodoro.h"  // for TimerType

// for different behaviours upon user inputs to the start screen
typedef enum {
    START_SCREEN_QUIT = 0,
    START_SCREEN_TIME_ENTERED = 1,
    START_SCREEN_SETTINGS = 2
} StartScreenResult;

// Initialize SDL window and renderer
int init_graphics(const Settings *settings);

// Show a centered, wrapped message and block until user presses Enter/Esc or closes the window.
void show_fullscreen_message(const char *text);

void graphics_begin_frame();

void graphics_end_frame();

// scrolling ticker state (shared)
extern int track_scroll;
extern int track_text_width;

// Get start time from user input
StartScreenResult get_start_time_from_user(int *hour, int *minute);

// Draw the shrinking/refilling pie chart based on fraction (0.0 to 1.0)
void draw_pie(double fraction, TimerType type);

// Render a digital countdown timer like "25:00"
void render_countdown(int seconds_left, TimerType type);

// draw the right-hand panel
void draw_panel(time_t now, int current_session, const time_t *session_starts, const time_t *session_ends, int num_sessions);

// Clean up SDL and TTF resources
void cleanup_graphics(void);

// Access the SDL renderer (used internally or for advanced drawing)
SDL_Renderer* get_renderer(void);

#endif

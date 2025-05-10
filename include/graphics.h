#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <SDL.h>
#include <time.h>      // for time_t, struct tm
#include "settings.h"
#include "pomodoro.h"  // for TimerType

// Initialize SDL window and renderer
int init_graphics(const Settings *settings);

void graphics_begin_frame();

void graphics_end_frame();

// scrolling ticker state (shared)
extern int track_scroll;
extern int track_text_width;

// Get start time from user input
int get_start_time_from_user(int *hour, int *minute);

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

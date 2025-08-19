#ifndef POMODORO_H
#define POMODORO_H

#include <time.h>       // for time_t
#include "settings.h"

// Timer type: WORK or BREAK
typedef enum {
    WORK,
    BREAK
} TimerType;

// Returns current time in seconds (fractional)
double get_time_now(void);

// Run a timer until end_time, updating GUI each frame
// duration_seconds is the total length of the session in seconds
int run_timer(double      end_time,
              TimerType   type,
              int         duration_seconds,
              int         current_session,
              const time_t *session_starts,
              const time_t *session_ends,
              int         num_sessions);

// Run the full Pomodoro sequence based on settings
int run_pomodoro(const Settings *settings, time_t start_time);

#endif

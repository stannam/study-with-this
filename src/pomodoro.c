#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>

#include "pomodoro.h"
#include "settings.h"
#include "graphics.h"
#include "music.h"

// Get current time in seconds with sub-second precision
double get_time_now(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

// Run a timer until end_time, updating the graphics every 500ms
// duration_seconds is the total length of the session in seconds
void run_timer(double       end_time,
               TimerType    type,
               int          duration_seconds,
               int          current_session,
               const time_t *session_starts,
               const time_t *session_ends,
               int          num_sessions)
{
    bool music_started = false;
    SDL_Event event;

    while (1) {
        // audio control
        if (type == WORK){
            // during work. play lofi except when alarm rings
            if (is_alarm_playing()) {
                stop_lofi();
                music_started = false;
            } else if (!music_started) {
                play_lofi();
                music_started = true;
            }
            track_scroll += 10; // move these pixels per tick. the higher the faster
        } else {
            // during break.
            if (music_started) {
                stop_lofi();
            }
            music_started = false;
        }

        // Handle window events to keep the window responsive
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_QUIT:
                cleanup_graphics();
                exit(0);
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == 'm' || event.key.keysym.sym == 'M') toggle_mute();
                if (event.key.keysym.sym == '[') adjust_volume(-8);
                if (event.key.keysym.sym == ']') adjust_volume(+8);
            } 
        }

        double now = get_time_now();
        double remaining = end_time - now;
        if (remaining <= 0.0) {
            remaining = 0.0;
        }

        // Compute fraction of time left
        double fraction = remaining / (double)duration_seconds;
        if (fraction < 0.0) fraction = 0.0;
        if (fraction > 1.0) fraction = 1.0;

        int seconds_left = (int)remaining;

        // Draw frame
        graphics_begin_frame();
        draw_pie(fraction, type);
        render_countdown(seconds_left, type);
        draw_panel(
            (time_t)time(NULL),      // current time
            current_session,         // from run_pomodoro()
            session_starts,          // array
            session_ends,            // array
            num_sessions
        );
        graphics_end_frame();

        // Break loop when timer expires
        if (remaining <= 0.0) {
            break;
        }

        SDL_Delay(500);
    }
}

// Run full Pomodoro sequence based on settings
void run_pomodoro(const Settings *settings, time_t base) {
    int n = settings->num_sessions;
    time_t session_starts[n];
    time_t session_ends[n];

    // compute all session times in advance
    session_starts[0] = base;
    session_ends  [0] = base + settings->work_time * 60;

    for (int session = 1; session < n; session++) {
        session_starts[session] = session_ends[session-1] + settings->break_time * 60;
        session_ends  [session] = session_starts[session] + settings->work_time * 60;
    }

    // actually running the sessions
    for (int session = 0; session < n; session++) {
        // Work session
        run_timer(
          (double)session_ends[session],  // end_time
          WORK,                           // type
          settings->work_time * 60,       // duration
          session,                        // current_session
          session_starts,                 // array of starts
          session_ends,                   // array of ends
          n                               // total sessions
        );

        // stop the lofi and trigger alarm
        stop_lofi();
        play_alarm();

        // Break (except after the last session)
        if (session < n-1) {
            run_timer(
              (double)(session_ends[session] + settings->break_time*60),
              BREAK,
              settings->break_time * 60,
              session,
              session_starts,
              session_ends,
              n
            );
            play_alarm();
        }
    }

    // Optionally notify completion
    // display_completion_message();
}

#include <stdio.h>
#include "settings.h"
#include "pomodoro.h"
#include "graphics.h"
#include "music.h"

/// Block until the user presses Enter (or closes the window).
static void wait_for_enter(void) {
    SDL_Event e;

    while (1) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                cleanup_graphics();
                cleanup_audio();
                exit(0);
            }
            if (e.type == SDL_KEYDOWN){
                if (e.key.keysym.sym == SDLK_RETURN ||
                    e.key.keysym.sym == SDLK_KP_ENTER) {
                    return;
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    cleanup_graphics();
                    cleanup_audio();
                    exit(0);
                }
            } 
        }
        SDL_Delay(500);
    }
}

// parse user's key input as time (hh:mm)
time_t decide_start_time(void){
    int hh = 0, mm = 0;
    if (!get_start_time_from_user(&hh, &mm)) {
        cleanup_graphics();
        return 1;
    }
    time_t now = time(NULL);
    struct tm tm_start = *localtime(&now);
    tm_start.tm_hour = hh;
    tm_start.tm_min = mm;
    tm_start.tm_sec = 0;
    return mktime(&tm_start);
}

int main(void) {
    Settings s = load_settings("settings.json");

    if (!init_graphics(&s)) {
        fprintf(stderr, "Failed to initialize graphics\n");
        return 1;
    }

    if (!init_audio(&s)) {
        fprintf(stderr,"Audio init failed\n");
        return 1;
    }

    while (1) {
        time_t base     = decide_start_time();
        time_t now      = time(NULL);

        // If a future time is given, pre-wait
        double seconds_until = difftime(base, now);
        if (seconds_until > 0) {
            run_timer(base, BREAK, (int)seconds_until,
                      -1, NULL, NULL, 0);
            play_alarm();
        }

        // start pomodoro
        run_pomodoro(&s, base);

        // end of final work session, wait for the user to hit Enter
        wait_for_enter();
    }

    // Clean up and exit
    cleanup_graphics();
    return 0;
}

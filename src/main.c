#include <stdio.h>
#include "settings.h"
#include "pomodoro.h"
#include "graphics.h"
#include "music.h"

// called whenever the program terminates
static void shutdown(int lid_con) {
    if (lid_con){
        system("sudo pmset -a disablesleep 0");
    }
    cleanup_graphics();
    cleanup_audio();
    exit(0);
}

// parse user's key input as time (hh:mm)
static time_t process_start_screen_input(StartScreenResult *result){
    int hh = 0, mm = 0;
    *result = get_start_time_from_user(&hh, &mm);

    switch (*result) {
        // For QUIT or SETTINGS, no need for a base time.
        case START_SCREEN_QUIT:
        case START_SCREEN_SETTINGS:
            return (time_t)0;

            // The user entered a time (hh:mm)
        case START_SCREEN_TIME_ENTERED: {
            time_t now = time(NULL);
            struct tm tm_start = *localtime(&now);
            tm_start.tm_hour = hh;
            tm_start.tm_min = mm;
            tm_start.tm_sec = 0;
            return mktime(&tm_start);
        }

        // fallback
        default:
            *result = START_SCREEN_QUIT;
            return (time_t)0;
    }
}

// after all sessions end, wait for the user presses Enter or ESC (or closes the window).
static void wait_for_enter(int lid_con) {
    SDL_Event e;

    while (1) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                shutdown(lid_con);
            }
            if (e.type == SDL_KEYDOWN){
                if (e.key.keysym.sym == SDLK_RETURN ||
                    e.key.keysym.sym == SDLK_KP_ENTER) {
                    return;
                } else if (e.key.keysym.sym == SDLK_ESCAPE) {
                    shutdown(lid_con);
                }
            } 
        }
        SDL_Delay(500);
    }
}

// helper function to inquire power setting.
static int inquire_power_state(void){
    char current_status[128];

    // Get the current sleep status
    FILE* pipe = popen("pmset -g | grep Sleep | grep -v \"Sleep On Power Button\"", "r");
    if (!pipe) {
        exit(EXIT_FAILURE);
    }
    fgets(current_status, sizeof(current_status), pipe);
    pclose(pipe);

    // return status  (0='no sleep', 1='sleep')
    return strstr(current_status, "0") != NULL ? 1 : 0;
}

int main(void) {
    // load settings and initialize
    Settings s = load_settings();

    // decide whether to touch lid
    int lid_con = 0;
    if (s.lid_con){
        lid_con = inquire_power_state();
    }

    if (init_graphics(&s)) {   // if init_graphics returns an error.
        fprintf(stderr, "Failed to initialize graphics\n");
        return 1;
    }

    if (!init_audio(&s)) {     // since graphics initialized, audio error can be presented on screen
        const char *audio_err = get_last_audio_error();
        if (!audio_err) {
            audio_err = "Audio initialization failed.";
        }

        // Show the error in the window before the initial UI.
        show_fullscreen_message(audio_err);
        shutdown(lid_con);  // User pressed Enter to exit. Shutdown procedure
    }

    if (lid_con){
        system("sudo pmset -a disablesleep 1");
    }

    // get string input from the user and start pomodoro
    while (1) {
        StartScreenResult res;
        time_t base     = process_start_screen_input(&res);
        time_t now      = time(NULL);

        // If the user exit the program and did not enter a time
        if (res == START_SCREEN_QUIT || base == 1) {
            shutdown(lid_con);
        }

        // If the user wants to change settings.json
        if (res == START_SCREEN_SETTINGS) {
            // settings logic: open settings.json in the file manager
            open_settings_in_file_manager();

            show_fullscreen_message("Restart the app to apply your new settings.");
            shutdown(lid_con);  // User pressed Enter to exit. Shutdown procedure
        }

        // default case: a time is given.
        double seconds_until = difftime(base, now);
        if (seconds_until > 0) {                                // a future time is given. start with break time.
            run_timer(base, BREAK, (int)seconds_until,
                      -1, NULL, NULL, 0);
            play_alarm();
        }

        // start pomodoro
        if(run_pomodoro(&s, base) == 1){
            // Terminated prematurely
            shutdown(lid_con);
        }

        // end of final work session, wait for the user to hit Enter
        wait_for_enter(lid_con);
    }
    shutdown(lid_con);
}

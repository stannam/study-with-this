#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <string.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include "graphics.h"
#include "roboto_font_data.h"
#include "settings.h"
#include "music.h"

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static TTF_Font *font_timer = NULL;
static TTF_Font *font_label = NULL;
static TTF_Font *font_clock = NULL;
static TTF_Font *font_time_table = NULL;
static TTF_Font *font_status = NULL;

static int layout_pad = 0;
static int layout_pie_size = 0;
static int layout_winW = 0;
static int layout_winH = 0;
static int l_panelW = 0;
static int r_panelW = 0;
static int status_size = 0;

int track_scroll = 0;
int track_text_width = 0;

// loading embedded font roboto_font_data.h
static TTF_Font *load_embedded_font(int pt_size) {
    SDL_RWops *rw = SDL_RWFromMem(Roboto_Regular_ttf, Roboto_Regular_ttf_len);
    if (!rw) {
        fprintf(stderr, "SDL_RWFromMem Error: %s\n", SDL_GetError());
        return NULL;
    }

    TTF_Font *font = TTF_OpenFontRW(rw, 1, pt_size);
    if (!font) {
        fprintf(stderr, "TTF_OpenFontRW Error: %s\n", TTF_GetError());
        return NULL;
    }

    return font;
}

int init_graphics(const Settings *settings) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return 0;
    }
    if (TTF_Init() == -1) {
        fprintf(stderr, "TTF_Init Error: %s\n", TTF_GetError());
        return 0;
    }

    window = SDL_CreateWindow(
        "Study With This",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        settings->width,
        settings->height,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        fprintf(stderr, "Window creation error: %s\n", SDL_GetError());
        return 0;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        fprintf(stderr, "Renderer creation error: %s\n", SDL_GetError());
        return 0;
    }

    // Determine dynamic sizes of padding, pie and font sizes, panel size, based on window height
    SDL_GetWindowSize(window, &layout_winW, &layout_winH);
    layout_pad      = (int)(layout_winH * 0.10f);
    layout_pie_size = (int)(layout_winH * 0.50f);
    l_panelW        = (int)(layout_winW * 0.70f);
    r_panelW        = layout_winW - layout_pad;

    int areaH = (int)(layout_winW * 0.20f);
    int timer_size = areaH / 2;
    if (timer_size < 1) timer_size = 1;
    int label_size = (int)(timer_size * 0.5f);
    if (label_size < 1) label_size = 1;
    int clock_size = (int)(r_panelW * 0.05f);
    if (clock_size < 1) clock_size = 1;
    int time_table_size = (int)(r_panelW * 0.03f);
    if (time_table_size < 1) time_table_size = 1;
    int status_size = (int)(time_table_size * 0.8f);
    if (status_size < 1) status_size = 1;

    // load the font of different sizes
    font_timer      = load_embedded_font(timer_size);
    font_label      = load_embedded_font(label_size);
    font_clock      = load_embedded_font(clock_size);
    font_time_table = load_embedded_font(time_table_size);
    font_status     = load_embedded_font(status_size);

    if (!font_timer || !font_label || !font_clock || !font_time_table || !font_status) {
        fprintf(stderr, "Font Error: Failed loading one or more fonts.\n");
        return 0;
    }
}

int get_start_time_from_user(int *hour, int *minute) {
        char buffer[6] = "";
    int running = 1;
    SDL_Event e;

    SDL_StartTextInput();

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT)
                return 0;
            else if (e.type == SDL_TEXTINPUT) {
                if (strlen(buffer) < 5) {
                    strncat(buffer, e.text.text, 1);
                }
            } else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(buffer) > 0) {
                    buffer[strlen(buffer)-1] = '\0';
                } else if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    if (sscanf(buffer, "%2d:%2d", hour, minute) == 2) {
                        SDL_StopTextInput();
                        return 1;
                    }
                }
            }
        }

        graphics_begin_frame();

        // Draw prompt
        // First, get current time to show with the prompt
        time_t now = time(NULL);
        struct tm *local = localtime(&now);
        char now_buf[32];
        snprintf(now_buf, sizeof(now_buf),
            "(Current time %02d:%02d)", local->tm_hour, local->tm_min);


        SDL_Color color = {255, 255, 255};
        SDL_Surface *sf1 = TTF_RenderText_Blended(font_label,
            "Enter the start time (HH:MM)", color);
        SDL_Texture *tx1 = SDL_CreateTextureFromSurface(renderer, sf1);
        int w1,h1; SDL_QueryTexture(tx1, NULL,NULL,&w1,&h1);
        SDL_Rect dst1 = { (layout_winW-w1)/2, layout_winH/3, w1, h1 };
        SDL_RenderCopy(renderer, tx1, NULL, &dst1);
        SDL_FreeSurface(sf1);
        SDL_DestroyTexture(tx1);

        // Render current time
        SDL_Surface *sf_now = TTF_RenderText_Blended(font_time_table, now_buf, color);
        SDL_Texture *tx_now = SDL_CreateTextureFromSurface(renderer, sf_now);
        int wn, hn; SDL_QueryTexture(tx_now, NULL, NULL, &wn, &hn);
        SDL_Rect dst_now = {
            (layout_winW - wn)/2,
            dst1.y + h1 + 5,  // 10px below the prompt
            wn, hn
        };
        SDL_RenderCopy(renderer, tx_now, NULL, &dst_now);
        SDL_FreeSurface(sf_now);
        SDL_DestroyTexture(tx_now);

        // Draw current input buffer
        SDL_Surface *sf2 = TTF_RenderText_Blended(font_label, buffer, color);
        SDL_Texture *tx2 = SDL_CreateTextureFromSurface(renderer, sf2);
        int w2,h2; SDL_QueryTexture(tx2, NULL,NULL,&w2,&h2);
        SDL_Rect dst2 = { (layout_winW-w2)/2, layout_winH/2, w2, h2 };
        SDL_RenderCopy(renderer, tx2, NULL, &dst2);
        SDL_FreeSurface(sf2);
        SDL_DestroyTexture(tx2);

        graphics_end_frame();
        SDL_Delay(50);  // 20 fps
    }
    SDL_StopTextInput();
    return 0;
}

void graphics_begin_frame(void) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void graphics_end_frame(void) {
    SDL_RenderPresent(renderer);
}

void draw_pie(double fraction, TimerType type) {
    int radius = layout_pie_size / 2;
    int cx = l_panelW / 2;
    int cy = layout_pad + radius;
    int segments = 360;

    if (type == WORK) {
        // Full red circle
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < segments; i++) {
            double angle = 2.0 * M_PI * i / segments;
            int x = cx + (int)(sin(angle) * radius);
            int y = cy - (int)(cos(angle) * radius);
            SDL_RenderDrawLine(renderer, cx, cy, x, y);
        }
        // Black wedge clockwise
        int black_segs = (int)((1.0 - fraction) * segments);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        for (int i = 0; i < black_segs; i++) {
            double angle = 2.0 * M_PI * i / segments;
            int x = cx + (int)(sin(angle) * radius);
            int y = cy - (int)(cos(angle) * radius);
            SDL_RenderDrawLine(renderer, cx, cy, x, y);
        }
    } else {
        // Break time: black background then red refill CCW
        int red_segs = (int)((1.0 - fraction) * segments);
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        for (int i = 0; i < red_segs; i++) {
            double angle = -2.0 * M_PI * i / segments;
            int x = cx + (int)(sin(angle) * radius);
            int y = cy - (int)(cos(angle) * radius);
            SDL_RenderDrawLine(renderer, cx, cy, x, y);
        }
    }
}

void render_countdown(int seconds_left, TimerType type) {
    int cx = l_panelW / 2;
    int labelY = layout_pad + layout_pie_size + layout_pad;
    int areaH = (int)(layout_winH * 0.20f);

    SDL_Color color = (type == WORK)
        ? (SDL_Color){255, 255, 255, 255}
        : (SDL_Color){255, 255, 0, 255};

    // Label using font_label
    const char *label = (type == WORK) ? "STUDY TIME" : "BREAK TIME";
    SDL_Surface *surfLabel = TTF_RenderText_Blended(font_label, label, color);
    SDL_Texture *texLabel = SDL_CreateTextureFromSurface(renderer, surfLabel);
    int wL, hL;
    SDL_QueryTexture(texLabel, NULL, NULL, &wL, &hL);
    SDL_Rect dstL = { cx - wL / 2, labelY + (areaH - hL) / 4, wL, hL };
    SDL_RenderCopy(renderer, texLabel, NULL, &dstL);
    SDL_FreeSurface(surfLabel);
    SDL_DestroyTexture(texLabel);

    // Countdown using font_timer
    int mins = seconds_left / 60;
    int secs = seconds_left % 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", mins, secs);
    SDL_Surface *surfCnt = TTF_RenderText_Blended(font_timer, buf, color);
    SDL_Texture *texCnt = SDL_CreateTextureFromSurface(renderer, surfCnt);
    int wC, hC;
    SDL_QueryTexture(texCnt, NULL, NULL, &wC, &hC);
    SDL_Rect dstC = { cx - wC / 2, labelY + (areaH - hL) / 4 + hL + (areaH - hL - hC) / 4, wC, hC };
    SDL_RenderCopy(renderer, texCnt, NULL, &dstC);
    SDL_FreeSurface(surfCnt);
    SDL_DestroyTexture(texCnt);
}

void draw_panel(time_t now,
                int current_session,
                const time_t *session_starts,
                const time_t *session_ends,
                int num_sessions)
{
    // 1) Panel metrics
    int pad        = layout_pad;                    // cached in init
    int winW       = layout_winW,
        winH       = layout_winH;
    int panelW     = (int)(winW * 0.25f);
    int panelX     = winW - panelW - pad;
    int panelY     = pad;
    int panelH     = winH - 2 * pad;

    // Colors
    SDL_Color white      = {255,255,255,255};
    SDL_Color black      = {  0,  0,  0,255};
    SDL_Color highlightBg= white;
    SDL_Color highlightFg= black;

    // For current music ticker boundary
    SDL_Rect clip = { panelX, panelY, panelW, panelH };
    SDL_RenderSetClipRect(renderer, &clip);

    int wl, hl, wc, hc;  // width and height of 'local time' label to be referred elsewhere

    // 2) Draw "Local time" label
    {
        SDL_Surface *sfl = TTF_RenderText_Blended(font_clock,
                                                 "Local time",
                                                 white);
        SDL_Texture *txl = SDL_CreateTextureFromSurface(renderer, sfl);
        SDL_QueryTexture(txl, NULL, NULL, &wl, &hl);

        // center that label in the panel top
        SDL_Rect dstl = {
            panelX + (panelW - wl)/2,
            pad,
            wl, hl
        };
        SDL_RenderCopy(renderer, txl, NULL, &dstl);

        SDL_FreeSurface(sfl);
        SDL_DestroyTexture(txl);
    }

    // And under "Local Time", draw a digital clock
    {
        struct tm *lt = localtime(&now);
        char timestr[16];
        snprintf(timestr,sizeof(timestr), "%02d:%02d:%02d",
                 lt->tm_hour, lt->tm_min, lt->tm_sec);

        SDL_Surface *sf = TTF_RenderText_Blended(font_clock, timestr, white);
        SDL_Texture *tx = SDL_CreateTextureFromSurface(renderer, sf);
        SDL_QueryTexture(tx, NULL,NULL,&wc,&hc);

        SDL_Rect dst = {
            panelX + (panelW - wc)/2,
            hl + 5 + pad,
            wc, hc
        };
        SDL_RenderCopy(renderer, tx, NULL, &dst);

        SDL_FreeSurface(sf);
        SDL_DestroyTexture(tx);
    }

    // 3) Draw the work-session timetable
    int y = pad + hc + hl + pad;
    for (int i = 0; i < num_sessions; i++) {
        // Format: "i   HH:MM - HH:MM"
        char buf[64];
        struct tm st_tm = *localtime(&session_starts[i]);
        struct tm et_tm = *localtime(&session_ends[i]);

        snprintf(buf, sizeof(buf),
                 "%d   %02d:%02d - %02d:%02d",
                 i+1,
                 st_tm.tm_hour, st_tm.tm_min,
                 et_tm.tm_hour, et_tm.tm_min);

        // Render text surface
        SDL_Surface *sf = TTF_RenderText_Blended(font_time_table,
                                buf,
                                (i == current_session) ? highlightFg : white);
        SDL_Texture *tx = SDL_CreateTextureFromSurface(renderer, sf);
        int w,h; SDL_QueryTexture(tx, NULL,NULL,&w,&h);

        // Optionally draw highlight background
        if (i == current_session) {
            SDL_SetRenderDrawColor(renderer,
                                   highlightBg.r,
                                   highlightBg.g,
                                   highlightBg.b,
                                   highlightBg.a);
            SDL_Rect bg = { panelX, y, panelW, h };
            SDL_RenderFillRect(renderer, &bg);
        }

        // Draw text
        SDL_Rect dst = {
            panelX + (panelW - w)/2,
            y,
            w, h
        };
        SDL_RenderCopy(renderer, tx, NULL, &dst);

        SDL_FreeSurface(sf);
        SDL_DestroyTexture(tx);

        y += h + 5;  // 5px line spacing
        if (y > panelY + panelH - h) break;
    }

    // 4) Status block on the right hand panel
    SDL_Color status_color = {200,200,200,255};
    const int spacing = 5;

    // Calculate starting Y so bottom padding == layout_pad
    // (countdown uses that same layout_pad at bottom)
    int sy = panelY + panelH - layout_pad - (status_size * 3 + spacing * 2);


    // Left margin inside the panel
    const int sx = panelX + spacing;

    // 1) Volume
    char volbuf[32];
    snprintf(volbuf, sizeof(volbuf),
             "Vol: %d%%",
             get_volume_percent());
    SDL_Surface *sf_vol = TTF_RenderText_Blended(font_status, volbuf, status_color);
    SDL_Texture *tx_vol = SDL_CreateTextureFromSurface(renderer, sf_vol);
    int wv,hv; SDL_QueryTexture(tx_vol, NULL,NULL,&wv,&hv);
    SDL_Rect dst_vol = {sx, sy, wv, hv};
    SDL_RenderCopy(renderer, tx_vol, NULL, &dst_vol);
    SDL_FreeSurface(sf_vol); SDL_DestroyTexture(tx_vol);

    // 2) Keys reminder
    const char *keys = "M mute [ ] vol up/down";
    SDL_Surface *sf_keys = TTF_RenderText_Blended(font_status, keys, status_color);
    SDL_Texture *tx_keys = SDL_CreateTextureFromSurface(renderer, sf_keys);
    int wk,hk; SDL_QueryTexture(tx_keys, NULL,NULL,&wk,&hk);
    SDL_Rect dst_keys = { sx, sy + hv + spacing, wk, hk };
    SDL_RenderCopy(renderer, tx_keys, NULL, &dst_keys);
    SDL_FreeSurface(sf_keys); SDL_DestroyTexture(tx_keys);

    // 3) Current track
    const char *track = get_current_lofi_name();
    SDL_Surface *sf_track = TTF_RenderText_Blended(font_status, track, status_color);
    SDL_Texture *tx_track = SDL_CreateTextureFromSurface(renderer, sf_track);

    int wt,ht;
    SDL_QueryTexture(tx_track, NULL, NULL, &track_text_width, &ht);

    int panelLeft   = panelX;
    int panelRight  = panelX + panelW;
    static bool first_time = true;
    if (first_time) {
        track_scroll = -panelW;  // start fully off-screen to the right
        first_time   = false;
    }
    int drawX = panelRight - track_scroll;
    if (track_scroll > panelW + track_text_width) {
        track_scroll = 0;
        drawX        = panelRight;
    }

    // actually drawing the current track
    SDL_Rect dst_track = { drawX, sy + hv + spacing + hk + spacing, track_text_width, ht };
    SDL_RenderCopy(renderer, tx_track, NULL, &dst_track);
    SDL_RenderSetClipRect(renderer, NULL);

    SDL_FreeSurface(sf_track); SDL_DestroyTexture(tx_track);  // clean up
}

void cleanup_graphics(void) {
    if (font_timer)  TTF_CloseFont(font_timer);
    if (font_label)  TTF_CloseFont(font_label);
    if (font_clock) TTF_CloseFont(font_clock);
    if (font_time_table) TTF_CloseFont(font_time_table);
    if (font_status) TTF_CloseFont(font_status);

    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
}

SDL_Renderer* get_renderer(void) {
    return renderer;
}

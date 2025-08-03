#ifndef SETTINGS_H
#define SETTINGS_H

#define MAX_PATH_LEN 1024

// initialize the base resource directory path
void initialize_resource_directory(void);

// get the settings.json path within the resource directory
void decide_settings_json_path(void);

// create a default settings file
void create_default_settings(void);

typedef struct {
    int work_time;
    int break_time;
    int num_sessions;
    int width;
    int height;
    int lid_con;
    char asset_directory[MAX_PATH_LEN];
    char music_directory[MAX_PATH_LEN];
    char alarm_sound[MAX_PATH_LEN];
} Settings;

Settings load_settings(void);

#endif

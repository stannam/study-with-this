#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"

#define MAX_PATH_LEN 1024

static char resource_directory[MAX_PATH_LEN];  // variable for resource directory path
static char settings_path[MAX_PATH_LEN];       // variable for settings path

// Initialize the base resource directory path
void initialize_resource_directory(void) {
    const char *home = getenv("HOME");  // Get the HOME environment variable
    if (home == NULL) {
        // If we can't get the HOME environment variable, return a default directory
        snprintf(resource_directory, MAX_PATH_LEN, "/usr/Documents/Study-with-me/resource");
    } else {
        snprintf(resource_directory, MAX_PATH_LEN, "%s/Documents/Study-with-me/resource", home);
    }
}

// Function to get the settings.json path within the resource directory
void decide_settings_json_path(void) {
    // Ensure the resource directory is initialized
    if (resource_directory[0] == '\0') {
        initialize_resource_directory();  // Initialize if not already done
    }

    snprintf(settings_path, MAX_PATH_LEN, "%s/settings.json", resource_directory);
}

// Function to create a default settings file
void create_default_settings(void) {
    FILE *file = fopen(settings_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Error creating settings file: %s\n", settings_path);
        exit(1);
    }

    // Write default settings to the file
    fprintf(file, "{\n");
    fprintf(file, "  \"work_time\": 50,\n");
    fprintf(file, "  \"break_time\": 10,\n");
    fprintf(file, "  \"num_sessions\": 5,\n");
    fprintf(file, "  \"width\": 800,\n");
    fprintf(file, "  \"height\": 500,\n");
    fprintf(file, "  \"asset_directory\": \"%s/sound\",\n", resource_directory);
    fprintf(file, "  \"music_directory\": \"lofi\",\n");
    fprintf(file, "  \"alarm_sound\": \"bell1.mp3\"\n");
    fprintf(file, "}\n");

    fclose(file);
    printf("Default settings file created at: %s\n", settings_path);
}

// Settings structure
typedef struct {
    int work_time;
    int break_time;
    int num_sessions;
    int width;
    int height;
    char asset_directory[MAX_PATH_LEN];
    char music_directory[MAX_PATH_LEN];
    char alarm_sound[MAX_PATH_LEN];
} Settings;

// Load the settings from the JSON file
Settings load_settings(void) {
    // if settings path is not specified yet, initialize
    if (settings_path[0] == '\0'){
        decide_settings_json_path();
    }

    Settings settings;
    FILE *file = fopen(settings_path, "r");

    if (file == NULL) {
        printf("Error opening settings file. Creating default settings...\n");
        create_default_settings();
        file = fopen(settings_path, "r");  // Open again after creation
        if (file == NULL) {
            printf("Still can't open settings file."); // if file is still NULL just exist
            exit(1);
        }
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);
    fread(buffer, 1, file_size, file);
    fclose(file);

    buffer[file_size] = '\0';  // Null-terminate the buffer

    // Parse the JSON data
    cJSON *json = cJSON_Parse(buffer);
    free(buffer);

    if (json == NULL) {
        printf("Error parsing settings file.\n");
        exit(1);
    }

    // Extract the values
    cJSON *work_time = cJSON_GetObjectItem(json, "work_time");
    cJSON *break_time = cJSON_GetObjectItem(json, "break_time");
    cJSON *num_sessions = cJSON_GetObjectItem(json, "num_sessions");
    cJSON *width = cJSON_GetObjectItem(json, "width");
    cJSON *height = cJSON_GetObjectItem(json, "height");
    cJSON *asset_directory = cJSON_GetObjectItem(json, "asset_directory");
    cJSON *music_directory = cJSON_GetObjectItem(json, "music_directory");
    cJSON *alarm_sound = cJSON_GetObjectItem(json, "alarm_sound");

    settings.work_time = work_time ? work_time->valueint : 50;  // Default to 50 if not found
    settings.break_time = break_time ? break_time->valueint : 10;  // Default to 10 if not found
    settings.num_sessions = num_sessions ? num_sessions->valueint : 5;  // Default to 5 if not found
    settings.width = width ? width->valueint : 800;  // Default to 800 if not found
    settings.height = height ? height->valueint : 500;  // Default to 500 if not found
    strncpy(settings.asset_directory, asset_directory ? asset_directory->valuestring : "", MAX_PATH_LEN);

    // Safely construct full path to music directory
    if (asset_directory && music_directory) {
        snprintf(settings.music_directory, MAX_PATH_LEN, "%s/%s/", asset_directory->valuestring, music_directory->valuestring);
    } else {
        strncpy(settings.music_directory, "", MAX_PATH_LEN);
    }

    // Safely construct full path to alarm sound file
    if (asset_directory && alarm_sound) {
        snprintf(settings.alarm_sound, MAX_PATH_LEN, "%s/%s", asset_directory->valuestring, alarm_sound->valuestring);
    } else {
        strncpy(settings.alarm_sound, "", MAX_PATH_LEN);
    }

    cJSON_Delete(json);
    return settings;
}

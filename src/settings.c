#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL_misc.h>

#include "platform.h"  // for platform-dependent file io
#include "cJSON.h"
#include "settings.h"
#include "bell1_mp3_data.h"


#define MAX_PATH_LEN 1024

static char resource_directory[MAX_PATH_LEN];  // variable for resource directory path
static char settings_path[MAX_PATH_LEN];       // variable for settings path

// ensure bell.mp3 exists
static void ensure_bell_sound_exists(const Settings *settings) {
    if (settings->asset_directory[0] == '\0' ||
        settings->alarm_sound[0] == '\0') {
        // Nothing to do if paths in settings.json are empty
        return;
        }

    char dir_path[MAX_PATH_LEN];

    // 1) Make sure asset_directory exists (mkdir -p)
    snprintf(dir_path, sizeof(dir_path), "%s", settings->asset_directory);

    if (platform_mkdir_p(dir_path) != 0) {
        fprintf(stderr, "Failed to create asset directory: %s\n", dir_path);
        return;
    }

    // 2) path = full path to the bell
    const char *path = settings->alarm_sound;

    FILE *f = fopen(path, "rb");
    if (f) {
        fclose(f);
        return;
    }

    // 4) Create and write the embedded MP3
    f = fopen(path, "wb");
    if (!f) {
        fprintf(stderr, "Error creating alarm sound: %s\n", path);
        return;
    }

    size_t written = fwrite(resources_bell1_mp3, 1,
                            (size_t)resources_bell1_mp3_len, f);

    if (written != (size_t)resources_bell1_mp3_len) {
        fprintf(stderr,
                "Short write when creating alarm sound (%zu/%u bytes): %s\n",
                written, resources_bell1_mp3_len, path);
    }

    fclose(f);
}

// get settings.json path to be used elsewhere.
const char *get_settings_path(void) {
    if (resource_directory[0] == '\0') {
        decide_settings_json_path();
    }
    return settings_path;
}

// open settings.json using external file manager default to each OS
void open_settings_in_file_manager(void) {
    const char *path = get_settings_path();
    if (!path || !path[0]) return;

    char url[1024];

    // build file_url
    strcpy(url, "file://");

    const char *src = path;
    char *dst = url + strlen(url);

    while (*src && (dst - url) < (int)(sizeof(url) - 1)) {
        // Convert native separators (PLATFORM_PATH_SEP) to URL-style '/'
        if (*src == PLATFORM_PATH_SEP) {
            *dst++ = '/';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';

    // and open the file_url
    SDL_OpenURL(url);
}

// Initialize the base resource directory path
void initialize_resource_directory(void) {
    char docs[MAX_PATH_LEN];
    char lofi_directory[MAX_PATH_LEN];

    // to create resource directory in the correct place, we need to locate Documents first
    if (platform_get_documents_dir(docs, sizeof(docs)) != 0) {
        fprintf(stderr, "Failed to locate Documents directory\n");
        exit(1);
    }

    // then set resource directory path
    snprintf(resource_directory, MAX_PATH_LEN,
             "%s%cStudy-with-me%cresource", docs, PLATFORM_PATH_SEP, PLATFORM_PATH_SEP);

    // ...and lofi directory as a subdirectory of resource
    snprintf(lofi_directory, MAX_PATH_LEN,
         "%s%csound%clofi", resource_directory, PLATFORM_PATH_SEP, PLATFORM_PATH_SEP);

    if (platform_mkdir_p(lofi_directory) != 0) {
        fprintf(stderr, "Failed to create resource or lofi directory: %s\n",
                lofi_directory);
        exit(1);
    }
}

// Function to decide the settings.json path within the resource directory
void decide_settings_json_path(void) {
    // Ensure the resource directory is initialized
    if (resource_directory[0] == '\0') {
        initialize_resource_directory();  // Initialize if not already done
    }

    snprintf(settings_path, MAX_PATH_LEN, "%s%csettings.json", resource_directory, PLATFORM_PATH_SEP);
}

// Function to create a default settings file
void create_default_settings(void) {
    // Mostly redundant, but ensure the paths exist once again.
    if (settings_path[0] == '\0') {
        decide_settings_json_path(); // this also ensures resource_directory is initialized
    }

    FILE *file = fopen(settings_path, "w");
    if (file == NULL) {
        fprintf(stderr, "Error creating settings file: %s\n", settings_path);
        exit(1);
    }

    // (Only relevant for windows)
    // Before writing default settings, the back slashes in path are incompatible with json. so convert \ to /
    char asset_directory_json[MAX_PATH_LEN];
    snprintf(asset_directory_json, sizeof(asset_directory_json), "%s%csound", resource_directory, PLATFORM_PATH_SEP);
    for (char *p = asset_directory_json; *p != '\0'; p++){
        if (*p == '\\') *p = '/';
    }

    // Write default settings to the file
    fprintf(file, "{\n");
    fprintf(file, "  \"work_time\": 50,\n");
    fprintf(file, "  \"break_time\": 10,\n");
    fprintf(file, "  \"num_sessions\": 5,\n");
    fprintf(file, "  \"width\": 800,\n");
    fprintf(file, "  \"height\": 500,\n");
    fprintf(file, "  \"asset_directory\": \"%s\",\n", asset_directory_json);
    fprintf(file, "  \"music_directory\": \"lofi\",\n");
    fprintf(file, "  \"alarm_sound\": \"bell1.mp3\",\n");
    fprintf(file, "  \"lid_con\": 0\n");
    fprintf(file, "}\n");

    fclose(file);
    printf("Default settings file created at: %s\n", settings_path);
}

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
    cJSON *lid_con = cJSON_GetObjectItem(json, "lid_con");

    settings.work_time = work_time ? work_time->valueint : 50;  // Default to 50 if not found
    settings.break_time = break_time ? break_time->valueint : 10;  // Default to 10 if not found
    settings.num_sessions = num_sessions ? num_sessions->valueint : 5;  // Default to 5 if not found
    settings.width = width ? width->valueint : 800;  // Default to 800 if not found
    settings.height = height ? height->valueint : 500;  // Default to 500 if not found
    settings.lid_con = lid_con ? lid_con->valueint : 0;  // Default to 0 if not found
    if (asset_directory && cJSON_IsString(asset_directory)) {
        strncpy(settings.asset_directory,
                asset_directory->valuestring,
                MAX_PATH_LEN - 1);
        settings.asset_directory[MAX_PATH_LEN - 1] = '\0';
    } else {
        settings.asset_directory[0] = '\0';
    }

    // Safely construct full path to music directory
    if (asset_directory && music_directory) {
        snprintf(settings.music_directory, MAX_PATH_LEN, "%s%c%s%c",
            asset_directory->valuestring, PLATFORM_PATH_SEP,
            music_directory->valuestring, PLATFORM_PATH_SEP
        );

    } else {
        strncpy(settings.music_directory, "", MAX_PATH_LEN);
    }

    // Safely construct full path to alarm sound file
    if (asset_directory && alarm_sound) {
        snprintf(settings.alarm_sound, MAX_PATH_LEN, "%s%c%s",
            asset_directory->valuestring, PLATFORM_PATH_SEP,
            alarm_sound->valuestring
        );
    } else {
        strncpy(settings.alarm_sound, "", MAX_PATH_LEN);
    }

    // ensure a bell sound exists. if not, build the default bell1.mp3
    ensure_bell_sound_exists(&settings);

    cJSON_Delete(json);
    return settings;
}

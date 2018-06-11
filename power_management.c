#include <unistd.h>
#include <string.h>
#include <stdio.h>

// Max number of chars for the backlight path, including the ./brightness
// and ./max_brightness paths
#define MAX_CHAR_BACKLIGHT_PATH 255

// Maximum number of chars expected to be read as max_brightness
#define MAX_CHAR_BRIGHTNESS_READ 20

#define max(x,y) ((x)>(y)? (x):(y))

// List of paths to check for brightness controls.
// This implies that both ./brightness and ./max_brightness exist in each
// specified path.
const char* screen_paths[] =
    {"/sys/devices/platform/panel/backlight/panel/"};

const char * str_brightness = "brightness";
const char * str_max_brightness = "max_brightness";
char buffer[MAX_CHAR_BACKLIGHT_PATH];
char buffer_read[MAX_CHAR_BRIGHTNESS_READ];

const char * get_backlight_path()
{
    for(int i = 0; i < sizeof(screen_paths); i++)
    {
        const char * current_path = screen_paths[i];
        if(*current_path == '\0')
            continue;

        int current_length = strlen(current_path);
        if(current_length +
                max(sizeof(str_brightness), sizeof(str_max_brightness))
                > MAX_CHAR_BACKLIGHT_PATH)
        {
            printf("Warning: at %s, %s ignored %s, "
                   "that was too big to fit in the buffer",
                   __FILE__, __LINE__, current_path);
            continue;
        }
        strncpy(buffer, current_path, MAX_CHAR_BACKLIGHT_PATH);

        // Note: buffer overflow prevented by the size check above unless
        // an int overflow occurs
        strncpy(buffer + current_length,
                str_brightness, sizeof(str_brightness));
        if(access(buffer, W_OK ) == -1) // An error occured
            continue;
        strncpy(buffer + current_length,
                str_max_brightness, sizeof(str_max_brightness));
        buffer[MAX_CHAR_BACKLIGHT_PATH - 1] = '\0';
        if(access(buffer, R_OK) == -1)
            continue;

        return current_path; // Found a decent match
    }
    return '\0';
}


void turn_on_screen()
{
    const char * backlight_path = get_backlight_path();
    if(*backlight_path == '\0')
        return;
    strcpy(buffer, backlight_path); // We already checked the size earlier
    int current_length = strlen(backlight_path);
    strcpy(buffer + current_length, str_max_brightness);

    FILE * f_max_b = fopen(buffer, "r");
    int number_read = fread(
            buffer_read, 1, MAX_CHAR_BRIGHTNESS_READ, f_max_b);
    fclose(f_max_b);
    if(number_read <= 0 )
        return; // An error occured: less than 1 byte read

    strcpy(buffer + current_length, str_brightness);
    FILE * f_b = fopen(buffer, "w");
    fprintf(f_b, buffer_read); // Set screen brightness to max brightness
    fclose(f_b);
}

void turn_off_screen()
{
    const char * backlight_path = get_backlight_path();
    if(*backlight_path == '\0')
        return;

    int current_length = strlen(backlight_path);
    strcpy(buffer + current_length, str_brightness);
    FILE * f_b = fopen(buffer, "w");
    fprintf(f_b, "0"); // Print 0 (null-terminated) to the screen brightness
    fclose(f_b);
}

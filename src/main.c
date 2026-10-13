#include <miyusic/player.h>
#include <ncurses.h>
#include <stdio.h>

static char* get_formatted_time(double seconds, char* buffer, size_t buffer_size);

int main(void) {
    Player player = {
        .pid = -1
    };

    int play_result = player_play(
        &player, 
        "https://www.youtube.com/watch?v=Qr0-7Ds79zo"
    );

    if (play_result == PLAYER_ERROR) {
        return 1;
    }

    initscr();
    cbreak();
    noecho();

    printw("Welcome to Miyusic!\n");
    printw("Now playing: Pink Floyd - Time\n");
    printw("Press Q to stop...\n");

    refresh();

    double position = 0.0;
    double duration = 0.0;

    char position_str[16];
    char duration_str[16];

    timeout(200);
    int ch;
    while ((ch = getch()) != 'q' && ch != 'Q') {
        if (ch == ' ') {
            if (player_toggle_pause(&player) == PLAYER_ERROR) {
                mvprintw(6, 0, "Failed to toggle playback!");
                clrtoeol();
                refresh();
            }
        }

        if (
            player_get_position(&player, &position) == PLAYER_ERROR ||
            player_get_duration(&player, &duration) == PLAYER_ERROR
        ) {
            mvprintw(4, 0, "Playback: Loading...");
            clrtoeol();
            refresh();
            continue;
        }

        if (
            get_formatted_time(position, position_str, sizeof(position_str)) == NULL ||
            get_formatted_time(duration, duration_str, sizeof(duration_str)) == NULL
        ) {
            break;
        }

        mvprintw(4, 0, "Playback: %s / %s", position_str, duration_str);
        clrtoeol();
        refresh();
    }

    endwin();

    if (player_stop(&player) == PLAYER_ERROR) {
        fprintf(stderr, "Failed to stop playback.\n");
        return 1;
    }

    return 0;
}

static char* get_formatted_time(double seconds, char* buffer, size_t buffer_size) {
    if (buffer == NULL || buffer_size == 0 || seconds < 0) {
        return NULL;
    }

    int total_seconds = (int)seconds;
    int written = snprintf(buffer, buffer_size, "%02d:%02d", total_seconds / 60, total_seconds % 60);
    if (written < 0 || (size_t)written >= buffer_size) {
        return NULL;
    }

    return buffer;
}
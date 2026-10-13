#ifndef PLAYER_H
#define PLAYER_H

#include <sys/types.h>
#include <stdbool.h>

#define PLAYER_SUCCESS  0
#define PLAYER_ERROR   -1

typedef struct {
    pid_t pid;

    bool playback_paused;

    double position_seconds;
    double duration_seconds;
} Player;

int player_play(Player* player, const char* url);
int player_toggle_pause(Player* player);
int player_stop(Player* player);

int player_get_position(Player* player, double* position);
int player_get_duration(Player* player, double* duration);

#endif
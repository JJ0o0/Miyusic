#ifndef PLAYER_H
#define PLAYER_H

#include <sys/types.h>

#define PLAYER_SUCCESS  0
#define PLAYER_ERROR   -1

typedef struct {
    pid_t pid;
} Player;

int player_play(Player* player, const char* url);
int player_stop(Player* player);

#endif
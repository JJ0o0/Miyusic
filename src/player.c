#include <miyusic/player.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>

int player_play(Player* player, const char* url) {
    if (player == NULL || url == NULL || *url == '\0') {
        return PLAYER_ERROR;
    }

    if (player->pid > 0) {
        if (player_stop(player) == PLAYER_ERROR) {
            return PLAYER_ERROR;
        }
    }

    pid_t pid = fork();
    if (pid == PLAYER_ERROR) {
        perror("fork");
        return PLAYER_ERROR;
    }

    if (pid == 0) {
        char* const mpv_command[7] = {
            "mpv",
            "--no-video",
            "--no-terminal",
            "--ytdl-format=bestaudio/best",
            "--ytdl-raw-options=extractor-args=youtube:player_client=android",
            (char*)url,
            NULL
        };

        execvp("mpv", mpv_command);
        perror("execvp");
        _exit(127);
    } else {
        player->pid = pid;
    }

    return PLAYER_SUCCESS;
}

int player_stop(Player* player) {
    if (player == NULL || player->pid <= 0) {
        return PLAYER_ERROR;
    }

    int kill_result = kill(player->pid, SIGTERM);
    if (kill_result == PLAYER_ERROR) {
        perror("kill");
        return PLAYER_ERROR;
    }

    int waitpid_status;
    pid_t waitpid_result;
    do {
        waitpid_result = waitpid(player->pid, &waitpid_status, 0);
    } while (waitpid_result == -1 && errno == EINTR);

    if (waitpid_result == PLAYER_ERROR) {
        perror("waitpid");
        return PLAYER_ERROR;
    }

    player->pid = -1;
    return PLAYER_SUCCESS;
}
#include <miyusic/player.h>
#include <stdio.h>

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

    printf("Pressione Enter para parar a musica...\n");
    getchar();

    if (player_stop(&player) == PLAYER_ERROR) {
        fprintf(stderr, "Failed to stop playback.\n");
        return 1;
    }

    return 0;
}
#include <miyusic/player.h>

#include <cjson/cJSON.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/un.h>

#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <errno.h>

static int player_send_command(const char* command, char* response, size_t response_size);
static int player_request(const char* command, cJSON** result);

static int player_get_property_double(Player* player, const char* property, double* value);

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
        char* const mpv_command[8] = {
            "mpv",
            "--no-video",
            "--no-terminal",
            "--ytdl-format=bestaudio/best",
            "--ytdl-raw-options=extractor-args=youtube:player_client=android",
            "--input-ipc-server=/tmp/miyusic-test.sock",
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

int player_toggle_pause(Player* player) {
    if (player == NULL || player->pid <= 0) {
        return PLAYER_ERROR;
    }

    const char* json_command = "{\"command\":[\"cycle\",\"pause\"]}\n";
    cJSON* response = NULL;
    
    int request_result = player_request(json_command, &response);
    if (request_result == PLAYER_ERROR) {
        return PLAYER_ERROR;
    }

    cJSON_Delete(response);
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

int player_get_position(Player* player, double* position) {
    return player_get_property_double(player, "time-pos", position);
}

int player_get_duration(Player* player, double* duration) {
    return player_get_property_double(player, "duration", duration);
}

static int player_send_command(const char* command, char* response, size_t response_size) {
    if (command == NULL || response == NULL || response_size < 2) {
        return PLAYER_ERROR;
    }

    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd == PLAYER_ERROR) {
        perror("socket");
        return PLAYER_ERROR;
    }

    struct sockaddr_un address = {0};
    address.sun_family = AF_UNIX;

    snprintf(
        address.sun_path, 
        sizeof(address.sun_path), 
        "%s",
        "/tmp/miyusic-test.sock"
    );

    int connect_result = connect(fd, (const struct sockaddr*)&address, sizeof(address));
    if (connect_result == PLAYER_ERROR) {
        perror("connect");
        close(fd);
        return PLAYER_ERROR;
    }

    size_t command_size = strlen(command);
    size_t total_sent = 0;
    while (total_sent < command_size) {
        ssize_t send_result = send(fd, command + total_sent, command_size - total_sent, MSG_NOSIGNAL);
        if (send_result == PLAYER_ERROR) {
            if (errno == EINTR) {
                continue;
            }

            perror("send");
            close(fd);
            return PLAYER_ERROR;
        }

        if (send_result == 0) {
            fprintf(stderr, "send: no byte send\n");
            close(fd);
            return PLAYER_ERROR;
        }

        total_sent += send_result;
    }

    size_t total_received = 0;
    while (total_received < response_size - 1) {
        ssize_t received = recv(fd, response + total_received, response_size - total_received - 1, 0);
        if (received == PLAYER_ERROR) {
            if (errno == EINTR) {
                continue;
            }

            perror("recv");
            close(fd);
            return PLAYER_ERROR;
        }

        if (received == 0) {
            fprintf(stderr, "recv: connection closed\n");
            close(fd);
            return PLAYER_ERROR;
        }

        total_received += received;
        response[total_received] = '\0';

        if (strchr(response, '\n')) {
            break;
        }
    }

    char* newline = strchr(response, '\n');
    if (newline == NULL) {
        fprintf(stderr, "recv: incomplete message\n");
        close(fd);
        return PLAYER_ERROR;
    }

    *newline = '\0';

    close(fd);
    return PLAYER_SUCCESS;
}

static int player_request(const char* command, cJSON** result) {
    if (command == NULL || result == NULL) {
        return PLAYER_ERROR;
    }

    *result = NULL;

    char response[1024];
    int send_result = player_send_command(command, response, sizeof(response));
    if (send_result == PLAYER_ERROR) {
        return PLAYER_ERROR;
    }

    cJSON* json_response = cJSON_Parse(response);
    if (json_response == NULL) {
        fprintf(stderr, "cJSON: parse failed\n");
        return PLAYER_ERROR;
    }

    cJSON* json_error = cJSON_GetObjectItemCaseSensitive(json_response, "error");
    if (!cJSON_IsString(json_error)) {
        cJSON_Delete(json_response);
        return PLAYER_ERROR;
    }

    if (strcmp(json_error->valuestring, "success") != 0) {
        if (strcmp(json_error->valuestring, "property unavailable") != 0) {
            fprintf(stderr, "mpv: %s\n", json_error->valuestring);
        }
        
        cJSON_Delete(json_response);
        return PLAYER_ERROR;
    }

    *result = json_response;
    return PLAYER_SUCCESS;
}

static int player_get_property_double(Player* player, const char* property, double* value) {
    if (player == NULL || player->pid <= 0 || property == NULL || value == NULL) {
        return PLAYER_ERROR;
    }

    char json_command[128];
    int written = snprintf(
        json_command, 
        sizeof(json_command), 
        "{\"command\":[\"get_property\",\"%s\"]}\n", 
        property
    );

    if (written < 0 || (size_t)written >= sizeof(json_command)) {
        fprintf(stderr, "snprintf: JSON command too long\n");
        return PLAYER_ERROR;
    }

    cJSON* response = NULL;

    int request_result = player_request(json_command, &response);
    if (request_result == PLAYER_ERROR) {
        return PLAYER_ERROR;
    }

    cJSON* data = cJSON_GetObjectItemCaseSensitive(response, "data");
    if (!cJSON_IsNumber(data)) {
        fprintf(stderr, "cJSON: data is not a number\n");
        cJSON_Delete(response);
        return PLAYER_ERROR;
    }

    *value = data->valuedouble;

    cJSON_Delete(response);
    return PLAYER_SUCCESS;
}
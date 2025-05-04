#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

pid_t monitor_pid = -1;

void start_monitor() {
    if (monitor_pid != -1) {
        printf("Monitor is already running.\n");
        return;
    }

    monitor_pid = fork();
    if (monitor_pid == 0) {
        // Child process: Monitor
        while (1) {
            pause ```c
(); // Wait for signals
        }
    } else if (monitor_pid < 0) {
        perror("Failed to fork monitor process");
    } else {
        printf("Monitor started with PID %d.\n", monitor_pid);
    }
}

void stop_monitor() {
    if (monitor_pid == -1) {
        printf("No monitor is running.\n");
        return;
    }

    kill(monitor_pid, SIGTERM);
    waitpid(monitor_pid, NULL, 0);
    printf("Monitor stopped.\n");
    monitor_pid = -1;
}

void list_hunts() {
    if (monitor_pid == -1) {
        printf("Monitor is not running. Start the monitor first.\n");
        return;
    }

    kill(monitor_pid, SIGUSR1);
}

void list_treasures(char *hunt_id) {
    if (monitor_pid == -1) {
        printf("Monitor is not running. Start the monitor first.\n");
        return;
    }

    // Send hunt_id to the monitor through a file or signal
    int fd = open("command_pipe", O_WRONLY);
    if (fd != -1) {
        write(fd, hunt_id, strlen(hunt_id) + 1);
        close(fd);
        kill(monitor_pid, SIGUSR1);
    } else {
        perror("Failed to open command pipe");
    }
}

void view_treasure(char *hunt_id, int id) {
    if (monitor_pid == -1) {
        printf("Monitor is not running. Start the monitor first.\n");
        return;
    }

    // Send hunt_id and id to the monitor through a file or signal
    char command[256];
    snprintf(command, sizeof(command), "%s %d", hunt_id, id);
    int fd = open("command_pipe", O_WRONLY);
    if (fd != -1) {
        write(fd, command, strlen(command) + 1);
        close(fd);
        kill(monitor_pid, SIGUSR1);
    } else {
        perror("Failed to open command pipe");
    }
}

void exit_program() {
    if (monitor_pid != -1) {
        printf("Stopping monitor before exiting...\n");
        stop_monitor();
    }
    exit(0);
}

void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        // Handle the signal from the monitor
        printf("Received signal from monitor.\n");
    } else if (sig == SIGCHLD) {
        // Handle monitor termination
        printf("Monitor process terminated.\n");
        monitor_pid = -1;
    }
}

int main() {
    signal(SIGUSR1, handle_signal);
    signal(SIGCHLD, handle_signal);

    char command[256];
    while (1) {
        printf("Enter command (start_monitor, list_hunts, list_treasures <hunt_id>, view_treasure <hunt_id> <id>, stop_monitor, exit): ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline

        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
        } else if (strcmp(command, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(command, "list_hunts") == 0) {
            list_hunts();
        } else if (strncmp(command, "list_treasures", 15) == 0) {
            char *hunt_id = command + 16;
            list_treasures(hunt_id);
        } else if (strncmp(command, "view_treasure", 13) == 0) {
            char *hunt_id = strtok(command + 14, " ");
            char *id_str = strtok(NULL, " ");
            if (hunt_id && id_str) {
                int id = atoi(id_str);
                view_treasure(hunt_id, id);
            } else {
                printf("Invalid command format.\n");
            }
        } else if (strcmp(command, "exit") == 0) {
            exit_program();
        } else {
            printf("Unknown command.\n");
        }
    }

    return 0;
}

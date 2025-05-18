#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>

#define MAX_CMD_LEN 100
#define MAX_HUNT_ID 50
#define MAX_TREASURE_ID 20

pid_t monitor_pid = 0;
int monitor_running = 0;

void handle_sigchld(int sig) {
    int status;
    pid_t child_pid = waitpid(-1, &status, WNOHANG);
    if (child_pid > 0 && child_pid == monitor_pid) {
        monitor_running = 0;
        monitor_pid = 0;
        printf("\nMonitor process terminated with status %d\n", status);
    }
}

void start_monitor() {
    if (monitor_running) {
        printf("Monitor is already running.\n");
        return;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process (monitor)
        execl("./monitor", "monitor", NULL);
        perror("Failed to start monitor");
        exit(1);
    } else if (pid > 0) {
        // Parent process
        monitor_pid = pid;
        monitor_running = 1;
        printf("Monitor started with PID %d\n", pid);
    } else {
        perror("Failed to start monitor");
    }
}

void start_monitor() {
    if (monitor_running) {
        printf("Monitor is already running.\n");
        return;
    }
    
    pid_t pid = fork();
    if (pid == 0) {
        // Child process (monitor)
        monitor_process();
    } else if (pid > 0) {
        // Parent process
        monitor_pid = pid;
        monitor_running = 1;
        printf("Monitor started with PID %d\n", pid);
    } else {
        perror("Failed to start monitor");
    }
}

void send_command_to_monitor(const char *cmd) {
    if (!monitor_running) {
        printf("Monitor is not running.\n");
        return;
    }
    
    int cmd_fd = open("monitor_cmd", O_WRONLY);
    if (cmd_fd == -1) {
        perror("Failed to send command to monitor");
        return;
    }
    
    write(cmd_fd, cmd, strlen(cmd) + 1);
    close(cmd_fd);
    
    // Send signal to wake up monitor
    kill(monitor_pid, SIGUSR1);
}

void stop_monitor() {
    if (!monitor_running) {
        printf("Monitor is not running.\n");
        return;
    }
    
    send_command_to_monitor("stop_monitor");
    printf("Requested monitor to stop...\n");
}
void calculate_scores() {
    DIR *dir;
    struct dirent *entry;
    
    dir = opendir(".");
    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                pid_t pid = fork();
                if (pid == 0) {
                    // Child process - run score calculator
                    execl("./score_calculator", "score_calculator", entry->d_name, NULL);
                    perror("Failed to execute score calculator");
                    exit(1);
                } else if (pid < 0) {
                    perror("Failed to fork for score calculation");
                }
            }
        }
        closedir(dir);
    }
    
    // Wait for all child processes to complete
    while (wait(NULL) > 0);
}

int main() {
    // Set up SIGCHLD handler
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
    
    printf("Treasure Hunt Hub - Interactive Interface\n");
    printf("Available commands:\n");
    printf("  start_monitor\n");
    printf("  list_hunts\n");
    printf("  list_treasures <hunt_id>\n");
    printf("  view_treasure <hunt_id> <treasure_id>\n");
    printf("  stop_monitor\n");
    printf("  exit\n");
    
    char command[MAX_CMD_LEN];
    while (1) {
        printf("\n> ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = '\0'; // Remove newline
        
        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
        }
        else if (strcmp(command, "list_hunts") == 0) {
            send_command_to_monitor("list_hunts");
        }
        else if (strncmp(command, "list_treasures ", 15) == 0) {
            send_command_to_monitor(command);
        }
        else if (strncmp(command, "view_treasure ", 14) == 0) {
            send_command_to_monitor(command);
        }
        else if (strcmp(command, "stop_monitor") == 0) {
            stop_monitor();
        }
        else if (strcmp(command, "calculate_score") == 0) {
            calculate_scores();

        else if (strcmp(command, "exit") == 0) {
            if (monitor_running) {
                printf("Error: Monitor is still running. Stop it first.\n");
            } else {
                break;
            }
        }
        else {
            printf("Unknown command. Try again.\n");
        }
    }
    
    return 0;
}

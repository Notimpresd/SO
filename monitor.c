#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#define MAX_CMD_LEN 100
#define MAX_HUNT_ID 50
#define MAX_TREASURE_ID 20

typedef struct {
    char id[MAX_TREASURE_ID];
    char user[MAX_NAME_LEN];
    double latitude;
    double longitude;
    char clue[200];
    int value;
} Treasure;

void handle_monitor_commands(const char *cmd) {
    if (strcmp(cmd, "list_hunts") == 0) {
        DIR *dir;
        struct dirent *entry;
        
        printf("\n=== Hunts List ===\n");
        dir = opendir(".");
        if (dir) {
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                    char filename[100];
                    snprintf(filename, sizeof(filename), "%s/treasures.dat", entry->d_name);
                    
                    FILE *file = fopen(filename, "rb");
                    if (file) {
                        fseek(file, 0, SEEK_END);
                        long size = ftell(file);
                        int count = size / sizeof(Treasure);
                        fclose(file);
                        
                        printf("%s: %d treasures\n", entry->d_name, count);
                    }
                }
            }
            closedir(dir);
        }
    }
    else if (strncmp(cmd, "list_treasures ", 15) == 0) {
        char hunt_id[MAX_HUNT_ID];
        strcpy(hunt_id, cmd + 15);
        
        char filename[100];
        snprintf(filename, sizeof(filename), "%s/treasures.dat", hunt_id);
        
        FILE *file = fopen(filename, "rb");
        if (file) {
            printf("\n=== Treasures in %s ===\n", hunt_id);
            Treasure treasure;
            while (fread(&treasure, sizeof(Treasure), 1, file)) {
                printf("%s\t%s\t%.6f\t%.6f\t%d\t%s\n", 
                       treasure.id, treasure.user, treasure.latitude, 
                       treasure.longitude, treasure.value, treasure.clue);
            }
            fclose(file);
        } else {
            printf("Hunt %s not found or has no treasures.\n", hunt_id);
        }
    }
    else if (strncmp(cmd, "view_treasure ", 14) == 0) {
        char hunt_id[MAX_HUNT_ID], treasure_id[MAX_TREASURE_ID];
        sscanf(cmd + 14, "%s %s", hunt_id, treasure_id);
        
        char filename[100];
        snprintf(filename, sizeof(filename), "%s/treasures.dat", hunt_id);
        
        FILE *file = fopen(filename, "rb");
        if (file) {
            Treasure treasure;
            int found = 0;
            while (fread(&treasure, sizeof(Treasure), 1, file)) {
                if (strcmp(treasure.id, treasure_id) == 0) {
                    found = 1;
                    printf("\nTreasure Details:\n");
                    printf("ID: %s\n", treasure.id);
                    printf("User: %s\n", treasure.user);
                    printf("Coordinates: %.6f, %.6f\n", treasure.latitude, treasure.longitude);
                    printf("Value: %d\n", treasure.value);
                    printf("Clue: %s\n", treasure.clue);
                    break;
                }
            }
            if (!found) {
                printf("Treasure %s not found in hunt %s.\n", treasure_id, hunt_id);
            }
            fclose(file);
        } else {
            printf("Hunt %s not found or has no treasures.\n", hunt_id);
        }
    }
}

int main() {
    // Set up signal handlers
    struct sigaction sa;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    
    sa.sa_handler = SIG_IGN;
    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    
    // Create command pipe
    mkfifo("monitor_cmd", 0666);
    
    while (1) {
        // Wait for commands
        int cmd_fd = open("monitor_cmd", O_RDONLY);
        char cmd[MAX_CMD_LEN];
        read(cmd_fd, cmd, sizeof(cmd));
        close(cmd_fd);
        
        if (strcmp(cmd, "stop_monitor") == 0) {
            // Delay exit to demonstrate async behavior
            usleep(500000); // 0.5 second delay
            break;
        }
        
        handle_monitor_commands(cmd);
    }
    
    // Clean up
    unlink("monitor_cmd");
    return 0;
}

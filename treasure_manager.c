#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <dirent.h>

#define MAX_NAME_LEN 50
#define MAX_CLUE_LEN 200
#define MAX_ID_LEN 20
#define RECORD_SIZE (MAX_ID_LEN + MAX_NAME_LEN + 2*sizeof(double) + MAX_CLUE_LEN + sizeof(int))

typedef struct {
    char id[MAX_ID_LEN];
    char user[MAX_NAME_LEN];
    double latitude;
    double longitude;
    char clue[MAX_CLUE_LEN];
    int value;
} Treasure;

void log_operation(const char *hunt_id, const char *operation) {
    char log_filename[100];
    snprintf(log_filename, sizeof(log_filename), "%s/logged_hunt", hunt_id);
    
    FILE *log_file = fopen(log_filename, "a");
    if (log_file) {
        time_t now = time(NULL);
        fprintf(log_file, "[%s] %s\n", ctime(&now), operation);
        fclose(log_file);
        
        // Create symbolic link
        char linkname[100];
        snprintf(linkname, sizeof(linkname), "logged_hunt-%s", hunt_id);
        symlink(log_filename, linkname);
    }
}

void add_treasure(const char *hunt_id) {
    Treasure treasure;
    printf("Enter treasure ID: ");
    scanf("%s", treasure.id);
    printf("Enter user name: ");
    scanf("%s", treasure.user);
    printf("Enter latitude: ");
    scanf("%lf", &treasure.latitude);
    printf("Enter longitude: ");
    scanf("%lf", &treasure.longitude);
    printf("Enter clue: ");
    scanf(" %[^\n]", treasure.clue);
    printf("Enter value: ");
    scanf("%d", &treasure.value);

    // Create hunt directory if it doesn't exist
    mkdir(hunt_id, 0755);

    char filename[100];
    snprintf(filename, sizeof(filename), "%s/treasures.dat", hunt_id);
    
    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }
    
    write(fd, &treasure, sizeof(Treasure));
    close(fd);
    
    char operation[200];
    snprintf(operation, sizeof(operation), "ADD treasure %s", treasure.id);
    log_operation(hunt_id, operation);
    
    printf("Treasure added successfully.\n");
}

void list_treasures(const char *hunt_id) {
    char filename[100];
    snprintf(filename, sizeof(filename), "%s/treasures.dat", hunt_id);
    
    struct stat file_stat;
    if (stat(filename, &file_stat) == -1) {
        printf("Hunt %s not found or has no treasures.\n", hunt_id);
        return;
    }
    
    printf("Hunt: %s\n", hunt_id);
    printf("File size: %ld bytes\n", file_stat.st_size);
    printf("Last modified: %s", ctime(&file_stat.st_mtime));
    
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }
    
    Treasure treasure;
    printf("\nTreasures:\n");
    printf("ID\tUser\tLatitude\tLongitude\tValue\tClue\n");
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        printf("%s\t%s\t%.6f\t%.6f\t%d\t%s\n", 
               treasure.id, treasure.user, treasure.latitude, 
               treasure.longitude, treasure.value, treasure.clue);
    }
    close(fd);
    
    char operation[200];
    snprintf(operation, sizeof(operation), "LIST treasures");
    log_operation(hunt_id, operation);
}

void view_treasure(const char *hunt_id, const char *treasure_id) {
    char filename[100];
    snprintf(filename, sizeof(filename), "%s/treasures.dat", hunt_id);
    
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printf("Hunt %s not found or has no treasures.\n", hunt_id);
        return;
    }
    
    Treasure treasure;
    int found = 0;
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        if (strcmp(treasure.id, treasure_id) == 0) {
            found = 1;
            printf("Treasure Details:\n");
            printf("ID: %s\n", treasure.id);
            printf("User: %s\n", treasure.user);
            printf("Coordinates: %.6f, %.6f\n", treasure.latitude, treasure.longitude);
            printf("Value: %d\n", treasure.value);
            printf("Clue: %s\n", treasure.clue);
            break;
        }
    }
    close(fd);
    
    if (!found) {
        printf("Treasure %s not found in hunt %s.\n", treasure_id, hunt_id);
    }
    
    char operation[200];
    snprintf(operation, sizeof(operation), "VIEW treasure %s", treasure_id);
    log_operation(hunt_id, operation);
}

void remove_treasure(const char *hunt_id, const char *treasure_id) {
    char filename[100];
    snprintf(filename, sizeof(filename), "%s/treasures.dat", hunt_id);
    
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printf("Hunt %s not found or has no treasures.\n", hunt_id);
        return;
    }
    
    char temp_filename[100];
    snprintf(temp_filename, sizeof(temp_filename), "%s/temp.dat", hunt_id);
    int temp_fd = open(temp_filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (temp_fd == -1) {
        perror("Error creating temp file");
        close(fd);
        return;
    }
    
    Treasure treasure;
    int found = 0;
    while (read(fd, &treasure, sizeof(Treasure)) > 0) {
        if (strcmp(treasure.id, treasure_id) != 0) {
            write(temp_fd, &treasure, sizeof(Treasure));
        } else {
            found = 1;
        }
    }
    close(fd);
    close(temp_fd);
    
    if (!found) {
        printf("Treasure %s not found in hunt %s.\n", treasure_id, hunt_id);
        unlink(temp_filename);
        return;
    }
    
    // Replace original file with temp file
    unlink(filename);
    rename(temp_filename, filename);
    
    char operation[200];
    snprintf(operation, sizeof(operation), "REMOVE treasure %s", treasure_id);
    log_operation(hunt_id, operation);
    
    printf("Treasure %s removed from hunt %s.\n", treasure_id, hunt_id);
}

void remove_hunt(const char *hunt_id) {
    // Remove the symbolic link first
    char linkname[100];
    snprintf(linkname, sizeof(linkname), "logged_hunt-%s", hunt_id);
    unlink(linkname);
    
    // Remove the directory and its contents
    char command[200];
    snprintf(command, sizeof(command), "rm -rf %s", hunt_id);
    system(command);
    
    printf("Hunt %s and all its treasures have been removed.\n", hunt_id);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage:\n");
        printf("  treasure_manager --add <hunt_id>\n");
        printf("  treasure_manager --list <hunt_id>\n");
        printf("  treasure_manager --view <hunt_id> <treasure_id>\n");
        printf("  treasure_manager --remove_treasure <hunt_id> <treasure_id>\n");
        printf("  treasure_manager --remove_hunt <hunt_id>\n");
        return 1;
    }
    
    if (strcmp(argv[1], "--add") == 0 && argc == 3) {
        add_treasure(argv[2]);
    }
    else if (strcmp(argv[1], "--list") == 0 && argc == 3) {
        list_treasures(argv[2]);
    }
    else if (strcmp(argv[1], "--view") == 0 && argc == 4) {
        view_treasure(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "--remove_treasure") == 0 && argc == 4) {
        remove_treasure(argv[2], argv[3]);
    }
    else if (strcmp(argv[1], "--remove_hunt") == 0 && argc == 3) {
        remove_hunt(argv[2]);
    }
    else {
        printf("Invalid command or arguments.\n");
        return 1;
    }
    
    return 0;
}

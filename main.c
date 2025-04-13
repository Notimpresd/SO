#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>

#define LOG_FILE "logged_hunt"
#define HUNT_DIR_PREFIX "hunt_"
#define TREASURE_FILE "treasures.dat"
#define MAX_HUNTS 100

typedef struct {
    int ID;
    char user_name[20];
    float GPS_longitude;
    float GPS_latitude;
    char clue[100];
    int value;
} Treasure;

// Function prototypes
void create_hunt_directory(char *hunt_id);
void log_operation(char *hunt_id, char *operation);
void create_symlink(char *hunt_id);
int get_next_treasure_id(char *hunt_id);
void update_treasure_count(char *hunt_id, int change);

void add_treasure(char *hunt_id) {
    create_hunt_directory(hunt_id);

    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s%s/%s", HUNT_DIR_PREFIX, hunt_id, TREASURE_FILE);

    Treasure new_treasure;
    new_treasure.ID = get_next_treasure_id(hunt_id);

    printf("Enter user name (max 19 chars): ");
    scanf("%19s", new_treasure.user_name);
    printf("Enter GPS longitude: ");
    scanf("%f", &new_treasure.GPS_longitude);
    printf("Enter GPS latitude: ");
    scanf("%f", &new_treasure.GPS_latitude);
    printf("Enter clue text (max 99 chars): ");
    scanf(" %[^\n]", new_treasure.clue);
    printf("Enter treasure value: ");
    scanf("%d", &new_treasure.value);

    int fd = open(treasure_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error opening treasure file");
        exit(EXIT_FAILURE);
    }

    if (write(fd, &new_treasure, sizeof(Treasure)) != sizeof(Treasure)) {
        perror("Error writing treasure");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
    update_treasure_count(hunt_id, 1);
    printf("\nTreasure added to hunt %s with ID %d.\n", hunt_id, new_treasure.ID);
}

void list_treasures(char *hunt_id) {
    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s%s/%s", HUNT_DIR_PREFIX, hunt_id, TREASURE_FILE);

    struct stat file_stat;
    if (stat(treasure_path, &file_stat) == -1) {
        perror("Error getting file info");
        exit(EXIT_FAILURE);
    }

    printf("\nHunt: %s\n", hunt_id);
    printf("File size: %ld bytes\n", file_stat.st_size);
    printf("Last modified: %s", ctime(&file_stat.st_mtime));

    int fd = open(treasure_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        exit(EXIT_FAILURE);
    }

    Treasure treasure;
    printf("\nTreasures in hunt %s:\n", hunt_id);
    printf("ID\tUser\t\tLongitude\tLatitude\tValue\tClue\n");
    printf("-----------------------------------------------------------------\n");

    while (read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)) {
        printf("%d\t%s\t%.6f\t%.6f\t%d\t%.30s%s\n",
               treasure.ID, treasure.user_name,
               treasure.GPS_longitude, treasure.GPS_latitude,
               treasure.value, treasure.clue,
               strlen(treasure.clue) > 30 ? "..." : "");
    }

    close(fd);
}

void view_treasure(char *hunt_id, int id) {
    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s%s/%s", HUNT_DIR_PREFIX, hunt_id, TREASURE_FILE);

    int fd = open(treasure_path, O_RDONLY);
    if (fd == -1) {
        perror("Error opening treasure file");
        exit(EXIT_FAILURE);
    }

    Treasure treasure;
    int found = 0;

    while (read(fd, &treasure, sizeof(Treasure)) == sizeof(Treasure)) {
        if (treasure.ID == id) {
            found = 1;
            printf("\nTreasure details (ID: %d):\n", id);
            printf("User: %s\n", treasure.user_name);
            printf("GPS Coordinates: %.6f, %.6f\n", treasure.GPS_longitude, treasure.GPS_latitude);
            printf("Value: %d\n", treasure.value);
            printf("Clue: %s\n", treasure.clue);
            break;
        }
    }

    close(fd);

    if (!found) {
        printf("Treasure with ID %d not found in hunt %s.\n", id, hunt_id);
    }
}

void remove_treasure(char *hunt_id, int id) {
    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s%s/%s", HUNT_DIR_PREFIX, hunt_id, TREASURE_FILE);

    char temp_path[256];
    snprintf(temp_path, sizeof(temp_path), "%s%s/temp.dat", HUNT_DIR_PREFIX, hunt_id);

    int fd_in = open(treasure_path, O_RDONLY);
    if (fd_in == -1) {
        perror("Error opening treasure file");
        exit(EXIT_FAILURE);
    }

    int fd_out = open(temp_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        perror("Error creating temp file");
        close(fd_in);
        exit(EXIT_FAILURE);
    }

    Treasure treasure;
    int found = 0;

    while (read(fd_in, &treasure, sizeof(Treasure)) == sizeof(Treasure)) {
        if (treasure.ID != id) {
            write(fd_out, &treasure, sizeof(Treasure));
        } else {
            found = 1;
        }
    }

    close(fd_in);
    close(fd_out);

    if (!found) {
        unlink(temp_path);
        printf("Treasure with ID %d not found in hunt %s.\n", id, hunt_id);
        return;
    }

    if (rename(temp_path, treasure_path) == -1) {
        perror("Error replacing treasure file");
        exit(EXIT_FAILURE);
    }

    update_treasure_count(hunt_id, -1);
    printf("\nTreasure %d removed from hunt %s.\n", id, hunt_id);
}

void remove_hunt(char *hunt_id) {
    char hunt_path[256];
    snprintf(hunt_path, sizeof(hunt_path), "%s%s", HUNT_DIR_PREFIX, hunt_id);

    char treasure_path[256];
    snprintf(treasure_path, sizeof(treasure_path), "%s/%s", hunt_path, TREASURE_FILE);

    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/%s", hunt_path, LOG_FILE);

    if (unlink(treasure_path) == -1) {
        perror("Error removing treasure file");
    }

    if (unlink(log_path) == -1) {
        perror("Error removing log file");
    }

    if (rmdir(hunt_path) == -1) {
        perror("Error removing hunt directory");
        exit(EXIT_FAILURE);
    }

    char symlink_path[256];
    snprintf(symlink_path, sizeof(symlink_path), "logged_hunt-%s", hunt_id);
    unlink(symlink_path);

    printf("\nHunt %s removed.\n", hunt_id);
}

void create_hunt_directory(char *hunt_id) {
    char hunt_path[256];
    snprintf(hunt_path, sizeof(hunt_path), "%s%s", HUNT_DIR_PREFIX, hunt_id);

    if (mkdir(hunt_path) == -1 && errno != EEXIST) {
        perror("Error creating hunt directory");
        exit(EXIT_FAILURE);
    }

    // Create log file if it doesn't exist
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s/%s", hunt_path, LOG_FILE);

    int fd = open(log_path, O_WRONLY | O_CREAT | O_APPEND, 0644);
    if (fd == -1) {
        perror("Error creating log file");
        exit(EXIT_FAILURE);
    }
    close(fd);

    create_symlink(hunt_id);
}

void log_operation(char *hunt_id, char *operation) {
    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s%s/%s", HUNT_DIR_PREFIX, hunt_id, LOG_FILE);

    int fd = open(log_path, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("Error opening log file");
        exit(EXIT_FAILURE);
    }

    time_t now = time(NULL);
    char *timestamp = ctime(&now);
    timestamp[strlen(timestamp)-1] = '\0'; // Remove newline

    char log_entry[512];
    snprintf(log_entry, sizeof(log_entry), "[%s] Operation: %s\n", timestamp, operation);

    write(fd, log_entry, strlen(log_entry));
    close(fd);

    printf("Operation \"%s\" from hunt %s has been recorded in log file.\n", operation, hunt_id);
}

void create_symlink(char *hunt_id) {
    char symlink_path[256];
    snprintf(symlink_path, sizeof(symlink_path), "logged_hunt-%s", hunt_id);

    char log_path[256];
    snprintf(log_path, sizeof(log_path), "%s%s/%s", HUNT_DIR_PREFIX, hunt_id, LOG_FILE);

    // Remove existing symlink if it exists
    unlink(symlink_path);

    if (symlink(log_path, symlink_path) == -1) {
        perror("Error creating symlink");
        exit(EXIT_FAILURE);
    }

    printf("\nSymlink for hunt %s created.\n", hunt_id);
}

int get_next_treasure_id(char *hunt_id) {
    char count_path[256];
    snprintf(count_path, sizeof(count_path), "%s%s/treasure_count.dat", HUNT_DIR_PREFIX, hunt_id);

    int count = 1;
    int fd = open(count_path, O_RDONLY);
    if (fd != -1) {
        read(fd, &count, sizeof(int));
        close(fd);
    }

    return count;
}

void update_treasure_count(char *hunt_id, int change) {
    char count_path[256];
    snprintf(count_path, sizeof(count_path), "%s%s/treasure_count.dat", HUNT_DIR_PREFIX, hunt_id);

    int count = 1;
    int fd = open(count_path, O_RDONLY);
    if (fd != -1) {
        read(fd, &count, sizeof(int));
        close(fd);
    }

    count += change;

    fd = open(count_path, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        perror("Error updating treasure count");
        exit(EXIT_FAILURE);
    }

    write(fd, &count, sizeof(int));
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <operation> <hunt_id> [id]\n", argv[0]);
        printf("Operations: add, list, view, remove_treasure, remove_hunt\n");
        exit(EXIT_FAILURE);
    }

    char *operation = argv[1];
    char *hunt_id = argv[2];

    if (strcmp(operation, "add") == 0) {
        add_treasure(hunt_id);
        log_operation(hunt_id, operation);
    }
    else if (strcmp(operation, "list") == 0) {
        list_treasures(hunt_id);
    }
    else if (strcmp(operation, "view") == 0 && argc == 4) {
        int id = atoi(argv[3]);
        view_treasure(hunt_id, id);
    }
    else if (strcmp(operation, "remove_treasure") == 0 && argc == 4) {
        int id = atoi(argv[3]);
        remove_treasure(hunt_id, id);
        log_operation(hunt_id, operation);
    }
    else if (strcmp(operation, "remove_hunt") == 0) {
        remove_hunt(hunt_id);
        log_operation(hunt_id, operation);
    }
    else {
        printf("Invalid arguments.\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

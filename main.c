#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct {
    int ID;
    char user_name[20];
    float GPS_longitude;
    float GPS_latitude;
    char clue[100];
    int value;
} Treasure;


void add_treasure(char *hunt_id)//add
{
    printf("Treasure added to hunt %s",hunt_id);
}

void list_treasures(char *hunt_id)//list
{
    printf("Treasures listed for hunt %s",hunt_id);

}

void view_treasure(char *hunt_id, int id)//view
{
    printf("Treasure %d viewed for %s",id,hunt_id);

}

void remove_treasure(char *hunt_id, int id)//remove_treasure
{
    printf("Treasure %d removed to hunt %s",id,hunt_id);

}

void remove_hunt(char *hunt_id)//remove_hunt
{
    printf("Hunt removed from hunt %s",hunt_id);

}

void log_operation(char *hunt_id, char *operation)//log
{
    printf("Operation %s from hunt %s log.",operation,hunt_id);

}

void create_symlink(char *hunt_id)//create_symlink
{
    printf("Symlink for hunt %s created.",hunt_id);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Not enough arguments.\n");
        exit(EXIT_FAILURE);
    }

    char *operation = argv[1];
    char *hunt_id = argv[2];

    if (strcmp(operation, "add") == 0) {
        add_treasure(hunt_id);
    } else if (strcmp(operation, "list") == 0) {
        list_treasures(hunt_id);
    } else if (strcmp(operation, "view") == 0 && argc == 4) {
        int id = atoi(argv[3]);
        view_treasure(hunt_id, id);
    } else if (strcmp(operation, "remove_treasure") == 0 && argc == 4) {
        int id = atoi(argv[3]);
        remove_treasure(hunt_id, id);
    } else if (strcmp(operation, "remove_hunt") == 0) {
        remove_hunt(hunt_id);
    } else {
        printf("Invalid arguments\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

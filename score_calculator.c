#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_NAME_LEN 50
#define MAX_ID_LEN 20

typedef struct {
    char id[MAX_ID_LEN];
    char user[MAX_NAME_LEN];
    double latitude;
    double longitude;
    char clue[200];
    int value;
} Treasure;

typedef struct {
    char user[MAX_NAME_LEN];
    int total_score;
} UserScore;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_id>\n", argv[0]);
        return 1;
    }
    
    char filename[100];
    snprintf(filename, sizeof(filename), "%s/treasures.dat", argv[1]);
    
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Hunt %s not found or has no treasures.\n", argv[1]);
        return 1;
    }
    
    UserScore scores[100];
    int num_users = 0;
    Treasure treasure;
    
    while (fread(&treasure, sizeof(Treasure), 1, file)) {
        int found = 0;
        for (int i = 0; i < num_users; i++) {
            if (strcmp(scores[i].user, treasure.user) == 0) {
                scores[i].total_score += treasure.value;
                found = 1;
                break;
            }
        }
        
        if (!found && num_users < 100) {
            strcpy(scores[num_users].user, treasure.user);
            scores[num_users].total_score = treasure.value;
            num_users++;
        }
    }
    fclose(file);
    
    printf("\n=== Scores for Hunt %s ===\n", argv[1]);
    for (int i = 0; i < num_users; i++) {
        printf("%s: %d\n", scores[i].user, scores[i].total_score);
    }
    
    return 0;
}

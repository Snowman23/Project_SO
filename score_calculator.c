#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manager_utils.h"

typedef struct {
    char username[30];
    int total_score;
} UserScore;

int find_user(UserScore* users, int user_count, const char* name) {
    for (int i = 0; i < user_count; ++i) {
        if (strcmp(users[i].username, name) == 0)
            return i;
    }
    return -1; 
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_directory>\n", argv[0]);
        return 1;
    }

    const char* hunt_dir = argv[1];
    char treasure_file_path[512];
    snprintf(treasure_file_path, sizeof(treasure_file_path), "%s/file.txt", hunt_dir);
    printf("Checking treasure file: '%s'\n", treasure_file_path);

    FileEntry treasures[20];
    int count = load_treasures(treasure_file_path, treasures, 20);

    if (count < 0) {
        fprintf(stderr, "Failed to load treasure file: %s\n", treasure_file_path);
        return 1;
    }

    if (count == 0) {
        printf("No treasures found in the file.\n");
        return 0;
    }


    UserScore users[30];
    int user_count = 0;

    for (int i = 0; i < count; ++i) {
        int idx = find_user(users, user_count, treasures[i].name);
        if (idx == -1) {
            strncpy(users[user_count].username, treasures[i].name, sizeof(users[user_count].username) - 1);
            users[user_count].username[sizeof(users[user_count].username) - 1] = '\0';
            users[user_count].total_score = treasures[i].value;
            user_count++;
        } else {
            users[idx].total_score += treasures[i].value;
        }
    }

    for (int i = 0; i < user_count; ++i) {
        printf("%s: %d\n", users[i].username, users[i].total_score);
    }

    return 0;
}

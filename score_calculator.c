#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_USERS 100
#define MAX_LINE 512

typedef struct {
    char username[64];
    int total_score;
} UserScore;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <hunt_directory>\n", argv[0]);
        return 1;
    }

    char filepath[256];
    snprintf(filepath, sizeof(filepath), "%s/file.txt", argv[1]);

    FILE *f = fopen(filepath, "r");
    if (!f) {
        perror("Could not open hunt file");
        return 1;
    }

    char line[MAX_LINE];
    UserScore users[MAX_USERS];
    int user_count = 0;

    // Skip header line (assuming the first line is header)
    fgets(line, sizeof(line), f);

    while (fgets(line, sizeof(line), f)) {
        int id, value;
        char name[64];
        float lat, lon;
        char clue_with_value[256];
        char *dot_pos;

        // Parse id, name, gps coords and rest (clue + value)
        // Format: id name lat lon clue... value
        // Example: 1 seeker_ash 34.0522 -118.2437 Under the oldest oak tree in the park. 500

        int scanned = sscanf(line, "%d %63s %f %f %[^\n]", &id, name, &lat, &lon, clue_with_value);
        if (scanned < 5) {
            fprintf(stderr, "Malformed line: %s", line);
            continue;
        }

        // Find last '.' in clue_with_value to separate clue and value
        dot_pos = strrchr(clue_with_value, '.');
        if (!dot_pos) {
            fprintf(stderr, "Malformed clue (missing '.'): %s\n", clue_with_value);
            continue;
        }

        // Extract value after the dot
        if (sscanf(dot_pos + 1, "%d", &value) != 1) {
            fprintf(stderr, "Could not read value from clue: %s\n", dot_pos + 1);
            continue;
        }

        // Sum the value to user's total
        int found = 0;
        for (int i = 0; i < user_count; i++) {
            if (strcmp(users[i].username, name) == 0) {
                users[i].total_score += value;
                found = 1;
                break;
            }
        }
        if (!found) {
            if (user_count < MAX_USERS) {
                strncpy(users[user_count].username, name, sizeof(users[user_count].username) - 1);
                users[user_count].username[sizeof(users[user_count].username) - 1] = '\0';
                users[user_count].total_score = value;
                user_count++;
            } else {
                fprintf(stderr, "User list full, skipping %s\n", name);
            }
        }
    }

    fclose(f);

    // Print scores per user
    printf("User Scores:\n");
    for (int i = 0; i < user_count; i++) {
        printf("%s: %d\n", users[i].username, users[i].total_score);
    }

    return 0;
}

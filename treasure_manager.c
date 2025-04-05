#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int id;
    char name[30];
    float lat;
    float lon;
    char clue[200];
    int value;
} FileEntry;

void print(FILE*f){
    char line[512];
    FileEntry entry;
    fgets(line, sizeof(line), f);

    while (fgets(line, sizeof(line), f)) {
        // Temporary holders
        char gps[32];
        char clueWithValue[300];
        char* dotPos;

        if (sscanf(line, "%d %s %[^ ] %[^\n]", &entry.id, entry.name, gps, clueWithValue) < 4) {
            printf("Malformed line: %s", line);
            continue;
        }

        sscanf(gps, "%f,%f", &entry.lat, &entry.lon);

        // Find the last '.' in clueWithValue
        dotPos = strrchr(clueWithValue, '.');
        if (!dotPos) {
            printf("No '.' found in clue line: %s", clueWithValue);
            continue;
        }

        // Copy clue (up to and including the dot)
        size_t clueLength = dotPos - clueWithValue + 1;
        strncpy(entry.clue, clueWithValue, clueLength);
        entry.clue[clueLength] = '\0'; // Null-terminate

        // Scan the value after the dot
        if (sscanf(dotPos + 1, "%d", &entry.value) != 1) {
            printf("Could not read value from line: %s\n", dotPos + 1);
            continue;
        }

        printf("ID: %d | Name: %s | GPS: %.4f, %.4f | Clue: %s | Value: %d\n",
               entry.id, entry.name, entry.lat, entry.lon, entry.clue, entry.value);
    }

    fclose(f);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        perror("Error opening file");
        return 1;
    }

    print(f);
    return 0;
}

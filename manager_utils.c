#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "manager_utils.h"

void list(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Error opening file");
        return;
    }

    char line[512];
    FileEntry entry;

    while (fgets(line, sizeof(line), f)) {
        char gps[64], clue[256];

        if (sscanf(line, "%d\t%49[^\t]\t%63[^\t]\t%255[^\t]\t%d",
                   &entry.id, entry.name, gps, clue, &entry.value) != 5)
            continue;

        if (sscanf(gps, "%f,%f", &entry.lat, &entry.lon) != 2)
            continue;

        strncpy(entry.clue, clue, sizeof(entry.clue));
        entry.clue[sizeof(entry.clue) - 1] = '\0';

        printf("ID: %d | Name: %s | GPS: %.4f, %.4f | Clue: %s | Value: %d\n",
               entry.id, entry.name, entry.lat, entry.lon, entry.clue, entry.value);
    }

    fclose(f);
}

void view(const char* filename, int id) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Error opening file");
        return;
    }

    char line[512];
    FileEntry entry;

    while (fgets(line, sizeof(line), f)) {
        char gps[64], clue[256];

        if (sscanf(line, "%d\t%49[^\t]\t%63[^\t]\t%255[^\t]\t%d",
                   &entry.id, entry.name, gps, clue, &entry.value) != 5)
            continue;

        if (entry.id != id)
            continue;

        if (sscanf(gps, "%f,%f", &entry.lat, &entry.lon) != 2)
            continue;

        strncpy(entry.clue, clue, sizeof(entry.clue));
        entry.clue[sizeof(entry.clue) - 1] = '\0';

        printf("ID: %d | Name: %s | GPS: %.4f, %.4f | Clue: %s | Value: %d\n",
               entry.id, entry.name, entry.lat, entry.lon, entry.clue, entry.value);

        fclose(f);
        return;
    }

    printf("Treasure with ID %d not found.\n", id);
    fclose(f);
}

void remove_treasure(const char* filename, int id) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Error opening file");
        return;
    }

    FILE* temp = fopen("temp.txt", "w");
    if (!temp) {
        perror("Error creating temporary file");
        fclose(f);
        return;
    }

    char line[512];
    int temp_id;

    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%d", &temp_id) == 1 && temp_id == id)
            continue; 
        fputs(line, temp);
    }

    fclose(f);
    fclose(temp);

    if (remove(filename) != 0)
        perror("Error removing original file");

    if (rename("temp.txt", filename) != 0)
        perror("Error renaming temporary file");
}

int count_treasures(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        count++;
    }

    fclose(f);
    return count;
}

int load_treasures(const char* filename, FileEntry* entries, int max_entries) {
    FILE* f = fopen(filename, "r");
    if (!f) return -1;

    char line[512];
    int index = 0;

    while (fgets(line, sizeof(line), f) && index < max_entries) {
        FileEntry* e = &entries[index];
        char gps[64], clueWithValue[300];
        char* dot;

        if (sscanf(line, "%d %s %[^ ] %[^\n]", &e->id, e->name, gps, clueWithValue) < 4)
            continue;

        if (sscanf(gps, "%f,%f", &e->lat, &e->lon) != 2)
            continue;

        dot = strrchr(clueWithValue, '.');
        if (!dot) continue;

        size_t clue_len = dot - clueWithValue + 1;
        strncpy(e->clue, clueWithValue, clue_len);
        e->clue[clue_len] = '\0';

        if (sscanf(dot + 1, "%d", &e->value) != 1)
            continue;

        index++;
    }

    fclose(f);
    return index;
}

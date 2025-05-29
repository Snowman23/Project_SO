#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include "manager_utils.h"

void add(const char* filename) {
    FILE* f = fopen(filename, "ab");  // append binary
    if (!f) {
        perror("Error opening file");
        return;
    }

    FileEntry new_entry;

    printf("Id: ");
    scanf("%d", &new_entry.id);

    printf("Name: ");
    scanf("%29s", new_entry.name);

    printf("Coordinates (lat lon): ");
    scanf("%f %f", &new_entry.lat, &new_entry.lon);

    while ((getchar()) != '\n'); // clear stdin buffer

    printf("Clue: ");
    fgets(new_entry.clue, sizeof(new_entry.clue), stdin);
    new_entry.clue[strcspn(new_entry.clue, "\n")] = 0;  // trim newline

    printf("Value: ");
    scanf("%d", &new_entry.value);

    fwrite(&new_entry, sizeof(FileEntry), 1, f);
    printf("Treasure added!\n");

    fclose(f);
}

void list(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Error opening file");
        return;
    }

    char line[512];
    FileEntry entry;
    fgets(line, sizeof(line), f);

    while (fgets(line, sizeof(line), f)) {
        char clue[256];

        if (sscanf(line, "%d,%[^,],%f %f,%[^,],%d", 
            &entry.id, entry.name, &entry.lat, &entry.lon, entry.clue, &entry.value)!= 6) {
            printf("Line parse failed\n");
            continue;
        }


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

    // Skip header
    fgets(line, sizeof(line), f);

    while (fgets(line, sizeof(line), f)) {
        char gps[64], clue[256];

        if (sscanf(line, "%d,%49[^,],%63[^,],%255[^,],%d", &entry.id, entry.name, gps, clue, &entry.value) != 5)
            continue;

        if (entry.id != id)
            continue;

        if (sscanf(gps, "%f %f", &entry.lat, &entry.lon) != 2)
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

void remove_hunt(char hunt_id[]) {
    DIR *dir = opendir(hunt_id);
    struct dirent *entry;
    char file[512];

    if (!dir) {
        perror("opendir failed");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        snprintf(file, sizeof(file), "%s/%s", hunt_id, entry->d_name);
        if (remove(file) != 0)
            perror("remove failed");
    }

    closedir(dir);

    if (rmdir(hunt_id) != 0)
        perror("rmdir failed");
    else
        printf("Hunt '%s' removed successfully.\n", hunt_id);
}

int count_treasures(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return 0;

    int count = 0;
    char line[512];
    fgets(line, sizeof(line), f);
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
    fgets(line, sizeof(line), f); // Skip header
    int index = 0;

    while (fgets(line, sizeof(line), f) && index < max_entries) {
        FileEntry* e = &entries[index];

        if (sscanf(line, "%d,%[^,],%f %f,%[^,],%d", 
            &e->id, e->name, &e->lat, &e->lon, e->clue, &e->value) != 6) {
            continue;
        }

        index++;
    }

    fclose(f);
    return index;
}

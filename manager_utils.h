#ifndef MANAGER_UTILS_H
#define MANAGER_UTILS_H

typedef struct {
    int id;
    char name[64];
    float lat;
    float lon;
    char clue[128];
    int value;
} FileEntry;

void list(const char* filename);
void view(const char* filename, int id);
void remove_treasure(const char* filename, int id);
int count_treasures(const char* filename);
int load_treasures(const char* filename, FileEntry* entries, int max_entries);

#endif

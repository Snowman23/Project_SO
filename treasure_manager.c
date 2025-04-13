#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct {
    int id;
    char name[30];
    float lat;
    float lon;
    char clue[200];
    int value;
} FileEntry;

void add(char hunt_id[]) {
    FILE* f = fopen(hunt_id, "a+");
    if (!f) {
        perror("Error opening file");
        return;
    }

    char t[200];
    FileEntry *new = malloc(sizeof(FileEntry));
    if (new == NULL) {
        perror("malloc failed");
        return;
    }

    printf("Rest of the details:\n");
    
    printf("Id: ");
    scanf("%d", &new->id);

    printf("Name: ");
    scanf("%29s", t); 
    strcpy(new->name, t);
    while ((getchar()) != '\n'); 

    printf("Coordinates (lat lon): ");
    scanf("%f %f", &new->lat, &new->lon);
    while ((getchar()) != '\n');  

    printf("Clue: ");
    fgets(t, sizeof(t), stdin);  
    t[strcspn(t, "\n")] = 0;     
    strncpy(new->clue, t, sizeof(new->clue));
    new->clue[sizeof(new->clue) - 1] = '\0';


    printf("Value: ");
    scanf("%d", &new->value);

    fprintf(f,"%d\t %s\t %.4f %.4f\t %s\t %d\n",new->id,new->name,new->lat,new->lon,new->clue,new->value);
    printf("Treasure added!\n");

    free(new);
    fclose(f);
}

void list(char hunt_id[]){
    FILE* f = fopen(hunt_id, "a+");
    if (!f) {
        perror("Error opening file");
        return;
    }

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

void view(char hunt_id[], int treasure_id) {
    FILE* f = fopen(hunt_id, "r");
    if (!f) {
        perror("Error opening file");
        return;
    }

    char line[512];
    FileEntry entry;
    fgets(line, sizeof(line), f);

    while (fgets(line, sizeof(line), f)) {
        char gps[32];
        char clueWithValue[300];
        char* dotPos;

        if (sscanf(line, "%d %s %[^ ] %[^\n]", &entry.id, entry.name, gps, clueWithValue) < 4) {
            continue;
        }

        sscanf(gps, "%f,%f", &entry.lat, &entry.lon);

        dotPos = strrchr(clueWithValue, '.');
        if (!dotPos) continue;

        size_t clueLength = dotPos - clueWithValue + 1;
        strncpy(entry.clue, clueWithValue, clueLength);
        entry.clue[clueLength] = '\0';

        if (sscanf(dotPos + 1, "%d", &entry.value) != 1) continue;

        if (entry.id == treasure_id) {
            printf("Treasure Found!\n");
            printf("ID: %d | Name: %s | GPS: %.4f, %.4f | Clue: %s | Value: %d\n",
                   entry.id, entry.name, entry.lat, entry.lon, entry.clue, entry.value);
            fclose(f);
            return;
        }
    }

    fclose(f);
}

void remove_treasure (char hunt_id[], int treasure_id){
    FILE* f = fopen(hunt_id, "r");
    FILE *temp = fopen("temp.txt", "w");
    if (!f || !temp) {
        perror("Error opening file");
        return;
    }

    char line[512];
    int found = 0;
    int current_id;

    if (fgets(line, sizeof(line), f)) {
        fputs(line, temp);
    }

    while (fgets(line, sizeof(line), f)) {
        if (sscanf(line, "%d", &current_id) == 1) {
            if (current_id == treasure_id) {
                found = 1;
                continue;  
            }
        }
        fputs(line, temp);
    }

    fclose(f);
    fclose(temp);

    if (found) {
        remove(hunt_id);
        rename("temp.txt", hunt_id);
        printf("All treasures with ID %d removed.\n", treasure_id);
    } else {
        remove("temp.txt");
        printf("No treasures with ID %d found.\n", treasure_id);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3 && argc != 4) {                        //  ./f    func    dir   id
        printf("Usage: ./treasure_manager <command> [args...]\n");
        return 1;
    }

    char *hunt_id = argv[1]; 
    char file[200];
    char logged[200];

    mkdir(hunt_id);   
    snprintf(file, sizeof(file), "%s/file.txt", hunt_id);
    snprintf(logged, sizeof(logged), "%s/logged_hunt.txt", hunt_id);

    if(strcmp(argv[2],"list")==0){
        list(file);
    }
    else if(strcmp(argv[2],"add")==0){
        add(file);
    }
    else if(strcmp(argv[2],"view")==0){
        int id=atoi(argv[3]);
        view(file,id);
    }
    else if(strcmp(argv[2],"remove_treasure")==0){
        int id=atoi(argv[3]);
        remove_treasure(file,id);
    }

    FILE* g = fopen(logged, "a");
    if (!g) {
        perror("Error opening file");
        return 1;
    }
    if (argc == 4)
        fprintf(g, "Command: %s | Hunt: %s | Treasure ID: %s\n", argv[1], argv[2], argv[3]);
    else
        fprintf(g, "Command: %s | Hunt: %s\n", argv[1], argv[2]);

    fclose(g);

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

pid_t monitor_pid = -1;

void handle_signal(int sig) {
    if (sig == SIGUSR1) {
        printf("Received SIGUSR1 signal: Listing hunts...\n");
        // This can be a place to list hunts
    } else if (sig == SIGUSR2) {
        printf("Received SIGUSR2 signal: Viewing treasure...\n");
        // Handle treasure view here
    }
}

void start_monitor() {
    monitor_pid = fork();
    if (monitor_pid == 0) {
        // Child process - monitor logic
        signal(SIGUSR1, handle_signal);
        signal(SIGUSR2, handle_signal);
        while (1) {
            usleep(1000000);  // Simulate work, replace with actual logic
        }
    }
}

void stop_monitor() {
    if (monitor_pid != -1) {
        kill(monitor_pid, SIGTERM);  // Send termination signal to monitor
        int status;
        waitpid(monitor_pid, &status, 0);  // Wait for monitor process to finish
        printf("Monitor process terminated with status: %d\n", status);
    }
}

void list_hunts() {
    if (monitor_pid != -1) {
        kill(monitor_pid, SIGUSR1);  // Send signal to monitor to list hunts
    }
}

void list_treasures() {
    if (monitor_pid != -1) {
        kill(monitor_pid, SIGUSR2);  // Send signal to monitor to list treasures
    }
}

void view_treasure(int id) {
    if (monitor_pid != -1) {
        // In this case, let's assume we're sending an ID to the monitor
        printf("Requesting to view treasure with ID: %d\n", id);
        // Here you can send a specific signal or data to the monitor
    }
}

int main() {
    char command[256];
    printf("Welcome to Treasure Hub!\n");

    while (1) {
        printf("\nEnter command: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Remove newline at end

        if (strcmp(command, "start_monitor") == 0) {
            start_monitor();
            printf("Monitor started.\n");
        } else if (strcmp(command, "list_hunts") == 0) {
            list_hunts();
        } else if (strcmp(command, "list_treasures") == 0) {
            list_treasures();
        } else if (strncmp(command, "view_treasure", 13) == 0) {
            int id = atoi(command + 14);  // Assuming the ID comes after the space
            view_treasure(id);
        } else if (strcmp(command, "stop_monitor") == 0) {
            stop_monitor();
        } else if (strcmp(command, "exit") == 0) {
            if (monitor_pid != -1) {
                printf("You must stop the monitor before exiting.\n");
            } else {
                printf("Exiting Treasure Hub.\n");
                break;
            }
        } else {
            printf("Invalid command.\n");
        }
    }

    return 0;
}

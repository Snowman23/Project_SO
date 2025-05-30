#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <ctype.h>
#include "manager_utils.h" 

pid_t monitor_pid = -1;
bool monitor_running = false;
int pipefd[2]; // pipe between main and monitor
volatile sig_atomic_t received_signal = 0;

void send_signal_to_monitor(int sig){
    if (monitor_pid > 0 && monitor_running){
        kill(monitor_pid, sig);
    } else {
        printf("Monitor not running.\n");
    }
}

void handle_signal(int sig){
    received_signal = sig;
}

void handle_sigchld(int sig){
    int status;
    pid_t pid = waitpid(monitor_pid, &status, WNOHANG);
    if (pid == monitor_pid) {
        printf("Monitor exited with status %d\n", status);
        monitor_running = false;
        monitor_pid = -1;
    }
}

void monitor_loop(){
    struct sigaction sa;
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGUSR1, &sa, NULL);
    sigaction(SIGUSR2, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);

    while (1){
        pause();
        if (received_signal) {
            FILE *f = fopen("hub_command.txt", "r");
            if (!f){
                perror("Monitor: Failed to open command file");
                received_signal = 0;
                continue;
            }

            char cmd[64], hunt[64];
            int id = -1;
            fscanf(f, "%s %s %d", cmd, hunt, &id);
            fflush(f);
            fclose(f);

            if (isdigit(hunt[0])) {
                char temp[128];
                snprintf(temp, sizeof(temp), "hunt%s", hunt);
                strncpy(hunt, temp, sizeof(hunt));
                hunt[sizeof(hunt) - 1] = '\0';
            }

            char path[128];
            snprintf(path, sizeof(path), "%s/file.txt", hunt);

            if (strcmp(cmd, "list_hunts") == 0){
                int count = count_treasures(path);
                printf("Hunt: %s | Treasures: %d\n", hunt, count);
            }
            else if (strcmp(cmd, "add_treasure") == 0){
                char path[128];
                snprintf(path, sizeof(path), "%s/file.txt", hunt);
                add(path);
            }
            else if (strcmp(cmd, "list_treasures") == 0){
                list(path);
            }
            else if (strcmp(cmd, "view_treasure") == 0){
                view(path, id);
            }
            else if (strcmp(cmd, "remove_hunt") == 0){
                remove_hunt(hunt);
            }
            else if (strcmp(cmd, "remove_treasure") == 0){
                char path[128];
                snprintf(path, sizeof(path), "%s/file.txt", hunt);
                remove_treasure(path, id);
                printf("Treasure removed successfully.\n");
                fflush(stdout);
            }
            else if (strcmp(cmd, "stop") == 0){
                printf("Monitor: Shutting down after delay...\n");
                usleep(1000000);
                exit(0);
            }
            fflush(stdout);
            received_signal = 0;
        }
    }
}

void start_monitor(){
    if (monitor_running){
        printf("Monitor already running.\n");
        return;
    }

    if (pipe(pipefd) == -1) {
        perror("Pipe failed");
        return;
    }

    monitor_pid = fork();
    if (monitor_pid == 0){
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        monitor_loop();
        exit(0);
    } else {
        close(pipefd[1]);
        monitor_running = true;
        printf("Monitor started with PID %d\n", monitor_pid);
    }
}

void write_command_to_file(const char *command, const char *hunt, int id){
    FILE *f = fopen("hub_command.txt", "w");
    if (!f){
        perror("Could not write to command file");
        return;
    }
    fprintf(f, "%s %s %d\n", command, hunt, id);
    fflush(f);
    fsync(fileno(f));  // ensure content is written to disk before signaling
    fclose(f);
}

void read_monitor_output(){
    char buffer[1024];
    ssize_t bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("%s", buffer);
    }
}

int main(){
    struct sigaction sa;
    sa.sa_handler = handle_sigchld; // Set handler function
    sigemptyset(&sa.sa_mask);       // No additional signals blocked
    sa.sa_flags = 0;                // No special flags
    sigaction(SIGCHLD, &sa, NULL);  // Register the handler

    char input[100];
    char last_hunt[64];
    int last_id = -1;

    while (1){
        printf("\nEnter command: ");
        fflush(stdout);

        if (!fgets(input, sizeof(input), stdin)) break;
        input[strcspn(input, "\n")] = '\0';

        if (strcmp(input, "start_monitor") == 0){
            start_monitor();
        }
        else if (strcmp(input, "add_treasure") == 0){
            if (!monitor_running){ printf("Monitor not running.\n"); continue; }
            printf("Enter hunt ID: ");
            scanf("%s", last_hunt); getchar();
            write_command_to_file("add_treasure", last_hunt, -1);
            send_signal_to_monitor(SIGUSR1);
            read_monitor_output();
        }
        else if (strcmp(input, "list_hunts") == 0){
            if (!monitor_running){ printf("Monitor not running.\n"); continue; }
            printf("Enter hunt ID: ");
            scanf("%s", last_hunt); getchar();
            write_command_to_file("list_hunts", last_hunt, -1);
            send_signal_to_monitor(SIGUSR1);
            read_monitor_output();
        }
        else if (strcmp(input, "list_treasures") == 0){
            if (!monitor_running){ printf("Monitor not running.\n"); continue; }
            printf("Enter hunt ID: ");
            scanf("%s", last_hunt); getchar();
            write_command_to_file("list_treasures", last_hunt, -1);
            send_signal_to_monitor(SIGUSR2);
            read_monitor_output();
        }
        else if (strcmp(input, "view_treasure") == 0){
            if (!monitor_running){ printf("Monitor not running.\n"); continue; }
            printf("Enter hunt ID: ");
            scanf("%s", last_hunt);
            printf("Enter treasure ID: ");
            scanf("%d", &last_id); getchar();
            write_command_to_file("view_treasure", last_hunt, last_id);
            send_signal_to_monitor(SIGTERM);
            read_monitor_output();
        }
        else if (strcmp(input, "remove_hunt") == 0){
            if (!monitor_running){ printf("Monitor not running.\n"); continue; }
            printf("Enter hunt ID: ");
            scanf("%s", last_hunt); getchar();
            write_command_to_file("remove_hunt", last_hunt, -1);
            send_signal_to_monitor(SIGUSR2);
            read_monitor_output();
        }
        else if (strcmp(input, "remove_treasure") == 0){
            if (!monitor_running){ printf("Monitor not running.\n"); continue; }
            printf("Enter hunt ID: ");
            scanf("%s", last_hunt);
            printf("Enter treasure ID to remove: ");
            scanf("%d", &last_id); getchar();
            write_command_to_file("remove_treasure", last_hunt, last_id);
            send_signal_to_monitor(SIGTERM);
            read_monitor_output();
            fflush(stdout);
        }
        else if (strcmp(input, "calculate_score") == 0) {
            if (!monitor_running) {
                printf("Monitor not running.\n");
                continue;
            }
            char hunt_id[64];
            printf("Enter hunt ID: ");
            if (scanf("%63s", hunt_id) != 1) {
                printf("Invalid input.\n");
                // Clear stdin just in case
                int c; while ((c = getchar()) != '\n' && c != EOF);
                continue;
            }
            // Clear newline after scanf
            int c; while ((c = getchar()) != '\n' && c != EOF);

            int fd[2];
            if (pipe(fd) == -1) {
                perror("pipe failed");
                continue;
            }

            pid_t pid = fork();
            if (pid == 0) {
                // Child process
                close(fd[0]);
                dup2(fd[1], STDOUT_FILENO);
                close(fd[1]);
                execl("./score_calculator", "score_calculator", hunt_id, NULL);
                perror("exec failed");
                exit(1);  // exit child if exec fails
            } else if (pid > 0) {
                // Parent process
                close(fd[1]);
                FILE *stream = fdopen(fd[0], "r");
                if (!stream) {
                    perror("fdopen failed");
                    close(fd[0]);
                    // just continue or return to main loop; no exit here
                } else {
                    printf("Scores for %s:\n", hunt_id);
                    fflush(stdout);

                    char buffer[256];
                    while (fgets(buffer, sizeof(buffer), stream)) {
                        printf("%s", buffer);
                    }
                    fclose(stream);
                }
                // Wait for child to avoid zombie
                waitpid(pid, NULL, 0);

                // Now just return to your main loop — NO exit here!
            } else {
                perror("fork failed");
            }
        }
        else if (strcmp(input, "stop_monitor") == 0){
            if (!monitor_running){
                printf("Monitor is not running.\n");
                continue;
            }
            write_command_to_file("stop", "", -1);
            send_signal_to_monitor(SIGINT);

            char buffer[1024];
            ssize_t bytes_read;
            while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                printf("%s", buffer);
            }
            waitpid(monitor_pid, NULL, 0);
            monitor_running = false;
            monitor_pid = -1;
        }
        else if (strcmp(input, "exit") == 0){
            if (monitor_running){
                printf("Error: Monitor is still running. Please stop it first.\n");
            } else {
                printf("Goodbye.\n");
                break; 
            }
        }
        else {
            printf("Unknown command.\n");
        }
    }
    return 0;
}

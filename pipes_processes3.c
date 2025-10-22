#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "You need to pass an argument for grep search term\n");
        exit(1);
    }

    char *search_term = argv[1];
    int P1_to_P2[2], P2_to_P3[2];

    char *cat_args[] = {"cat", "scores", NULL};
    char *grep_args[] = {"grep", search_term, NULL};
    char *sort_args[] = {"sort", NULL};

    // Create pipe for P1 P2 communication
    if (pipe(P1_to_P2) == -1) {
        perror("pipe");
        exit(1);
    }

    // Fork P1
    pid_t pid_P1 = fork();
    if (pid_P1 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid_P1 == 0) { // P1 - executes "cat scores"
        close(P1_to_P2[0]); 
        dup2(P1_to_P2[1], STDOUT_FILENO); 
        close(P1_to_P2[1]);

        execvp("cat", cat_args);
        perror("execvp cat failed");
        exit(1);
    }

    // Create pipe for P2 -> P3 communication
    if (pipe(P2_to_P3) == -1) {
        perror("pipe");
        exit(1);
    }

    // Fork P2
    pid_t pid_P2 = fork();
    if (pid_P2 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid_P2 == 0) { 
        close(P1_to_P2[1]); 
        dup2(P1_to_P2[0], STDIN_FILENO); 
        close(P1_to_P2[0]);

        close(P2_to_P3[0]); 
        dup2(P2_to_P3[1], STDOUT_FILENO); 
        close(P2_to_P3[1]);

        execvp("grep", grep_args);
        perror("execvp grep failed");
        exit(1);
    }

    // Close unused pipe ends in the parent
    close(P1_to_P2[0]);
    close(P1_to_P2[1]);
    close(P2_to_P3[1]);

    // Fork P3
    pid_t pid_P3 = fork();
    if (pid_P3 < 0) {
        perror("fork");
        exit(1);
    }

    if (pid_P3 == 0) {
        dup2(P2_to_P3[0], STDIN_FILENO); 
        close(P2_to_P3[0]);

        execvp("sort", sort_args);
        perror("execvp sort failed");
        exit(1);
    }

    close(P2_to_P3[0]);

    // Wait for P1, P2, and P3 to complete
    waitpid(pid_P1, NULL, 0);
    waitpid(pid_P2, NULL, 0);
    waitpid(pid_P3, NULL, 0);

    return 0;
}

/*******************************************************************************
 * Name        : spfind.c
 * Author      : Siddhanth Patel
 * Date        : April 1, 2020
 * Description : A program that prints sorted file paths given a directory and permission strings.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <wait.h>
#include <stdbool.h>

bool starts_with(const char *str, const char *prefix) {
    /* TODO:
       Return true if the string starts with prefix, false otherwise.
       Note that prefix might be longer than the string itself.
     */
     if (strlen(prefix)>strlen(str)){
        return false;
     }
     else{
        for (int i = 0; i < strlen(prefix); i++){
            if (prefix[i]!=str[i]){
                return false;
            }
        }
        return true;
     }

}

int main(int argc, char *argv[]) {
    if (argc == 1){
        printf("Usage: ./pfind -d <directory> -p <permissions string> [-h]\n");
        return EXIT_SUCCESS;
    }

    int pfind_to_sort[2], sort_to_parent[2];
    pipe(pfind_to_sort);
    pipe(sort_to_parent);

    pid_t pid[2];

    if ((pid[0] = fork() == 0)) {
        // PFIND CHILD
        close(pfind_to_sort[0]);
        dup2(pfind_to_sort[1], STDOUT_FILENO);

        close(sort_to_parent[0]);
        close(sort_to_parent[1]);

        if (execv("pfind", argv) == -1) {
            fprintf(stderr, "Error: pfind failed.\n");
            return EXIT_FAILURE;
        }

    } else if (pid[0] < 0){
        fprintf(stderr, "Error: fork failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if ((pid[1] = fork()) == 0) {
        // grep
        close(pfind_to_sort[1]);
        dup2(pfind_to_sort[0], STDIN_FILENO);
        close(sort_to_parent[0]);
        dup2(sort_to_parent[1], STDOUT_FILENO);

        if (execlp("sort", "sort", NULL) == -1){
            fprintf(stderr, "Error: sort failed.\n");
            return EXIT_FAILURE;
        }
    } else if (pid[1] < 0){
        fprintf(stderr, "Error: fork failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    // IN PARENT

    close(sort_to_parent[1]);
    dup2(sort_to_parent[0], STDIN_FILENO);

    // Close all unrelated file descriptors.
    close(pfind_to_sort[0]);
    close(pfind_to_sort[1]);

    int status;
    for (int i = 0; i<2; i++){
        do {
            // Wait for the child to complete, and get the status of how it
            // terminated.
            pid_t w = waitpid(pid[0], &status, WUNTRACED | WCONTINUED);
            if (w == -1) {
                // waitpid failed.
                perror("waitpid()");
                exit(EXIT_FAILURE);
            }
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));

        if (WEXITSTATUS(status) == EXIT_FAILURE && status != 0) {
            return EXIT_FAILURE;
        }
    }

    int matches = 0;
    char buffer[4096];
    while (1) {
        ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("read()");
                exit(EXIT_FAILURE);
            }
        } else if (count == 0) {
            if (matches == 0){
                printf("Total matches: %d\n", matches);
            } else {
                matches-=2;
                printf("Total matches: %d\n", matches);
            }
            break;
        } else {
            for (int i = 0; i < sizeof(buffer); i++){
                if (buffer[i]=='\n'){
                    matches++;
                }
            }
            if (starts_with(buffer,"Usage")){
                write(STDOUT_FILENO, buffer, count);
                break;
            }
            write(STDOUT_FILENO, buffer, count);
        }
    }
    return EXIT_SUCCESS;
}

/*******************************************************************************
 * Name        : minishell.c
 * Author      : Siddhanth Patel
 * Partner     : Elijah Wendel
 * Date        : 15 April 2020
 * Description : Miniature shell script.
 * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <setjmp.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <pwd.h>
#include <limits.h>
#include <errno.h>
#include <wait.h>

#define BRIGHTBLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"
#define BUFSIZE 4096
#define MAX_ELEMENTS   2048

sigjmp_buf jmpbuf;

bool child_running = false;

bool starts_with(const char *str, const char *prefix) {
    /* TODO:
       Return true if the string starts with prefix, false otherwise.
       Note that prefix might be longer than the string itself.
     */
    if (strlen(prefix) > strlen(str)) {
      return false;
    }
    for (int i = 0; i < strlen(prefix); i++) {
      if (prefix[i] != str[i]) {
        return false;
      }
    }
    return true;
}

void changedir(char* path){
    if (!strcmp(path, "~") || !strcmp(path, "")){
        struct passwd *pw;
        if ((pw = getpwuid(getuid())) == NULL){
            fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        }
        if (chdir(pw->pw_dir) == -1){
            fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", path, strerror(errno));
        }
    } else if (starts_with(path, "~/")) {
        struct passwd *pw;
        if ((pw = getpwuid(getuid())) == NULL) {
            fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        }
        char* temp = (pw->pw_dir);
        path += 1;
        char* temp2 = (char*)calloc(1,(strlen(temp) + strlen(path) + 1) * sizeof(char));
        strcpy(temp2, temp);
        strcat(temp2, path);
        if (chdir(temp2) == -1) {
            fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", temp2, strerror(errno));
        }
        free(temp2);
    } else {
        if (chdir(path) == -1){
            fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n", path, strerror(errno));
        }
    }
}

void catch_signal(int sig) {
    if (!child_running) {
        write(STDOUT_FILENO, "\n", 1);
        siglongjmp(jmpbuf, 1);
    }
}

int main(int argc, char *argv[]){
    struct sigaction action;

    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catch_signal;
    action.sa_flags = SA_RESTART; /* Restart syscalls if possible */

    if (sigaction(SIGINT, &action, NULL) == -1) {
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    if (sigaction(SIGTERM, &action, NULL) == -1) {
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

    char buf[BUFSIZE];
    /* Saves information about the calling environment, so you can return to
       this place later, sigsetjmp returns 0 the first time. When it returns
       from siglongjmp, it returns the value supplied in siglongjump. */
    sigsetjmp(jmpbuf, 1);
    do {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
            return EXIT_FAILURE;
        }

        printf("[%s%s%s]$ ", BRIGHTBLUE, cwd, DEFAULT);
        fflush(stdout);

        ssize_t bytes_read = read(STDIN_FILENO, buf, BUFSIZE - 1);
        if (bytes_read > 0) {
            buf[bytes_read - 1] = '\0';
        }
        char *token = strtok(buf, " ");
        char *argvals[MAX_ELEMENTS];
        int num_tokens = 0;
        while(token!=NULL) {
            argvals[num_tokens] = strdup(token);
            num_tokens++;
            token = strtok(NULL, " ");
        }
        if (num_tokens == 0) {
            continue;
        } if (strcmp(argvals[0], "exit") == 0) {
            for (int j = 0; j < num_tokens; j++) {
                   free(argvals[j]);
            }
            return EXIT_SUCCESS;
        } else if (!strcmp("cd", argvals[0])) {
            if (num_tokens > 2) {
                if (argvals[1][0] == '\"' && argvals[num_tokens-1][strlen(argvals[num_tokens-1])-1] == '\"') {
                    int size_of_dir = 0;
                    for (int i = 1; i < num_tokens; i++) {
                        size_of_dir += strlen(argvals[i]);
                    }
                    char *spaced_path = (char *) calloc(1, (size_of_dir + num_tokens) * sizeof(char));
                    for (int i = 1; i < num_tokens; i++) {
                        strcat(spaced_path, argvals[i]);
                        if (i != (num_tokens-1)) {
                            strcat(spaced_path, " ");
                        }
                    }
                    spaced_path += 1;
                    spaced_path[strlen(spaced_path)-1] = '\0';
                    changedir(spaced_path);
                    spaced_path -= 1;
                    free(spaced_path);
                } else {
                    fprintf(stderr, "Error: Too many arguments to cd.\n");
                }
            } else if (num_tokens == 1) {
                changedir("~");
            } else {
                changedir(argvals[1]);
            }
        } else if (num_tokens >= 1){
            // free(argvals[num_tokens]);
            argvals[num_tokens] = NULL;
            pid_t pid = fork();
            if (pid < 0){
                fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
            } else if (pid == 0){ //child
                if (execvp(argvals[0], argvals) == -1){
                    fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
                }
                for (int j = 0; j < num_tokens; j++) {
                   free(argvals[j]);
                }
                exit(EXIT_FAILURE);
            } else { //parent
                int status;

                child_running = true;
                pid_t w = wait(&status);
                if (w < 0) {
                    // waitpid failed.
                    fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
                    for (int j = 0; j < num_tokens; j++) {
                        free(argvals[j]);
                    }
                    return EXIT_FAILURE;
                }
                child_running = false;
            }
        }
        for (int j = 0; j < num_tokens; j++) {
            free(argvals[j]);
        }
    } while (true);
}

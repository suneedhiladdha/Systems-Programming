/*******************************************************************************
 * Name        : nforks.c
 * Author      : Siddhanth Patel
 * Partner     : Elijah Wendel
 * Date        : March 13, 2020
 * Description : Working with PIDS
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

/**
 * Determines whether or not the input string represents a valid integer.
 * A valid integer has an optional minus sign, followed by a series of digits
 * [0-9].
 */
bool is_integer(char *input) {
    int start = 0, len = strlen(input);

    if (len >= 1 && input[0] == '-') {
        if (len < 2) {
            return false;
        }
        start = 1;
    }
    for (int i = start; i < len; i++) {
        if (!isdigit(input[i])) {
            return false;
        }
    }
    return true;
}

/**
 * Takes as input a string and an in-out parameter value.
 * If the string can be parsed, the integer value is assigned to the value
 * parameter and true is returned.
 * Otherwise, false is returned and the best attempt to modify the value
 * parameter is made.
 */
bool get_integer(char *input, int *value) {
    long long long_long_i;
    if (sscanf(input, "%lld", &long_long_i) != 1) {
        return false;
    }
    *value = (int)long_long_i;
    if (long_long_i != (long long)*value) {
        fprintf(stderr, "Warning: Integer overflow with '%s'.\n", input);
        return false;
    }
    return true;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num forks>\n", argv[0]);
        return EXIT_FAILURE;
    }
    int num_forks = 0;
    if (!is_integer(argv[1]) || !get_integer(argv[1], &num_forks) ||
        num_forks <= 0) {
        fprintf(stderr, "Error: Invalid number of forks '%s'.\n", argv[1]);
        return EXIT_FAILURE;
    }
    pid_t forks[num_forks];
    for (int i; i < num_forks; i++){
        if ((forks[i] = fork()) < 0) {
            fprintf(stderr, "Error: fork[%d] failed. %s.\n", i, strerror(errno));
            return EXIT_FAILURE;
        }
        if (forks[i] < 0) {
            fprintf(stderr, "An error occured!\n");
        } else if (forks[i] == 0) {
            if (execl("randomsleep", "", NULL) == -1) {
                fprintf(stderr, "Error: execl() failed. %s.\n", strerror(errno));
                return EXIT_FAILURE;
            }
        }
    }
    for (int i = 0; i < num_forks; i++){
        if (wait(NULL) == -1){
            fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
        }
        printf("Child with pid %d has terminated.\n", forks[i]);
    }

    printf("All children are done sleeping.\n");
    return EXIT_SUCCESS;
}

/*******************************************************************************
 * Name        : mtsieve.c
 * Author      : Siddhanth Patel
 * Date        : 27 April 2020
 * Description : Working with threads to find primes with two or more 3 digits.
 * Pledge : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <ctype.h>
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
#include <getopt.h>
#include <float.h>
#include <sys/sysinfo.h>
#include <math.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

#define MAXINPUT 8

int total_count = 0;
pthread_mutex_t lock;

typedef struct arg_struct {
    int start;
    int end;
} thread_args;


bool is_integer(char *input) {
    int initial = 0, len = strlen(input);

    if (len >= 1 && input[0] == '-') {
        if (len < 2) {
            return false;
        }
        initial = 1;
    }
    for (int i = initial; i < len; i++) {
        if (!isdigit(input[i])) {
            return false;
        }
    }
    return true;
}

bool get_integer(char *input, int *value) {
    long long long_long_i;
    if (sscanf(input, "%lld", &long_long_i) != 1) {
        return false;
    }
    *value = (int)long_long_i;
    if (long_long_i != (long long)*value) {
        //fprintf(stderr, "Warning: Integer overflow with '%s'.\n", input);
        return false;
    }
    return true;
}

bool has_secthree(int prime){
    int holder = 0;
    while (prime > 0){
        holder = prime % 10;
        prime = prime / 10;
        if (holder == 3){
            return true;
        }
    }
    return false;
}

bool has_three(int prime){
    int holder = 0;
    while (prime > 0){
        holder = prime % 10;
        prime = prime / 10;
        if (holder == 3 && has_secthree(prime)){
            return true;
        }
    }
    return false;
}

bool error_free(char *parameter, char *integer){
    if (integer==NULL){
        printf("Error: Option -%s requires an argument.\n", parameter);
        return false;
    }
    if (!is_integer(integer)){
        printf("Error: Invalid input '%s' received for parameter '-%s'.\n", integer, parameter);
        return false;
    }
    int val;
    if (!get_integer(integer, &val)){
        printf("Error: Integer overflow for parameter '-%s'.\n", parameter);
        return false;
    }
    return true;
}

bool val_error_free(bool sFlag, bool eFlag, bool tFlag, int sInt, int eInt, int tInt, char *nextval){
    if (nextval != NULL){
        printf("Error: Non-option argument '%s' supplied.\n", nextval);
        return false;
    }
    if (!sFlag){
        printf("Error: Required argument <starting value> is missing.\n");
        return false;
    }
    if (sInt < 2){
        printf("Error: Starting value must be >= 2.\n");
        return false;
    }
    if (!eFlag){
        printf("Error: Required argument <ending value> is missing.\n");
        return false;
    }
    if (eInt < 2){
        printf("Error: Ending value must be >= 2.\n");
        return false;
    }
    if (eInt < sInt){
        printf("Error: Ending value must be >= starting value.\n");
        return false;
    }
    if (!tFlag){
        printf("Error: Required argument <num threads> is missing.\n");
        return false;
    }
    if (tInt < 1){
        printf("Error: Number of threads cannot be less than 1.\n");
        return false;
    }
    if (tInt > get_nprocs()){
        printf("Error: Number of threads cannot exceed twice the number of processors(%d).\n", get_nprocs());
        return false;
    }
    return true;
}

void *segmented_sieve(void *ptr){
    thread_args* targs = (thread_args *)ptr;
    int sub_count = 0;
    int start = targs->start;
    int end = targs->end;
    int sqrt_limit = (int)sqrtf(end);

    //CREATE LOW PRIMES
    bool *low_primes = (bool *) malloc (sizeof (bool) * (sqrt_limit + 1));

    for (int i = 0; i <= sqrt_limit; i++){
        low_primes[i] = true;
    }

    for (int p = 2; p*p < sqrt_limit; p++){
        if (low_primes[p] == true){
            for (int j = p*p; j <= sqrt_limit; j+=p){
                low_primes[j] = false;
            }
        }
    }

    // CREATE HIGH PRIMES
    int length_high = end - start + 1;
    bool *high_primes = (bool *) malloc (sizeof (bool) * length_high + 1);

    for (int i = 0; i <= length_high; i++){
        high_primes[i] = true;
    }

    for (int p = 2; p < sqrt_limit; p++){
        if (low_primes[p]){
            int i = (int)ceil((double)start/p) * p - start;
            if (start <= p){
                i += p;
            }
            while (i < length_high){
                high_primes[i] = false;
                i += p;
            }
        }
    }

    for (int i = 2; i <= length_high; i++){
        if (high_primes[i] == true){
            if (i + start > 100 && has_three(i + start)){
                sub_count++;
            }
        }
    }

    int retval;
    if ((retval = pthread_mutex_lock(&lock)) != 0) {
        fprintf(stderr, "Warning: Cannot lock mutex. %s.\n", strerror(retval));
    }
    total_count += sub_count;
    if ((retval = pthread_mutex_unlock(&lock)) != 0) {
        fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n", strerror(retval));
    }
    free(low_primes);
    free(high_primes);
    pthread_exit(NULL);
}


int main(int argc, char **argv) {

    if (argc == 1){
        printf("Usage: ./mtsieve -s <starting value> -e <ending value> -t <num threads>");
        return EXIT_FAILURE;
    }

    bool sFlag, eFlag, tFlag = false;
    int sInt, eInt, tInt, count = 0;
    char *parameter = "s";
    int opt;

    while ((opt = getopt(argc, argv, ":s:e:t:")) != -1) {
        switch (opt) {
            case 's':
                parameter = "s";
                sFlag = true;
                if (!error_free(parameter, optarg)){
                    return EXIT_FAILURE;
                }
                sInt = atoi(optarg);
                count = optind;
                break;
            case 'e':
                parameter = "e";
                eFlag = true;
                if (!error_free(parameter, optarg)){
                    return EXIT_FAILURE;
                }
                eInt = atoi(optarg);
                count = optind;
                break;
            case 't':
                parameter = "t";
                tFlag = true;
                if (!error_free(parameter, optarg)){
                    return EXIT_FAILURE;
                }
                tInt = atoi(optarg);
                count = optind;
                break;
            case '?':
                if (optopt == 'e' || optopt == 's' || optopt == 't') {
                    fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Error: Unknown option character '\\x%x'.\n", optopt);
                }
                return EXIT_FAILURE;
            case ':':
                if (optopt == 'e' || optopt == 's' || optopt == 't') {
                    fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt);
                } else if (isprint(optopt)) {
                    fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
                } else {
                    fprintf(stderr, "Error: Unknown option character '\\x%x'.\n", optopt);
                }
                return EXIT_FAILURE;
        }
    }

    if (!val_error_free(sFlag, eFlag, tFlag, sInt, eInt, tInt, argv[count])){
        return EXIT_FAILURE;
    }

    // CREATES SEGMENTS HERE
    int num_threads = tInt;
    int count_test = eInt - sInt + 1;
    int remainder = 0;

    if (num_threads > count_test){
        num_threads = count_test;
    } else {
        remainder = count_test % num_threads;
        count_test = count_test / num_threads;
    }
    printf("Finding all prime numbers between %d and %d.\n", sInt, eInt);

    if (num_threads > 1){
        printf("%d segments:\n", num_threads);
    } else {
        printf("%d segment:\n", num_threads);
    }

    int start_var = sInt;
    int end_var = 0;
    remainder ++;
    // PERFORMS ALGORITHM HERE BY CREATING THREADS
    int retval;
    if ((retval = pthread_mutex_init(&lock, NULL)) != 0) {
        fprintf(stderr, "Error: Cannot create mutex. %s.\n", strerror(retval));
        return EXIT_FAILURE;
    }

    pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
    thread_args *targs = (thread_args *)malloc(num_threads * sizeof(thread_args));

    for (int i = 0; i < num_threads; i++) {
        targs[i].start = start_var;
        end_var = start_var + count_test - 1;
        if (remainder != 0){
            remainder--;
            end_var++;
        }
        printf("   [%d, %d]\n", start_var, end_var);
        targs[i].end = end_var;
        start_var = end_var + 1;

        if ((retval = pthread_create(&threads[i], NULL, segmented_sieve, (void *)(&targs[i]))) != 0) {
            fprintf(stderr, "Error: Cannot create thread %d. %s.\n", i + 1, strerror(retval));
            break;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Warning: Thread %d did not join properly.\n", i + 1);
        }
    }

    if ((retval = pthread_mutex_destroy(&lock)) != 0) {
        fprintf(stderr, "Error: Cannot destroy mutex. %s.\n", strerror(retval));
        return EXIT_FAILURE;
    }

    printf("Total primes between %d and %d with two or more '3' digits: %d\n", sInt, eInt, total_count);
    free(threads);
    free(targs);
    return EXIT_SUCCESS;
}

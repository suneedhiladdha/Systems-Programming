/*******************************************************************************
 * Name        : sort.c
 * Author      : Siddhanth Patel
 * Date        : February 21, 2020
 * Description : Uses quicksort to sort a file of either ints, doubles, or
 *               strings.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <float.h>
#include "quicksort.h"

#define MAX_STRLEN     64 // Not including '\0'
#define MAX_ELEMENTS 1024

typedef enum {
    STRING,
    INT,
    DOUBLE
} elem_t;

/**
 * Basic structure of sort.c:
 *
 * Parses args with getopt.
 * Opens input file for reading.
 * Allocates space in a char** for at least MAX_ELEMENTS strings to be stored,
 * where MAX_ELEMENTS is 1024.
 * Reads in the file
 * - For each line, allocates space in each index of the char** to store the
 *   line.
 * Closes the file, after reading in all the lines.
 * Calls quicksort based on type (int, double, string) supplied on the command
 * line.
 * Frees all data.
 * Ensures there are no memory leaks with valgrind.
 */

bool fileexists(char *fileone, char *filetype){
    char *stringtype = "string";
    char *inttype = "int";
    char *doubletype = "double";
    char *buf[MAX_ELEMENTS];
    for (int i = 0; i < MAX_ELEMENTS; i++) {
        buf[i] = (char*)calloc(MAX_STRLEN + 2, sizeof(char));
    }
    char *strarray[MAX_STRLEN];
    int numarray[MAX_STRLEN];
    double doubarray[MAX_STRLEN];

    FILE *file = fopen(fileone, "r");
    if (file == NULL){
        printf("Error: Cannot open '%s'. %s.\n", fileone, strerror(errno));
        return EXIT_FAILURE;
    }
    int count = 0;
    while (fgets(buf[count], MAX_STRLEN + 2, file)) {
        char *eoln = strchr(buf[count], '\n');
        if (eoln == NULL) {
            buf[count][MAX_STRLEN] = '\0';
        } else {
            *eoln = '\0';
        }
        count++;
    }
    fclose(file);
    if (filetype==stringtype){
        for(int i = 0; i < count; i++){
            strarray[i] = buf[i];
        }
        quicksort(strarray, count, sizeof(char*), str_cmp);
        for (int i=0; i<count; i++){
            printf("%s\n", strarray[i]);
        }
    }
    if (filetype==inttype){
        for(int i = 0; i < count; i++){
            numarray[i] = atoi(buf[i]);
        }
        quicksort(&numarray, count, sizeof(int), int_cmp);
        for (int i=0; i<count; i++){
            printf("%d\n", numarray[i]);
        }
    }
    if (filetype==doubletype){
        for(int i = 0; i < count; i++){
            doubarray[i] = atof(buf[i]);
        }
        quicksort(&doubarray, count, sizeof(double), dbl_cmp);
        for (int i=0; i<count; i++){
            printf("%f\n", doubarray[i]);
        }
    }
    for(int i = 0; i < MAX_ELEMENTS; i++) {
        free(buf[i]);
    }
    return EXIT_SUCCESS;
}

void usage(){
    printf("Usage: ./sort [-i|-d] [filename]\n");
    printf("   -i: Specifies the file contains ints.\n");
    printf("   -d: Specifies the file contains doubles.\n");
    printf("   filename: The file to sort.\n");
    printf("   No flags defaults to sorting strings.\n");
}

int main(int argc, char **argv) {
    if (argc == 1){
        usage();
        return EXIT_FAILURE;
    } else if (argc ==2 && argv[1][0]!='-'){
        fileexists(argv[1], "string");
    } else {
        int opt;
        while ((opt = getopt(argc, argv, ":i :d")) != -1) {
            switch (opt) {
            case 'i':
                if (argc ==2) {
                    usage();
                } else {
                    fileexists(argv[2], "int");
                }
                break;
            case 'd':
                if (argc ==2) {
                    usage();
                } else {
                    fileexists(argv[2], "double");
                }
                break;
            case '?':
                fprintf(stderr, "Error: Unknown option '%s' received.\n", argv[1]);
                usage();
                return EXIT_FAILURE;
            }
        }
    }
    return EXIT_SUCCESS;
}

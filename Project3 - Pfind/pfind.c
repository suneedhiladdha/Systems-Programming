/*******************************************************************************
 * Name        : pfind.c
 * Author      : Siddhanth Patel
 * Date        : March 8, 2020
 * Description : A program that finds and prints files with a specific set of permissions.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/stat.h>
#include "pfunctions.h"

#define BUFSIZE 128

int main(int argc, char *argv[]){
    int count = 1;
    bool help = false, foundp = false, foundd = false, unknown = false;
    char *dfile, *pmission, *unknownopt;
    int opt;
    if (argc == 1){
        display_usage();
        return EXIT_FAILURE;
    } else {
        while ((opt = getopt(argc, argv, ":h d p")) != -1) {
            switch (opt) {
            case 'h':
                help = true;
                break;
            case 'd':
                count++;
                foundd = true;
                if (foundp){
                    dfile = argv[count+1];
                } else {
                    dfile = argv[count];
                }
                break;
            case 'p':
                count++;
                foundp = true;
                if (foundd){
                    pmission = argv[count+1];
                } else {
                    pmission = argv[count];
                }
                break;
            case '?':
                unknownopt = argv[count];
                unknown = true;
            }
        }
        if (unknown){
            if (argc == 5){
                pmission = argv[3];
            } else {
                fprintf(stderr, "Error: Unknown option '%s' received.\n", unknownopt);
                return EXIT_FAILURE;
            }
        }
        if (help){
            display_usage();
            return EXIT_SUCCESS;
        }
        if (!foundd && !foundp){
            display_usage();
            return EXIT_FAILURE;
        } else if (!foundd && foundp){
            fprintf(stderr, "Error: Required argument -d <directory> not found.\n");
            return EXIT_FAILURE;
        } else if (!foundp && foundd){
            fprintf(stderr, "Error: Required argument -p <permissions string> not found.\n");
            return EXIT_FAILURE;
        } else {
            if (fullpath(dfile, pmission, 0)!=2){
                return EXIT_SUCCESS;
            }
        }
    }
    return EXIT_SUCCESS;
}

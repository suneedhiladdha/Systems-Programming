/*******************************************************************************
 * Name        : pfunctions.c
 * Author      : Siddhanth Patel
 * Date        : March 8, 2020
 * Description : Helper functions for the program that finds and prints files with a specific set of permissions.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <dirent.h>
#include "pfunctions.h"

#define BUFSIZE 128

int count = 0;
int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR,
               S_IRGRP, S_IWGRP, S_IXGRP,
               S_IROTH, S_IWOTH, S_IXOTH};

/**
 * Returns a string (pointer to char array) containing the permissions of the
 * file referenced in statbuf.
 * Allocates enough space on the heap to hold the 10 printable characters.
 * The first char is always a - (dash), since all files must be regular files.
 * The remaining 9 characters represent the permissions of user (owner), group,
 * and others: r, w, x, or -.
 */

void display_usage(){
    printf("Usage: ./pfind -d <directory> -p <permissions string> [-h]\n");
}

char* permission_string(struct stat *statbuf) {
    int permission_valid;
    char* final = (char*) malloc(sizeof(char) * 10);
    //final[0] = '-'; // this is to print the file type bit, as displayed in `$ ls -l`.
    for (int i = 0; i < 8; i += 3) {
        permission_valid = statbuf->st_mode & perms[i];
        if (permission_valid) {
            final[i] = 'r';
        } else {
            final[i] = '-';
        }
        permission_valid = statbuf->st_mode & perms[i+1];
        if (permission_valid) {
            final[i+1] = 'w';
        } else {
            final[i+1] = '-';
        }
        permission_valid = statbuf->st_mode & perms[i+2];
        if (permission_valid) {
            final[i+2] = 'x';
        } else {
            final[i+2] = '-';
        }
    }
    return final;
}

bool verifypmission(char *pmission, size_t length){
    bool trueperm = true;
    if (length != 9){
        return false;
    } else {
        for (int i = 0; i < 8; i += 3) {
            if (pmission[i] != 'r' && pmission[i] != '-'){
                trueperm = false;
            }
            if (pmission[i+1] != 'w' && pmission[i+1] != '-'){
                trueperm = false;
            }
            if (pmission[i+2] != 'x' && pmission[i+2] != '-'){
                trueperm = false;
            }
        }
        return trueperm;
    }
}

bool matchstring(char *perms, char *pmission){
    for (int i = 0; i <= 8; i += 1) {
        if (pmission[i] != perms[i]){
            return false;
        }
    }
    return true;
}

bool unsafedirectory(char *perms){
    bool notsafe = true;
    for (int i = 0; i <= 8; i += 1) {
        notsafe = notsafe && (perms[i] == '-');
    }
    return notsafe;
}

int fullpath(char *filename, char *pmission, int count) {
    char path[PATH_MAX];
    if (realpath(filename, path) == NULL) {
        fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", filename, strerror(errno));
        return EXIT_FAILURE;
    }

    if (!verifypmission(pmission, strlen(pmission))){ // VERIFIES IF PERMISSION INPUT IS VALID
        fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", pmission);
        return EXIT_FAILURE;
    }

    DIR *dir;
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "Error: Cannot open directory '%s'. %s\n",
                path, strerror(errno));
        return EXIT_FAILURE;
    }

    struct dirent *entry;
    struct stat sb;
    char full_filename[PATH_MAX];
    size_t pathlen = 0;

    // Set the initial character to the NULL byte.
    // If the path is root '/', you can now take the strlen of a properly
    // terminated empty string.
    full_filename[0] = '\0';
    if (strcmp(path, "/")) {
        // If path is not the root - '/', then ...

        // If there is no NULL byte among the first n bytes of path,
        // the full_filename will not be terminated. So, copy up to and
        // including PATH_MAX characters.
        strncpy(full_filename, path, PATH_MAX);
    }
    // Add + 1 for the trailing '/' that we're going to add.
    pathlen = strlen(full_filename) + 1;
    full_filename[pathlen - 1] = '/';
    full_filename[pathlen] = '\0';

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        strncpy(full_filename + pathlen, entry->d_name, PATH_MAX - pathlen);
        if (lstat(full_filename, &sb) < 0) {
            fprintf(stderr, "Error: Cannot stat file '%s'. %s\n",
                    full_filename, strerror(errno));
            continue;
        }

        struct stat statbuf;
            if (stat(full_filename, &statbuf) < 0) { return EXIT_FAILURE;} // LOAD EACH FILENAME INTO STATBUF
            char *perms = permission_string(&statbuf); // LOAD PERMISSION STRING OF EACH FILENAME INTO PERMS

        bool geterror = false;
        if (entry->d_type == DT_DIR) {
            if (matchstring(perms, pmission)){ // IF PERMS EQUALS PERMISSION INPUT THEN PRINT THE FILE PATH
                printf("%s\n", full_filename);
                count = 2;
            }
            if (unsafedirectory(perms)){
                fprintf(stderr, "Error: Cannot open directory '%s'. Permission denied.\n", full_filename);
                geterror = true;
            }
            if (!geterror){
                if (count == 2){
                    count = fullpath(full_filename, pmission, 2);
                } else {
                    count = fullpath(full_filename, pmission, 0);
                }
            }
        } else {
            if (matchstring(perms, pmission)){ // IF PERMS EQUALS PERMISSION INPUT THEN PRINT THE FILE PATH
                printf("%s\n", full_filename);
                count = 2;
            }
        }
        free(perms);
    }
    closedir(dir);
    return count;
}

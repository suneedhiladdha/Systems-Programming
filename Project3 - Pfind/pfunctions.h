/*******************************************************************************
 * Name        : pfunctions.h
 * Author      : Siddhanth Patel
 * Date        : March 8, 2020
 * Description : List of functions for the program that finds and prints files with a specific set of permissions.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#ifndef PFUNCTION_H_
#define PFUNCTIONS_H_

/* Function prototype */
void display_usage();
char* permission_string(struct stat *statbuf);
bool verifypmission(char *pmission, size_t length);
bool matchstring(char *perms, char *pmission);
bool unsafedirectory(char *perms);
int fullpath(char *filename, char *pmission, int count);

#endif // PFUNCTIONS_H_

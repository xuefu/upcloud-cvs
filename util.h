#ifndef _STAGE_H_
#define _STAGE_H_

#include "util.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>

#define PATH_LEN 1024
#define NAME_LEN 32
#define	MAX_PASS_LEN 16		/* max #chars for user to enter */

/* get the user name from the config file meta */
void get_user_name(char *user, int len);

/* get the bucket name from the config file meta */
void get_bucket_name(char *bucket, int len);

int exist_upc_dir(const char *path);

/* get the path of backup */
void get_path_of_back(char *path);

/* get the password from the stdin and */
char *getpass(const char *prompt);

/* get the path and md5 : return 0 if no md5 , return 1 if md5 */
int get_path_md5(char *from_str, char *path, char *md5);

/* cd to the parent root path */
void change_dir();

#endif

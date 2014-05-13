#ifndef _STAGE_H_
#define _STAGE_H_

#include "util.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <dirent.h>

#define PATH_LEN 1024

/* get the user name from the config file meta */
void get_user_name(char *user, int len);

/* get the bucket name from the config file meta */
void get_bucket_name(char *bucket, int len);

int exist_upc_dir(const char *path);

/* get the path of backup */
void get_path_of_back(char *path);

#endif

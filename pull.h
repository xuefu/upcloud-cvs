#ifndef _PULL_H
#define _PULL_H

#include <upyun.h>
#include <stdlib.h>

#define PATH_LEN 1024

extern upyun_t *thiz;

typedef struct tree_file_s
{
  char *path;
  time_t date;
  char type;
  long size;
  int nchild;

  struct tree_file_s **child;
  struct tree_file_s *parent;
} tree_file_t;

/* initial the local bucket */
int init_local_bucket(const char *user_bucket_info, char *user, char *bucket);

/* read all files of the bucket and construct the general tree */
void bucket_readdir(tree_file_t *tft, const char *prefix);

/* download all the file  according the general tree */
void pull_bucket(tree_file_t *tft);

#endif

#ifndef _PULL_H
#define _PULL_H

#include <upyun.h>
#include <stdlib.h>

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

void bucket_readdir(tree_file_t *tft, const char *prefix);
void pull_bucket(tree_file_t *tft);

#endif

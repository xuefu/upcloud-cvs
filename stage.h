#ifndef _UTIL_H
#define _UTIL_H

#include "pull.h"
#include "util.h"

#define PATH_LEN 1024

void save_origin_tree(tree_file_t *tft, char *bucket);
void push_origin_tree(tree_file_t *tft);
void save_stage_tree(tree_file_t *tft);
void local_readdir(tree_file_t *tft, const char *dir, const char *path_of_back);
void current_changed_file();
void add_dir_to_added(tree_file_t *tft);
void add_file_to_added(const char *file);
void show_status();

#endif

#ifndef _UTIL_H
#define _UTIL_H

#include "pull.h"


int save_user_bucket_info(const char *user_bucket_info, char *user, char *bucket);
void save_origin_tree(tree_file_t *tft);
void push_origin_tree(tree_file_t *tft);
void set_path_of_back();
void get_user_name(char *user);
void get_bucket_name(char *bucket);
void save_stage_tree(tree_file_t *tft);
void general_readdir(tree_file_t *tft, const char *dir);

#endif

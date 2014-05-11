#ifndef _UTIL_H
#define _UTIL_H

#include "pull.h"


int init_local_bucket(const char *user_bucket_info, char *user, char *bucket);
void save_origin_tree(tree_file_t *tft, char *bucket);
void push_origin_tree(tree_file_t *tft);
void set_path_of_back();
void get_user_name(char *user, int len);
void get_bucket_name(char *bucket, int len);
void save_stage_tree(tree_file_t *tft);
void local_readdir(tree_file_t *tft, const char *dir);
void current_changed_file();
void add_file_to_stage(const char *file);
void show_status();


#endif

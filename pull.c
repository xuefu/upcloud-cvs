/*
 * ===================================================================================
 *
 *      Filename: pull.c
 *
 *   Description: 下载云空间文件
 *
 *       Version: 1.0
 *       Created: 2014/05/28 20:02:18
 *      Revision: none
 *      Compiler: gcc
 *  
 *        Author: xuefu
 *  Organization: none
 *
 * ===================================================================================
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pull.h"


/* parse the name of user and bucket from the argv like 'bucket_name@user_name' */
static int parse_user_bucket(const char *user_bucket_info, char *user, char *bucket)
{
  char *user_name;
  char *temp = malloc(strlen(user_bucket_info) + 1);
  strcpy(temp, user_bucket_info);

  if((user_name=strstr(temp, "@")) != NULL)
  {
    *user_name++ = '\0';
    strcpy(bucket, temp);
    strcpy(user, user_name);
    free(temp);
    return 0;
  }

  printf("usage: upc clone bucket@user.\n");
  free(temp);
  return -1;
}

/* initial the local bucket */
int init_local_bucket(const char *user_bucket_info, char *user, char *bucket)
{
  FILE *fp;
  char path[PATH_LEN];
  char temp_path[PATH_LEN];

  if(parse_user_bucket(user_bucket_info, user, bucket))
    return -1;

  strcpy(path, bucket);
  if(mkdir(path, 0777) < 0)
  {
    perror("mkdir error: ");
    return -1;
  }

  /* make configuration directory of local bucket */
  strcat(path, "/.upc");
  if(mkdir(path, 0777) < 0)
  {
    perror("mkdir error: ");
    return -1;
  }

  strcpy(temp_path, path);
  strcat(temp_path, "/added");
  fp = fopen(temp_path, "w");
  fclose(fp);

  strcpy(temp_path, path);
  strcat(temp_path, "/stage");
  fp = fopen(temp_path, "w");
  fclose(fp);

  strcpy(temp_path, path);
  strcat(temp_path, "/removed");
  fp = fopen(temp_path, "w");
  fclose(fp);

  strcpy(temp_path, path);
  strcat(temp_path, "/modified");
  fp = fopen(temp_path, "w");
  fclose(fp);

  /* save the bucket_name@user_name to the file meta */
  strcat(path, "/meta");
  fp = fopen(path, "w");
  fwrite(user_bucket_info, sizeof(char), strlen(user_bucket_info), fp);
  fclose(fp);

  return 0;
}

/* get the number of all files below one directory */
static int bucket_dir_len(upyun_dir_item_t *item)
{
  int child;

  child = 0;
  for(; item != NULL; item=item->next)
  {
    child++;
  }

  return child;
}

/* read all files of the bucket and construct the general tree */
int bucket_readdir(tree_file_t *tft, const char *prefix)
{
  int nchild; /* the tft has n childs */

  int status;
  upyun_dir_item_t *item = NULL;
  upyun_ret_e ret = upyun_read_dir(thiz, prefix, &item, &status);

  if(ret != UPYUN_RET_OK || (status != 200 && status == 401))
  {
    printf("Unauthorized...\n");
    return -1;
  }

  nchild = bucket_dir_len(item);
  tft->path = calloc(1, strlen(prefix)+1);
  strcpy(tft->path, prefix);
  tft->type = 'D';
  tft->nchild = nchild;
  tft->child = calloc(nchild, sizeof(tree_file_t *));

  upyun_dir_item_t *s_item; 
  s_item = item;

  nchild = 0;
  for(; item!=NULL; item=item->next)
  {
    if(item->file_type==1) /* item is directory */
    {
      tree_file_t *new_dir = calloc(1, sizeof(tree_file_t));
      new_dir->md5 = NULL;
      tft->child[nchild] = new_dir;

      char *new_prefix;
      new_prefix = calloc(1, strlen(prefix)+strlen(item->file_name)+2);
      sprintf(new_prefix, "%s%s/", prefix, item->file_name);
      new_dir->parent = tft;

      /* recursive calls */
      bucket_readdir(new_dir, new_prefix);        
      free(new_prefix);
    } else { /* item is file */
      tree_file_t *new_file = calloc(1, sizeof(tree_file_t));
      char *new_prefix = calloc(1, strlen(prefix)+strlen(item->file_name)+1);
      sprintf(new_prefix, "%s%s", prefix, item->file_name);
      new_file->path = new_prefix;
      new_file->type = 'F';
      new_file->md5 = NULL;
      new_file->nchild = 0;
      new_file->child = 0;
      new_file->parent = tft;

      tft->child[nchild] = new_file;
    }
    nchild++;
  }
  upyun_dir_items_free(s_item);
  return 0;
}

static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

/* download all the file  according the general tree */
void pull_bucket(tree_file_t *tft)
{
  int nchild, i;
  char path_download[4096] = {0};
  upyun_ret_e ret = UPYUN_RET_OK;
  int status = 0;
  nchild = tft->nchild;

  for(i =0; i<nchild; i++)
  {
    if(tft->child[i]->type == 'F') /* file */
    {
      sprintf(path_download, "%s", tft->child[i]->path + 1);
      FILE *fp;
      /* make file and open it */
      if ((fp = fopen(path_download, "w")) == NULL )
      {
        printf("open file %s error\n", path_download );
        return;
      }
      printf("downloading file %s...\n", path_download);
      ret = upyun_download_file(thiz, tft->child[i]->path, 
          (UPYUN_CONTENT_CALLBACK)write_data,
          (void *) fp,
          &status);
      fclose(fp);
      if(ret != UPYUN_RET_OK || status != 200)
      {
        printf("upyun_download_file error: %d\n", ret);
        printf("status: %d\n", status);
      }
      printf("completed.\n");
    } else  { /* directory */
      sprintf(path_download, "%s", (tft->child[i]->path)+1);
      /* check whether the directory exists */
      if(access(path_download, F_OK) <  0)
      {
        mkdir(path_download, 0777);
      }
      /* recursive calls */ 
      pull_bucket(tft->child[i]);
    }
  }
}


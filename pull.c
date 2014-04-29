#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "pull.h"

int bucket_dir_len(upyun_dir_item_t *item)
{
  int child;

  child = 0;

  for(; item != NULL; item=item->next)
  {
    child++;
  }

  return child;
}

void bucket_readdir(tree_file_t *tft, const char *prefix)
{
  int nchild; // the tft has n childs

  int status;
  upyun_dir_item_t *item = NULL;
  upyun_ret_e ret = upyun_read_dir(thiz, prefix, &item, &status);
  if(ret != UPYUN_RET_OK || status != 200)
  {
    printf("upyun_read_dir %s error: %d\n", prefix, ret);
    printf("status: %d\n", status);
  }

  nchild = bucket_dir_len(item);
  tft->path = calloc(1, strlen(prefix)+1);
  strcpy(tft->path, prefix);
  tft->type = 'D';
  tft->nchild = nchild;
  tft->child = calloc(nchild, sizeof(tree_file_t *));

  upyun_dir_item_t *s_item = item;

  nchild = 0;
  for(; item!=NULL; item=item->next)
  {
    // item is directory
    if(item->file_type==1)
    {
      tree_file_t *new_dir = calloc(1, sizeof(tree_file_t));
      new_dir->date = item->date;
      new_dir->size = item->file_size;
      tft->child[nchild] = new_dir;

      char *new_prefix = calloc(1, strlen(prefix)+strlen(item->file_name)+2);
      sprintf(new_prefix, "%s%s/", prefix, item->file_name);
      new_dir->parent = tft;

      bucket_readdir(new_dir, new_prefix);
      free(new_prefix);
    } else { // item is file
      tree_file_t *new_file = calloc(1, sizeof(tree_file_t));
      char *new_prefix = calloc(1, strlen(prefix)+strlen(item->file_name)+1);
      //new_file->path = calloc(1, strlen(prefix)+strlen(item->file_name)+1);
      sprintf(new_prefix, "%s%s", prefix, item->file_name);
      new_file->path = new_prefix;
      new_file->date = item->date;
      new_file->type = 'F';
      new_file->size = item->file_size;
      new_file->nchild = 0;
      new_file->child = 0;
      new_file->parent = tft;

      tft->child[nchild] = new_file;
    }
  nchild++;
  }
  upyun_dir_items_free(s_item);
}

size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
  int written = fwrite(ptr, size, nmemb, (FILE *)stream);
  return written;
}

void pull_bucket(tree_file_t *tft)
{
  int nchild, i;
  char path_download[4096] = {0};
  upyun_ret_e ret = UPYUN_RET_OK;
  int status = 0;
  nchild = tft->nchild;

  for(i =0; i<nchild; i++)
  {
    if(tft->child[i]->type == 'F')
    {
      sprintf(path_download, "%s", ++(tft->child[i]->path));
      FILE *fp = NULL;
      if ((fp = fopen(path_download, "w")) == NULL )
      {
        printf("open file %s error\n", path_download );

        ret = upyun_download_file(thiz, tft->child[i]->path, 
            (UPYUN_CONTENT_CALLBACK)write_data,
            (void *) fp,
            &status);
        fclose(fp);
        if(ret != UPYUN_RET_OK || status != 200)
        {
          printf("upyun_read_dir error: %d\n", ret);
          printf("status: %d\n", status);
        }
      }
    } else  {
      sprintf(path_download, "%s", ++(tft->child[i]->path));
      if(access(path_download, F_OK) <  0)
      {
        mkdir(path_download, 0777);
      }
      pull_bucket(tft->child[i]);
    }
  }
}

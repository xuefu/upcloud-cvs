#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <upyun.h>

#include "push.h"
#include "util.h"

/* remotely remove the file tagged with removed in the stage */
void handle_removed_file()
{
  FILE *fp;
  char removed_path[PATH_LEN] = ".upc";
  char temp[PATH_LEN];
  int len;
  int status;
  upyun_ret_e ret = UPYUN_RET_OK;

  while(access(removed_path, F_OK) < 0)
  {
    strcpy(temp, removed_path);
    sprintf(removed_path, "../%s", temp);
  }
  strcat(removed_path, "/removed");
  fp = fopen(removed_path, "r");

  while(fgets(temp, PATH_LEN, fp) != NULL)
  {
    get_path_md5(temp, temp, NULL);
    len = strlen(temp);
    if(temp[len-1] != '\n')// delete all the file
    {
      ret = upyun_get_fileinfo(thiz, temp, NULL, &status);
      if(status == 404)
      {
        continue;
      }

      printf("removing file %s ...\n", temp);
      ret = upyun_remove_file(thiz, temp, &status);

      if(ret != UPYUN_RET_OK || status != 200)
      {
        printf("\nupyun_remove_file %s error: %d\n", temp, ret);
        printf("status: %d\n", status);
      }
      printf("completed.\n");
    }
  }

  rewind(fp);
  while(fgets(temp, PATH_LEN, fp) != NULL)
  {
    get_path_md5(temp, temp, NULL);
    len = strlen(temp);
    if(temp[len-1] == '\n') // delete all the directory
    {
      temp[len-1] = '\0';
      printf("removing directory %s ...\n", temp);
      ret = upyun_remove_file(thiz, temp, &status);

      if(ret != UPYUN_RET_OK || status != 200)
      {
        printf("\nupyun_remove_file %s error: %d\n", temp, ret);
        printf("status: %d\n", status);
      }
      printf("completed.\n");
    }
  }
  fclose(fp);
}

void handle_added_file()
{
  struct stat file_stat;
  FILE *fp;
  char added_path[PATH_LEN] = ".upc";
  char temp[PATH_LEN];
  char path_of_back[PATH_LEN];
  int len;
  int status;
  upyun_ret_e ret = UPYUN_RET_OK;

  get_path_of_back(path_of_back);

  while(access(added_path, F_OK) < 0)
  {
    strcpy(temp, added_path);
    sprintf(added_path, "../%s", temp);
  }
  strcat(added_path, "/added");
  fp = fopen(added_path, "r");

  while(fgets(temp, PATH_LEN, fp) != NULL)
  {
    get_path_md5(temp, temp, NULL);
    len = strlen(temp);

    if(temp[len-1] == '\n')
    {
      temp[len-1] = '\0';
      ret = upyun_get_fileinfo(thiz, temp, NULL, &status);
      if(status == 200)
      {
        printf("duplicate file %s\n", temp);
        continue;
      }
      printf("make directory %s ...\n", temp);
      ret = upyun_make_dir(thiz, temp, 0, &status); 

      if(ret != UPYUN_RET_OK || status != 200)
      {
        printf("upyun_make_dir %s error: %d\n", temp, ret);
        printf("status: %d\n", status);
      }
      printf("completed.\n");
    } else {
      ret = upyun_get_fileinfo(thiz, temp, NULL, &status);
      if(status == 200)
      {
        printf("duplicate file %s\n", temp);
        continue;
      }
      char *local_path = temp + strlen(path_of_back);
      stat(local_path, &file_stat);
      upyun_content_t content = {0};
      content.type = UPYUN_CONTENT_FILE;
      content.u.fp = fopen(local_path, "rb");
      content.len = file_stat.st_size;

      printf("uploading file %s ...\n", local_path);
      ret = upyun_upload_file(thiz, temp, &content, NULL, NULL, &status);
      if(ret != UPYUN_RET_OK || status != 200)
      {
        printf("upyun_upload_file %s error: %d\n", temp, ret);
        printf("status: %d\n", status);
      }
      printf("completed.\n");
      fclose(content.u.fp);
    }
  }
  fclose(fp);
}

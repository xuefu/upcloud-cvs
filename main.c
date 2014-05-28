/*
 * ===================================================================================
 *
 *      Filename: main.c
 *
 *   Description: main
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

#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "pull.h"
#include "util.h"
#include "push.h"
#include "stage.h"

#define NAME_LEN 32
#define UPC_VERSION 1.0

upyun_t *thiz = NULL;

void upcloud_usage()
{
  printf("usage: upc [--version] [--help] \n \t   <command> [<args>]\n");
  printf("最常用的upc命令有:\n" 
      "\tupc clone bucket_name@user_name\n"
      "\tupc add <file|directory>\n"
      "\tupc push\n"
      "\tupc status\n"
      "\tupc reset\n");
}

void upcloud_reset_stage()
{
  FILE *fp;
  char upc_path[PATH_LEN] = ".upc";
  char temp_path[PATH_LEN];

  while(access(upc_path, F_OK) < 0)
  {
    strcpy(temp_path, upc_path);
    sprintf(upc_path, "../%s", temp_path);
  }

  strcpy(temp_path, upc_path);
  strcat(temp_path, "/stage");
  fp = fopen(temp_path, "w");
  fclose(fp);

  strcpy(temp_path, upc_path);
  strcat(temp_path, "/added");
  fp = fopen(temp_path, "w");
  fclose(fp);

  strcpy(temp_path, upc_path);
  strcat(temp_path, "/removed");
  fp = fopen(temp_path, "w");
  fclose(fp);
}

long get_bucket_usage(upyun_t *thiz, char *bucket)
{
  int status = 0;
  upyun_ret_e ret = UPYUN_RET_OK;
  upyun_usage_info_t bucket_usage;

  char path[NAME_LEN] = "/";
  strcat(path, bucket);
  strcat(path, "/");

  ret = upyun_get_usage(thiz, path, &bucket_usage, &status);

  if(ret != UPYUN_RET_OK || (status != 200 && status == 401))
  {
    printf("Unauthorized...\n");
    return -1;
  }
  return bucket_usage.usage;
}

/* check whether a directory or file */
int is_dir(const char *dir)
{
  struct stat stat_info;
  stat(dir, &stat_info);

  if(S_ISDIR(stat_info.st_mode))
    return 1;
  return 0;
}


int stage_is_empty()
{
  struct stat file_stat;
  char upc_path[PATH_LEN] = ".upc";
  char temp_path[PATH_LEN];

  while(access(upc_path, F_OK) < 0)
  {
    strcpy(temp_path, upc_path);
    sprintf(upc_path, "../%s", temp_path);
  }


  strcpy(temp_path, upc_path);
  strcat(temp_path, "/added");
  stat(temp_path, &file_stat);
  if(file_stat.st_size != 0)
  {
    return 0;
  }

  strcpy(temp_path, upc_path);
  strcat(temp_path, "/removed");
  stat(temp_path, &file_stat);
  if(file_stat.st_size != 0)
  {
    return 0;
  }

  strcpy(temp_path, upc_path);
  strcat(temp_path, "/stage");
  truncate(temp_path, 0);

  return 1;
}

void update_stage(tree_file_t *tft, const char *dir)
{
  if(!stage_is_empty())
  {
    printf("The stage is not empty, please push or reset local bucket, then do it.\n");
    exit(0);
  }

  char path_of_back[PATH_LEN];
  get_path_of_back(path_of_back);

  if(is_dir(dir))
  {
    if(exist_upc_dir(dir))
    {
      local_readdir(tft, dir, path_of_back);
      save_stage_tree(tft);
      current_changed_file();
      if(stage_is_empty())
      {
        printf("no file changed.\n");
      }
    } else {
      local_readdir(tft, dir, path_of_back);
      add_dir_to_added(tft); 
    }
  } else {
    add_file_to_added(dir);
  }
}

int main(int argc, char *argv[])
{
  if(argc == 1) /* upc */
  {
    upcloud_usage();
    return 0;

  /* upc status, upc push, upc usage, upc --version, upc --help */
  } else if(argc == 2) { 
  /* upc reset */
  if(strcmp(argv[1], "reset") == 0)
  {
    if(!below_upc_repo())
    {
      printf("fatal: Not a upc repository (or any of the parent directories): .upc\n");
      return 0;
    }
    upcloud_reset_stage();
    return 0;
  }

  /* upd usage */
  if(strcmp(argv[1], "usage") == 0)
  {
    if(!below_upc_repo())
    {
      printf("fatal: Not a upc repository (or any of the parent directories): .upc\n");
      return 0;
    }

    long usage;
    char user_name[NAME_LEN];
    char bucket_name[NAME_LEN];
    char *password;

    get_user_name(user_name, NAME_LEN);
    get_bucket_name(bucket_name, NAME_LEN);
    printf("username:   %s\n", user_name);
    printf("bucketname: %s\n", bucket_name);
    password = getpass("Enter password: ");

    upyun_global_init();
    upyun_config_t conf = {0};
    conf.user = user_name;
    conf.passwd = password;
    conf.endpoint = UPYUN_ED_AUTO;
    conf.debug = 0;

    upyun_t *u = upyun_create(&conf);
    thiz = u;

    usage = get_bucket_usage(thiz, bucket_name);
    if(usage != -1)
    {
      printf("has been used space: %4.2f MB\n", (float)usage/1024.0/1024.0);
    }
    return 0;
  }

  /* upc push */
  if(strcmp(argv[1], "push") == 0)
  {
    if(!below_upc_repo())
    {
      printf("fatal: Not a upc repository (or any of the parent directories): .upc\n");
      return 0;
    }

    char user_name[NAME_LEN];
    char bucket_name[NAME_LEN];
    char *password;

    get_user_name(user_name, NAME_LEN);
    get_bucket_name(bucket_name, NAME_LEN);
    printf("username:   %s\n", user_name);
    printf("bucketname: %s\n", bucket_name);
    password = getpass("Enter password: ");

    upyun_global_init();
    upyun_config_t conf = {0};
    conf.user = user_name;
    conf.passwd = password;
    conf.endpoint = UPYUN_ED_AUTO;
    conf.debug = 0;

    upyun_t *u = upyun_create(&conf);
    thiz = u;

    /* test whether the password is right */
    if(get_bucket_usage(thiz, bucket_name) < 0)
    {
      return -1;
    }

    handle_removed_file();
    handle_added_file();

    upcloud_reset_stage();

    tree_file_t *tft = calloc(1, sizeof(tree_file_t));

    int len = strlen(bucket_name);
    char *prefix = calloc(1, len+3);
    char *tmp = prefix;
    *tmp++ = '/';
    strcpy(tmp, bucket_name);
    prefix[len+1] = '/';

    bucket_readdir(tft, prefix);

    change_dir();

    save_origin_tree(tft, bucket_name);

    upyun_destroy(u);

    return 0;
  }

  /* upc status */
  if(strcmp(argv[1], "status") == 0)
  {
    if(!below_upc_repo())
    {
      printf("fatal: Not a upc repository (or any of the parent directories): .upc\n");
      return 0;
    }

    show_status();
    return 0;
  }

  /* upc --version */
  if(strcmp(argv[1], "--version") == 0)
  {
    printf("upc version %.1f\n", UPC_VERSION);
    return 0;
  }
  
  /* upc --help */
  if(strcmp(argv[1], "--help") == 0)
  {
    upcloud_usage();
    return 0;
  }

  printf("fatal: upc has no command : %s\n"
      "upc --help for more usage.\n", argv[1]);

  /* upc add <file|directory>, upc clone bucket_name@user_name */
  } else if(argc == 3) {


  /* upc clone  bucket@user */
  if(strcmp(argv[1], "clone") == 0)
  {
    int len_argv2 = strlen(argv[2]);

    if(*argv[2] == '@' || argv[2][len_argv2-1] == '@' || strchr(argv[2], '@') == NULL)
    {
      printf("usage: upc clone bucket@user.\n");
      exit(0);
    }

    char user_name[NAME_LEN];
    char bucket_name[NAME_LEN];

    if(init_local_bucket(argv[2], user_name, bucket_name) < 0)
      return -1;

    int len = strlen(bucket_name);
    char *password;
    printf("username:   %s\n", user_name);
    printf("bucketname: %s\n", bucket_name);
    password = getpass("Enter password: ");

    upyun_global_init();
    upyun_config_t conf = {0};
    conf.user = user_name;
    conf.passwd = password;
    conf.endpoint = UPYUN_ED_AUTO;
    conf.debug = 0;

    upyun_t *u = upyun_create(&conf);
    thiz = u;

    tree_file_t *tft = calloc(1, sizeof(tree_file_t));

    char *prefix = calloc(1, len+3);
    char *tmp = prefix;
    *tmp++ = '/';
    strcpy(tmp, bucket_name);
    prefix[len+1] = '/';

    if(bucket_readdir(tft, prefix) < 0)
    {
      sprintf(bucket_name, "rm -rf %s", tmp);
      system(bucket_name);
      exit(-1);
    }

    pull_bucket(tft);

    save_origin_tree(tft, bucket_name);

    upyun_destroy(u);

    return 0;
  }
  /****************************/

  /* upc add <dir>|<file> */
  if(strcmp(argv[1], "add") == 0)
  {

    if(!below_upc_repo())
    {
      printf("fatal: Not a upc repository (or any of the parent directories): .upc\n");
      return 0;
    }

    if(access(argv[2], F_OK) < 0)
    {
      printf("%s: ", argv[2]);
      fflush(stdout);
      perror("");
      printf("usage: upc add <dir>|<file>\n");
      exit(0);
    }
    tree_file_t *tft = calloc(1, sizeof(tree_file_t));
    char arg[NAME_LEN];
    int len;
    strcpy(arg, argv[2]);
    len = strlen(arg);
    if(arg[len-1] == '/')
      arg[len-1] = '\0';

    update_stage(tft, arg);

    return 0;
  }

  printf("fatal: upc has no command %s %s\n", argv[1], argv[2]);
  /* other */
  } else {
    printf("Maybe, you should type `upc --help` for help.\n\n");
    upcloud_usage();
  }

  return 0;
}

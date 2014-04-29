#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>

#include "pull.h"

upyun_t *thiz = NULL;

void upcloud_usage()
{
  printf("usage: upc [--version] [--help] \n \t   <command> [<args>]\n");
  printf("最常用的upc命令有:\n");
}

int main(int argc, char *argv[])
{
  if(argc == 1)
  {
    upcloud_usage();
    exit(0);
  }

  /* upc clone  bucket@user */
  if(strcmp(argv[1], "clone") == 0)
  {
    if(argc == 2 || (argc > 2 && *argv[2] == '@'))
    {
      printf("usage: upc clone bucket@user.\n");
      exit(0);
    }

    char *user_name;
    char *bucket_name;
    bucket_name = argv[2];
    user_name = strstr(bucket_name, "@");
    if(user_name != NULL)
    {
      *user_name++ = '\0';
    }

    if(mkdir(bucket_name, 0777) < 0)
    {
      if(errno == EEXIST)
        printf("the dir %s exists.\n", bucket_name);
      exit(0);
    }

    int len = strlen(user_name);
    char passwd[16];
    printf("username:   %s\n", user_name);
    printf("bucketname: %s\n", bucket_name);
    printf("password:   ");
    scanf("%s", passwd);

    upyun_global_init();
    upyun_config_t conf = {0};
    conf.user = user_name;
    conf.passwd = passwd;
    conf.endpoint = UPYUN_ED_AUTO;
    conf.debug = 0;

    upyun_t *u = upyun_create(&conf);
    thiz = u;

    tree_file_t *tft = calloc(1, sizeof(tree_file_t));

    char *prefix = calloc(1, len+3);
    char *tmp = prefix;
    *tmp++ = '/';
    strcpy(tmp, user_name);
    prefix[len+1] = '/';

    bucket_readdir(tft, prefix);

    pull_bucket(tft);

    upyun_destroy(u);
  }
  /****************************/

  return 0;
}

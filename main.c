#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "pull.h"
#include "util.h"
#include "push.h"

upyun_t *thiz = NULL;

void upcloud_usage()
{
    printf("usage: upc [--version] [--help] \n \t   <command> [<args>]\n");
    printf("最常用的upc命令有:\n");
}

void upcloud_reset()
{
    char upc_path[64] = ".upc";
    char stage_path[64];
    char added_path[64];
    char removed_path[64];
    char temp[64];

    while(access(".upc", F_OK) < 0)
    {
        strcpy(temp, upc_path);
        sprintf(upc_path, "../%s", temp);
    }

    strcpy(stage_path, upc_path);
    strcpy(added_path, upc_path);
    strcpy(removed_path, upc_path);
    strcat(stage_path, "/stage");
    strcat(added_path, "/added");
    strcat(removed_path, "/removed");
    FILE *fp = fopen(stage_path, "w");
    fclose(fp);
    fp = fopen(added_path, "w");
    fclose(fp);
    fp = fopen(removed_path, "w");
    fclose(fp);

    printf("reset success!\n");
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

        char user_name[32];
        char bucket_name[32];

        if(save_user_bucket_info(argv[2], user_name, bucket_name) < 0)
            return -1;

        int len = strlen(bucket_name);
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
        strcpy(tmp, bucket_name);
        prefix[len+1] = '/';

        bucket_readdir(tft, prefix);

        save_origin_tree(tft);

        pull_bucket(tft);


        upyun_destroy(u);
    }
    /****************************/

    /* upc add <dir>|<file> */
    if(strcmp(argv[1], "add") == 0)
    {
        if(argc == 2)
        {
            printf("usage: upc add <dir>|<file>\n");
            exit(0);
        }
        tree_file_t *tft = calloc(1, sizeof(tree_file_t));
        char arg[32];
        int len;
        strcpy(arg, argv[2]);
        len = strlen(arg);
        if(arg[len-1] == '/')
            arg[len-1] = '\0';

        general_readdir(tft, arg);
    }
    /************************/

    /* upc reset */
    if(strcmp(argv[1], "reset") == 0)
    {
        upcloud_reset();
    }
    /*********************************/

    if(strcmp(argv[1], "push") == 0)
    {
        char user_name[16];
        char bucket_name[16];
        char password[16];

        get_user_name(user_name);
        get_bucket_name(bucket_name);
        printf("user:  %s\n", user_name);
        printf("bucket: %s\n", bucket_name);
        printf("password: ");
        scanf("%s", password);

        upyun_global_init();
        upyun_config_t conf = {0};
        conf.user = user_name;
        conf.passwd = password;
        conf.endpoint = UPYUN_ED_AUTO;
        conf.debug = 0;

        upyun_t *u = upyun_create(&conf);
        thiz = u;

        handle_removed_file();
        handle_added_file();

        upcloud_reset();

        tree_file_t *tft = calloc(1, sizeof(tree_file_t));

        int len = strlen(bucket_name);
        char *prefix = calloc(1, len+3);
        char *tmp = prefix;
        *tmp++ = '/';
        strcpy(tmp, bucket_name);
        prefix[len+1] = '/';

        bucket_readdir(tft, prefix);

        push_origin_tree(tft);

        upyun_destroy(u);
    }
    return 0;
}

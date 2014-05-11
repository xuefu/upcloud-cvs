#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util.h"

#define PATH_LEN 1024

static char path_of_back[PATH_LEN];

/* parse the name of user and bucket from the argv like 'bucket_name@user_name' */
static int parse_user_bucket(const char *user_bucket_info, char *user, char *bucket)
{
    char *user_name;
    if((user_name=strstr(user_bucket_info, "@")) != NULL)
    {
        *user_name++ = '\0';
        strcpy(bucket, user_bucket_info);
        strcpy(user, user_name);
        return 0;
    }
    printf("usage: upc clone bucket@user.\n");
    return -1;
}

/* initial the local bucket */
int init_local_bucket(const char *user_bucket_info, char *user, char *bucket)
{
    FILE *fp;
    char path[PATH_LEN];

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

    /* save the bucket_name@user_name to the file meta */
    strcat(path, "/meta");
    fp = fopen(path, "w");
    fwrite(user_bucket_info, sizeof(char), strlen(user_bucket_info), fp);
    fclose(fp);

    return 0;
}

/* save path of the general tree to the file */
static void save_tree(tree_file_t *tft, FILE *fp)
{
    char path[PATH_LEN];

    int i, nchild;
    nchild = tft->nchild;

    for(i = 0; i < nchild; i++)
    {
        sprintf(path, "%s\n", tft->child[i]->path);
        fwrite(path, sizeof(char), strlen(path), fp);
        if(tft->type == 'D')
        {
            save_tree(tft->child[i], fp); /* recursive calls */ 
        }
    }
}

/* save the tree of remote bucket to local file */
void save_origin_tree(tree_file_t *tft, char *bucket)
{
    FILE *fp;
    char origin_path[PATH_LEN];
    strcpy(origin_path, bucket);
    strcat(origin_path, "/.upc/origin");
    fp = fopen(origin_path, "a");
    save_tree(tft, fp);
    fclose(fp);
}

/* save the tree of remote bucket to local file after push stage */
void push_origin_tree(tree_file_t *tft)
{
    FILE *fp;
    char origin_path[PATH_LEN] = ".upc/origin";
    char temp_path[PATH_LEN];

    while(access(origin_path, F_OK) < 0)
    {
        strcpy(temp_path, origin_path);
        sprintf(origin_path, "../%s", temp_path);
    }

    /* clear the content of the file origin */
    fp = fopen(origin_path, "w");
    fclose(fp);

    fp = fopen(origin_path, "a");
    save_tree(tft, fp);
    fclose(fp);
}

/* check whether the upc directory exists in the current directory */
static int exist_upc_dir()
{
    DIR *pDir;
    struct dirent *ent;

    pDir = opendir(".");

    while((ent=readdir(pDir)) != NULL)
    {
        if(ent->d_type & DT_DIR)
        {
            if(strcmp(ent->d_name, ".upc") == 0)
            {
                if (closedir(pDir) < 0) 
                {
                    printf("cant't close directory\n");
                }
                return 1;
            }
        }
    }
    if (closedir(pDir) < 0) 
    {
        printf("cant't close directory\n");
    }
    return 0;
}

/* get the name of current directory */
static void get_current_dir_name(char *dir_name)
{
    char absolute_path[PATH_LEN];
    char *ptr;
    char *ptr2;

    if(getcwd(absolute_path, sizeof(absolute_path)) == NULL)
        perror("get current directory error: ");
    ptr = strtok(absolute_path, "/");
    if(ptr == NULL)
    {
        printf("fatal: Not a upc repository (or any of the parent directories): .upc\n");
        exit(0);
    }
    while(ptr != NULL)
    {
        ptr2 = ptr;
        ptr = strtok(NULL, "/");
    }
    strcpy(dir_name, ptr2);
    /* change the current directory to the parent directory */
    chdir("..");
}

/* init the global value: path_of_back */
void set_path_of_back()
{
    char current[PATH_LEN];
    char back_up[PATH_LEN];
    char absolute_path[PATH_LEN];

    if(getcwd(absolute_path, sizeof(absolute_path)) == NULL)
        perror("get current directory error: ");

    while(!exist_upc_dir())
    {
        get_current_dir_name(current);
        strcpy(back_up, path_of_back);
        sprintf(path_of_back, "/%s%s/", current, back_up);
    }
    get_current_dir_name(current);
    strcpy(back_up, path_of_back);
    sprintf(path_of_back, "/%s%s/", current, back_up);
    chdir(absolute_path);
}

/* get the user name from the config file meta */
void get_user_name(char *user, int len)
{
    FILE *fp;
    char temp_path[PATH_LEN];
    char meta_path[PATH_LEN] = ".upc/meta";

    while(access(meta_path, F_OK) < 0)
    {
        strcpy(temp_path, meta_path);
        sprintf(meta_path, "../%s", temp_path);
    }
    fp = fopen(meta_path, "r");
    fgets(user, len, fp);
    char *ptr = strstr(user, "@");
    strcpy(user, ++ptr);
    fclose(fp);
}

/* get the bucket name from the config file meta */
void get_bucket_name(char *bucket, int len)
{
    FILE *fp;
    char temp_path[PATH_LEN];
    char meta_path[PATH_LEN] = ".upc/meta";

    while(access(meta_path, F_OK) < 0)
    {
        strcpy(temp_path, meta_path);
        sprintf(meta_path, "../%s", temp_path);
    }
    fp = fopen(meta_path, "r");
    fgets(bucket, len, fp);
    char *ptr = strstr(bucket, "@");
    *ptr = '\0';
    fclose(fp);
}

/* save all files of the local bucket to stage */
void save_stage_tree(tree_file_t *tft)
{
    FILE *fp;
    char temp_path[PATH_LEN];
    char stage_path[PATH_LEN] = ".upc";
    while(access(stage_path, F_OK) < 0)
    {
        strcpy(temp_path, stage_path);
        sprintf(stage_path, "../%s", temp_path);
    }

    strcat(stage_path, "/stage");

    fp = fopen(stage_path, "a");
    /* if the command is 'upc add .', we should not store
     * the first path to stage
     * */
    if(strcmp(tft->path, path_of_back) != 0)    
    /* for 'upc add <directory>' not 'upc add .' */
    { 
        sprintf(temp_path, "%s\n", tft->path);
        fwrite(temp_path, sizeof(char), strlen(temp_path), fp);
    }
    save_tree(tft, fp);
    fclose(fp);
}

/* get the number of all files below one directory
 * except '.', '..' and '.upc'
 * */
static int dir_child_len(const char *dir)
{
    int nchild = 0;
    DIR *pDir;
    struct dirent *ent;

    pDir = opendir(dir);
    while((ent=readdir(pDir)) != NULL)
    {
        if (strcmp(ent->d_name, ".upc")==0 || strcmp(ent->d_name, ".")==0
                || strcmp(ent->d_name, "..")==0) 
        {
            continue; 
        }
        nchild++;
    }

    if (closedir(pDir) < 0) 
    {
        printf("cant't close directory\n");
    }

    return nchild;
}

/* read all files of the local bucket and construct the general tree */
void local_readdir(tree_file_t *tft, const char *dir)
{
    int nchild; // the tft has n child
    DIR *pDir;
    struct dirent *ent; // the directory dir dirent info
    struct stat file_stat; // the new file's stat info

    stat(dir, &file_stat);
    nchild = dir_child_len(dir);
    pDir = opendir(dir);

    // Initialize the parent
    if(strcmp(dir, ".") == 0)
    {
        tft->path = calloc(1, strlen(path_of_back)+1);
        sprintf(tft->path, "%s", path_of_back);
    } else {
        tft->path = calloc(1, strlen(dir)+strlen(path_of_back)+2);
        sprintf(tft->path, "%s%s/", path_of_back, dir);
    }

    tft->date = file_stat.st_mtime;
    tft->type = 'D';
    tft->size = file_stat.st_size;
    tft->nchild = nchild;
    /* not only allocate *child but also **child.
    * tft->child = calloc(1, nchild); 
    * */
    tft->child = calloc(nchild, sizeof(tree_file_t *));

    nchild = 0;
    while ((ent=readdir(pDir)) != NULL) 
    {
        if (ent->d_type & DT_DIR) /* directory */
        {
            if (strcmp(ent->d_name, ".upc")==0 || strcmp(ent->d_name, ".")==0 
                    || strcmp(ent->d_name, "..")==0) 
            {
                continue; 
            }
            tree_file_t *new_dir = calloc(1, sizeof(tree_file_t));
            tft->child[nchild] = new_dir;
            char *new_path;
            if(strcmp(dir, ".") == 0)
            {
                new_path = calloc(1, strlen(ent->d_name)+1);
                strcpy(new_path, ent->d_name);
            } else {
                new_path = calloc(1, strlen(dir)+strlen(ent->d_name)+2);
                sprintf(new_path, "%s/%s", dir, ent->d_name);
            }
            new_dir->parent = tft;
            local_readdir(new_dir, new_path); /* recursively calls */
            free(new_path);
        } else { /* file */
            tree_file_t *new_file = calloc(1, sizeof(tree_file_t));
            char *new_path;
            if(strcmp(dir, ".") == 0)
            {
                new_path = calloc(1, strlen(path_of_back)+strlen(dir)+strlen(ent->d_name)+1);
                new_file->path = calloc(1, strlen(path_of_back)+strlen(dir)+strlen(ent->d_name)+1);
                sprintf(new_path, "%s%s", path_of_back, ent->d_name);
            } else {
                new_path = calloc(1, strlen(path_of_back)+strlen(dir)+strlen(ent->d_name)+2);
                new_file->path = calloc(1, strlen(path_of_back)+strlen(dir)+strlen(ent->d_name)+2);
                sprintf(new_path, "%s%s/%s", path_of_back, dir, ent->d_name);
            }
            stat(new_path, &file_stat);
            strcpy(new_file->path, new_path);
            free(new_path);
            new_file->date = file_stat.st_mtime;
            new_file->type = 'F';
            new_file->size = file_stat.st_size;
            new_file->nchild = 0;
            new_file->child = 0;
            new_file->parent = tft;

            tft->child[nchild] = new_file;
        }
        nchild++;
    }
    if (closedir(pDir) < 0) 
    {
        printf("cant't close directory\n");
    }
}


/* if 'upc add <file>', then call it to store the path to stage */
void add_file_to_stage(const char *file)
{
    FILE *fp;
    char file_path[PATH_LEN];
    char stage_path[PATH_LEN] = ".upc/stage";
    char temp_path[PATH_LEN];

    printf("%s\n", path_of_back);
    sprintf(file_path, "%s%s\n", path_of_back, file);
    printf("%s\n", file_path);

    while(access(stage_path, F_OK) < 0)
    {
        strcpy(temp_path, stage_path);
        sprintf(stage_path, "../%s", temp_path);
    }

    fp = fopen(stage_path, "a");
    fwrite(file_path, sizeof(char), strlen(file_path), fp);
    fclose(fp);
}

/* check whether the buf exists in the stage or not */
int exist_in_stage(char *buf, char *stage_path)
{
    FILE *fp_stage;
    char temp_path[PATH_LEN];

    fp_stage = fopen(stage_path, "r");

    while(fgets(temp_path, PATH_LEN, fp_stage) != NULL)
    {
        if(strcmp(temp_path, buf) == 0)
        {
            return 1;
        }
    }

    fclose(fp_stage);
    return 0;
}

/* check whether the buf exists in the origin or not */
int exist_in_origin(char *buf, char *origin_path)
{
    FILE *fp_origin;
    char temp_path[PATH_LEN];

    fp_origin = fopen(origin_path, "r");

    while(fgets(temp_path, PATH_LEN, fp_origin) != NULL)
    {
        if(strcmp(temp_path, buf) == 0)
        {
            return 1;
        }
    }

    fclose(fp_origin);
    return 0;
}

/* find out the changed file and store them in the added or removed */
void current_changed_file()
{
    FILE *fp_removed;
    FILE *fp_origin;
    FILE *fp_stage;
    FILE *fp_added;

    char stage_path[PATH_LEN];
    char origin_path[PATH_LEN];
    char removed_path[PATH_LEN] = ".upc";
    char added_path[PATH_LEN];
    char temp_path[PATH_LEN];

    while(access(removed_path, F_OK) < 0)
    {
        strcpy(temp_path, removed_path);
        sprintf(removed_path, "../%s", temp_path);
    }
    strcpy(stage_path, removed_path);
    strcpy(origin_path, removed_path);
    strcpy(added_path, removed_path);
    strcat(stage_path, "/stage");
    strcat(origin_path, "/origin");
    strcat(removed_path, "/removed");
    strcat(added_path, "/added");

    fp_origin = fopen(origin_path, "r");
    fp_removed = fopen(removed_path, "a");

    /* store the deleted file to the removed */
    while(fgets(temp_path, PATH_LEN, fp_origin) != NULL)
    {
        if(!exist_in_stage(temp_path, stage_path))
        {
            fwrite(temp_path, sizeof(char), strlen(temp_path), fp_removed);
        }
    }
    fclose(fp_removed);
    fclose(fp_origin);

    fp_stage = fopen(stage_path, "r");
    fp_added = fopen(added_path, "a");
    /* store the added file to the added */
    while(fgets(temp_path, PATH_LEN, fp_stage) != NULL)
    {
        if(!exist_in_origin(temp_path, origin_path))
        {
            fwrite(temp_path, sizeof(char), strlen(temp_path), fp_added);
        }
    }
    fclose(fp_added);
    fclose(fp_stage);
}

void show_status()
{
    FILE *fp;
    int n = 0;
    char *ptr;
    char upc_path[PATH_LEN] = ".upc";
    char temp_path[PATH_LEN];
    char buf[PATH_LEN];

    set_path_of_back();

    while(access(upc_path, F_OK) < 0)
    {
        strcpy(temp_path, upc_path);
        sprintf(upc_path, "../%s", temp_path);
    }
    strcpy(temp_path, upc_path);
    strcat(temp_path, "/added");

    fp = fopen(temp_path, "r");
    while(fgets(buf, PATH_LEN, fp) != NULL) n++;
    rewind(fp);
    printf("changes need to be pushed: \n"
            "\t(\"upc clear\" can clear the stage)\n\n"
            "%d files has been added to the stage:\n", n);
    while(fgets(buf, PATH_LEN, fp) != NULL)
    {
        ptr = buf + strlen(path_of_back);
        printf("+\t%s", ptr);
    }
    fclose(fp);

    strcpy(temp_path, upc_path);
    strcat(temp_path, "/removed");

    n = 0;
    fp = fopen(temp_path, "r");
    while(fgets(buf, PATH_LEN, fp) != NULL) n++;
    rewind(fp);
    printf("%d files has been removed to the stage:\n", n);
    while(fgets(buf, PATH_LEN, fp) != NULL)
    {
        ptr = buf + strlen(path_of_back);
        printf("-\t%s", ptr);
    }
    fclose(fp);
    /*
    printf("\nchanges need to be add to the stage.\n"
            "\t(\"upc add .\" can add to the stage)\n\n"
            "\n");
            */
}



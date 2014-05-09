#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util.h"

static char path_of_bucket[32];
static char path_of_back[64];

int parse_user_bucket(const char *user_bucket_info, char *user, char *bucket)
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

int save_user_bucket_info(const char *user_bucket_info, char *user, char *bucket)
{
    FILE *fp;
    char info[64];
    char path[64];

    strcpy(info, user_bucket_info);

    if(parse_user_bucket(info, user, bucket))
        return -1;

    strcpy(path, bucket);
    if(mkdir(path, 0777) < 0)
    {
        perror("mkdir error: ");
        return -1;
    }

    strcat(path, "/.upc");
    if(mkdir(path, 0777) < 0)
    {
        perror("mkdir error: ");
        return -1;
    }
    strcpy(path_of_bucket, path);
    strcat(path, "/meta");
    fp = fopen(path, "w");
    fwrite(user_bucket_info, sizeof(char), strlen(user_bucket_info), fp);
    fclose(fp);
    return 0;
}

void save_tree(tree_file_t *tft, FILE *fp)
{
    char path[1024];

    int i, nchild;
    nchild = tft->nchild;

    for(i = 0; i < nchild; i++)
    {
        sprintf(path, "%s\n", tft->child[i]->path);
        fwrite(path, sizeof(char), strlen(path), fp);
        if(tft->type == 'D')
        {
            save_tree(tft->child[i], fp);
        }
    }
}

void save_origin_tree(tree_file_t *tft)
{
    FILE *fp;
    char origin[32];
    strcpy(origin, path_of_bucket);
    strcat(origin, "/origin");
    fp = fopen(origin, "a");
    save_tree(tft, fp);
    fclose(fp);
}

void push_origin_tree(tree_file_t *tft)
{
    FILE *fp;
    char origin[32] = ".upc/origin";
    char temp[64];

    while(access(origin, F_OK) < 0)
    {
        strcpy(temp, origin);
        sprintf(origin, "../%s", temp);
    }

    fp = fopen(origin, "w");
    fclose(fp);
    fp = fopen(origin, "a");
    save_tree(tft, fp);
    fclose(fp);
}

int exist_upc_dir()
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

void get_current_dir_name(char *dir_name)
{
    char absolute_path[1024];
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
    chdir("..");
}

void set_path_of_back()
{
    char current[1024];
    char back_up[1024];
    char absolute_path[1024];

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

void get_user_name(char *user)
{
    FILE *fp;
    char temp[64];
    char meta[64] = ".upc/meta";

    while(access(meta, F_OK) < 0)
    {
        strcpy(temp, meta);
        sprintf(meta, "../%s", temp);
    }
    fp = fopen(meta, "r");
    fgets(user, 16, fp);
    char *ptr = strstr(user, "@");
    strcpy(user, ++ptr);
    fclose(fp);
}

void get_bucket_name(char *bucket)
{
    FILE *fp;
    char temp[64];
    char meta[64] = ".upc/meta";

    while(access(meta, F_OK) < 0)
    {
        strcpy(temp, meta);
        sprintf(meta, "../%s", temp);
    }
    fp = fopen(meta, "r");
    fgets(bucket, 16, fp);
    char *ptr = strstr(bucket, "@");
    *ptr = '\0';
    fclose(fp);
}

void save_stage_tree(tree_file_t *tft)
{
    FILE *fp;
    char temp[64];
    char stage[64] = ".upc";
    while(access(stage, F_OK) < 0)
    {
        strcpy(temp, stage);
        sprintf(stage, "../%s", temp);
    }

    strcat(stage, "/stage");

    fp = fopen(stage, "a");
    if(strcmp(tft->path, path_of_back) != 0)
    {
        sprintf(temp, "%s\n", tft->path);
        fwrite(temp, sizeof(char), strlen(temp), fp);
    }
    save_tree(tft, fp);
    fclose(fp);
}

int dir_child_len(const char *dir)
{
    int nchild = 0;
    DIR *pDir;
    struct dirent *ent;

    pDir = opendir(dir);
    while((ent=readdir(pDir)) != NULL)
    {
        if (strcmp(ent->d_name, ".upc")==0 || strcmp(ent->d_name, ".")==0 || strcmp(ent->d_name, "..")==0) 
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
    //tft->child = calloc(1, nchild);
    tft->child = calloc(nchild, sizeof(tree_file_t *));

    nchild = 0;
    while ((ent=readdir(pDir)) != NULL) 
    {
        if (ent->d_type & DT_DIR) 
        {
            if (strcmp(ent->d_name, ".upc")==0 || strcmp(ent->d_name, ".")==0 || strcmp(ent->d_name, "..")==0) 
            {
                continue; 
            }
            tree_file_t *new_dir = calloc(1, sizeof(tree_file_t));
            tft->child[nchild] = new_dir;
            char *new_path; if(strcmp(dir, ".") == 0)
            {
                new_path = calloc(1, strlen(ent->d_name)+1);
                strcpy(new_path, ent->d_name);
            } else {
                new_path = calloc(1, strlen(dir)+strlen(ent->d_name)+2);
                sprintf(new_path, "%s/%s", dir, ent->d_name);
            }
            new_dir->parent = tft;
            local_readdir(new_dir, new_path);
            free(new_path);
        } else {
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

int is_dir(const char *dir)
{
    struct stat stat_info;
    stat(dir, &stat_info);

    if(S_ISDIR(stat_info.st_mode))
        return 1;
    return 0;
}

void add_file_to_stage(const char *file)
{
    FILE *fp;
    char file_path[64];
    char stage_path[64] = ".upc/stage";
    char temp[64];

    printf("%s\n", path_of_back);
    sprintf(file_path, "%s%s\n", path_of_back, file);
    printf("%s\n", file_path);

    while(access(stage_path, F_OK) < 0)
    {
        strcpy(temp, stage_path);
        sprintf(stage_path, "../%s", temp);
    }

    fp = fopen(stage_path, "a");
    fwrite(file_path, sizeof(char), strlen(file_path), fp);
    fclose(fp);
}

int exist_in_stage(char *buf, char *stage_path)
{
    FILE *fp_stage;
    char temp[64];

    fp_stage = fopen(stage_path, "r");

    while(fgets(temp, 64, fp_stage) != NULL)
    {
        if(strcmp(temp, buf) == 0)
        {
            return 1;
        }
    }

    fclose(fp_stage);
    return 0;
}

int exist_in_origin(char *buf, char *origin_path)
{
    FILE *fp_origin;
    char temp[64];

    fp_origin = fopen(origin_path, "r");

    while(fgets(temp, 64, fp_origin) != NULL)
    {
        if(strcmp(temp, buf) == 0)
        {
            return 1;
        }
    }

    fclose(fp_origin);
    return 0;
}

void current_changed_file()
{
    FILE *fp_removed;
    FILE *fp_origin;
    FILE *fp_stage;
    FILE *fp_added;

    char stage_path[64];
    char origin_path[64];
    char removed_path[64] = ".upc";
    char added_path[64];
    char temp[64];

    while(access(removed_path, F_OK) < 0)
    {
        strcpy(temp, removed_path);
        sprintf(removed_path, "../%s", temp);
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

    while(fgets(temp, 64, fp_origin) != NULL)
    {
        if(!exist_in_stage(temp, stage_path))
        {
            fwrite(temp, sizeof(char), strlen(temp), fp_removed);
        }
    }
    fclose(fp_removed);
    fclose(fp_origin);

    fp_stage = fopen(stage_path, "r");
    fp_added = fopen(added_path, "a");
    while(fgets(temp, 64, fp_stage) != NULL)
    {
        if(!exist_in_origin(temp, origin_path))
        {
            fwrite(temp, sizeof(char), strlen(temp), fp_added);
        }
    }
    fclose(fp_added);
    fclose(fp_stage);
}


void general_readdir(tree_file_t *tft, const char *dir)
{
    struct stat file_stat;
    stat(".upc/stage", &file_stat);
    if(file_stat.st_size != 0)
    {
        printf("The stage is not empty, please push or reset, then do it.\n");
        exit(0);
    }
    if(is_dir(dir))
    {
        set_path_of_back();
        local_readdir(tft, dir);
        save_stage_tree(tft);
        current_changed_file();
    } else {
        set_path_of_back();
        add_file_to_stage(dir);
        current_changed_file();
    }
}


#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>

#include "stage.h"
#include "color.h"

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

/* save all files of the local bucket to stage */
void save_stage_tree(tree_file_t *tft)
{
  FILE *fp;
  char temp_path[PATH_LEN];
  char path_of_back[PATH_LEN];
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
  get_path_of_back(path_of_back);
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
void local_readdir(tree_file_t *tft, const char *dir, const char *path_of_back)
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
      local_readdir(new_dir, new_path, path_of_back); /* recursively calls */
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

/* if 'upc add <file>', then call it to store the path to added */
void add_file_to_added(const char *file)
{
  FILE *fp;
  char file_path[PATH_LEN];
  char added_path[PATH_LEN] = ".upc";
  char temp_path[PATH_LEN];

  get_path_of_back(temp_path);
  sprintf(file_path, "%s%s\n", temp_path, file);

  while(access(added_path, F_OK) < 0)
  {
    strcpy(temp_path, added_path);
    sprintf(added_path, "../%s", temp_path);
  }

  strcpy(temp_path, added_path);
  strcat(temp_path, "/origin");

  if(exist_in_origin(file_path, temp_path))
  {
    int len = strlen(file_path);
    file_path[len-1] =  '\0';
    printf("this file %s exists in the origin bucket.\n", file_path);
    exit(0);
  }

  strcat(added_path, "/added");
  fp = fopen(added_path, "a");
  fwrite(file_path, sizeof(char), strlen(file_path), fp);
  fclose(fp);
}

void add_dir_to_added(tree_file_t *tft)
{
  FILE *fp;
  char upc_path[PATH_LEN] = ".upc";
  char temp_path[PATH_LEN];

  while(access(upc_path, F_OK) < 0)
  {
    strcpy(temp_path, upc_path);
    sprintf(upc_path, "../%s", temp_path);
  }
  strcat(upc_path, "/added");

  fp = fopen(upc_path, "a");
  strcpy(temp_path, tft->path);
  strcat(temp_path, "\n");
  fwrite(temp_path, sizeof(char), strlen(temp_path), fp);
  save_tree(tft, fp);
  fclose(fp);

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
  //char *ptr;
  char upc_path[PATH_LEN] = ".upc";
  char temp_path[PATH_LEN];
  //char path_of_back[PATH_LEN];
  char buf[PATH_LEN];

  //  get_path_of_back(path_of_back);
  while(access(upc_path, F_OK) < 0)
  {
    strcpy(temp_path, upc_path);
    sprintf(upc_path, "../%s", temp_path);
  }
  strcpy(temp_path, upc_path);
  strcat(temp_path, "/added");

  if((fp = fopen(temp_path, "r")) == NULL)
  {
    perror("open file error: ");
  };
  while(fgets(buf, PATH_LEN, fp) != NULL) n++;
  rewind(fp);
  printf("changes need to be pushed: \n"
      "\t(\"upc reset\" can reset the stage)\n\n"
      "%d files has been added to the stage:\n", n);
  while(fgets(buf, PATH_LEN, fp) != NULL)
  {
    n = strlen(buf);

    if(buf[n-2] == '/')
    {
      fg_green_color();
      printf("\t+\t");
      fg_blue_color();
      printf("%s", buf);
    } else {
      fg_green_color();
      printf("\t+\t%s", buf);
    }
  }
  printf("\n");
  reset_color();
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
    n = strlen(buf);

    if(buf[n-2] == '/')
    {
      fg_red_color();
      printf("\t-\t");
      fg_blue_color();
      printf("%s", buf);
    } else {
      fg_red_color();
      printf("\t-\t%s", buf);
    }
  }
  printf("\n");
  reset_color();
  fclose(fp);
  /*
     printf("\nchanges need to be added to the stage.\n"
     "\t(\"upc add .\" can add to the stage)\n\n"
     "\n");
     */
}

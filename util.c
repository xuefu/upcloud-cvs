#include "util.h"

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

/* check whether the upc directory exists in the current directory */
int exist_upc_dir(const char *path)
{
  DIR *pDir;
  struct dirent *ent;

  pDir = opendir(path);

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

/* get the path of backup */
void get_path_of_back(char *path)
{
  char current[PATH_LEN];
  char back_up[PATH_LEN];
  char path_of_back[PATH_LEN];
  char absolute_path[PATH_LEN];

  if(getcwd(absolute_path, sizeof(absolute_path)) == NULL)
    perror("get current directory error: ");

  while(!exist_upc_dir("."))
  {
    get_current_dir_name(current);
    strcpy(back_up, path_of_back);
    sprintf(path_of_back, "/%s%s", current, back_up);
  }
  get_current_dir_name(current);
  strcpy(back_up, path_of_back);
  sprintf(path_of_back, "/%s%s/", current, back_up);
  strcpy(path, path_of_back);
  chdir(absolute_path);
}

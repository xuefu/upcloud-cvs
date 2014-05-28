/*
 * ===================================================================================
 *
 *      Filename: util.c
 *
 *   Description: utilities functions
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
    /* change the current directory to the parent directory */
    chdir("..");
    strcpy(back_up, path_of_back);
    sprintf(path_of_back, "/%s%s", current, back_up);
  }
  get_current_dir_name(current);
  strcpy(back_up, path_of_back);
  sprintf(path_of_back, "/%s%s/", current, back_up);
  strcpy(path, path_of_back);
  chdir(absolute_path);
}

/* get the password from the stdin and */
char* getpass(const char *prompt)
{
  static char		buf[MAX_PASS_LEN + 1];	/* null byte at end */
  char			*ptr;
  sigset_t		sig, osig;
  struct termios	ts, ots;
  FILE			*fp;
  int				c;

  if ((fp = fopen(ctermid(NULL), "r+")) == NULL)
    return(NULL);
  setbuf(fp, NULL);

  sigemptyset(&sig);
  sigaddset(&sig, SIGINT);		/* block SIGINT */
  sigaddset(&sig, SIGTSTP);		/* block SIGTSTP */
  sigprocmask(SIG_BLOCK, &sig, &osig);	/* and save mask */

  tcgetattr(fileno(fp), &ts);		/* save tty state */
  ots = ts;						/* structure copy */
  ts.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
  tcsetattr(fileno(fp), TCSAFLUSH, &ts);
  fputs(prompt, fp);

  ptr = buf;
  while ((c = getc(fp)) != EOF && c != '\n')
    if (ptr < &buf[MAX_PASS_LEN])
      *ptr++ = c;
  *ptr = 0;			/* null terminate */
  putc('\n', fp);		/* we echo a newline */

  tcsetattr(fileno(fp), TCSAFLUSH, &ots); /* restore TTY state */
  sigprocmask(SIG_SETMASK, &osig, NULL);  /* restore mask */
  fclose(fp);			/* done with /dev/tty */
  return(buf);
}

/* get the path and md5 : return 0 if no md5 , return 1 if md5 */
int get_path_md5(char *from_str, char *path, char *md5)
{
  char *p;

  p = strchr(from_str, ':');

  if(p == NULL)
    return 0;

  *p++ = '\0';

  if(path != NULL)
    strcpy(path, from_str);
  if(md5 != NULL)
    strcpy(md5, p);

  return 1;
}

/* cd to the parent root path */
void change_dir()
{
  char current[PATH_LEN];
  char bucket[NAME_LEN];

  get_bucket_name(bucket, NAME_LEN);
  get_current_dir_name(current);

  while(strcmp(current, bucket) != 0)
  {
    chdir("..");   
    get_current_dir_name(current);
  }

  chdir("..");   
}

int below_upc_repo()
{
  char current_path[PATH_LEN];
  char absolute_path[PATH_LEN];

  getcwd(absolute_path, sizeof(absolute_path));

  while(!exist_upc_dir("."))
  {
    getcwd(current_path, sizeof(current_path));

    if(chdir("..") == 0 || strcmp(current_path, "/") == 0)
    {
      chdir(absolute_path);
      return 0;
    }
  } 
  chdir(absolute_path);
  return 1;
}

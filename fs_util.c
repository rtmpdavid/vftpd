#include "fs_util.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>

int file_abs_name(char *abs_name, const char* rel_name, const char *srv_dir, const char* sess_dir)
{
  if(!rel_name)
  {
    file_abs_name(abs_name, "", srv_dir, sess_dir);
    abs_name[strlen(abs_name)-1] = '\0';

    return 0;
  }

  if(srv_dir)
  {
    if(rel_name[0] == '/')
      sprintf(abs_name, "%s%s", srv_dir, rel_name);
    else
    {
      if(strcmp(sess_dir, "/"))
	sprintf(abs_name, "%s%s/%s", srv_dir, sess_dir, rel_name);
      else
	sprintf(abs_name, "%s%s%s", srv_dir, sess_dir, rel_name);
    }
  }

  else
  {
    if(rel_name[0] == '/')      
      strcpy(abs_name, rel_name);
    else
    {
      if(!strcmp(sess_dir, "/"))
	sprintf(abs_name, "/%s", rel_name);
      else
	sprintf(abs_name, "%s/%s", sess_dir, rel_name);
    }
  }

  return 0;
}

int file_stat_string(char* stat_str, const char* dirname, const char* filename)
{
  char abs_name[255];
  sprintf(abs_name, "%s/%s", dirname, filename);

  struct stat file_stat;
  stat(abs_name, &file_stat);

  if(!(S_ISDIR(file_stat.st_mode)||S_ISREG(file_stat.st_mode)))
    return -1;

  char file_time[255];

  time_t now;
  time(&now);
  struct tm now_tm, *mod_tm, *tmp_tm;
  tmp_tm = gmtime(&now);
  memcpy(&now_tm, tmp_tm, sizeof(struct tm));
  mod_tm = gmtime(&(file_stat.st_mtime));
  if( (mod_tm->tm_year < (now_tm.tm_year-1)) && (mod_tm->tm_mon < now_tm.tm_mon) && (mod_tm->tm_mday < now_tm.tm_mday))
    strftime(file_time, 255, "%Y %d %R", mod_tm);
  else	
    strftime(file_time, 255, "%b %d %R", mod_tm);


  return sprintf(stat_str, "%crw-rw-rw- 1 ftp ftp %u %s %s", S_ISDIR(file_stat.st_mode)?'d':'-', (unsigned int)(file_stat.st_size), file_time, filename);
}

int file_exists(const char *abs_name)
{
  struct stat tmp;
  return !stat(abs_name, &tmp);
}

size_t file_size(const char *abs_name)
{
  struct stat file_stat;
  stat(abs_name, &file_stat);
  
  return file_stat.st_size;
}

int file_is_dir(const char *abs_name)
{
  struct stat file_stat;
  stat(abs_name, &file_stat);

  return S_ISDIR(file_stat.st_mode);
}

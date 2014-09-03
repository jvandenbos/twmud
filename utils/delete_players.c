#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <dirent.h>
#include <time.h>

#define DIR_PREFIX "../lib/players.d"
#define EXPIRE_TIME 30

void main() {
  
  char dir[100];
  char sd;
  DIR *dp;
  struct dirent *de;
  struct stat statbuf;
  char pathname[100];
  time_t delete_time;

  delete_time = time(NULL) - (EXPIRE_TIME * 86400);

  for (sd = 'a'; sd <= 'z'; sd++) {
    sprintf(dir,"%s/%c",DIR_PREFIX, sd);
    printf("Examining directory: %c\n\r",sd);

    if (!(dp = opendir(dir))) {
      perror(dir);
      break;
    }

    while ((de = readdir(dp)))
      if (strcmp(".",de->d_name) && strcmp("..",de->d_name)) {
	sprintf(pathname,"%s/%s",dir,de->d_name);
	if (stat(pathname,&statbuf)) {
	  perror(pathname);
	  continue;
	}

	if (statbuf.st_mtime < delete_time) {
	  printf("Removing player: %s\n\rLast Access: %s\n\r\n\r",
		 de->d_name, ctime(&statbuf.st_mtime));
	  remove(pathname);
	}
      }
    closedir(dp);
  }
}
      




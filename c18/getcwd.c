/*  getcwd.c
 *
 *  This program gets the current working directory
 *
 *  Usage: ./getcwd
 */

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef PATH_MAX
  int path_max = PATH_MAX;
#else
  int path_max = pathconf(path, _PC_PATH_MAX);
  if (path_max <= 0) {
    path_max = 4096;
  }
#endif

void pexit(const char *fCall);
char * _getcwd(char *, size_t);

int main(int argc, char *argv[]) {
  char buffer[path_max];

  if (_getcwd(buffer, path_max) == NULL) {
    pexit("_getcwd");
  }

  printf("%s\n", buffer);

  return EXIT_SUCCESS;
}

char * _getcwd(char *buffer, size_t size) {
  int cwd_fd = open(".", O_RDONLY);
  int path_size;
  DIR *dir;
  char current_file[path_max], temp[size];
  struct dirent *entry;
  struct stat cwd_stat, parent_stat;

  if (cwd_fd < 0) {
    pexit("open");
  }

  while(1) {
    if (stat(".", &cwd_stat) == -1) {
      return NULL;
    }

    if (stat("..", &parent_stat) == -1) {
      return NULL;
    }

    // Check if we are at the '/' level
    if (cwd_stat.st_dev == parent_stat.st_dev && cwd_stat.st_ino == parent_stat.st_ino) {
      snprintf(temp, size, "%s", buffer);
      path_size = snprintf(buffer, size, "/%s", temp);
      break;
    }

    dir = opendir("..");

    if (dir == NULL) {
      return NULL;
    }

    errno = 0;
    entry = readdir(dir);

    while (entry != NULL) {
      if (strncmp(entry->d_name, ".", path_max) != 0 && strncmp(entry->d_name, "..", path_max) != 0) {
        snprintf(current_file, path_max, "../%s", entry->d_name);

        if (stat(current_file, &parent_stat) == -1) {
          return NULL;
        }

        if (cwd_stat.st_dev == parent_stat.st_dev && cwd_stat.st_ino == parent_stat.st_ino) {
          snprintf(temp, size, "%s", buffer);
          snprintf(buffer, size, "%s/%s", entry->d_name, temp);
          break;
        }
      }

      entry = readdir(dir);
    }

    if (errno != 0) {
      pexit("readdir");
    }

    if (closedir(dir) == -1) {
      return NULL;
    }

    if (chdir("..") == -1) {
      return NULL;
    }
  }

  if (fchdir(cwd_fd) == -1) {
    return NULL;
  }

  if (close(cwd_fd) == -1) {
    return NULL;
  }

  if (path_size > size) {
    errno = ERANGE;
    return NULL;
  }

  return buffer;
}

void pexit(const char *fCall) {
  perror(fCall);
  exit(EXIT_FAILURE);
}

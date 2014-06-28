/*
 * Modify the program in Listing 18-2 (list_files.c) to use readdir_r()
 * instead of readdir().
 */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <stddef.h>

static void listFiles(const char *);
void pexit(const char *fCall);

int main (int argc, char *argv[]) {
  if (argc == 1) {
    listFiles(".");
  } else {
    for (argv++; *argv; argv++) {
      listFiles(*argv);
    }
  }

  exit(EXIT_SUCCESS);
}

/* List all files in directory 'dirpath' */
static void listFiles(const char *dirpath) {
  DIR *dirp;
  struct dirent *result, *entryp;
  int max_name;
  int isCurrent = (strncmp(dirpath, ".", 1) == 0);

  max_name = pathconf(dirpath, _PC_NAME_MAX);
  if (max_name == -1) {
    max_name = 255;
  }

  entryp = malloc(offsetof(struct dirent, d_name) + max_name + 1);
  if (entryp == NULL) {
    pexit("malloc");
  }

  dirp = opendir(dirpath);
  if (dirp == NULL) {
    pexit("opendir");
  }

  for (;;) {
    errno = readdir_r(dirp, entryp, &result);

    if (errno != 0) {
      pexit("readdir_r");
    }

    if (result == NULL) {
      break;
    }

    if (strncmp(entryp->d_name, ".", max_name) == 0 || strncmp(entryp->d_name, "..", max_name) == 0) {
      continue;
    }

    if (!isCurrent) {
      printf("%s/", dirpath);
    }

    printf("%s\n", entryp->d_name);
  }

  if (closedir(dirp) == -1) {
    pexit("closedir");
  }
}

void pexit(const char *fCall) {
  perror(fCall);
  exit(EXIT_FAILURE);
}

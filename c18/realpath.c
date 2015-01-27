/*
 * Here we are implementing `realpath()`, it works like this:
 *
 * char *realpath(const char *pathname, char *resolved_path);
 * Returns pointer to resolved pathname on success, or NULL on error
 *
 * Our program use:  ./real_path <path>
 */

#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>

#ifdef PATH_MAX
  int path_max = PATH_MAX;
#else
  int path_max = pathconf(path, _PC_PATH_MAX);
  if (path_max <= 0) {
    path_max = 4096;
  }
#endif

void helpAndLeave(const char *progname, int status);
void pexit(const char *fCall);
char * _realpath(const char *, char *);

int main (int argc, char *argv[]) {
  if (argc != 2) {
    helpAndLeave(argv[0], EXIT_FAILURE);
  }

  char buf[path_max];

  char *path = _realpath(argv[1], buf);

  if (path != NULL) {
    printf("%s\n", path);
  }

  exit(EXIT_SUCCESS);
}

char * _realpath(const char *path, char *resolved_path) {
  char resolved_symlink[path_max];
  char path_dup[PATH_MAX];
  char *resolved_root = resolved_path;
  int bytesPlaced;

  strncpy(path_dup, path, PATH_MAX);
  path = path_dup;

  if (*path != '/') {
    if (getcwd(resolved_path, PATH_MAX - 1) == NULL) {
      return NULL;
    }

    resolved_path += strlen(resolved_path);

    if (*(resolved_path - 1) != '/') {
      *resolved_path = '/';
      resolved_path++;
      *resolved_path = '\0';
    }
  } else {
    path++;
    *resolved_path = '/';
    *(resolved_path++) = '\0';
  }

  while (*path != '\0') {
    if (*path == '/') {
      path++;
      continue;
    }

    if (*path == '.') {
      if (*(path + 1) == '/' || *(path + 1) == '\0') {
        path++;
      } else if(*(path + 1) == '.') {
        if (*(path + 2) == '/' || *(path + 2) == '\0') {
          path += 2;

          if (resolved_path == resolved_root + 1) {
            continue;
          }

          if (*(resolved_path - 1) == '/') {
            resolved_path--;
          }

          while (*(resolved_path - 1) != '/' && resolved_path != resolved_root) {
            *(resolved_path--) = '\0';
          }

          continue;
        }
      }
    }

    while (*path != '\0' && *path != '/')
    {
       *(resolved_path++) = *(path++);
    }

    // After we append a new path we check if it is a symlink
    *resolved_path = '\0';
    bytesPlaced = readlink(resolved_root, resolved_symlink, PATH_MAX - 1);

    if (bytesPlaced == -1) {
      if (errno != EINVAL) {
        return NULL;
      }
    } else {
      resolved_symlink[bytesPlaced] = '\0';

      if (*resolved_symlink != '/') {
        while (*(resolved_path - 1) != '/') {
          resolved_path--;
        }
      } else {
        resolved_path = resolved_root;
      }

      strncat(resolved_symlink, path, PATH_MAX - 1);
      strncpy(path_dup, resolved_symlink, PATH_MAX - 1);
      path = path_dup;
    }
  }

  *resolved_path = '\0';
  return resolved_root;
}

void helpAndLeave(const char *progname, int status) {
  FILE *stream = stderr;

  if (status == EXIT_SUCCESS) {
    stream = stdout;
  }

  fprintf(stream, "Usage: %s path", progname);
  exit(status);
}

void pexit(const char *fCall) {
  perror(fCall);
  exit(EXIT_FAILURE);
}

/* guard.c
 *
 * Log events from directory and sub-directories \o/
 *
 *  Usage: ./guard <path>
 */

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ftw.h>
#include <sys/inotify.h>
#include <limits.h>

#define SIZE (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

static int guard(const char *, const struct stat *, int, struct FTW *);
static void displayInotifyEvent(struct inotify_event *);
void helpAndLeave(const char *progname, int status);
void pexit(const char *fCall);

int inotifyFd;

int main(int argc, char *argv[]) {
  char buffer[SIZE];
  ssize_t numRead;
  char *p;
  struct inotify_event *event;

  if (argc < 2) {
    helpAndLeave(argv[0], EXIT_FAILURE);
  }

  inotifyFd = inotify_init();

  if (inotifyFd == -1) {
    pexit("inotify_init");
  }

  if (nftw(argv[1], &guard, 10, FTW_PHYS) == -1) {
    pexit("nftw");
  }

  while(1) {
    numRead = read(inotifyFd, buffer, SIZE);

    if (numRead == 0) {
      printf("read() from inotify fd returned 0!");
      exit(EXIT_FAILURE);
    }

    if (numRead == -1) {
      pexit("read");
    }

    for (p = buffer; p < buffer + numRead;) {
      event = (struct inotify_event *) p;
      displayInotifyEvent(event);
      p += sizeof(struct inotify_event) + event->len;
    }
  }

  exit(EXIT_SUCCESS);
}

static int guard(const char *path, const struct stat *sb, int flag, struct FTW *ftwb) {
    int wd;

    switch (sb->st_mode & S_IFMT) {
      case S_IFDIR:
        wd = inotify_add_watch(inotifyFd, path, IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);

        if (wd == -1) {
          pexit("inotify_add_watch");
        }

        printf("Watching %s using wd %d\n", path, wd);
      break;
    }

    return 0;
}

static void displayInotifyEvent(struct inotify_event *i) {
    printf("    wd =%2d; ", i->wd);

    if (i->cookie > 0) {
      printf("cookie =%4d; ", i->cookie);
    }

    if (i->mask & IN_CREATE && i->len > 0) {
      printf("File %s was created!\n", i->name);
    }

    if (i->mask & IN_DELETE && i->len > 0) {
      printf("File %s was deleted!\n", i->name);
    }

    if (i->mask & IN_MOVED_FROM && i->len > 0) {
      printf("File renamed from: %s\n", i->name);
    }

    if (i->mask & IN_MOVED_TO && i->len > 0) {
      printf("To: %s\n", i->name);
    }
}

void helpAndLeave(const char *progname, int status) {
  FILE *stream = stderr;

  if (status == EXIT_SUCCESS) {
    stream = stdout;
  }

  fprintf(stream, "Usage: %s <path>", progname);
  exit(status);
}

void pexit(const char *fCall) {
  perror(fCall);
  exit(EXIT_FAILURE);
}

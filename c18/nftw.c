/*  nftw.c
 *
 *  Uses nftw() to traverse a directory tree and finishes by printing out
 *  counts and percentages of the various types (regular, directory,
 *  symbolic link, and so on) of files in the tree.
 *
 *  Usage: ./nftw
 */

#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <ftw.h>

#define TYPES 7
typedef enum { REGULAR, DIRECTORY, CHAR, BLOCK, LINK, FIFO, SOCKET } Types;
static int types_quantity[7];


static int parse_files(const char *, const struct stat *, int, struct FTW *);
void helpAndLeave(const char *progname, int status);
void pexit(const char *fCall);

int main(int argc, char *argv[]) {
  int i, total = 0;

  if (argc < 2) {
    helpAndLeave(argv[0], EXIT_FAILURE);
  }

  if (nftw(argv[1], &parse_files, 10, FTW_PHYS) == -1) {
    pexit("nftw");
  }

  for (i = 0; i < TYPES; i++) {
    total += types_quantity[i];
  }

  printf("Regular: %d %f\n", types_quantity[REGULAR], (types_quantity[REGULAR] * 100.0) / total);
  printf("Directory: %d %f\n", types_quantity[DIRECTORY], (types_quantity[DIRECTORY] * 100.0) / total);
  printf("Char: %d %f\n", types_quantity[CHAR], (types_quantity[CHAR] * 100.0) / total);
  printf("Block: %d %f\n", types_quantity[BLOCK], (types_quantity[BLOCK] * 100.0) / total);
  printf("Link: %d %f\n", types_quantity[LINK], (types_quantity[LINK] * 100.0) / total);
  printf("Fifo: %d %f\n", types_quantity[FIFO], (types_quantity[FIFO] * 100.0) / total);
  printf("Socket: %d %f\n", types_quantity[SOCKET], (types_quantity[SOCKET] * 100.0) / total);

  exit(EXIT_SUCCESS);
}

static int parse_files(const char *path, const struct stat *sb, int flag, struct FTW *ftwb)
{
    switch (sb->st_mode & S_IFMT) {
      case S_IFREG:  types_quantity[REGULAR]++;    break;
      case S_IFDIR:  types_quantity[DIRECTORY]++;    break;
      case S_IFCHR:  types_quantity[CHAR]++;   break;
      case S_IFBLK:  types_quantity[BLOCK]++;  break;
      case S_IFLNK:  types_quantity[LINK]++;  break;
      case S_IFIFO:  types_quantity[FIFO]++;   break;
      case S_IFSOCK: types_quantity[SOCKET]++; break;
    }

    return 0;
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

#include <unistd.h>   // Include constants like STDIN_FILENO
#include <fcntl.h>    // Include constants like O_CREAT, for file creation
#include <sys/stat.h> // Include constants like S_IRUSR, for file permissions
#include <sys/uio.h>
#include "tlpi_hdr.h"

static const size_t BUFFER_SIZE = 25;

typedef struct my_iovec {
  void *base; /* Start address of buffer */
  size_t len; /* Number of bytes to transfer to/from buffer */
} Iovec;

void error(const char *);
int openFile(char *);
ssize_t readv_dup(int fd, const Iovec *iov, int iovcount);
ssize_t writev_dup(int fd, const Iovec *iov, int iovcount);

int main(int argc, char *argv[]) {
  int fd;
  ssize_t bytesRead, bytesWritten, totalBytesRead, totalBytesWritten;
  Iovec iov[2];
  char buffer1[BUFFER_SIZE], buffer2[BUFFER_SIZE];

  if (argc < 2 || strcmp(argv[1], "--help") == 0) {
    usageErr("%s filename\n", argv[0]);
  }

  fd = openFile(argv[1]);

  iov[0].base = buffer1;
  iov[0].len  = BUFFER_SIZE;

  iov[1].base = buffer2;
  iov[1].len  = BUFFER_SIZE;

  totalBytesRead = totalBytesWritten = 2 * BUFFER_SIZE;

  bytesRead = readv_dup(fd, iov, 2);

  printf("Total bytes requested: %ld; bytes read: %ld\n", (long) totalBytesRead, (long) bytesRead);

  if (lseek(fd, 0, SEEK_END) == -1) {
    error("seeking to the end of the file");
  }

  bytesWritten = writev_dup(fd, iov, 2);

  printf("Total bytes requested: %ld; bytes written: %ld\n", (long) totalBytesWritten, (long) bytesWritten);

  if (close(fd) == -1) {
    error("Failed to close file");
  }

  exit(EXIT_SUCCESS);
}

void error(const char *message) {
  printf("%s\n", message);

  exit(EXIT_FAILURE);
}

int openFile(char *name)
{
  int fd;

  fd = open(name, O_RDWR);

  if (fd == -1) {
    errExit("opening file");
  }

  return fd;
}

ssize_t readv_dup(int fd, const Iovec *iov, int iovcount) {
  ssize_t numRead, totalRead = 0;
  int i;

  for (i = 0; i < iovcount; i++) {
    numRead = read(fd, iov[i].base, iov[i].len);

    if (numRead == -1) {
      error("Couldn't read from file");
    }

    totalRead += numRead;
  }

  return totalRead;
}

ssize_t writev_dup(int fd, const Iovec *iov, int iovcount) {
  ssize_t numWritten, totalWritten = 0;
  int i;

  for (i = 0; i < iovcount; i++) {
    numWritten = write(fd, iov[i].base, iov[i].len);

    if (numWritten != iov[i].len) {
      error("Couldn't write to file");
    }

    totalWritten += numWritten;
  }

  return totalWritten;
}

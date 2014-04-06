/*
    Implementing getpwnam() using setpwent(), getpwent(), and endpwent().
*/

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>

struct passwd * _getpwnam(const char *name);

int main(int argc, char *argv[]) {
  struct passwd *record;

  record = _getpwnam("bernardo");

  if (record != NULL) {
    printf("User %s found!\n", record->pw_name);
  }

  record = _getpwnam("test");

  if (record != NULL) {
    printf("User %s found!\n", record->pw_name);
  }

  exit(EXIT_SUCCESS);
}

struct passwd * _getpwnam(const char *name) {
  struct passwd *record;

  while ((record = getpwent()) != NULL) {
    if (strcmp(record->pw_name, name) == 0) {
      endpwent();

      return record;
    }
  }

  endpwent();

  return NULL;
}

/* display_acl.c
 *
 * Displays the permissions from the ACL entry that corresponds to a particular
 * user or group. If the ACL entry that corresponds to the given user or group
 * falls into the group class, then the program should additionally display the
 * permissions that would apply after the ACL entry has been modified by the ACL
 * mask entry.
 *
 * Usage: ./display_acl <u,g> <name, id> <file>
**/

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <string.h>
#include <sys/acl.h>
#include <acl/libacl.h>

typedef enum { USER, GROUP } chtype;

void helpAndLeave(const char *progname, int status);
void pexit(const char *fCall);
uid_t userIdFromName(const char *name);
gid_t groupIdFromName(const char *name);
void printPermissions(acl_entry_t, acl_entry_t, int);

int main(int argc, char *argv[])
{
  acl_t acl;
  acl_entry_t entry = NULL, selectedEntry = NULL, maskEntry = NULL, otherEntry = NULL;
  acl_tag_t tag_type, selectedTagType;
  int type, entryId, moreEntries;
  unsigned int id, entryOwnerId;

  if (argc != 4) {
    helpAndLeave(argv[0], EXIT_FAILURE);
  }

  switch (*argv[1]) {
    case 'u': type = USER;  break;
    case 'g': type = GROUP; break;
    default:  helpAndLeave(argv[0], EXIT_FAILURE);
  }

  if (type == USER) {
    id = userIdFromName(argv[2]);
  } else {
    id = groupIdFromName(argv[2]);
  }

  if (id == -1) {
    fprintf(stderr, "Invalid user or group identifier: %s", argv[2]);
    exit(EXIT_FAILURE);
  }

  acl = acl_get_file(argv[3], ACL_TYPE_ACCESS);

  if (acl == NULL) {
    pexit("acl_get_file");
  }

  for (entryId = ACL_FIRST_ENTRY; ; entryId = ACL_NEXT_ENTRY) {
    moreEntries = acl_get_entry(acl, entryId, &entry);

    if (moreEntries == -1) {
      pexit("acl_get_entry");
    } else if (moreEntries == 0) {
      break;
    }

    if (acl_get_tag_type(entry, &tag_type) == -1) {
      pexit("acl_get_tag_type");
    }

    if (tag_type == ACL_MASK) {
      maskEntry = entry;
    } else if (tag_type == ACL_OTHER) {
      otherEntry = entry;
    } else {
      if (tag_type == ACL_USER_OBJ || tag_type == ACL_USER) {
        entryOwnerId = userIdFromName(acl_get_qualifier(entry));
      } else {
        entryOwnerId = groupIdFromName(acl_get_qualifier(entry));
      }

      if (id == entryOwnerId) {
        selectedEntry = entry;
        selectedTagType = tag_type;
        break;
      }
    }
  }

  if (selectedEntry != NULL) {
    if (selectedTagType == ACL_USER_OBJ) {
      printPermissions(selectedEntry, maskEntry, 0);
    } else {
      if (maskEntry == NULL) {
        fprintf(stderr, "Necessary ACL_MASK not present.");
        exit(EXIT_FAILURE);
      }

      printPermissions(selectedEntry, maskEntry, 1);
    }
  } else {
    printPermissions(otherEntry, maskEntry, 0);
  }

  acl_free(acl);

  exit(EXIT_SUCCESS);
}

void helpAndLeave(const char *progname, int status) {
  FILE *stream = stderr;

  if (status == EXIT_SUCCESS) {
    stream = stdout;
  }

  fprintf(stream, "Usage: %s <u,g> <id,name> <file>", progname);
  exit(status);
}

void pexit(const char *fCall) {
  perror(fCall);
  exit(EXIT_FAILURE);
}

uid_t userIdFromName(const char *name) {
  struct passwd *pwd;
  uid_t uid;
  char *endptr;

  if (name == NULL || *name == '\0') {
    return -1;
  }

  uid = strtol(name, &endptr, 10);

  if (*endptr == '\0') {
    return uid;
  }

  pwd = getpwnam(name);

  if (pwd == NULL) {
    return -1;
  }

  return pwd->pw_uid;
}

gid_t groupIdFromName(const char *name) {
  struct group *grp;
  gid_t gid;
  char *endptr;

  if (name == NULL || *name == '\0') {
    return -1;
  }

  gid = strtol(name, &endptr, 10);

  if (*endptr == '\0') {
    return gid;
  }

  grp = getgrnam(name);

  if (grp == NULL) {
    return -1;
  }

  return grp->gr_gid;
}

void printPermissions(acl_entry_t entry, acl_entry_t mask, int applyMask) {
  acl_permset_t permissionSet, maskPermissionSet = NULL;
  char permissions[] = "---";
  int maskPermissions[3] = { 1 };
  int entryOk, maskOk;

  if (acl_get_permset(entry, &permissionSet) == -1) {
    pexit("acl_get_permset");
  }

  if (applyMask) {
    if (acl_get_permset(mask, &maskPermissionSet) == -1) {
      pexit("acl_get_permset");
    }

    maskOk = acl_get_perm(maskPermissionSet, ACL_READ);
    if (maskOk == -1) maskOk = 0;
    maskPermissions[0] = maskOk;

    maskOk = acl_get_perm(maskPermissionSet, ACL_WRITE);
    if (maskOk == -1) maskOk = 0;
    maskPermissions[1] = maskOk;

    maskOk = acl_get_perm(maskPermissionSet, ACL_EXECUTE);
    if (maskOk == -1) maskOk = 0;
    maskPermissions[2] = maskOk;
  }

  entryOk = acl_get_perm(permissionSet, ACL_READ);

  if (entryOk == -1) {
    pexit("acl_get_perm");
  }

  if (entryOk && maskPermissions[0]) {
    permissions[0] = 'r';
  }

  entryOk = acl_get_perm(permissionSet, ACL_WRITE);

  if (entryOk == -1) {
    pexit("acl_get_perm");
  }

  if (entryOk && maskPermissions[1]) {
    permissions[1] = 'w';
  }

  entryOk = acl_get_perm(permissionSet, ACL_EXECUTE);

  if (entryOk == -1) {
    pexit("acl_get_perm");
  }

  if (entryOk && maskPermissions[2]) {
    permissions[2] = 'x';
  }

  printf("%s\n", permissions);
}

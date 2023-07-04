#include <errno.h>
#include <stdio.h>

int execv (char *image, char **argv)

{
  fprintf (stderr, "this version doesn't support %s\n", image);
  errno = ENOSYS;
  return (-1);
}

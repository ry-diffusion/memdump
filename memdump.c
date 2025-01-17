#include "inject.h"

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

internal FILE *fp;

use(free, void, void *);

OnInjected {
  char buff[PATH_MAX] = {0};
  const char *memdump_quiet = getenv("MEMDUMP_QUIET");

  if (!getcwd(buff, PATH_MAX)) {
    fputs("[memdump] unable to get current working directory!\n", stderr);
    abort();
  }

  sprintf(buff, "%s/memdump_output-%d", buff, getpid());
  fp = fopen(buff, "w");

  if (!fp) {
    perror(buff);
    _exit(1);
  }

  if (memdump_quiet && *memdump_quiet == '1') {
    unsetenv("MEMDUMP_QUIET");
    return;
  }

  sprintf(buff, "[memdump] injected to %d\n", getpid());
  fputs(buff, stderr);
}

OnDestructed {
  if (fp)
    fclose(fp);

  drop(free);
}

void free(void *ptr) {
  import(free);

  size_t size = sizeOfBlock(ptr);

  if (fp && ptr) {
    fprintf(fp, "%p: ", ptr);
    fwrite(ptr, size, 1, fp);
    fputs("\n", fp);
  }

  return call(free, ptr);
}
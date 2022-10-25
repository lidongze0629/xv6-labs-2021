#include "kernel/types.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "user/user.h"

#define MAX_LINE_LEN 512

int
readline(char *buf) {
  char *p;
  int n;

  p = buf;
  while ((n = read(0, p, 1)) != 0) {
    if (*p == '\n') {
      *p = 0;
      return 1;
    }
    p++;
    if (p >= buf + MAX_LINE_LEN) {
      fprintf(2, "line too long\n");
      exit(1);
    }
  }

  *p = 0;
  return 0;
}


int
main(int argc, char *argv[]) {
  char* nargv[MAXARG];  // new argv 
  char **p = nargv;

  // defaults to "echo" command
  if (argc < 2) {
    *p++ = "echo";
  } else {
    *p++ = argv[1];
  }

  // copy other params
  for (int i = 2; i < argc; ++i) {
    *p++ = argv[i];
  }
  
  // read from standard input
  char buf[MAX_LINE_LEN];
  while (readline(buf)) {
    char **p_copy = p;
    if (fork() == 0) {
      *p_copy = buf;
      exec(nargv[0], nargv);
    }
    wait((int *)0);
  }
  
  exit(0);
}

#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[]) {

  int p[2];
  char buf[4];

  if (pipe(p) < 0) {
    fprintf(2, "pipe error\n");
    exit(1);
  }

  if (fork() == 0) {
    // child: recevied ping from parent
    read(p[0], buf, sizeof(buf));
    printf("%d: received %s\n", getpid(), buf);
    close(p[0]);
    // send pong to parent
    write(p[1], "pong", 4);
    close(p[1]);
    exit(0);
  } else {
    // parent: send ping to child
    write(p[1], "ping", 4);
    close(p[1]);
    // wait for child process exit
    wait((int *)0);
    // recevied pong from child
    read(p[0], buf, sizeof(buf));
    printf("%d: received %s\n", getpid(), buf);
    close(p[0]);
    exit(0);
  }
}

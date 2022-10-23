#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
fork1(void) {
  int pid;
  pid = fork();
  if (pid == -1) {
    fprintf(2, "fork error\n");
    exit(1);
  }
  return pid;
}

int
create_pipe(int *p) {
  // printf("pid %d access.\n", getpid());
  // close the write descriptors cause don't need to write to parent pipe
  close(p[1]);

  int prime = -1;
  int rfln = -1;  // read_from_left_neighbor

  int child_pipe_exist = 0;
  int _p[2]; 

  // read returns zero when the write-side of a pipe is closed.
  while (read(p[0], &rfln, sizeof(rfln)) > 0) {
    if (prime == -1) {
      printf("prime %d\n", rfln);
      prime = rfln;
    } else {
      if ((rfln % prime) != 0) {
        // need to pass to the next pipe of pipeline
        if (!child_pipe_exist) {
          // create a new pipe
          if (pipe(_p) < 0) {
            fprintf(2, "pipe error\n");
            exit(1);
          }
          if (fork1() == 0) {
            // child
            create_pipe(_p);
          } else {
            // close the read descriptors
            close(_p[0]);
          }
          // set child_pipe_exist
          child_pipe_exist = 1;
        }
        write(_p[1], &rfln, sizeof(rfln));
      }
      // else do nothing but drop
    }
  }

  // close the read descriptors
  close(p[0]);

  // close the write descriptors if exists
  if (child_pipe_exist) {
    // wait after closed
    close(_p[1]);
    wait((int *)0);
  }

  exit(0);
}

int
main(int argc, char *argv[]) {
 
  int p[2];

  if (pipe(p) < 0) {
    fprintf(2, "pipe error\n");
    exit(1);
  }

  if (fork1() == 0) {
    // child
    create_pipe(p);
  } else {
    // parent: feeds the numbers 2 through 35 into the pipeline
    // close the read descriptors
    close(p[0]);
    for (int i = 2; i <= 35; ++i) {
      // It's simplest to directly write 4-byte ints to the pipes
      write(p[1], &i, sizeof(i));
    }
    // close the write descriptors
    close(p[1]);
    wait((int *)0);
  }

  exit(0);
}

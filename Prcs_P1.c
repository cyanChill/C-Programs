#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
  int fd1 = open("destination1.txt", O_CREAT, 0777);
  if (fd1 < 0) { perror("creation"); }

  int fd2 = open("destination2.txt", O_CREAT, 0777);
  if (fd1 < 0) { perror("creation"); }

  if (close(fd1) < 0) { perror("close"); }
  if (close(fd2) < 0) { perror("close"); }
}

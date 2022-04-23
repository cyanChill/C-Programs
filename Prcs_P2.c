#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
  int srcfd = open("source.txt", O_RDONLY);
  int fd1 = open("destination1.txt", O_WRONLY);
  int fd2 = open("destination2.txt", O_WRONLY);

  // If we failed to open any of these files with those permissions
  if (srcfd < 0 || fd1 < 0 || fd2 < 0) { 
    perror("open");
    return -1;
  }

  char buf;
  int count = 0;
  while (read(srcfd, &buf, 1) > 0) {
    // First 100 characters goes to "destination1.txt"
    if (count < 100) { // 0-99 is 100 numbers
      // If buff = '1', we write "L"
      if (buf == '1') { write(fd1, "L", 1); }
      else { write(fd1, &buf, 1); }
    // Otherwise next 50 characters goes to "destination2.txt"
    } else { // 100-149 is 50 numbers
      // If buff = '1', we write "L"
      if (buf == '3') { write(fd2, "E", 1); }
      else { write(fd2, &buf, 1); }
    }

    count++;

    // Reset count after we looped through the pattern
    if (count == 150) { count = 0; }
  }

  // Close the files
  if (close(srcfd) < 0) { perror("close"); }
  if (close(fd1) < 0) { perror("close"); }
  if (close(fd2) < 0) { perror("close"); }
}

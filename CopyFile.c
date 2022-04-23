#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void handleFileError(char* filepath);

int main (int argc, char* argv[]) {
  char* sourceFile = argv[1];
  char* destinationFile = argv[2];
  int fd1, fd2;

  // Open the files & do error handling
  fd1 = open(sourceFile, O_RDONLY);

  if (fd1 == -1) {
    handleFileError(sourceFile);
    return -1;
  }

  // Creates the destination file if it doesn't exist
  fd2 = open(destinationFile, O_WRONLY | O_CREAT, 0731);
  if (fd2 == -1) {
    handleFileError(destinationFile);
    return -1;
  }

  const int readAmount = 100;
  int bytesread, byteswritten;
  char buff[readAmount];

  // Read every 100 bytes & write those bytes to the end of the destination file
  do {
    bytesread = read(fd1, buff, readAmount);

    // Handle read errors
    if (bytesread == -1) {
      printf("Encountered read error with %s", sourceFile);
      perror("Error");
      return -1;
    }

    byteswritten = write(fd2, buff, bytesread);

    // Handles write errors
    if (byteswritten == -1) {
      printf("Encountered write error with %s", destinationFile);
      perror("Error");
      return -1;
    }
  } while (bytesread > 0);

  // Close each file
  if (close(fd1) < 0) {
    printf("Failed to close %s\n", sourceFile);
  }

  if (close(fd2) < 0) {
    printf("Failed to close %s\n", destinationFile);
  }

  return 0;
}

void handleFileError(char* filepath) {
  if (errno == ENOENT) {
    printf("%s does not exist\n", filepath);
  } else if (errno == EACCES) {
    printf("%s is not accessible\n", filepath);
  } else {
    perror("Error");
  }
}

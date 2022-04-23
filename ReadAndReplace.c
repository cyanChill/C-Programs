#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

void handleFileError(char* filepath);
void handleWriteError(char* filepath);
char* replaceChars (char readStr[]);

int main (int argc, char* argv[]) {
  char* sourceFile = argv[1];
  if (!sourceFile) {
    sourceFile = "source.txt";
  }
  char* destinationFile = argv[2];
  if (!destinationFile) { 
    destinationFile = "destination.txt";
  }
  int fd1, fd2;

  // Open the files & do error handling
  fd1 = open(sourceFile, O_RDONLY);
  if (fd1 == -1) {
    handleFileError(sourceFile);
    return -1;
  }

  fd2 = open(destinationFile, O_WRONLY | O_CREAT, 0731);
  if (fd2 == -1) {
    handleFileError(destinationFile);
    return -1;
  }

  int bytesread, byteswritten;
  char buff[100];

  do {
    bytesread = read(fd1, buff, 100);

    // Handle read errors
    if (bytesread == -1) {
      printf("Encountered read error with %s", sourceFile);
      perror("Error");
      return -1;
    }

    if (bytesread > 0) {
      /* 
        Not wise to combine the writes into a single write due to
        previous content still being in the buffer (if we didn't)
        read 100 bytes
      */
      byteswritten = write(fd2, replaceChars(buff), bytesread);
      // Handles write errors
      if (byteswritten == -1) {
        handleWriteError(destinationFile);
        return -1;
      }

      byteswritten = write(fd2, "XYZ", 3);
      if (byteswritten == -1) {
        handleWriteError(destinationFile);
        return -1;
      }
    }
  } while (bytesread > 0);

  // Close each file
  if (close(fd1) < 0) {
    printf("Failed to close source.txt\n");
  }

  if (close(fd2) < 0) {
    printf("Failed to close destination\n");
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

void handleWriteError(char* filepath) {
  printf("Encountered write error with %s", filepath);
    perror("Error");
}

// Replaces all "1" (one) with a "L"
char* replaceChars (char readStr[]) {
  int i;
  int len = strlen(readStr);

  for (i = 0; i < len; i++) {
    if (readStr[i] == '1') {
      readStr[i] = 'L';
    }
  }
  return readStr;
}

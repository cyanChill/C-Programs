#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void handleFileError(char* filepath);

int main (int argc, char* argv[]) {
  char* filename = argv[1];
  int filedescriptor;

  // Open the file
  filedescriptor = open(filename, O_RDONLY);
  if (filedescriptor == -1) {
    handleFileError(filename);
    return -1;
  }

  const int readAmount = 100;
  int bytesread;
  char buff[readAmount];

  // Read every 100 bytes, leaving the loop if we read 0 bytes
  do {
    bytesread = read(filedescriptor, buff, readAmount);

    // If we somehow encounter an error when reading
    if (bytesread == -1) {
      printf("Encountered read error with %s", filename);
      perror("Error");
      return -1;
    }

    /* 
      Need precision flag as if we don't read "readAmount" bytes, leftover
      data from the previous read will still be in "buff"
    */
    printf("%.*s", bytesread, buff);
  } while (bytesread > 0);

  // Close the file
  if (close(filedescriptor) < 0) {
    printf("Failed to close %s\n", filename);
    return -1;
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

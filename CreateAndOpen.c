#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

void handleFileError(char* filepath);

int main (int argc, char* argv[]) {
  char* filename = argv[1];
  int returnval;

  // Create (if the file doesn't exist) & Open a file in Read Mode
  returnval = open(filename, O_RDONLY | O_CREAT, 0731);
  if (returnval > 0) {
    printf("%s has been opened in Read Mode (and created if it previously didn't exist)\n", filename);
  } else {
    handleFileError(filename);
    return -1;
  }

  // Close the file
  if (close(returnval) < 0) {
    printf("Failed to close %s\n", filename);
    return -1;
  }

  printf("Successfully closed %s\n", filename);
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

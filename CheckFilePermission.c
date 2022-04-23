#include <stdio.h>
#include <unistd.h>
#include <errno.h>

void handleFileError(char* filepath);

int main (int argc, char* argv[]) {
  char* filepath = argv[1];
  int returnval;

  // Check file existence
  returnval = access(filepath, F_OK);
  if (returnval == 0) {
    printf("%s exists\n", filepath);
  } else {
    handleFileError(filepath);
    // Return as no need for addition checks for Read & Write as file doesn't exist
    return -1; 
  }

  // Check read access
  returnval = access(filepath, R_OK);
  if (returnval == 0) {
    printf("%s can be read\n", filepath);
  } else {
    handleFileError(filepath);
  }

  // Check write access
  returnval = access(filepath, W_OK);
  if (returnval == 0) {
    printf("%s can be written to\n", filepath);
  } else {
    handleFileError(filepath);
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

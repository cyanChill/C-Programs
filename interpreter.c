#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h> // For exit()

/* 
  Some weirdness with the pipe examples:
   - Running "ps -u username | grep firefox" before "tail /etc/passwd | grep username" will have
     both commands display their correct outputs; but running "tail /etc/passwd | grep username"
     before "ps -u username | grep firefox" will only show the output if we input
     "tail /etc/passwd | grep username" and not for "ps -u username | grep firefox". We can only
     see the output for "ps -u username | grep firefox" if we throw an error with the "ps" command;
     ie: "ps -u username grep firefox", then running "ps -u username | grep firefox" directly afterwards
     will have the output for "ps -u username | grep firefox" show up in the interpeter.
*/

// Structure to mimic input of an executable
typedef struct {
  int numArgs;
  char** argv;
} MyArgs;

char* getCmd ();
char* trim (char* s);
MyArgs splitArgs (char* cmdStr);
void toLowerCase (char* str);
bool shouldExit (char* cmd);
bool needPipe (int argc, char* argv[]);


/*  We assume that we can only pipe once */
int main (int argc, char* argv[]) {
  bool shouldContinue = true;
  pid_t c_pid, c_pid2;
  char* cmdin;
  MyArgs argsInfo;
  int myargc;
  char** myargv;

  do {
    // Get a command from the user
    cmdin = getCmd();
    if (cmdin[0] == '\0') { // If we get a null input
      c_pid = 1;  // To bypass loop condition
      continue;
    }
    argsInfo = splitArgs(cmdin);

    myargc = argsInfo.numArgs;
    myargv = argsInfo.argv;

    // Check if we should exit
    if (myargc == 1 && shouldExit(myargv[0])) {
      shouldContinue = false;
    } else {  // Run the command
      // Check if we need a pipe
      bool doPipe = needPipe(myargc, myargv);

      if (doPipe) {
        // Splitting the args
        bool foundPipe = false;
        int pipePoint = 0;
  
        // Find where the pipe is at in the myargv
        for (int i = 0; i < myargc; i++) {
          if (strcmp(myargv[i], "|") == 0) {
            pipePoint = i;
            break;
          }
        }

        // Populate the arrays
        char** args1Arr = malloc((pipePoint + 1) * sizeof(char*));
        char** args2Arr = malloc((myargc - pipePoint - 1) * sizeof(char*)); 

        // Populate 1st pipe command args
        for (int j = 0; j < pipePoint; j++) { args1Arr[j] = myargv[j]; }
        // Populate 2nd pipe command args
        for (int k = 0; k < myargc - pipePoint - 1; k++) { 
          args2Arr[k] = myargv[k + pipePoint + 1];
        }

        int stdin_cpy = dup(0), stdout_cpy = dup(1);

        int pipefd[2];
        if (pipe(pipefd) == -1) {
          perror("pipe");
          exit(EXIT_FAILURE);
        }

        c_pid = fork();
        if (c_pid == -1) {
          perror("fork");
          exit(EXIT_FAILURE);
        }

        if (c_pid > 0) {  // Parent Process
          waitpid(c_pid, NULL, 0);
          c_pid2 = fork();
        }

        if (c_pid == 0) {  // Child 1 Process
          dup2(pipefd[1], 1); // Redirect stdout to pipe write
          close(pipefd[0]); // Close pipe read

          execvp(args1Arr[0], args1Arr);
          printf("Command Failed\n");
        } else if (c_pid2 == 0) {
          dup2(pipefd[0], 0); // Redirect stdin to pipe read
          close(pipefd[1]); // Close pipe write

          execvp(args2Arr[0], args2Arr);
          printf("Command Failed\n");
        }
        
        if (c_pid > 0 && c_pid2 > 0) {  // Parent Process
          close(pipefd[0]);
          close(pipefd[1]);
          waitpid(c_pid2, NULL, 0);
          c_pid2 = 0; // Reset c_pid2
          // Free up allocated memory
          free(args1Arr);
          free(args2Arr);
          free(argsInfo.argv);
        }

      } else {  // We know we don't need to use pipe
        c_pid = fork(); // Starting command running process
        if (c_pid == -1) {
          perror("fork");
          exit(EXIT_FAILURE);
        }

        if (c_pid > 0) {  // Parent process
          waitpid(c_pid, NULL, 0);  // Wait for child process before proceeding
        } else {  // Child process
          char** nullendargs = realloc(myargv, sizeof(myargv) + sizeof(char*));  // Add 1 spot to array for NULL
          nullendargs[myargc] = NULL;

          execvp(myargv[0], nullendargs);
          printf("Command Failed\n");
        }
      }
    }

  } while (shouldContinue && c_pid > 0);
}

// Function to read command input
char* getCmd () {
  char *cmdin = NULL;
  size_t len = 0;

  printf("command: ");
  // Rather than scanf since scanf would read the previous line if we enter nothing
  getline(&cmdin, &len, stdin);

  // Return Trimmed input:
  return trim(cmdin);
}


// Function to trim a string
char* trim (char* s) {
  char* end;
  // Trim leading spaces
  while (isspace((unsigned char) *s)) s++;
  // All spaces
  if (*s == 0) return s;
  // Trim trailing spaces
  end = s + strlen(s) - 1;
  while (end > s && isspace((unsigned char) *end)) end--;
  // Write new null terminator character
  end[1] = '\0';
  return s;
}


// Function to split the args from the command string obtained from getCmd()
MyArgs splitArgs (char* cmdStr) {
  MyArgs rtnArgs;
  char** argArr;
  int words = 1;

  for (int i = 0; cmdStr[i] != '\0'; i++) {
    // Split by space as we expect the arguments of the command to be seperated by a space
    if (cmdStr[i] == ' ') { words++; }
  }

  argArr = malloc(words * sizeof(char*)); // Allocate space for words in memory
  argArr[0] = strtok(cmdStr, " ");  // Split by space
  for (int j = 1; j < words; j++) {
    argArr[j] = strtok(NULL, " "); // Continuously split cmdStr by space from where it last left off
  }

  rtnArgs.numArgs = words;
  rtnArgs.argv = argArr;

  return rtnArgs;
}


// Function to make a string lowercase
void toLowerCase (char* str) {
  int i, len = strlen(str);
  for (i = 0; i < len; i++) { str[i] = tolower(str[i]); }
}


// Function to check if we entered "exit" as the command
bool shouldExit (char* cmd) {
  char cmdcpy[strlen(cmd)];
  strcpy(cmdcpy, cmd);
  toLowerCase(cmdcpy);

  // If strings are equal, strcmp returns 0
  return strcmp(cmdcpy, "exit") == 0 ? true : false;
}


// Function to check if a pipe is necessary (ie: there is a "|" in the command we entered)
bool needPipe (int argc, char* argv[]) {
  for (int i = 0; i < argc; i++) {
    // Looks for the first "|" in the command
    if (strcmp(argv[i], "|") == 0) { return true; }
  }
  return false;
}

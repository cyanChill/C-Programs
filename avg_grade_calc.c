#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
  int x, y, n;
} Grades;
const int BUFF_SIZE = 255;
const int MAX_GRADE_DIGITS = 32;

Grades getDataInfo (char* filename);
int* closestFactors (int total);
void errorWrap (int code, char* msg);
double** getGradeMatrix (char* filename, Grades fileInfo);
double** getMatrixCols(double** matrix, int to, int from, Grades fileInfo);
double getAvg(double** matrix, int numRows);

int main (int argc, char* argv[]) {
  if (argc != 2) { // Check for invalid args
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char* filename = argv[1];
  // Fetch values for x, y, & n
  Grades fileInfo = getDataInfo(filename);
  double** gradeMatrix = getGradeMatrix(filename, fileInfo);

  printf("Num of Chapters: %d\n", fileInfo.x);
  printf("Num of Homework: %d\n", fileInfo.y);
  printf("Num of Rows: %d\n", fileInfo.n);  
  printf("------------------------------------\n");

  double avgs[fileInfo.x][fileInfo.y];  // Will store nested arrays of averages
  pid_t man_pid, work_pid;

  // Make "x" Manager Processes
  for (int manager = 0; manager < fileInfo.x; manager++) {
    int pipefd1[2];

    errorWrap(pipe(pipefd1), "pipe");
    int dataStart = manager * fileInfo.y, dataEnd = dataStart + fileInfo.y - 1; // 0-based indexing
    // Extract only the necessary data that can be accessed in this specific manager process
    double** managerData = getMatrixCols(gradeMatrix, dataStart, dataEnd, fileInfo);

    man_pid = fork();

    if (man_pid == 0) { // In Manager Process
      double chapterAvgs[fileInfo.y];  // Stores data we'll pass later to Director

      // Make "y" Worker Processes
      for (int worker = 0; worker < fileInfo.y; worker++) {
        int pipefd2[2];

        errorWrap(pipe(pipefd2), "pipe");
        // Extract only the necessary data that can be accessed in this specific worker process
        double** workerData = getMatrixCols(managerData, worker, worker, fileInfo);
        work_pid = fork();

        if (work_pid == 0) {  // In Worker Process
          double hwAvg = getAvg(workerData, fileInfo.n);
          close(pipefd2[0]);  // Close read end
          write(pipefd2[1], &hwAvg, sizeof(hwAvg));
          _exit(0); // Exit current worker process
        } else {  // In Parent Process
          waitpid(work_pid, NULL, 0);
          close(pipefd2[1]); // Close write end
          free(workerData);
          read(pipefd2[0], &chapterAvgs[worker], sizeof(double)); // Store Worker Data
        }
      }

      close(pipefd1[0]); // Close read end
      write(pipefd1[1], &chapterAvgs, sizeof(chapterAvgs));
      free(managerData);
      _exit(0); // Exit current manager process
    } else {  // In director process
      waitpid(man_pid, NULL, 0);
      close(pipefd1[1]);  // Close write end
      read(pipefd1[0], &avgs[manager], sizeof(double[fileInfo.y]));
    }
  }

  // Printing out the data recieved
  for (int i = 0; i < fileInfo.x; i++) {
    for (int j = 0; j < fileInfo.y; j ++) {
      printf("The average for Chpt %d, HW %d is: %f\n", i + 1, j + 1, avgs[i][j]);
    }
  }
}


// Returns [x, y, n]
Grades getDataInfo (char* filename) {
  int fd;
  Grades rtnInfo;
  rtnInfo.n = 0;

  fd = open(filename, O_RDONLY);
  errorWrap(fd, "read");

  char buf[BUFF_SIZE];
  int rowEntries = 1, i = 0, n = 0;
  bool completeRow;


  while ((n = read(fd, &buf, BUFF_SIZE)) > 0) {
    for (i = 0; i < n; i++) {
      if (buf[i] == '\n') {
        completeRow = true;
        rtnInfo.n++; 
      } else if (!completeRow && (buf[i] == ' ' || buf[i] == '\t')) { 
        rowEntries++; 
      }
    }
  }

  // If the file doesn't end with a newline
  if (buf[i - 1] != '\n') { rtnInfo.n++; }

  // Calculate x & y from "rowEntries"
  int* result = closestFactors(rowEntries);
  rtnInfo.x = result[0];
  rtnInfo.y = result[1];

  errorWrap(close(fd), "close");

  return rtnInfo;
}


// Function to return the closest factors that make up a number
// "x" will contain the bigger of the 2 factors
int* closestFactors (int total) {
  int a = 1, b = total, temp;
  int difference = b - 1;
  // Appropriate stopping point
  int stopPnt = (b / 2);
  if (stopPnt * 2 != b) { stopPnt++; }  // For odd totals

  for (int i = 2; i <= difference; i++) {
    if (total % i == 0) {
      temp = total / i;
      if (temp - i < difference) {
        difference = temp - i;
        a = i;
        b = temp;
      }
    }
  }

  static int result[2];
  result[0] = b;
  result[1] = a;

  return result;
}


// Ends program if code = -1
void errorWrap (int code, char* msg) {
  if (code == -1) {
    perror(msg);
    exit(EXIT_FAILURE);
  }
}


// Function to turn the data text file into a matrix
double** getGradeMatrix (char* filename, Grades fileInfo) {
  int totalRows = fileInfo.n, totalCols = fileInfo.x * fileInfo.y, i;
  double** matrix = malloc(totalRows * sizeof(double)); // Allocate memory for rows
  // Allocate memory for columns
  for (i = 0; i < totalRows; i++) { matrix[i] = malloc(totalCols * sizeof(double)); }

  int fd;
  fd = open(filename, O_RDONLY);
  errorWrap(fd, "read");

  char buf;
  char num[MAX_GRADE_DIGITS];
  memset(num, 0, sizeof(num));
  int currcol = 0, currrow = 0, idx = 0;

  while (read(fd, &buf, 1) > 0) {
    if (buf == ' ' || buf == '\t' || buf == '\n' || buf == '\0') {
      matrix[currrow][currcol] = atof(num);
      currcol++;
      idx = 0;
      memset(num, 0, sizeof(num));
    } else {
      num[idx] = buf;
      idx++;
    }

    if (buf == '\n' || buf == '\0') {
      // Reset the column count & add to row count
      currcol = 0;
      currrow++;
    }
  }

  // Make sure we write the last number to the last entry of the matrix
  // In case we don't catch the end of file from the read system call
  matrix[currrow][currcol] = atof(num);
  
  errorWrap(close(fd), "close");

  return matrix;
}


// Extract the columns from "to" to "from" from a matrix
double** getMatrixCols(double** matrix, int to, int from, Grades fileInfo) {
  int numRows = fileInfo.n, numCols = fileInfo.x * fileInfo.y, 
      width = from - to + 1, i, j, idx = 0; // width is +1 due to 0-based indexing

  double** rtnMatrix = malloc(numRows * sizeof(double)); // Allocate memory for rows
  // Allocate memory for columns
  for (i = 0; i < numRows; i++) { rtnMatrix[i] = malloc(width * sizeof(double)); }

  // Populating data:
  for (i = 0; i < numRows; i++) {
    for (j = 0; j < numCols; j++) {
      if (j >= to && j <= from ) {
        rtnMatrix[i][idx] = matrix[i][j];
        idx++;
      }
    }

    idx = 0;
  }

  return rtnMatrix;
}

double getAvg(double** matrix, int numRows) {
  int i;
  double avg = 0;

  for (i = 0; i < numRows; i++) { avg += matrix[i][0]; }

  return avg / numRows;
}

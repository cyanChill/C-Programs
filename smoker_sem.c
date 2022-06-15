#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "sem.h"

#define CHILD           0       /* Return value of child process from fork call*/
#define TRUE            0 
#define FALSE           1

#define NumOfCigarette  10			/* How many cigarettes will be made */
#define PrepareTime     3       /* Time to prepare ingredients */
#define MaxWorkTime     4       /* Time to make cigarette */


FILE *fp0, *fp1, *fp2, *fp3;


int main() {
  int lock = semget(IPC_PRIVATE,1,0666 | IPC_CREAT); /* Mutex Variable */
  /* When these semaphors are 1, that specific process is doing a task */
  int smoker_match = semget(IPC_PRIVATE,1,0666 | IPC_CREAT);
  int smoker_paper = semget(IPC_PRIVATE,1,0666 | IPC_CREAT);
  int smoker_tobacco = semget(IPC_PRIVATE,1,0666 | IPC_CREAT);
  int agent = semget(IPC_PRIVATE,1,0666 | IPC_CREAT);
  
  sem_create(lock, 1);
  sem_create(smoker_match, 0);
  sem_create(smoker_paper, 0);
  sem_create(smoker_tobacco, 0);
  sem_create(agent, 0);

  /* Shared file to track the remaining number of cigarettes that can be made */
  fp0 = fopen("cigarettesLeft.txt", "w+");
  fprintf(fp0, "%d\n", NumOfCigarette);    // Initialize number of paper to 0
	fclose(fp0);

  int pid;                  // Process ID after fork call
  int i;                    // index
  int N = NumOfCigarette;   // Number of times agent supplies ingredients
  int numLeft0, numLeft1, numLeft2, numLeft3; // Number of cigarettes left
  int randNum;              // Contains one of the 3 choices of what the agent brings out
  int status;			          // Exit status of child process
  int flag1, flag2, flag3;
  srand(time(NULL));        // Set randomness

  // Creating child processes
  if ((pid = fork()) == -1) {
    perror("fork"); // Fork failed!
    exit(1);
  }

  if (pid == CHILD) { // Agent process
    printf("Agent's Pid: %d\n", getpid());
    N = NumOfCigarette;

    for (i = 0; i < N; i++) {
      P(lock);
      // Spend time preparing ingredients
      sleep(rand() % PrepareTime + 1);
      /* Pick a "random" number from 1-3 that represents the choice of the 2 ingredients supplied */
      randNum = rand() % 3 + 1;

      if (randNum == 1) {
        printf("Agent places Tobacco & Paper on table...\n");
        V(smoker_match);  // Wake up smoker with match
      } else if (randNum == 2) {
        printf("Agent places Tobacco & Match on table...\n");
        V(smoker_paper);  // Wake up smoker with paper
      } else {
        printf("Agent places Match & Paper on table...\n");
        V(smoker_tobacco);  // Wake up smoker with tobacco
      }
      V(lock);
      P(agent);   // Agent sleeps

      // Reduce number of cigarettes left once the agent needs to resupply ingredients
      fp0 = fopen("cigarettesLeft.txt", "r+");
      fscanf(fp0, "%d", &numLeft0);
      fseek(fp0, 0L, 0);
      numLeft0 -= 1;
      fprintf(fp0, "%d \n", numLeft0);
      fclose(fp0);
    }
    /*
      To make sure the program ends, we unlock all the smokers and they'll end as they'll read 0 cigarettes left in the file
        - "Wake all the smokers up to leave"
    */
    V(smoker_match);
    V(smoker_paper);
    V(smoker_tobacco);

  } else {
    // Parent Process - Fork of another child process
    if ((pid = fork()) == -1) {
      perror("fork"); // Fork failed!
      exit(1);
    }

    if (pid == CHILD) { // Smoker with Match Process
      printf("Smoker with Match's Pid: %d\n", getpid());

      flag1 = FALSE;
      while (flag1 == FALSE) {
        P(smoker_match);  // Smoker with Match sleeps immediately
        P(lock);

        // Check amount of cigarettes that can be made
        fp1 = fopen("cigarettesLeft.txt", "r+");
        fscanf(fp1, "%d", &numLeft1);
        fclose(fp1);

        if (numLeft1 == 0) {
          // To leave loop as no more cigarettes can be made
          flag1 = TRUE; 
          V(lock);
        } else {
          // Pick up tobacco & paper
          printf("> Smoker with Match making cigarette...\n");
          V(agent);
          V(lock);
          // Spend time making the cigarette
          sleep(rand() % MaxWorkTime + 1); 
          printf("> Smoker with Match smoking cigarette.\n");
        }
      }
      
    } else {
      // Parent Process - Fork of another child process
      if ((pid = fork()) == -1) {
        perror("fork"); // Fork failed!
        exit(1);
      }

      if (pid == CHILD) { // Smoker with Paper Process
        printf("Smoker with Paper's Pid: %d\n", getpid());

        flag2 = FALSE;
        while (flag2 == FALSE) {
          P(smoker_paper);  // Smoker with Paper sleeps immediately
          P(lock);

          // Check amount of cigarettes that can be made
          fp2 = fopen("cigarettesLeft.txt", "r+");
          fscanf(fp2, "%d", &numLeft2);
          fclose(fp2);

          if (numLeft2 == 0) {
            // To leave loop as no more cigarettes can be made
            flag2 = TRUE;
            V(lock);
          } else {
            // Pick up match & tobacco
            printf("> Smoker with Paper making cigarette...\n");
            V(agent);
            V(lock);
            // Spend time making the cigarette
            sleep(rand() % MaxWorkTime + 1);  
            printf("> Smoker with Paper smoking cigarette.\n");
          }
        }

      } else {
        // Parent Process - Fork of another child process
        if ((pid = fork()) == -1) {
          perror("fork"); // Fork failed!
          exit(1);
        }

        if (pid == CHILD) { // Smoker with Tobacco Process
          printf("Smoker with Tobacco's Pid: %d\n", getpid());

          flag3 = FALSE;
          while (flag3 == FALSE) {
            P(smoker_tobacco);  // Smoker with Tobacco sleeps immediately
            P(lock);

            // Check amount of cigarettes that can be made
            fp3 = fopen("cigarettesLeft.txt", "r+");
            fscanf(fp3, "%d", &numLeft3);
            fclose(fp3);

            if (numLeft3 == 0) {
              // To leave loop as no more cigarettes can be made
              flag3 = TRUE;
              V(lock);
            } else {
              // Pick up match & paper
              printf("> Smoker with Tobacco making cigarette...\n");
              V(agent);
              V(lock);
              // Spend time making the cigarette
              sleep(rand() % MaxWorkTime + 1); 
              printf("> Smoker with Tobacco smoking cigarette.\n");
            }
          }

        } else {
          // Now parent process waits for the child processes to finish
          pid = wait(&status);
          printf("child(pid = %d) exited with the status %d. \n", pid, status);

          pid = wait(&status);
          printf("child(pid = %d) exited with the status %d. \n", pid, status);

          pid = wait(&status);
          printf("child(pid = %d) exited with the status %d. \n", pid, status);

          pid = wait(&status);
          printf("child(pid = %d) exited with the status %d. \n", pid, status);
        }
        exit(0);
      }
      exit(0);
    }
    exit(0);
  }
  exit(0);
}

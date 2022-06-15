#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>


#define NumOfCigarette    10			/* How many cigarettes will be made */
#define SelectionDelay    3       /* How long it takes for the agent to prepare ingredients */
#define CreationDelay     4       /* How fast a smoker makes & smokes a cigarette */
#define END_CODE          0

int action = 0;                   /* Value from 0-3 */
sem_t ingredientsReady;           /* Indicate to smokers that the ingredients are ready */
pthread_mutex_t agentLock;        /* Limits when agent can put new ingredients */

void* agentRoutine(void* args) {
  int i, randNum;
  char* ingredients;  // Hold the name of the ingredients displayed
  printf("Hello from agent Thread\n");

  for (i = 0; i < NumOfCigarette; i++) {
    pthread_mutex_lock(&agentLock);
    sleep(1 + rand() % SelectionDelay);
    /* Pick a "random" number from 1-3 that represents the choice of the 2 ingredients supplied */
    randNum = rand() % 3 + 1;
    ingredients = randNum == 1 ? "Paper & Tobacco" :
                  randNum == 2 ? "Match & Tobacco" 
                               : "Match & Paper";
    printf("Agent places %s (Option %d) on table...\n", ingredients, randNum);
    sem_post(&ingredientsReady);
    action = randNum;
  }
  /* Once the last cigarette is made, we prepare the steps to end the smoker threads */
  pthread_mutex_lock(&agentLock);
  action = END_CODE; // Indicate end of routine to other threads
  sem_post(&ingredientsReady); // To indicate that the other threads should end
  free(args);
}

void* smokerRoutine(void* args) {
  int smokerType = *(int *)args;
  char* smokerLabel = smokerType == 1 ? "Match" :
                      smokerType == 2 ? "Paper" 
                                      : "Tobacco";
  printf("Hello from Smoker with %s Thread.\n", smokerLabel);

  while (sem_wait(&ingredientsReady) == 0) {
    if (smokerType != action) { // If the action to be done isn't for this smoker
      sem_post(&ingredientsReady);
      if (action == END_CODE) break;  // Exit the loop if the action is set to 0 (exit action)
      sleep(1 + rand() % SelectionDelay);
    } else {
      printf(" > Smoker (%d) with %s is making the cigarette.\n", smokerType, smokerLabel);
      // Notify agent when we're making cigarette
      pthread_mutex_unlock(&agentLock);
      sleep(1 + rand() % CreationDelay);
      printf(" > Smoker (%d) with %s is smoking the cigarette.\n", smokerType, smokerLabel);
    }
  }
  free(args);
}

int main(int argc, char *argv[]) {
  int i;
  pthread_t thr[4]; // 4 threads we'll access
  srand(time(NULL)); // Set randomness
  /* Semaphore for showing when the ingredients are on the table [start with no ingredients] */
  sem_init(&ingredientsReady, 0, 0);
  /* Initiate the mutex that decides when the agent can display ingredients */
  pthread_mutex_init(&agentLock, NULL);

  // Create the agent thread
  if (pthread_create(&thr[0], NULL, &agentRoutine, NULL) != 0) {
    perror("Failed to create agent thread");
  }

  // Create the 3 smoker threads
  for (i = 1; i < 4; i++) {
    int* a = malloc(sizeof(int));
    *a = i;
    if (pthread_create(&thr[i], NULL, &smokerRoutine, a) != 0) {
      perror("Failed to create smoker thread");
    }
  }

  // Wait for all 4 threads to finish
  for (i = 0; i < 4; i++) {
    if (pthread_join(thr[i], NULL) != 0) {
      perror("Failed to join thread");
    }
  }
  
  // Delete semaphore & mutex for good practice since it's use is over
  sem_destroy(&ingredientsReady);
  pthread_mutex_destroy(&agentLock);

  return 0;
}

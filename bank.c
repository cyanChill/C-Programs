#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "sem.h"

#define CHILD      			 0					/* Return value of child process from fork call */
#define TRUE       			 0  
#define FALSE      			 1

#define NumOfDepositAttempt  5			/* How many times Dad will deposit money */
#define NumOfWithdrawAttempt 20			/* Total number of attempts SON_1 and SON_2 has */

#define DepositAmount        80			/* The amount of money Dad deposits at a time */
#define WithdrawAmount       20			/* The amount of money Son withdraws at a time */
#define initBalance          0

FILE *fp1, *fp2, *fp3, *fp4;				/* File Pointers */

int main() {
	int pid;							// Process ID after fork call
	int i;								// Loop index
	int N;								// Number of times dad does update
	int N_Att;						// Number of time sons allowed to do update
	int status;						// Exit status of child process
	int bal1, bal2;				// Balance read by processes
	int flag, flag1;			// End of loop variables
	srand(time(NULL));

	// Initialize the file balance to be $40
	// You should try with different initial balance, after you formulate the solution. Like initial Balance to be 0.
	fp1 = fopen("balance.txt","w+");
	bal1 = initBalance;
	fprintf(fp1, "%d\n", bal1);
	fclose(fp1);
	
	// Initialize the number of withdraw attempts 
	fp4 = fopen("attempt.txt", "w+");
	N_Att = NumOfWithdrawAttempt;
	fprintf(fp4, "%d\n", N_Att);
	fclose(fp4);

	/* Semaphores: */
	int semModify = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);			// Such that only 1 process can modify the bal at the same time
	int semATMs = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);				// Such that only 2 processes can run at the same time
	int semAttempts = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);		// Such that only 1 process can modify the attempts at the same time

	sem_create(semModify, 1);
	sem_create(semATMs, 2);
	sem_create(semAttempts, 1);
	

	// Create child processes that will do the updates
	if ((pid = fork()) == -1) {
		// fork failed!
		perror("fork");
		exit(1);
	}
	
	if (pid == CHILD){
		// First Child Process. 
		// Dad tries to do some updates.
		// **you need to identify the logical issues and synchronization issues and solve them using logic and semaphores.**
		printf("Dad's Pid: %d\n",getpid());
		N = NumOfDepositAttempt;
		for (i = 1; i <= N; i++) {   
			// Dad process need some time to go to the bank.
			int r = rand() % 5 + 1;
			sleep(r);
			// After r second Dad process reached the Bank.
			P(semATMs);
			printf("Dad is requesting to view the balance.\n"); // Dad is requesting to get hold of an ATM.
			fp1 = fopen("balance.txt", "r+"); // Dad successfully got hold of the ATM.
			fscanf(fp1, "%d", &bal2);
			printf("Dad reads balance = %d \n", bal2);
			r = rand() % 5 + 1;
			printf("Dad wants to deposit money\n");
			printf("Dad needs %d sec to prepare money.\n", r);

			sleep(r); // Dad Process is sleeping for r sec. You need to make sure that other processes can work in the mean time.
			// After some time Dad process wakes up.
			// It is possible that the balance has changed during the time dad process is sleeping
			// Dad process starts to deposit the money. So Dad process needs the access to ATM.
			// Only after getting access to the ATM dad process can deposit money.

			P(semModify);
			// Get latest balance (as it may have changed)
			fseek(fp1, 0L, 0);
			fscanf(fp1, "%d", &bal2);

			fseek(fp1, 0L, 0); // Dad will now deposit the money. And update the current balance.
			bal2 += DepositAmount;
			fprintf(fp1, "%d \n", bal2);
			fclose(fp1);

			printf("Dad writes new balance = %d \n", bal2);
			printf("Dad will deposit %d more time\n", N - i); // Dad depostited the money. 
			V(semModify);
			V(semATMs);
			sleep(rand() % 10 + 1);	/* Dad will wait some time for requesting to see balance again.*/
		}

	} else { 
		// Parent Process. Fork off another child process.
		if ((pid = fork()) == -1) {
			// Fork failed!
			perror("fork");
			exit(1);
		}

		if (pid == CHILD) {
			printf("First Son's Pid: %d\n",getpid());
			// Second child process. 
      // **you need to identify the logical issues and synchronization issues and solve them using logic and semaphores.**
      // Son1 tries to view the balance
			flag = FALSE;
			while (flag == FALSE) {
				// Son1 process needs some time to go to the bank.
        int r = rand() % 5 + 1;
        sleep(r);
        // After r second Son1 process reached the Bank.
				P(semATMs);
				printf("SON_1 is requesting to view the balance.\n"); // Son_1 is requesting to get hold of the ATM.
				P(semAttempts);
				fp3 = fopen("attempt.txt" , "r+"); // Son_1 successfully got hold of the ATM.
				fscanf(fp3, "%d", &N_Att); // Son_1 Checks if he has more than 0 attempt remaining.
				printf("Attempt remaining: %d.\n", N_Att);

				if (N_Att == 0) {
					fclose(fp3);
					flag = TRUE;
					V(semAttempts);
				} else {
					/* Not all attempts are successful */
					fseek(fp3, 0L, 0); // SON_1 will write the number of attempt remaining in the attampt.txt file.
					N_Att -= 1;
					fprintf(fp3, "%d\n", N_Att);
					fclose(fp3);
					V(semAttempts);

					fp2 = fopen("balance.txt", "r+"); // Son_1 reads the balance. 
					fscanf(fp2, "%d", &bal2);
					printf("SON_1 reads balance. Available Balance: %d \n", bal2); 
					printf("SON_1 wants to withdraw money. "); // And if balance is greater than Withdraw amount, then son can withdraw money.
					fseek(fp2, 0L, 0);

					P(semModify);
					// Get latest balance if it was modified
					fscanf(fp2, "%d", &bal2);
					fseek(fp2, 0L, 0);

					if (bal2 >= WithdrawAmount) {
						// We can withdraw as we have enough balance to not go negative
						bal2 -= WithdrawAmount;
						fprintf(fp2, "%d\n", bal2);
						printf("SON_1 withdrawed %d. New Balance: %d \n",WithdrawAmount, bal2);
					} else {
						printf("SON_1 cannot withdraw from balance.\n");
					}

					fclose(fp2);
					V(semModify);	

					printf("Number of attempts remaining: %d \n", N_Att);
				}
				V(semATMs);

				sleep(rand() % 10 + 1); //SON_1 will wait some time before the next request.
			}

		} else {
			//Parent Process. Fork off one more child process.
			if ((pid = fork()) == -1) {
				//fork failed!
				perror("fork");
				exit(1);
			}

			if (pid == CHILD) {
				printf("Second Son's Pid: %d\n",getpid()); 
				// Third child process. 
        // **you need to identify the logical issues and synchronization issues and solve them using logic and semaphores.**
        // Son2 tries to view the balance.
				flag1 = FALSE;
				while(flag1 == FALSE) {
					// Son2 process needs some time to go to the bank.
          int r = rand() % 5 + 1;
          sleep(r);
					// After r second Son2 process reached the Bank.
					P(semATMs);
          printf("SON_2 is requesting to view the balance.\n"); // Son_2 is requesting to get hold of the ATM.
					P(semAttempts);
	
					fp3 = fopen("attempt.txt", "r+"); // Son_2 successfully got hold of the ATM.
					fscanf(fp3, "%d", &N_Att); // Son_2 Checks if he has more than 0 attempt remaining.
					printf("Attempt remaining: %d.\n", N_Att);

					if(N_Att == 0) {
						fclose(fp3);
						flag1 = TRUE;
						V(semAttempts);
					} else {
						/* Not all attempts are successful */
						fseek(fp3, 0L, 0); // SON_2 will write the number of attempt remaining in the attempt.txt file.
						N_Att -= 1;
						fprintf(fp3, "%d\n", N_Att);
						fclose(fp3);
						V(semAttempts);

						fp2 = fopen("balance.txt", "r+"); // Son_2 reads the balance. 
						fscanf(fp2, "%d", &bal2);
						printf("SON_2 reads balance. Available Balance: %d \n", bal2);
						printf("SON_2 wants to withdraw money. "); // And if balance is greater than Withdraw amount, then son can withdraw money.
						fseek(fp2, 0L, 0);

						P(semModify);
						// Get latest balance if it was modified
						fscanf(fp2, "%d", &bal2);
						fseek(fp2, 0L, 0);

						if (bal2 >= WithdrawAmount) {
							// We can withdraw as we have enough balance to not go negative
							bal2 -= WithdrawAmount;
							fprintf(fp2, "%d\n", bal2);
							printf("SON_2 withdrawed %d. New Balance: %d \n",WithdrawAmount, bal2);
						} else {
							printf("SON_2 cannot withdraw from balance.\n");
						}

						fclose(fp2);
						V(semModify);

						printf("Number of attempts remaining: %d \n", N_Att);
					}	
					V(semATMs);

					sleep(rand() % 10 + 1);// SON_2 will wait some time before the next request.
				}
			} else {	
				// Now parent process waits for the child processes to finish
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





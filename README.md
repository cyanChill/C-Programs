# Information

This branch contains 3 files which I started to learn about process synchronization by using semaphores & mutexes.

| File            | Description        |
|-----------------|--------------------|
| `bank.c`  | This contains the solution of the problem we want to solve -- we have 2 ATMs with a "Dad" process depositing money and 2 "Son" processes withdrawing money at random time intervals. We don't want the Son process to withdraw when there's no money (ie: prevent creating negative balance). Each process "group" (ie: Dad group and Son group can try some sort of modification [deposit/withdraw] a certain amount of attempts. |
| `sem.h`    | This file was provided and for us to be able to make semaphores & mutexes. |

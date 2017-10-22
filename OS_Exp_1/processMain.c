#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int procSequence[10] = { 0 };
static int count = 0;

void*
MyThread1(void)
{
      int n;
      int h = 0, leap = 1, k, m, i;

      printf("\nInput the value of n:\n");
      scanf("%d", &n);
      printf("\nThe prime sequence as following:\n");

      for (m = 2; m <= n; ++m) {
            k = m / 2;
            for (i = 2; i <= k; ++i) {
                  if (m % i == 0) {
                        leap = 0;
                        break;
                  }
            }

            if (leap) {
                  printf("%d ", m);
                  h++;
            }

            leap = 1;
      }
      printf("\n");

      printf("\nThread 1 exit!\n");
      pthread_exit(0);
}

void*
MyThread2(void)
{
      int fib0 = 0, fib1 = 1, fib2, i, N;

      printf("\nInput Fib value N:\n");
      scanf("%d", &N);
      printf("\nThe Fib sequence as following:\n");

      for (i = 0; i < N; ++i) {
            if (i == 0) {
                  printf("0 ");
            } else if (i == 1) {
                  printf("1 ");
            } else {
                  fib2 = fib0 + fib1;
                  printf("%d ", fib2);
                  fib0 = fib1;
                  fib1 = fib2;
            }
      }

      printf("\nThread2 exit!\n");
      pthread_exit(0);
}

int
mainThread()
{
      int ret1 = 0, ret2 = 0;
      pthread_t id1, id2;

      ret1 = pthread_create(&id1, NULL, (void*)MyThread1, NULL);
      if (ret1) {
            printf("Create pthread ERROR!\n");
            return 1;
      }

      ret2 = pthread_create(&id2, NULL, (void*)MyThread2, NULL);
      if (ret2) {
            printf("Create pthread ERROR!\n");
            return 1;
      }

      procSequence[count++] = id1;
      procSequence[count++] = id2;

      pthread_join(id1, NULL);
      pthread_join(id2, NULL);

      return 0;
}

void
childFunction(int i)
{
      switch (i) {
            case 4:
                  printf("This is No.4 Process, ID is %d, "
                         "Parent ID is %d\n",
                         getpid(),
                         getppid());
                  procSequence[count++] = getpid();
                  // TODO: mainThread()
                  mainThread();

            case 5:
                  printf("This is No.5 Process, ID is %d, "
                         "Parent ID is %d\n",
                         getpid(),
                         getppid());
                  procSequence[count++] = getpid();
                  execl("proc2", "./proc2", NULL);
      }
}

int
createProcess(void)
{
      int i;
      for (i = 4; i < 6; i++) {
            pid_t child;
            child = fork();
            if (child < 0) {
                  printf("ERROR happended in fork funtion!\n");
                  return 1;
            } else if (child == 0) {
                  // TODO: childFunction(i)
                  childFunction(i);
            }
      }

      for (i = 0; i < 2; i++) {
            pid_t cpid = wait(NULL);
            printf("The process %d exit!\n", cpid);
      }

      return 0;
}

void
Function(int i)
{
      switch (i) {
            case 2:
                  printf("This is No.2 Process, ID is %d, Parent ID is %d, "
                         "Now will execute command \"ls\":\n",
                         getpid(),
                         getppid());
                  // printf("Now will execute command: ls:\n");
                  execl("/bin/ls", "ls", NULL);
                  procSequence[count++] = getpid();
                  break;

            case 3:
                  printf("This is No.3 Process, ID is %d, Parent ID is %d, "
                         "Now will create 2 processes:\n",
                         getpid(),
                         getppid());
                  // printf("Now will create 2 processes:\n");
                  // TODO: createProcess()
                  // createProcess(4);
                  // createProcess(5);
                  procSequence[count++] = getpid();
                  createProcess();
                  break;
      }

      exit(0);
}

// void procTree(void) {
//       int i, j, k = 1;
//       printf("Process 1, %d", procSequence[0][0]);
// 	for(i = 1; i < 3; i++) {
// 		for(j = 0; j < 2; j++){
// 			printf("Process %d, %d", k++, procSequence[i][j]);
// 		}
// 		printf("\n");
// 	}

// 	k = 1;
// 	for(i = 0; i < 2; i++){
// 		printf("Thread %d, %d", k++, procSequence[3][i]);
// 	}
// }

void
procTree(void)
{
      // int i;
      // for (i = 1; i < count - 1; i++) {
      //       printf("Process %d: %d\n", i, procSequence[i - 1]);
      // }

      // for (i = 6; i < 8; i++) {
      //       printf("Thread %d, %d\n", i - 5, procSequence[i - 1]);
      // }

      printf("\n"
             "Process1─┬─Process2\n"
             "         │\n"
             "         └─Process3─┬─Process4─┬─Thread1\n"
             "                    │          │\n"
             "                    │          └─Thread2\n"
             "                    │\n"
             "                    └─Process5\n");
}

int
main()
{
      procSequence[count++] = getpid();
      int i;
      for (i = 2; i < 4; i++) {
            pid_t child;
            child = fork();
            if (child < 0) {
                  printf("ERROR happended in fork fanction!\n");
                  return 0;
            } else if (child == 0) {
                  Function(i);
            }
      }

      for (i = 0; i < 2; i++) {
            pid_t cpid = wait(NULL);
            printf("The process %d exit!\n", cpid);
      }

      printf("The No.1 parent process %d is exit!\n\n\n", getpid());
      procTree();
      return 0;
}
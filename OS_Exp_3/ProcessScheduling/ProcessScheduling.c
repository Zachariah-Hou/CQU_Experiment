#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#define MAXLINE 255
#define THREAD_NUMBER 20
#define TIME_SLICE 100000
#define MAX_TIME 100
#define TRUE 1
#define FALSE 0
#define BUGNUM 2147483647

typedef int bool;
static int timer = 0;
static int tt = 0;

// 定义 PCB 结构体
struct PCB
{
    pthread_t id;          // 线程ID
    int priority;          // 优先级（反正我也不想写了……）
    int arrivalTime;       // 到达时间（都按同时到达算吧= = 不然太麻烦了……）
    int intervalTime;      // 区间时间
    bool* operationalFlag; // 可以运行置为 TRUE，阻塞置为 FALSE
} PCBQueue[THREAD_NUMBER];

// FCFS & SJF 子线程控制
void*
Subthread(void* threadCount)
{
    int count = *(int*)threadCount;
    int subTimer = 0;
    while (TRUE) {
        if (subTimer >= PCBQueue[count].intervalTime) pthread_exit(0);

        // 等待 Flag 被置 TRUE
        while (!(PCBQueue[count].operationalFlag[timer])) {
            usleep(TIME_SLICE);
            timer++;
        }

        // 说明线程可以运行，将记时器自增并输出
        printf(count < 9 ? "Thread  " : "Thread "); // 强迫症犯了，控制字母和数字之间的空格
        printf("%d : %d\n", count + 1, ++timer);
        subTimer++;
        // printf("Timer: %d\n", timer);
        usleep(TIME_SLICE);
    }
    pthread_exit(0);
}

// RR 子线程控制
void*
RRSubthread(void* threadCount)
{

    int count = *(int*)threadCount;
    int subTimer = 0;
    int t = timer - 1;
    while (TRUE) {
        // int it = PCBQueue[count].intervalTime;
        if (subTimer >= PCBQueue[count].intervalTime) pthread_exit(0);
        // if (timer == -1) pthread_exit(0);
        // if (timer == tt) pthread_exit(0);

        // 等待 Timer 前进
        while (t == timer);
        // 等待 Flag 被置 TRUE
        while (!(PCBQueue[count].operationalFlag[timer]));
        t = timer;
        // 说明线程可以运行，将记时器自增并输出
        printf(count < 9 ? "Thread  " : "Thread "); // 强迫症犯了，控制字母和数字之间的空格
        printf("%d : %d\n", count + 1, timer + 1);
        subTimer++;
    }
    pthread_exit(0);
}

// FCFS 调度
void
FCFSQueInit()
{
    int currTime = 0;
    for (int i = 0; i < THREAD_NUMBER; i++) {
        for (int j = currTime; j < currTime + PCBQueue[i].intervalTime; j++) {
            PCBQueue[i].operationalFlag[j] = TRUE;
        }
        currTime += PCBQueue[i].intervalTime;
        // printf("%d\n", currTime);
    }
}

// 执行 FCFS 调度
int
DoFCFS()
{
    FCFSQueInit();
    int totalTime = 0;
    for (int i = 0; i < THREAD_NUMBER; i++) {
        totalTime += PCBQueue[i].intervalTime;
    }
    int ret[THREAD_NUMBER] = { 0 };
    int threadCount = 0;
    for (int i = 0; i < THREAD_NUMBER; i++) {
        ret[i] = pthread_create(
          &PCBQueue[threadCount].id, NULL, Subthread, &threadCount);
        if (ret[i]) {
            printf("Create thread %d error!\n", i);
            return 1;
        }
        pthread_join(PCBQueue[threadCount].id, NULL);
        threadCount++;
    }
    // for (int i = 0; i < totalTime; i++) {
    //     timer++;
    //     usleep(TIME_SLICE);
    // }
    return 0;
}

// SJF 调度排序
int*
SJFSort(int* sorted)
{
    int threadQueue[THREAD_NUMBER];
    for (int i = 0; i < THREAD_NUMBER; i++) {
        threadQueue[i] = PCBQueue[i].intervalTime;
    }

    for (int i = 0; i < THREAD_NUMBER; i++) {
        int min = 0;
        for (int j = 0; j < THREAD_NUMBER; j++) {
            if (threadQueue[min] > threadQueue[j]) {
                min = j;
            }
        }
        // printf("%d ", min);
        threadQueue[min] = BUGNUM;
        sorted[i] = min;
    }
    return sorted;
}

// SJF 调度
void
SJFQueInit(int* threadQueue)
{
    int currTime = 0;
    for (int i = 0; i < THREAD_NUMBER; i++) {
        for (int j = currTime; j < currTime + PCBQueue[threadQueue[i]].intervalTime; j++) {
            PCBQueue[threadQueue[i]].operationalFlag[j] = TRUE;
        }
        currTime += PCBQueue[threadQueue[i]].intervalTime;
        // printf("%d\n", currTime);
    }
}

// 执行 SJF 调度
int
DoSJF()
{
    int ret[THREAD_NUMBER] = {0};
    int threadQueue[THREAD_NUMBER];
    SJFSort(threadQueue);
    SJFQueInit(threadQueue);
    for (int i = 0; i < THREAD_NUMBER; i++) {
        ret[threadQueue[i]] = pthread_create(
          &PCBQueue[threadQueue[i]].id, NULL, Subthread, &threadQueue[i]);
        if (ret[threadQueue[i]]) {
            printf("Create thread %d error!\n", threadQueue[i]);
            return 1;
        }
        pthread_join(PCBQueue[threadQueue[i]].id, NULL);
    }
    return 0;
}

// RR 调度
void
RRQueInit()
{
    int threadIntervalTime[THREAD_NUMBER];
    int totalTime = 0;
    int currTime = 0;
    for (int i = 0; i < THREAD_NUMBER; i++) {
        threadIntervalTime[i] = PCBQueue[i].intervalTime;
        totalTime += PCBQueue[i].intervalTime;
    }
    printf("%d\n", totalTime);
    // for (int i = 0; i < totalTime; i++) {
    //     for (int j = 0; j < THREAD_NUMBER; j++) {
    //         if (threadIntervalTime[j] == 0) continue;
    //         PCBQueue[j].operationalFlag[++currTime] = TRUE;
    //         threadIntervalTime[j]--;
    //     }
    // }

    for (int i = 0, j = 0; i < totalTime; i++, j++) {
        if (j >= THREAD_NUMBER) {
            j = 0;
        }

        if (threadIntervalTime[j] <= 0) {
            i--;
            continue;
        } else {
            PCBQueue[j].operationalFlag[i] = TRUE;
            threadIntervalTime[j]--;
        }
    }
}

// 执行 RR 调度
int
DoRR()
{
    RRQueInit();
    int ret[THREAD_NUMBER] = { 0 };
    int totalTime = 0;
    for (int i = 0; i < THREAD_NUMBER; i++) {
        totalTime += PCBQueue[i].intervalTime;
    }

    tt = totalTime;

    for (int i = 0; i < THREAD_NUMBER; i++) {
        ret[i] = pthread_create(
          &PCBQueue[i].id, NULL, RRSubthread, &i);
        if (ret[i]) {
            printf("Create thread %d error!\n", i);
            return 1;
        }
    }

    for (int i = 0; i < THREAD_NUMBER; i++) {
        timer++;
        usleep(TIME_SLICE);
    }

    return 0;
}

// 给出帮助文档
void
printHelp()
{
    printf("\n请在可执行文件后加选项参数(纯大写或纯小写均可)：\n\n"
           "    --FCFS\t执行先到先服务调度,\n"
           "    --SJF \t执行最短优先调度,\n"
           "    --RR  \t执行时间片轮转调度,\n"
           "    --PS  \t执行优先级调度(开发中),\n"
           "    --MLQS\t执行多级队列调度（开发中）.\n"
           "\n例如： ./a.out --fcfs\n\n"
           );
    return;
}

// 调度算法类型
enum SchedType
{
    SCHED_TYPE_HELP = 0x01 << 0,
    SCHED_TYPE_FCFS = 0x01 << 1,
    SCHED_TYPE_SJF  = 0x01 << 2,
    SCHED_TYPE_RR   = 0x01 << 3,
    SCHED_TYPE_PS   = 0x01 << 4,
    SCHED_TYPE_MLQS = 0x01 << 5,
};

// 程序入口、主进程、主线程
int
main(int argc, char* argv[])
{
    char* strCmdArg;

    if (argc <= 1) {
        strCmdArg = "help";
    } else {
        strCmdArg = argv[1];
    }

    enum SchedType currSched;

    // 判断 argv 的内容是否合法，并将合法项赋给 currSched
    if (!strcmp(strCmdArg, "--fcfs") || !strcmp(strCmdArg, "--FCFS")) {
        currSched = SCHED_TYPE_FCFS;
    } else if (!strcmp(strCmdArg, "--sjf") || !strcmp(strCmdArg, "--SJF")) {
        currSched = SCHED_TYPE_SJF;
    } else if (!strcmp(strCmdArg, "--rr") || !strcmp(strCmdArg, "--RR")) {
        currSched = SCHED_TYPE_RR;
    } else if (!strcmp(strCmdArg, "--ps") || !strcmp(strCmdArg, "--PS")) {
        currSched = SCHED_TYPE_PS;
    } else if (!strcmp(strCmdArg, "--mlqs") ||
               !strcmp(strCmdArg, "--MLQS")) {
        currSched = SCHED_TYPE_MLQS;
    } else {
        currSched = SCHED_TYPE_HELP;
    }

    // 初始化所有 PCB 中的 operationalFlag
    // 这里将每个线程的区间时间设置为 1~5 的随机数
    srand((unsigned)time(NULL));
    for (int i = 0; i < THREAD_NUMBER; i++) {
        PCBQueue[i].intervalTime = rand() % 5 + 1;
        PCBQueue[i].operationalFlag = (int*)malloc(MAX_TIME * sizeof(int));
        for (int j = 0; j < MAX_TIME; j++) {
            PCBQueue[i].operationalFlag[j] = 0;
        }
    }

    // 依据 currSched 的值，运行相应的调度算法
    switch (currSched) {
        case SCHED_TYPE_HELP:
            printHelp();
            break;
        case SCHED_TYPE_FCFS:
            printf("FCFS\n");
            DoFCFS();
            break;
        case SCHED_TYPE_SJF:
            printf("SJF\n");
            DoSJF();
            break;
        case SCHED_TYPE_RR:
            printf("RR\n");
            DoRR();
            break;
        case SCHED_TYPE_PS:
            // 没时间了不写了……
            printf("PS\n");
            break;
        case SCHED_TYPE_MLQS:
            // 这个我也不想写了……
            printf("MLQS\n");
            break;
        default:
            printHelp();
    }

    return 0;
}
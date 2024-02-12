#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/wait.h>       // for wait() funct
#include <sys/time.h>

#define PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define SHNAME "shmem"
#define P_MUTX "parent_mutex"
#define C_MUTX "child_mutex"
#define FCFS "queue"
#define ITER 1000

typedef enum {False, True} bool;

typedef struct mem{
    char segm[1024][1024];
    bool isin;
    bool asgn;
    FILE* f;
    int req;
    int total;
    int id;
    int waiting;
}Mem;
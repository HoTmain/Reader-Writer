#include "erg1.h"


int main(int argc, char* argv[]) {

    if(argc != 4) {
        printf("Please give: input text file, number of rows per section and number of readers\n");
       	exit(1);
    }

    FILE* fp = fopen(argv[1], "r");
    if(fp == NULL) {
        printf("Could not open file %s", argv[1]);
        exit(EXIT_FAILURE);
    }
    int linesc=1;
    for (int c = getc(fp); c != EOF; c = getc(fp))      //get number of total lines of file first; it will become useful later
        if (c == '\n')
            linesc++;
    if(atoi(argv[2]) > linesc) {
        perror("lines per segment is more than the total lines of the file itself!");
        exit(EXIT_FAILURE);
    }

    //######    IN CASE OF FORCE STOP OR EARLY TERMINATION ETC.     #######
    sem_unlink(FCFS);
    sem_unlink(P_MUTX);
    sem_unlink(C_MUTX);
    sem_unlink("asign");

    shm_unlink(SHNAME);



    //###########   CREATE SEMAPHORES, SHARED MEM ETC.  ###########
    int fd= shm_open(SHNAME, O_CREAT | O_EXCL | O_RDWR, PERMS);

     if (fd == -1) {
        perror("shm_open error");
        exit(EXIT_FAILURE);
    }

    if (ftruncate(fd, sizeof(Mem))== -1) {
        perror("ftruncate_error");
        exit(EXIT_FAILURE);
    }
    sem_t* parnt= sem_open(P_MUTX,  O_CREAT | O_EXCL, PERMS, 0);
    sem_t* chld= sem_open(C_MUTX,  O_CREAT | O_EXCL, PERMS, 1);
    sem_t* asign= sem_open("asign",  O_CREAT | O_EXCL, PERMS, 0);
    sem_t* fcfs= sem_open(FCFS,  O_CREAT | O_EXCL, PERMS, 0);       //here will wait all readers with different segment

    
    int lps= atoi(argv[2]);     //lines per segment
    int rdrs= atoi(argv[3]);
    Mem* mem= mmap(NULL, sizeof(Mem), PROT_WRITE, MAP_SHARED, fd, 0);
    mem->id= -1;
    mem->isin= False;
    mem->asgn= False;
    mem->waiting= 0;
    mem->total= rdrs;     //total readers
    mem->req= 0;


    pid_t pid;
    int i;
    for (i= 0; i< rdrs; i++) {        //create all readers first
        pid= fork();
        if(pid == 0)
            break;
    }
    int max_seg= linesc/lps;
    if((float)max_seg < (float)linesc/lps)
        max_seg++;

    int line_cap= 0;
    for(int a= lps*(linesc/lps); a< linesc; a++)
        line_cap++;



    //#######   START   #######//
    struct timeval cur_time;
    char fname[]= "fil0.txt";
    if(pid == 0) {
        fname[3]= i+ '0';
        open(fname, O_CREAT | O_WRONLY | O_APPEND, 0644);
        srand(getpid());
        mem->f= fopen(fname, "a");
        int pos= rand()%max_seg +1;                  ////  ############    request submission starts here    ############
        int line= rand()%lps;
        if(line_cap >0 && line >= line_cap)
            line= line_cap-1;
        gettimeofday(&cur_time, NULL);
        fprintf(mem->f, " <%d,%d> Submit <%ld>, ", pos, line, cur_time.tv_usec*1000);     //time in milliseconds
        int j= 0;

        while(j< ITER) {
            
            sem_wait(chld);
            if (mem->isin == False) {       //the first child to reach gets to read its segment, along with children with same segment
                mem->id= pos;
                mem->isin= True;
                sem_post(parnt);            //after the segment has been decided does the parent continue (signal the parent) (1)
            }
            sem_post(chld);

            if (pos != mem->id) {       //children with different segment will wait in line
                sem_wait(chld);
                mem->waiting++;
                sem_post(chld);

                sem_wait(fcfs);
                
                sem_wait(chld);
                mem->waiting--;
                sem_post(chld);
                continue;
            }


            sem_wait(chld);
            mem->req++;
            sem_post(chld);

            sem_wait(chld); //one child may enter, otherwise multiple children would have to wait for the signal 2 from writer
            if(mem->asgn == False) {
                sem_wait(asign);    //wait for writer to assign segment, then read it from shm (signal from writer) 2
                mem->asgn= True;
            }
            sem_post(chld);
            int k= j;
            while(pos == mem->id && k < ITER ) {
                gettimeofday(&cur_time, NULL);
                fprintf(mem->f, "Answer <%ld>: %s", cur_time.tv_usec*1000, mem->segm[line]);      //############    request answer from writer is here   ############
                // usleep(20000);
                if((double)(rand()/RAND_MAX) <= 0.3)
                    pos= rand()%(linesc/lps) +1;
                line= rand()%lps;
                if(line_cap >0 && line >= line_cap)
                    line= line_cap-1;
                gettimeofday(&cur_time, NULL);
                fprintf(mem->f, " <%d,%d> Submit @ <%ld>, ", pos, line, cur_time.tv_sec);
                k++;
            }
            j= k-1;
            
            sem_wait(chld);
            mem->req--;
            sem_post(chld);
            if(mem->req == 0) {
                sem_post(parnt);        //only after all children have finished with the segment does the writer choose a new one
            }
            j++;
        }
        sem_wait(chld);
        mem->total--;
        sem_post(chld);
        exit(0);
    }

    //#####  WRITER  #####//
    int inqueue;
    FILE* f;
    do{
        sem_wait(parnt);        //wait signal from reader, then assign segment  in shm

        f= fopen(argv[1], "r");
        int to= mem->id * lps;       //mem->id * lines per segment
        int from= to -lps +1;       //from the first line of the segment (segment_n - lps + 1)
        do {
            fgets(mem->segm[0], sizeof(mem->segm[0]), f);
            from--;
        }while(from);

        int line= 1;
        do {
            fgets(mem->segm[line], sizeof(mem->segm[0]), f);
            line++;
        }while(line< lps);

        sem_post(asign);        //after assigning segment does writer signal reader (2)

        sem_wait(parnt);    //wait untill readers have finished with the segment
        fclose(f);
        
        mem->asgn= False;
        mem->isin= False;
        inqueue= mem->waiting;
        for(int c=0; c<inqueue; c++)
            sem_post(fcfs);

    }while(mem->total > 0);



    if(sem_unlink(FCFS) < 0)
        perror("FCFS_unlink failed");
    if(sem_unlink(P_MUTX) < 0)
        perror("P_MUTX_unlink failed");
    if(sem_unlink(C_MUTX) < 0)
        perror("C_MUTX_unlink failed");
    if(sem_unlink("asign") < 0)
        perror("asign_unlink failed");
    if (shm_unlink(SHNAME) < 0)
        perror("shm_unlink failed");
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    key_t key = ftok("shmfile", 65);
    if (key == -1) {
        perror("ftok failed");
        exit(1);
    }

    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    int *int_mem = (int *)shmat(shmid, NULL, 0);
    char *str_mem = (char *)(int_mem + 1); 
    if (int_mem == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    pid_t pidB, pidC;

    *int_mem = 10;
    sprintf(str_mem, "I AM PROCESS A whose id is : %d", getpid());
    printf("%d %s\n", *int_mem, str_mem);

    pidB = fork();  
    if (pidB == 0) {
        while (*int_mem != 10) {
            usleep(100000);  
        }
        *int_mem = 20;  
        sprintf(str_mem, "I AM PROCESS B whose id is : %d", getpid()); 
        printf("%d %s\n", *int_mem, str_mem);

        pidC = fork();
        if (pidC == 0) {
            while (*int_mem != 20) {
                usleep(100000);  
            }
            *int_mem = 30;  
            sprintf(str_mem, "I AM PROCESS C whose id is : %d", getpid());  
            printf("%d %s\n", *int_mem, str_mem);
            exit(0);
        }
        
        waitpid(pidC, NULL, 0);  
        exit(0);
    }

    waitpid(pidB, NULL, 0);

    printf("Good Bye World, I am Done!!\n");

    if (shmdt(int_mem) == -1) {
        perror("shmdt failed");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(1);
    }

    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

int main() {
    key_t key = ftok("shmfile", 65);
    if (key == -1) {
        perror("ftok failed in Process C");
        exit(1);
    }

    int shmid = shmget(key, 1024, 0666);
    if (shmid == -1) {
        perror("shmget failed in Process C");
        exit(1);
    }

    int *int_mem = (int *)shmat(shmid, NULL, 0);
    char *str_mem = (char *)(int_mem + 1);  
    if (int_mem == (void *)-1) {
        perror("shmat failed in Process C");
        exit(1);
    }

    while (*int_mem != 20) {
        sleep(1);  
        printf("Current value in shared memory: %d\n", *int_mem);
    }

    *int_mem = 30;  
    sprintf(str_mem, "I am Process C with ID: %d", getpid());  
    printf("%d %s\n", *int_mem, str_mem);

    if (shmdt(int_mem) == -1) {
        perror("shmdt failed in Process C");
        exit(1);
    }

    return 0;
}

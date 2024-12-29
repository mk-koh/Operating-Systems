#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>

int main() {
    key_t key = ftok("shmfile", 65);
    if (key == -1) {
        perror("ftok failed in Process A");
        exit(1);
    }

    int shmid = shmget(key, 1024, 0666 | IPC_CREAT);
    if (shmid == -1) {
        perror("shmget failed in Process A");
        exit(1);
    }

    int *int_mem = (int *)shmat(shmid, NULL, 0);
    char *str_mem = (char *)(int_mem + 1); 
    if (int_mem == (void *)-1) {
        perror("shmat failed in Process A");
        exit(1);
    }

    *int_mem = 10;
    sprintf(str_mem, "I am Process A with ID: %d", getpid());
    printf("%d %s\n", *int_mem, str_mem);

    while (*int_mem != 20) {
        usleep(500000); 
    }

    printf("%d %s\n", *int_mem, str_mem);

    while (*int_mem != 30) {
        usleep(500000); 
    }

    printf("%d %s\n", *int_mem, str_mem);

    if (shmdt(int_mem) == -1) {
        perror("shmdt failed in Process A");
        exit(1);
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl failed in Process A");
        exit(1);
    }

    printf("Good Bye World, I am Done!!\n");

    return 0;
}

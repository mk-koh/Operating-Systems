#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <errno.h>

#define SHM_KEY 1234   
#define SEM_KEY 5678  

struct shared_data {
    int value;             
    int readers_count;    
    int writer_waiting;   
};

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

#define SEM_WAIT(semid, semnum) \
    do { \
        struct sembuf sb = {semnum, -1, 0}; \
        if (semop(semid, &sb, 1) == -1) { \
            perror("semop (WAIT) failed"); \
            exit(1); \
        } \
    } while (0)

#define SEM_SIGNAL(semid, semnum) \
    do { \
        struct sembuf sb = {semnum, 1, 0}; \
        if (semop(semid, &sb, 1) == -1) { \
            perror("semop (SIGNAL) failed"); \
            exit(1); \
        } \
    } while (0)

//Writer
void writer(int shm_id, int sem_id, struct shared_data *shm_data) {
    pid_t pid = getpid();

    SEM_WAIT(sem_id, 0);  

    shm_data->writer_waiting = 1;

    while (shm_data->readers_count > 0) {
        SEM_WAIT(sem_id, 1); 
    }

    shm_data->value *= 2;
    printf("Writer Process Id %d Writing %d\n", pid, shm_data->value);

    shm_data->writer_waiting = 0;  
    SEM_SIGNAL(sem_id, 0); 
    
    SEM_SIGNAL(sem_id, 1);  

    exit(0);
}

//Reader
void reader(int shm_id, int sem_id, struct shared_data *shm_data) {
    pid_t pid = getpid();

    SEM_WAIT(sem_id, 0);

    if (shm_data->writer_waiting > 0) {
        SEM_SIGNAL(sem_id, 0); 
        return;  
    }

    shm_data->readers_count++;
    SEM_SIGNAL(sem_id, 0); 

    printf("Reader's Process Id %d Reading %d\n", pid, shm_data->value);

    SEM_WAIT(sem_id, 0);
    shm_data->readers_count--;
    SEM_SIGNAL(sem_id, 0); 

    exit(0);  
}

int main() {
    srand(time(NULL));

    int num_processes;
    printf("Enter the number of readers and writers (N): ");
    scanf("%d", &num_processes);

    int total_processes = (1 << num_processes); 

    int shm_id = shmget(SHM_KEY, sizeof(struct shared_data), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("shmget failed");
        exit(1);
    }

    struct shared_data *shm_data = (struct shared_data *)shmat(shm_id, NULL, 0);
    if (shm_data == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    shm_data->value = 1;
    shm_data->readers_count = 0;
    shm_data->writer_waiting = 0;

    int sem_id = semget(SEM_KEY, 3, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("semget failed");
        exit(1);
    }

    union semun sem_union;
    sem_union.val = 1; 
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1) {
        perror("semctl failed");
        exit(1);
    }

    sem_union.val = 0;  
    if (semctl(sem_id, 1, SETVAL, sem_union) == -1) {
        perror("semctl failed");
        exit(1);
    }

    sem_union.val = 0; 
    if (semctl(sem_id, 2, SETVAL, sem_union) == -1) {
        perror("semctl failed");
        exit(1);
    }

    pid_t pid;
    for (int i = 0; i < total_processes; i++) {
        pid = fork();
        if (pid == 0) {
            if (i < total_processes / 2) {
                writer(shm_id, sem_id, shm_data);  
            } else {
                reader(shm_id, sem_id, shm_data);  
            }
        }
    }

    //Wait for all child
    for (int i = 0; i < total_processes; i++) {
        wait(NULL);
    }

    //Clean
    if (shmdt(shm_data) == -1) {
        perror("shmdt failed");
        exit(1);
    }

    if (shmctl(shm_id, IPC_RMID, NULL) == -1) {
        perror("shmctl failed");
        exit(1);
    }

    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1) {
        perror("semctl failed");
        exit(1);
    }

    return 0;
}

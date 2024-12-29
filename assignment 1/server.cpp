#include <iostream>
#include <mqueue.h>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define SERVER_QUEUE_NAME "/mq_server"
#define CLIENT_QUEUE_NAME "/mq_client"
#define QUEUE_PERMISSIONS 0660
#define MAX_MSG_SIZE 64
#define MAX_MESSAGES 10

void reverse_string(char* str) {
    int len = strlen(str);
    for (int i = 0; i < len / 2; i++) {
        std::swap(str[i], str[len - i - 1]);
    }
}

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq_server = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);
    if (mq_server == -1) {
        perror("Server: mq_open (server)");
        exit(1);
    }

    mqd_t mq_client = mq_open(CLIENT_QUEUE_NAME, O_WRONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);
    if (mq_client == -1) {
        perror("Server: mq_open (client)");
        exit(1);
    }

    char buffer[MAX_MSG_SIZE + 1];
    ssize_t bytes_read;

    std::cout << "I AM SERVER! I WILL RECEIVE MESSAGE FROM CLIENT AND WILL SEND REVERSE OF THIS" << std::endl;

    while (true) {
        bytes_read = mq_receive(mq_server, buffer, MAX_MSG_SIZE, nullptr);
        if (bytes_read == -1) {
            perror("Server: mq_receive");
            exit(1);
        }
        buffer[bytes_read] = '\0'; 
        std::cout << "MESSAGE RECEIVED FROM CLIENT: \n" << buffer << std::endl;

        reverse_string(buffer);
        std::cout << "\nSENT REVERSE OF MESSAGE TO CLIENT" << std::endl;

        if (mq_send(mq_client, buffer, strlen(buffer), 0) == -1) {
            perror("Server: mq_send");
            exit(1);
        }
    }

    mq_close(mq_server);
    mq_close(mq_client);
    mq_unlink(SERVER_QUEUE_NAME);
    mq_unlink(CLIENT_QUEUE_NAME);

    return 0;
}

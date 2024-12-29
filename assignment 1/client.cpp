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

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    mqd_t mq_client = mq_open(CLIENT_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);
    if (mq_client == -1) {
        perror("Client: mq_open (client)");
        exit(1);
    }

    mqd_t mq_server = mq_open(SERVER_QUEUE_NAME, O_WRONLY | O_CREAT, QUEUE_PERMISSIONS, &attr);
    if (mq_server == -1) {
        perror("Client: mq_open (server)");
        exit(1);
    }

    char buffer[MAX_MSG_SIZE];
    std::string input;

    std::cout << "I AM CLIENT. ENTER MESSAGE WHICH I WILL PASS TO SERVER (Press <ENTER>):" << std::endl;

    while (true) {
        std::getline(std::cin, input);
        if (input.length() >= MAX_MSG_SIZE) {
            std::cerr << "Input too long. Max size is " << MAX_MSG_SIZE - 1 << " characters." << std::endl;
            continue;
        }

        if (mq_send(mq_server, input.c_str(), input.length(), 0) == -1) {
            perror("Client: mq_send");
            exit(1);
        }

        ssize_t bytes_read = mq_receive(mq_client, buffer, MAX_MSG_SIZE, nullptr);
        if (bytes_read == -1) {
            perror("Client: mq_receive");
            exit(1);
        }
        buffer[bytes_read] = '\0';

        std::cout << "THE REVERSED STRING RECEIVED FROM SERVER: \n" << buffer << std::endl;
        std::cout << "ENTER MESSAGE WHICH I WILL PASS TO SERVER (Press <ENTER>):" << std::endl;
    }

    mq_close(mq_client);
    mq_close(mq_server);
    mq_unlink(CLIENT_QUEUE_NAME);
    mq_unlink(SERVER_QUEUE_NAME);

    return 0;
}

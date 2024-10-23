#include "../../include/server.h"

int main(int argc, char* argv[]) {
    // 0 get args
    char* serverip = NULL;
    unsigned short serverport;
    if (argc < 2 || argc > 3) {
        printf("Usage: ./server [ip] <port>\n");
        exit(EXIT_FAILURE);
    } else if (argc == 2) {
        serverip = NULL;
        serverport = (unsigned short)atoi(argv[1]);
    } else {
        serverip = argv[1];
        serverport = (unsigned short)atoi(argv[2]);
    }
    // 1 pre
    Server* server = NULL;
    try {
        server = new Server(serverip, serverport, CLIENTS_H, PACKET_HEAD_SIZE_H, MAX_DATA_LEN_H);
    } catch (...) {
        fprintf(stderr, "Err: new server failed.\n");
        exit(EXIT_FAILURE);
    }
    // 2 main-while
    while (1) {
        // (1) epoll_wait
        int event_count = epoll_wait(server->getEpfd(), server->getEvs(), server->getClientnum() + 1, -1);
        if (event_count <= 0) {
            fprintf(stderr, "Err: epoll wait failed.\n");
            continue;
        }
        // (2) process-while
        for (int i = 0; i < event_count; i++) {
            if (server->getEvs()[i].data.fd == server->getFd()) {
                try {
                    server->addClient();
                } catch (...) {
                    fprintf(stderr, "Err: add client falied..\n");
                    continue;
                }
            } else {
                    if (server->clientProcess(server->getEvs()[i].data.fd) == -1) {
                        fprintf(stderr, "Err: client%d process failed.\n", server->getEvs()[i].data.fd);
                        continue;
                    }
            }
        }
    }
    delete server;
    exit(EXIT_SUCCESS);
}


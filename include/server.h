#ifndef CLIENT_MANAGE_H
#define CLIENT_MANAGE_H

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <unordered_map>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdexcept>
#include <mqueue.h>

#include "packet.h"

#define CLIENTS_H 1024

class Server {
    private:
        int servfd;
        int epfd;
        struct epoll_event ev;
        struct epoll_event evs[CLIENTS_H + 1] = {0};
        const int CLIENTNUM;
        const int PACKET_HEAD_SIZE;
        const int MAX_DATA_LEN;
        int packetsize;
        Packet* packet;
        std::unordered_map<std::string, int> clients;
        int userssize;
        char* usersbuf;

    public:
        Server(char* serverip, unsigned short serverport, int clientnum, int packet_head_size, int max_data_len);
        int getFd() { return this->servfd; }
        int getEpfd() { return this->epfd; }
        struct epoll_event* getEvs() { return this->evs; }
        int getClientnum() { return this->CLIENTNUM; }
        int getUserssize() { return this->userssize; }
        char* getUsersbuf() { return this->usersbuf; }
        size_t setUsersbuf();
        void addClient();
        int clientProcess(int clifd);
        void delClient(int clifd);
        ~Server();
};

#endif


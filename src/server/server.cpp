#include <iostream>

#include "../../include/server.h"
#include "../../include/m-net.h"

Server::~Server() {
    if (this->servfd != 0 && this->epfd != 0) {
        epoll_ctl(this->epfd, EPOLL_CTL_DEL, this->servfd, NULL);
        close(this->epfd);
        close(this->servfd);
    }
    if (this->packet != NULL) {
        free((void*)this->packet);
    }
    if (this->usersbuf != NULL) {
        free((void*)this->usersbuf);
    }
}

Server::Server(char* serverip, unsigned short serverport, int clientnum, int packet_head_size, int max_data_len) : CLIENTNUM(clientnum), PACKET_HEAD_SIZE(packet_head_size), MAX_DATA_LEN(max_data_len) {
    // 1 create listen socket
    this->servfd = createListenSocket(serverip, serverport);
    if (this->servfd == -1) {
        throw std::runtime_error("Err: create listen socket failed.\n");
    }
    if (makeSocketNonBlocking(this->servfd) == -1) {
        throw std::runtime_error("Err: make socket non-blocking failed.\n");
    }
    // 2 create epoll
    this->epfd = epoll_create1(0);
    if (this->epfd == -1) {
        throw std::runtime_error("Err: create epoll handler failed.\n");
    }
    // 3 create public buf
    // (1) packet
    this->packetsize = (this->PACKET_HEAD_SIZE + this->MAX_DATA_LEN + 1) * sizeof(char);
    this->packet = (Packet*)calloc(this->packetsize, 1);
    if (this->packet == NULL) {
        throw std::runtime_error("Err: calloc public packet failed.\n");
    }
    // (2) users
    this->userssize = (CLIENTS_H * (MAX_NAME_LEN_H + 1)) * sizeof(char);
    this->usersbuf = (char*)calloc(this->userssize, 1);
    if (this->usersbuf == NULL) {
        throw std::runtime_error("Err: calloc public users buf failed.\n");
    }
    // 4 add listen socket to epoll
    this->ev.events = EPOLLIN;
    this->ev.data.fd = this->servfd;
    if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, this->servfd, &(this->ev)) == -1) {
        throw std::runtime_error("Err: add listen_fd to epoll failed.\n");
    }
    return;
}

void Server::delClient(int clifd) {
    if (clifd != 0 && this->epfd != 0) {
        epoll_ctl(this->epfd, EPOLL_CTL_DEL, clifd, NULL);
        close(clifd);
    }
    std::string keytmp(this->packet->srcname);
    this->clients.erase(keytmp);
}

void Server::addClient() {
    // 1 accept
    int clifd = accept(this->servfd, NULL, NULL);
    if (clifd == -1) {
        throw std::runtime_error("Err: accept failed.\n");
    }
    if (makeSocketNonBlocking(clifd) == -1) {
        close(clifd);
        throw std::runtime_error("Err: make socket non-blocking failed.\n");
    }
    // 2 add client socket to epoll
    this->ev.events = EPOLLIN;
    this->ev.data.fd = clifd;
    if (epoll_ctl(this->epfd, EPOLL_CTL_ADD, clifd, &(this->ev)) == -1) {
        throw std::runtime_error("Err: add client_fd to epoll failed.\n");
        close(clifd);
    }
}

size_t Server::setUsersbuf() {
    size_t single_len = 0;
    size_t all_len = 0;
    memset((void*)(this->getUsersbuf()), 0x00, this->getUserssize());
    for (const auto& pair : this->clients) {
        strncpy(this->getUsersbuf() + single_len, pair.first.c_str(), pair.first.length());
        single_len = pair.first.length() + 1;
        all_len += single_len;
    }
    return all_len;
}

void printMap(const std::unordered_map<char*, int>& myMap) {
    for (const auto& pair : myMap) {
        std::cout << "Key: " << pair.first << ", Value: " << pair.second << std::endl;
    }
}

int Server::clientProcess(int clifd) {
    ssize_t recv_ret;
    // 1 head
    // (1) read head from client_fd
    bzero((void*)(this->packet), this->packetsize);
    recv_ret = readn(clifd, (char*)(this->packet), this->PACKET_HEAD_SIZE);
    if (recv_ret < 0) {
        fprintf(stderr, "Err: recv from client%d falied.\n", clifd);
        return -1;
    } else if (recv_ret == 0) {
        printf("client%d closed.\n", clifd);
        return -1;
    }
    // (2) head process
    if (this->packet->trans_type == REGISTER) {
        std::string keytmp(this->packet->srcname);
        auto it = this->clients.find(keytmp);
        if (it != this->clients.end()) {
            this->packet->datalen = -1;
            if (writen(clifd, (char*)packet, PACKET_HEAD_SIZE) < 0) {
                fprintf(stderr, "Err: send name occupied info failed.\n");
                return -1;
            }
            this->delClient(clifd);
            printf("debug1\n");
        } else {
            this->clients.insert({keytmp, clifd});
            this->packet->datalen = this->setUsersbuf();
            for (const auto& pair : this->clients) {
                if (writen(pair.second, (char*)(this->packet), PACKET_HEAD_SIZE) == -1) {
                    fprintf(stderr, "Err: users head - to %s failed.\n", pair.first.c_str());
                    continue;
                }
                if (writen(pair.second, this->getUsersbuf(), this->packet->datalen) == -1) {
                    fprintf(stderr, "Err: users body - to %s failed.\n", pair.first.c_str());
                    continue;
                }
            }
            printf("debug2\n");
        }
        return 0;
    }
    // 2 body
    // (1) read body from client_fd
    recv_ret = readn(clifd, this->packet->data, this->packet->datalen);
    if (recv_ret < 0) {
        fprintf(stderr, "Err: recv from client%d falied.\n", clifd);
        return -1;
    } else if (recv_ret == 0) {
        printf("client%d closed.\n", clifd);
        return -1;
    }
    // (2) body process
    if (this->packet->trans_type == ALL) {
        printf("debug all :\n");
        printf("head :\n");
        printf("type : %d\n", this->packet->trans_type);
        printf("datalen : %d\n", this->packet->datalen);
        printf("srcname : %s\n", this->packet->srcname);
        printf("dstname : %s\n", this->packet->dstname);
        printf("data : %s\n", this->packet->data);
        for (auto it = this->clients.begin(); it != this->clients.end(); ++it) {
            if (writen(it->second, (char*)(this->packet), this->PACKET_HEAD_SIZE + this->packet->datalen) < 0) {
                fprintf(stderr, "Err: all - from %s forward to %s failed.\n", this->packet->srcname, it->first.c_str());
                return -1;
            }
        }
    } else if (this->packet->trans_type == SINGLE) {
        printf("debug all :\n");
        printf("head :\n");
        printf("type : %d\n", this->packet->trans_type);
        printf("datalen : %d\n", this->packet->datalen);
        printf("srcname : %s\n", this->packet->srcname);
        printf("dstname : %s\n", this->packet->dstname);
        printf("data : %s\n", this->packet->data);
        std::string keytmp(this->packet->dstname);
        auto it = this->clients.find(keytmp);
        if (it != this->clients.end()) {
            if (writen(it->second, (char*)(this->packet), this->PACKET_HEAD_SIZE + this->packet->datalen) == -1) {
                fprintf(stderr, "Err: single - from %s forward to %s failed.\n", this->packet->srcname, this->packet->dstname);
                return -1;
            }
        } else {
            fprintf(stderr, "Err: search %s failed.\n", this->packet->dstname);
            return -1;
        }
    } else {
        fprintf(stderr, "Err: unknown type, may be attack.\n");
    }
    return 0;
}


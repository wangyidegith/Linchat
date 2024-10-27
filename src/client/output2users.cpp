#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>

#include "../../include/client-shared.h"
#include "../../include/packet.h"
#include "../../include/m-net.h"

#define CLIENTS_H 1024

int main() {
    // 1 pre
    // (1) create usocket
    int ulissockfd = createListenUsocket(USERS_USOCKET_PATH);
    if (ulissockfd < 0) {
        std::cerr << "create users unix socket failed." << std::endl;
        return -1;
    }
    // (2) accept
    int usockfd = accept(ulissockfd, NULL, NULL);
    if (usockfd < 0) {
        std::cerr << "accept connect from read worker falied." << std::endl;
        close(ulissockfd);
        return -1;
    }
    // (3) pre res
    // a packet
    size_t packetsize = PACKET_HEAD_SIZE_H * sizeof(char);
    Packet* packet = (Packet*)malloc(packetsize);
    if (packet == NULL) {
        close(ulissockfd);
        close(usockfd);
        return -1;
    }
    // b users
    int userssize = (CLIENTS_H * (MAX_NAME_LEN_H + 1) + 1) * sizeof(char);
    char* usersbuf = (char*)calloc(userssize, 1);
    if (usersbuf == NULL) {
        close(ulissockfd);
        close(usockfd);
        free((void*)packet);
        return -1;
    }
    int userslen = 0;
    // c other
    ssize_t recvlen;
    // 2 main-while
    while (1) {
        // (1) read head
        bzero((void*)packet, packetsize);
        recvlen = readn_b(usockfd, (char*)packet, PACKET_HEAD_SIZE_H);
        if (recvlen < 0) {
            std::cerr << "Err: recv error." << std::endl;
            continue;
        } else if (recvlen == 0) {
            std::cout << "Err: unix socket closed." << std::endl;
            break;
        }
        // (2) read body
        bzero((void*)usersbuf, userslen);
        recvlen = readn(usockfd, usersbuf, packet->datalen);
        userslen = packet->datalen;
        if (recvlen < 0) {
            std::cerr << "Err: recv error." << std::endl;;
            continue;
        } else if (recvlen == 0) {
            std::cout << "Err: unix socket closed." << std::endl;
            break;
        }
        char* cur = usersbuf;
        std::cout << "online users :" << std::endl;
        while (strlen(cur) == 0) {
            std::cout << cur << std::endl;
            cur += strlen(cur) + 1;
        }
    }
    // 3 free
    free(packet);
    free(usersbuf);
    close(usockfd);
    close(ulissockfd);
    return 0;
}


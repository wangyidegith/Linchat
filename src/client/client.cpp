#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "../../include/client-shared.h"
#include "../../include/packet.h"
#include "../../include/m-net.h"

#define CLIENTS_H 1024

void readWorker(int sockfd) {
    // 1 pre
    // (1) users
    int userssize = (CLIENTS_H * (MAX_NAME_LEN_H + 1)) * sizeof(char);
    char* usersbuf = (char*)calloc(userssize, 1);
    if (usersbuf == NULL) {
        std::cerr << "Err: create usersbuf failed." << std::endl;
        return;
    }
    int userslen = 0;
    // (2) packet
    size_t packetsize = (PACKET_HEAD_SIZE_H + MAX_DATA_LEN_H + 1) * sizeof(char);
    Packet* packet = (Packet*)malloc(packetsize);
    if (packet == NULL) {
        std::cerr << "Err: create packet failed." << std::endl;
        return;
    }
    // (3) mq_recvall
    mqd_t mq_recvall = mq_open(RECV_ALL_MQUEUE, O_WRONLY);
    if (mq_recvall == (mqd_t)-1) {
        std::cerr << "Err: get recv all mq failed." << std::endl;
        return;
    }
    // (4) mq_recvsingle
    mqd_t mq_recvsingle = mq_open(RECV_SINGLE_MQUEUE, O_WRONLY);
    if (mq_recvsingle == (mqd_t)-1) {
        std::cerr << "Err: get recv all mq failed." << std::endl;
        return;
    }
    ssize_t recvlen;
    // 2 main-while
    while (1) {
        bzero((void*)packet, packetsize);
        recvlen = readn_b(sockfd, (char*)packet, PACKET_HEAD_SIZE_H);
        if (recvlen < 0) {
            std::cerr << "Err: recv error." << std::endl;
            continue;
        } else if (recvlen == 0) {
            std::cout << "server closed." << std::endl;
            break;
        }
        if (packet->trans_type == REGISTER) {
            bzero((void*)usersbuf, userslen);
            recvlen = readn_b(sockfd, usersbuf, packet->datalen);
            userslen = packet->datalen;
            if (recvlen < 0) {
                std::cerr << "Err: recv error." << std::endl;;
                continue;
            } else if (recvlen == 0) {
                std::cout << "server closed." << std::endl;
                break;
            }
            int usockfd = createConnectUsocket(USERS_USOCKET_PATH);
            if (writen(usockfd, (char*)packet, PACKET_HEAD_SIZE_H) < 0) {
                std::cerr << "Err: send error" << std::endl;
                continue;
            }
            if (writen(usockfd, usersbuf, packet->datalen) < 0) {
                std::cerr << "Err: send error" << std::endl;
                continue;
            }
        } else {
            bzero((void*)packet, packetsize);
            recvlen = readn_b(sockfd, packet->data, packet->datalen);
            if (recvlen < 0) {
                std::cerr << "Err: recv error." << std::endl;;
                continue;
            } else if (recvlen == 0) {
                std::cout << "server closed." << std::endl;
                break;
            }
            if (packet->trans_type == ALL) {
                if (mq_send_n(mq_recvall, (char*)packet, PACKET_HEAD_SIZE_H + packet->datalen)) {
                    std::cerr << "send packet to recv all mq falied." << std::endl;
                    continue;
                }
            } else if (packet->trans_type == SINGLE) {
                if (mq_send_n(mq_recvsingle, (char*)packet, PACKET_HEAD_SIZE_H + packet->datalen)) {
                    std::cerr << "send packet to recv single mq falied." << std::endl;
                    continue;
                }
            } else {
                std::cerr << "unknown type, may be attack." << std::endl;
                continue;
            }
        }
    }
    mq_close(mq_recvsingle);
    mq_close(mq_recvall);
    free(packet);
    free(usersbuf);
}

void writeWorker(int sockfd) {
    // 1 pre
    // (1) get mq
    mqd_t mq = mq_open(SEND_MQUEUE, O_RDONLY);
    if (mq == (mqd_t)-1) {
        std::cerr << "Err: get send mq failed." << std::endl;
        return;
    }
    // (2) pre packet
    size_t packetsize = (PACKET_HEAD_SIZE_H + MAX_DATA_LEN_H + 1) * sizeof(char);
    Packet* packet = (Packet*)malloc(packetsize);
    if (packet == NULL) {
        std::cerr << "Err: create packet failed." << std::endl;
        mq_close(mq);
        return;
    }
    // 2 main-while
    while (1) {
        bzero((void*)packet, packetsize);
        if (mq_receive_n(mq, (char*)packet, PACKET_HEAD_SIZE_H) <= 0) {
            std::cerr << "Err: read from send mq failed." << std::endl;
            continue;
        } else {
            if (mq_receive_n(mq, (char*)packet + PACKET_HEAD_SIZE_H, packet->datalen) <= 0) {
                std::cerr << "Err: read from send mq failed." << std::endl;
                continue;
            }
        }
        if (writen(sockfd, (char*)packet, PACKET_HEAD_SIZE_H + packet->datalen)) {
            std::cerr << "Err: send packet to socket falied." << std::endl;
        }
    }
    // 3 free
    free((void*)packet);
    mq_close(mq);
}


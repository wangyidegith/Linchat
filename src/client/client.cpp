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
    sleep(1);
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
    // (5) create usocket
    int usockfd = createConnectUsocket(USERS_USOCKET_PATH);
    if (usockfd == -1) {
        fprintf(stderr, "Err: create connect unix socket failed.\n");
        return;
    }
    // 2 main-while
    while (1) {
        bzero((void*)packet, packetsize);
        recvlen = readn_b(sockfd, (char*)packet, PACKET_HEAD_SIZE_H);
        if (recvlen < 0) {
            std::cerr << "Err: recv error." << std::endl;
            continue;
        } else if (recvlen == 0) {
            std::cout << "Err: server closed in read worker head." << std::endl;
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
                std::cout << "Err: server closed in read worker register." << std::endl;
                break;
            }
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
                std::cout << "Err: server closed in read worker body." << std::endl;
                break;
            }
            if (packet->trans_type == ALL) {
                if (mq_send(mq_recvall, (char*)packet, PACKET_HEAD_SIZE_H + packet->datalen, 0)) {
                    perror("mq_send");
                    continue;
                }
            } else if (packet->trans_type == SINGLE) {
                if (mq_send(mq_recvsingle, (char*)packet, PACKET_HEAD_SIZE_H + packet->datalen, 0)) {
                    perror("mq_send");
                    continue;
                }
            } else {
                std::cerr << "Err: unknown type, may be attack." << std::endl;
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
    sleep(1);
    // 1 pre
    // (1) get mq
    mqd_t mq = mq_open(SEND_MQUEUE, O_RDONLY);
    if (mq == (mqd_t)-1) {
        std::cerr << "Err: get send mq failed." << std::endl;
        return;
    }
    // (2) pre packet
    size_t packetsize = (PACKET_HEAD_SIZE_H + MAX_DATA_LEN_H + 1) * sizeof(char);   // 296
    Packet* packet = (Packet*)malloc(packetsize);
    if (packet == NULL) {
        std::cerr << "Err: create packet failed." << std::endl;
        mq_close(mq);
        return;
    }
    /*
    struct mq_attr attr;
    if (mq_getattr(mq, &attr) == -1) {
        perror("mq_getattr");
        return;
    } else if (attr.mq_msgsize == 404) {
        printf("Max message size: %ld\n", attr.mq_msgsize);
    } else {
        printf("???\n");
    }
    */
    // 2 main-while
    while (1) {
        bzero((void*)packet, packetsize);
        if (mq_receive(mq, (char*)packet, packetsize, 0) <= 0) {
            perror("mq_receive");
            fprintf(stderr, "Err: mq_r in client.cpp error.\n");
            break;
            // continue;
        }
        printf("debug :\n");
        printf("head :\n");
        printf("type : %d\n", packet->trans_type);
        printf("datalen : %d\n", packet->datalen);
        printf("srcname : %s\n", packet->srcname);
        printf("dstname : %s\n", packet->dstname);
        printf("data : %s\n", packet->data);
        if (writen(sockfd, (char*)packet, PACKET_HEAD_SIZE_H + packet->datalen) < 0) {
            std::cerr << "Err: send packet to socket falied." << std::endl;
        }
    }
    // 3 free
    free((void*)packet);
    mq_close(mq);
}


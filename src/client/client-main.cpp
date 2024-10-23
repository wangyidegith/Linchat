#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>

#include "../../include/client-shared.h"
#include "../../include/m-net.h"
#include "../../include/packet.h"
#include "../../include/client.h"

int main(int argc, char* argv[]) {
    // 1 get args
    if (argc != 4) {
        std::cerr << "Usage: ./client <server-ip> <server-port> <username>" << std::endl;
        return -1;
    }
    if (strlen(argv[3]) > MAX_NAME_LEN_H) {
        std::cerr << "Err: username's length must < 16" << std::endl;
        return -1;
    }
    const char* username = argv[3];
    // 2 connect server
    int sockfd = createConnectSocket(argv[1], (unsigned short)atoi(argv[2]));
    if (sockfd == -1) {
        std::cerr << "Err: connect server failed." << std::endl;
        return -1;
    }
    // 3 register
    // (1) pre packet, data is NULL
    size_t packetsize = PACKET_HEAD_SIZE_H * sizeof(char);
    Packet* packet = (Packet*)malloc(packetsize);
    if (packet == NULL) {
        std::cerr << "Err: create packet failed." << std::endl;
        close(sockfd);
        return -1;
    }
    encode(packet, REGISTER, 0, username, NULL, NULL);
    // (2) send packet to server
    if (writen(sockfd, (char*)packet, PACKET_HEAD_SIZE_H) < 0) {
        std::cerr << "Err: send register packet error." << std::endl;
        close(sockfd);
        return -1;
    }
    // (3) read echo from server
    size_t recvlen = recvn_b(sockfd, (char*)packet, PACKET_HEAD_SIZE_H, MSG_PEEK);
    if (recvlen < 0) {
        std::cerr << "Err: read register echo error." << std::endl;
        close(sockfd);
        return -1;
    } else if (recvlen == 0) {
        std::cout << "server closed." << std::endl;
        close(sockfd);
        return 0;
    } else {
        if (packet->datalen == -1) {
            std::cout << "Sorry! your username is occupied. Please alter." << std::endl;
            close(sockfd);
            return 0;
        } else {
            std::cout << "debug" << std::endl;
            free((void*)packet);
        }
    }
    // 4 pre shared res
    // (1) create SEND_MQUEUE
    mqd_t mq_send = mq_open(SEND_MQUEUE, O_CREAT | O_RDWR, 0644, NULL);
    if (mq_send == (mqd_t)-1) {
        std::cerr << "Err: create send mq failed." << std::endl;
        return -1;
    }
    // (2) create DSTNAME_SHM
    int dst_shm_fd = shm_open(DSTNAME_SHM, O_CREAT | O_RDWR, 0644);
    if (dst_shm_fd == -1) {
        std::cerr << "Err: create dstname shm failed." << std::endl;
        return -1;
    }
    if (ftruncate(dst_shm_fd, MAX_NAME_LEN_H + 1) == -1) {
        std::cerr << "Err: set dstname shm size failed." << std::endl;
        return -1;
    }
    char* dstname = (char*)mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, dst_shm_fd, 0);
    if (dstname == MAP_FAILED) {
        std::cerr << "Err: get dstname shm failed." << std::endl;
        return -1;
    }
    memset((void*)dstname, 0x00, MAX_NAME_LEN_H + 1);
    // (3) create RECV_ALL_MQUEUE
    mqd_t mq_recvall = mq_open(RECV_ALL_MQUEUE, O_CREAT | O_RDWR, 0644, NULL);
    if (mq_recvall == (mqd_t)-1) {
        std::cerr << "Err: create recv all mq failed." << std::endl;
        return -1;
    }
    // (4) create RECV_SINGLE_MQUEUE
    mqd_t mq_recvsingle = mq_open(RECV_SINGLE_MQUEUE, O_CREAT | O_RDWR, 0644, NULL);
    if (mq_recvsingle == (mqd_t)-1) {
        std::cerr << "Err: create recv single mq failed." << std::endl;
        return -1;
    }
    // TODO:5 tmux
    // 6 start read and write thread
    std::thread readWorkerT(readWorker, sockfd);
    std::thread writeWorkerT(writeWorker, sockfd);
    // 7 free
    readWorkerT.join();
    writeWorkerT.join();
    mq_unlink(RECV_SINGLE_MQUEUE);
    mq_unlink(RECV_ALL_MQUEUE);
    close(dst_shm_fd);
    shm_unlink(DSTNAME_SHM);
    mq_unlink(SEND_MQUEUE);
    close(sockfd);
    return 0;
}


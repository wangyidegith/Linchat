#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <mqueue.h>
#include <sys/stat.h>

#include "../../include/client-shared.h"
#include "../../include/packet.h"
#include "../../include/m-net.h"

int main(int argc, char* argv[]) {
    // 0 get args
    if (argc != 2) {
        return -1;
    }
    char* username = argv[1];
    // 1 pre
    // (1) get mq
    mqd_t mq = mq_open(SEND_MQUEUE, O_WRONLY);
    if (mq == (mqd_t)-1) {
        std::cerr << "Err: get send mq failed." << std::endl;
        return -1;
    }
    // (2) get shm
    int shm_fd = shm_open(DSTNAME_SHM, O_RDONLY, 0644);
    if (shm_fd == -1) {
        std::cerr << "Err: open dstname shm failed." << std::endl;
        mq_close(mq);
        return -1;
    }
    char* dstname = (char*)mmap(0, 4096, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (dstname == MAP_FAILED) {
        std::cerr << "Err: get dstname shm failed." << std::endl;
        return -1;
    }
    // (3) buf
    char databuf[MAX_DATA_LEN_H + 1] = {0};
    size_t packetsize = (PACKET_HEAD_SIZE_H + MAX_DATA_LEN_H + 1) * sizeof(char);
    Packet* packet = (Packet*)malloc(packetsize);
    if (packet == NULL) {
        return -1;
    }
    int datalen;
    // 2 main-while
    while (1) {
        std::cout << "<";
        bzero((void*)databuf, MAX_DATA_LEN_H + 1);
        std::cin.getline(databuf, MAX_DATA_LEN_H + 1);
        if (strlen(databuf) == 0) {
            continue;
        }
        if (strlen(dstname) == 0) {
            std::cout << "Please choose a friend." << std::endl;
            continue;
        }
        bzero((void*)packet, packetsize);
        datalen = strlen(databuf);
        encode(packet, SINGLE, datalen, username, dstname, databuf);
        if (mq_send_n(mq, (char*)packet, PACKET_HEAD_SIZE_H + datalen)) {
            std::cerr << "send packet to send mq falied." << std::endl;
        }
    }
    // 3 free
    free((void*)packet);
    mq_close(mq);
    return 0;
}


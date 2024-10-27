#include <iostream>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../../include/client-shared.h"
#include "../../include/packet.h"
#include "../../include/m-net.h"

int main(int argc, char* argv[]) {
    // 0 get args
    if (argc != 2) {
        std::cerr << "Usage: ./elfname <username>" << std::endl;
        return -1;
    }
    const char* username = argv[1];
    // 1 pre
    // (1) get mq
    mqd_t mq = mq_open(SEND_MQUEUE, O_WRONLY);
    if (mq == (mqd_t)-1) {
        std::cerr << "Err: get send mq failed." << std::endl;
        return -1;
    }
    // (2) pre databuf, packet, and other var
    char databuf[MAX_DATA_LEN_H + 1] = {0};
    size_t packetsize = (PACKET_HEAD_SIZE_H + MAX_DATA_LEN_H + 1) * sizeof(char);
    Packet* packet = (Packet*)malloc(packetsize);
    if (packet == NULL) {
        std::cerr << "Err: create packet failed." << std::endl;
        mq_close(mq);
        return -1;
    }
    int datalen;
    // 2 main-while
    while (1) {
        // (1) get msg
        std::cout << ">";
        bzero((void*)databuf, MAX_DATA_LEN_H + 1);
        std::cin.getline(databuf, MAX_DATA_LEN_H + 1);
        datalen = strlen(databuf);
        if (datalen == 0) {
            continue;
        }
        // (2) encode
        bzero((void*)packet, packetsize);
        encode(packet, ALL, datalen, username, NULL, databuf);
        // (3) send packet to mq
        if (mq_send(mq, (char*)packet, PACKET_HEAD_SIZE_H + datalen, 0)) {
            perror("mq_send");
        }
    }
    // 3 free
    free((void*)packet);
    mq_close(mq);
    return 0;
}


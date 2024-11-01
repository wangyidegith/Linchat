#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <sys/stat.h>

#include "../../include/client-shared.h"
#include "../../include/packet.h"
#include "../../include/m-net.h"

int main() {
    // get mq
    mqd_t mq = mq_open(RECV_SINGLE_MQUEUE, O_RDONLY);
    if (mq == (mqd_t)-1) {
        std::cerr << "Err: get recv single mq failed." << std::endl;;
        return -1;
    }
    // pre res
    size_t packetsize = (PACKET_HEAD_SIZE_H + MAX_DATA_LEN_H + 1) * sizeof(char);
    Packet* packet = (Packet*)malloc(packetsize);
    if (packet == NULL) {
        return -1;
    }
    while (1) {
        // get msg from mq
        bzero((void*)packet, packetsize);
        // read
        if (mq_receive(mq, (char*)packet, packetsize, 0) <= 0) {
            perror("mq_receive");
            fprintf(stderr, "mq_r in output2single.cpp error.\n");
            break;
            // continue;
        }
        // send data to sreen
        std::cout << packet->srcname << ": " << packet->data << std::endl;
    }
    free((void*)packet);
    mq_close(mq);
    return 0;
}


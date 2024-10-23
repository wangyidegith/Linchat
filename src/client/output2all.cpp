#include <iostream>
#include <unistd.h>
#include <string.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "../../include/client-shared.h"
#include "../../include/packet.h"
#include "../../include/m-net.h"

int main() {
    // 1 pre
    // (1) get mq
    mqd_t mq = mq_open(RECV_ALL_MQUEUE, O_RDONLY);
    if (mq == (mqd_t)-1) {
        std::cerr << "Err: get recv single mq failed." << std::endl;;
        return -1;
    }
    // (2) pre packet
    size_t packetsize = (PACKET_HEAD_SIZE_H + MAX_DATA_LEN_H + 1) * sizeof(char);
    Packet* packet = (Packet*)malloc(packetsize);
    if (packet == NULL) {
        return -1;
    }
    // 2 main-while
    while (1) {
        bzero((void*)packet, packetsize);
        if (mq_receive_n(mq, (char*)packet, PACKET_HEAD_SIZE_H) <= 0) {
            break;
        } else {
            if (mq_receive_n(mq, packet->data, packet->datalen) <= 0) {
                break;
            }
        }
        std::cout << packet->srcname << ": " << packet->data << std::endl;
    }
    // 3 free
    free((void*)packet);
    mq_close(mq);
    return 0;
}


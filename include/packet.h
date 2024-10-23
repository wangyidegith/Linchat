#ifndef PACKET_H
#define PACKET_H

#define MAX_DATA_LEN_H 255

#define NAME_OCCUPIED "Sorry, your name is occupied, please alter." 

typedef enum TRANS_TYPE {
    REGISTER,
    ALL,
    SINGLE,
} TRANS_TYPE;

#define MAX_NAME_LEN_H 15
typedef struct Packet {
    TRANS_TYPE trans_type;
    int datalen;
    char srcname[MAX_NAME_LEN_H + 1];
    char dstname[MAX_NAME_LEN_H + 1];

    char data[];
} Packet;
#define PACKET_HEAD_SIZE_H sizeof(Packet)

void encode(Packet* p, TRANS_TYPE trans_type, const int datalen, const char* srcname, const char* dstname, char* data);

#endif

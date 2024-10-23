#include <string.h>

#include "../../include/packet.h"

void encode(Packet* p, TRANS_TYPE trans_type, const int datalen, const char* srcname, const char* dstname, char* data) {
    p->trans_type = trans_type;
    p->datalen = datalen;
    if (srcname != NULL) {
        strncpy(p->srcname, srcname, strlen(srcname));
    }
    if (dstname != NULL) {
        strncpy(p->dstname, dstname, strlen(dstname));
    }
    if (data != NULL) {
        strncpy(p->data, data, strlen(data));
    }
}

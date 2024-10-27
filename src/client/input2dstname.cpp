#include <iostream>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "../../include/client-shared.h"
#include "../../include/packet.h"

int main() {
    int shm_fd = shm_open(DSTNAME_SHM, O_RDWR, 0644);
    if (shm_fd == -1) {
        std::cerr << "Err: open dstname shm failed." << std::endl;
        return -1;
    }
    char* dstname = (char*)mmap(0, MAX_NAME_LEN_H + 1, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (dstname == MAP_FAILED) {
        std::cerr << "Err: get dstname shm failed." << std::endl;
        return -1;
    }
    while (1) {
        std::cout << ">";
        std::cin.getline(dstname, MAX_NAME_LEN_H + 1);
        dstname[strlen(dstname)] = '\0';
    }
    return 0;
}

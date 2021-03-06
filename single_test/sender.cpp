/*
 *
 *      Using Flush + Reload to perform covert-channel communication
 *      between sender + receiver
 *
 *      sender side code
 *
 */
#include "util.hpp"
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>


int PAGE_SIZE = 4096;

int STRIDE = PAGE_SIZE * 2;

int offset[] = {12, 135, 235, 345, 465, 568, 648, 771};

inline unsigned long probe(const uint8_t* addr){
    volatile unsigned long time;
    asm __volatile__(
        " mfence            \n"
        " lfence            \n"
        " rdtsc             \n"
        " lfence            \n"
        " movl %%eax, %%esi \n"
        " movl (%1), %%eax  \n"
        " lfence            \n"
        " rdtsc             \n"
        " subl %%esi, %%eax \n"
        " clflush 0(%1)     \n"
        : "=a" (time)
        : "c" (addr)
        : "%esi", "%edx"
    );
    return time;
}

inline void flush(const uint8_t* addr){
    asm __volatile__("mfence\nclflush 0(%0)" : : "r"(addr) :);
}

int main(int argc, char** argv){

    // Allocate the memory and get base address
    int map_length = PAGE_SIZE * 17;
    int fd = open("/bin/ls", O_RDONLY);
    assert(fd > 0);
    const uint8_t* base_addr = (const uint8_t*)mmap(NULL, map_length, PROT_READ, MAP_SHARED, fd, 0);
    printf("original base addr = %lx\n", base_addr);
    base_addr = (uint8_t*) ((( (uintptr_t)base_addr >> 12) + 1) << 12);
    printf("used base addr = %lx\n", base_addr);

    // flush all the memory
    for (int i=0; i<PAGE_SIZE*17; i++){
        flush(base_addr + i);
    }

    // test functionality
    char text_buf[] = "send me\n";
    char char_sent = '0';
    char foo;

    // send text
    while(1){
        char_sent = text_buf[0];
        // send char_sent
        for(int j=0; j<8; j++){
            if(char_sent & 0x1) {   // send 1
                const uint8_t* target_addr = base_addr + j * STRIDE + offset[j];
                foo = *target_addr; 
            }
            char_sent = char_sent >> 1;
        }
    }


    return 0;
}


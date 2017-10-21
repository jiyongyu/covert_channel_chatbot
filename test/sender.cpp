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
#include <time.h>
#include <string.h>

int SEND_TIMES = 500000;

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

void flushMem(const uint8_t* base_addr, int size_in_bytes);

int main(int argc, char** argv){

    // Allocate the memory and get base address
    int map_length = PAGE_SIZE * 17;
    int stride = PAGE_SIZE*2;
    int fd = open("/bin/ls", O_RDONLY);
    assert(fd > 0);
    const uint8_t* base_addr = (const uint8_t*)mmap(NULL, map_length, PROT_READ, MAP_SHARED, fd, 0);
    printf("original base addr = %lx\n", base_addr);
    base_addr = (uint8_t*) ((( (uintptr_t)base_addr >> 12) + 1) << 12);
    printf("used base addr = %lx\n", base_addr);

    // flush all the memory
    flushMem(base_addr, PAGE_SIZE * 16);

    // test functionality
    char char_sent = '0';
    char foo;
    
    printf("Please type a message.\n");
    bool sending = true;
    while(sending){
        char text_buf[128];
        for(int i=0; i<128; i++){
            text_buf[i] = 0;
        }
        fgets(text_buf, sizeof(text_buf), stdin);
        clock_t begin = clock();

        printf("text_buf = %s\n", text_buf);
 
        // send text
        for(int i=0; i<128; i++){
            for(int r=0; r<SEND_TIMES; r++){
                char_sent = 1;
                // send char_sent
                for(int j=0; j<8; j++){
                    if(char_sent & 0x1) {   // send 1
                        const uint8_t* target_addr = base_addr + j * STRIDE + offset[j];
                        foo = *target_addr;
                    }
                    char_sent = char_sent >> 1;
                }
            }

            for(int r=0; r<SEND_TIMES; r++){
                char_sent = text_buf[i];
                // send char_sent
                for(int j=0; j<8; j++){
                    if(char_sent & 0x1) {   // send 1
                        const uint8_t* target_addr = base_addr + j * STRIDE + offset[j];
                        foo = *target_addr;
                    }
                    char_sent = char_sent >> 1;
                }
            }
            if (text_buf[i] == '\n')
                break;
        }

        clock_t end = clock();
        double bytes_per_sec = strlen(text_buf) / ((double)(end - begin) / CLOCKS_PER_SEC);
        printf("%d chars, bytes_per_sec = %f\n", strlen(text_buf), bytes_per_sec);
    }

    printf("Sender finished.\n");

    return 0;
}


void flushMem(const uint8_t* base_addr, int size_in_bytes){
    for(int i=0; i<size_in_bytes; i++)
        flush(base_addr + i);
}

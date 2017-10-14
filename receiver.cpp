/*
 *
 *      Using Flush + Reload to perform covert-channel communication 
 *      between sender + receiver
 *
 *      receiver side code
 *
 */
#include "util.hpp"
#include "inttypes.h"

#define STRIDE  64

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

int main(int argv, char** argv){

    // Allocate memory and get the base address
    
    uint8_t* base_addr = (uint8_t*) malloc(sizeof(uint8_t) * 4096 * 2);
    printf("original base addr = %lx\n", base_addr);
    base_addr = (uint8_t*)((( (uintptr_t)base_addr >> 12) + 1) << 12);
    printf("used base addr = %lx\n", base_addr);

    // Initialize memory pages
    for (int i=0; i<64; i++){
        for (int j=0; j<64; j++){
            // the nth page has all bytes == n
            base_addr[i * 64 + j] = i;
        }
    }

    // flush all the memory
    for (int i=0; i<4096; i++){
        flush(base_addr + i);
    }

    // test functionality
    unsigned long res_time = 0;
    for (int i=0; i<10; i++){
        for (int j=0; j<8; j++){
            res_time = probe(base_addr + j * STRIDE);
            flush(base_addr + j * STRIDE);

            printf("j = %d, respond time = %d", j, res_time);
        }
    }
    return 0;
}

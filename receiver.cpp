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

int offsets[] = {12,135,235,345,465,568,648,771};

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

    // Allocate memory and get the base address
    uint8_t* base_addr = (uint8_t*) calloc(4096 * 8, sizeof(uint8_t));
    //uint8_t* base_addr = (uint8_t*) malloc(sizeof(uint8_t) * 4096 * 9);
    printf("original base addr = %lx\n", base_addr);
    //base_addr = (uint8_t*)((( (uintptr_t)base_addr >> 12) + 1) << 12);
    printf("used base addr = %lx\n", base_addr);

    // Initialize memory pages
    //for (int i=0; i<8888; i++){
        //for (int j=0; j<4096; j++){
            //// the nth page has all bytes == n
            //base_addr[i * 4096 + j] = i;
        //}
    //}

    // flush all the memory
    for (int i=0; i<4096*8; i++){
        flush(base_addr + i);
    }

    // test functionality
    unsigned long res_time = 0;
    for (int i=0; i<10; i++){
        for (int j=0; j<8; j++){
            //uint8_t *target_addr = base_addr + j * 4096 + offsets[j];
            //uint8_t foo = *target_addr;
            res_time = probe(base_addr + j * 4096 + offsets[j]);

            printf("j = %d, respond time = %d\n", j, res_time);
        }
    }
    return 0;
}

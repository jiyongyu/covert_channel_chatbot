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
#include "algorithm"    // std::min
#include <sys/mann.h>

#define PAGE_SIZE   4096
#define SET_SIZE    64
#define NUM_SETS    64
#define NUM_WAYS    8

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

void sleep(int i){
    for (int ii =0; ii < i; ii++){
        for (int jj=0; jj<10000; jj++){
            ;
        }
    }
}

int main(int argc, char** argv){

    // Allocate memory and get the base address
    uint8_t* base_addr = (uint8_t*) malloc(sizeof(uint8_t) * PAGE_SIZE * 9);
    printf("original base addr = %lx\n", base_addr);
    base_addr = (uint8_t*)((( (uintptr_t)base_addr >> 12) + 1) << 12);
    printf("used base addr = %lx\n", base_addr);

    // Initialize memory pages
    for (int i=0; i<8; i++){
        for (int j=0; j<PAGE_SIZE; j++){
            // the nth page has all bytes == n
            base_addr[i * PAGE_SIZE + j] = i+8;
        }
    }

    // flush all the memory
    for (int i=0; i<PAGE_SIZE*8; i++){
        flush(base_addr + i);
    }

    // test functionality
    unsigned long res_time[NUM_SETS];
    unsigned long res_time_new = 0;

    for(int sleep = 1000; sleep < 20000; sleep += 100){
        for(int r=0; r<1000; r++){
    for (int i=0; i<10; i++){
        for (int j=0; j<NUM_SETS; j++){
            res_time[NUM_SETS] = 999;
        }
        // flush every lines in every sets
        for(int j=0; j<NUM_SETS; j++){
            for(int k=0; k<NUM_WAYS; k++){
                uint8_t* flush_addr = base_addr + k * PAGE_SIZE + j * SET_SIZE;
                flush(flush_addr);
            }
        }

        sleep(1000);

        // probe every lines in every sets
        // prefetching works after print 8 res_time_new
        for(int j=0; j<NUM_SETS; j++){
            for(int k=0; k<NUM_WAYS; k++){
                uint8_t* probe_addr = base_addr + k * PAGE_SIZE + j * SET_SIZE;
                res_time_new = probe(probe_addr);
                printf("res_time_new = %d\n", res_time_new);
            }
        }

        for (int j=0; j<NUM_SETS; j++){
            printf("j = %d, respond time = %d\n", j, res_time[j]);
        }
    }
        }
    }
    return 0;
}

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
#include <sys/mman.h>
#include <fcntl.h>
#include <assert.h>

#define PAGE_SIZE   4096

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
    int map_length = PAGE_SIZE * 9;
    int fd = open("/bin/ls", O_RDONLY);
    assert(fd > 0);
    const uint8_t* base_addr = (const uint8_t*) mmap(NULL, PAGE_SIZE * 9, PROT_READ, MAP_SHARED, fd, 0);
    assert(base_addr != MAP_FAILED);
    printf("original base addr = %lx\n", base_addr);
    base_addr = (uint8_t*)((( (uintptr_t)base_addr >> 12) + 1) << 12);
    printf("used base addr = %lx\n", base_addr);

    // flush all the memory
    for (int i=0; i<PAGE_SIZE*8; i++){
        flush(base_addr + i);
    }

    // test functionality
    unsigned long res_time[8];
    unsigned long res_time_new = 0;
    //for (int i=0; i<10; i++){
        //for (int j=0; j<8; j++){
            ////uint8_t *target_addr = base_addr + j * 4096 + offsets[j];
            ////uint8_t foo = *target_addr;
            //res_time = probe(base_addr + j * PAGE_SIZE + offsets[j]);

            //printf("j = %d, respond time = %d\n", j, res_time);
        //}
    //}
    

    for (int i=0; i<10; i++){
        for (int j=0; j<8; j++){
            res_time[j] = 999;
        }
        // flush every lines in every sets
        for(int j=0; j<8; j++){
            const uint8_t* flush_addr = base_addr + j * PAGE_SIZE + j;
            flush(flush_addr);
        }

        sleep(1000);

        // probe every lines in every sets
        // prefetching works after print 8 res_time_new
        for(int j=0; j<8; j++){
            const uint8_t* probe_addr = base_addr + j * PAGE_SIZE + j;
            res_time[j] = probe(probe_addr);
        }

        for (int j=0; j<8; j++){
            printf("j = %d, respond time = %d\n", j, res_time[j]);
        }
    }
    return 0;
}

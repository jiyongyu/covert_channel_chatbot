/*
 *      Construct covert-channel communication between sender process and receiver process
 *
 *      targeting L1
 *
 *      sender flush the cache lines
 */
#include "util.hpp"
#include "inttypes.h"
#include "algorithm"    // std::min

#define PAGE_SIZE   4096
#define SET_SIZE    64
#define NUM_SETS    64
#define NUM_WAYS    8


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
    uint8_t* base_addr = (uint8_t*) malloc(sizeof(uint8_t) * PAGE_SIZE * 512);
    printf("original base addr = %lx\n", base_addr);
    base_addr = (uint8_t*)((( (uintptr_t)base_addr >> 12) + 1) << 12);
    printf("used base addr = %lx\n", base_addr);

    // Initialize memory pages
    for (int i=0; i<256; i++){
        for (int j=0; j<PAGE_SIZE; j++){
            // the nth page has all bytes == n
            base_addr[i * PAGE_SIZE + j] = i % 8 + 8;
        }
    }

    // flush all the memory
    for (int i=0; i<PAGE_SIZE*256; i++){
        flush(base_addr + i);
    }

    // test functionality
    unsigned long res_time[NUM_WAYS];
    unsigned long res_time_new = 0;
    //for (int i=0; i<10; i++){
        //for (int j=0; j<8; j++){
            ////uint8_t *target_addr = base_addr + j * 4096 + offsets[j];
            ////uint8_t foo = *target_addr;
            //res_time = probe(base_addr + j * PAGE_SIZE + offsets[j]);

            //printf("j = %d, respond time = %d\n", j, res_time);
        //}
    //}
    
    char foo;
    //for (int i=0; i<10; i++){
        //// flush every lines in every sets
        //for(int j=0; j<8; j++){ // over sets
            //for(int k=0; k<NUM_WAYS; k++){ // over ways
                //uint8_t* load_addr = base_addr + (j * 8 + k) * PAGE_SIZE + j * SET_SIZE;
                //foo = *load_addr;
            //}
        //}

        //sleep(10000);

        //// probe every lines in every sets
        //// prefetching works after print 8 res_time_new
        //for(int j=0; j<8; j++){
            //for(int k=0; k<NUM_WAYS; k++){
                //uint8_t* probe_addr = base_addr + (j * 8 + k) * PAGE_SIZE + j * SET_SIZE;
                //res_time[j][k] = probe(probe_addr);
            //}
        //}
        //for(int j=0; j<8; j++){
            //for(int k=0; k<NUM_WAYS; k++){
                //printf("respond time = %d in set %d way %d \n", res_time[j][k], j, k);
            //}
        //}
    //}

    for (int i=0; i<10; i++){
        // load every lines in every sets
        for(int k=0; k<NUM_WAYS;k++){
            uint8_t* load_addr = base_addr + k * PAGE_SIZE;
            foo = *load_addr;
        }

        sleep(3000);

        // probe every lines in every sets
        // prefetching works after print 8 res_time_new
        for(int k=0; k<NUM_WAYS; k++){
            uint8_t* probe_addr = base_addr + k * PAGE_SIZE;
            res_time[k] = probe(probe_addr);
        }
        for(int k=0; k<NUM_WAYS; k++){
            printf("respond time = %d in way %d\n", res_time[k], k);
        }
    }
    return 0;
}

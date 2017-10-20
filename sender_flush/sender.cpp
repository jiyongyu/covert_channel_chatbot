/*
 *      Construct covert-channel communication between sender process and receiver process
 *
 *      targeting L1
 *
 *      sender flush the cache lines
 */
#include "util.hpp"

#define threshold       120
#define PAGE_SIZE       4096    // bytes
#define SET_SIZE        64      // bytes (per way)
#define NUM_SETS        64
#define NUM_WAYS        8


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
    uint8_t* base_addr = (uint8_t*) malloc(sizeof(uint8_t) * PAGE_SIZE * 512);
    printf("original base addr = %lx\n", base_addr);
    base_addr = (uint8_t*) ((( (uintptr_t)base_addr >> 12) + 1) << 12);
    printf("used base addr = %lx\n", base_addr);

    // Initialize the memory pages
    for (int i=0; i<256; i++){
        for (int j=0; j<PAGE_SIZE; j++){
            // the nth page has all bytes == n
            base_addr[i * PAGE_SIZE + j] = i % 8;
        }
    }

    // flush all the memory
    for (int i=0; i<PAGE_SIZE*256; i++){
        flush(base_addr + i);
    }

    // test functionality
    char text_buf[] = "send me\n";
    char char_sent = '0';
    char foo;
    for(int i=0; i<128; i++){
        if (char_sent == '\n')
            break;

        while(1){
            char_sent = text_buf[i];
            // send char_sent

            //for(int j=0; j<8; j++){
                ////if(char_sent & 0x1) {   // send 1 on set j
                    //for (int k=0; k<NUM_WAYS; k++){
                        //uint8_t* load_addr = base_addr + (j * 8 + k) * PAGE_SIZE + j * SET_SIZE;
                        //foo = *load_addr;
                    //}
                ////}
                //char_sent = char_sent >> 1;
            //}
                    for (int k=0; k<NUM_WAYS; k++){
                        uint8_t* load_addr = base_addr + k * PAGE_SIZE;
                        foo = *load_addr;
                    }
        }
    }
    return 0;
}


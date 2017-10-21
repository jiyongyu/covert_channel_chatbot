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

int PAGE_SIZE = 4096;

int STRIDE = PAGE_SIZE * 2;

int offset[] = {12,135,235,345,465,568,648,771};

inline uint64_t probe(const uint8_t* addr){
    volatile uint64_t time;
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

char getSentChar(const uint8_t* base_addr);

void flushMem(const uint8_t* base_addr, int size_in_bytes);

int main(int argc, char** argv){

    // Allocate memory and get the base address
    int map_length = PAGE_SIZE * 17;
    int fd = open("/bin/ls", O_RDONLY);
    assert(fd > 0);
    const uint8_t* base_addr = (const uint8_t*) mmap(NULL, map_length, PROT_READ, MAP_SHARED, fd, 0);
    assert(base_addr != MAP_FAILED);
    printf("original base addr = %lx\n", base_addr);
    base_addr = (uint8_t*)((( (uintptr_t)base_addr >> 12) + 1) << 12);
    printf("used base addr = %lx\n", base_addr);

    // flush all the memory
    flushMem(base_addr, PAGE_SIZE*16);

    printf("Please press enter.\n");
    char text_buf[2];
    fgets(text_buf, sizeof(text_buf), stdin);

    printf("Receiver now listening.\n");

    bool listening = true;
    bool get_mark = false;
    while(listening){
        char temp_char;
        temp_char = getSentChar(base_addr);
        if(temp_char != 0){
            if(get_mark && temp_char != 1){
                printf("%c\n", temp_char, (uint8_t)temp_char);
                get_mark = false;
            }
            if(temp_char == 1){
                get_mark = true;
            }
        }
        flushMem(base_addr, PAGE_SIZE*16);
    }

    printf("Receiver finished.\n");

    return 0;
}

void flushMem(const uint8_t* base_addr, int size_in_bytes){
    for (int i=0; i<size_in_bytes; i++)
        flush(base_addr + i);
}

char getSentChar(const uint8_t* base_addr){
    unsigned long res_time[8];
    char prev_char = (char) 1;
    char curr_char = (char) 0;
    while(prev_char != curr_char){
        prev_char = curr_char;
        curr_char = (char) 0;

        for (int j=0; j<8; j++){
            res_time[j] = 0;
        }

        // flush every lines in every sets
        for(int j=0; j<8; j++){
            const uint8_t* flush_addr = base_addr + j * STRIDE + offset[j];
            flush(flush_addr);
        }

        sleep(10);

        // probe every lines in every sets
        // prefetching works after print 8 res_time_new
        for(int j=0; j<8; j++){
            const uint8_t* probe_addr = base_addr + j * STRIDE + offset[j];
            res_time[j] += probe(probe_addr);
        }

        for (int i=0; i<8; i++){
            if (res_time[i] < 200){ // hit
                curr_char = curr_char | (1 << i);
            }
        }
    }
    return curr_char;
}

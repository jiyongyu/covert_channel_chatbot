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

int RECV_MODE = 0;
int SEND_MODE = 1;

int SEND_TIMES = 50000;

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

void sleep(int i);

char getSentChar(const uint8_t* base_addr);

void flushMem(const uint8_t* base_addr, int size_in_bytes);

int main(int argc, char** argv){

    // Allocate the memory and get base address
    int map_length = PAGE_SIZE * 17;
    int stride = PAGE_SIZE*2;
    int fd = open("/bin/ls", O_RDONLY);
    assert(fd > 0);
    const uint8_t* base_addr = (const uint8_t*)mmap(NULL, map_length, PROT_READ, MAP_SHARED, fd, 0);
    base_addr = (uint8_t*) ((( (uintptr_t)base_addr >> 12) + 1) << 12);

    // flush all the memory
    flushMem(base_addr, PAGE_SIZE * 16);


    char char_recv = '0';
    char char_sent = '0';
    char foo = '0';
    bool flag = false;
    bool mode = SEND_MODE;

    printf("\n\n * * * Welcome to the cache side-channel world. * * * \n\n\n");

    while(1){
        if (mode == SEND_MODE){
            printf("You're in SENDER mode. Press 'recv' to enter RECEIVER mode.\n");
        }

        if (mode == RECV_MODE){
            char_recv = getSentChar(base_addr);
            if(flag && char_recv != 1){
                printf("%c\n", char_recv);
                flag = false;
                if(char_recv == '\n')
                    mode = SEND_MODE;
            }
            if(char_recv == 1){
                flag = true;
            }
        } else if (mode == SEND_MODE){
            char text_buf[128];
            for(int i=0; i<128; i++){
                text_buf[i] = 0;
            }
            fgets(text_buf, sizeof(text_buf), stdin);
            if(strcmp(text_buf, "recv\n") == 0){
                printf("You're in RECEIVER mode. Now send message on the other end.\n");
                mode = RECV_MODE;
                continue;
            }

            clock_t begin = clock();
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
            double bytes_per_sec = strlen(text_buf) / ( (double) (end-begin) / CLOCKS_PER_SEC );
            printf("%f bytes per second\n\n", bytes_per_sec);
        } else {
            assert(0);
        }
    }

    return 0;
}


void flushMem(const uint8_t* base_addr, int size_in_bytes){
    for(int i=0; i<size_in_bytes; i++)
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

void sleep(int i){
    for (int ii =0; ii < i; ii++){
        for (int jj=0; jj<10000; jj++){
            ;
        }
    }
}

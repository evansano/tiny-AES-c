#include <stdio.h>
#include <string.h>
#include <stdint.h>

// Enable ECB, CTR and CBC mode. Note this can be done before including aes.h or at compile-time.
// E.g. with GCC by using the -D flag: gcc -c aes.c -DCBC=0 -DCTR=1 -DECB=1
#define CTR 1
#define LOOPS 50
#include "aes.h"

int arrSizes[8] = {1024, 8192, 65536, 1048576, 5242880, 10485760, 104857600, 1073741824};
char* arrSizeHuman[8] = {"1 kB", "8 kB", "64 kB", "1 MB" "5 MB", "10 MB", "100 MB", "1 GB"};
// int arrSizes[3] = {1024, 8192, 65536};
// char* arrSizeHuman[3] = {"1 kB", "8 kB", "64 kB"};
static void phex(uint8_t* str);


int comm_sz, my_rank;

int main(int argc, char **argv)
{
    double start, end, elapsed;
    #ifdef AES128
        printf("\nTesting AES128\n\n");
    #elif defined(AES192)
        printf("\nTesting AES192\n\n");
    #elif defined(AES256)
        printf("\nTesting AES256\n\n");
    #else
        printf("You need to specify a symbol between AES128, AES192 or AES256. Exiting");
        return 0;
    #endif
    MPI_Init(NULL, NULL);
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_rank(comm, &my_rank);
    MPI_Comm_size(comm, &comm_sz);
    uint8_t key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    uint8_t iv[16]  = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
    struct AES_ctx ctx;
    long long int num_blocks, local_num_blocks;
    uint8_t* local_in;
    uint8_t * in;
    FILE *fh;
    int i, j, k;
    if(my_rank == 0){
        
        fh = fopen(argv[1], "w+");
        fprintf(fh, "Processes: %d\n", comm_sz);

    }
    for(i = 0; i < 8 ; i++){
        elapsed = 0.0;
        for(k = 0 ; k < LOOPS ; k++){
            if(my_rank == 0){
                #if defined AES_BLOCKLEN
                  num_blocks = arrSizes[i] / AES_BLOCKLEN;
                #else
                  num_blocks = arrSizes[i] / 16;
                #endif
                local_num_blocks = num_blocks / comm_sz;

                in = malloc(arrSizes[i]*sizeof(int));
                // Fill all elemnts with hex value of the ASCII 'A'
                for(j = 0 ; j < arrSizes[i] ; j++){
                    in[j] = 0x41;
                }
                start = MPI_Wtime();
            }
            
            MPI_Bcast(&local_num_blocks, 1, MPI_LONG_LONG, 0, comm);
            local_in = malloc(AES_BLOCKLEN*local_num_blocks*sizeof(int));
            MPI_Scatter(in, AES_BLOCKLEN*local_num_blocks, MPI_INT,
                          local_in, AES_BLOCKLEN*local_num_blocks, MPI_INT, 0, comm);
            AES_init_ctx_iv(&ctx, key, iv);
            AES_CTR_xcrypt_buffer(&ctx, local_in, local_num_blocks*AES_BLOCKLEN);
            if(my_rank == 0){
                end = MPI_Wtime();
                elapsed += end-start;
                free(in);
            }
            MPI_Barrier(comm);

            
            free(local_in);
        }
        if(my_rank == 0){
            elapsed /= LOOPS;
            fprintf(fh, "Runs: %d\nFile size: %s\nAverage Elapsed time: %f seconds\n\n", LOOPS, arrSizeHuman[i], elapsed);
        }
    }
    if(my_rank == 0){
        fclose(fh);
    }

    MPI_Finalize();
    return 0;
}


// prints string as hex
static void phex(uint8_t* str)
{

#ifdef AES128
    uint8_t len = 16;
#elif defined(AES192)
    uint8_t len = 24;
#elif defined(AES256)
    uint8_t len = 32;
#endif

    unsigned char i;
    for (i = 0; i < len; ++i)
        printf("%.2x", str[i]);
    printf("\n");
}






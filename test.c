#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define CTR 1
#define LOOPS 100
#include "aes.h"

int arrSizes[6] = {      1024,    8192,   65536,  1048576, 8388608, 67108864};//, 1073741824};
char* arrSizeHuman[6] = {"1 kB", "8 kB", "64 kB", "1 MB",  "8 MB",  "64 MB"};//,  "1 GB"};
// int arrSizes[3] = {1024, 8192, 65536};
// char* arrSizeHuman[3] = {"1 kB", "8 kB", "64 kB"};


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
    int key[16] = { 0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c };
    int iv[16]  = { 0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };
    struct AES_ctx ctx;
    long long int num_blocks, local_num_blocks;
    char* local_in;
    char * in;
    FILE *fh;
    int i, j, k;
    if(my_rank == 0){
        
        fh = fopen(argv[1], "w+");
        fprintf(fh, "Processes: %d\n", comm_sz);

    }
    for(i = 0; i < 6 ; i++){
        if(my_rank == 0){
            elapsed = 0.0;
        }
        for(k = 0 ; k < LOOPS ; k++){

            if(my_rank == 0){
                //printf("[%s] pre-setup\n", arrSizeHuman[i]);
                #if defined AES_BLOCKLEN
                  num_blocks = arrSizes[i] / AES_BLOCKLEN;
                #else
                  num_blocks = arrSizes[i] / 16;
                #endif
                local_num_blocks = num_blocks / comm_sz;

                in = malloc(arrSizes[i]*sizeof(char));
                // Fill all elemnts with hex value of the ASCII 'A'
                for(j = 0 ; j < arrSizes[i] ; j++){
                    in[j] = 0x41;
                }
                //printf("[%s] post-setup\n", arrSizeHuman[i]);
            }
            MPI_Barrier(comm);
            if(my_rank == 0){
                start = MPI_Wtime();
            }
            
            MPI_Bcast(&local_num_blocks, 1, MPI_LONG_LONG, 0, comm);
            local_in = malloc(arrSizes[i]/comm_sz*sizeof(char));
            // if(my_rank == 0){
            //     printf("[%s] pre-scatter\n", arrSizeHuman[i]);
            // }
            MPI_Scatter(in, arrSizes[i]/comm_sz, MPI_CHAR, local_in, arrSizes[i]/comm_sz, MPI_CHAR, 0, comm);
            // if(my_rank == 0){
            //     printf("[%s] post-scatter\n", arrSizeHuman[i]);
            // }
            AES_init_ctx_iv(&ctx, key, iv);
            AES_CTR_xcrypt_buffer(&ctx, local_in, arrSizes[i]/comm_sz);
            MPI_Gather(local_in, arrSizes[i]/comm_sz, MPI_CHAR, in, arrSizes[i]/comm_sz, MPI_CHAR, 0, comm);
            
            if(my_rank == 0){
                end = MPI_Wtime();
                elapsed += end-start;
                free(in);
            }

            MPI_Barrier(comm);
            free(local_in);
        } // End one loop
        
        if(my_rank == 0){
            elapsed /= LOOPS;
            fprintf(fh, "Runs: %d\nFile size: %s\nAverage Elapsed time: %f seconds\n\n", LOOPS, arrSizeHuman[i], elapsed);
        }

    } // End of one filesize
    if(my_rank == 0){
        fclose(fh);
    }

    MPI_Finalize();
    return 0;
}







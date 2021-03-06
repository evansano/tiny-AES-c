#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>
#include <mpi.h>
// #define the macros below to 1/0 to enable/disable the mode of operation.
//
// CBC enables AES encryption in CBC-mode of operation.
// CTR enables encryption in counter-mode.
// ECB enables the basic ECB 16-byte block algorithm. All can be enabled simultaneously.

// The #ifndef-guard allows it to be configured before #include'ing or at compile time.

#ifndef CTR
  #define CTR 1
#endif


#define AES128 1
//#define AES192 1
//#define AES256 1

#define AES_BLOCKLEN 16 //Block length in bytes AES is 128b block only

#if defined(AES256) && (AES256 == 1)
    #define AES_KEYLEN 32
    #define AES_keyExpSize 240
#elif defined(AES192) && (AES192 == 1)
    #define AES_KEYLEN 24
    #define AES_keyExpSize 208
#else
    #define AES_KEYLEN 16   // Key length in bytes
    #define AES_keyExpSize 176
#endif

struct AES_ctx
{
  int RoundKey[AES_keyExpSize];
#if defined(CTR) && (CTR == 1)
  int Iv[AES_BLOCKLEN];
#endif
};

void AES_init_ctx(struct AES_ctx* ctx, const int* key);
#if defined(CTR) && (CTR == 1)
void AES_init_ctx_iv(struct AES_ctx* ctx, const int* key, const int* iv);
void AES_ctx_set_iv(struct AES_ctx* ctx, const int* iv);
#endif

#if defined(CTR) && (CTR == 1)

// Same function for encrypting as for decrypting. 
// IV is incremented for every block, and used after encryption as XOR-compliment for output
// Suggesting https://en.wikipedia.org/wiki/Padding_(cryptography)#PKCS7 for padding scheme
// NOTES: you need to set IV in ctx with AES_init_ctx_iv() or AES_ctx_set_iv()
//        no IV should ever be reused with the same key 
void AES_CTR_xcrypt_buffer(struct AES_ctx* ctx, int* buf,int length);

#endif // #if defined(CTR) && (CTR == 1)


#endif //_AES_H_

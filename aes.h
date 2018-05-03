// Evan Sano refined this file from skeleton code to only relevant code
#ifndef _AES_H_
#define _AES_H_

#include <stdint.h>
#include <mpi.h>

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
  uint8_t RoundKey[AES_keyExpSize];
#if defined(CTR) && (CTR == 1)
  uint8_t Iv[AES_BLOCKLEN];
#endif
};

void AES_init_ctx(struct AES_ctx* ctx, const uint8_t* key);
#if defined(CTR) && (CTR == 1)
void AES_init_ctx_iv(struct AES_ctx* ctx, const uint8_t* key, const uint8_t* iv);
void AES_ctx_set_iv(struct AES_ctx* ctx, const uint8_t* iv);
#endif

#if defined(CTR) && (CTR == 1)

// Same function for encrypting as for decrypting. 
void AES_CTR_xcrypt_buffer(struct AES_ctx* ctx, char* buf,int length);

#endif // #if defined(CTR) && (CTR == 1)


#endif //_AES_H_

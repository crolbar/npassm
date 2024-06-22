#include <stdio.h>
#include <openssl/conf.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/aes.h>
#include <string.h>


char* encrypt_db(
        FILE* f,
        char* password,
        unsigned char* plaintext,
        int* len)
{

    unsigned char salt[16];
    RAND_bytes(salt, sizeof(salt));
    fwrite(salt, 1, sizeof(salt), f);

    unsigned char iv[16];
    RAND_bytes(iv, sizeof(iv));
    fwrite(iv, 1, sizeof(iv), f);

    unsigned char key[32];
    PKCS5_PBKDF2_HMAC_SHA1(
            password, strlen(password),
            salt, sizeof(salt),
            100000,
            sizeof(key), key);


    int c_len = *len + AES_BLOCK_SIZE;
    int f_len = 0;

    unsigned char* cipher = malloc(c_len);

    {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

        EVP_EncryptInit_ex(ctx,
                EVP_aes_256_cbc(), NULL,
                key, iv);

        EVP_EncryptUpdate(ctx,
                cipher, &c_len,
                plaintext, *len);

        EVP_EncryptFinal_ex(ctx,
                cipher+c_len, &f_len);

        EVP_CIPHER_CTX_free(ctx);
    }

    *len = c_len + f_len;

    return (char*)cipher;
}

char* decrypt_db(FILE* f, int* len, char* password) {
    unsigned char salt[16];
    unsigned char iv[16];

    if (fread(salt, 1, sizeof(salt), f)) {};
    if (fread(iv, 1, sizeof(iv), f)) {};
    if (fread(len, sizeof(*len), 1, f)) {};

    unsigned char* cipher = malloc(*len);
    if (fread(cipher, 1, *len, f)) {};



    unsigned char key[32];
    PKCS5_PBKDF2_HMAC_SHA1(
        password, strlen(password),
        salt, sizeof(salt),
        100000,
        sizeof(key), key);



    int u_len = *len;
    int f_len = 0;

    unsigned char* ser_db = malloc(*len);

    {
        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();

        EVP_DecryptInit_ex(ctx,
                EVP_aes_256_cbc(), NULL,
                key, iv);


        EVP_DecryptUpdate(ctx,
                ser_db, &u_len,
                cipher, *len);

        if (EVP_DecryptFinal_ex(ctx,
                ser_db+u_len, &f_len) != 1)
        {
            printf("Bad decrypt. Meybe wrong password?");
            exit(1);
        }

        EVP_CIPHER_CTX_free(ctx);
    }

    *len = u_len + f_len;

    return (char*)ser_db;
}

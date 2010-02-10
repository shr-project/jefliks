#ifndef __SHA1_H__
#define __SHA1_H__

#define RUNTIME_ENDIAN_CHECK

typedef struct _sha1_ctx sha1_ctx;

sha1_ctx *sha1_init();
void sha1_update(sha1_ctx* context, unsigned char* data, unsigned int len);
void sha1_final(sha1_ctx* context, unsigned char digest[20]);
void sha1_print(unsigned char digest[20], char data[41]);

#endif

#ifndef __BASE64_H__
#define __BASE64_H__

int base64_encode(const unsigned char *source, size_t sourcelen, char *target, size_t targetlen);
size_t base64_decode(const char *source, unsigned char *target, size_t targetlen);

#endif

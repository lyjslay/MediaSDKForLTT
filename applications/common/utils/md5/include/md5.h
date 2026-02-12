#ifndef MD5_H
#define MD5_H
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

typedef struct
{
	uint32_t count[2];
	uint32_t state[4];
	unsigned char buffer[64];
} CVI_MD5_CTX_S;


#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))

#define FF(a,b,c,d,x,s,ac) \
{ \
	a += F(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}
#define GG(a,b,c,d,x,s,ac) \
{ \
	a += G(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}
#define HH(a,b,c,d,x,s,ac) \
{ \
	a += H(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}
#define II(a,b,c,d,x,s,ac) \
{ \
	a += I(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}
void CVI_MD5_Init(CVI_MD5_CTX_S *context);
void CVI_MD5_Update(CVI_MD5_CTX_S *context, unsigned char *input, uint32_t inputlen);
void CVI_MD5_Final(CVI_MD5_CTX_S *context, unsigned char digest[16]);
void CVI_MD5_Transform(uint32_t state[4], unsigned char block[64]);
void CVI_MD5_Encode(unsigned char *output, uint32_t *input, uint32_t len);
void CVI_MD5_Decode(uint32_t *output, unsigned char *input, uint32_t len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* End of #ifdef __cplusplus */

#endif
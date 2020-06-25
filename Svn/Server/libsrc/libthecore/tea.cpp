#include "tea.h"
#include <cstring>

#define TEA_ROUND		32
#define DELTA			0x9E3779B9

char tea_nilbuf[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

inline void tea_code(const uint32_t sz, const uint32_t sy, const uint32_t *key, uint32_t *dest)
{
	uint32_t	y = sy, z = sz, sum = 0;

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    y	+= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);
    sum	+= DELTA;
    z	+= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);

    *(dest++)	= y;
    *dest	= z;
}

inline void tea_decode(const uint32_t sz, const uint32_t sy, const uint32_t *key, uint32_t *dest)
{
	uint32_t y = sy, z = sz, sum = DELTA * TEA_ROUND;

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    z -= ((y << 4 ^ y >> 5) + y) ^ (sum + key[sum >> 11 & 3]);
    sum -= DELTA;
    y -= ((z << 4 ^ z >> 5) + z) ^ (sum + key[sum & 3]);

    *(dest++)	= y;
    *dest	= z;
}

int32_t TEA_Encrypt(uint32_t *dest, const uint32_t *src, const uint32_t * key, int32_t size)
{
    int32_t		i;
    int32_t		resize;

    if (size % 8 != 0)
    {
	resize = size + 8 - (size % 8);
	memset((char *) src + size, 0, resize - size);
    }
    else
	resize = size;

    for (i = 0; i < resize >> 3; i++, dest += 2, src += 2)
	tea_code(*(src + 1), *src, key, dest);

    return (resize);
}

int32_t TEA_Decrypt(uint32_t *dest, const uint32_t *src, const uint32_t * key, int32_t size)
{
    int32_t		i;
    int32_t		resize;

    if (size % 8 != 0)
	resize = size + 8 - (size % 8);
    else
	resize = size;

    for (i = 0; i < resize >> 3; i++, dest += 2, src += 2)
	tea_decode(*(src + 1), *src, key, dest);

    return (resize);
}


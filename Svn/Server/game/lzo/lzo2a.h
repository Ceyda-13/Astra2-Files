


#ifndef __LZO2A_H
#define __LZO2A_H

#ifndef __LZOCONF_H
#include <lzoconf.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif



#define LZO2A_MEM_DECOMPRESS    (0)

LZO_EXTERN(int)
lzo2a_decompress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );

LZO_EXTERN(int)
lzo2a_decompress_safe   ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



#define LZO2A_999_MEM_COMPRESS  ((lzo_uint32) (8 * 16384L * sizeof(short)))

#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo2a_999_compress      ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
#endif



#ifdef __cplusplus
}
#endif

#endif


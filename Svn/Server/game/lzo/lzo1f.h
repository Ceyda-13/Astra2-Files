


#ifndef __LZO1F_H
#define __LZO1F_H

#ifndef __LZOCONF_H
#include <lzoconf.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif




#define LZO1F_MEM_COMPRESS      ((lzo_uint32) (16384L * lzo_sizeof_dict_t))
#define LZO1F_MEM_DECOMPRESS    (0)


LZO_EXTERN(int)
lzo1f_decompress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );

LZO_EXTERN(int)
lzo1f_decompress_safe   ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



LZO_EXTERN(int)
lzo1f_1_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



#define LZO1F_999_MEM_COMPRESS  ((lzo_uint32) (5 * 16384L * sizeof(short)))

#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1f_999_compress      ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
#endif



#ifdef __cplusplus
}
#endif

#endif


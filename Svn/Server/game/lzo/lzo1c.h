


#ifndef __LZO1C_H
#define __LZO1C_H

#ifndef __LZOCONF_H
#include <lzoconf.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif




#define LZO1C_MEM_COMPRESS      ((lzo_uint32) (16384L * lzo_sizeof_dict_t))
#define LZO1C_MEM_DECOMPRESS    (0)


#define LZO1C_BEST_SPEED             1
#define LZO1C_BEST_COMPRESSION       9
#define LZO1C_DEFAULT_COMPRESSION  (-1)


LZO_EXTERN(int)
lzo1c_compress          ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem,
                                int compression_level );

LZO_EXTERN(int)
lzo1c_decompress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );

LZO_EXTERN(int)
lzo1c_decompress_safe   ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



LZO_EXTERN(int)
lzo1c_1_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_2_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_3_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_4_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_5_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_6_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_7_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_8_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1c_9_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



#define LZO1C_99_MEM_COMPRESS   ((lzo_uint32) (65536L * lzo_sizeof_dict_t))

#if !defined(LZO_99_UNSUPPORTED)
LZO_EXTERN(int)
lzo1c_99_compress       ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
#endif


#define LZO1C_999_MEM_COMPRESS  ((lzo_uint32) (5 * 16384L * sizeof(short)))

#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1c_999_compress      ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
#endif



#ifdef __cplusplus
}
#endif

#endif


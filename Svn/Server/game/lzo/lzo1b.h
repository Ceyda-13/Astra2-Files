


#ifndef __LZO1B_H
#define __LZO1B_H

#ifndef __LZOCONF_H
#include <lzoconf.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif




#define LZO1B_MEM_COMPRESS      ((lzo_uint32) (16384L * lzo_sizeof_dict_t))
#define LZO1B_MEM_DECOMPRESS    (0)


#define LZO1B_BEST_SPEED             1
#define LZO1B_BEST_COMPRESSION       9
#define LZO1B_DEFAULT_COMPRESSION  (-1)


LZO_EXTERN(int)
lzo1b_compress          ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem,
                                int compression_level );

LZO_EXTERN(int)
lzo1b_decompress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );

LZO_EXTERN(int)
lzo1b_decompress_safe   ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



LZO_EXTERN(int)
lzo1b_1_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1b_2_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1b_3_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1b_4_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1b_5_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1b_6_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1b_7_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1b_8_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
LZO_EXTERN(int)
lzo1b_9_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



#define LZO1B_99_MEM_COMPRESS   ((lzo_uint32) (65536L * lzo_sizeof_dict_t))

#if !defined(LZO_99_UNSUPPORTED)
LZO_EXTERN(int)
lzo1b_99_compress       ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
#endif


#define LZO1B_999_MEM_COMPRESS  ((lzo_uint32) (3 * 65536L * sizeof(lzo_uint32)))

#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1b_999_compress      ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
#endif



#ifdef __cplusplus
}
#endif

#endif


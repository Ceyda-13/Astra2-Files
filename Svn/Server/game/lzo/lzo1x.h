


#ifndef __LZO1X_H
#define __LZO1X_H

#ifndef __LZOCONF_H
#include <lzoconf.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif




#define LZO1X_MEM_COMPRESS      LZO1X_1_MEM_COMPRESS
#define LZO1X_MEM_DECOMPRESS    (0)
#define LZO1X_MEM_OPTIMIZE      (0)


LZO_EXTERN(int)
lzo1x_decompress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );

LZO_EXTERN(int)
lzo1x_decompress_safe   ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



#define LZO1X_1_MEM_COMPRESS    ((lzo_uint32) (16384L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



#define LZO1X_1_11_MEM_COMPRESS ((lzo_uint32) (2048L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_11_compress     ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );


#define LZO1X_1_12_MEM_COMPRESS ((lzo_uint32) (4096L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_12_compress     ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );


#define LZO1X_1_15_MEM_COMPRESS ((lzo_uint32) (32768L * lzo_sizeof_dict_t))

LZO_EXTERN(int)
lzo1x_1_15_compress     ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



#define LZO1X_999_MEM_COMPRESS  ((lzo_uint32) (14 * 16384L * sizeof(short)))

#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1x_999_compress      ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
#endif



#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1x_999_compress_dict     ( const lzo_byte *in , lzo_uint  in_len,
                                    lzo_byte *out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_byte *dict, lzo_uint dict_len );

LZO_EXTERN(int)
lzo1x_999_compress_level    ( const lzo_byte *in , lzo_uint  in_len,
                                    lzo_byte *out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_byte *dict, lzo_uint dict_len,
                                    lzo_progress_callback_t cb,
                                    int compression_level );
#endif

LZO_EXTERN(int)
lzo1x_decompress_dict_safe ( const lzo_byte *in,  lzo_uint  in_len,
                                   lzo_byte *out, lzo_uintp out_len,
                                   lzo_voidp wrkmem,
                             const lzo_byte *dict, lzo_uint dict_len );



LZO_EXTERN(int)
lzo1x_optimize          (       lzo_byte *in , lzo_uint  in_len,
                                lzo_byte *out, lzo_uintp out_len,
                                lzo_voidp wrkmem );



#ifdef __cplusplus
}
#endif

#endif


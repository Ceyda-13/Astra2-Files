


#ifndef __LZO1Y_H
#define __LZO1Y_H

#ifndef __LZOCONF_H
#include <lzoconf.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif




#define LZO1Y_MEM_COMPRESS      ((lzo_uint32) (16384L * lzo_sizeof_dict_t))
#define LZO1Y_MEM_DECOMPRESS    (0)
#define LZO1Y_MEM_OPTIMIZE      (0)


LZO_EXTERN(int)
lzo1y_decompress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );

LZO_EXTERN(int)
lzo1y_decompress_safe   ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



LZO_EXTERN(int)
lzo1y_1_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



#define LZO1Y_999_MEM_COMPRESS  ((lzo_uint32) (14 * 16384L * sizeof(short)))

#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1y_999_compress      ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
#endif




#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1y_999_compress_dict     ( const lzo_byte *in , lzo_uint  in_len,
                                    lzo_byte *out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_byte *dict, lzo_uint dict_len );

LZO_EXTERN(int)
lzo1y_999_compress_level    ( const lzo_byte *in , lzo_uint  in_len,
                                    lzo_byte *out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_byte *dict, lzo_uint dict_len,
                                    lzo_progress_callback_t cb,
                                    int compression_level );
#endif

LZO_EXTERN(int)
lzo1y_decompress_dict_safe ( const lzo_byte *in,  lzo_uint  in_len,
                                   lzo_byte *out, lzo_uintp out_len,
                                   lzo_voidp wrkmem,
                             const lzo_byte *dict, lzo_uint dict_len );



LZO_EXTERN(int)
lzo1y_optimize          (       lzo_byte *in , lzo_uint  in_len,
                                lzo_byte *out, lzo_uintp out_len,
                                lzo_voidp wrkmem );



#ifdef __cplusplus
}
#endif

#endif





#ifndef __LZO1Z_H
#define __LZO1Z_H

#ifndef __LZOCONF_H
#include <lzoconf.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif




#define LZO1Z_MEM_COMPRESS      ((lzo_uint32) (16384L * lzo_sizeof_dict_t))
#define LZO1Z_MEM_DECOMPRESS    (0)
#define LZO1Z_MEM_OPTIMIZE      (0)


LZO_EXTERN(int)
lzo1z_decompress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );

LZO_EXTERN(int)
lzo1z_decompress_safe   ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );



#if 0
LZO_EXTERN(int)
lzo1z_1_compress        ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
#endif



#define LZO1Z_999_MEM_COMPRESS  ((lzo_uint32) (14 * 16384L * sizeof(short)))

#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1z_999_compress      ( const lzo_byte *src, lzo_uint  src_len,
                                lzo_byte *dst, lzo_uintp dst_len,
                                lzo_voidp wrkmem );
#endif



#if !defined(LZO_999_UNSUPPORTED)
LZO_EXTERN(int)
lzo1z_999_compress_dict     ( const lzo_byte *in , lzo_uint  in_len,
                                    lzo_byte *out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_byte *dict, lzo_uint dict_len );

LZO_EXTERN(int)
lzo1z_999_compress_level    ( const lzo_byte *in , lzo_uint  in_len,
                                    lzo_byte *out, lzo_uintp out_len,
                                    lzo_voidp wrkmem,
                              const lzo_byte *dict, lzo_uint dict_len,
                                    lzo_progress_callback_t cb,
                                    int compression_level );
#endif

LZO_EXTERN(int)
lzo1z_decompress_dict_safe ( const lzo_byte *in,  lzo_uint  in_len,
                                   lzo_byte *out, lzo_uintp out_len,
                                   lzo_voidp wrkmem,
                             const lzo_byte *dict, lzo_uint dict_len );



#if 0
LZO_EXTERN(int)
lzo1z_optimize          (       lzo_byte *in , lzo_uint  in_len,
                                lzo_byte *out, lzo_uintp out_len,
                                lzo_voidp wrkmem );
#endif



#ifdef __cplusplus
}
#endif

#endif







#ifndef __LZOCONF_H
#  include <lzoconf.h>
#endif

#ifndef __LZO16BIT_H
#define __LZO16BIT_H

#if defined(__LZO_STRICT_16BIT)
#if (UINT_MAX < LZO_0xffffffffL)

#ifdef __cplusplus
extern "C" {
#endif



#ifndef LZO_99_UNSUPPORTED
#define LZO_99_UNSUPPORTED
#endif
#ifndef LZO_999_UNSUPPORTED
#define LZO_999_UNSUPPORTED
#endif

typedef unsigned int        lzo_uint;
typedef int                 lzo_int;
#define LZO_UINT_MAX        UINT_MAX
#define LZO_INT_MAX         INT_MAX

#define lzo_sizeof_dict_t   sizeof(lzo_uint)



#if defined(__LZO_DOS16) || defined(__LZO_WIN16)

#if 0
#define __LZO_MMODEL        __far
#else
#define __LZO_MMODEL
#endif

#endif


#ifdef __cplusplus
}
#endif

#endif
#endif

#endif





#ifndef __LZOUTIL_H
#define __LZOUTIL_H

#ifndef __LZOCONF_H
#include <lzoconf.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif



LZO_EXTERN(lzo_voidp) lzo_alloc(lzo_uint _nelems, lzo_uint _size);
LZO_EXTERN(lzo_voidp) lzo_malloc(lzo_uint _size);
LZO_EXTERN(void) lzo_free(lzo_voidp _ptr);

typedef lzo_voidp (__LZO_ENTRY *lzo_alloc_hook_t) (lzo_uint, lzo_uint);
typedef void (__LZO_ENTRY *lzo_free_hook_t) (lzo_voidp);

LZO_EXTERN_VAR(lzo_alloc_hook_t) lzo_alloc_hook;
LZO_EXTERN_VAR(lzo_free_hook_t) lzo_free_hook;



#if !defined(LZO_FILEP)
#  define LZO_FILEP             void *
#endif

LZO_EXTERN(lzo_uint)
lzo_fread(LZO_FILEP f, lzo_voidp buf, lzo_uint size);
LZO_EXTERN(lzo_uint)
lzo_fwrite(LZO_FILEP f, const lzo_voidp buf, lzo_uint size);


#if (LZO_UINT_MAX <= UINT_MAX)
#  define lzo_fread(f,b,s)      (fread(b,1,s,f))
#  define lzo_fwrite(f,b,s)     (fwrite(b,1,s,f))
#endif


#ifdef __cplusplus
}
#endif

#endif


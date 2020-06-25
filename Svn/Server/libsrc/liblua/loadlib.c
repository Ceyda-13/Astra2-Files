#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"


#undef LOADLIB


#ifdef USE_DLOPEN
#define LOADLIB

#include <dlfcn.h>

static int loadlib(lua_State *L)
{
 const char *path=luaL_checkstring(L,1);
 const char *init=luaL_checkstring(L,2);
 void *lib=dlopen(path,RTLD_NOW);
 if (lib!=NULL)
 {
  lua_CFunction f=(lua_CFunction) dlsym(lib,init);
  if (f!=NULL)
  {
   lua_pushlightuserdata(L,lib);
   lua_pushcclosure(L,f,1);
   return 1;
  }
 }
 lua_pushnil(L);
 lua_pushstring(L,dlerror());
 lua_pushstring(L,(lib!=NULL) ? "init" : "open");
 if (lib!=NULL) dlclose(lib);
 return 3;
}

#endif



#ifndef USE_DLL
#ifdef _WIN32
#define USE_DLL	1
#else
#define USE_DLL	0
#endif
#endif


#if USE_DLL
#define LOADLIB

#include <windows.h>

static void pusherror(lua_State *L)
{
 int error=GetLastError();
 char buffer[128];
 if (FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
	0, error, 0, buffer, sizeof(buffer), 0))
  lua_pushstring(L,buffer);
 else
  lua_pushfstring(L,"system error %d\n",error);
}

static int loadlib(lua_State *L)
{
 const char *path=luaL_checkstring(L,1);
 const char *init=luaL_checkstring(L,2);
 HINSTANCE lib=LoadLibrary(path);
 if (lib!=NULL)
 {
  lua_CFunction f=(lua_CFunction) GetProcAddress(lib,init);
  if (f!=NULL)
  {
   lua_pushlightuserdata(L,lib);
   lua_pushcclosure(L,f,1);
   return 1;
  }
 }
 lua_pushnil(L);
 pusherror(L);
 lua_pushstring(L,(lib!=NULL) ? "init" : "open");
 if (lib!=NULL) FreeLibrary(lib);
 return 3;
}

#endif



#ifndef LOADLIB

#ifdef linux
#define LOADLIB
#endif

#ifdef sun
#define LOADLIB
#endif

#ifdef sgi
#define LOADLIB
#endif

#ifdef BSD
#define LOADLIB
#endif

#ifdef _WIN32
#define LOADLIB
#endif

#ifdef LOADLIB
#undef LOADLIB
#define LOADLIB	"`loadlib' not installed (check your Lua configuration)"
#else
#define LOADLIB	"`loadlib' not supported"
#endif

static int loadlib(lua_State *L)
{
 lua_pushnil(L);
 lua_pushliteral(L,LOADLIB);
 lua_pushliteral(L,"absent");
 return 3;
}
#endif

LUALIB_API int luaopen_loadlib (lua_State *L)
{
 lua_register(L,"loadlib",loadlib);
 return 0;
}


#ifndef __INC_ETERLIB_DEBUG_H__
#define __INC_ETERLIB_DEBUG_H__

#include <windows.h>
#include <XORstr.h>

extern void SetLogLevel(UINT uLevel);
extern void Log(UINT uLevel, const char* c_szMsg);
extern void Logn(UINT uLevel, const char* c_szMsg);
extern void Logf(UINT uLevel, const char* c_szFormat, ...);
extern void Lognf(UINT uLevel, const char* c_szFormat, ...);

extern void _Trace(const char* c_szMsg);
extern void _Tracen(const char* c_szMsg);
extern void _Tracenf(const char* c_szFormat, ...);
extern void _Tracef(const char* c_szFormat, ...);
extern void _TraceError(const char* c_szFormat, ...);
extern void _TraceErrorWithoutEnter(const char* c_szFormat, ...);

extern void LogBox(const char* c_szMsg, const char * c_szCaption = NULL, HWND hWnd = NULL);
extern void LogBoxf(const char* c_szMsg, ...);

extern void LogFile(const char* c_szMsg);
extern void LogFilef(const char * c_szMessage, ...);
extern void OpenConsoleWindow(void);
extern void CloseConsoleWindow();
extern void SetupLog(void);

extern void OpenLogFile(bool bUseLogFile = true);
extern void CloseLogFile();

extern HWND g_PopupHwnd;

#define CHECK_RETURN(flag, string)			\
	if (flag)								\
	{										\
		LogBox(string);						\
		return;								\
	}										\

#define XORTrace(szInput)																					\
    _Trace(CXString(sizeof(szInput), []{																		\
        struct  constexpr_string_type { const char * chars = szInput; };								\
        return  NXorStringFuncs::variadic_toolbox::apply_range<sizeof(szInput),							\
            NXorStringFuncs::compile_time::string_builder<constexpr_string_type>::produce>::result{};	\
    }().chars).DecryptString().c_str())

#define XORTracen(szInput)																					\
    _Tracen(CXString(sizeof(szInput), []{																		\
        struct  constexpr_string_type { const char * chars = szInput; };								\
        return  NXorStringFuncs::variadic_toolbox::apply_range<sizeof(szInput),							\
            NXorStringFuncs::compile_time::string_builder<constexpr_string_type>::produce>::result{};	\
    }().chars).DecryptString().c_str())

#define XORTracenf(szInput, ...)																					\
    _Tracenf(CXString(sizeof(szInput), []{																		\
        struct  constexpr_string_type { const char * chars = szInput; };								\
        return  NXorStringFuncs::variadic_toolbox::apply_range<sizeof(szInput),							\
            NXorStringFuncs::compile_time::string_builder<constexpr_string_type>::produce>::result{};	\
    }().chars).DecryptString().c_str(), __VA_ARGS__)

#define XORTracef(szInput, ...)																					\
    _Tracef(CXString(sizeof(szInput), []{																		\
        struct  constexpr_string_type { const char * chars = szInput; };								\
        return  NXorStringFuncs::variadic_toolbox::apply_range<sizeof(szInput),							\
            NXorStringFuncs::compile_time::string_builder<constexpr_string_type>::produce>::result{};	\
    }().chars).DecryptString().c_str(), __VA_ARGS__)

#define XORTraceError(szInput, ...)																					\
    _TraceError(CXString(sizeof(szInput), []{																		\
        struct  constexpr_string_type { const char * chars = szInput; };								\
        return  NXorStringFuncs::variadic_toolbox::apply_range<sizeof(szInput),							\
            NXorStringFuncs::compile_time::string_builder<constexpr_string_type>::produce>::result{};	\
    }().chars).DecryptString().c_str(), __VA_ARGS__)

#define XORTraceErrorWithoutEnter(szInput, ...)																					\
    _TraceErrorWithoutEnter(CXString(sizeof(szInput), []{																		\
        struct  constexpr_string_type { const char * chars = szInput; };								\
        return  NXorStringFuncs::variadic_toolbox::apply_range<sizeof(szInput),							\
            NXorStringFuncs::compile_time::string_builder<constexpr_string_type>::produce>::result{};	\
    }().chars).DecryptString().c_str(), __VA_ARGS__)

#define Trace XORTrace
#define Tracen XORTracen
#define Tracenf XORTracenf
#define Tracef XORTracef
#define TraceError XORTraceError
#define TraceErrorWithoutEnter XORTraceErrorWithoutEnter

#endif

#include "Stdafx.h"
#include "Exception.h"

#if defined HAVE_EXECINFO_H
#include <execinfo.h>
#endif

#ifdef _WIN32
#include <Windows.h>
//#include <DbgHelp.h>
#include <mutex>
#endif

static int32_t capture_stack_trace(int32_t frames_to_skip, int32_t max_frames, void **out_frames)
{
#ifdef _WIN32
	if (max_frames > 32)
		max_frames = 32;

	uint16_t capturedFrames = 0;

	typedef uint16_t(NTAPI FuncRtlCaptureStackBackTrace)(uint32_t, uint32_t, PVOID *, PDWORD);
	HMODULE hmodule = LoadLibrary(TEXT("kernel32.dll"));
	if (hmodule)
	{
		FuncRtlCaptureStackBackTrace *ptrRtlCaptureStackBackTrace = (FuncRtlCaptureStackBackTrace *)GetProcAddress(hmodule, "RtlCaptureStackBackTrace");
		if (ptrRtlCaptureStackBackTrace)
			capturedFrames = ptrRtlCaptureStackBackTrace(frames_to_skip + 1, max_frames, out_frames, nullptr);
		FreeLibrary(hmodule);
	}

	return capturedFrames;

#elif defined HAVE_EXECINFO_H
	memset(out_frames, 0, (sizeof(void *)) * max_frames);

	if (out_hash)
		*out_hash = 0;

	return (backtrace(out_frames, max_frames));
#else
	return 0;
#endif
}

static std::vector<std::string> get_stack_frames_text(void **frames, int32_t num_frames)
{
#ifdef _WIN32
	static std::recursive_mutex mutex;
	std::unique_lock<std::recursive_mutex> mutex_lock(mutex);

	BOOL result = SymInitialize(GetCurrentProcess(), nullptr, true);
	if (!result)
		return std::vector<std::string>();

	std::vector<std::string> backtrace_text;
	for (uint16_t i = 0; i < num_frames; i++)
	{
		uint8_t buffer[sizeof(IMAGEHLP_SYMBOL64) + 128];
		IMAGEHLP_SYMBOL64 *symbol64 = reinterpret_cast<IMAGEHLP_SYMBOL64*>(buffer);
		memset(symbol64, 0, sizeof(IMAGEHLP_SYMBOL64) + 128);
		symbol64->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
		symbol64->MaxNameLength = 128;

		DWORD64 displacement = 0;
		BOOL result = SymGetSymFromAddr64(GetCurrentProcess(), (DWORD64)frames[i], &displacement, symbol64);
		if (result)
		{
			IMAGEHLP_LINE64 line64;
			DWORD displacement = 0;
			memset(&line64, 0, sizeof(IMAGEHLP_LINE64));
			line64.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
			result = SymGetLineFromAddr64(GetCurrentProcess(), (DWORD64)frames[i], &displacement, &line64);
			if (result)
			{
				char szText[512];
				sprintf_s(szText, "%s (%s, line %d)",
					symbol64->Name, line64.FileName, (int32_t)line64.LineNumber);

				backtrace_text.emplace_back(szText);
			}
			else
			{
				backtrace_text.emplace_back(symbol64->Name);
			}
		}
	}

	SymCleanup(GetCurrentProcess());
	return backtrace_text;

#elif defined HAVE_EXECINFO_H

	char **strings;
	strings = backtrace_symbols(frames, num_frames);
	if (!strings)
	{
		return std::vector<std::string>();
	}

	std::vector<std::string> backtrace_text;

	for (int32_t cnt = 0; cnt < num_frames; cnt++)
	{
		char *ptr = strings[cnt];
		char *filename = ptr;
		const char *function = "";

		while (*ptr)
		{
			if (*ptr == '(')
			{
				*(ptr++) = 0;
				function = ptr;
				break;
			}
			ptr++;
		}

		if (function[0])
		{
			while (*ptr)
			{
				if (*ptr == '+')
				{
					*(ptr++) = 0;
					break;
				}
				if (*ptr == ')')
				{
					*(ptr++) = 0;
					break;
				}
				ptr++;
			}
		}

		int32_t status;
		char *new_function = abi::__cxa_demangle(function, nullptr, nullptr, &status);
		if (new_function)
		{
			function = new_function;
		}

		backtrace_text.emplace_back(string_format("%1 (%2)", function, filename));

		if (new_function)
		{
			free(new_function);
		}
	}

	free(strings);
	return backtrace_text;
#else
	return std::vector<std::string>();
#endif
}

Exception::Exception(const std::string &message) : message(message)
{
	num_frames = capture_stack_trace(1, max_frames, frames);
	for (int32_t i = num_frames; i < max_frames; i++)
		frames[i] = nullptr;
}

const char* Exception::what() const throw()
{
	buffer = message;
	return buffer.c_str();
}

std::vector<std::string> Exception::get_stack_trace() const
{
	return get_stack_frames_text(frames, num_frames);
}

std::string Exception::get_message_and_stack_trace() const
{
	std::vector<std::string> stack_trace = get_stack_trace();
	std::string text = message;
	for (auto & elem : stack_trace)
	{
#ifdef _WIN32
		text += "\r\n    at ";
#else
		text += "\n    at ";
#endif

		text += elem.c_str();
	}

	return text;
}

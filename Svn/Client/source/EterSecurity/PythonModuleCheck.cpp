#include "StdAfx.h"
#include "AnticheatManager.h"
#include "CheatQueueManager.h"
#include "NTDefinations.h"
#include <detours.h>
#include <Python/Python.h>
#include <XORstr.h>
#include "../EterPack/EterPackManager.h"

typedef PyObject*(* TPy_InitModule4)(const char *name, PyMethodDef *methods, const char *doc, PyObject *self, int apiver);
static TPy_InitModule4 OPy_InitModule4 = nullptr;

typedef struct _mod *(* TPyParser_ASTFromString)(const char *, const char *, int, PyCompilerFlags *flags,  PyArena *);
static TPyParser_ASTFromString OPyParser_ASTFromString = nullptr;

typedef struct _mod *(* TPyParser_ASTFromFile)(FILE *, const char *, int, char *, char *, PyCompilerFlags *, int *,  PyArena *);
static TPyParser_ASTFromFile OPyParser_ASTFromFile = nullptr;

static std::map <std::string, PyObject*> gs_PythonModules;
static inline bool IsKnownModule(PyObject* pmodule)
{
	for (const auto& it : gs_PythonModules)
	{
		if (it.second == pmodule)
			return true;
	}
	return false;
}

PyObject * Py_InitModule4Detour(const char *name, PyMethodDef *methods, const char *doc, PyObject *self, int apiver)
{

	auto pyObj = OPy_InitModule4(name, methods, doc, self, apiver);
	if (pyObj)
		gs_PythonModules.emplace(name, pyObj);
	return pyObj;
}
struct _mod * PyParser_ASTFromStringDetour(const char * arg1, const char * arg2, int arg3, PyCompilerFlags *flags, PyArena * arena)
{
	return OPyParser_ASTFromString(arg1, arg2, arg3, flags, arena);
}
struct _mod * PyParser_ASTFromFileDetour(FILE * arg1, const char * arg2, int arg3, char * arg4, char * arg5, PyCompilerFlags * arg6, int * arg7,  PyArena * arena)
{

	CAnticheatManager::Instance().ExitByAnticheat(UNALLOWED_PYTHON_API, 0, 0, true, arg2);
	return OPyParser_ASTFromFile(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arena);
}

bool CAnticheatManager::InitializePythonHooks()
{
	OPy_InitModule4 = (TPy_InitModule4)DetourFunction((PBYTE)&Py_InitModule4, (PBYTE)Py_InitModule4Detour);
	if (!OPy_InitModule4)
	{
	}
	
	OPyParser_ASTFromFile = (TPyParser_ASTFromFile)DetourFunction((PBYTE)&PyParser_ASTFromFile, (PBYTE)PyParser_ASTFromFileDetour);
	if (!OPyParser_ASTFromFile)
	{
	}
	return true;
}
void CAnticheatManager::RemovePythonModuleWatcher()
{
	if (OPy_InitModule4)
	{
		if (DetourRemove((PBYTE)OPy_InitModule4, (PBYTE)Py_InitModule4Detour))
		{
			OPy_InitModule4 = nullptr;
		}
	}
}
void CAnticheatManager::DestroyPythonHooks()
{
	if (OPy_InitModule4)
		DetourRemove((PBYTE)OPy_InitModule4, (PBYTE)Py_InitModule4Detour);
	if (OPyParser_ASTFromFile)
		DetourRemove((PBYTE)OPyParser_ASTFromFile, (PBYTE)PyParser_ASTFromFileDetour);
}

inline auto PyTupleToStdVector(PyObject* pyTupleObj)
{
	auto vBuffer = std::vector <std::string>();

	if (PyTuple_Check(pyTupleObj))
	{
		for (Py_ssize_t i = 0; i < PyTuple_Size(pyTupleObj); ++i)
		{
			PyObject *value = PyTuple_GetItem(pyTupleObj, i);
			vBuffer.emplace_back(PyString_AsString(value));
		}
	}
	return vBuffer;
}

void CAnticheatManager::CheckPythonModules()
{
	auto modules = PyImport_GetModuleDict();

	Py_ssize_t pos = 0;
	PyObject *mod_name, *mod;
    while (PyDict_Next(modules, &pos, &mod_name, &mod))
	{
		auto stModuleName = std::string(PyString_AsString(mod_name));

		if (PyObject_HasAttrString(mod, XOR("__file__")))
		{
			auto pyModuleFileName  = PyObject_GetAttrString(mod, XOR("__file__"));
			if (!pyModuleFileName || pyModuleFileName == Py_None)
			{
				ExitByAnticheat(UNKNOWN_PYTHON_MODULE, 0, 0, true, XOR("Unknown module detected: %s"));
				continue;
			}

			auto stCurrModuleFile = std::string(PyString_AsString(pyModuleFileName));
			if (!CEterPackManager::Instance().isExist(stCurrModuleFile.c_str()))
			{
				char szWarningMsg[256];
				sprintf_s(szWarningMsg, XOR("Suspected module file detected: %s"), stCurrModuleFile.c_str());
				ExitByAnticheat(SUSPECTED_PYTHON_FILE, 0, 0, true, szWarningMsg);
			}
		}
		else
		{
			char szWarningMsg[256];
			sprintf_s(szWarningMsg, XOR("Suspected module detected: %s"), stModuleName.c_str());

			if (!IsKnownModule(mod))
			{
				ExitByAnticheat(SUSPECTED_PYTHON_BUILTIN, 0, 0, true, szWarningMsg);
			}
		}
	}
}

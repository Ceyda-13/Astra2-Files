#include "StdAfx.h"
#include "PythonApplication.h"
#include "ProcessScanner.h"
#include "PythonExceptionSender.h"
#include "resource.h"
#include "Version.h"

#ifdef _DEBUG
#include <crtdbg.h>
#endif

#include "../eterPack/EterPackManager.h"
#include "../eterLib/Util.h"
#include "../CWebBrowser/CWebBrowser.h"
#include "../eterBase/CPostIt.h"

#include "CheckLatestFiles.h"

#include "Hackshield.h"
#include "NProtectGameGuard.h"
#include "WiseLogicXTrap.h"
//#include <vld.h>
#ifdef ENABLE_ANTICHEAT
#include "../eterSecurity/AnticheatManager.h"
#include "../eterSecurity/ProtectionMacros.h"
#include "../eterSecurity/CheatQueueManager.h"
#include "../EterSecurity/md5.h"
#pragma comment(lib, "Wtsapi32.lib")
#pragma comment(lib, "Dbghelp.lib")
#endif

extern "C" {
extern int _fltused;
volatile int _AVOID_FLOATING_POINT_LIBRARY_BUG = _fltused;
/*FILE __iob_func[3] = { *stdin, *stdout, *stderr };
_declspec (dllexport) uint32_t NvOptimusEnablement = 0x00000001;
__declspec (dllexport) int32_t AmdPowerXpressRequestHighPerformance = 1;*/
};

//#pragma comment(lib, "kernel32.lib")
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
//#pragma comment(lib, "odbccp32.lib")
//#pragma comment(lib, "Advapi32.lib")

//#pragma comment(lib, "imagehlp.lib")
//#pragma comment(lib, "strmiids.lib")

#ifdef _DEBUG
// #pragma comment(linker, "/NODEFAULTLIB:libcmt.lib")
#pragma comment(linker, "/NODEFAULTLIB:libci.lib")
#pragma comment(lib, "python_d.lib")
#pragma comment(lib, "cryptopp_d.lib")
#pragma comment(lib, "lzo_d.lib")
#pragma comment(lib, "devil_d.lib")
#pragma comment(lib, "mss32_d.lib")
#pragma comment(lib, "tinyxml_d.lib")
#else
#pragma comment(linker, "/NODEFAULTLIB:libci.lib")
#pragma comment(lib, "python.lib")
#pragma comment(lib, "cryptopp.lib")
#pragma comment(lib, "lzo.lib")
#pragma comment(lib, "devil.lib")
#pragma comment(lib, "mss32.lib")
#pragma comment(lib, "tinyxml.lib")
#endif

#pragma comment(lib, "version.lib")
#pragma comment(lib, "granny.lib")
#pragma comment(lib, "d3d8.lib")
#pragma comment(lib, "d3dx8.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "SpeedTreeRT.lib")
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "ws2_32.lib")
#ifdef ENABLE_DISCORD_RPC
#pragma comment(lib, "discord-rpc.lib")
#endif

#include <stdlib.h>
bool __IS_TEST_SERVER_MODE__=false;

extern bool SetDefaultCodePage(DWORD codePage);

#ifdef USE_OPENID
extern int openid_test;
#endif

std::string sc_apszPythonLibraryFilenames[] =
{
	XOR("UserDict.pyc"),
	XOR("__future__.pyc"),
	XOR("copy_reg.pyc"),
	XOR("linecache.pyc"),
	XOR("ntpath.pyc"),
	XOR("os.pyc"),
	XOR("site.pyc"),
	XOR("stat.pyc"),
	XOR("string.pyc"),
	XOR("traceback.pyc"),
	XOR("types.pyc"),
};

#ifdef ENABLE_FIX_D_YMIR_WORK
#include <windows.h>
#include <string>
bool WrongPathYmir(const std::string& dirName_in)
{
    DWORD type = GetFileAttributesA(dirName_in.c_str());

    if (type == INVALID_FILE_ATTRIBUTES)
        return false;

    if (type & FILE_ATTRIBUTE_DIRECTORY)
        return true;

    return false;
}
#endif

char gs_szErrorString[512] = "";

void ApplicationSetErrorString(const char* szErrorString)
{
	strcpy(gs_szErrorString, szErrorString);
}

bool CheckPythonLibraryFilenames()
{
	for (int i = 0; i < sizeof(sc_apszPythonLibraryFilenames) / sizeof(sc_apszPythonLibraryFilenames[0]); ++i)
	{
		std::string stFilename = XOR("lib\\");
		stFilename += sc_apszPythonLibraryFilenames[i].c_str();

		if (_access(stFilename.c_str(), 0) != 0)
			return false;

		// LogBoxf("%s %s", stFilename.c_str(), sc_apszPythonLibraryFilenames[i].c_str());

		MoveFile(stFilename.c_str(), stFilename.c_str());
	}

	return true;
}

struct ApplicationStringTable
{
	HINSTANCE m_hInstance;
	std::map<DWORD, std::string> m_kMap_dwID_stLocale;
} gs_kAppStrTable;

void ApplicationStringTable_Initialize(HINSTANCE hInstance)
{
	gs_kAppStrTable.m_hInstance=hInstance;
}

const std::string& ApplicationStringTable_GetString(DWORD dwID, LPCSTR szKey)
{
	char szBuffer[512];
	char szIniFileName[256];
	char szLocale[256];

	::GetCurrentDirectory(sizeof(szIniFileName), szIniFileName);
	if(szIniFileName[lstrlen(szIniFileName)-1] != '\\')
		strcat(szIniFileName, XOR("\\"));
	strcat(szIniFileName, XOR("metin2client.dat"));

	strcpy(szLocale, LocaleService_GetLocalePath());
	if(strnicmp(szLocale, XOR("locale/"), strlen(XOR("locale/"))) == 0)
		strcpy(szLocale, LocaleService_GetLocalePath() + strlen(XOR("locale/")));
	::GetPrivateProfileString(szLocale, szKey, NULL, szBuffer, sizeof(szBuffer)-1, szIniFileName);
	if(szBuffer[0] == '\0')
		LoadString(gs_kAppStrTable.m_hInstance, dwID, szBuffer, sizeof(szBuffer)-1);
	if(szBuffer[0] == '\0')
		::GetPrivateProfileString(XOR("en"), szKey, NULL, szBuffer, sizeof(szBuffer)-1, szIniFileName);
	if(szBuffer[0] == '\0')
		strcpy(szBuffer, szKey);

	std::string& rstLocale=gs_kAppStrTable.m_kMap_dwID_stLocale[dwID];
	rstLocale=szBuffer;

	return rstLocale;
}

const std::string& ApplicationStringTable_GetString(DWORD dwID)
{
	char szBuffer[512];

	LoadString(gs_kAppStrTable.m_hInstance, dwID, szBuffer, sizeof(szBuffer)-1);
	std::string& rstLocale=gs_kAppStrTable.m_kMap_dwID_stLocale[dwID];
	rstLocale=szBuffer;

	return rstLocale;
}

const char* ApplicationStringTable_GetStringz(DWORD dwID, LPCSTR szKey)
{
	return ApplicationStringTable_GetString(dwID, szKey).c_str();
}

const char* ApplicationStringTable_GetStringz(DWORD dwID)
{
	return ApplicationStringTable_GetString(dwID).c_str();
}

int Setup(LPSTR lpCmdLine);

bool PackInitialize(const char * c_pszFolder)
{
	NANOBEGIN
	if (_access(c_pszFolder, 0) != 0)
		return true;

	std::string stFolder(c_pszFolder);
	stFolder += XOR("/");

	bool bPackFirst = TRUE;

#ifdef NDEBUG // @@correction001 _DISTRIBUTE -> NDEBUG
	Tracef("Note: PackFirst mode enabled. [pack]\n"); // @@correction026

#else
	bPackFirst = FALSE;
	Tracef("Note: PackFirst mode not enabled. [file]\n"); // @@correction026
#endif

	CTextFileLoader::SetCacheMode();
	CEterPackManager::Instance().SetSearchMode(bPackFirst);

	CSoundData::SetPackMode();

	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("root"))).c_str(), XOR(""));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("minimap_update"))).c_str(), XOR(""));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("login_update"))).c_str(), XOR(""));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("model_update"))).c_str(), XOR(""));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("model_update2"))).c_str(), XOR(""));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("model_update3"))).c_str(), XOR(""));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("icon_update"))).c_str(), XOR(""));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("map_update"))).c_str(), XOR(""));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("new_skills"))).c_str(), XOR(""));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("map"))).c_str(), XOR(""));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("awaken_update"))).c_str(), XOR("d:/ymir work/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("bgm"))).c_str(), XOR("bgm/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("locale"))).c_str(), XOR("locale/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("bin"))).c_str(), XOR("d:/ymir work/bin/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("miniboss_update"))).c_str(), XOR("d:/ymir work/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("ramadan19_update"))).c_str(), XOR("d:/ymir work/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("christmas18_update"))).c_str(), XOR("d:/ymir work/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("dance_update"))).c_str(), XOR("d:/ymir work/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("easter19_update"))).c_str(), XOR("d:/ymir work/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("halloween18_update"))).c_str(), XOR("d:/ymir work/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("runes_update"))).c_str(), XOR("d:/ymir work/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("hydra_update"))).c_str(), XOR("d:/ymir work/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("effect"))).c_str(), XOR("d:/ymir work/effect/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("environment"))).c_str(), XOR("d:/ymir work/environment/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("interface"))).c_str(), XOR("d:/ymir work/interface/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("monster"))).c_str(), XOR("d:/ymir work/monster/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("monster2"))).c_str(), XOR("d:/ymir work/monster2/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("npc"))).c_str(), XOR("d:/ymir work/npc/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("npc2"))).c_str(), XOR("d:/ymir work/npc2/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("npc_mount"))).c_str(), XOR("d:/ymir work/npc_mount/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("npc_pet"))).c_str(), XOR("d:/ymir work/npc_pet/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("pc"))).c_str(), XOR("d:/ymir work/pc/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("pc2"))).c_str(), XOR("d:/ymir work/pc2/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("pc3"))).c_str(), XOR("d:/ymir work/pc3/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("item"))).c_str(), XOR("d:/ymir work/item/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("guild"))).c_str(), XOR("d:/ymir work/guild/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("special"))).c_str(), XOR("d:/ymir work/special/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("terrainmaps"))).c_str(), XOR("d:/ymir work/terrainmaps/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("tree"))).c_str(), XOR("d:/ymir work/tree/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("ui"))).c_str(), XOR("d:/ymir work/ui/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("uiloading"))).c_str(), XOR("d:/ymir work/uiloading/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("zone"))).c_str(), XOR("d:/ymir work/zone/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("guild_icon"))).c_str(), XOR("guild_icon/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("icon"))).c_str(), XOR("icon/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("rune_icon"))).c_str(), XOR("icon/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("season1"))).c_str(), XOR("season1/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("season2"))).c_str(), XOR("season2/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("sound"))).c_str(), XOR("sound/"));
	CEterPackManager::Instance().RegisterPack((stFolder + std::string(XOR("textureset"))).c_str(), XOR("textureset/"));
	NANOEND
	return true;
}

bool RunMainScript(CPythonLauncher& pyLauncher, const char* lpCmdLine)
{
	initpack();
	initdbg();
	initime();
	initgrp();
	initgrpImage();
	initgrpText();
	initwndMgr();
	/////////////////////////////////////////////
	initudp();
	initapp();
	initsystemSetting();
	initchr();
	initchrmgr();
	initPlayer();
	initItem();
	initNonPlayer();
	initTrade();
	initChat();
	initTextTail();
	initnet();
	initMiniMap();
	initProfiler();
	initEvent();
	initeffect();
	initfly();
	initsnd();
	initeventmgr();
	initshop();
	initskill();
	initquest();
	initBackground();
	initMessenger();
	initsafebox();
	initguild();
	initServerStateChecker();
#ifdef ENABLE_ACCE_SYSTEM
	initAcce();
#endif
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
	initChangeLook();
#endif
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
	initOfflinePrivateShop();
#endif
#ifdef ENABLE_GROWTH_PET_SYSTEM
	initskillpet();
#endif
#ifdef ENABLE_SWITCHBOT
	initSwitchbot();
#endif
	NANOBEGIN

	// @@correction043
    PyObject * builtins = PyImport_ImportModule(XOR("__builtin__"));
#ifdef NDEBUG // @@correction001 _DISTRIBUTE -> NDEBUG
	PyModule_AddIntConstant(builtins, XOR("__DEBUG__"), 1);
#else
	PyModule_AddIntConstant(builtins, XOR("__DEBUG__"), 0);
#endif

	// RegisterCommandLine
	{
		std::string stRegisterCmdLine;

		const char * loginMark = XOR("-cs");
		const char * loginMark_NonEncode = XOR("-ncs");
		const char * seperator = XOR(" ");

		std::string stCmdLine;
		const int CmdSize = 3;
		vector<std::string> stVec;
		SplitLine(lpCmdLine,seperator,&stVec);
		if (CmdSize == stVec.size() && stVec[0]==loginMark)
		{
			char buf[MAX_PATH];	//TODO ¾Æ·¡ ÇÔ¼ö string ÇüÅÂ·Î ¼öÁ¤
			base64_decode(stVec[2].c_str(),buf);
			stVec[2] = buf;
			string_join(seperator,stVec,&stCmdLine);
		}
		else if (CmdSize <= stVec.size() && stVec[0]==loginMark_NonEncode)
		{
			stVec[0] = loginMark;
			string_join(XOR(" "),stVec,&stCmdLine);
		}
		else
			stCmdLine = lpCmdLine;

		// @@correction043
		PyModule_AddStringConstant(builtins, XOR("__COMMAND_LINE__"), stCmdLine.c_str());
	}
	{
		vector<std::string> stVec;
		SplitLine(lpCmdLine,XOR(" ") ,&stVec);

		if (stVec.size() != 0 && XOR("--pause-before-create-window") == stVec[0])
		{
#ifdef XTRAP_CLIENT_ENABLE
			if (!XTrap_CheckInit())
				return false;
#endif
			system(XOR("pause"));
		}
		__PROTECTOR_END__("RunFile")
		if (!pyLauncher.RunFile(XOR("system.py")))
		{
			TraceError("RunMain Error");
			return false;
		}
		__PROTECTOR_END__("RunFile")
	}

	NANOEND
	return true;
}

bool Main(HINSTANCE hInstance, LPSTR lpCmdLine)
{
#ifdef LOCALE_SERVICE_YMIR
	extern bool g_isScreenShotKey;
	g_isScreenShotKey = true;
#endif

	DWORD dwRandSeed=time(NULL)+DWORD(GetCurrentProcess());
	srandom(dwRandSeed);
	srand(random());

	SetLogLevel(1);

#ifdef LOCALE_SERVICE_VIETNAM_MILD
	extern BOOL USE_VIETNAM_CONVERT_WEAPON_VNUM;
	USE_VIETNAM_CONVERT_WEAPON_VNUM = true;
#endif

	if (_access(XOR("perf_game_update.txt"), 0)==0)
	{
		DeleteFile(XOR("perf_game_update.txt"));
	}

	if (_access(XOR("newpatch.exe"), 0)==0)
	{
		system(XOR("patchupdater.exe"));
		return false;
	}
#ifndef __VTUNE__
	ilInit();
#endif
	if (!Setup(lpCmdLine))
		return false;

#ifdef _DEBUG
	OpenConsoleWindow();
	OpenLogFile(true); // true == uses syserr.txt and log.txt
#else
	OpenLogFile(false); // false == uses syserr.txt only
#endif

	static CLZO				lzo;
	static CEterPackManager	EterPackManager;

	if (!PackInitialize(XOR("pack")))
	{
		LogBox(XOR("Pack Initialization failed. Check log.txt file.."));
		return false;
	}

	if(LocaleService_LoadGlobal(hInstance))
		SetDefaultCodePage(LocaleService_GetCodePage());

	CPythonApplication * app = new CPythonApplication;

	app->Initialize(hInstance);

#ifdef ENABLE_ANTICHEAT
	CCheatDetectQueueMgr::Instance().Initialize();
#endif

	bool ret = false;
	CPythonLauncher pyLauncher;
	CPythonExceptionSender pyExceptionSender;
	SetExceptionSender(&pyExceptionSender);

	if (pyLauncher.Create())
	{
		ret = RunMainScript(pyLauncher, lpCmdLine);
	}

	//ProcessScanner_ReleaseQuitEvent();

	app->Clear();

	timeEndPeriod(1);
	pyLauncher.Clear();

#ifdef ENABLE_ANTICHEAT
	CAnticheatManager::Instance().FinalizeAnticheatRoutines();
#endif

	app->Destroy();
	delete app;

	return ret;
}

HANDLE CreateMetin2GameMutex()
{
	SECURITY_ATTRIBUTES sa;
	ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
	sa.nLength				= sizeof(sa);
	sa.lpSecurityDescriptor	= NULL;
	sa.bInheritHandle		= FALSE;

	return CreateMutex(&sa, FALSE, XOR("Metin2GameMutex"));
}

void DestroyMetin2GameMutex(HANDLE hMutex)
{
	if (hMutex)
	{
		ReleaseMutex(hMutex);
		hMutex = NULL;
	}
}

void __ErrorPythonLibraryIsNotExist()
{
	LogBoxf(XOR("FATAL ERROR!! Python Library file not exist!"));
}

bool __IsTimeStampOption(LPSTR lpCmdLine)
{
	const char* TIMESTAMP = XOR("/timestamp");
	return (strncmp(lpCmdLine, TIMESTAMP, strlen(TIMESTAMP))==0);
}

void __PrintTimeStamp()
{
#ifdef	_DEBUG
	if (__IS_TEST_SERVER_MODE__)
		LogBoxf(XOR("METIN2 BINARY TEST DEBUG VERSION %s  ( MS C++ %d Compiled )"), __TIMESTAMP__, _MSC_VER);
	else
		LogBoxf(XOR("METIN2 BINARY DEBUG VERSION %s ( MS C++ %d Compiled )"), __TIMESTAMP__, _MSC_VER);

#else
	if (__IS_TEST_SERVER_MODE__)
		LogBoxf(XOR("METIN2 BINARY TEST VERSION %s  ( MS C++ %d Compiled )"), __TIMESTAMP__, _MSC_VER);
	else
		LogBoxf(XOR("METIN2 BINARY DISTRIBUTE VERSION %s ( MS C++ %d Compiled )"), __TIMESTAMP__, _MSC_VER);
#endif
}

bool __IsLocaleOption(LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, XOR("--locale")) == 0);
}

bool __IsLocaleVersion(LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, XOR("--perforce-revision")) == 0);
}

#ifdef USE_OPENID
//2012.07.16 ±è¿ë¿í
//ÀÏº» OpenID Áö¿ø. ÀÎÁõÅ° ÀÎÀÚ Ãß°¡
bool __IsOpenIDAuthKeyOption(LPSTR lpCmdLine)
{
	return (strcmp(lpCmdLine, XOR("--openid-authkey")) == 0);
}

bool __IsOpenIDTestOption(LPSTR lpCmdLine) //Å¬¶óÀÌ¾ðÆ®¿¡¼­ ·Î±×ÀÎÀÌ °¡´ÉÇÏ´Ù.
{
	return (strcmp(lpCmdLine, XOR("--openid-test")) == 0);
}
#endif /* USE_OPENID */

typedef struct file
{
	std::string sMD5;
	std::string sFileName;
} files;
files filess[] = {
	// {XOR("e089ce52b0617a6530069f22e0bdba2a"),	XOR("miles/mssa3d.m3d")},
	// {XOR("85267776d45dbf5475c7d9882f08117c"),	XOR("miles/mssds3d.m3d")},
	// {XOR("cb71b1791009eca618e9b1ad4baa4fa9"),	XOR("miles/mssdsp.flt")},
	// {XOR("2727e2671482a55b2f1f16aa88d2780f"),	XOR("miles/mssdx7.m3d")},
	// {XOR("788bd950efe89fa5166292bd6729fa62"),	XOR("miles/msseax.m3d")},
	// {XOR("189576dfe55af3b70db7e3e2312cd0fd"),	XOR("miles/mssmp3.asi")},
	// {XOR("7fae15b559eb91f491a5f75cfa103cd4"),	XOR("miles/mssrsx.m3d")},
	// {XOR("bdc9ad58ade17dbd939522eee447416f"),	XOR("miles/msssoft.m3d")},
	// {XOR("3d5342edebe722748ace78c930f4d8a5"),	XOR("miles/mssvoice.asi")},
	{XOR("6400e224b8b44ece59a992e6d8233719"),	XOR("mss32.dll")},
	{XOR("1ac3d612389fa679f5ca3c6bab855145"),	XOR("speedtreert.dll")},
	// {XOR("ba06046ea15e9c289b85d2094bba4424"),	XOR("lib/encodings/__init__.pyc")},
	// {XOR("1f259cc01d56cefa0fb30d27ac3b71df"),	XOR("lib/encodings/aliases.pyc")},
	// {XOR("38323497aa31ee25570f40ea4240548c"),	XOR("lib/encodings/cp949.pyc")},
	// {XOR("65a610ab6a19eda04e9a8f4ff943c581"),	XOR("lib/xml/dom/__init__.pyc")},
	// {XOR("e9804f9a317e897da5ff498008e0f04a"),	XOR("lib/xml/dom/domreg.pyc")},
	// {XOR("a8b1709b88fbb3e25daa2cdcc353d866"),	XOR("lib/xml/dom/expatbuilder.pyc")},
	// {XOR("94f575010e305c5b93df8936b6075645"),	XOR("lib/xml/dom/minicompat.pyc")},
	// {XOR("cc3048222d0c6ec24e8e3ca54d952d99"),	XOR("lib/xml/dom/minidom.pyc")},
	// {XOR("5ae04ad13ab5ddca4be66a2f31b15b2f"),	XOR("lib/xml/dom/minidomer.pyc")},
	// {XOR("bd999b535cde52b16505a583aa920338"),	XOR("lib/xml/dom/nodefilter.pyc")},
	// {XOR("96cfca27f2b68224ef64fe3b5ec3ddc3"),	XOR("lib/xml/dom/xmlbuilder.pyc")},
	// {XOR("fed6b1b3e745f96c0635c0499095188a"),	XOR("lib/xml/parsers/__init__.pyc")},
	// {XOR("f320705d36f034d44cdf315e04d4b654"),	XOR("lib/xml/parsers/expat.pyc")},
	// {XOR("6e9845c8370d140bdbbec3d0acee86db"),	XOR("lib/xml/parsers/invers.pyc")},
	// {XOR("c51c938e06ad1e6ea30d4807f2ad4b4b"),	XOR("lib/xml/__init__.pyc")},
	// {XOR("086c381bec95c53e252b8bc83da1a2fd"),	XOR("lib/__future__.pyc")},
	// {XOR("84d89786fce180565eef9659b19e6e8a"),	XOR("lib/_abcoll.pyc")},
	// {XOR("0c3c615f4fcc5c741897f48597170fef"),	XOR("lib/_locale.pyc")},
	// {XOR("c22a4ba66c6ee0f03189e0dd4e1e0adc"),	XOR("lib/_weakrefset.pyc")},
	// {XOR("6b844c8bbf493ccb900449573dcbebad"),	XOR("lib/abc.pyc")},
	// {XOR("49e53f51333bcddd44058e6accd74e85"),	XOR("lib/bisect.pyc")},
	// {XOR("b120e3d73fa40961739a1083657d7118"),	XOR("lib/codecs.pyc")},
	// {XOR("71e4d8c688e34b3b8a5d04e126bc0978"),	XOR("lib/collections.pyc")},
	// {XOR("4d5d19c9612301082e7e3f07df69eaf1"),	XOR("lib/copy.pyc")},
	// {XOR("a1e2bf9e1cab8e3c0097c6612848d8b6"),	XOR("lib/copy_reg.pyc")},
	// {XOR("58f6e2128968a2e5267e6dde538f1130"),	XOR("lib/fnmatch.pyc")},
	// {XOR("4b3e1a3b6ba4b30b8d85752b6feb17eb"),	XOR("lib/functools.pyc")},
	// {XOR("1ad801ae299e4633f5aeea4bc98fe96c"),	XOR("lib/genericpath.pyc")},
	// {XOR("d92eec6dda1a5e78bce634a1d987a8dc"),	XOR("lib/heapq.pyc")},
	// {XOR("390ac941c64dc3ce476f7e0c452598a9"),	XOR("lib/keyword.pyc")},
	// {XOR("c4c28529b05b4093f629fb7b23603cd2"),	XOR("lib/linecache.pyc")},
	// {XOR("0c3c615f4fcc5c741897f48597170fef"),	XOR("lib/locale.pyc")},
	// {XOR("11afe39908111e18b46cb2a7ef5dffe3"),	XOR("lib/ntpath.pyc")},
	// {XOR("09fdb0424e03b1a529886aeb2f2f46bb"),	XOR("lib/os.pyc")},
	// {XOR("df558c7a3f12f6d5bc8e08407aef5e9f"),	XOR("lib/pyexpat.pyd")},
	// {XOR("69fa1eb136c558a561e3e3811bfd7d4f"),	XOR("lib/pyexpat_d.pdb")},
	// {XOR("d84def6d367c433643eb1d5cad43b6b5"),	XOR("lib/pyexpat_d.pyd")},
	// {XOR("76c36bc0220c134985b3745b9fd7685b"),	XOR("lib/re.pyc")},
	// {XOR("f0544f6d97d063c870b76ba467e0c4d8"),	XOR("lib/shutil.pyc")},
	// {XOR("766c4241499e7461cad821c1af39062f"),	XOR("lib/site.pyc")},
	// {XOR("1dfe04723ddf06b60946e5ffd2b7b8d2"),	XOR("lib/sre_compile.pyc")},
	// {XOR("fc4d75864cc5c00b0c71e5bf9cefa63e"),	XOR("lib/sre_constants.pyc")},
	// {XOR("19efcf121105fce9ce5c2a0cc8ad7fa1"),	XOR("lib/sre_parse.pyc")},
	// {XOR("dfafd73e6352f3dd589fa0231456f0d6"),	XOR("lib/stat.pyc")},
	// {XOR("09d8d9d8e2e4830a9de0d3a69f500a29"),	XOR("lib/string.pyc")},
	// {XOR("718a1b1bad36c96387351bacb558ddd9"),	XOR("lib/sysconfig.pyc")},
	// {XOR("8751ac5b6e8fd2b754379f7ec7c6c3f5"),	XOR("lib/traceback.pyc")},
	// {XOR("3b4af109b8c51090237731d01feab07b"),	XOR("lib/types.pyc")},
	// {XOR("0f6a0d1b3d35cd97817f426a78759c87"),	XOR("lib/UserDict.pyc")},
	// {XOR("46c809e008eda13d384314128fc2e578"),	XOR("lib/warnings.pyc")},
	// {XOR("728b0da5a4f927a5f88538e486678cd3"),	XOR("lib/weakref.pyc")},
};

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	if (strstr(lpCmdLine, XOR("--hackshield")) != 0)
		return 0;
#ifdef ENABLE_FIX_D_YMIR_WORK
    if (WrongPathYmir(XOR("D:\\ymir work\\")))
	{
		MessageBox(NULL, XOR("Lütfen D:\\ymir work\\ Siliniz !"), XOR("OtoPatcher"), MB_ICONSTOP);
		return 0;
	}
#endif
	// if (strstr(lpCmdLine, XOR("--hgfo-qrdtm")) == 0 && !_access(XOR("jgaagjqwtagjjagjags.txt"), 0) == 0)
	// {
		// MessageBox(NULL, XOR("Lutfen otopack ile giris yapiniz!"), XOR("OtoPatcher"), MB_ICONSTOP);
		// return 0;
	// }

#ifdef ENABLE_ANTICHEAT
	static CAnticheatManager kAnticheatManager;
	kAnticheatManager.InitializePythonHooks();
#endif

	for (auto* it = std::begin(filess); it != std::end(filess); ++it)
		if (strcmp(kAnticheatManager.GetFileMd5(it->sFileName).c_str(), it->sMD5.c_str()))
			return 0;
			// MessageBox(NULL, it->sFileName, it->sMD5, MB_ICONSTOP);

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_CHECK_CRT_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc( 110247 );
#endif

	ApplicationStringTable_Initialize(hInstance);

	LocaleService_LoadConfig(XOR("locale.cfg"));
	SetDefaultCodePage(LocaleService_GetCodePage());

#ifdef XTRAP_CLIENT_ENABLE
	if (!XTrap_Init())
		return 0;
#endif

#ifdef USE_AHNLAB_HACKSHIELD
	if (!HackShield_Init())
		return 0;
#endif

#ifdef USE_NPROTECT_GAMEGUARD
	if (!GameGuard_Init())
		return 0;
#endif

#if defined(CHECK_LATEST_DATA_FILES)
	if (!CheckLatestFiles())
		return 0;
#endif

	bool bQuit = false;
	bool bAuthKeyChecked = false;	//OpenID ¹öÀü¿¡¼­ ÀÎÁõÅ°°¡ µé¾î¿Ô´ÂÁö ¾Ë±â À§ÇÑ ÀÎÀÚ.
	int nArgc = 0;
	PCHAR* szArgv = CommandLineToArgv( lpCmdLine, &nArgc );

	for( int i=0; i < nArgc; i++ ) {
		if(szArgv[i] == 0)
			continue;
		if (__IsLocaleVersion(szArgv[i])) // #0000829: [M2EU] ¹öÀü ÆÄÀÏÀÌ Ç×»ó »ý±âÁö ¾Êµµ·Ï ¼öÁ¤
		{
			char szModuleName[MAX_PATH];
			char szVersionPath[MAX_PATH];
			GetModuleFileName(NULL, szModuleName, sizeof(szModuleName));
			sprintf(szVersionPath, XOR("%s.version"), szModuleName);
			FILE* fp = fopen(szVersionPath, XOR("wt"));
			if (fp)
			{
				extern int METIN2_GET_VERSION();
				fprintf(fp, XOR("r%d\n"), METIN2_GET_VERSION());
				fclose(fp);
			}
			bQuit = true;
		} else if (__IsLocaleOption(szArgv[i]))
		{
			FILE* fp=fopen(XOR("locale.txt"), XOR("wt"));
			fprintf(fp, XOR("service[%s] code_page[%d]"),
				LocaleService_GetName(), LocaleService_GetCodePage());
			fclose(fp);
			bQuit = true;
		} else if (__IsTimeStampOption(szArgv[i]))
		{
			__PrintTimeStamp();
			bQuit = true;
		} else if ((strcmp(szArgv[i], XOR("--force-set-locale")) == 0))
		{
			// locale ¼³Á¤¿£ ÀÎÀÚ°¡ µÎ °³ ´õ ÇÊ¿äÇÔ (·ÎÄÉÀÏ ¸íÄª, µ¥ÀÌÅÍ °æ·Î)
			if (nArgc <= i + 2)
			{
				MessageBox(NULL, XOR("Invalid arguments"), ApplicationStringTable_GetStringz(IDS_APP_NAME, XOR("APP_NAME")), MB_ICONSTOP);
				goto Clean;
			}

			const char* localeName = szArgv[++i];
			const char* localePath = szArgv[++i];

			LocaleService_ForceSetLocale(localeName, localePath);
		}
#ifdef USE_OPENID
		else if (__IsOpenIDAuthKeyOption(szArgv[i]))	//2012.07.16 OpenID : ±è¿ë¿í
		{
			// ÀÎÁõÅ° ¼³Á¤¿£ ÀÎÀÚ°¡ ÇÑ °³ ´õ ÇÊ¿äÇÔ (ÀÎÁõÅ°)
			if (nArgc <= i + 1)
			{
				MessageBox(NULL, XOR("Invalid arguments"), ApplicationStringTable_GetStringz(IDS_APP_NAME, XOR("APP_NAME")), MB_ICONSTOP);
				goto Clean;
			}

			const char* authKey = szArgv[++i];

			//ongoing (2012.07.16)
			//ÀÎÁõÅ° ÀúÀåÇÏ´Â ºÎºÐ
			LocaleService_SetOpenIDAuthKey(authKey);

			bAuthKeyChecked = true;
		}
		else if (__IsOpenIDTestOption(szArgv[i]))
		{
			openid_test = 1;

		}
#endif /* USE_OPENID */
	}

#ifdef USE_OPENID
	//OpenID
	//OpenID Å¬¶óÀÌ¾ðÆ®ÀÇ °æ¿ìÀÎÁõÅ°¸¦ ¹Þ¾Æ¿ÀÁö ¾ÊÀ» °æ¿ì (À¥À» Á¦¿ÜÇÏ°í ½ÇÇà ½Ã) Å¬¶óÀÌ¾ðÆ® Á¾·á.

	if (false == bAuthKeyChecked && !openid_test)
	{
		MessageBox(NULL, XOR("Invalid execution"), ApplicationStringTable_GetStringz(IDS_APP_NAME, XOR("APP_NAME")), MB_ICONSTOP);
		goto Clean;
	}
#endif /* USE_OPENID */


	if(bQuit)
		goto Clean;

#if defined(NEEDED_COMMAND_ARGUMENT)
	// ¿É¼ÇÀÌ ¾øÀ¸¸é ºñÁ¤»ó ½ÇÇàÀ¸·Î °£ÁÖ, ÇÁ·Î±×·¥ Á¾·á
	if (strstr(lpCmdLine, NEEDED_COMMAND_ARGUMENT) == 0) {
		MessageBox(NULL, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, XOR("ERR_MUST_LAUNCH_FROM_PATCHER")), ApplicationStringTable_GetStringz(IDS_APP_NAME, XOR("APP_NAME")), MB_ICONSTOP);
			goto Clean;
	}
#endif

#if defined(NEEDED_COMMAND_CLIPBOARD)
	{
		CHAR szSecKey[256];
		CPostIt cPostIt( XOR("VOLUME1") );

		if( cPostIt.Get( XOR("SEC_KEY"), szSecKey, sizeof(szSecKey) ) == FALSE ) {
			MessageBox(NULL, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, XOR("ERR_MUST_LAUNCH_FROM_PATCHER")), ApplicationStringTable_GetStringz(IDS_APP_NAME, XOR("APP_NAME")), MB_ICONSTOP);
			goto Clean;
		}
		if( strstr(szSecKey, NEEDED_COMMAND_CLIPBOARD) == 0 ) {
			MessageBox(NULL, ApplicationStringTable_GetStringz(IDS_ERR_MUST_LAUNCH_FROM_PATCHER, XOR("ERR_MUST_LAUNCH_FROM_PATCHER")), ApplicationStringTable_GetStringz(IDS_APP_NAME, XOR("APP_NAME")), MB_ICONSTOP);
			goto Clean;
		}
		cPostIt.Empty();
	}
#endif

	WebBrowser_Startup(hInstance);

	Main(hInstance, lpCmdLine);

	if (!CheckPythonLibraryFilenames())
	{
		__ErrorPythonLibraryIsNotExist();
		goto Clean;
	}


#ifdef USE_NPROTECT_GAMEGUARD
	GameGuard_NoticeMessage();
#endif

	WebBrowser_Cleanup();

	::CoUninitialize();

	if(gs_szErrorString[0])
		MessageBox(NULL, gs_szErrorString, ApplicationStringTable_GetStringz(IDS_APP_NAME, XOR("APP_NAME")), MB_ICONSTOP);

Clean:
#ifdef USE_AHNLAB_HACKSHIELD
	HackShield_Shutdown();
#endif
	SAFE_FREE_GLOBAL(szArgv);

	return 0;
}

// @@correction020
static void GrannyError(granny_log_message_type Type,
						granny_log_message_origin Origin,
						char const*  File,
						granny_int32x Line,
						char const *Error,
						void *UserData)
{
	if (Origin == GrannyFileReadingLogMessage || Origin==GrannyControlLogMessage || Origin == GrannyMeshBindingLogMessage)
		return;

	TraceError("GRANNY: %s(%d): ERROR: %s --- [%d] %s --- [%d] %s", File, Line, Error, Type, GrannyGetLogMessageTypeString(Type), Origin, GrannyGetLogMessageOriginString(Origin));
}

int Setup(LPSTR lpCmdLine)
{
	/*
	 *	Å¸ÀÌ¸Ó Á¤¹Ðµµ¸¦ ¿Ã¸°´Ù.
	 */
	TIMECAPS tc;
	UINT wTimerRes;

	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
		return 0;

	wTimerRes = MINMAX(tc.wPeriodMin, 1, tc.wPeriodMax);
	timeBeginPeriod(wTimerRes);

	granny_log_callback Callback;
    Callback.Function = GrannyError;
    Callback.UserData = 0;
    GrannySetLogCallback(&Callback);
	return 1;
}

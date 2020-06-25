#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>

#if defined(_WIN32) || defined(WIN32) || defined(_WIN64) || defined(WIN64)
#include <io.h>
#include <windows.h>
#include <tchar.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#ifdef ENABLE_XTRAP_SYSTEM
#include <XTrap_S_Interface.h>
#endif

#include "char.h"
#include "config.h"
#include "event.h"
#include "log.h"
#include "desc.h"
#include "packet.h"
#include "XTrapManager.h"

#define CSFILE_NUM				2
#define XTRAP_CS1_CHECK_CYCLE	PASSES_PER_SEC(20)

#ifdef ENABLE_XTRAP_SYSTEM
uint8_t g_XTrap_ClientMap[CSFILE_NUM][XTRAP_CS4_BUFSIZE_MAP];


struct CXTrapManager::sXTrapContext
{
	PFN_XTrap_S_Start			XTrap_S_Start;
	PFN_XTrap_S_SessionInit		XTrap_S_SessionInit;
	PFN_XTrap_CS_Step1			XTrap_CS_Step1;
	PFN_XTrap_CS_Step3			XTrap_CS_Step3;

	PFN_XTrap_S_SetActiveCode	XTrap_S_SetActiveCode;
	PFN_XTrap_S_SetOption		XTrap_S_SetOption;
	PFN_XTrap_S_SetAllowDelay	XTrap_S_SetAllowDelay;
	PFN_XTrap_S_SendGamePacket	XTrap_S_SendGamePacket;
	PFN_XTrap_S_RecvGamePacket	XTrap_S_RecvGamePacket;

	void*		  hXTrap4Server;
};
#endif

CXTrapManager::CXTrapManager()
{
#ifdef ENABLE_XTRAP_SYSTEM
	m_pImpl = M2_NEW sXTrapContext;
	memset( m_pImpl, 0x00, sizeof(sXTrapContext) );
#endif
}

CXTrapManager::~CXTrapManager()
{
#ifdef ENABLE_XTRAP_SYSTEM
#ifdef __FreeBSD__
	if (m_pImpl->hXTrap4Server)
	{
		dlclose(m_pImpl->hXTrap4Server);
	}
#endif

	M2_DELETE(m_pImpl);
#endif
}


bool CXTrapManager::LoadXTrapModule()
{
#ifdef ENABLE_XTRAP_SYSTEM
#ifdef __FreeBSD__
	bool bClientMapFileLoaded = false;
	for(int32_t i=0; i<CSFILE_NUM; ++i )
	{
		if( LoadClientMapFile(i) )
		{
			bClientMapFileLoaded = true;
		}
	}

	if( !bClientMapFileLoaded )
	{
		sys_err("XTrap-failed to load at least one client map file. map file name should be map1.CS3 or map2.CS3");
		return false;
	}

	char sDllBinFile[]	="./libXTrap4Server.so";

	m_pImpl->hXTrap4Server = dlopen(sDllBinFile, RTLD_LAZY);

	if (m_pImpl->hXTrap4Server == 0)
	{
		sys_err("XTrap-failed to load so reason:%s", dlerror()) ;
		return false;
	}

	void* hXTrapHandle = m_pImpl->hXTrap4Server;

	m_pImpl->XTrap_S_Start		    = (PFN_XTrap_S_Start)			dlsym(hXTrapHandle, "XTrap_S_Start");
	m_pImpl->XTrap_S_SessionInit    = (PFN_XTrap_S_SessionInit)		dlsym(hXTrapHandle, "XTrap_S_SessionInit");
	m_pImpl->XTrap_CS_Step1		    = (PFN_XTrap_CS_Step1)			dlsym(hXTrapHandle, "XTrap_CS_Step1");
	m_pImpl->XTrap_CS_Step3		    = (PFN_XTrap_CS_Step3)			dlsym(hXTrapHandle, "XTrap_CS_Step3");
	m_pImpl->XTrap_S_SetActiveCode  = (PFN_XTrap_S_SetActiveCode)	dlsym(hXTrapHandle, "XTrap_S_SetActiveCode");
	m_pImpl->XTrap_S_SetOption	    = (PFN_XTrap_S_SetOption)		dlsym(hXTrapHandle, "XTrap_S_SetOption");
	m_pImpl->XTrap_S_SetAllowDelay  = (PFN_XTrap_S_SetAllowDelay)	dlsym(hXTrapHandle, "XTrap_S_SetAllowDelay");
	m_pImpl->XTrap_S_SendGamePacket = (PFN_XTrap_S_SendGamePacket)	dlsym(hXTrapHandle, "XTrap_S_SendGamePacket");
	m_pImpl->XTrap_S_RecvGamePacket = (PFN_XTrap_S_RecvGamePacket)	dlsym(hXTrapHandle, "XTrap_S_RecvGamePacket");

	if (m_pImpl->XTrap_S_Start			== nullptr ||
		m_pImpl->XTrap_S_SessionInit	== nullptr ||
		m_pImpl->XTrap_CS_Step1			== nullptr ||
		m_pImpl->XTrap_CS_Step3			== nullptr ||
		m_pImpl->XTrap_S_SetOption		== nullptr ||
		m_pImpl->XTrap_S_SetAllowDelay	== nullptr ||
		m_pImpl->XTrap_S_SendGamePacket	== nullptr	||
		m_pImpl->XTrap_S_RecvGamePacket	== nullptr)
	{
		sys_err("XTrap-failed to load function ptrs");
		return	false;
	}

	m_pImpl->XTrap_S_Start( 600, CSFILE_NUM, g_XTrap_ClientMap, nullptr );

	m_pImpl->XTrap_S_SetActiveCode( XTRAP_ACTIVE_CODE_THEMIDA );

	signal(SIGUSR2, CXTrapManager::MapReloadSignalHandler);

#endif
#endif
	return true;
}

bool CXTrapManager::LoadClientMapFile( uint32_t iMapIndex )
{
#ifdef ENABLE_XTRAP_SYSTEM
#ifdef __FreeBSD__
	if( iMapIndex >= CSFILE_NUM )
	{
		return false;
	}

	char szFileName[1024] = {0,};
	snprintf(szFileName, sizeof(szFileName), "map%d.CS3", iMapIndex+1);

	FILE* fi = 0;
	fi = fopen(szFileName, "rb");
	if (fi == nullptr)
	{
		return false;
	}

	fread(g_XTrap_ClientMap[iMapIndex], XTRAP_CS4_BUFSIZE_MAP, 1, fi);
	fclose(fi);
#endif
#endif
	return true;
}

EVENTINFO(xtrap_cs1_check_info)
{
	DynamicCharacterPtr ptrPC;
};

EVENTFUNC(xtrap_cs1_check_event)
{
#ifdef ENABLE_XTRAP_SYSTEM
	xtrap_cs1_check_info* info = dynamic_cast<xtrap_cs1_check_info*>( event->info );

	if ( info == nullptr )
	{
		sys_err( "<xtrap_event> info null pointer" );
		return 0;
	}

	TPacketXTrapCSVerify pack;
	pack.bHeader = HEADER_GC_XTRAP_CS1_REQUEST;

	bool bSuccess = CXTrapManager::instance().Verify_CSStep1( info->ptrPC, pack.bPacketData );

	LPDESC lpClientDesc = info->ptrPC.Get()->GetDesc();
	if( !lpClientDesc )
	{
		sys_err( "<xtrap_event> client session is invalid" );
		return 0;
	}

	lpClientDesc->Packet( &pack, sizeof(pack) );

	if( bSuccess )
	{
		return XTRAP_CS1_CHECK_CYCLE;
	}

	sys_err( "XTrap: hack is detected %s", lpClientDesc->GetHostName() );

	info->ptrPC.Get()->Disconnect("XTrapCheckInvalid");
	lpClientDesc->SetPhase(PHASE_CLOSE);

	return 0;
#else
	return XTRAP_CS1_CHECK_CYCLE;
#endif
}

bool CXTrapManager::CreateClientSession( LPCHARACTER lpCharSession )
{
#ifdef ENABLE_XTRAP_SYSTEM
	if( !bXTrapEnabled )
		return true;

	if( !lpCharSession )
		return false;

	uint32_t dwSessionID = lpCharSession->GetPlayerID();

	ClientSessionMap::iterator it = m_mapClientSessions.find( dwSessionID );
	if( it != m_mapClientSessions.end() )
	{
		sys_err("XTrap: client session is already registered");
		return false;
	}

	sSessionInfo			infoData;

	uint32_t dwReturn = m_pImpl->XTrap_S_SessionInit( 600, CSFILE_NUM, g_XTrap_ClientMap, infoData.szSessionBuf );
	if( dwReturn != 0 )
	{
		sys_err("XTrap: client session init failed");
	}

	xtrap_cs1_check_info*  event_info = AllocEventInfo<xtrap_cs1_check_info>();
	event_info->ptrPC = lpCharSession;

	infoData.m_pCheckEvent = event_create(xtrap_cs1_check_event, event_info, XTRAP_CS1_CHECK_CYCLE);
	m_mapClientSessions[dwSessionID] = infoData;
#endif
	return true;
}

void CXTrapManager::DestroyClientSession( LPCHARACTER lpCharSession )
{
#ifdef ENABLE_XTRAP_SYSTEM
	if( !bXTrapEnabled )
		return;

	if( !lpCharSession )
		return;

	uint32_t dwSessionID = lpCharSession->GetPlayerID();

	ClientSessionMap::iterator it = m_mapClientSessions.find( dwSessionID );
	if( it == m_mapClientSessions.end() )
	{
		sys_err("XTrap: client session is already destroyed");
		return;
	}

	event_cancel(&(it->second.m_pCheckEvent) );
	m_mapClientSessions.erase(it);
#endif
}


bool CXTrapManager::Verify_CSStep1( LPCHARACTER lpCharSession, uint8_t* pBufData )
{
#ifdef ENABLE_XTRAP_SYSTEM
	if( !bXTrapEnabled )
		return false;

	if( !lpCharSession )
		return false;

	uint32_t dwSessionID = lpCharSession->GetPlayerID();

	ClientSessionMap::iterator it = m_mapClientSessions.find( dwSessionID );
	if( it == m_mapClientSessions.end() )
	{
		sys_err("XTrap: client session is already destroyed");
		return false;
	}

	int32_t nReturn = m_pImpl->XTrap_CS_Step1( it->second.szSessionBuf, it->second.szPackBuf );

	memcpy( pBufData, it->second.szPackBuf, VERIFY_PACK_LEN );

	return (nReturn == 0) ? true : false;
#else
	return true;
#endif
}

void CXTrapManager::Verify_CSStep3( LPCHARACTER lpCharSession, uint8_t* pBufData )
{
#ifdef ENABLE_XTRAP_SYSTEM
	if( !bXTrapEnabled )
		return;

	if( !lpCharSession )
		return;

	uint32_t dwSessionID = lpCharSession->GetPlayerID();

	ClientSessionMap::iterator it = m_mapClientSessions.find( dwSessionID );
	if( it == m_mapClientSessions.end() )
	{
		sys_log(0, "XTrap: client session is alreay destroyed");
		return;
	}

	memcpy( it->second.szPackBuf, pBufData, VERIFY_PACK_LEN );
	m_pImpl->XTrap_CS_Step3( it->second.szSessionBuf, it->second.szPackBuf );
#endif
}


#include "StdAfx.h"

#include <io.h>
#include <assert.h>

#include "EterPackManager.h"

#include "../EterBase/Debug.h"
#include "../EterBase/CRC32.h"

/*#ifdef ENABLE_ANTICHEAT
#include "../AntiCheatLib/CheatQueueManager.h"
#include <ulib/crypt/cstr.h>
#endif*/

void CEterPackManager::ConvertFileName(const char * c_szFileName, std::string & rstrFileName)
{
	rstrFileName = c_szFileName;
	stl_lowers(rstrFileName);

	for (DWORD i = 0; i < rstrFileName.length(); ++i)
		if (rstrFileName[i] == '\\')
			rstrFileName[i] = '/';
}

bool CEterPackManager::Get(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
{
	if (m_iSearchMode == SEARCH_FILE_FIRST)
	{
		if (GetFromFile(rMappedFile, c_szFileName, pData))
			return true;

		return GetFromPack(rMappedFile, c_szFileName, pData);
	}

	if (GetFromPack(rMappedFile, c_szFileName, pData))
		return true;

	return GetFromFile(rMappedFile, c_szFileName, pData);
}

bool CEterPackManager::GetFromPack(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
{
	FinderLock lock(m_csFinder);

	static std::string strFileName(c_szFileName);

	ConvertFileName(c_szFileName, strFileName);
	for (auto& itor: m_PackMap)
		if (itor.second->Get(rMappedFile, strFileName.c_str(), pData))
			return true;

#ifdef _DEBUG
	TraceError("CANNOT_FIND_PACK_FILE [%s]", strFileName.c_str());
#endif

	return false;
}

bool CEterPackManager::GetFromFile(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData)
{
	return rMappedFile.Create(c_szFileName, pData, 0, 0) ? true : false;
}

bool CEterPackManager::isExistInPack(const char * c_szFileName)
{
	std::string strFileName;
	ConvertFileName(c_szFileName, strFileName);
	for (auto& itor: m_PackMap)
		if (itor.second->IsExist(strFileName.c_str()))
			return true;

	return false;
}

bool CEterPackManager::isExist(const char * c_szFileName)
{
	if (m_iSearchMode == SEARCH_PACK_FIRST)
	{
		if (isExistInPack(c_szFileName))
			return true;

		return _access(c_szFileName, 0) == 0 ? true : false;
	}

	if (_access(c_szFileName, 0) == 0)
		return true;

	return isExistInPack(c_szFileName);
}

bool CEterPackManager::RegisterPack(const char * c_szName, const char * c_szDirectory)
{
	CEterPack * pEterPack = NULL;
	{
		auto itor = m_PackMap.find(c_szName);

		if (m_PackMap.end() == itor)
		{
			pEterPack = new CEterPack;
			if (pEterPack->Create(c_szName, c_szDirectory))
				m_PackMap.insert(TEterPackMap::value_type(c_szName, pEterPack));
			else
			{
#ifdef _DEBUG
				Tracef("The eterpack doesn't exist [%s]\n", c_szName);
#endif
				delete pEterPack;
				pEterPack = NULL;
				return false;
			}
		}
		else
			pEterPack = itor->second;
	}

	return true;
}

void CEterPackManager::SetSearchMode(bool bPackFirst)
{
	m_iSearchMode = bPackFirst ? SEARCH_PACK_FIRST : SEARCH_FILE_FIRST;
}

int CEterPackManager::GetSearchMode()
{
	return m_iSearchMode;
}

CEterPackManager::CEterPackManager() : m_iSearchMode(SEARCH_FILE_FIRST)
{
	InitializeCriticalSection(&m_csFinder);
}

CEterPackManager::~CEterPackManager()
{
	for (auto& i : m_PackMap)
		delete i.second;

	DeleteCriticalSection(&m_csFinder);
}


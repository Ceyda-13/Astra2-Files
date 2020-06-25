#pragma once

#include <windows.h>
#include <unordered_map>
#include "../EterBase/Singleton.h"
#include "../EterBase/Stl.h"
#include "../UserInterface/Locale_inc.h"

#include "EterPack.h"

class CEterPackManager : public CSingleton<CEterPackManager>
{
	public:
		struct FinderLock
		{
			FinderLock(CRITICAL_SECTION& cs) : p_cs(&cs)
			{
				EnterCriticalSection(p_cs);
			}

			~FinderLock()
			{
				LeaveCriticalSection(p_cs);
			}

			CRITICAL_SECTION* p_cs;
		};

		enum ESearchModes
		{
			SEARCH_FILE_FIRST,
			SEARCH_PACK_FIRST
		};

		typedef std::unordered_map<std::string, CEterPack*, stringhash> TEterPackMap;

	public:
		CEterPackManager();
		virtual ~CEterPackManager();

		void SetSearchMode(bool bPackFirst);
		int GetSearchMode();
		bool Get(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData);
		bool GetFromPack(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData);
		bool GetFromFile(CMappedFile & rMappedFile, const char * c_szFileName, LPCVOID * pData);
		bool isExist(const char * c_szFileName);
		bool isExistInPack(const char * c_szFileName);

		bool RegisterPack(const char * c_szName, const char * c_szDirectory);

	protected:
		void ConvertFileName(const char * c_szFileName, std::string & rstrFileName); 

	protected:
		int						m_iSearchMode;
		TEterPackMap			m_PackMap;
		CRITICAL_SECTION		m_csFinder;
};


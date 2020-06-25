#pragma once

#include <string>

class CProperty
{
	public:
		CProperty(const char * c_pszFileName);
		~CProperty();

		void			Clear();

		const char *	GetFileName();

		bool			GetVector(const char * c_pszKey, CTokenVector & rTokenVector);
		bool			GetString(const char * c_pszKey, const char ** c_ppString);

		void			PutVector(const char * c_pszKey, const CTokenVector & c_rTokenVector);
		void			PutString(const char * c_pszKey, const char * c_pszString);

		DWORD			GetSize();
		unsigned long long			GetCRC();

		bool			ReadFromXML(const char* c_pszCRC);

	protected:
		std::string							m_stFileName;
		std::string							m_stCRC;
		const char *						mc_pFileName;
		unsigned long long					m_dwCRC;

		CTokenVectorMap						m_stTokenMap;
};

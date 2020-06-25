#include "StdAfx.h"
#include <cstring>
#include "../EterBase/TempFile.h"

#include "PropertyManager.h"
#include "Property.h"

CProperty::CProperty(const char * c_pszFileName) : mc_pFileName(nullptr), m_dwCRC(0)
{
	m_stFileName = c_pszFileName;
	StringPath(m_stFileName);

	mc_pFileName = strrchr(m_stFileName.c_str(), '/');

	if (!mc_pFileName)
		mc_pFileName = m_stFileName.c_str();
	else
		++mc_pFileName;
}

CProperty::~CProperty() = default;

unsigned long long CProperty::GetCRC()
{
	return m_dwCRC;
}

const char * CProperty::GetFileName()
{
	return (m_stFileName.c_str());
}

bool CProperty::GetString(const char * c_pszKey, const char ** c_ppString)
{
	std::string stTempKey = c_pszKey;
	stl_lowers(stTempKey);
	CTokenVectorMap::iterator it = m_stTokenMap.find(stTempKey.c_str());
	if (m_stTokenMap.end() == it)
		return false;

	*c_ppString = it->second[0].c_str();
	return true;
}

DWORD CProperty::GetSize()
{
	return m_stTokenMap.size();
}

bool CProperty::GetVector(const char * c_pszKey, CTokenVector & rTokenVector)
{
	std::string stTempKey = c_pszKey;
	stl_lowers(stTempKey);
	CTokenVectorMap::iterator it = m_stTokenMap.find(stTempKey.c_str());

	if (m_stTokenMap.end() == it)
		return false;

	 for (auto& itor : it->second)
		 rTokenVector.push_back(itor);

	return true;
}

void CProperty::PutString(const char * c_pszKey, const char * c_pszString)
{
	std::string stTempKey = c_pszKey;
	stl_lowers(stTempKey);
	CTokenVectorMap::iterator itor = m_stTokenMap.find(stTempKey);

	if (itor != m_stTokenMap.end())
		m_stTokenMap.erase(itor);

	CTokenVector tokenVector;
	tokenVector.push_back(c_pszString);

	m_stTokenMap.insert(CTokenVectorMap::value_type(stTempKey, tokenVector));
}

void CProperty::PutVector(const char * c_pszKey, const CTokenVector & c_rTokenVector)
{
	std::string stTempKey = c_pszKey;
	stl_lowers(stTempKey);

	m_stTokenMap.insert(CTokenVectorMap::value_type(stTempKey, c_rTokenVector));
}

void CProperty::Clear()
{
	m_stTokenMap.clear();
}

bool CProperty::ReadFromXML(const char* c_pszCRC)
{
	m_dwCRC = std::stoull(c_pszCRC);
	return true;
}


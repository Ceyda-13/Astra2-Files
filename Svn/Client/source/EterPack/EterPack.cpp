#include "StdAfx.h"

#include <windows.h>
#include <mmsystem.h>
#include <io.h>
#include <assert.h>

#include "EterPack.h"
#include "Inline.h"

#include "../EterBase/utils.h"
#include "../EterBase/Debug.h"
#include "../EterBase/CRC32.h"

#include <iostream>
#include <fstream>

#define ENABLE_CRC32_CHECK

CEterPack::CEterPack() : m_indexCount(0), m_indexData(NULL)
{

}

CEterPack::~CEterPack()
{
	Destroy();
}

void CEterPack::Destroy()
{
	m_indexCount = 0;
	m_DataPositionMap.clear();

	SAFE_DELETE_ARRAY(m_indexData);

	memset(m_dbName, 0, sizeof(m_dbName));
	memset(m_indexFileName, 0, sizeof(m_indexFileName));
}

const std::string& CEterPack::GetPathName()
{
	return m_stPathName;
}

bool CEterPack::Create(const char * dbname, const char* pathName)
{
	m_stPathName = pathName;

	strncpy(m_dbName, dbname, DBNAME_MAX_LEN);

	strncpy(m_indexFileName, dbname, MAX_PATH);
	strcat(m_indexFileName, ".oei");

	m_stDataFileName = dbname;
	m_stDataFileName += ".oep";

	if (!CreateIndexFile())
		return false;

	if (!CreateDataFile())
		return false;

	__BuildIndex();

	return true;
}

static DWORD s_adwEterPackKey[] =
{
	45129401,
	92367215,
	681285731,
	1710201,
};

static DWORD s_adwEterPackSecurityKey[] =
{
	78952482,
	527348324,
	1632942,
	486274726,
};

bool CEterPack::__BuildIndex()
{

	CMappedFile file;
	LPCVOID pvData;
	CLZObject zObj;

	if (!file.Create(m_indexFileName, &pvData, 0, 0))
	{
		TraceError("Cannot open pack index file! %s", m_dbName);
		return false;
	}

	if (file.Size() < eterpack::c_HeaderSize)
	{
		TraceError("Pack index file header error! %s", m_dbName);
		return false;
	}

	DWORD fourcc = *(DWORD *) pvData;

	BYTE * pbData;
	UINT uiFileSize;

	if (fourcc == eterpack::c_IndexCC)
	{
		pbData = (BYTE *) pvData;
		uiFileSize = file.Size();
	}
	else if (fourcc == CLZObject::ms_dwFourCC)
	{
		if (!CLZO::Instance().Decompress(zObj, (const BYTE *) pvData, s_adwEterPackKey))
			return false;

		if (zObj.GetSize() < eterpack::c_HeaderSize)
			return false;

		pbData = zObj.GetBuffer();
		uiFileSize = zObj.GetSize();
	}
	else
	{
		TraceError("Pack index file fourcc error! %s", m_dbName);
		return false;
	}

	pbData += sizeof(DWORD);

	DWORD ver = *(DWORD *) pbData;
	pbData += sizeof(DWORD);

	if (ver != eterpack::c_Version)
	{
		TraceError("Pack index file version error! %s", m_dbName);
		return false;
	}

	m_indexCount = *(long *) pbData;
	pbData += sizeof(long);
	// TraceError("Name %s, indexCount %d", m_dbName, m_indexCount);

	if (uiFileSize < eterpack::c_HeaderSize + sizeof(TEterPackIndex) * m_indexCount)
	{
		TraceError("Pack index file size error! %s, indexCount %d", m_dbName, m_indexCount);
		return false;
	}

	m_indexData = new TEterPackIndex[m_indexCount];
	memcpy(m_indexData, pbData, sizeof(TEterPackIndex) * m_indexCount);

	TEterPackIndex * index = m_indexData;

	for (int i = 0; i < m_indexCount; ++i, ++index)
	{
		if (strcmp(m_stPathName.c_str(), "*"))
		{
			std::string s(m_stPathName);
			s += index->filename;
			strncpy(index->filename, s.c_str(), sizeof(index->filename));
			index->filename_crc = GetCRC32(s.c_str(), s.length());
			s.clear();
		}
		// else
			// TraceError("Name %s, crc %d", index->filename, index->filename_crc);

		if (!index->filename_crc)
			PushFreeIndex(index);
		else
			m_DataPositionMap.insert(TDataPositionMap::value_type(index->filename_crc, index));
	}
	return true;
}

bool CEterPack::Get(CMappedFile& out_file, const char * filename, LPCVOID * data)
{
	TEterPackIndex * index = FindIndex(filename);

	if (!index)
		return false;

	out_file.Create(m_stDataFileName.c_str(), data, index->data_position, index->data_size);

	bool bIsSecurityCheckRequired = (index->compressed_type == COMPRESSED_TYPE_SECURITY);

	if (bIsSecurityCheckRequired)
	{
#ifdef ENABLE_CRC32_CHECK
		DWORD dwCrc32 = GetCRC32((const char*)(*data), index->data_size);

		if (index->data_crc != dwCrc32)
			return false;
#endif
	}

	if (COMPRESSED_TYPE_COMPRESS == index->compressed_type)
	{
		CLZObject * zObj = new CLZObject;

		if (!CLZO::Instance().Decompress(*zObj, static_cast<const BYTE *>(*data)))
		{
			TraceError("Failed to decompress : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}
	else if (COMPRESSED_TYPE_SECURITY == index->compressed_type)
	{
		CLZObject * zObj = new CLZObject;

		if (!CLZO::Instance().Decompress(*zObj, static_cast<const BYTE *>(*data), s_adwEterPackSecurityKey))
		{
			TraceError("Failed to encrypt : %s", filename);
			delete zObj;
			return false;
		}

		out_file.BindLZObject(zObj);
		*data = zObj->GetBuffer();
	}

	return true;
}

bool CEterPack::CreateIndexFile()
{
	FILE * fp;

	if (NULL != (fp = fopen(m_indexFileName, "rb")))
	{
		fclose(fp);
		return true;
	}

	return false;
}

void CEterPack::PushFreeIndex(TEterPackIndex* index)
{
	if (index->filename_crc != 0)
	{
		TDataPositionMap::iterator i = m_DataPositionMap.find(index->filename_crc);

		if (i != m_DataPositionMap.end())
			m_DataPositionMap.erase(i);

		index->filename_crc = 0;
		memset(index->filename, 0, sizeof(index->filename));
	}
}

TEterPackIndex* CEterPack::FindIndex(const char * filename)
{
	static char tmpFilename[MAX_PATH + 1];
	strncpy(tmpFilename, filename, MAX_PATH);
	inlineConvertPackFilename(tmpFilename);

	DWORD filename_crc = GetCRC32(tmpFilename, strlen(tmpFilename));
	TDataPositionMap::iterator i = m_DataPositionMap.find(filename_crc);

	if (i == m_DataPositionMap.end())
		return NULL;

	return (i->second);
}

bool CEterPack::IsExist(const char * filename)
{
	return FindIndex(filename) ? true : false;
}

bool CEterPack::CreateDataFile()
{
	FILE * fp;

	if (NULL != (fp = fopen(m_stDataFileName.c_str(), "rb")))
	{
		fclose(fp);
		return true;
	}

	return false;
}

const char * CEterPack::GetDBName()
{
	return m_dbName;
}


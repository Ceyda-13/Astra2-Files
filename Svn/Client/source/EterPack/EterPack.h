#pragma once

#include <list>
#include <unordered_map>

#include "../EterBase/MappedFile.h"

#ifndef MAKEFOURCC
#define MAKEFOURCC(ch0, ch1, ch2, ch3)                                                  \
                        ((DWORD)(BYTE) (ch0       ) | ((DWORD)(BYTE) (ch1) <<  8) |     \
                        ((DWORD)(BYTE) (ch2) << 16) | ((DWORD)(BYTE) (ch3) << 24))
#endif

namespace eterpack
{
	const DWORD	c_PackCC = MAKEFOURCC('O', 'E', 'P', 'C');
	const DWORD	c_IndexCC = MAKEFOURCC('O', 'E', 'P', 'C');
	const DWORD c_Version = 2;

	const DWORD c_HeaderSize = sizeof(DWORD) + sizeof(DWORD) + sizeof(long);
};

enum EEterPackTypes
{
	DBNAME_MAX_LEN = 255,
	FILENAME_MAX_LEN = 160,
	FREE_INDEX_BLOCK_SIZE = 32768,
	FREE_INDEX_MAX_SIZE = 512,
	DATA_BLOCK_SIZE = 256,

	COMPRESSED_TYPE_NONE = 0,
	COMPRESSED_TYPE_COMPRESS = 1,
	COMPRESSED_TYPE_SECURITY = 2,
	COMPRESSED_TYPE_COUNT = 6,
};

#pragma pack(push, 4)
typedef struct SEterPackIndex
{
	long			id;
	char			filename[FILENAME_MAX_LEN + 1];
	DWORD			filename_crc;
	long			real_data_size;
	long			data_size;
	DWORD			data_crc;
	long			data_position;
	char			compressed_type;
} TEterPackIndex;
#pragma pack(pop)

typedef std::unordered_map<DWORD, TEterPackIndex *> TDataPositionMap;

class CEterPack
{
	public:
		CEterPack();
		virtual ~CEterPack();

		void				Destroy();
		bool				Create(const char * dbname, const char * pathName);

		const std::string&	GetPathName();
		const char *		GetDBName();

		bool				Get(CMappedFile & mappedFile, const char * filename, LPCVOID * data);
		bool				IsExist(const char * filename);

	private:
		bool				__BuildIndex();

		bool				CreateIndexFile();
		TEterPackIndex *	FindIndex(const char * filename);
		void				PushFreeIndex(TEterPackIndex * index);

		bool				CreateDataFile();
	protected:
		long					m_indexCount;

		char					m_dbName[DBNAME_MAX_LEN+1];
		char					m_indexFileName[MAX_PATH+1];
		TEterPackIndex *		m_indexData;

		TDataPositionMap		m_DataPositionMap;

		std::string				m_stDataFileName;
		std::string				m_stPathName;
};


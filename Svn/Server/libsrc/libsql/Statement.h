#pragma once
#include "AsyncSQL.h"

#include <string>
#include <vector>

class CStmt
{
	public:
		CStmt();
		virtual ~CStmt();

		bool    Prepare(CAsyncSQL * sql, const char * c_pszQuery);
		bool    BindParam(enum_field_types type, void * p, int32_t iMaxLen=0);
		bool    BindResult(enum_field_types type, void * p, int32_t iMaxLen=0);
		int32_t	Execute();
		bool    Fetch();

		void    Error(const char * c_pszMsg);

	public:
		int32_t	iRows;

	private:
		void    Destroy();

		MYSQL_STMT *            m_pkStmt;

		std::string             m_stQuery;

		std::vector<MYSQL_BIND> m_vec_param;
		uint32_t            m_uiParamCount;
		uint32_t *	m_puiParamLen;

		std::vector<MYSQL_BIND> m_vec_result;
		uint32_t            m_uiResultCount;
};  


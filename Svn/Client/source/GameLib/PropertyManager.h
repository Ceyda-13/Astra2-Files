#pragma once

#include "../EterPack/EterPack.h"

class CPropertyManager : public CSingleton<CPropertyManager>
{
	public:
		CPropertyManager();
		virtual ~CPropertyManager();

		void			Clear();
		bool			Initialize();
		bool			Get(DWORD dwCRC, CProperty ** ppProperty);

	protected:
		typedef std::map<DWORD, CProperty *> TPropertyCRCMap;
		TPropertyCRCMap								m_PropertyByCRCMap;
};


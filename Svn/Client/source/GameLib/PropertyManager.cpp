#include "StdAfx.h"
#include "../EterPack/EterPackManager.h"

#include "PropertyManager.h"
#include "Property.h"
#include <tinyxml/tinyxml.h>

CPropertyManager::CPropertyManager() = default;

CPropertyManager::~CPropertyManager()
{
	Clear();
}

bool CPropertyManager::Initialize()
{
	CMappedFile kPropertyXML;
	PBYTE pbPropertyXML;
	if (CEterPackManager::Instance().Get(kPropertyXML, "property.xml", (LPCVOID*)&pbPropertyXML))
	{
		char* pszXML = new char[kPropertyXML.Size() + 1];
		memcpy(pszXML, pbPropertyXML, kPropertyXML.Size());
		pszXML[kPropertyXML.Size()] = 0;
		std::stringstream kXML;
		kXML << pszXML;

		tinyxml2::XMLDocument doc;
		if (doc.Parse(pszXML) != tinyxml2::XML_SUCCESS)
		{
			TraceError("CPropertyManager::Initialize: Failed to load %s (%s)", "property.xml", doc.ErrorName());
			delete[] pszXML;
			return false;
		}

		tinyxml2::XMLElement *levelElement = doc.FirstChildElement("PropertyList");
		if (!levelElement)
			return true;

		for (tinyxml2::XMLElement* child = levelElement->FirstChildElement("Property"); child != nullptr; child = child->NextSiblingElement())
		{
			CProperty* pProperty = new CProperty(child->Attribute("filename"));
			if (!pProperty->ReadFromXML(child->Attribute("crc")))
			{
				TraceError("CPropertyManager::Initialize: Cannot register '%s'!", child->Attribute("filename"));
				delete pProperty;
				continue;
			}

			DWORD dwCRC = pProperty->GetCRC();

			TPropertyCRCMap::iterator itor = m_PropertyByCRCMap.find(dwCRC);

			if (m_PropertyByCRCMap.end() != itor)
			{
				Tracef("CPropertyManager::Initialize: Property(%u) already registered, replace %s to %s\n",
					dwCRC, itor->second->GetFileName(), child->Attribute("filename"));

				delete itor->second;
				itor->second = pProperty;
			}
			else
				m_PropertyByCRCMap.insert(TPropertyCRCMap::value_type(dwCRC, pProperty));


			const tinyxml2::XMLAttribute* pAttrib = child->FirstAttribute();
			while (pAttrib)
			{
				CTokenVector kVec;
				kVec.push_back(pAttrib->Value());
				pProperty->PutVector(pAttrib->Name(), kVec);
				pAttrib = pAttrib->Next();
			}

		}
		delete[] pszXML;
	}

	return true;
}

bool CPropertyManager::Get(DWORD dwCRC, CProperty ** ppProperty)
{
	TPropertyCRCMap::iterator itor = m_PropertyByCRCMap.find(dwCRC);

	if (m_PropertyByCRCMap.end() == itor)
		return false;

	*ppProperty = itor->second;
	return true;
}

void CPropertyManager::Clear()
{
	stl_wipe_second(m_PropertyByCRCMap);
}


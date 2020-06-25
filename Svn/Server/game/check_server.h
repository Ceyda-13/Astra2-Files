#pragma once

#include <string>
#include <vector>
#include "CheckServerKey.h"

class CCheckServer : public singleton<CCheckServer>
{
public:
	CCheckServer()
		: m_bValid(false)
	{}

	FORCEINLINE void AddServerKey(const char* pServerKey) {
		m_vKey.push_back(pServerKey);
	}

	FORCEINLINE bool CheckIP(const char* pIP) {
		m_bValid = false;

#ifndef _USE_SERVER_KEY_
		m_bValid = true;
		return m_bValid;
#endif
		for (uint32_t i = 0; i < m_vKey.size(); i++) {
			std::string errorString;
			if (CheckServerKey(m_vKey[i].c_str(), pIP, "", errorString)) {
				m_bValid = true;
				break;
			}
		}
		return m_bValid;
	}

	FORCEINLINE bool IsValid() const {
		return m_bValid;
	}

private:
	std::vector<std::string> m_vKey;
	bool m_bValid;
};




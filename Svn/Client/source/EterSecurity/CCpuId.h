#ifdef ENABLE_HARDWARE_INFOS
#pragma once

#include <string>

class CCpuId
{
public:
	CCpuId();
	~CCpuId();

	std::string getCpuID() { return m_CpuID; };

private:
	std::string m_CpuID;
};

#endif
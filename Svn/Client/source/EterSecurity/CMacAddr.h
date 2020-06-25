#ifdef ENABLE_HARDWARE_INFOS
#include <string>
#include "CNetworkAdapter.h"

class CMacAddr
{
public:
	CMacAddr();
	~CMacAddr();
	std::string getMacAddr() { return m_macAddr; };

private:

private:
	typedef unsigned char UCHAR;
	void setMACData(std::string test);
	bool IsPrimaryAdapter(DWORD dwIndex);
	bool findValidMac();
	bool initAdapters();
	int GetPrimaryAdapterIndex(void);

private:
	std::string m_macAddr;
	CNetworkAdapter*	m_pAdapters;
	UINT				m_nCount;
};

#endif
#ifdef ENABLE_HARDWARE_INFOS
#include <string>
#include "../eterBase/Singleton.h"

class HWIDMANAGER : public CSingleton <HWIDMANAGER>
{
	public:
		HWIDMANAGER();
		~HWIDMANAGER();
		std::string getMachineGUID(){ return m_MachineGUID; };
		std::string getMacAddr(){ return m_MacAddr; };
		std::string getHDDSerial(){ return m_HDDSerial; };
		std::string getHDDModel(){ return m_HDDModel; };
		std::string getCPUid(){ return m_CpuId; };

	protected:
		std::string m_MachineGUID;
		std::string m_MacAddr;
		std::string m_HDDSerial;
		std::string m_HDDModel;
		std::string m_CpuId;
};
#endif
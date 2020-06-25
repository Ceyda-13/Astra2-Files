#include "StdAfx.h"
#include "HWIDManager.h"
#include "CMachineGuid.h"
#include "CHddData.h"
#include "CCpuId.h"
#include "CMacAddr.h"

#ifdef ENABLE_HARDWARE_INFOS
HWIDMANAGER::HWIDMANAGER()
{
	CMachineGuid	c_MachineGuid;
	CMacAddr		c_MacAddr;
	CHddData		c_HddData;
	CCpuId			c_CpuId;

	m_MachineGUID	= c_MachineGuid.getMachineGUID();
	m_MacAddr		= c_MacAddr.getMacAddr();
	m_HDDSerial		= c_HddData.getHDDSerialNumber();
	m_HDDModel		= c_HddData.getHDDModelNumber();
	m_CpuId			= c_CpuId.getCpuID();
}

HWIDMANAGER::~HWIDMANAGER() = default;
#endif


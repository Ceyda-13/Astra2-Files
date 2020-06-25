#pragma once

#ifdef ENABLE_ACCE_SYSTEM
#include "Packet.h"

class CPythonAcce : public CSingleton<CPythonAcce>
{
	public:
		uint64_t	dwPrice;
		typedef std::vector<TAcceMaterial> TAcceMaterials;

	public:
		CPythonAcce();
		virtual ~CPythonAcce();

		void	Clear();
		void	AddMaterial(uint64_t dwRefPrice, BYTE bPos, TItemPos tPos);
		void	AddResult(DWORD dwItemVnum, DWORD dwMinAbs, DWORD dwMaxAbs);
		void	RemoveMaterial(uint64_t dwRefPrice, BYTE bPos);
		uint64_t	GetPrice() {return dwPrice;}
		bool	GetAttachedItem(BYTE bPos, BYTE & bHere, WORD & wCell);
		void	GetResultItem(DWORD & dwItemVnum, DWORD & dwMinAbs, DWORD & dwMaxAbs);

	protected:
		TAcceResult	m_vAcceResult;
		TAcceMaterials	m_vAcceMaterials;
};
#endif

#include "StdAfx.h"
#include "PythonOfflinePrivateShop.h"
#include "../GameLib/ItemManager.h"

CPythonOffflinePrivateShop::CPythonOffflinePrivateShop(void)
{
	Clear(-1);
}

CPythonOffflinePrivateShop::~CPythonOffflinePrivateShop(void)
{
	Clear(-1);
}

void CPythonOffflinePrivateShop::Clear(int8_t iType)
{
	switch (iType)
	{
		case 0:
		{
			for (auto it : m_OffflinePrivateShopData)
				delete it;

			m_OffflinePrivateShopData.clear();
		}
		break;

		case 1:
		{
			for (auto it : m_OffflinePrivateShopSearchData)
				delete it;

			m_OffflinePrivateShopSearchData.clear();
		}
		break;

		case 2:
		{
			for (auto it : m_OffflinePrivateShopLogData)
				delete it;

			m_OffflinePrivateShopLogData.clear();
		}
		break;

		default:
		{
			for (auto it : m_OffflinePrivateShopData)
				delete it;

			m_OffflinePrivateShopData.clear();

			for (auto it : m_OffflinePrivateShopSearchData)
				delete it;

			m_OffflinePrivateShopSearchData.clear();

			for (auto it : m_OffflinePrivateShopLogData)
				delete it;

			m_OffflinePrivateShopLogData.clear();
		}
		break;
	}
}

const TShopItemData * CPythonOffflinePrivateShop::GetOffflinePrivateShopItemData(BYTE bType, int id, DWORD dwSlotId) const
{
	return &(m_OffflinePrivateShopData[id]->items[dwSlotId]);
}

TShopItemData* CPythonOffflinePrivateShop::GetOffflinePrivateShopItemData(DWORD dwVid, DWORD dwSlotId)
{
	for (auto& it : m_OffflinePrivateShopData)
		if (it->dwVid == dwVid)
			return &it->items[dwSlotId];

	return nullptr;
}

DWORD CPythonOffflinePrivateShop::GetItemMetinSocket(BYTE bType, int id, DWORD dwSlotId, int dwMetinSocketIndex)
{
	if (dwMetinSocketIndex >= ITEM_SOCKET_SLOT_MAX_NUM)
		return 0;

	if (!bType)
		return m_OffflinePrivateShopData[id]->items[dwSlotId].alSockets[dwMetinSocketIndex];

	return 0;
}

void CPythonOffflinePrivateShop::GetItemAttribute(BYTE bType, int id, DWORD dwSlotId, int dwAttrSlotIndex, BYTE * pbyType, int * psValue)
{
	*pbyType = 0;
	*psValue = 0;

	if (dwAttrSlotIndex >= ITEM_ATTRIBUTE_SLOT_MAX_NUM)
		return;

	if (!bType)
	{
		*pbyType = m_OffflinePrivateShopData[id]->items[dwSlotId].aAttr[dwAttrSlotIndex].bType;
		*psValue = m_OffflinePrivateShopData[id]->items[dwSlotId].aAttr[dwAttrSlotIndex].sValue;
	}
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
DWORD CPythonOffflinePrivateShop::GetItemChangeLookVnum(BYTE bType, int id, DWORD dwSlotId)
{
	if (!bType)
		return m_OffflinePrivateShopData[id]->items[dwSlotId].changelook;

	return 0;
}
#endif

PyObject * opsGetItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid, iAttributeSlotIndex, sValue;
	BYTE byType, bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &slotid))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 3, &iAttributeSlotIndex))
		return Py_BuildException();

	if (!bType)
	{
		if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
			return Py_BuildValue("ii", 0, 0);
	}

	CPythonOffflinePrivateShop::Instance().GetItemAttribute(bType, id, slotid, iAttributeSlotIndex, &byType, &sValue);

	return Py_BuildValue("ii", byType, sValue);
}

PyObject * opsGetItemMetinSocket(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid, iMetinSocketIndex;
	BYTE bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &slotid))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 3, &iMetinSocketIndex))
		return Py_BuildException();

	if (!bType)
	{
		if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
			return Py_BuildValue("i", 0);
	}

	int nMetinSocketValue = CPythonOffflinePrivateShop::Instance().GetItemMetinSocket(bType, id, slotid, iMetinSocketIndex);
	return Py_BuildValue("i", nMetinSocketValue);
}

PyObject * opsGetItemVnum(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid;
	BYTE bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &slotid))
		return Py_BuildException();

	if (!bType)
	{
		if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
			return Py_BuildValue("i", 0);
	}

	DWORD dwVnum = CPythonOffflinePrivateShop::Instance().GetOffflinePrivateShopItemData(bType, id, slotid)->vnum;
	return Py_BuildValue("i", dwVnum);
}

PyObject * opsGetItemCount(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid;
	BYTE bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &slotid))
		return Py_BuildException();

	if (!bType)
	{
		if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
			return Py_BuildValue("i", 0);
	}

	DWORD dwCount = CPythonOffflinePrivateShop::Instance().GetOffflinePrivateShopItemData(bType, id, slotid)->count;
	return Py_BuildValue("i", dwCount);
}

PyObject * opsGetItemPrice(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid, price;
	BYTE bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &slotid))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 3, &price))
		return Py_BuildException();

	/*if (price >= SHOP_PRICE_MAX_NUM)
		return Py_BuildValue("i", 0);*/

	if (!bType)
	{
		if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
			return Py_BuildValue("i", 0);
	}


	uint64_t dwPrice = CPythonOffflinePrivateShop::Instance().GetOffflinePrivateShopItemData(bType, id, slotid)->price;
	return Py_BuildValue("K", dwPrice);
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject * opsGetItemChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	int id, slotid;
	BYTE bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &slotid))
		return Py_BuildException();

	if (!bType)
	{
		if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
			return Py_BuildValue("i", 0);
	}

	int iChangeLook = CPythonOffflinePrivateShop::Instance().GetItemChangeLookVnum(bType, id, slotid);
	return Py_BuildValue("i", iChangeLook);
}
#endif

PyObject * opsGetPosition(PyObject* poSelf, PyObject* poArgs)
{
	int id;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
		return Py_BuildValue("iii", 0, 0, 0);

	const TOfflinePrivateShopInfo * Info = CPythonOffflinePrivateShop::Instance().GetInfoVectorRef()[id];
	return Py_BuildValue("iii", Info->dwX, Info->dwY, Info->dwMapIndex);
}

PyObject * opsGetSign(PyObject* poSelf, PyObject* poArgs)
{
	int id;
	BYTE bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &id))
		return Py_BuildException();

	if (!bType)
	{
		if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
			return Py_BuildValue("s", "");
	}

	if (!bType)
		return Py_BuildValue("s", CPythonOffflinePrivateShop::Instance().GetInfoVectorRef()[id]->szSign);

	return Py_BuildValue("i", 0);
}

PyObject * opsGetTime(PyObject* poSelf, PyObject* poArgs)
{
	int id;

	if (!PyTuple_GetInteger(poArgs, 0, &id))
		return Py_BuildException();

	if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
		return Py_BuildValue("ii", 0, 0);

	const TOfflinePrivateShopInfo * Info = CPythonOffflinePrivateShop::Instance().GetInfoVectorRef()[id];
	return Py_BuildValue("ii", Info->dwStartTime, Info->dwEndTime);
}

PyObject * opsGetVid(PyObject* poSelf, PyObject* poArgs)
{
	int id;
	BYTE bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &id))
		return Py_BuildException();

	if (!bType)
	{
		if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
			return Py_BuildValue("i", 0);
	}

	if (!bType)
		return Py_BuildValue("i", CPythonOffflinePrivateShop::Instance().GetInfoVectorRef()[id]->dwVid);

	return Py_BuildValue("i", 0);
}

PyObject * opsGetPrice(PyObject* poSelf, PyObject* poArgs)
{
	int id, price;
	BYTE bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 1, &id))
		return Py_BuildException();

	if (!PyTuple_GetInteger(poArgs, 2, &price))
		return Py_BuildException();

	if (!bType)
	{
		if (id >= CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size())
			return Py_BuildValue("i", 0);
	}

	if (!bType)
		return Py_BuildValue("K", CPythonOffflinePrivateShop::Instance().GetInfoVectorRef()[id]->dwPrices);

	return Py_BuildValue("i", 0);
}

PyObject * opsGetShopCount(PyObject* poSelf, PyObject* poArgs)
{
	BYTE bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	if (!bType)
		return Py_BuildValue("i", CPythonOffflinePrivateShop::Instance().GetInfoVectorRef().size());

	return Py_BuildValue("i", 0);
}

PyObject * opsGetWorth(PyObject* poSelf, PyObject* poArgs)
{
	uint64_t iWorth = 0;
	int iID = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	auto& pData = CPythonOffflinePrivateShop::Instance().GetInfoVectorRef()[iID]->items;
	for (auto& it : pData)
		iWorth += it.price;

	return Py_BuildValue("K", iWorth);
}

PyObject * opsGetUnlockSlots(PyObject* poSelf, PyObject* poArgs)
{
	int iID = 0;
	if (!PyTuple_GetInteger(poArgs, 0, &iID))
		return Py_BuildException();

	auto* pData = CPythonOffflinePrivateShop::Instance().GetInfoVectorRef()[iID];

	return Py_BuildValue("i", pData->bUnlockSlots);
}

PyObject * opsClear(PyObject* poSelf, PyObject* poArgs)
{
	BYTE bType;

	if (!PyTuple_GetInteger(poArgs, 0, &bType))
		return Py_BuildException();

	CPythonOffflinePrivateShop::Instance().Clear(bType);

	return Py_BuildNone();
}

PyObject * opsGetPrivateShopSearchResultMaxCount(PyObject* poSelf, PyObject* poArgs)
{
	auto& vInfo = CPythonOffflinePrivateShop::Instance().GetSearchInfoVectorRef();
	return Py_BuildValue("i", vInfo.size());
}

PyObject * opsGetPrivateShopSearchResult(PyObject* poSelf, PyObject* poArgs)
{
	int dwLine;
	if (!PyTuple_GetInteger(poArgs, 0, &dwLine))
		return Py_BuildException();

	auto& vInfo = CPythonOffflinePrivateShop::Instance().GetSearchInfoVectorRef();
	if (dwLine >= vInfo.size())
		return Py_BuildValue("ssii", "", "", 0, 0);

	auto* pData = vInfo[dwLine];
	CItemManager::Instance().SelectItemData(pData->item.vnum);
	auto* pItemData = CItemManager::Instance().GetSelectedItemDataPointer();
	return Py_BuildValue("ssiK", pItemData->GetName(), pData->szSellerName, pData->item.count, pData->item.price);
}

#ifdef ENABLE_CHANGE_LOOK_SYSTEM
PyObject * opsGetPrivateShopItemChangeLookVnum(PyObject* poSelf, PyObject* poArgs)
{
	int dwLine;
	if (!PyTuple_GetInteger(poArgs, 0, &dwLine))
		return Py_BuildException();

	auto& vInfo = CPythonOffflinePrivateShop::Instance().GetSearchInfoVectorRef();
	if (dwLine >= vInfo.size())
		return Py_BuildValue("i", 0);

	auto* pData = vInfo[dwLine];
	return Py_BuildValue("i", pData->item.changelook);
}
#endif

PyObject * opsGetPrivateShopSelectItemVnum(PyObject* poSelf, PyObject* poArgs)
{
	int dwLine;
	if (!PyTuple_GetInteger(poArgs, 0, &dwLine))
		return Py_BuildException();

	auto& vInfo = CPythonOffflinePrivateShop::Instance().GetSearchInfoVectorRef();
	if (dwLine >= vInfo.size())
		return Py_BuildValue("i", 0);

	auto* pData = vInfo[dwLine];
	return Py_BuildValue("i", pData->item.vnum);
}

PyObject * opsGetPrivateShopSelectItemMetinSocket(PyObject* poSelf, PyObject* poArgs)
{
	int dwLine;
	if (!PyTuple_GetInteger(poArgs, 0, &dwLine))
		return Py_BuildException();

	BYTE bSlot;
	if (!PyTuple_GetInteger(poArgs, 1, &bSlot))
		return Py_BuildException();

	auto& vInfo = CPythonOffflinePrivateShop::Instance().GetSearchInfoVectorRef();
	if (dwLine >= vInfo.size())
		return Py_BuildValue("i", 0);

	auto* pData = vInfo[dwLine];
	return Py_BuildValue("i", pData->item.alSockets[bSlot]);
}

PyObject * opsGetPrivateShopSelectItemAttribute(PyObject* poSelf, PyObject* poArgs)
{
	int dwLine;
	if (!PyTuple_GetInteger(poArgs, 0, &dwLine))
		return Py_BuildException();

	BYTE bSlot;
	if (!PyTuple_GetInteger(poArgs, 1, &bSlot))
		return Py_BuildException();

	auto& vInfo = CPythonOffflinePrivateShop::Instance().GetSearchInfoVectorRef();
	if (dwLine >= vInfo.size())
		return Py_BuildValue("i", 0);

	auto* pData = vInfo[dwLine];
	return Py_BuildValue("ii", pData->item.aAttr[bSlot].bType, pData->item.aAttr[bSlot].sValue);
}

PyObject * opsGetPrivateShopSearchInfo(PyObject* poSelf, PyObject* poArgs)
{
	int dwLine;
	if (!PyTuple_GetInteger(poArgs, 0, &dwLine))
		return Py_BuildException();

	auto& vInfo = CPythonOffflinePrivateShop::Instance().GetSearchInfoVectorRef();
	if (dwLine >= vInfo.size())
		return Py_BuildValue("ii", 0, 0);

	auto* pData = vInfo[dwLine];
	return Py_BuildValue("ii", pData->dwVid, pData->item.display_pos);
}


PyObject* opsGetPrivateShopLogResultMaxCount(PyObject* poSelf, PyObject* poArgs)
{
	auto& vInfo = CPythonOffflinePrivateShop::Instance().GetLogVectorRef();
	return Py_BuildValue("i", vInfo.size());
}

PyObject* opsGetPrivateShopLogResult(PyObject* poSelf, PyObject* poArgs)
{
	int dwLine;
	if (!PyTuple_GetInteger(poArgs, 0, &dwLine))
		return Py_BuildException();

	auto& vInfo = CPythonOffflinePrivateShop::Instance().GetLogVectorRef();
	if (dwLine >= vInfo.size())
		return Py_BuildValue("iii", 0, 0, 0);

	auto* pData = vInfo[dwLine];
	return Py_BuildValue("Kii", pData->dwPrice, pData->dwVnum, pData->dwDate);
}

void initOfflinePrivateShop()
{
	static PyMethodDef s_methods[] =
	{
		{ "GetItemAttribute",						opsGetItemAttribute,						METH_VARARGS },
		{ "GetItemMetinSocket",						opsGetItemMetinSocket,						METH_VARARGS },
		{ "GetItemVnum",							opsGetItemVnum,								METH_VARARGS },
		{ "GetItemCount",							opsGetItemCount,							METH_VARARGS },
		{ "GetItemPrice",							opsGetItemPrice,							METH_VARARGS },
#ifdef ENABLE_CHANGE_LOOK_SYSTEM								
		{ "GetItemChangeLookVnum",					opsGetItemChangeLookVnum,					METH_VARARGS },
#endif				
		{ "GetPosition",							opsGetPosition,								METH_VARARGS },
		{ "GetSign",								opsGetSign,									METH_VARARGS },
		{ "GetTime",								opsGetTime,									METH_VARARGS },
		{ "GetVid",									opsGetVid,									METH_VARARGS },
		{ "GetPrice",								opsGetPrice,								METH_VARARGS },
		{ "GetShopCount",							opsGetShopCount,							METH_VARARGS },
		{ "GetWorth",								opsGetWorth,								METH_VARARGS },
		{ "GetUnlockSlots",							opsGetUnlockSlots,							METH_VARARGS },
		{ "Clear",									opsClear,									METH_VARARGS },
		{ "GetPrivateShopSearchResultMaxCount",		opsGetPrivateShopSearchResultMaxCount,		METH_VARARGS },
		{ "GetPrivateShopSearchResult",				opsGetPrivateShopSearchResult,				METH_VARARGS },
		{ "GetPrivateShopSelectItemVnum",			opsGetPrivateShopSelectItemVnum,			METH_VARARGS },
		{ "GetPrivateShopSelectItemMetinSocket",	opsGetPrivateShopSelectItemMetinSocket,		METH_VARARGS },
		{ "GetPrivateShopSelectItemAttribute",		opsGetPrivateShopSelectItemAttribute,		METH_VARARGS },
		{ "GetPrivateShopSearchInfo",				opsGetPrivateShopSearchInfo,				METH_VARARGS },
#ifdef ENABLE_CHANGE_LOOK_SYSTEM
		{ "GetPrivateShopItemChangeLookVnum",		opsGetPrivateShopItemChangeLookVnum,		METH_VARARGS },
#endif
		{ "GetPrivateShopLogResultMaxCount",		opsGetPrivateShopLogResultMaxCount,			METH_VARARGS },
		{ "GetPrivateShopLogResult",				opsGetPrivateShopLogResult,					METH_VARARGS },
		{ NULL,						NULL,						NULL },
	};

	PyObject * poModule = Py_InitModule("ops", s_methods);
}


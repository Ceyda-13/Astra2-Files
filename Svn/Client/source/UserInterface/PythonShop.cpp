#include "stdafx.h"
#include "PythonShop.h"
#include "Packet.h"

#include "PythonNetworkStream.h"

#ifdef ENABLE_SHOP_TAB
void CPythonShop::SetItemData(DWORD dwIndex, const TShopItemData & c_rShopItemData)
{
	DWORD dwSlotPos = dwIndex % SHOP_HOST_ITEM_MAX_NUM;
	items[dwSlotPos] = c_rShopItemData;
}

BOOL CPythonShop::GetItemData(DWORD dwIndex, const TShopItemData ** c_ppItemData)
{
	DWORD dwSlotPos = dwIndex % SHOP_HOST_ITEM_MAX_NUM;
	*c_ppItemData = &items[dwSlotPos];

	return true;
}
#else
void CPythonShop::SetTabCoinType(BYTE tabIdx, BYTE coinType)
{
	if (tabIdx >= m_bTabCount)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return;
	}
	m_aShoptabs[tabIdx].coinType = coinType;
}

BYTE CPythonShop::GetTabCoinType(BYTE tabIdx)
{
	if (tabIdx >= m_bTabCount)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return 0xff;
	}
	return m_aShoptabs[tabIdx].coinType;
}

void CPythonShop::SetTabName(BYTE tabIdx, const char* name)
{
	if (tabIdx >= m_bTabCount)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return;
	}
	m_aShoptabs[tabIdx].name = name;
}

const char* CPythonShop::GetTabName(BYTE tabIdx)
{
	if (tabIdx >= m_bTabCount)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d.", tabIdx, SHOP_TAB_COUNT_MAX);
		return NULL;
	}

	return m_aShoptabs[tabIdx].name.c_str();
}

void CPythonShop::SetItemData(DWORD dwIndex, const TShopItemData & c_rShopItemData)
{
	BYTE tabIdx = dwIndex / SHOP_HOST_ITEM_MAX_NUM;
	DWORD dwSlotPos = dwIndex % SHOP_HOST_ITEM_MAX_NUM;

	SetItemData(tabIdx, dwSlotPos, c_rShopItemData);
}

BOOL CPythonShop::GetItemData(DWORD dwIndex, const TShopItemData ** c_ppItemData)
{
	BYTE tabIdx = dwIndex / SHOP_HOST_ITEM_MAX_NUM;
	DWORD dwSlotPos = dwIndex % SHOP_HOST_ITEM_MAX_NUM;

	return GetItemData(tabIdx, dwSlotPos, c_ppItemData);
}

void CPythonShop::SetItemData(BYTE tabIdx, DWORD dwSlotPos, const TShopItemData & c_rShopItemData)
{
	if (tabIdx >= SHOP_TAB_COUNT_MAX || dwSlotPos >= SHOP_HOST_ITEM_MAX_NUM)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d. dwSlotPos(%d) must be less than %d", tabIdx, SHOP_TAB_COUNT_MAX, dwSlotPos, SHOP_HOST_ITEM_MAX_NUM);
		return;
	}

	m_aShoptabs[tabIdx].items[dwSlotPos] = c_rShopItemData;
}

BOOL CPythonShop::GetItemData(BYTE tabIdx, DWORD dwSlotPos, const TShopItemData ** c_ppItemData)
{
	if (tabIdx >= SHOP_TAB_COUNT_MAX || dwSlotPos >= SHOP_HOST_ITEM_MAX_NUM)
	{
		TraceError("Out of Index. tabIdx(%d) must be less than %d. dwSlotPos(%d) must be less than %d", tabIdx, SHOP_TAB_COUNT_MAX, dwSlotPos, SHOP_HOST_ITEM_MAX_NUM);
		return FALSE;
	}

	*c_ppItemData = &m_aShoptabs[tabIdx].items[dwSlotPos];

	return TRUE;
}
#endif

void CPythonShop::ClearPrivateShopStock()
{
	m_PrivateShopItemStock.clear();
}

void CPythonShop::AddPrivateShopItemStock(TItemPos ItemPos, BYTE dwDisplayPos, uint64_t dwPrice)
{
	DelPrivateShopItemStock(ItemPos);

	TShopItemTable SellingItem;
	memset(&SellingItem, 0, sizeof(SellingItem));
	SellingItem.vnum = 0;
	SellingItem.count = 0;
	SellingItem.pos = ItemPos;
	SellingItem.price = dwPrice;
	SellingItem.display_pos = dwDisplayPos;
	m_PrivateShopItemStock.insert(make_pair(ItemPos, SellingItem));
}
void CPythonShop::DelPrivateShopItemStock(TItemPos ItemPos)
{
	if (m_PrivateShopItemStock.end() == m_PrivateShopItemStock.find(ItemPos))
		return;

	m_PrivateShopItemStock.erase(ItemPos);
}

uint64_t CPythonShop::GetPrivateShopItemPrice(TItemPos ItemPos)
{
	TPrivateShopItemStock::iterator itor = m_PrivateShopItemStock.find(ItemPos);

	if (m_PrivateShopItemStock.end() == itor)
		return 0;

	TShopItemTable & rShopItemTable = itor->second;
	return rShopItemTable.price;
}

struct ItemStockSortFunc
{
	bool operator() (TShopItemTable & rkLeft, TShopItemTable & rkRight)
	{
		return rkLeft.display_pos < rkRight.display_pos;
	}
};
void CPythonShop::BuildPrivateShop(const char * c_szName
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
, int iTime, int iTitle, int iPoly
#endif
)
{
	std::vector<TShopItemTable> ItemStock;
	ItemStock.reserve(m_PrivateShopItemStock.size());

	TPrivateShopItemStock::iterator itor = m_PrivateShopItemStock.begin();
	for (; itor != m_PrivateShopItemStock.end(); ++itor)
	{
		ItemStock.push_back(itor->second);
	}

	std::sort(ItemStock.begin(), ItemStock.end(), ItemStockSortFunc());

	CPythonNetworkStream::Instance().SendBuildPrivateShopPacket(c_szName, ItemStock
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
, iTime, iTitle, iPoly
#endif
);
}

void CPythonShop::Open(BOOL isPrivateShop, BOOL isMainPrivateShop)
{
	m_isShoping = TRUE;
	m_isPrivateShop = isPrivateShop;
	m_isMainPlayerPrivateShop = isMainPrivateShop;
}

void CPythonShop::Close()
{
	m_isShoping = FALSE;
	m_isPrivateShop = FALSE;
	m_isMainPlayerPrivateShop = FALSE;
}

BOOL CPythonShop::IsOpen()
{
	return m_isShoping;
}

BOOL CPythonShop::IsPrivateShop()
{
	return m_isPrivateShop;
}

BOOL CPythonShop::IsMainPlayerPrivateShop()
{
	return m_isMainPlayerPrivateShop;
}

void CPythonShop::Clear()
{
	m_isShoping = FALSE;
	m_isPrivateShop = FALSE;
	m_isMainPlayerPrivateShop = FALSE;
	ClearPrivateShopStock();
	m_bTabCount = 1;
#ifdef ENABLE_SHOP_TAB
	memset(items, 0, sizeof(items));
#else
	for (int i = 0; i < SHOP_TAB_COUNT_MAX; i++)
		memset (m_aShoptabs[i].items, 0, sizeof(TShopItemData) * SHOP_HOST_ITEM_MAX_NUM);
#endif
}

CPythonShop::CPythonShop(void)
{
	Clear();
}

CPythonShop::~CPythonShop(void)
{
}

PyObject * shopOpen(PyObject * poSelf, PyObject * poArgs)
{
	int isPrivateShop = false;
	PyTuple_GetInteger(poArgs, 0, &isPrivateShop);
	int isMainPrivateShop = false;
	PyTuple_GetInteger(poArgs, 1, &isMainPrivateShop);

	CPythonShop& rkShop=CPythonShop::Instance();
	rkShop.Open(isPrivateShop, isMainPrivateShop);
	return Py_BuildNone();
}

PyObject * shopClose(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	rkShop.Close();
	return Py_BuildNone();
}

PyObject * shopIsOpen(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsOpen());
}

PyObject * shopIsPrviateShop(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsPrivateShop());
}

PyObject * shopIsMainPlayerPrivateShop(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop& rkShop=CPythonShop::Instance();
	return Py_BuildValue("i", rkShop.IsMainPlayerPrivateShop());
}

PyObject * shopGetItemID(PyObject * poSelf, PyObject * poArgs)
{
	int nPos;
	if (!PyTuple_GetInteger(poArgs, 0, &nPos))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(nPos, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->vnum);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemCount(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->count);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("K", c_pItemData->price);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemMetinSocket(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int iMetinSocketIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iMetinSocketIndex))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->alSockets[iMetinSocketIndex]);

	return Py_BuildValue("i", 0);
}

PyObject * shopGetItemAttribute(PyObject * poSelf, PyObject * poArgs)
{
	int iIndex;
	if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		return Py_BuildException();
	int iAttrSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &iAttrSlotIndex))
		return Py_BuildException();

	if (iAttrSlotIndex >= 0 && iAttrSlotIndex < ITEM_ATTRIBUTE_SLOT_MAX_NUM)
	{
		const TShopItemData * c_pItemData;
		if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
			return Py_BuildValue("ii", c_pItemData->aAttr[iAttrSlotIndex].bType, c_pItemData->aAttr[iAttrSlotIndex].sValue);
	}

	return Py_BuildValue("ii", 0, 0);
}

PyObject * shopClearPrivateShopStock(PyObject * poSelf, PyObject * poArgs)
{
	CPythonShop::Instance().ClearPrivateShopStock();
	return Py_BuildNone();
}

PyObject * shopAddPrivateShopItemStock(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	WORD wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();
	int iDisplaySlotIndex;
	if (!PyTuple_GetInteger(poArgs, 2, &iDisplaySlotIndex))
		return Py_BuildException();
	uint64_t iPrice;
	if (!PyTuple_GetUnsignedLongLong(poArgs, 3, &iPrice))
		return Py_BuildException();

	CPythonShop::Instance().AddPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex), iDisplaySlotIndex, iPrice);
	return Py_BuildNone();
}
PyObject * shopDelPrivateShopItemStock(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	WORD wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();

	CPythonShop::Instance().DelPrivateShopItemStock(TItemPos(bItemWindowType, wItemSlotIndex));
	return Py_BuildNone();
}
PyObject * shopGetPrivateShopItemPrice(PyObject * poSelf, PyObject * poArgs)
{
	BYTE bItemWindowType;
	if (!PyTuple_GetInteger(poArgs, 0, &bItemWindowType))
		return Py_BuildException();
	WORD wItemSlotIndex;
	if (!PyTuple_GetInteger(poArgs, 1, &wItemSlotIndex))
		return Py_BuildException();

	uint64_t iValue = CPythonShop::Instance().GetPrivateShopItemPrice(TItemPos(bItemWindowType, wItemSlotIndex));
	return Py_BuildValue("K", iValue);
}
PyObject * shopBuildPrivateShop(PyObject * poSelf, PyObject * poArgs)
{
	char * szName;
	if (!PyTuple_GetString(poArgs, 0, &szName))
		return Py_BuildException();

#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
	int iTime;
	if (!PyTuple_GetInteger(poArgs, 1, &iTime))
		return Py_BuildException();

	int iTitle;
	if (!PyTuple_GetInteger(poArgs, 2, &iTitle))
		return Py_BuildException();

	int iPoly;
	if (!PyTuple_GetInteger(poArgs, 3, &iPoly))
		return Py_BuildException();

	CPythonShop::Instance().BuildPrivateShop(szName, iTime, iTitle, iPoly);
#else
	CPythonShop::Instance().BuildPrivateShop(szName);
#endif
	return Py_BuildNone();
}

PyObject * shopGetTabCount(PyObject * poSelf, PyObject * poArgs)
{
	return Py_BuildValue("i", CPythonShop::Instance().GetTabCount());
}

PyObject * shopGetTabName(PyObject * poSelf, PyObject * poArgs)
{
#ifdef ENABLE_SHOP_TAB
	return Py_BuildValue("s", NULL);
#else
	BYTE bTabIdx;
	if (!PyTuple_GetInteger(poArgs, 0, &bTabIdx))
		return Py_BuildException();

	return Py_BuildValue("s", CPythonShop::Instance().GetTabName(bTabIdx));
#endif
}

PyObject * shopGetTabCoinType(PyObject * poSelf, PyObject * poArgs)
{
#ifdef ENABLE_SHOP_TAB
	return Py_BuildValue("i", 0);
#else
	BYTE bTabIdx;
	if (!PyTuple_GetInteger(poArgs, 0, &bTabIdx))
		return Py_BuildException();

	return Py_BuildValue("i", CPythonShop::Instance().GetTabCoinType(bTabIdx));
#endif
}

// #ifdef ENABLE_CHANGE_LOOK_SYSTEM
// PyObject * shopGetItemChangeLook(PyObject * poSelf, PyObject * poArgs)
// {
	// int iIndex;
	// if (!PyTuple_GetInteger(poArgs, 0, &iIndex))
		// return Py_BuildException();

	// const TShopItemData * c_pItemData;
	// if (CPythonShop::Instance().GetItemData(iIndex, &c_pItemData))
		// return Py_BuildValue("i", c_pItemData->changelook);

	// return Py_BuildValue("i", 0);
// }
// #endif

#ifdef ENABLE_MULTISHOP
PyObject * shopGetBuyWithItem(PyObject * poSelf, PyObject * poArgs)
{
	int nPos;
	if (!PyTuple_GetInteger(poArgs, 0, &nPos))
		return Py_BuildException();

	const TShopItemData * c_pItemData;
	if (CPythonShop::Instance().GetItemData(nPos, &c_pItemData))
		return Py_BuildValue("i", c_pItemData->wPriceVnum);

	return Py_BuildValue("i", 0);
}
#endif

#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
PyObject* shopGetUnlockSlots(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildValue("i", CPythonShop::Instance().GetUnlockSlots());
}
#endif

void initshop()
{
	static PyMethodDef s_methods[] =
	{
		// Shop
		{ "Open",						shopOpen,						METH_VARARGS },
		{ "Close",						shopClose,						METH_VARARGS },
		{ "IsOpen",						shopIsOpen,						METH_VARARGS },
		{ "IsPrivateShop",				shopIsPrviateShop,				METH_VARARGS },
		{ "IsMainPlayerPrivateShop",	shopIsMainPlayerPrivateShop,	METH_VARARGS },
		{ "GetItemID",					shopGetItemID,					METH_VARARGS },
		{ "GetItemCount",				shopGetItemCount,				METH_VARARGS },
		{ "GetItemPrice",				shopGetItemPrice,				METH_VARARGS },

		{ "GetItemMetinSocket",			shopGetItemMetinSocket,			METH_VARARGS },
		{ "GetItemAttribute",			shopGetItemAttribute,			METH_VARARGS },
		{ "GetTabCount",				shopGetTabCount,				METH_VARARGS },
		{ "GetTabName",					shopGetTabName,					METH_VARARGS },
		{ "GetTabCoinType",				shopGetTabCoinType,				METH_VARARGS },

		// Private Shop
		{ "ClearPrivateShopStock",		shopClearPrivateShopStock,		METH_VARARGS },
		{ "AddPrivateShopItemStock",	shopAddPrivateShopItemStock,	METH_VARARGS },
		{ "DelPrivateShopItemStock",	shopDelPrivateShopItemStock,	METH_VARARGS },
		{ "GetPrivateShopItemPrice",	shopGetPrivateShopItemPrice,	METH_VARARGS },

		{ "BuildPrivateShop",			shopBuildPrivateShop,			METH_VARARGS },

// #ifdef ENABLE_CHANGE_LOOK_SYSTEM
		// { "GetItemChangeLook", 		shopGetItemChangeLook, 		METH_VARARGS },
// #endif
#ifdef ENABLE_MULTISHOP
		{ "GetBuyWithItem",				shopGetBuyWithItem,				METH_VARARGS },
#endif
#ifdef ENABLE_OFFLINE_PRIVATE_SHOP
		{ "GetUnlockSlots",				shopGetUnlockSlots,				METH_VARARGS },
#endif
		{ NULL,							NULL,							NULL },
	};
	PyObject * poModule = Py_InitModule("shop", s_methods);

#ifdef ENABLE_SHOP_TAB
	PyModule_AddIntConstant(poModule, "SHOP_PAGE_COLUMN", SHOP_PAGE_COLUMN);
	PyModule_AddIntConstant(poModule, "SHOP_PAGE_ROW", SHOP_PAGE_ROW);
	PyModule_AddIntConstant(poModule, "SHOP_PAGE_SIZE", SHOP_PAGE_SIZE);
	PyModule_AddIntConstant(poModule, "SHOP_PAGE_COUNT", SHOP_PAGE_COUNT);
#endif
	PyModule_AddIntConstant(poModule, "SHOP_SLOT_COUNT", SHOP_HOST_ITEM_MAX_NUM);
	PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_GOLD", SHOP_COIN_TYPE_GOLD);
	PyModule_AddIntConstant(poModule, "SHOP_COIN_TYPE_SECONDARY_COIN", SHOP_COIN_TYPE_SECONDARY_COIN);
}

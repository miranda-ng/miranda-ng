#include "stdafx.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////////

struct CTreeItemData
{
	bool  m_isProtocol;
	char *m_pszModule;
	int   m_nStatus;
};

class CSettingsDlg : public CDlgBase
{
	CCtrlCheck chkMsg, chkState;
	CCtrlTreeView m_tree;

	void DisableChildren(HTREEITEM htiParent, int iState)
	{
		TVITEMEX tvi = {};
		tvi.mask = TVIF_STATE;
		tvi.stateMask = TVIS_STATEIMAGEMASK;
		if (iState == 1) {
			tvi.mask |= TVIF_STATEEX;
			tvi.uStateEx = TVIS_EX_DISABLED;
			tvi.state = INDEXTOSTATEIMAGEMASK(2);
		}
		else tvi.state = INDEXTOSTATEIMAGEMASK(1);

		for (HTREEITEM hti = m_tree.GetChild(htiParent); hti; hti = m_tree.GetNextSibling(hti)) {
			tvi.hItem = hti;
			m_tree.SetItem(&tvi);
		}
	}

	void FreeMemory(HTREEITEM hti)
	{
		for (HTREEITEM h = m_tree.GetChild(hti); h; h = m_tree.GetNextSibling(h)) {
			FreeMemory(h);
			CTreeItemData *pData = GetItemData(h);
			if (pData)
				delete pData;
		}
	}

	CTreeItemData* GetItemData(HTREEITEM hti)
	{
		TVITEMEX tvi = {};
		tvi.hItem = hti;
		tvi.mask = TVIF_PARAM | TVIF_HANDLE;
		return (m_tree.GetItem(&tvi)) ? (CTreeItemData *)tvi.lParam : nullptr;
	}

	void SaveExclusion(HTREEITEM htiParent)
	{
		for (HTREEITEM hti = m_tree.GetChild(htiParent); hti; hti = m_tree.GetNextSibling(hti)) {
			CTreeItemData *pData = GetItemData(hti);
			bool isChecked = m_tree.IsSelected(hti);
			if (pData->m_isProtocol) {
				g_plugin.ExcludeProtocol(pData->m_pszModule, isChecked);
				SaveExclusion(hti);
			}
			else g_plugin.ExcludeProtocolStatus(pData->m_pszModule, pData->m_nStatus, isChecked);
		}
	}

	HTREEITEM TreeInsert(wchar_t *pName, HTREEITEM htiParent, bool bSelected, CTreeItemData *pData)
	{
		TVINSERTSTRUCT tvi = {};
		tvi.hParent = htiParent;
		tvi.hInsertAfter = TVI_LAST;
		tvi.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_STATE;
		tvi.item.pszText = pName;
		tvi.item.lParam = reinterpret_cast<LPARAM>(pData);
		tvi.item.stateMask = TVIS_SELECTED;
		tvi.item.state = (bSelected) ? TVIS_SELECTED : 0;
		return m_tree.InsertItem(&tvi);
	}

public:
	CSettingsDlg() :
		CDlgBase(g_plugin, IDD_DIALOG_SETTINGS),
		m_tree(this, IDC_TREE_PROTOCOLS),
		chkMsg(this, IDC_CHECK_SYNCK_STATUS_MSG),
		chkState(this, IDC_CHECK_STATUSES)
	{
		CreateLink(chkMsg, g_plugin.bSyncStatusMsg);
		CreateLink(chkState, g_plugin.bSyncStatusState);

		m_tree.OnItemChanged = Callback(this, &CSettingsDlg::onItemChanged_Tree);
	}

	bool OnInitDialog() override
	{
		for (auto &pAccount : Accounts()) {
			if (mir_strcmpi(pAccount->szProtoName, "SKYPE")) {
				g_plugin.ExcludeProtocol(pAccount->szModuleName, true);
				continue;
			}

			CTreeItemData *pItemData = new CTreeItemData;
			pItemData->m_isProtocol = true;
			pItemData->m_pszModule = pAccount->szModuleName;			

			bool bProtocolExcluded = g_plugin.IsProtocolExcluded(pAccount->szModuleName);
			HTREEITEM hti = TreeInsert(pAccount->tszAccountName, TVI_ROOT, bProtocolExcluded, pItemData);
			if (hti) {
				int nStatusBits = CallProtoService(pAccount->szModuleName, PS_GETCAPS, PFLAGNUM_2, 0);
				int nStatusExcluded = CallProtoService(pAccount->szModuleName, PS_GETCAPS, PFLAGNUM_5, 0);
				pItemData = new CTreeItemData;
				pItemData->m_isProtocol = false;
				pItemData->m_pszModule = pAccount->szModuleName;
				pItemData->m_nStatus = ID_STATUS_OFFLINE;
				bool bStatusExcluded = g_plugin.IsProtocolStatusExcluded(pAccount->szModuleName, pItemData->m_nStatus);
				TreeInsert(TranslateW(g_aStatusCode[5].m_ptszStatusName), hti, bStatusExcluded, pItemData);
				for (size_t k = 0; k < _countof(g_aStatusCode); ++k) {
					const CMirandaStatus2SkypeStatus &m2s = g_aStatusCode[k];
					unsigned long statusFlags = Proto_Status2Flag(m2s.m_nMirandaStatus);
					if ((m2s.m_nMirandaStatus != ID_STATUS_OFFLINE) && (nStatusBits & statusFlags) && !(nStatusExcluded & statusFlags)) {
						pItemData = new CTreeItemData;
						pItemData->m_isProtocol = false;
						pItemData->m_pszModule = pAccount->szModuleName;
						pItemData->m_nStatus = m2s.m_nMirandaStatus;
						bStatusExcluded = g_plugin.IsProtocolStatusExcluded(pAccount->szModuleName, pItemData->m_nStatus);
						TreeInsert(TranslateW(m2s.m_ptszStatusName), hti, bStatusExcluded, pItemData);
					}
				}

				m_tree.Expand(hti, TVE_EXPAND);
			}
		}
		return true;
	}

	bool OnApply() override
	{
		SaveExclusion(TVI_ROOT);
		return true;
	}

	void OnDestroy() override
	{
		FreeMemory(TVI_ROOT);
	}

	void onItemChanged_Tree(CCtrlTreeView::TEventInfo *ev)
	{
		DisableChildren(ev->hItem, m_tree.GetItemState(ev->hItem, TVIS_STATEIMAGEMASK) >> 12);
	}
};

int SSC_OptInitialise(WPARAM wp, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 910000000;
	odp.szTitle.a = LPGEN("Change Skype status");
	odp.szGroup.a = LPGEN("Plugins");
	odp.pDialog = new CSettingsDlg();
	g_plugin.addOptions(wp, &odp);
	return 0;
}

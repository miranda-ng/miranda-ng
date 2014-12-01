#include "stdafx.h"
#include "resource.h"

/////////////////////////////////////////////////////////////////////////////////////////////

struct CTreeItemData
{
	enum EType
	{
		Protocol,
		Status
	};

	EType m_nType;
	char* m_pszModule;
	int m_nStatus;
};

enum ETreeCheckBoxState
{
	// tree check box state
	TCBS_NOSTATEBOX = 0,
	TCBS_UNCHECKED = 1,
	TCBS_CHECKED = 2,
	TCBS_DISABLE_UNCHECKED = 3,
	TCBS_DISABLE_CHECKED = 4,
};

HTREEITEM tree_insert_item(HWND hDlg, HWND hwndTree, TCHAR *pName, HTREEITEM htiParent, ETreeCheckBoxState nState, CTreeItemData *pData)
{
	TVINSERTSTRUCT tvi = { 0 };
	tvi.hParent = htiParent;
	tvi.hInsertAfter = TVI_LAST;
	tvi.item.mask = TVIF_TEXT|TVIF_PARAM|TVIF_STATE;
	tvi.item.pszText = pName;
	tvi.item.lParam = reinterpret_cast<LPARAM>(pData);
	tvi.item.stateMask = TVIS_STATEIMAGEMASK;
	tvi.item.state = INDEXTOSTATEIMAGEMASK(nState);
	return TreeView_InsertItem(hwndTree,&tvi);
}

void InitProtocolTree(HWND hDlg,HWND hwndTreeCtrl)
{
	int cAccounts = 0;
	PROTOACCOUNT** ppAccount;

	enum{OFFLINE_STATUS_INDEX = 5};

	ProtoEnumAccounts(&cAccounts, &ppAccount);
	for(int i = 0; i < cAccounts;++i)
	{
		PROTOACCOUNT* pAccount = ppAccount[i];
		CTreeItemData* pItemData = new CTreeItemData;
		pItemData->m_nType = CTreeItemData::Protocol;
		pItemData->m_pszModule = pAccount->szModuleName;

		bool bProtocolExcluded = g_Options.IsProtocolExcluded(pAccount->szModuleName);
		HTREEITEM hti = tree_insert_item(hDlg,hwndTreeCtrl,pAccount->tszAccountName,TVI_ROOT,((true == bProtocolExcluded) ? TCBS_CHECKED : TCBS_UNCHECKED),pItemData);
		if (hti)
		{
			int nStatusBits = CallProtoService(pAccount->szModuleName,PS_GETCAPS,PFLAGNUM_2,0);
			int nStatusExcluded = CallProtoService(pAccount->szModuleName,PS_GETCAPS,PFLAGNUM_5,0);
			pItemData = new CTreeItemData;
			pItemData->m_nType = CTreeItemData::Status;
			pItemData->m_pszModule = pAccount->szModuleName;
			pItemData->m_nStatus = ID_STATUS_OFFLINE;
			bool bStatusExcluded = g_Options.IsProtocolStatusExcluded(pAccount->szModuleName,pItemData->m_nStatus);
			ETreeCheckBoxState nState = TCBS_UNCHECKED;
			if (bProtocolExcluded) {
				if (bStatusExcluded)
					nState = TCBS_DISABLE_CHECKED;
				else
					nState = TCBS_DISABLE_UNCHECKED;
			}
			else {
				if (bStatusExcluded)
					nState = TCBS_CHECKED;
				else
					nState = TCBS_UNCHECKED;
			}
			tree_insert_item(hDlg,hwndTreeCtrl,TranslateTS(g_aStatusCode[OFFLINE_STATUS_INDEX].m_ptszStatusName),hti,nState,pItemData);
			for(size_t k = 0; k < SIZEOF(g_aStatusCode); ++k) {
				const CMirandaStatus2SkypeStatus& m2s = g_aStatusCode[k];
				unsigned long statusFlags = Proto_Status2Flag(m2s.m_nMirandaStatus);
				if ((m2s.m_nMirandaStatus != ID_STATUS_OFFLINE) && (nStatusBits & statusFlags) && !(nStatusExcluded & statusFlags)) {
					pItemData = new CTreeItemData;
					pItemData->m_nType = CTreeItemData::Status;
					pItemData->m_pszModule = pAccount->szModuleName;
					pItemData->m_nStatus = m2s.m_nMirandaStatus;
					bool bStatusExcluded = g_Options.IsProtocolStatusExcluded(pAccount->szModuleName,pItemData->m_nStatus);
					if (bProtocolExcluded) {
						if (bStatusExcluded)
							nState = TCBS_DISABLE_CHECKED;
						else
							nState = TCBS_DISABLE_UNCHECKED;
					}
					else {
						if (bStatusExcluded)
							nState = TCBS_CHECKED;
						else
							nState = TCBS_UNCHECKED;
					}

					tree_insert_item(hDlg,hwndTreeCtrl,TranslateTS(m2s.m_ptszStatusName),hti,nState,pItemData);
				}
			}

			TreeView_Expand(hwndTreeCtrl,hti,TVE_EXPAND);
		}
	}
}

inline HTREEITEM tree_get_child_item(HWND hwndTreeCtrl,HTREEITEM hti)
{
	return TreeView_GetChild(hwndTreeCtrl,hti);
}

inline HTREEITEM tree_get_next_sibling_item(HWND hwndTreeCtrl,HTREEITEM hti)
{
	return TreeView_GetNextSibling(hwndTreeCtrl,hti);
}

const CTreeItemData* get_item_data(HWND hwndTreeCtrl,HTREEITEM hti)
{
	TVITEM tvi = {0};
	tvi.hItem = hti;
	tvi.mask = TVIF_PARAM|TVIF_HANDLE;
	if (TRUE == ::SendMessage(hwndTreeCtrl,TVM_GETITEM,0,reinterpret_cast<LPARAM>(&tvi))) {
		CTreeItemData* pData = reinterpret_cast<CTreeItemData*>(tvi.lParam);
		return pData;
	}
	return NULL;
}


inline ETreeCheckBoxState tree_get_state_image(HWND hwndTree,HTREEITEM hti)
{
	TVITEM tvi;
	tvi.hItem = hti;
	tvi.mask = TVIF_STATE|TVIF_HANDLE;
	tvi.stateMask = TVIS_STATEIMAGEMASK;
	if (TRUE == ::SendMessage(hwndTree,TVM_GETITEM,0,reinterpret_cast<LPARAM>(&tvi)))
	{
		UINT nState = (tvi.state >> 12);
		return static_cast<ETreeCheckBoxState>(nState);
	}

	_ASSERT(!"we should never get here!");
	return TCBS_UNCHECKED;
}

void FreeMemory(HWND hwndTreeCtrl,HTREEITEM hti)
{
	for(HTREEITEM h = tree_get_child_item(hwndTreeCtrl,hti);h;h = tree_get_next_sibling_item(hwndTreeCtrl,h)) {
		FreeMemory(hwndTreeCtrl,h);
		const CTreeItemData* pData = get_item_data(hwndTreeCtrl,h);
		if (pData)
			delete pData;
	}
}

bool tree_set_item_state(HWND hwndTree,HTREEITEM hti,ETreeCheckBoxState nState)
{
	TVITEM tvi;
	memset(&tvi, 0, sizeof(tvi));

	tvi.mask = TVIF_STATE|TVIF_HANDLE;
	tvi.hItem = hti;

	tvi.stateMask = TVIS_STATEIMAGEMASK;
	tvi.state = INDEXTOSTATEIMAGEMASK(nState);

	return TRUE == ::SendMessage(hwndTree,TVM_SETITEM,0,reinterpret_cast<LPARAM>(&tvi));
}

void disable_children(HWND hwndTree,HTREEITEM htiParent,bool bDisable)
{
	for(HTREEITEM hti = tree_get_child_item(hwndTree,htiParent);hti;hti = tree_get_next_sibling_item(hwndTree,hti)) {
		ETreeCheckBoxState nState = tree_get_state_image(hwndTree,hti);
		if (bDisable) {
			if (TCBS_CHECKED == nState)
				nState = TCBS_DISABLE_CHECKED;
			else if (TCBS_UNCHECKED == nState)
				nState = TCBS_DISABLE_UNCHECKED;
		}
		else {
			if (TCBS_DISABLE_CHECKED == nState)
				nState = TCBS_CHECKED;
			else if (TCBS_DISABLE_UNCHECKED == nState)
				nState = TCBS_UNCHECKED;
		}
		tree_set_item_state(hwndTree,hti,nState);
	}
}

void save_exclusion_list(HWND hwndTree,HTREEITEM htiParent)
{
	for(HTREEITEM hti = tree_get_child_item(hwndTree,htiParent);hti;hti = tree_get_next_sibling_item(hwndTree,hti)) {
		const CTreeItemData* pData = get_item_data(hwndTree,hti);
		ETreeCheckBoxState nState = tree_get_state_image(hwndTree,hti);
		if (CTreeItemData::Protocol == pData->m_nType) {
			g_Options.ExcludeProtocol(pData->m_pszModule,TCBS_CHECKED == nState);
			save_exclusion_list(hwndTree,hti);
		}
		else g_Options.ExcludeProtocolStatus(pData->m_pszModule,pData->m_nStatus,((TCBS_CHECKED == nState) || (TCBS_DISABLE_CHECKED == nState)));
	}			
}

class CImageListWrapper
{
public:
	CImageListWrapper()
		: m_hImageList(ImageList_LoadImage(g_hModule,MAKEINTRESOURCE(IDB_TREE_STATE),16,0,RGB(255,255,255),IMAGE_BITMAP,LR_DEFAULTCOLOR))
	{
	}

	~CImageListWrapper()
	{
		if (m_hImageList)
			ImageList_Destroy(m_hImageList);
	}

	operator HIMAGELIST()const
	{
		return m_hImageList;
	}

private:
	HIMAGELIST m_hImageList;
};

HIMAGELIST get_image_list()
{
	static CImageListWrapper wrapper;
	return wrapper;
}

static INT_PTR CALLBACK SettingsDlgProc(HWND hdlg,UINT msg,WPARAM wp,LPARAM lp)
{
	switch(msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);
		{
			HWND hwndTreeCtrl = GetDlgItem(hdlg,IDC_TREE_PROTOCOLS);
			HIMAGELIST hImage = get_image_list();
			if (hImage)
				TreeView_SetImageList(hwndTreeCtrl,hImage,TVSIL_STATE);
			InitProtocolTree(hdlg,hwndTreeCtrl);
		}
		CheckDlgButton(hdlg,IDC_CHECK_SYNCK_STATUS_MSG,(true == g_Options.GetSyncStatusMsgFlag()) ? 1 : 0);
		CheckDlgButton(hdlg,IDC_CHECK_STATUSES,(true == g_Options.GetSyncStatusStateFlag()) ? 1 : 0);				
		return TRUE;

	case WM_NOTIFY:
		{
			LPNMHDR pNMHDR = reinterpret_cast<LPNMHDR>(lp);
			switch(pNMHDR->code) {
			case PSN_APPLY:
				save_exclusion_list(GetDlgItem(hdlg,IDC_TREE_PROTOCOLS),TVI_ROOT);
				g_Options.SetSyncStatusMsgFlag(1 == IsDlgButtonChecked(hdlg,IDC_CHECK_SYNCK_STATUS_MSG));
				g_Options.SetSyncStatusStateFlag(1 == IsDlgButtonChecked(hdlg,IDC_CHECK_STATUSES));
				break;

			case NM_CLICK:
				if (IDC_TREE_PROTOCOLS == wp) {
					DWORD pos = ::GetMessagePos();

					HWND hwndTree = ::GetDlgItem(hdlg,IDC_TREE_PROTOCOLS);

					TVHITTESTINFO tvhti;
					tvhti.pt.x = LOWORD(pos);
					tvhti.pt.y = HIWORD(pos);
					::ScreenToClient(hwndTree,&(tvhti.pt));

					HTREEITEM hti = reinterpret_cast<HTREEITEM>(::SendMessage(hwndTree,TVM_HITTEST,0,reinterpret_cast<LPARAM>(&tvhti)));
					if (hti && (tvhti.flags&(TVHT_ONITEMSTATEICON|TVHT_ONITEMICON))) {
						ETreeCheckBoxState nState = tree_get_state_image(hwndTree,hti);
						if (TCBS_CHECKED == nState || TCBS_UNCHECKED == nState) {
							if (TCBS_CHECKED == nState)
								nState = TCBS_UNCHECKED;
							else
								nState = TCBS_CHECKED;

							tree_set_item_state(hwndTree,hti,nState);
							disable_children(hwndTree,hti,TCBS_CHECKED == nState);
							PropSheet_Changed(::GetParent(hdlg),hdlg);
						}
					}
				}
			}
		}
		break;

	case WM_COMMAND:
		if (BN_CLICKED == HIWORD(wp) && ((IDC_CHECK_SYNCK_STATUS_MSG == LOWORD(wp)) || (IDC_CHECK_STATUSES == LOWORD(wp))))
			PropSheet_Changed(::GetParent(hdlg),hdlg);
		break;

	case WM_DESTROY:
		FreeMemory(GetDlgItem(hdlg,IDC_TREE_PROTOCOLS),TVI_ROOT);
		break;
	}
	return FALSE;
}

int SSC_OptInitialise(WPARAM wp, LPARAM lp)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 910000000;
	odp.hInstance = g_hModule;
	odp.pszTitle = LPGEN("Change Skype status");
	odp.pszGroup = LPGEN("Plugins");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_DIALOG_SETTINGS);
	odp.pfnDlgProc = SettingsDlgProc;
	Options_AddPage(wp, &odp);
	return 0;
}

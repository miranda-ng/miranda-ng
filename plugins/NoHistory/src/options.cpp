#include "stdafx.h"

class CDlgOptionsDlg : public CDlgBase
{
	HANDLE hItemAll, hItemNew;

	void SetListGroupIcons(HANDLE hFirstItem, HANDLE hParentItem, int *groupChildCount = nullptr)
	{
		int iconOn = 1;
		int childCount = 0;

		int typeOfFirst = clist.GetItemType(hFirstItem);

		// check groups
		HANDLE hItem = (typeOfFirst == CLCIT_GROUP) ? hFirstItem : clist.GetNextItem(hFirstItem, CLGN_NEXTGROUP);
		while (hItem) {
			HANDLE hChildItem = clist.GetNextItem(hItem, CLGN_CHILD);
			if (hChildItem)
				SetListGroupIcons(hChildItem, hItem, &childCount);
			if (iconOn && clist.GetExtraImage(hItem, 0) == 0)
				iconOn = 0;
			hItem = clist.GetNextItem(hItem, CLGN_NEXTGROUP);
		}

		// check contacts
		hItem = (typeOfFirst == CLCIT_CONTACT) ? hFirstItem : clist.GetNextItem(hFirstItem, CLGN_NEXTCONTACT);
		while (hItem) {
			int iImage = clist.GetExtraImage(hItem, 0);
			if (iconOn && iImage == 0)
				iconOn = 0;
			if (iImage != EMPTY_EXTRA_ICON)
				childCount++;
			hItem = clist.GetNextItem(hItem, CLGN_NEXTCONTACT);
		}

		// set icons
		clist.SetExtraImage(hParentItem, 0, childCount ? (iconOn ? 1 : 0) : EMPTY_EXTRA_ICON);
		if (groupChildCount)
			*groupChildCount += childCount;
	}

	void SetAllChildIcons(HANDLE hFirstItem, int iColumn, int iImage)
	{
		int typeOfFirst = clist.GetItemType(hFirstItem);
		// check groups
		HANDLE hItem = (typeOfFirst == CLCIT_GROUP) ? hFirstItem : clist.GetNextItem(hFirstItem, CLGN_NEXTGROUP);
		while (hItem) {
			HANDLE hChildItem = clist.GetNextItem(hItem, CLGN_CHILD);
			if (hChildItem)
				SetAllChildIcons(hChildItem, iColumn, iImage);
			hItem = clist.GetNextItem(hItem, CLGN_NEXTGROUP);
		}
		
		// check contacts
		hItem = (typeOfFirst == CLCIT_CONTACT) ? hFirstItem : clist.GetNextItem(hFirstItem, CLGN_NEXTCONTACT);
		while (hItem) {
			int iOldIcon = clist.GetExtraImage(hItem, iColumn);
			if (iOldIcon != EMPTY_EXTRA_ICON && iOldIcon != iImage)
				clist.SetExtraImage(hItem, iColumn, iImage);
			hItem = clist.GetNextItem(hItem, CLGN_NEXTCONTACT);
		}
	}

	void SetAllContactIcons()
	{
		for (auto &hContact : Contacts()) {
			if (!Contact::IsGroupChat(hContact)) {
				HANDLE hItem = clist.FindContact(hContact);
				if (hItem) {
					bool disabled = (g_plugin.getByte(hContact, DBSETTING_REMOVE) == 1);
					clist.SetExtraImage(hItem, 0, disabled ? 1 : 0);
				}
			}
		}
	}

	CCtrlClc clist;
	CCtrlCheck chkOnStartup;

public:
	CDlgOptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT),
		clist(this, IDC_LIST),
		chkOnStartup(this, IDC_ONSTARTUP)
	{
		CreateLink(chkOnStartup, g_plugin.bWipeOnStartup);

		clist.OnNewContact = clist.OnListRebuilt = Callback(this, &CDlgOptionsDlg::onListRebuilt);
		clist.OnOptionsChanged = Callback(this, &CDlgOptionsDlg::onOptionsChanged);
		clist.OnContactMoved = Callback(this, &CDlgOptionsDlg::onContactMoved);
		clist.OnClick = Callback(this, &CDlgOptionsDlg::onClick);
	}

	bool OnInitDialog() override
	{
		HIMAGELIST hIml = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 2, 2);
		g_plugin.addImgListIcon(hIml, IDI_HKEEP);
		g_plugin.addImgListIcon(hIml, IDI_HREMOVE);
		clist.SetExtraImageList(hIml);

		SendDlgItemMessage(m_hwnd, IDC_PIC_KEEP, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_HKEEP), 0);
		SendDlgItemMessage(m_hwnd, IDC_PIC_REMOVE, STM_SETICON, (WPARAM)g_plugin.getIcon(IDI_HREMOVE), 0);

		onOptionsChanged(0);
		clist.SetExtraColumns(1);

		CLCINFOITEM cii = { sizeof(cii) };
		cii.flags = CLCIIF_GROUPFONT;

		cii.pszText = TranslateT("** All contacts **");
		hItemAll = clist.AddInfoItem(&cii);

		cii.pszText = TranslateT("** New contacts **");
		hItemNew = clist.AddInfoItem(&cii);
		clist.SetExtraImage(hItemNew, 0, g_plugin.bEnabledForNew);

		onListRebuilt(0);
		SetFocus(clist.GetHwnd());

		SendDlgItemMessage(m_hwnd, IDC_SPN_TIMEOUT, UDM_SETRANGE, 0, (LPARAM)MAKELONG(60 * 60, 1));
		return true;
	}

	bool OnApply() override
	{
		g_plugin.bEnabledForNew = clist.GetExtraImage(hItemNew, 0);

		for (auto &hContact : Contacts()) {
			if (!Contact::IsGroupChat(hContact)) {
				HANDLE hItem = clist.FindContact(hContact);
				if (hItem) {
					int iImage = clist.GetExtraImage(hItem, 0);
					g_plugin.setByte(hContact, DBSETTING_REMOVE, iImage == 1);
				}
			}
		}
		return true;
	}

	void OnDestroy() override
	{
		ImageList_Destroy(clist.GetExtraImageList());
	}

	void onListRebuilt(CCtrlClc::TEventInfo *)
	{
		SetAllContactIcons();
		SetListGroupIcons(clist.GetNextItem(CLGN_ROOT, 0), hItemAll);
	}

	void onOptionsChanged(CCtrlClc::TEventInfo *)
	{
		SetWindowLongPtr(clist.GetHwnd(), GWL_STYLE, GetWindowLongPtr(clist.GetHwnd(), GWL_STYLE) | CLS_SHOWHIDDEN | CLS_HIDEEMPTYGROUPS);
	}

	void onContactMoved(CCtrlClc::TEventInfo *)
	{
		SetListGroupIcons(clist.GetNextItem(CLGN_ROOT, 0), hItemAll);
	}

	void onClick(CCtrlClc::TEventInfo *ev)
	{
		// Make sure we have an extra column
		NMCLISTCONTROL *nm = ev->info;
		if (nm->iColumn == -1)
			return;

		// Find clicked item
		uint32_t hitFlags;
		HANDLE hItem = clist.HitTest(nm->pt.x, nm->pt.y, &hitFlags);
		if (hItem == nullptr || !(hitFlags & CLCHT_ONITEMEXTRA))
			return;

		// Get image in clicked column
		int iImage = clist.GetExtraImage(hItem, 0);
		if (iImage == 0)
			iImage = 0 + 1;
		else if (iImage == 1)
			iImage = 0;

		// Update list, making sure that the options are mutually exclusive
		switch (clist.GetItemType(hItem)) {
		case CLCIT_CONTACT:
			clist.SetExtraImage(hItem, 0, iImage);
			break;
		case CLCIT_INFO:
			if (hItem == hItemAll)
				SetAllChildIcons(hItem, 0, iImage);
			else
				clist.SetExtraImage(hItem, 0, iImage);
			break;
		case CLCIT_GROUP:
			hItem = clist.GetNextItem(hItem, CLGN_CHILD);
			if (hItem)
				SetAllChildIcons(hItem, 0, iImage);
		}
		
		// Update the all/none icons
		SetListGroupIcons(clist.GetNextItem(CLGN_ROOT, 0), hItemAll);

		// Activate Apply button
		NotifyChange();
	}
};

int OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.position = -790000000;
	odp.szGroup.a = LPGEN("History");
	odp.szTitle.a = LPGEN("No History");
	odp.szTab.a = LPGEN("Logging");
	odp.pDialog = new CDlgOptionsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

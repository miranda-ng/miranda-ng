#include "stdafx.h"

class COptionsDlg : public CDlgBase
{
	CCtrlCheck chkExpand, chkRestore, chkWarnDelete;

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPTIONS),
		chkExpand(this, IDC_EXPANDSETTINGS),
		chkRestore(this, IDC_RESTORESETTINGS),
		chkWarnDelete(this, IDC_WARNONDEL)
	{
		CreateLink(chkExpand, g_plugin.bExpandSettingsOnOpen);
		CreateLink(chkRestore, g_plugin.bRestoreOnOpen);
		CreateLink(chkWarnDelete, g_plugin.bWarnOnDelete);
	}
};

class CPopupOptionsDlg : public CDlgBase
{
	CCtrlEdit  edtTimeout;
	CCtrlCheck chkUsePopups;
	CCtrlColor clrBack, clrText;

public:
	CPopupOptionsDlg() :
		CDlgBase(g_plugin, IDD_POPUP_OPTS),
		clrBack(this, IDC_COLOUR),
		clrText(this, IDC_TXT_COLOUR),
		edtTimeout(this, IDC_POPUPTIMEOUT),
		chkUsePopups(this, IDC_POPUPS)
	{
		CreateLink(clrBack, g_plugin.iPopupBkColor);
		CreateLink(clrText, g_plugin.iPopupTxtColor);
		CreateLink(edtTimeout, g_plugin.iPopupDelay);
	}

	bool OnInitDialog() override
	{
		chkUsePopups.SetState(g_bUsePopups);
		return true;
	}

	bool OnApply() override
	{
		g_plugin.setByte("UsePopUps", g_bUsePopups = chkUsePopups.GetState());
		return true;
	}
};

INT OptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szTitle.a = modFullname;

	odp.szGroup.a = LPGEN("Database");
	odp.pDialog = new COptionsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szGroup.a = LPGEN("Popups");
	odp.pDialog = new CPopupOptionsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

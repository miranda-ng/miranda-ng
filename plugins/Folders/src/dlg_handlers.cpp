#include "stdafx.h"

PFolderItem lastItem = nullptr;

CDlgBase *pHelpDialog = nullptr;

class CHelpDialog : public CDlgBase
{
	CCtrlButton m_btnClose;
	CCtrlRichEdit m_redtHelp;

public:
	CHelpDialog() :
		CDlgBase(g_plugin, IDD_VARIABLES_HELP),
		m_btnClose(this, IDCLOSE),
		m_redtHelp(this, IDC_HELP_RICHEDIT)
	{
		m_btnClose.OnClick = Callback(this, &CHelpDialog::OnCloseClick);
	}

	bool OnInitDialog() override
	{
		wchar_t tszMessage[2048];
		mir_snwprintf(tszMessage, L"%s\r\n%s\r\n\r\n%s\t\t%s\r\n%%miranda_path%%\t\t%s\r\n%%profile_path%%\t\t%s\r\n\t\t\t%s\r\n%%current_profile%%\t\t%s\r\n\t\t\t%s\r\n\r\n\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n%s\r\n\r\n%s\r\n%s\r\n%s\r\n%%miranda_path%%\t\t\t%s\r\n%%profile_path%%\t\t\t%s\r\n%%current_profile%%\t\t\t%s\r\n%%temp%%\t\t\t\t%s\r\n%%profile_path%%\\%%current_profile%%\t%s\r\n%%miranda_path%%\\plugins\\config\t%s\r\n'   %%miranda_path%%\\\\\\\\     '\t\t%s\r\n\r\n%s",
			TranslateT("Don't forget to click on Apply to save the changes. If you don't then the changes won't"),
			TranslateT("be saved to the database, they will only be valid for this session."),
			TranslateT("Variable string"),
			TranslateT("What it expands to:"),
			TranslateT("Expands to your Miranda path (e.g., c:\\program files\\miranda ng)."),
			TranslateT("Expands to your profile path - the value found in mirandaboot.ini,"),
			TranslateT("ProfileDir section (usually inside Miranda's folder)."),
			TranslateT("Expands to your current profile name without the extension."),
			TranslateT("(e.g., default if your your profile is default.dat)."),
			TranslateT("Environment variables"),
			TranslateT("The plugin can also expand environment variables; the variables are specified like in any other"),
			TranslateT("program that can use environment variables, i.e., %<env variable>%."),
			TranslateT("Note: Environment variables are expanded before any Miranda variables. So if you have, for"),
			TranslateT("example, %profile_path% defined as a system variable then it will be expanded to that value"),
			TranslateT("instead of expanding to Miranda's profile path."),
			TranslateT("Examples:"),
			TranslateT("If the value for the ProfileDir inside mirandaboot.ini, ProfileDir section is '.\\profiles\\', current"),
			TranslateT("profile is 'default.dat' and Miranda path is 'c:\\program files\\miranda ng\\' then:"),
			TranslateT("will expand to 'c:\\program files\\miranda ng'"),
			TranslateT("will expand to 'c:\\program files\\miranda ng\\profiles'"),
			TranslateT("will expand to 'default'"),
			TranslateT("will expand to the temp folder of the current user."),
			TranslateT("will expand to 'c:\\program files\\miranda ng\\profiles\\default'"),
			TranslateT("will expand to 'c:\\program files\\miranda ng\\plugins\\config'"),
			TranslateT("will expand to 'c:\\program files\\miranda ng'"),
			TranslateT("Notice that the spaces at the beginning and the end of the string are trimmed, as well as the last."));
		m_redtHelp.SetText(tszMessage);
		return true;
	}

	void OnDestroy() override
	{
		if (pHelpDialog == this)
			pHelpDialog = nullptr;
	}

	void OnCloseClick(CCtrlBase*)
	{
		Close();
	}
};

class COptDialog : public CDlgBase
{
	CCtrlListBox m_lbSections, m_lbItems;
	CCtrlEdit m_edtPreview, m_edtEdit;
	CCtrlButton m_btnRefresh, m_btnHelp;

	PFolderItem GetSelectedItem()
	{
		int index = m_lbItems.GetCurSel();
		if (index == LB_ERR)
			return nullptr;

		return (PFolderItem)m_lbItems.GetItemData(index);
	}

	int ContainsSection(const wchar_t *section)
	{
		int index = m_lbSections.SendMsg(LB_FINDSTRINGEXACT, -1, (LPARAM)section);
		return (index != LB_ERR);
	}

	void LoadRegisteredFolderSections()
	{
		for (auto &it : lstRegisteredFolders) {
			wchar_t *translated = mir_a2u(it->GetSection());
			if (!ContainsSection(TranslateW(translated))) {
				int idx = m_lbSections.AddString(TranslateW(translated), 0);
				m_lbSections.SetItemData(idx, (LPARAM)it->GetSection());
			}
			mir_free(translated);
		}
	}

	void LoadRegisteredFolderItems()
	{
		int idx = m_lbSections.GetCurSel();
		if (idx == LB_ERR)
			return;

		char* szSection = (char*)m_lbSections.GetItemData(idx);

		m_lbItems.ResetContent();
		for (auto &it : lstRegisteredFolders) {
			if (!mir_strcmp(szSection, it->GetSection())) {
				idx = m_lbItems.AddString(TranslateW(it->GetUserName()), 0);
				m_lbItems.SetItemData(idx, (LPARAM)it);
			}
		}
		m_lbItems.SetCurSel(0); //select the first item
		OnItemsSelChange(0); //tell the dialog to refresh the preview
	}

	void RefreshPreview()
	{
		wchar_t tmp[MAX_FOLDER_SIZE];
		m_edtEdit.GetText(tmp, _countof(tmp));
		m_edtPreview.SetText(ExpandPath(tmp));
	}

	void LoadItem(PFolderItem item)
	{
		if (!item)
			return;

		m_edtEdit.SetText(item->GetFormat());
		RefreshPreview();
	}

	void SaveItem(PFolderItem item, int bEnableApply)
	{
		if (!item)
			return;

		wchar_t buffer[MAX_FOLDER_SIZE];
		m_edtEdit.GetText(buffer, _countof(buffer));
		item->SetFormat(buffer);

		if (bEnableApply)
			NotifyChange();
	}

	int ChangesNotSaved(PFolderItem item)
	{
		if (!item)
			return 0;

		wchar_t buffer[MAX_FOLDER_SIZE];
		m_edtEdit.GetText(buffer, _countof(buffer));
		return mir_wstrcmp(item->GetFormat(), buffer) != 0;
	}

	void CheckForChanges(int bNeedConfirmation = 1)
	{
		if (ChangesNotSaved(lastItem))
			if ((!bNeedConfirmation) || MessageBox(m_hwnd, TranslateT("Some changes weren't saved. Apply the changes now?"), TranslateT("Changes not saved"), MB_YESNO | MB_ICONINFORMATION) == IDYES)
				SaveItem(lastItem, TRUE);
	}

public:
	COptDialog() :
		CDlgBase(g_plugin, IDD_OPT_FOLDERS),
		m_lbSections(this, IDC_FOLDERS_SECTIONS_LIST),
		m_lbItems(this, IDC_FOLDERS_ITEMS_LIST),
		m_edtPreview(this, IDC_PREVIEW_EDIT),
		m_edtEdit(this, IDC_FOLDER_EDIT),
		m_btnRefresh(this, IDC_REFRESH_BUTTON),
		m_btnHelp(this, IDC_HELP_BUTTON)
	{
		m_edtEdit.OnChange = Callback(this, &COptDialog::OnEditChange);
		m_btnRefresh.OnClick = Callback(this, &COptDialog::OnRefreshClick);
		m_btnHelp.OnClick = Callback(this, &COptDialog::OnHelpClick);
		m_lbSections.OnSelChange = Callback(this, &COptDialog::OnSectionsSelChange);
		m_lbItems.OnSelChange = Callback(this, &COptDialog::OnItemsSelChange);
	}

	bool OnInitDialog() override
	{
		lastItem = nullptr;
		LoadRegisteredFolderSections();
		return true;
	}

	void OnEditChange(CCtrlBase*)
	{
		RefreshPreview();
	}

	void OnRefreshClick(CCtrlBase*)
	{
		RefreshPreview();
	}

	void OnHelpClick(CCtrlBase*)
	{
		if (pHelpDialog == nullptr) {
			pHelpDialog = new CHelpDialog();
			pHelpDialog->Show();
		}
		else {
			SetForegroundWindow(pHelpDialog->GetHwnd());
			SetFocus(pHelpDialog->GetHwnd());
		}
	}

	void OnSectionsSelChange(CCtrlBase*)
	{
		CheckForChanges();
		LoadRegisteredFolderItems();
	}

	void OnItemsSelChange(CCtrlBase*)
	{
		PFolderItem item = GetSelectedItem();
		if (item != nullptr) {
			CheckForChanges();
			LoadItem(item);
		}
		lastItem = item;
	}

	bool OnApply() override
	{
		PFolderItem item = GetSelectedItem();
		if (item) {
			SaveItem(item, FALSE);
			LoadItem(item);
		}

		for (auto &it : lstRegisteredFolders)
			it->Save();
		CallPathChangedEvents();
		return true;
	}
};

int OnOptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szTitle.a = LPGEN("Folders");
	odp.szGroup.a = LPGEN("Customize");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pDialog = new COptDialog;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

#include "stdafx.h"

// Option dialog

class COptionsDlg : public CDlgBase
{
	TemplateInfo *m_curr = 0;

	void UpdatePreview(CCtrlButton*)
	{
		replaceStrW(m_curr->tmpValue, m_edit.GetText());

		HistoryArray::ItemData item;
		item.hContact = db_find_first();
		while (item.hContact && !item.hEvent) {
			item.hEvent = db_event_first(item.hContact);
			if (!item.hEvent)
				item.hContact = db_find_next(item.hContact);
		}

		if (item.hContact && item.hEvent) {
			item.load(ELM_DATA);
			
			ptrW wszText(TplFormatStringEx(int(m_curr-templates), m_curr->tmpValue, item.hContact, &item));
			preview.SetText(wszText);
			gpreview.SetText(wszText);
		}
		else {
			preview.SetText(L"");
			gpreview.SetText(L"");
		}
	}

	CCtrlBase preview, gpreview;
	CCtrlEdit m_edit;
	CCtrlMButton btnDiscard, btnPreview, bthVarHelp;
	CCtrlTreeView m_tree;

public:
	COptionsDlg() :
		CDlgBase(g_plugin, IDD_OPT_TEMPLATES),
		m_edit(this, IDC_EDITTEMPLATE),
		m_tree(this, IDC_TEMPLATES),
		preview(this, IDC_PREVIEW),
		gpreview(this, IDC_GPREVIEW),
		btnDiscard(this, IDC_DISCARD, g_plugin.getIcon(ICO_RESET), LPGEN("Cancel edit")),
		bthVarHelp(this, IDC_VARHELP, g_plugin.getIcon(ICO_VARHELP), LPGEN("Help on variables")),
		btnPreview(this, IDC_UPDATEPREVIEW, g_plugin.getIcon(ICO_PREVIEW), LPGEN("Update preview"))
	{
		btnDiscard.OnClick = Callback(this, &COptionsDlg::onClick_Discard);
		btnPreview.OnClick = Callback(this, &COptionsDlg::UpdatePreview);
		bthVarHelp.OnClick = Callback(this, &COptionsDlg::onVarHelp);

		m_tree.OnSelChanged = Callback(this, &COptionsDlg::onSelChanged);
	}

	bool OnInitDialog() override
	{
		HIMAGELIST himgTree = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 1, 1);
		m_tree.SetImageList(himgTree, TVSIL_NORMAL);

		ImageList_AddIcon(himgTree, g_plugin.getIcon(ICO_TPLGROUP));

		HTREEITEM hGroup = 0, hFirst = 0;
		const wchar_t *pwszPrevGroup = nullptr;
		for (auto &it : templates) {
			if (mir_wstrcmp(it.group, pwszPrevGroup)) {
				if (hGroup)
					m_tree.Expand(hGroup, TVE_EXPAND);

				TVINSERTSTRUCT tvis = {};
				tvis.hInsertAfter = TVI_LAST;
				tvis.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
				tvis.item.state = tvis.item.stateMask = TVIS_BOLD;
				tvis.item.pszText = it.group;
				tvis.item.lParam = 0;
				hGroup = m_tree.InsertItem(&tvis);

				if (!hFirst)
					hFirst = hGroup;
			}

			TVINSERTSTRUCT tvis;
			tvis.hParent = hGroup;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tvis.item.pszText = it.title;
			tvis.item.iSelectedImage = tvis.item.iImage = ImageList_AddIcon(himgTree, g_plugin.getIcon(it.icon));
			tvis.item.lParam = (LPARAM)&it;
			m_tree.InsertItem(&tvis);

			pwszPrevGroup = it.group;
		}

		if (hGroup)
			m_tree.Expand(hGroup, TVE_EXPAND);

		m_tree.SelectItem(hFirst);
		m_tree.EnsureVisible(hFirst);
		return true;
	}

	bool OnApply() override
	{
		if (m_curr != nullptr)
			replaceStrW(m_curr->tmpValue, m_edit.GetText());

		for (auto &it : templates) {
			if (it.tmpValue) {
				replaceStrW(it.value, it.tmpValue);
				it.tmpValue = nullptr;
			}
		}
		SaveTemplates();
		return true;
	}

	void OnDestroy() override
	{
		for (auto &it : templates)
			replaceStrW(it.tmpValue, nullptr);
	}

	void onClick_Discard(CCtrlButton *)
	{
		replaceStrW(m_curr->tmpValue, 0);

		if (m_curr->value)
			m_edit.SetText(m_curr->value);
		else
			m_edit.SetText(m_curr->defvalue);

		UpdatePreview(0);
	}

	void onVarHelp(CCtrlButton*)
	{
		CMStringW wszVarHelp;
		wszVarHelp.Format(L"%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s\n%s - %s",
			L"%", TranslateT("simply % character"),
			L"%n", TranslateT("line break"),
			L"%S", TranslateT("my nick"),
			L"%N", TranslateT("buddy\'s nick"),
			L"%c", TranslateT("event count"),
			L"%I", TranslateT("icon"),
			L"%i", TranslateT("direction icon"),
			L"%D", TranslateT("direction symbol"),
			L"%t", TranslateT("timestamp"),
			L"%h", TranslateT("hour (24 hour format, 0-23)"),
			L"%a", TranslateT("hour (12 hour format)"),
			L"%m", TranslateT("minute"),
			L"%s", TranslateT("second"),
			L"%o", TranslateT("month"),
			L"%d", TranslateT("day of month"),
			L"%y", TranslateT("year"),
			L"%w", TranslateT("day of week (Sunday, Monday... translatable)"),
			L"%p", TranslateT("am/pm symbol"),
			L"%O", TranslateT("name of month, translatable"),
			L"%M", TranslateT("the message string itself"));
		MessageBox(m_hwnd, wszVarHelp, TranslateT("Variables help"), MB_OK);
	}

	void onSelChanged(CCtrlTreeView::TEventInfo*)
	{
		TVITEMEX tvi;
		tvi.hItem = m_tree.GetSelection();
		tvi.mask = TVIF_PARAM;
		m_tree.GetItem(&tvi);

		if (tvi.lParam == 0) {
			m_edit.Disable();
			preview.Disable();
			gpreview.Disable();
			btnDiscard.Disable();
			btnPreview.Disable();
			bthVarHelp.Disable();

			HTREEITEM hItem = m_tree.GetChild(tvi.hItem);
			if (hItem) {
				m_tree.Expand(tvi.hItem, TVE_EXPAND);
				m_tree.SelectItem(hItem);
			}
			return;
		}

		m_edit.Enable();
		preview.Enable();
		gpreview.Enable();
		btnDiscard.Enable();
		btnPreview.Enable();
		bthVarHelp.Enable();

		if (m_curr != nullptr)
			replaceStrW(m_curr->tmpValue, m_edit.GetText());

		m_curr = (TemplateInfo*)tvi.lParam;

		if (m_curr->tmpValue)
			m_edit.SetText(m_curr->tmpValue);
		else if (m_curr->value)
			m_edit.SetText(m_curr->value);
		else
			m_edit.SetText(m_curr->defvalue);

		UpdatePreview(0);
	}
};

int OptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.szGroup.a = LPGEN("History");
	odp.szTitle.a = LPGEN("NewStory");
	odp.flags = ODPF_BOLDGROUPS;

	odp.szTab.a = LPGEN("Templates");
	odp.pDialog = new COptionsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

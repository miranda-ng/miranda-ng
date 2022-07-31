#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// General options dialog

class CGeneralOptsDlg : public CDlgBase
{
	CCtrlCheck chkGrouping, chkVScroll, chkDrawEdge;

public:
	CGeneralOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_ADVANCED),
		chkVScroll(this, IDC_VSCROLL),
		chkDrawEdge(this, IDC_DRAWEDGE),
		chkGrouping(this, IDC_GROUPING)
	{
		CreateLink(chkVScroll, g_plugin.bOptVScroll);
		CreateLink(chkGrouping, g_bOptGrouping);
		CreateLink(chkDrawEdge, g_bOptDrawEdge);
	}

	bool OnApply() override
	{
		g_plugin.bDrawEdge = g_bOptDrawEdge;
		g_plugin.bMsgGrouping = g_bOptGrouping;
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Template options dialog

class CTemplateOptsDlg : public CDlgBase
{
	MCONTACT m_hContact;
	MEVENT m_hDbEVent;
	TemplateInfo *m_curr = 0;

	void UpdatePreview(CCtrlButton*)
	{
		replaceStrW(m_curr->tmpValue, m_edit.GetText());

		ItemData item;
		item.hContact = m_hContact;
		item.hEvent = m_hDbEVent;
		item.load(true);
			
		ptrW wszText(TplFormatStringEx(int(m_curr-templates), m_curr->tmpValue, item.hContact, &item));
		preview.SetText(wszText);
		gpreview.SetText(wszText);
	}

	CCtrlBase preview, gpreview;
	CCtrlEdit m_edit;
	CCtrlMButton btnDiscard, btnPreview, bthVarHelp, btnReset;
	CCtrlTreeView m_tree;

public:
	CTemplateOptsDlg() :
		CDlgBase(g_plugin, IDD_OPT_TEMPLATES),
		m_edit(this, IDC_EDITTEMPLATE),
		m_tree(this, IDC_TEMPLATES),
		preview(this, IDC_PREVIEW),
		gpreview(this, IDC_GPREVIEW),
		btnReset(this, IDC_RESET, Skin_LoadIcon(SKINICON_OTHER_UNDO), LPGEN("Reset to default")),
		btnDiscard(this, IDC_DISCARD, g_plugin.getIcon(ICO_RESET), LPGEN("Cancel edit")),
		bthVarHelp(this, IDC_VARHELP, g_plugin.getIcon(ICO_VARHELP), LPGEN("Variables help")),
		btnPreview(this, IDC_UPDATEPREVIEW, g_plugin.getIcon(ICO_PREVIEW), LPGEN("Update preview"))
	{
		btnReset.OnClick = Callback(this, &CTemplateOptsDlg::onClick_Reset);
		btnDiscard.OnClick = Callback(this, &CTemplateOptsDlg::onClick_Discard);
		btnPreview.OnClick = Callback(this, &CTemplateOptsDlg::UpdatePreview);
		bthVarHelp.OnClick = Callback(this, &CTemplateOptsDlg::onVarHelp);

		m_tree.OnSelChanged = Callback(this, &CTemplateOptsDlg::onSelChanged);
	}

	bool OnInitDialog() override
	{
		HIMAGELIST himgTree = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 1, 1);
		m_tree.SetImageList(himgTree, TVSIL_NORMAL);

		ImageList_AddIcon(himgTree, g_plugin.getIcon(ICO_TPLGROUP));

		m_hContact = db_add_contact();
		Proto_AddToContact(m_hContact, META_PROTO);
		Contact::Hide(m_hContact);
		Contact::RemoveFromList(m_hContact);
		db_set_ws(m_hContact, META_PROTO, "Nick", TranslateT("Test contact"));

		DBEVENTINFO dbei = {};
		dbei.pBlob = (uint8_t *)Translate("The quick brown fox jumps over the lazy dog.");
		dbei.cbBlob = (uint32_t)strlen((char *)dbei.pBlob);
		dbei.flags = DBEF_TEMPORARY;
		dbei.eventType = EVENTTYPE_MESSAGE;
		dbei.timestamp = time(0);
		m_hDbEVent = db_event_add(m_hContact, &dbei);

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
				tvis.item.pszText = TranslateW(it.group);
				tvis.item.lParam = 0;
				hGroup = m_tree.InsertItem(&tvis);

				if (!hFirst)
					hFirst = hGroup;
			}

			TVINSERTSTRUCT tvis;
			tvis.hParent = hGroup;
			tvis.hInsertAfter = TVI_LAST;
			tvis.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
			tvis.item.pszText = TranslateW(it.title);
			tvis.item.iSelectedImage = tvis.item.iImage = ImageList_AddIcon(himgTree, it.getIcon());
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
		db_event_delete(m_hDbEVent);
		db_delete_contact(m_hContact);

		for (auto &it : templates)
			replaceStrW(it.tmpValue, nullptr);
	}

	void onClick_Reset(CCtrlButton *)
	{
		if (m_curr) {
			replaceStrW(m_curr->tmpValue, nullptr);
			replaceStrW(m_curr->value, nullptr);
			m_edit.SetText(m_curr->defvalue);
		}

		UpdatePreview(0);
		NotifyChange();
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
			L"%%", TranslateT("simply % character"),
			L"%n", TranslateT("a \"hard\" line break (cr/lf - will break indent)"),
			L"%S", TranslateT("my nickname"),
			L"%N", TranslateT("buddy\'s nickname"),
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
			L"%y", TranslateT("year (4 digits)"),
			L"%w", TranslateT("day of week (Sunday, Monday... translatable)"),
			L"%p", TranslateT("AM/PM symbol"),
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
	odp.pDialog = new CTemplateOptsDlg();
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.a = LPGEN("Advanced");
	odp.pDialog = new CGeneralOptsDlg();
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

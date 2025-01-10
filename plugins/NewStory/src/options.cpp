/*
Copyright (c) 2005 Victor Pavlychko (nullbyte@sotline.net.ua)
Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////
// Basic options page class

class CBaseOptsDlg : public CDlgBase
{
protected:
	CBaseOptsDlg(int iDlgId) :
		CDlgBase(g_plugin, iDlgId)
	{
		m_OnFinishWizard = Callback(this, &CBaseOptsDlg::OnFinish);
	}

	void OnFinish(void *)
	{
		WindowList_BroadcastAsync(g_hNewstoryLogs, NSM_SET_OPTIONS, 0, 0);
		WindowList_BroadcastAsync(g_hNewstoryHistLogs, NSM_SET_OPTIONS, 0, 0);
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// General options dialog

class CGeneralOptsDlg : public CBaseOptsDlg
{
	CCtrlSpin  spinHeight;
	CCtrlCheck chkGrouping, chkVScroll, chkDrawEdge, chkSortOrder, chkHppCompat;
	CCtrlCheck chkShowType, chkShowPreview, chkShowDirection;

public:
	CGeneralOptsDlg() :
		CBaseOptsDlg(IDD_OPT_ADVANCED),
		spinHeight(this, IDC_SPIN1, 500, 200),
		chkVScroll(this, IDC_VSCROLL),
		chkShowType(this, IDC_SHOW_TYPE),
		chkDrawEdge(this, IDC_DRAWEDGE),
		chkGrouping(this, IDC_GROUPING),
		chkHppCompat(this, IDC_HPP_COMPAT),
		chkSortOrder(this, IDC_SORT_ASCENDING),
		chkShowPreview(this, IDC_SHOW_PREVIEW),
		chkShowDirection(this, IDC_SHOW_DIRECTION)
	{
		CreateLink(chkVScroll, g_plugin.bOptVScroll);
		CreateLink(chkShowType, g_bShowType);
		CreateLink(chkGrouping, g_bOptGrouping);
		CreateLink(chkDrawEdge, g_bOptDrawEdge);
		CreateLink(chkSortOrder, g_plugin.bSortAscending);
		CreateLink(chkHppCompat, g_bOptHppCompat);
		CreateLink(chkShowPreview, g_bShowPreview);
		CreateLink(chkShowDirection, g_bShowDirection);

		CreateLink(spinHeight, g_iPreviewHeight);
	}

	bool OnApply() override
	{
		g_plugin.LoadOptions();
		return true;
	}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Template options dialog

static void AppendSymbol(CMStringW &buf, const wchar_t *pwszSymbol, const wchar_t *pwszMeaning)
{
	if (!buf.IsEmpty())
		buf.AppendChar('\n');
	buf.AppendFormat(L"%s - %s", pwszSymbol, pwszMeaning);
}

class CTemplateOptsDlg : public CBaseOptsDlg
{
	TemplateInfo *m_curr = 0;
	ItemData *m_tempItem;
	NewstoryListData *m_histCtrl;

	CCtrlBase preview, gpreview;
	CCtrlEdit m_edit;
	CCtrlColor clr0, clr1, clr2, clr3, clr4;
	CCtrlMButton btnDiscard, bthVarHelp, btnReset;
	CCtrlTreeView m_tree;

public:
	CTemplateOptsDlg() :
		CBaseOptsDlg(IDD_OPT_TEMPLATES),
		clr0(this, IDC_COLOR1),
		clr1(this, IDC_COLOR2),
		clr2(this, IDC_COLOR3),
		clr3(this, IDC_COLOR4),
		clr4(this, IDC_COLOR5),
		m_edit(this, IDC_EDITTEMPLATE),
		m_tree(this, IDC_TEMPLATES),
		preview(this, IDC_PREVIEW),
		gpreview(this, IDC_GPREVIEW),
		btnReset(this, IDC_RESET, Skin_LoadIcon(SKINICON_OTHER_UNDO), LPGEN("Reset to default")),
		btnDiscard(this, IDC_DISCARD, g_plugin.getIcon(IDI_RESET), LPGEN("Cancel edit")),
		bthVarHelp(this, IDC_VARHELP, g_plugin.getIcon(IDI_VARHELP), LPGEN("Variables help"))
	{
		CreateLink(clr0, g_clCustom0);
		CreateLink(clr1, g_clCustom1);
		CreateLink(clr2, g_clCustom2);
		CreateLink(clr3, g_clCustom3);
		CreateLink(clr4, g_clCustom4);

		btnReset.OnClick = Callback(this, &CTemplateOptsDlg::onClick_Reset);
		btnDiscard.OnClick = Callback(this, &CTemplateOptsDlg::onClick_Discard);
		bthVarHelp.OnClick = Callback(this, &CTemplateOptsDlg::onClick_Help);

		m_edit.OnChange = Callback(this, &CTemplateOptsDlg::onChange_Edit);

		m_tree.OnSelChanged = Callback(this, &CTemplateOptsDlg::onSelChanged);
	}

	bool OnInitDialog() override
	{
		HIMAGELIST himgTree = ImageList_Create(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 | ILC_MASK, 1, 1);
		m_tree.SetImageList(himgTree, TVSIL_NORMAL);

		ImageList_AddIcon(himgTree, g_plugin.getIcon(IDI_TPLGROUP));

		m_histCtrl = (NewstoryListData *)GetWindowLongPtr(gpreview.GetHwnd(), 0);
		m_histCtrl->bReadOnly = true;

		m_tempItem = m_histCtrl->items.insert(0);
		m_tempItem->pOwner = m_histCtrl;
		m_tempItem->wszNick = TranslateT("Test contact");
		m_tempItem->wtext = mir_wstrdup(TranslateT("The quick brown fox jumps over the lazy dog."));
		m_tempItem->dbe.flags = DBEF_TEMPORARY | DBEF_BOOKMARK;
		m_tempItem->dbe.szModule = MODULENAME;
		m_tempItem->dbe.eventType = EVENTTYPE_MESSAGE;
		m_tempItem->dbe.iTimestamp = time(0);
		m_histCtrl->totalCount++;

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

		WindowList_Add(g_hNewstoryWindows, m_hwnd);
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

		g_plugin.LoadOptions();
		onChange_Edit();
		SaveTemplates();
		return true;
	}

	void OnDestroy() override
	{
		WindowList_Remove(g_hNewstoryWindows, m_hwnd);

		for (auto &it : templates)
			replaceStrW(it.tmpValue, nullptr);
	}

	void onClick_Reset(CCtrlButton *)
	{
		if (m_curr) {
			replaceStrW(m_curr->tmpValue, nullptr);
			replaceStrW(m_curr->value, nullptr);
			m_edit.SetText(TranslateW(m_curr->defvalue));
		}

		onChange_Edit();
		NotifyChange();
	}

	void onClick_Discard(CCtrlButton *)
	{
		replaceStrW(m_curr->tmpValue, 0);

		if (m_curr->value)
			m_edit.SetText(m_curr->value);
		else
			m_edit.SetText(TranslateW(m_curr->defvalue));

		onChange_Edit();
	}

	void onClick_Help(CCtrlButton *)
	{
		CMStringW wszVarHelp;
		AppendSymbol(wszVarHelp, L"%%", TranslateT("simply % character"));
		AppendSymbol(wszVarHelp, L"%n", TranslateT("a \"hard\" line break (cr/lf - will break indent)"));
		AppendSymbol(wszVarHelp, L"%S", TranslateT("my nickname"));
		AppendSymbol(wszVarHelp, L"%N", TranslateT("buddy\'s nickname"));
		AppendSymbol(wszVarHelp, L"%c", TranslateT("event count"));
		AppendSymbol(wszVarHelp, L"%D", TranslateT("direction symbol"));
		AppendSymbol(wszVarHelp, L"%t", TranslateT("timestamp"));
		AppendSymbol(wszVarHelp, L"%h", TranslateT("hour (24 hour format, 0-23)"));
		AppendSymbol(wszVarHelp, L"%a", TranslateT("hour (12 hour format)"));
		AppendSymbol(wszVarHelp, L"%m", TranslateT("minute"));
		AppendSymbol(wszVarHelp, L"%s", TranslateT("second"));
		AppendSymbol(wszVarHelp, L"%o", TranslateT("month"));
		AppendSymbol(wszVarHelp, L"%d", TranslateT("day of month"));
		AppendSymbol(wszVarHelp, L"%y", TranslateT("year (4 digits)"));
		AppendSymbol(wszVarHelp, L"%w", TranslateT("day of week (Sunday, Monday... translatable)"));
		AppendSymbol(wszVarHelp, L"%p", TranslateT("AM/PM symbol"));
		AppendSymbol(wszVarHelp, L"%O", TranslateT("name of month, translatable"));
		AppendSymbol(wszVarHelp, L"%M", TranslateT("the message string itself"));
		AppendSymbol(wszVarHelp, L"[c0]", TranslateT("default text color"));
		AppendSymbol(wszVarHelp, L"[c1]", TranslateT("nickname's color"));
		AppendSymbol(wszVarHelp, L"[c2]-[c6]", TranslateT("one of the user defined custom color from the options page for the following text output (X is a number from 2 to 6, referring to the color index)"));
		MessageBox(m_hwnd, wszVarHelp, TranslateT("Variables help"), MB_OK);
	}

	void onChange_Edit(CCtrlEdit* = 0)
	{
		if (m_curr == nullptr)
			return;

		replaceStrW(m_curr->tmpValue, m_edit.GetText());

		m_tempItem->savedHeight = -1;
		m_tempItem->fill(int(m_curr - templates)); // copy data from template to event

		CMStringW wszText(m_tempItem->formatStringEx(m_curr->tmpValue));
		preview.SetText(wszText);

		m_tempItem->setText(wszText);
		InvalidateRect(gpreview.GetHwnd(), 0, TRUE);
	}

	void onSelChanged(CCtrlTreeView::TEventInfo *)
	{
		TVITEMEX tvi;
		tvi.hItem = m_tree.GetSelection();
		tvi.mask = TVIF_PARAM;
		m_tree.GetItem(&tvi);

		bool bEnabled = tvi.lParam != 0;
		m_edit.Enable(bEnabled);
		preview.Enable(bEnabled);
		gpreview.Enable(bEnabled);
		btnReset.Enable(bEnabled);
		btnDiscard.Enable(bEnabled);
		bthVarHelp.Enable(bEnabled);
		
		if (!bEnabled) {
			HTREEITEM hItem = m_tree.GetChild(tvi.hItem);
			if (hItem)
				m_tree.Expand(tvi.hItem, TVE_EXPAND);
			return;
		}

		if (m_curr != nullptr)
			replaceStrW(m_curr->tmpValue, m_edit.GetText());

		m_curr = (TemplateInfo *)tvi.lParam;

		if (m_curr->tmpValue)
			m_edit.SetText(m_curr->tmpValue);
		else if (m_curr->value)
			m_edit.SetText(m_curr->value);
		else
			m_edit.SetText(TranslateW(m_curr->defvalue));

		onChange_Edit();
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

/*
WhenWasIt (birthday reminder) plugin for Miranda IM

Copyright © 2006 Cristian Libotean

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

void FillPopupData(POPUPDATAW &ppd, int dtb)
{
	int popupTimeout = (dtb == 0) ? g_plugin.popupTimeoutToday : g_plugin.popupTimeout;

	ppd.colorBack = g_plugin.background;
	ppd.colorText = g_plugin.foreground;
	ppd.iSeconds = popupTimeout;
}

void PopupNotifyNoBirthdays()
{
	POPUPDATAW ppd;
	FillPopupData(ppd, -1);
	ppd.lchIcon = GetDTBIcon(-1);

	wcsncpy(ppd.lpwzContactName, TranslateT("WhenWasIt"), MAX_CONTACTNAME - 1);
	wcsncpy(ppd.lpwzText, TranslateT("No upcoming birthdays."), MAX_SECONDLINE - 1);
	PUAddPopupW(&ppd);
}

wchar_t *BuildDTBText(int dtb, wchar_t *name, wchar_t *text, int size)
{
	if (dtb > 1)
		mir_snwprintf(text, size, TranslateT("%s has birthday in %d days."), name, dtb);
	else if (dtb == 1)
		mir_snwprintf(text, size, TranslateT("%s has birthday tomorrow."), name);
	else
		mir_snwprintf(text, size, TranslateT("%s has birthday today."), name);

	return text;
}

wchar_t *BuildDABText(int dab, wchar_t *name, wchar_t *text, int size)
{
	if (dab > 1)
		mir_snwprintf(text, size, TranslateT("%s had birthday %d days ago."), name, dab);
	else if (dab == 1)
		mir_snwprintf(text, size, TranslateT("%s had birthday yesterday."), name);
	else
		mir_snwprintf(text, size, TranslateT("%s has birthday today (Should not happen, please report)."), name);

	return text;
}

int PopupNotifyBirthday(MCONTACT hContact, int dtb, int age)
{
	if (g_plugin.bIgnoreSubcontacts && db_mc_isSub(hContact))
		return 0;

	wchar_t *name = Clist_GetContactDisplayName(hContact);

	wchar_t text[1024];
	BuildDTBText(dtb, name, text, _countof(text));
	int gender = GetContactGender(hContact);

	POPUPDATAW ppd;
	FillPopupData(ppd, dtb);
	ppd.lchContact = hContact;
	ppd.PluginWindowProc = DlgProcPopup;
	ppd.lchIcon = GetDTBIcon(dtb);

	mir_snwprintf(ppd.lpwzContactName, MAX_CONTACTNAME, TranslateT("Birthday - %s"), name);
	wchar_t *sex;
	switch (toupper(gender)) {
	case 'M':
		sex = TranslateT("He");
		break;
	case 'F':
		sex = TranslateT("She");
		break;
	default:
		sex = TranslateT("He/She");
		break;
	}
	if (age > 0) {
		if (dtb > 0)
			mir_snwprintf(ppd.lpwzText, MAX_SECONDLINE, TranslateT("%s\n%s will be %d years old."), text, sex, age);
		else
			mir_snwprintf(ppd.lpwzText, MAX_SECONDLINE, TranslateT("%s\n%s just turned %d."), text, sex, age);
	}
	else
		mir_wstrncpy(ppd.lpwzText, text, MAX_SECONDLINE - 1);

	PUAddPopupW(&ppd);

	return 0;
}

int PopupNotifyMissedBirthday(MCONTACT hContact, int dab, int age)
{
	if (g_plugin.bIgnoreSubcontacts && db_mc_isSub(hContact))
		return 0;

	wchar_t *name = Clist_GetContactDisplayName(hContact);

	wchar_t text[1024];
	BuildDABText(dab, name, text, _countof(text));
	int gender = GetContactGender(hContact);

	POPUPDATAW ppd;
	FillPopupData(ppd, dab);
	ppd.lchContact = hContact;
	ppd.PluginWindowProc = DlgProcPopup;
	ppd.lchIcon = GetDTBIcon(dab);

	mir_snwprintf(ppd.lpwzContactName, MAX_CONTACTNAME, TranslateT("Birthday - %s"), name);
	wchar_t *sex;
	switch (toupper(gender)) {
	case 'M':
		sex = TranslateT("He");
		break;
	case 'F':
		sex = TranslateT("She");
		break;
	default:
		sex = TranslateT("He/She");
		break;
	}
	if (age > 0)
		mir_snwprintf(ppd.lpwzText, MAX_SECONDLINE, TranslateT("%s\n%s just turned %d."), text, sex, age);
	else
		mir_wstrncpy(ppd.lpwzText, text, MAX_SECONDLINE - 1);

	PUAddPopupW(&ppd);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static class CUpcomingDlg *g_pUpcomingDlg = nullptr;

class CUpcomingDlg : public CBasicListDlg
{
	int timeout;

	CTimer m_timer;

public:
	CUpcomingDlg() :
		CBasicListDlg(IDD_UPCOMING),
		m_timer(this, 1002)
	{
		SetMinSize(400, 160);

		m_timer.OnEvent = Callback(this, &CUpcomingDlg::onTimer);
	}

	bool OnInitDialog() override
	{
		Window_SetIcon_IcoLib(m_hwnd, hListMenu);

		g_pUpcomingDlg = this;
		timeout = g_plugin.cDlgTimeout;

		m_list.SetExtendedListViewStyleEx(LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);

		LVCOLUMN col;
		col.mask = LVCF_TEXT | LVCF_WIDTH;
		col.pszText = TranslateT("Contact");
		col.cx = 300;
		m_list.InsertColumn(0, &col);

		col.pszText = TranslateT("Age");
		col.cx = 45;
		m_list.InsertColumn(1, &col);

		col.pszText = TranslateT("DTB");
		col.cx = 45;
		m_list.InsertColumn(2, &col);

		m_list.SetColumnWidth(0, LVSCW_AUTOSIZE);

		if (timeout > 0)
			m_timer.Start(1000);

		Utils_RestoreWindowPosition(m_hwnd, NULL, MODULENAME, "BirthdayListUpcoming");
		return true;
	}

	void OnDestroy() override
	{
		g_pUpcomingDlg = nullptr;
		Utils_SaveWindowPosition(m_hwnd, NULL, MODULENAME, "BirthdayListUpcoming");
		Window_FreeIcon_IcoLib(m_hwnd);
		m_timer.Stop();
	}

	void OnResize() override
	{
		RECT rcWin;
		GetWindowRect(m_hwnd, &rcWin);

		int cx = rcWin.right - rcWin.left;
		int cy = rcWin.bottom - rcWin.top;
		SetWindowPos(m_list.GetHwnd(), nullptr, 0, 0, (cx - 30), (cy - 80), (SWP_NOZORDER | SWP_NOMOVE));

		m_list.SetColumnWidth(0, (cx - 150));
		SetWindowPos(GetDlgItem(m_hwnd, IDOK), nullptr, ((cx / 2) - 95), (cy - 67), 0, 0, SWP_NOSIZE);
		RedrawWindow(m_hwnd, nullptr, nullptr, (RDW_FRAME | RDW_INVALIDATE));
	}

	void onTimer(CTimer *)
	{
		const int MAX_SIZE = 512;
		wchar_t buffer[MAX_SIZE];
		timeout--;
		mir_snwprintf(buffer, (timeout != 2) ? TranslateT("Closing in %d seconds") : TranslateT("Closing in %d second"), timeout);
		SetDlgItemText(m_hwnd, IDOK, buffer);

		if (timeout <= 0)
			Close();
	}

	void AddBirthDay(MCONTACT hContact, wchar_t *message, int dtb, int age)
	{
		LVFINDINFO fi = { 0 };
		fi.flags = LVFI_PARAM;
		fi.lParam = (LPARAM)hContact;
		if (-1 != m_list.FindItem(-1, &fi))
			return; /* Allready in list. */

		int index = m_list.GetItemCount();
		LVITEM item = { 0 };
		item.iItem = index;
		item.mask = LVIF_PARAM | LVIF_TEXT;
		item.lParam = (LPARAM)hContact;
		item.pszText = message;
		m_list.InsertItem(&item);

		wchar_t buffer[512];
		mir_snwprintf(buffer, L"%d", age);
		m_list.SetItemText(index, 1, buffer);

		mir_snwprintf(buffer, L"%d", dtb);
		m_list.SetItemText(index, 2, buffer);

		Sort(2);
	}
};

int DialogNotifyBirthday(MCONTACT hContact, int dtb, int age)
{
	wchar_t text[1024];
	BuildDTBText(dtb, Clist_GetContactDisplayName(hContact), text, _countof(text));

	if (!g_pUpcomingDlg) {
		g_pUpcomingDlg = new CUpcomingDlg();
		g_pUpcomingDlg->Show(g_plugin.bOpenInBackground ? SW_SHOWNOACTIVATE : SW_SHOW);
	}

	g_pUpcomingDlg->AddBirthDay(hContact, text, dtb, age);
	return 0;
}

int DialogNotifyMissedBirthday(MCONTACT hContact, int dab, int age)
{
	wchar_t text[1024];
	BuildDABText(dab, Clist_GetContactDisplayName(hContact), text, _countof(text));

	if (!g_pUpcomingDlg) {
		g_pUpcomingDlg = new CUpcomingDlg();
		g_pUpcomingDlg->Show(g_plugin.bOpenInBackground ? SW_SHOWNOACTIVATE : SW_SHOW);
	}

	g_pUpcomingDlg->AddBirthDay(hContact, text, -dab, age);
	return 0;
}

void CloseUpcoming()
{
	if (g_pUpcomingDlg)
		g_pUpcomingDlg->Close();
}

int SoundNotifyBirthday(int dtb)
{
	if (dtb == 0)
		Skin_PlaySound(BIRTHDAY_TODAY_SOUND);
	else if (dtb <= g_plugin.cSoundNearDays)
		Skin_PlaySound(BIRTHDAY_NEAR_SOUND);

	return 0;
}

//if oldClistIcon != -1 it will remove the old location of the clist extra icon
//called with oldClistIcon != -1 from dlg_handlers whtn the extra icon slot changes.
int RefreshAllContactListIcons(int oldClistIcon)
{
	for (auto &hContact : Contacts()) {
		if (oldClistIcon != -1)
			ExtraIcon_Clear(hWWIExtraIcons, hContact);

		RefreshContactListIcons(hContact); //will change bBirthdayFound if needed
	}
	return 0;
}

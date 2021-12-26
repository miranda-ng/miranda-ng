/*
"Last Seen mod" plugin for Miranda IM
Copyright ( C ) 2002-03  micron-x
Copyright ( C ) 2005-07  Y.B.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or ( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

MISSEDCONTACTS mcs;

WPARAM IsUserMissed(WPARAM contact)
{
	for (int loop = 0; loop < mcs.count; loop++)
		if (mcs.wpcontact[loop] == contact)
			return MAKEWPARAM(1, loop);

	return 0;
}

int RemoveUser(int pos)
{
	for (int loop = pos; loop < mcs.count - 1; loop++)
		mcs.wpcontact[loop] = mcs.wpcontact[loop + 1];

	mcs.count--;
	return 0;
}

int ResetMissed(void)
{
	for (auto &hContact : Contacts())
		g_plugin.setByte(hContact, "Missed", 0);

	memset(&mcs, 0, sizeof(mcs));
	return 0;
}

int CheckIfOnline(void)
{
	for (auto &hContact : Contacts())
		if (Clist_GetContactIcon(hContact) != ICON_OFFLINE)
			g_plugin.setByte(hContact, "Missed", 2);

	return 0;
}

INT_PTR CALLBACK MissedDlgProc(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
	POINT pt;
	RECT rcinit, rcresized, rcb, rcd;
	HWND htemp;

	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hdlg);

		htemp = GetDlgItem(hdlg, IDC_CONTACTS);
		GetWindowRect(htemp, &rcinit);
		SetWindowPos(htemp, nullptr, 0, 0, rcinit.right - rcinit.left, mcs.count*(rcinit.bottom - rcinit.top) / 2, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		GetWindowRect(htemp, &rcresized);

		htemp = GetDlgItem(hdlg, IDOK);
		GetWindowRect(htemp, &rcb);
		pt.x = rcb.left;
		pt.y = rcb.top;

		ScreenToClient(hdlg, &pt);
		MoveWindow(htemp, pt.x, pt.y + (rcresized.bottom - rcinit.bottom), (rcb.right - rcb.left), (rcb.bottom - rcb.top), FALSE);
		GetWindowRect(hdlg, &rcd);
		SetWindowPos(hdlg, nullptr, 0, 0, rcd.right - rcd.left, rcd.bottom - rcd.top + (rcresized.bottom - rcinit.bottom), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

		SetDlgItemText(hdlg, IDC_CONTACTS, (LPCTSTR)lparam);
		ShowWindow(hdlg, SW_SHOWNOACTIVATE);
		break;

	case WM_CLOSE:
		EndDialog(hdlg, 0);
		break;

	case WM_COMMAND:
		if (LOWORD(wparam) == IDOK)
			SendMessage(hdlg, WM_CLOSE, 0, 0);
		break;
	}

	return 0;
}

int ShowMissed(void)
{
	if (!mcs.count)
		return 0;

	CMStringW buf;
	for (int loop = 0; loop < mcs.count; loop++) {
		buf.Append(Clist_GetContactDisplayName(mcs.wpcontact[loop]));
		if (g_plugin.getByte("MissedOnes_Count", 0))
			buf.AppendFormat(L" [%i]", mcs.times[loop]);

		buf.AppendChar('\n');
	}

	CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_MISSED), nullptr, MissedDlgProc, (LPARAM)buf.c_str());
	return 0;
}

int Test(WPARAM wparam, LPARAM lparam)
{
	if (lparam < ICON_OFFLINE || lparam > ICON_INVIS)
		return 0;

	if (Ignore_IsIgnored(wparam, IGNOREEVENT_USERONLINE))
		return 0;

	if (g_plugin.getByte((MCONTACT)wparam, "Missed", 0) == 2)
		return 0;

	switch (lparam) {
	case ICON_OFFLINE:
		if (g_plugin.getByte((MCONTACT)wparam, "Missed", 0) == 1) {
			uint16_t missed = IsUserMissed(wparam);
			if (!LOWORD(missed)) {
				mcs.times[mcs.count] = 1;
				mcs.wpcontact[mcs.count++] = wparam;
			}
			else mcs.times[HIWORD(missed)]++;

			g_plugin.setByte((MCONTACT)wparam, "Missed", 0);
		}
		break;

	case ICON_ONLINE:
	case ICON_AWAY:
	case ICON_NA:
	case ICON_OCC:
	case ICON_DND:
	case ICON_FREE:
	case ICON_INVIS:
		g_plugin.setByte((MCONTACT)wparam, "Missed", 1);
		break;
	}

	return 0;
}

int ModeChange_mo(WPARAM, LPARAM lparam)
{
	ACKDATA *ack = (ACKDATA *)lparam;
	if (ack->type != ACKTYPE_STATUS || ack->result != ACKRESULT_SUCCESS || ack->hContact != NULL)
		return 0;

	int isetting = Proto_GetStatus(ack->szModule);
	switch (isetting) {
	case ID_STATUS_AWAY:
	case ID_STATUS_DND:
	case ID_STATUS_NA:
		if (ehmissed == nullptr) {
			memset(&mcs, 0, sizeof(mcs));
			CheckIfOnline();
			ehmissed = HookEvent(ME_CLIST_CONTACTICONCHANGED, Test);
		}
		break;

	default:
		if (ehmissed != nullptr) {
			UnhookEvent(ehmissed);
			ehmissed = nullptr;
			ShowMissed();
			ResetMissed();
		}
		break;
	}

	return 0;
}

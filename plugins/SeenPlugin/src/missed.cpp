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

#include "seen.h"

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
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		db_set_b(hContact, S_MOD, "Missed", 0);

	memset(&mcs, 0, sizeof(mcs));
	return 0;
}

int CheckIfOnline(void)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact))
		if (CallService(MS_CLIST_GETCONTACTICON, hContact, 0) != ICON_OFFLINE)
			db_set_b(hContact, S_MOD, "Missed", 2);

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
		SetWindowPos(htemp, NULL, 0, 0, rcinit.right - rcinit.left, mcs.count*(rcinit.bottom - rcinit.top) / 2, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
		GetWindowRect(htemp, &rcresized);

		htemp = GetDlgItem(hdlg, IDOK);
		GetWindowRect(htemp, &rcb);
		pt.x = rcb.left;
		pt.y = rcb.top;

		ScreenToClient(hdlg, &pt);
		MoveWindow(htemp, pt.x, pt.y + (rcresized.bottom - rcinit.bottom), (rcb.right - rcb.left), (rcb.bottom - rcb.top), FALSE);
		GetWindowRect(hdlg, &rcd);
		SetWindowPos(hdlg, NULL, 0, 0, rcd.right - rcd.left, rcd.bottom - rcd.top + (rcresized.bottom - rcinit.bottom), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);

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

	TCHAR sztemp[1024], szcount[7];
	for (int loop = 0; loop < mcs.count; loop++) {
		mir_tstrncat(sztemp, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, mcs.wpcontact[loop], GCDNF_TCHAR), SIZEOF(sztemp) - mir_tstrlen(sztemp));
		if (db_get_b(NULL, S_MOD, "MissedOnes_Count", 0)) {
			mir_sntprintf(szcount, SIZEOF(szcount), _T(" [%i]"), mcs.times[loop]);
			mir_tstrcat(sztemp, szcount);
		}

		mir_tstrcat(sztemp, _T("\n"));
	}

	CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_MISSED), NULL, MissedDlgProc, (LPARAM)sztemp);
	return 0;
}

int Test(WPARAM wparam, LPARAM lparam)
{
	if (lparam < ICON_OFFLINE || lparam > ICON_INVIS)
		return 0;

	if (CallService(MS_IGNORE_ISIGNORED, wparam, IGNOREEVENT_USERONLINE))
		return 0;

	if (db_get_b((MCONTACT)wparam, S_MOD, "Missed", 0) == 2)
		return 0;

	switch (lparam) {
	case ICON_OFFLINE:
		if (db_get_b((MCONTACT)wparam, S_MOD, "Missed", 0) == 1) {
			WORD missed = IsUserMissed(wparam);
			if (!LOWORD(missed)) {
				mcs.times[mcs.count] = 1;
				mcs.wpcontact[mcs.count++] = wparam;
			}
			else mcs.times[HIWORD(missed)]++;

			db_set_b((MCONTACT)wparam, S_MOD, "Missed", 0);
		}
		break;

	case ICON_ONLINE:
	case ICON_AWAY:
	case ICON_NA:
	case ICON_OCC:
	case ICON_DND:
	case ICON_FREE:
	case ICON_INVIS:
		db_set_b((MCONTACT)wparam, S_MOD, "Missed", 1);
		break;
	}

	return 0;
}

int ModeChange_mo(WPARAM, LPARAM lparam)
{
	ACKDATA *ack = (ACKDATA *)lparam;
	if (ack->type != ACKTYPE_STATUS || ack->result != ACKRESULT_SUCCESS || ack->hContact != NULL)
		return 0;

	int isetting = CallProtoService(ack->szModule, PS_GETSTATUS, 0, 0);
	switch (isetting) {
	case ID_STATUS_AWAY:
	case ID_STATUS_DND:
	case ID_STATUS_NA:
		if (ehmissed == NULL) {
			memset(&mcs, 0, sizeof(mcs));
			CheckIfOnline();
			ehmissed = HookEvent(ME_CLIST_CONTACTICONCHANGED, Test);
		}
		break;

	default:
		if (ehmissed != NULL) {
			UnhookEvent(ehmissed);
			ehmissed = NULL;
			ShowMissed();
			ResetMissed();
		}
		break;
	}

	return 0;
}

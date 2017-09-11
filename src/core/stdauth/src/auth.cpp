/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (ñ) 2012-17 Miranda NG project (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

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

#define MS_AUTH_SHOWREQUEST	"Auth/ShowRequest"
#define MS_AUTH_SHOWADDED	"Auth/ShowAdded"

INT_PTR CALLBACK DlgProcAuthReq(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcAdded(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);

INT_PTR ShowReqWindow(WPARAM, LPARAM lParam)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_AUTHREQ), NULL, DlgProcAuthReq, (LPARAM)((CLISTEVENT *)lParam)->hDbEvent);
	return 0;
}

INT_PTR ShowAddedWindow(WPARAM, LPARAM lParam)
{
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ADDED), NULL, DlgProcAdded, (LPARAM)((CLISTEVENT *)lParam)->hDbEvent);
	return 0;
}

static int AuthEventAdded(WPARAM, LPARAM lParam)
{
	wchar_t szTooltip[256];
	MEVENT hDbEvent = (MEVENT)lParam;

	DBEVENTINFO dbei = {};
	db_event_get(lParam, &dbei);
	if (dbei.flags & (DBEF_SENT | DBEF_READ) || (dbei.eventType != EVENTTYPE_AUTHREQUEST && dbei.eventType != EVENTTYPE_ADDED))
		return 0;

	dbei.cbBlob = db_event_getBlobSize(hDbEvent);
	dbei.pBlob = (PBYTE)alloca(dbei.cbBlob);
	db_event_get(hDbEvent, &dbei);

	MCONTACT hContact = DbGetAuthEventContact(&dbei);

	CLISTEVENT cli = {};
	cli.hContact = hContact;
	cli.szTooltip.w = szTooltip;
	cli.flags = CLEF_UNICODE;
	cli.lParam = lParam;
	cli.hDbEvent = hDbEvent;

	ptrW szUid(Contact_GetInfo(CNF_UNIQUEID, hContact));

	if (dbei.eventType == EVENTTYPE_AUTHREQUEST) {
		Skin_PlaySound("AuthRequest");
		if (szUid)
			mir_snwprintf(szTooltip, TranslateT("%s requests authorization"), szUid);
		else
			mir_snwprintf(szTooltip, TranslateT("%u requests authorization"), *(PDWORD)dbei.pBlob);

		cli.hIcon = Skin_LoadIcon(SKINICON_AUTH_REQUEST);
		cli.pszService = MS_AUTH_SHOWREQUEST;
		pcli->pfnAddEvent(&cli);
	}
	else if (dbei.eventType == EVENTTYPE_ADDED) {
		Skin_PlaySound("AddedEvent");
		if (szUid)
			mir_snwprintf(szTooltip, TranslateT("%s added you to their contact list"), szUid);
		else
			mir_snwprintf(szTooltip, TranslateT("%u added you to their contact list"), *(PDWORD)dbei.pBlob);

		cli.hIcon = Skin_LoadIcon(SKINICON_AUTH_ADD);
		cli.pszService = MS_AUTH_SHOWADDED;
		pcli->pfnAddEvent(&cli);
	}
	return 0;
}

int LoadSendRecvAuthModule(void)
{
	CreateServiceFunction(MS_AUTH_SHOWREQUEST, ShowReqWindow);
	CreateServiceFunction(MS_AUTH_SHOWADDED, ShowAddedWindow);
	HookEvent(ME_DB_EVENT_ADDED, AuthEventAdded);

	Skin_AddSound("AuthRequest", LPGENW("Alerts"), LPGENW("Authorization request"));
	Skin_AddSound("AddedEvent", LPGENW("Alerts"), LPGENW("Added event"));
	return 0;
}

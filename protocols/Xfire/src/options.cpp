/*
 *  Plugin of miranda IM(ICQ) for Communicating with users of the XFire Network.
 *
 *  Copyright (C) 2010 by
 *          dufte <dufte@justmail.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 *
 *  Based on J. Lawler              - BaseProtocol
 *			 Herbert Poul/Beat Wolf - xfirelib
 *
 *  Miranda ICQ: the free icq client for MS Windows
 *  Copyright (C) 2000-2008  Richard Hughes, Roland Rabien & Tristan Van de Vreede
 *
 */

#include "stdafx.h"

#include "baseProtocol.h"
#include "m_variables.h"
#include "recvprefspacket.h"
#include "Xfire_gamelist.h"
#include "addgamedialog.h"
#include "variables.h"

extern HANDLE XFireWorkingFolder;
extern HANDLE XFireIconFolder;
extern xfire_prefitem xfireconfig[XFIRE_RECVPREFSPACKET_MAXCONFIGS];
extern Xfire_gamelist xgamelist;

struct mytreeitem {
	TCHAR name[256];
	char dbentry[256];
	int parent;
	int icon;
	HTREEITEM hitem;
};

extern int bpStatus;
BOOL alreadyInit = FALSE;
HWND ghwndDlg2;
HWND addgameDialog = NULL;

xfireconfigitem xfireconfigitems[XFIRE_RECVPREFSPACKET_SUPPORTEDONFIGS] = {
		{ IDC_KONFIG_1, 1, "sendgamestatus" },
		{ IDC_KONFIG_2, 3, "hideprofile" },
		{ IDC_KONFIG_3, 2, "ipportdetec" },
		{ IDC_KONFIG_4, 0xa, "shownicks" },
		{ IDC_KONFIG_5, 0xb, "ts2detection" },
		{ IDC_KONFIG_6, 0xc, "sendtyping" },
		{ IDC_KONFIG_7, 8, "friendsoffriends" },
};

#define NUM_ICONS 4

static mytreeitem mytree[] = {
		{ LPGENT("Avatars"), "", 1, 0 },
		{ LPGENT("Disable avatars"), "noavatars", 0, 0 },
		{ LPGENT("Don't download avatars of clan members"), "noclanavatars", 0, 0 },
		{ LPGENT("Use alternate way for Avatar download"), "specialavatarload", 0, 0 },
		{ LPGENT("General"), "", 1, 0 },
		{ LPGENT("Automatically reconnect on protocol version changes"), "recprotoverchg", 0, 0 },
		{ LPGENT("No IP/Port in StatusMsg"), "noipportinstatus", 0, 0 },
		{ LPGENT("Use Online status for unsupported global statuses"), "oninsteadafk", 0, 0 },
		{ LPGENT("Don't move friends to clan groups"), "skipfriendsgroups", 0, 0 },
		{ LPGENT("GameServerQuery support"), "gsqsupport", 0, 0 },
		{ LPGENT("No custom away message"), "nocustomaway", 0, 0 },
		{ LPGENT("Remove friend of friend buddies from database"), "fofdbremove", 0, 0 },
		/*{"Show usernames only","onlyusername",0},*/
		{ LPGENT("Hide yourself in buddylist"), "skipmyself", 0, 0 },
		{ LPGENT("Don't display game search results"), "dontdisresults", 0, 0 },
		{ LPGENT("Don't display game search status window"), "dontdissstatus", 0, 0 },
		{ LPGENT("Display popup if someone starts a game"), "gamepopup", 0, 0 },
		{ LPGENT("Don't automatically create clan groups"), "noclangroups", 0, 0 },
		{ LPGENT("Enable MBot support"), "mbotsupport", 0, 0 },
		{ LPGENT("Game detection"), "", 1, 0 },
		/*{"Scan for games on every Miranda start","scanalways",0},*/
		{ LPGENT("Disable game detection"), "nogamedetect", 0, 0 },
		/*{"Enable server IP/Port detection","ipportdetec",0},*/
		/*{"Enable TeamSpeak2/Ventrilo detection","",0},*/
		{ LPGENT("Use TSRemote.dll to get TeamSpeak server info"), "ts2useremote", 0, 0 },
		{ LPGENT("Disable popups when ingame"), "nopopups", 0, 0 },
		{ LPGENT("Disable sound events when ingame"), "nosoundev", 0, 0 },
		{ LPGENT("Files"), "", 1, 0 },
		{ LPGENT("Automatically update xfire_games.ini (pro-laming.de)"), "autoiniupdate", 0, 0 },
		{ LPGENT("Automatically update icons.dll (pro-laming.de)"), "autoicodllupdate", 0, 0 },
		{ LPGENT("Download missing game icons from Xfire website"), "xfiresitegameico", 0, 0 },
		{ LPGENT("No backup on update"), "nobackupini", 0, 0 },
		{ LPGENT("Background updating"), "dontaskforupdate", 0, 0 },
};

//funktion zum auslesen aller einträge unter XFireBlock
static int enumSettingsProc(const char *szSetting, LPARAM lParam)
{
	if (strlen(szSetting) > 0)
	{
		SendDlgItemMessageA((HWND)lParam, IDC_BLOCKUSER, LB_ADDSTRING, 0, (LPARAM)szSetting);
	}
	EnableDlgItem((HWND)lParam, IDC_REMUSER, TRUE);
	return 0;
}

//damit die änderungen sofort sichtbar sind
static INT_PTR CALLBACK DlgProcOpts2(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char inipath[XFIRE_MAX_STATIC_STRING_LEN] = "";
	static BOOL inifound = FALSE;
	static BOOL dllfound = FALSE;
	static int nomsgboxsel[] = { 1, 0, 2 };
	static HWND hwndTree = NULL;
	static char login[128];

	switch (msg)
	{
	case PSM_CHANGED:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_INITDIALOG:
	{
		DBVARIANT dbv;
		TranslateDialogDefault(hwndDlg);

		ghwndDlg2 = hwndDlg;

		hwndTree = GetDlgItem(hwndDlg, IDC_TREE);
		SetWindowLongPtr(hwndTree, GWL_STYLE, GetWindowLongPtr(hwndTree, GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);
		SendMessage(hwndDlg, DM_REBUILD_TREE, 0, 0);

		strcpy(inipath, XFireGetFoldersPath("IniFile"));
		strcat(inipath, "xfire_games.ini");

		FILE * f = fopen(inipath, "r");
		if (f != NULL)
		{
			fclose(f);
			CheckDlgButton(hwndDlg, IDC_CHKG, 1);
			inifound = TRUE;
		}
		else
		{
			EnableDlgItem(hwndDlg, IDC_SETUPGAMES, FALSE);
			inifound = FALSE;
		}

		strcpy(inipath, XFireGetFoldersPath("IconsFile"));
		strcat(inipath, "icons.dll");

		f = fopen(inipath, "r");
		if (f != NULL)
		{
			fclose(f);
			CheckDlgButton(hwndDlg, IDC_CHKI, 1);
			dllfound = TRUE;
		}
		else
			dllfound = FALSE;

		if (!db_get(NULL, protocolname, "login", &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_LOGIN, dbv.pszVal);
			db_free(&dbv);
		}
		if (!db_get(NULL, protocolname, "Nick", &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_NICK, dbv.pszVal);
			db_free(&dbv);
		}
		if (!db_get(NULL, protocolname, "password", &dbv)) {
			//bit of a security hole here, since it's easy to extract a password from an edit box
			SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal);
			db_free(&dbv);
		}

		char temp[255] = "";
		mir_snprintf(temp, SIZEOF(temp), "%d", db_get_b(NULL, protocolname, "protover", 0x5b));
		SetDlgItemTextA(hwndDlg, IDC_PVER, temp);

		EnableWindow(GetDlgItem(hwndDlg, IDC_LASTGAME), FALSE);
		if (!db_get(NULL, protocolname, "LastGame", &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_LASTGAME, dbv.pszVal);
			db_free(&dbv);
		}

		if (bpStatus == ID_STATUS_OFFLINE&&bpStatus != ID_STATUS_CONNECTING) {
			EnableDlgItem(hwndDlg, IDC_NICK, FALSE);
		}
		else
		{
			int size = sizeof(xfireconfigitems) / sizeof(xfireconfigitem);
			for (int i = 0; i < size; i++)
			{
				EnableDlgItem(hwndDlg, xfireconfigitems[i].id, TRUE);
				if (xfireconfig[xfireconfigitems[i].xfireconfigid].wasset == 0)
				{
					CheckDlgButton(hwndDlg, xfireconfigitems[i].id, 1);
				}
				else
				{
					CheckDlgButton(hwndDlg, xfireconfigitems[i].id, 0);
				}
			}
			//wenn die erste option aktiv ist, untere aktivieren, sonst deaktivieren
			if (!(BYTE)IsDlgButtonChecked(hwndDlg, IDC_KONFIG_1))
			{
				CheckDlgButton(hwndDlg, IDC_KONFIG_2, 0);
				CheckDlgButton(hwndDlg, IDC_KONFIG_3, 0);
				EnableDlgItem(hwndDlg, IDC_KONFIG_2, FALSE);
				EnableDlgItem(hwndDlg, IDC_KONFIG_3, FALSE);
			}
		}

		return TRUE;
	}



	case WM_COMMAND:
		if (!(BYTE)IsDlgButtonChecked(hwndDlg, IDC_KONFIG_1))
		{
			CheckDlgButton(hwndDlg, IDC_KONFIG_2, 0);
			CheckDlgButton(hwndDlg, IDC_KONFIG_3, 0);
			EnableDlgItem(hwndDlg, IDC_KONFIG_2, FALSE);
			EnableDlgItem(hwndDlg, IDC_KONFIG_3, FALSE);
		}
		else
		{
			EnableDlgItem(hwndDlg, IDC_KONFIG_2, TRUE);
			EnableDlgItem(hwndDlg, IDC_KONFIG_3, TRUE);
		}
		if (LOWORD(wParam) == IDC_URLNEWACC) {
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)"http://www.xfire.com/register/");
			return TRUE;
		}
		if (LOWORD(wParam) == IDC_LOSTPW) {
			CallService(MS_UTILS_OPENURL, 1, (LPARAM)"https://secure.xfire.com/lost_password/");
			return TRUE;
		}

		if ((LOWORD(wParam) == 19901 || LOWORD(wParam) == 25466 || LOWORD(wParam) == IDC_LOGIN || LOWORD(wParam) == IDC_PIP || LOWORD(wParam) == IDC_PPORT || LOWORD(wParam) == IDC_NICK || LOWORD(wParam) == IDC_PASSWORD) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
		{
			int reconnectRequired = 0;
			char str[128];
			DBVARIANT dbv;

			GetDlgItemTextA(hwndDlg, IDC_LOGIN, login, SIZEOF(login));
			dbv.pszVal = NULL;
			if (db_get(NULL, protocolname, "login", &dbv) || lstrcmpA(login, dbv.pszVal))
				reconnectRequired = 1;
			if (dbv.pszVal != NULL)
				db_free(&dbv);

			//den login lowercasen
			int size = mir_strlen(login);
			BOOL mustlowercase = FALSE;
			for (int i = 0; i < size; i++)
			{
				if (login[i] >= 'A'&&login[i] <= 'Z')
					mustlowercase = TRUE;
				login[i] = tolower(login[i]);
			}
			if (mustlowercase) {
				MessageBox(NULL, TranslateT("The username must be lowercase, so it will be lowercased saved."), TranslateT("XFire Options"), MB_OK | MB_ICONINFORMATION);
				SetDlgItemTextA(hwndDlg, IDC_LOGIN, login);
			}

			db_set_s(NULL, protocolname, "login", login);
			db_set_s(NULL, protocolname, "Username", login);

			//nur wenn der nick erfolgreich übertragen wurde
			GetDlgItemTextA(hwndDlg, IDC_NICK, login, SIZEOF(login));
			dbv.pszVal = NULL;
			if (db_get(NULL, protocolname, "Nick", &dbv) || lstrcmpA(login, dbv.pszVal))
			{
				if (CallService(XFIRE_SET_NICK, 0, (WPARAM)login))
					db_set_s(NULL, protocolname, "Nick", login);
			}
			if (dbv.pszVal != NULL)
				db_free(&dbv);

			GetDlgItemTextA(hwndDlg, IDC_PASSWORD, str, SIZEOF(str));
			dbv.pszVal = NULL;
			if (db_get(NULL, protocolname, "password", &dbv) || lstrcmpA(str, dbv.pszVal))
				reconnectRequired = 1;
			if (dbv.pszVal != NULL)
				db_free(&dbv);
			db_set_s(NULL, protocolname, "password", str);
			GetDlgItemTextA(hwndDlg, IDC_SERVER, str, SIZEOF(str));

			//neue preferencen sichern
			if (bpStatus != ID_STATUS_OFFLINE&&bpStatus != ID_STATUS_CONNECTING)
			{
				int size = sizeof(xfireconfigitems) / sizeof(xfireconfigitem);
				for (int i = 0; i < size; i++)
				{
					if (!(BYTE)IsDlgButtonChecked(hwndDlg, xfireconfigitems[i].id))
					{
						db_set_b(NULL, protocolname, xfireconfigitems[i].dbentry, 0);
						xfireconfig[xfireconfigitems[i].xfireconfigid].wasset = 1;
						xfireconfig[xfireconfigitems[i].xfireconfigid].data[0] = 1;
						xfireconfig[xfireconfigitems[i].xfireconfigid].data[1] = 1;
						xfireconfig[xfireconfigitems[i].xfireconfigid].data[2] = 0;
						xfireconfig[xfireconfigitems[i].xfireconfigid].data[3] = 0x30;
					}
					else
					{
						xfireconfig[xfireconfigitems[i].xfireconfigid].wasset = 0;
						db_set_b(NULL, protocolname, xfireconfigitems[i].dbentry, 1);
					}
				}
				CallService(XFIRE_SEND_PREFS, 0, 0);
			}

			//protocolversion wird autoamtisch vergeben
			//GetDlgItemTextA(hwndDlg,IDC_PVER,str,SIZEOF(str));
			//db_set_b(NULL,protocolname,"protover",(char)atoi(str));

			if (reconnectRequired)
				MessageBox(hwndDlg, TranslateT("The changes you have made require you to reconnect to the XFire network before they take effect"), TranslateT("XFire Options"), MB_OK | MB_ICONINFORMATION);
			return TRUE;
		}

		}
		break;
	}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcOpts3(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char inipath[XFIRE_MAX_STATIC_STRING_LEN] = "";
	static BOOL inifound = FALSE;
	static BOOL dllfound = FALSE;
	static int nomsgboxsel[] = { 1, 0, 2 };
	static HWND hwndTree = NULL;

	switch (msg)
	{
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);

		hwndTree = GetDlgItem(hwndDlg, IDC_TREE);

		SetWindowLongPtr(hwndTree, GWL_STYLE, GetWindowLongPtr(hwndTree, GWL_STYLE) | TVS_NOHSCROLL | TVS_CHECKBOXES);

		SendMessage(hwndDlg, DM_REBUILD_TREE, 0, 0);

		SendDlgItemMessage(hwndDlg, IDC_NOMSG, CB_ADDSTRING, 0, (LPARAM)TranslateT("Hidden"));
		SendDlgItemMessage(hwndDlg, IDC_NOMSG, CB_ADDSTRING, 0, (LPARAM)TranslateT("Message box"));
		SendDlgItemMessage(hwndDlg, IDC_NOMSG, CB_ADDSTRING, 0, (LPARAM)TranslateT("Popup"));

		//scanalways
		SendDlgItemMessage(hwndDlg, IDC_SCANUPDATECB, CB_ADDSTRING, 0, (LPARAM)TranslateT("No"));
		SendDlgItemMessage(hwndDlg, IDC_SCANUPDATECB, CB_ADDSTRING, 0, (LPARAM)TranslateT("On every start"));
		SendDlgItemMessage(hwndDlg, IDC_SCANUPDATECB, CB_ADDSTRING, 0, (LPARAM)TranslateT("Daily"));

		SendDlgItemMessage(hwndDlg, IDC_SCANUPDATECB, CB_SETCURSEL, db_get_b(NULL, protocolname, "scanalways", 0), 0);

		SendDlgItemMessage(hwndDlg, IDC_NOMSG, CB_SETCURSEL, nomsgboxsel[db_get_b(NULL, protocolname, "nomsgbox", 0)], 0);

		/* Gruppen raussuchen */
		{
			int gruppen_id = 0;
			char temp[8];
			DBVARIANT dbv;

			SendDlgItemMessage(hwndDlg, IDC_CLANGROUP, CB_ADDSTRING, 0, (LPARAM)TranslateT("<Root Group>"));
			SendDlgItemMessage(hwndDlg, IDC_FOFGROUP, CB_ADDSTRING, 0, (LPARAM)TranslateT("<Root Group>"));

			mir_snprintf(temp, SIZEOF(temp), "%d", gruppen_id);
			while (!db_get_s(NULL, "CListGroups", temp, &dbv))
			{
				gruppen_id++;
				mir_snprintf(temp, SIZEOF(temp), "%d", gruppen_id);

				if (dbv.pszVal != NULL) {
					SendDlgItemMessageA(hwndDlg, IDC_CLANGROUP, CB_ADDSTRING, 0, (LPARAM)&dbv.pszVal[1]);
					SendDlgItemMessageA(hwndDlg, IDC_FOFGROUP, CB_ADDSTRING, 0, (LPARAM)&dbv.pszVal[1]);
					db_free(&dbv);
				}
			}
			SendDlgItemMessage(hwndDlg, IDC_CLANGROUP, CB_SETCURSEL, db_get_b(NULL, protocolname, "mainclangroup", 0), 0);
			SendDlgItemMessage(hwndDlg, IDC_FOFGROUP, CB_SETCURSEL, db_get_b(NULL, protocolname, "fofgroup", 0), 0);
		}



		return TRUE;
	}

	case DM_REBUILD_TREE:
		//baue optionsmenü auf
		TreeView_SelectItem(hwndTree, NULL);
		ShowWindow(hwndTree, SW_HIDE);
		TreeView_DeleteAllItems(hwndTree);
		{
			TVINSERTSTRUCT tvis;
			HTREEITEM lastSec = NULL;

			tvis.hParent = NULL;
			tvis.hInsertAfter = TVI_SORT;
			tvis.item.state = tvis.item.stateMask = TVIS_EXPANDED;

			int size = sizeof(mytree) / sizeof(mytreeitem);


			for (int i = 0; i < size; i++)
			{
				tvis.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
				tvis.item.iImage = -1;
				tvis.item.stateMask = TVIS_EXPANDED;
				tvis.item.state = TVIS_EXPANDED;
				tvis.hParent = lastSec;
				if (mytree[i].parent) {
					tvis.hParent = NULL;
					tvis.item.lParam = -1;
					tvis.item.pszText = TranslateTS(mytree[i].name);
					tvis.hParent = tvis.item.hItem = TreeView_InsertItem(hwndTree, &tvis);
					lastSec = tvis.hParent;
					tvis.item.stateMask = TVIS_STATEIMAGEMASK;
					tvis.item.state = INDEXTOSTATEIMAGEMASK(0);
					TreeView_SetItem(hwndTree, &tvis.item);
				}
				else
				{
					tvis.item.stateMask = TVIS_STATEIMAGEMASK;
					tvis.item.state = INDEXTOSTATEIMAGEMASK(db_get_b(NULL, protocolname, mytree[i].dbentry, 0) == 1 ? 2 : 1);
					tvis.item.lParam = 0;
					tvis.item.pszText = TranslateTS(mytree[i].name);
					mytree[i].hitem = TreeView_InsertItem(hwndTree, &tvis);
				}
			}
		}
		{
			TVITEM tvi;
			tvi.hItem = TreeView_GetRoot(hwndTree);
			while (tvi.hItem != NULL) {
				tvi.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;
				TreeView_GetItem(hwndTree, &tvi);
				if (tvi.lParam == -1)
					TreeView_SetItemState(hwndTree, tvi.hItem, INDEXTOSTATEIMAGEMASK(0), TVIS_STATEIMAGEMASK);

				tvi.hItem = TreeView_GetNextSibling(hwndTree, tvi.hItem);
			}
		}


		ShowWindow(hwndTree, SW_SHOW);

		break;

	case PSM_CHANGED:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_COMMAND:
		CheckDlgButton(hwndDlg, IDC_CHKI, dllfound);
		CheckDlgButton(hwndDlg, IDC_CHKG, inifound);

		if (HIWORD(wParam) == CBN_SELCHANGE)
		{
			SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
			return 0;
		}

		if ((LOWORD(wParam) == IDC_LOGIN || LOWORD(wParam) == 1013 || LOWORD(wParam) == 1015 || LOWORD(wParam) == 3 || LOWORD(wParam) == IDC_NICK || LOWORD(wParam) == IDC_PASSWORD) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->idFrom)
		{
		case IDC_TREE:
		{
			switch (((NMHDR*)lParam)->code) { //wenn was geändert wurde, apply aktivieren
			case TVN_SELCHANGEDA:
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			case TVN_KEYDOWN: //tastatursteuerung
			{
				NMTVKEYDOWN* ptkd = (NMTVKEYDOWN*)lParam;
				if (ptkd&&ptkd->wVKey == VK_SPACE&&TreeView_GetSelection(ptkd->hdr.hwndFrom))
					SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			case NM_CLICK: //wenn was geklickt wurde, apply aktivieren
			{
				TVHITTESTINFO hti;
				hti.pt.x = (short)LOWORD(GetMessagePos());
				hti.pt.y = (short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom, &hti.pt);
				if (TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom, &hti))
					if (hti.flags&TVHT_ONITEM)
						if (hti.flags&TVHT_ONITEMSTATEICON)
							if (TreeView_GetParent(hwndTree, hti.hItem) != NULL)
								SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
				break;
			}
			}
		}
		}
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
		{
			int reconnectRequired = 0;
			int ccc;

			hwndTree = GetDlgItem(hwndDlg, IDC_TREE);

			//optionen speichern
			for (int i = 0; i < sizeof(mytree) / sizeof(mytreeitem); i++)
			{
				TVITEM tvic;
				if (mytree[i].parent == 0)
				{
					tvic.hItem = mytree[i].hitem;
					tvic.mask = TVIF_PARAM | TVIF_HANDLE | TVIF_STATE;

					TreeView_GetItem(hwndTree, &tvic);
					if (((tvic.state & TVIS_STATEIMAGEMASK) >> 12 == 2))
						db_set_b(NULL, protocolname, mytree[i].dbentry, 1);
					else
						db_set_b(NULL, protocolname, mytree[i].dbentry, 0);
				}
			}

			db_set_b(NULL, protocolname, "nomsgbox", (BYTE)nomsgboxsel[SendDlgItemMessage(hwndDlg, IDC_NOMSG, CB_GETCURSEL, 0, 0)]);

			ccc = SendDlgItemMessage(hwndDlg, IDC_CLANGROUP, CB_GETCURSEL, 0, 0);
			db_set_b(NULL, protocolname, "mainclangroup", (BYTE)ccc);
			ccc = SendDlgItemMessage(hwndDlg, IDC_FOFGROUP, CB_GETCURSEL, 0, 0);
			db_set_b(NULL, protocolname, "fofgroup", (BYTE)ccc);
			ccc = SendDlgItemMessage(hwndDlg, IDC_SCANUPDATECB, CB_GETCURSEL, 0, 0);
			db_set_b(NULL, protocolname, "scanalways", (BYTE)ccc);

			//protocolversion wird autoamtisch vergeben
			//GetDlgItemTextA(hwndDlg,IDC_PVER,str,SIZEOF(str));
			//db_set_b(NULL,protocolname,"protover",(char)atoi(str));

			if (reconnectRequired) MessageBox(hwndDlg, TranslateT("The changes you have made require you to reconnect to the XFire network before they take effect"), TranslateT("XFire Options"), MB_OK | MB_ICONINFORMATION);
			return TRUE;
		}

		}
		break;
	}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcOpts4(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char inipath[XFIRE_MAX_STATIC_STRING_LEN] = "";
	static BOOL inifound = FALSE;
	static BOOL dllfound = FALSE;
	static int nomsgboxsel[] = { 1, 0, 2 };
	static HWND hwndTree = NULL;

	switch (msg)
	{
	case WM_INITDIALOG:
	{
		//addgamedia auf 0 setzen
		TranslateDialogDefault(hwndDlg);

		strcpy(inipath, XFireGetFoldersPath("IniFile"));
		strcat(inipath, "xfire_games.ini");

		FILE * f = fopen(inipath, "r");
		if (f != NULL)
		{
			fclose(f);
			CheckDlgButton(hwndDlg, IDC_CHKG, 1);
			inifound = TRUE;
		}
		else
		{
			EnableDlgItem(hwndDlg, IDC_SETUPGAMES, FALSE);
			inifound = FALSE;
		}

		strcpy(inipath, XFireGetFoldersPath("IconsFile"));
		strcat(inipath, "icons.dll");

		f = fopen(inipath, "r");
		if (f != NULL)
		{
			fclose(f);
			CheckDlgButton(hwndDlg, IDC_CHKI, 1);
			dllfound = TRUE;
		}
		else
			dllfound = FALSE;

		//alle blockierten nutzer in die liste einfügen
		DBCONTACTENUMSETTINGS dbces;

		// enum all setting the contact has for the module
		dbces.pfnEnumProc = enumSettingsProc;
		dbces.szModule = "XFireBlock";
		dbces.lParam = (LPARAM)hwndDlg;
		CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);

		SendMessage(GetDlgItem(hwndDlg, IDC_REMUSER), BM_SETIMAGE, IMAGE_ICON, (WPARAM)LoadSkinnedIcon(SKINICON_OTHER_DELETE));

		strcpy(inipath, XFireGetFoldersPath("IniFile"));
		SetDlgItemTextA(hwndDlg, IDC_FILESSHOULDBE, inipath);

		EnableDlgItem(hwndDlg, IDC_REMUSER, FALSE);

		return TRUE;
	}

	case PSM_CHANGED:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_COMMAND:
		CheckDlgButton(hwndDlg, IDC_CHKI, dllfound);
		CheckDlgButton(hwndDlg, IDC_CHKG, inifound);

		if (LOWORD(wParam) == IDC_REMUSER) //nutzer soll aus der blockierliste raus
		{
			char temp[255];
			int sel = SendDlgItemMessage(hwndDlg, IDC_BLOCKUSER, LB_GETCURSEL, 0, 0);
			if (sel != LB_ERR) //nur wenn was ausgewählt wurde
			{
				SendDlgItemMessageA(hwndDlg, IDC_BLOCKUSER, LB_GETTEXT, sel, (LPARAM)temp);
				SendDlgItemMessage(hwndDlg, IDC_BLOCKUSER, LB_DELETESTRING, sel, 0);
				db_unset(NULL, "XFireBlock", temp);
				if (SendDlgItemMessage(hwndDlg, IDC_BLOCKUSER, LB_GETCOUNT, 0, 0) == 0)
					EnableDlgItem(hwndDlg, IDC_REMUSER, FALSE);
			}
		}


		if ((LOWORD(wParam) == 9508 || LOWORD(wParam) == IDC_LOGIN || LOWORD(wParam) == IDC_SETUPGAMES || LOWORD(wParam) == IDC_PASSWORD) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;

		break;

	case WM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
		{
			return TRUE;
		}

		}
		break;
	}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcOpts5(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DBVARIANT dbv;

	switch (msg)
	{
	case WM_INITDIALOG:
	{
		TranslateDialogDefault(hwndDlg);

		CheckDlgButton(hwndDlg, IDC_ENABLESTSMSG, db_get_b(NULL, protocolname, "autosetstatusmsg", 0));
		CheckDlgButton(hwndDlg, IDC_CHGSTATUS, db_get_b(NULL, protocolname, "statuschgtype", 0));
		CheckDlgButton(hwndDlg, IDC_DNDFIRST, db_get_b(NULL, protocolname, "dndfirst", 0));
		if (!db_get(NULL, protocolname, "setstatusmsg", &dbv)) {
			SetDlgItemTextA(hwndDlg, IDC_STATUSMSG, dbv.pszVal);
			db_free(&dbv);
		}
		if (!IsDlgButtonChecked(hwndDlg, IDC_ENABLESTSMSG))
		{
			EnableDlgItem(hwndDlg, IDC_STATUSMSG, FALSE);
		}
		/*	if (!ServiceExists(MS_VARS_FORMATSTRING))
			{
			EnableDlgItem(hwndDlg, IDC_STATUSMSG, FALSE);
			EnableDlgItem(hwndDlg, IDC_ENABLESTSMSG, FALSE);
			EnableDlgItem(hwndDlg, IDC_CHGSTATUS, FALSE);
			EnableDlgItem(hwndDlg, IDC_DNDFIRST, FALSE);
			}*/

		return TRUE;
	}

	case PSM_CHANGED:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;

	case WM_COMMAND:
		if (IsDlgButtonChecked(hwndDlg, IDC_ENABLESTSMSG)) {
			EnableDlgItem(hwndDlg, IDC_STATUSMSG, TRUE);
		}
		else
		{
			EnableDlgItem(hwndDlg, IDC_STATUSMSG, FALSE);
		}

		if ((LOWORD(wParam) == IDC_LOGIN || LOWORD(wParam) == IDC_STATUSMSG || LOWORD(wParam) == IDC_PASSWORD) && (HIWORD(wParam) != EN_CHANGE || (HWND)lParam != GetFocus()))
			return 0;
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);

		break;

	case WM_NOTIFY:
	{
		switch (((LPNMHDR)lParam)->code)
		{
		case PSN_APPLY:
		{
			int reconnectRequired = 0;
			char str[512];

			GetDlgItemTextA(hwndDlg, IDC_STATUSMSG, str, SIZEOF(str));
			db_set_s(NULL, protocolname, "setstatusmsg", str);

			db_set_b(NULL, protocolname, "autosetstatusmsg", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ENABLESTSMSG));
			db_set_b(NULL, protocolname, "statuschgtype", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_CHGSTATUS));
			db_set_b(NULL, protocolname, "dndfirst", (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DNDFIRST));

			return TRUE;
		}

		}
		break;
	}
	}
	return FALSE;
}

static INT_PTR CALLBACK DlgProcOpts6(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_INITDIALOG:
	{
		addgameDialog = NULL;

		TranslateDialogDefault(hwndDlg);

		//elemente erstmal abstellen
		EnableDlgItem(hwndDlg, IDC_DONTDETECT, FALSE);
		EnableDlgItem(hwndDlg, IDC_NOSTATUSMSG, FALSE);
		EnableDlgItem(hwndDlg, IDC_NOTINSTARTMENU, FALSE);
		EnableDlgItem(hwndDlg, IDC_APPLY, FALSE);
		EnableDlgItem(hwndDlg, IDC_EXTRAPARAMS, FALSE);
		ShowWindow(GetDlgItem(hwndDlg, IDC_EDITGAME), SW_HIDE);

		//gamelist füllen
		SendMessage(hwndDlg, WM_FILLGAMELIST, 0, 0);

		//SendMessage(GetDlgItem(hwndDlg,IDC_CREATETXTLIST),BM_SETIMAGE,IMAGE_ICON,(WPARAM)LoadSkinnedIcon(SKINICON_OTHER_USERDETAILS));

		return TRUE;
	}
	case WM_FILLGAMELIST:
	{
		//spielliste leeren
		SendDlgItemMessage((HWND)hwndDlg, IDC_LGAMELIST, LB_RESETCONTENT, 0, 0);
		//spiele auslesen und in die liste einfügen
		int found = db_get_w(NULL, protocolname, "foundgames", 0);
		char temp[XFIRE_MAXSIZEOFGAMENAME];
		for (int i = 0; i < found; i++)
		{
			//id auslesen
			mir_snprintf(temp, SIZEOF(temp), "gameid_%d", i);
			int gameid = db_get_w(NULL, protocolname, temp, 0);
			//spielnamen auslesen
			xgamelist.getGamename(gameid, temp, XFIRE_MAXSIZEOFGAMENAME);
			//eintrag einfügen
			int idx = SendDlgItemMessageA((HWND)hwndDlg, IDC_LGAMELIST, LB_ADDSTRING, 0, (LPARAM)temp);
			//id an das element übergeben
			SendDlgItemMessage((HWND)hwndDlg, IDC_LGAMELIST, LB_SETITEMDATA, idx, gameid);
		}
		return TRUE;
	}
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDC_REMOVE)
		{
			int idx = SendDlgItemMessage(hwndDlg, IDC_LGAMELIST, LB_GETCURSEL, 0, 0);

			//was ausgewählt in der liste?
			if (idx != LB_ERR) {
				//user fragen ob er das game wirklich löschen will
				if (MessageBox(hwndDlg, TranslateT("Are you sure you want to remove this game?"), TranslateT("XFire Options"), MB_YESNO | MB_ICONQUESTION) == IDYES) {
					//gameliste blocken
					xgamelist.Block(TRUE);
					//spielid auslesen
					int gameid = SendDlgItemMessage((HWND)hwndDlg, IDC_LGAMELIST, LB_GETITEMDATA, idx, 0);
					//spiel per gameid entfernen
					xgamelist.Removegame(gameid);
					//db säubern
					xgamelist.clearDatabase(TRUE);
					//derzeitige gameliste in die datenbank eintragen
					xgamelist.writeDatabase();
					//startmenu leeren
					xgamelist.clearStartmenu();
					//startmenu neuerzeugen
					xgamelist.createStartmenu();
					//gameliste unblocken
					xgamelist.Block(FALSE);
					//gamelist neu füllen
					SendMessage(hwndDlg, WM_FILLGAMELIST, 0, 0);
					//erstes vorauswählen
					SendDlgItemMessage(hwndDlg, IDC_LGAMELIST, LB_SETCURSEL, 0, 0);
					//liste refresh
					SendMessage(hwndDlg, WM_COMMAND, MAKELONG(IDC_LGAMELIST, LBN_SELCHANGE), 0);
				}
			}
			else
				MessageBox(hwndDlg, TranslateT("Please select a game."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);

		}
		else
			if (LOWORD(wParam) == IDC_ADDGAME)
			{
			//gameliste blocken
			xgamelist.Block(TRUE);
			//if (DialogBox(hinstance,MAKEINTRESOURCE(IDD_ADDGAME),hwndDlg,DlgAddGameProc)) {
			AddGameDialog(hwndDlg);
			//gameliste unblocken
			xgamelist.Block(FALSE);
			//gamelist neu füllen
			SendMessage(hwndDlg, WM_FILLGAMELIST, 0, 0);
			}
		//copy gamelist to clipboard button entfernt
		/*else if (LOWORD(wParam)==IDC_CREATETXTLIST) //gameliste als textform für debugging erstellen
		{
		//gameliste blocken
		xgamelist.Block(TRUE);

		//alle games durchgehen
		Xfire_game* nextgame;
		//output string
		char* out=new char[10];
		xgamelist.setString("Xfire-gamelist:\r\n",&out);
		while(xgamelist.getnextGame(&nextgame))
		{
		if (nextgame->name)
		{
		xgamelist.appendString("\r\nName: ",&out);
		xgamelist.appendString(nextgame->name,&out);
		}
		if (nextgame->path)
		{
		xgamelist.appendString("\r\nPath: ",&out);
		xgamelist.appendString(nextgame->path,&out);
		}
		if (nextgame->launchparams)
		{
		xgamelist.appendString("\r\nLaunch: ",&out);
		xgamelist.appendString(nextgame->launchparams,&out);
		}

		char temp[10];
		_itoa(nextgame->id,temp,10);
		xgamelist.appendString("\r\nId: ",&out);
		xgamelist.appendString(temp,&out);

		_itoa(nextgame->send_gameid,temp,10);
		xgamelist.appendString("\r\nSend-Id: ",&out);
		xgamelist.appendString(temp,&out);

		if (nextgame->skip)
		{
		xgamelist.appendString("\r\nThis game will be skipped in game detection!",&out);
		}

		xgamelist.appendString("\r\n",&out);
		}

		if (OpenClipboard(NULL))
		{
		HGLOBAL clipbuffer;
		char* buffer;

		EmptyClipboard();
		clipbuffer = GlobalAlloc(GMEM_DDESHARE, strlen(out)+1);
		buffer = (char*)GlobalLock(clipbuffer);
		strcpy(buffer, LPCSTR(out));
		GlobalUnlock(clipbuffer);

		SetClipboardData(CF_TEXT, clipbuffer);
		CloseClipboard();
		}

		if (out!=NULL) delete[] out;

		//gameliste unblocken
		xgamelist.Block(FALSE);
		}*/
			else if (LOWORD(wParam) == IDC_EDITGAME) {
				int idx = SendDlgItemMessage(hwndDlg, IDC_LGAMELIST, LB_GETCURSEL, 0, 0);

				//was ausgewählt in der liste?
				if (idx != LB_ERR) {
					//gameliste blocken
					xgamelist.Block(TRUE);
					//gameid der aktuellen auswahl auslesen
					int gameid = SendDlgItemMessage((HWND)hwndDlg, IDC_LGAMELIST, LB_GETITEMDATA, idx, 0);
					//spielobject holen
					Xfire_game* tempgame = xgamelist.getGamebyGameid(gameid);
					//gültiger verweis?
					if (tempgame) {
						//editmodus des addgamedialog
						AddGameDialog(hwndDlg, tempgame);
						//elemente wieder unsichtbar machen
						EnableDlgItem(hwndDlg, IDC_DONTDETECT, FALSE);
						EnableDlgItem(hwndDlg, IDC_NOSTATUSMSG, FALSE);
						EnableDlgItem(hwndDlg, IDC_NOTINSTARTMENU, FALSE);
						EnableDlgItem(hwndDlg, IDC_APPLY, FALSE);
						EnableDlgItem(hwndDlg, IDC_EXTRAPARAMS, FALSE);
						ShowWindow(GetDlgItem(hwndDlg, IDC_EDITGAME), SW_HIDE);
					}
					else
						MessageBox(hwndDlg, TranslateT("Error: unknown game ID."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
					//gameliste unblocken
					xgamelist.Block(FALSE);
					//gamelist neu füllen
					SendMessage(hwndDlg, WM_FILLGAMELIST, 0, 0);
				}
				else
					MessageBox(hwndDlg, TranslateT("Please select a game."), TranslateT("XFire Options"), MB_OK | MB_ICONEXCLAMATION);
			}
			else //wurde ein spiel aus der liste gewählt?
				if (HIWORD(wParam) == LBN_SELCHANGE && LOWORD(wParam) == IDC_LGAMELIST)
				{
				int idx = SendDlgItemMessage(hwndDlg, IDC_LGAMELIST, LB_GETCURSEL, 0, 0);
				//es wurde was ausgewählt?
				if (idx != LB_ERR)
				{
					//textlänge auslesen
					int size = SendDlgItemMessage(hwndDlg, IDC_LGAMELIST, LB_GETTEXTLEN, idx, 0);
					//textbuffer anlegen
					char* text = new char[size + 1];
					SendDlgItemMessageA(hwndDlg, IDC_LGAMELIST, LB_GETTEXT, idx, (LPARAM)text);
					SetDlgItemTextA(hwndDlg, IDC_GAMENAME, text);
					//textbuffer löschen
					if (text != NULL)
					{
						delete text;
						text = NULL;
					}
					//id des spielsbekommen
					int gameid = SendDlgItemMessage((HWND)hwndDlg, IDC_LGAMELIST, LB_GETITEMDATA, idx, 0);

					HICON hicon = xgamelist.iconmngr.getGameIcon(gameid);
					//iconhandle holen und setzen
					if (hicon)
						SendMessage(GetDlgItem(hwndDlg, IDC_GAMEICO), STM_SETICON, (WPARAM)hicon, 0);
					else
						SendMessage(GetDlgItem(hwndDlg, IDC_GAMEICO), STM_SETICON, 0, 0);

					//elemente aktivieren
					EnableDlgItem(hwndDlg, IDC_DONTDETECT, TRUE);
					EnableDlgItem(hwndDlg, IDC_NOSTATUSMSG, TRUE);
					EnableDlgItem(hwndDlg, IDC_APPLY, TRUE);
					EnableDlgItem(hwndDlg, IDC_NOTINSTARTMENU, TRUE);
					EnableDlgItem(hwndDlg, IDC_EXTRAPARAMS, TRUE);

					Xfire_game* xgtemp = xgamelist.getGamebyGameid(gameid);
					if (xgtemp && xgtemp->custom)
					{
						ShowWindow(GetDlgItem(hwndDlg, IDC_MANADDED), SW_SHOW);
						ShowWindow(GetDlgItem(hwndDlg, IDC_EDITGAME), SW_SHOW);
					}
					else
					{
						ShowWindow(GetDlgItem(hwndDlg, IDC_MANADDED), SW_HIDE);
						ShowWindow(GetDlgItem(hwndDlg, IDC_EDITGAME), SW_HIDE);
					}

					//gameskip wert setzen
					char temp[64] = "";
					mir_snprintf(temp, SIZEOF(temp), "gameskip_%d", gameid);
					CheckDlgButton(hwndDlg, IDC_DONTDETECT, db_get_b(NULL, protocolname, temp, 0));
					mir_snprintf(temp, SIZEOF(temp), "gamenostatus_%d", gameid);
					CheckDlgButton(hwndDlg, IDC_NOSTATUSMSG, db_get_b(NULL, protocolname, temp, 0));
					mir_snprintf(temp, SIZEOF(temp), "notinstartmenu_%d", gameid);
					CheckDlgButton(hwndDlg, IDC_NOTINSTARTMENU, db_get_b(NULL, protocolname, temp, 0));

					//extra parameter auslesen, aber nur, wenn das spiel auch sowas unterstützt
					if (xgtemp && xgtemp->haveExtraGameArgs())
					{
						EnableDlgItem(hwndDlg, IDC_EXTRAPARAMS, TRUE);
						mir_snprintf(temp, SIZEOF(temp), "gameextraparams_%d", gameid);
						DBVARIANT dbv;
						if (!db_get(NULL, protocolname, temp, &dbv))
						{
							SetDlgItemTextA(hwndDlg, IDC_EXTRAPARAMS, dbv.pszVal);
							db_free(&dbv);
						}
						else
							SetDlgItemText(hwndDlg, IDC_EXTRAPARAMS, _T(""));
					}
					else
					{
						EnableDlgItem(hwndDlg, IDC_EXTRAPARAMS, FALSE);
						SetDlgItemText(hwndDlg, IDC_EXTRAPARAMS, TranslateT("Not supported"));
					}
				}
				}
				else if (LOWORD(wParam) == IDC_APPLY)
				{
					//auswahl speichern
					int idx = SendDlgItemMessage(hwndDlg, IDC_LGAMELIST, LB_GETCURSEL, 0, 0);
					//es wurde was ausgewählt?
					if (idx != LB_ERR)
					{
						int gameid = SendDlgItemMessage((HWND)hwndDlg, IDC_LGAMELIST, LB_GETITEMDATA, idx, 0);
						int dbid;

						//gamelist blocken
						xgamelist.Block(TRUE);

						if (xgamelist.Gameinlist(gameid, &dbid))
						{
							Xfire_game* game = xgamelist.getGame(dbid);
							if (game)
							{
								game->skip = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_DONTDETECT);
								game->noicqstatus = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOSTATUSMSG);
								game->notinstartmenu = (BYTE)IsDlgButtonChecked(hwndDlg, IDC_NOTINSTARTMENU);

								//extra parameter auslesen und das gameobj schreiben
								char str[128] = "";
								GetDlgItemTextA(hwndDlg, IDC_EXTRAPARAMS, str, SIZEOF(str));
								if (str[0] != 0)
								{
									//extra parameter sind gesetzt, zuweisen
									game->setString(str, &game->extraparams);
								}
								else
								{
									//extra parameter leer, wenn gesetzt entfernen/freigeben
									if (game->extraparams)
									{
										delete[] game->extraparams;
										game->extraparams = NULL;
									}
								}


								game->refreshMenuitem();
								game->writeToDB(dbid);

								SetDlgItemText(hwndDlg, IDC_TEXTSTATUS, TranslateT("Configuration saved!"));
							}
							else
							{
								SetDlgItemText(hwndDlg, IDC_TEXTSTATUS, TranslateT("Game not found?!"));
							}
						}
						else
						{
							SetDlgItemText(hwndDlg, IDC_TEXTSTATUS, TranslateT("Game not found?!"));
						}

						//gamelist unblocken
						xgamelist.Block(FALSE);
					}
				}
			break;
	}

	case WM_NOTIFY:
	{
		break;
	}
	}
	return FALSE;
}

int OptInit(WPARAM wParam, LPARAM)
{
	ghwndDlg2 = NULL;

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hinstance;
	odp.ptszTitle = LPGENT("XFire");
	odp.ptszGroup = LPGENT("Network");
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;

	odp.pfnDlgProc = DlgProcOpts2;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTLOGIN);
	odp.ptszTab = LPGENT("Account");
	Options_AddPage(wParam, &odp);

	odp.pfnDlgProc = DlgProcOpts3;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTFEAT);
	odp.ptszTab = LPGENT("Features");
	Options_AddPage(wParam, &odp);

	odp.pfnDlgProc = DlgProcOpts4;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTFEAT2);
	odp.ptszTab = LPGENT("Blocklist / Games");
	Options_AddPage(wParam, &odp);

	odp.pfnDlgProc = DlgProcOpts5;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTFEAT3);
	odp.ptszTab = LPGENT("StatusMsg");
	Options_AddPage(wParam, &odp);

	odp.pfnDlgProc = DlgProcOpts6;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTFEAT4);
	odp.ptszTab = LPGENT("Games");
	Options_AddPage(wParam, &odp);
	return 0;
}
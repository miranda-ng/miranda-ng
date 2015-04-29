// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2008 Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Implements Manage Server List dialog
// -----------------------------------------------------------------------------

#include "stdafx.h"

static int bListInit = 0;
static HANDLE hItemAll;
static int dwUploadDelay = 1000; // initial setting, it is too low for icq server but good for short updates

static HWND hwndUploadContacts=NULL;
static const UINT settingsControls[]={IDOK};

static WORD* pwGroupIds = NULL;
static int cbGroupIds = 0;

// Init default clist options
static void ResetCListOptions(HWND hwndList)
{
	SetWindowLongPtr(hwndList, GWL_STYLE, GetWindowLongPtr(hwndList, GWL_STYLE)|CLS_SHOWHIDDEN);
	if (CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_HIDEEMPTYGROUPS) // hide empty groups
		SendMessage(hwndList, CLM_SETHIDEEMPTYGROUPS, (WPARAM) TRUE, 0);
}

// Selects the "All contacts" checkbox if all other list entries
// are selected, deselects it if not.
static void UpdateAllContactsCheckmark(HWND hwndList, CIcqProto* ppro, HANDLE phItemAll)
{
	int check = 1;

	MCONTACT hContact = db_find_first(ppro->m_szModuleName);
	while (hContact)
	{
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		if (hItem)
		{
			if (!SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
			{ // if any of our contacts is unchecked, uncheck all contacts as well
				check = 0;
				break;
			}
		}
		hContact = db_find_next(hContact, ppro->m_szModuleName);
	}

	SendMessage(hwndList, CLM_SETCHECKMARK, (WPARAM)phItemAll, check);
}

// Loop over all contacts and update the checkmark
// that indicates wether or not they are already uploaded
static int UpdateCheckmarks(HWND hwndList, CIcqProto* ppro, HANDLE phItemAll)
{
	int bAll = 1;
	bListInit = 1; // lock CLC events

	MCONTACT hContact = db_find_first(ppro->m_szModuleName);
	while (hContact)
	{
		HANDLE hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
		if (hItem)
		{
			if (ppro->getWord(hContact, DBSETTING_SERVLIST_ID, 0))
				SendMessage(hwndList, CLM_SETCHECKMARK, (WPARAM)hItem, 1);
			else
				bAll = 0;
		}
		hContact = db_find_next(hContact, ppro->m_szModuleName);
	}

	// Update the "All contacts" checkmark
	if (phItemAll)
		SendMessage(hwndList, CLM_SETCHECKMARK, (WPARAM)phItemAll, bAll);

	bListInit = 0;

	return bAll;
}

static void DeleteOtherContactsFromControl(HWND hCtrl, CIcqProto* ppro)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		HANDLE hItem = (HANDLE)SendMessage(hCtrl, CLM_FINDCONTACT, hContact, 0);
		if (hItem)
			if (!ppro->IsICQContact(hContact))
				SendMessage(hCtrl, CLM_DELETEITEM, (WPARAM)hItem, 0);
	}
}

static void AppendToUploadLog(HWND hwndDlg, const char *fmt, ...)
{
	va_list va;
	char szText[1024];
	int iItem;

	va_start(va, fmt);
	mir_vsnprintf(szText, sizeof(szText), fmt, va);
	va_end(va);

	iItem = ListBoxAddStringUtf(GetDlgItem(hwndDlg, IDC_LOG), szText);
	SendDlgItemMessage(hwndDlg, IDC_LOG, LB_SETTOPINDEX, iItem, 0);
}

static void DeleteLastUploadLogLine(HWND hwndDlg)
{
	SendDlgItemMessage(hwndDlg, IDC_LOG, LB_DELETESTRING, SendDlgItemMessage(hwndDlg, IDC_LOG, LB_GETCOUNT, 0, 0)-1, 0);
}

static void GetLastUploadLogLine(HWND hwndDlg, char *szBuf, size_t cbBuf)
{
	WCHAR str[MAX_PATH];
	SendDlgItemMessageW(hwndDlg, IDC_LOG, LB_GETTEXT, SendDlgItemMessage(hwndDlg, IDC_LOG, LB_GETCOUNT, 0, 0)-1, (LPARAM)str);
	make_utf8_string_static(str, szBuf, cbBuf);
}

static int GroupEnumIdsEnumProc(const char *szSetting,LPARAM lParam)
{ 
	// it is probably server group
	if (szSetting && mir_strlen(szSetting) < 5) {
		char val[MAX_PATH+2]; // dummy
		if (db_get_static(NULL, (char*)lParam, szSetting, val, MAX_PATH))
			return 0; // this converts all string types to DBVT_ASCIIZ

		pwGroupIds = (WORD*)SAFE_REALLOC(pwGroupIds, (cbGroupIds+1)*sizeof(WORD));
		pwGroupIds[cbGroupIds] = (WORD)strtoul(szSetting, NULL, 0x10);
		cbGroupIds++;
	}
	return 0;
}

static void enumServerGroups(CIcqProto* ppro)
{
	char szModule[MAX_PATH+9];
	mir_snprintf(szModule, SIZEOF(szModule), "%s%s", ppro->m_szModuleName, "SrvGroups");

	DBCONTACTENUMSETTINGS dbces = { 0 };
	dbces.pfnEnumProc = &GroupEnumIdsEnumProc;
	dbces.szModule = szModule;
	dbces.lParam = (LPARAM)szModule;
	CallService(MS_DB_CONTACT_ENUMSETTINGS, 0, (LPARAM)&dbces);
}

static DWORD sendUploadGroup(CIcqProto* ppro, WORD wAction, WORD wGroupId, char* szItemName)
{
	DWORD dwCookie;
	cookie_servlist_action* ack;

	if (ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action)))
	{ // we have cookie good, go on
		ack->wGroupId = wGroupId;
		ack->dwAction = SSA_SERVLIST_ACK;
		dwCookie = ppro->AllocateCookie(CKT_SERVERLIST, wAction, 0, ack);
		ack->lParam = dwCookie;

		ppro->icq_sendServerGroup(dwCookie, wAction, ack->wGroupId, szItemName, NULL, 0, 0);
		return dwCookie;
	}
	return 0;
}

static DWORD sendUploadBuddy(CIcqProto* ppro, MCONTACT hContact, WORD wAction, DWORD dwUin, char *szUID, WORD wContactId, WORD wGroupId, WORD wItemType)
{
	DWORD dwCookie;
	cookie_servlist_action* ack;

	if (ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action)))
	{ // we have cookie good, go on
		ack->hContact = hContact;
		ack->wContactId = wContactId;
		ack->wGroupId = wGroupId;
		ack->dwAction = SSA_SERVLIST_ACK;
		dwCookie = ppro->AllocateCookie(CKT_SERVERLIST, wAction, hContact, ack);
		ack->lParam = dwCookie;

		if (wItemType == SSI_ITEM_BUDDY)
			ppro->icq_sendServerContact(hContact, dwCookie, wAction, ack->wGroupId, ack->wContactId, SSOP_ITEM_ACTION | SSOF_CONTACT, 500, NULL);
		else
			ppro->icq_sendSimpleItem(dwCookie, wAction, dwUin, szUID, ack->wGroupId, ack->wContactId, wItemType, SSOP_ITEM_ACTION, 500);

		return dwCookie;
	}
	return 0;
}

static char* getServerResultDesc(int wCode)
{
	switch (wCode)
	{
		case 0:   return LPGEN("OK");
		case 2:   return LPGEN("NOT FOUND");
		case 3:   return LPGEN("ALREADY EXISTS");
		case 0xA: return LPGEN("INVALID DATA");
		case 0xC: return LPGEN("LIST FULL");
		default:  return LPGEN("FAILED");
	}
}

#define ACTION_NONE             0
#define ACTION_ADDBUDDY         1
#define ACTION_ADDBUDDYAUTH     2
#define ACTION_REMOVEBUDDY      3
#define ACTION_ADDGROUP         4
#define ACTION_REMOVEGROUP      5
#define ACTION_UPDATESTATE      6
#define ACTION_MOVECONTACT      7
#define ACTION_ADDVISIBLE       8
#define ACTION_REMOVEVISIBLE    9
#define ACTION_ADDINVISIBLE     10
#define ACTION_REMOVEINVISIBLE  11

#define STATE_READY         1
#define STATE_REGROUP       2
#define STATE_ITEMS         3
#define STATE_VISIBILITY    5
#define STATE_CONSOLIDATE   4

#define M_PROTOACK      (WM_USER+100)
#define M_UPLOADMORE    (WM_USER+101)
#define M_INITCLIST     (WM_USER+102)

static INT_PTR CALLBACK DlgProcUploadList(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	CIcqProto* ppro = (CIcqProto*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);

	static int working;
	static HANDLE hProtoAckHook;
	static int currentSequence;
	static int currentAction;
	static int currentState;
	static MCONTACT hCurrentContact;
	static int lastAckResult = 0;
	static WORD wNewContactId;
	static WORD wNewGroupId;
	static char *szNewGroupName;
	static WORD wNewVisibilityId;

	switch (message) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);

		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, lParam);
		{
			char str[MAX_PATH];

			working = 0;
			hProtoAckHook = NULL;
			currentState = STATE_READY;

			ResetCListOptions(GetDlgItem(hwndDlg, IDC_CLIST));

			AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Select contacts you want to store on server."), str, MAX_PATH));
			AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Ready..."), str, MAX_PATH));
		}
		return TRUE;

		// The M_PROTOACK message is received when the
		// server has responded to our last update packet
	case M_PROTOACK:
	{
		int bMulti = 0;
		ACKDATA *ack = (ACKDATA*)lParam;
		char szLastLogLine[MAX_PATH];
		char str[MAX_PATH];

		// Is this an ack we are waiting for?
		if (strcmpnull(ack->szModule, ppro->m_szModuleName))
			break;

		if (ack->type == ICQACKTYPE_RATEWARNING) { // we are sending tooo fast, slow down the process
			if (ack->hProcess != (HANDLE)1) break; // check class
			if (ack->lParam == 2 || ack->lParam == 3) // check status
			{
				GetLastUploadLogLine(hwndDlg, szLastLogLine, MAX_PATH);
				DeleteLastUploadLogLine(hwndDlg);
				AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Server rate warning -> slowing down the process."), str, MAX_PATH));
				AppendToUploadLog(hwndDlg, szLastLogLine);

				dwUploadDelay *= 2;

				break;
			}
			if (ack->lParam == 4) dwUploadDelay /= 2; // the rate is ok, turn up
		}

		if (ack->type != ICQACKTYPE_SERVERCLIST)
			break;

		if ((int)ack->hProcess != currentSequence)
			break;

		lastAckResult = ack->result == ACKRESULT_SUCCESS ? 0 : 1;

		switch (currentAction) {
		case ACTION_ADDBUDDY:
			if (ack->result == ACKRESULT_SUCCESS) {
				ppro->setByte(hCurrentContact, "Auth", 0);
				ppro->setWord(hCurrentContact, DBSETTING_SERVLIST_ID, wNewContactId);
				ppro->setWord(hCurrentContact, DBSETTING_SERVLIST_GROUP, wNewGroupId);
				break;
			}
			else { // If the server refused to add the contact without authorization,
				// we try again _with_ authorization TLV
				ppro->setByte(hCurrentContact, "Auth", 1);

				DWORD dwUIN;
				uid_str szUID;
				if (!ppro->getContactUid(hCurrentContact, &dwUIN, &szUID)) {
					currentAction = ACTION_ADDBUDDYAUTH;
					currentSequence = sendUploadBuddy(ppro, hCurrentContact, ICQ_LISTS_ADDTOLIST, dwUIN, szUID, wNewContactId, wNewGroupId, SSI_ITEM_BUDDY);
				}

				return FALSE;
			}

		case ACTION_ADDBUDDYAUTH:
			if (ack->result == ACKRESULT_SUCCESS) {
				ppro->setWord(hCurrentContact, DBSETTING_SERVLIST_ID, wNewContactId);
				ppro->setWord(hCurrentContact, DBSETTING_SERVLIST_GROUP, wNewGroupId);
			}
			else {
				db_unset(hCurrentContact, ppro->m_szModuleName, "Auth");
				ppro->FreeServerID(wNewContactId, SSIT_ITEM);
			}

			break;

		case ACTION_REMOVEBUDDY:
			if (ack->result == ACKRESULT_SUCCESS) { // clear obsolete settings
				ppro->FreeServerID(wNewContactId, SSIT_ITEM);
				db_unset(hCurrentContact, ppro->m_szModuleName, DBSETTING_SERVLIST_ID);
				db_unset(hCurrentContact, ppro->m_szModuleName, DBSETTING_SERVLIST_GROUP);
				db_unset(hCurrentContact, ppro->m_szModuleName, "Auth");
			}
			break;

		case ACTION_ADDGROUP:
			if (ack->result == ACKRESULT_SUCCESS) {
				void* groupData;
				int groupSize;
				cookie_servlist_action* ack;

				ppro->setServListGroupName(wNewGroupId, szNewGroupName); // add group to list
				ppro->setServListGroupLinkID(szNewGroupName, wNewGroupId); // grouppath is known

				groupData = ppro->collectGroups(&groupSize);
				groupData = SAFE_REALLOC(groupData, groupSize + 2);
				*(((WORD*)groupData) + (groupSize >> 1)) = wNewGroupId; // add this new group id
				groupSize += 2;

				ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
				if (ack) {
					DWORD dwCookie; // we do not use this

					ack->dwAction = SSA_SERVLIST_ACK;
					dwCookie = ppro->AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_UPDATEGROUP, 0, ack);

					ppro->icq_sendServerGroup(dwCookie, ICQ_LISTS_UPDATEGROUP, 0, ack->szGroupName, groupData, groupSize, 0);
				}
				SAFE_FREE((void**)&groupData);
			}
			else
				ppro->FreeServerID(wNewGroupId, SSIT_GROUP);

			SAFE_FREE((void**)&szNewGroupName);
			break;

		case ACTION_REMOVEGROUP:
			if (ack->result == ACKRESULT_SUCCESS) {
				void* groupData;
				int groupSize;
				cookie_servlist_action* ack;

				ppro->FreeServerID(wNewGroupId, SSIT_GROUP);
				ppro->setServListGroupName(wNewGroupId, NULL); // remove group from list
				ppro->removeGroupPathLinks(wNewGroupId); // grouppath is known

				groupData = ppro->collectGroups(&groupSize);

				ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));
				if (ack) {
					DWORD dwCookie; // we do not use this

					ack->dwAction = SSA_SERVLIST_ACK;
					dwCookie = ppro->AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_UPDATEGROUP, 0, ack);

					ppro->icq_sendServerGroup(dwCookie, ICQ_LISTS_UPDATEGROUP, 0, ack->szGroupName, groupData, groupSize, 0);
				}
				SAFE_FREE((void**)&groupData);
			}
			break;

		case ACTION_UPDATESTATE:
			// do nothing
			break;

		case ACTION_MOVECONTACT:
			if (ack->result == ACKRESULT_SUCCESS) {
				ppro->FreeServerID(ppro->getWord(hCurrentContact, DBSETTING_SERVLIST_ID, 0), SSIT_ITEM);
				ppro->setWord(hCurrentContact, DBSETTING_SERVLIST_ID, wNewContactId);
				ppro->setWord(hCurrentContact, DBSETTING_SERVLIST_GROUP, wNewGroupId);
				dwUploadDelay *= 2; // we double the delay here (2 packets)
			}
			break;

		case ACTION_ADDVISIBLE:
			if (ack->result == ACKRESULT_SUCCESS)
				ppro->setWord(hCurrentContact, DBSETTING_SERVLIST_PERMIT, wNewContactId);
			else
				ppro->FreeServerID(wNewContactId, SSIT_ITEM);
			break;

		case ACTION_ADDINVISIBLE:
			if (ack->result == ACKRESULT_SUCCESS)
				ppro->setWord(hCurrentContact, DBSETTING_SERVLIST_DENY, wNewContactId);
			else
				ppro->FreeServerID(wNewContactId, SSIT_ITEM);
			break;

		case ACTION_REMOVEVISIBLE:
			if (ack->result == ACKRESULT_SUCCESS) {
				ppro->FreeServerID(wNewContactId, SSIT_ITEM);
				ppro->setWord(hCurrentContact, DBSETTING_SERVLIST_PERMIT, 0);
			}
			break;

		case ACTION_REMOVEINVISIBLE:
			if (ack->result == ACKRESULT_SUCCESS) {
				ppro->FreeServerID(wNewContactId, SSIT_ITEM);
				ppro->setWord(hCurrentContact, DBSETTING_SERVLIST_DENY, 0);
			}
			break;
		}

		// Update the log window
		GetLastUploadLogLine(hwndDlg, szLastLogLine, MAX_PATH);
		DeleteLastUploadLogLine(hwndDlg);
		AppendToUploadLog(hwndDlg, "%s%s", szLastLogLine,
								ICQTranslateUtfStatic(getServerResultDesc(ack->lParam), str, MAX_PATH));

		if (!bMulti) {
			SetTimer(hwndDlg, M_UPLOADMORE, dwUploadDelay, 0); // delay
		}
	}
		break;

	case WM_TIMER:
	{
		switch (wParam) {
		case M_UPLOADMORE:
			KillTimer(hwndDlg, M_UPLOADMORE);
			if (currentAction == ACTION_MOVECONTACT)
				dwUploadDelay /= 2; // turn it back

			PostMessage(hwndDlg, M_UPLOADMORE, 0, 0);

			return 0;
		}
	}

		// The M_UPLOADMORE window message is received when the user presses 'Update'
		// and every time an ack from the server has been taken care of.
	case M_UPLOADMORE:
		{
			MCONTACT hContact;
			HANDLE hItem;
			char *pszNick;
			char *pszGroup;
			int isChecked;
			int isOnServer;
			BOOL bUidOk;
			char str[MAX_PATH];
			HWND hwndList = GetDlgItem(hwndDlg, IDC_CLIST);

			switch (currentState) {
			case STATE_REGROUP:

				// TODO: iterate over all checked groups and create if needed
				// if creation requires reallocation of groups do it here

				currentState = STATE_ITEMS;
				hCurrentContact = NULL;
				PostMessage(hwndDlg, M_UPLOADMORE, 0, 0);
				break;

			case STATE_ITEMS:
				// Iterate over all contacts until one is found that
				// needs to be updated on the server
				if (hCurrentContact == NULL)
					hContact = db_find_first(ppro->m_szModuleName);
				else { // we do not want to go thru all contacts over and over again
					hContact = hCurrentContact;
					if (lastAckResult) // if the last operation on this contact fail, do not do it again, go to next
						hContact = db_find_next(hContact, ppro->m_szModuleName);
				}

				while (hContact) {
					hCurrentContact = hContact;

					hItem = (HANDLE)SendMessage(hwndList, CLM_FINDCONTACT, hContact, 0);
					if (hItem) {
						isChecked = SendMessage(hwndList, CLM_GETCHECKMARK, (WPARAM)hItem, 0) != 0;
						isOnServer = ppro->getWord(hContact, DBSETTING_SERVLIST_ID, 0) != 0;

						DWORD dwUin;
						uid_str szUid;
						bUidOk = !ppro->getContactUid(hContact, &dwUin, &szUid);

						// Is this one out of sync?
						if (bUidOk && (isChecked != isOnServer)) {
							// Only upload custom nicks
							pszNick = ppro->getSettingStringUtf(hContact, "CList", "MyHandle", NULL);

							if (isChecked) {  // Queue for uploading
								pszGroup = ppro->getContactCListGroup(hContact);
								if (!mir_strlen(pszGroup))
									pszGroup = null_strdup(DEFAULT_SS_GROUP);

								// Get group ID from cache, if not ready use parent group, if still not ready create one
								wNewGroupId = ppro->getServListGroupLinkID(pszGroup);
								if (!wNewGroupId && strstrnull(pszGroup, "\\") != NULL) { // if it is sub-group, take master parent
									strstrnull(pszGroup, "\\")[0] = '\0';
									wNewGroupId = ppro->getServListGroupLinkID(pszGroup);
								}
								if (!wNewGroupId && currentAction != ACTION_ADDGROUP) { // if the group still does not exist and there was no try before, try to add group
									AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Adding group \"%s\"..."), str, MAX_PATH), pszGroup);

									wNewGroupId = ppro->GenerateServerID(SSIT_GROUP, 0); // ???
									szNewGroupName = pszGroup;
									currentAction = ACTION_ADDGROUP;
									currentSequence = sendUploadGroup(ppro, ICQ_LISTS_ADDTOLIST, wNewGroupId, pszGroup);
									SAFE_FREE(&pszNick);
									return FALSE;
								}

								SAFE_FREE(&pszGroup);

								AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Uploading %s..."), str, MAX_PATH), pszNick ? pszNick : strUID(dwUin, szUid));

								currentAction = ACTION_ADDBUDDY;

								if (wNewGroupId) {
									wNewContactId = ppro->GenerateServerID(SSIT_ITEM, 0);

									currentSequence = sendUploadBuddy(ppro, hCurrentContact, ICQ_LISTS_ADDTOLIST, dwUin, szUid,
																				 wNewContactId, wNewGroupId, SSI_ITEM_BUDDY);
									SAFE_FREE(&pszNick);
									return FALSE;
								}
								
								char szLastLogLine[MAX_PATH];
								// Update the log window with the failure and continue with next contact
								GetLastUploadLogLine(hwndDlg, szLastLogLine, MAX_PATH);
								DeleteLastUploadLogLine(hwndDlg);
								AppendToUploadLog(hwndDlg, "%s%s", szLastLogLine, ICQTranslateUtfStatic(LPGEN("FAILED"), str, MAX_PATH));
								AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("No upload group available"), str, MAX_PATH));
								ppro->debugLogA("Upload failed, no group");
								currentState = STATE_READY;
							}
							else { // Queue for deletion
								if (pszNick)
									AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Deleting %s..."), str, MAX_PATH), pszNick);
								else
									AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Deleting %s..."), str, MAX_PATH), strUID(dwUin, szUid));

								wNewGroupId = ppro->getWord(hContact, DBSETTING_SERVLIST_GROUP, 0);
								wNewContactId = ppro->getWord(hContact, DBSETTING_SERVLIST_ID, 0);
								currentAction = ACTION_REMOVEBUDDY;
								currentSequence = sendUploadBuddy(ppro, hContact, ICQ_LISTS_REMOVEFROMLIST, dwUin, szUid,
																			 wNewContactId, wNewGroupId, SSI_ITEM_BUDDY);
							}
							SAFE_FREE((void**)&pszNick);
							break;
						}

						// the contact is and should be on server, check if it is in correct group, move otherwise
						if (bUidOk && isChecked) {
							WORD wCurrentGroupId = ppro->getWord(hContact, DBSETTING_SERVLIST_GROUP, 0);

							pszGroup = ppro->getContactCListGroup(hContact);
							if (!mir_strlen(pszGroup))
								pszGroup = null_strdup(DEFAULT_SS_GROUP);
							wNewGroupId = ppro->getServListGroupLinkID(pszGroup);
							if (!wNewGroupId && strstrnull(pszGroup, "\\") != NULL) { // if it is sub-group, take master parent
								strstrnull(pszGroup, "\\")[0] = '\0';
								wNewGroupId = ppro->getServListGroupLinkID(pszGroup);
							}

							// if the group still does not exist and there was no try before, try to add group
							if (!wNewGroupId && currentAction != ACTION_ADDGROUP) {
								AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Adding group \"%s\"..."), str, MAX_PATH), pszGroup);

								wNewGroupId = ppro->GenerateServerID(SSIT_GROUP, 0);
								szNewGroupName = pszGroup;
								currentAction = ACTION_ADDGROUP;
								currentSequence = sendUploadGroup(ppro, ICQ_LISTS_ADDTOLIST, wNewGroupId, pszGroup);
								return FALSE;
							}

							// we have a group the contact should be in, move it
							if (wNewGroupId && (wNewGroupId != wCurrentGroupId)) {
								WORD wCurrentContactId = ppro->getWord(hContact, DBSETTING_SERVLIST_ID, 0);

								pszNick = ppro->getSettingStringUtf(hContact, "CList", "MyHandle", NULL);
								if (pszNick)
									AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Moving %s to group \"%s\"..."), str, MAX_PATH), pszNick, pszGroup);
								else
									AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Moving %s to group \"%s\"..."), str, MAX_PATH), strUID(dwUin, szUid), pszGroup);

								currentAction = ACTION_MOVECONTACT;
								wNewContactId = ppro->GenerateServerID(SSIT_ITEM, 0);
								sendUploadBuddy(ppro, hContact, ICQ_LISTS_REMOVEFROMLIST, dwUin, szUid, wCurrentContactId, wCurrentGroupId, SSI_ITEM_BUDDY);
								currentSequence = sendUploadBuddy(ppro, hContact, ICQ_LISTS_ADDTOLIST, dwUin, szUid, wNewContactId, wNewGroupId, SSI_ITEM_BUDDY);
								SAFE_FREE((void**)&pszNick);
								SAFE_FREE((void**)&pszGroup);

								break;
							}
							SAFE_FREE((void**)&pszGroup);
						}
					}
					hContact = db_find_next(hContact);
				}
				if (!hContact) {
					currentState = STATE_VISIBILITY;
					hCurrentContact = NULL;
					PostMessage(hwndDlg, M_UPLOADMORE, 0, 0);
				}
				break;

			case STATE_VISIBILITY:
				// Iterate over all contacts until one is found that
				// needs to be updated on the server
				if (hCurrentContact == NULL)
					hContact = db_find_first(ppro->m_szModuleName);
				else { // we do not want to go thru all contacts over and over again
					hContact = hCurrentContact;
					if (lastAckResult) // if the last operation on this contact fail, do not do it again, go to next
						hContact = db_find_next(hContact, ppro->m_szModuleName);
				}

				while (hContact) {
					WORD wApparentMode = ppro->getWord(hContact, "ApparentMode", 0);
					WORD wDenyId = ppro->getWord(hContact, DBSETTING_SERVLIST_DENY, 0);
					WORD wPermitId = ppro->getWord(hContact, DBSETTING_SERVLIST_PERMIT, 0);
					WORD wIgnoreId = ppro->getWord(hContact, DBSETTING_SERVLIST_IGNORE, 0);

					hCurrentContact = hContact;

					DWORD dwUin;
					uid_str szUid;
					ppro->getContactUid(hContact, &dwUin, &szUid);

					if (wApparentMode == ID_STATUS_ONLINE) { // contact is on the visible list
						if (wPermitId == 0) {
							currentAction = ACTION_ADDVISIBLE;
							wNewContactId = ppro->GenerateServerID(SSIT_ITEM, 0);
							AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Adding %s to visible list..."), str, MAX_PATH), strUID(dwUin, szUid));
							currentSequence = sendUploadBuddy(ppro, hContact, ICQ_LISTS_ADDTOLIST, dwUin, szUid, wNewContactId, 0, SSI_ITEM_PERMIT);
							break;
						}
					}
					if (wApparentMode == ID_STATUS_OFFLINE) { // contact is on the invisible list
						if (wDenyId == 0 && wIgnoreId == 0) {
							currentAction = ACTION_ADDINVISIBLE;
							wNewContactId = ppro->GenerateServerID(SSIT_ITEM, 0);
							AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Adding %s to invisible list..."), str, MAX_PATH), strUID(dwUin, szUid));
							currentSequence = sendUploadBuddy(ppro, hContact, ICQ_LISTS_ADDTOLIST, dwUin, szUid, wNewContactId, 0, SSI_ITEM_DENY);
							break;
						}
					}
					if (wApparentMode != ID_STATUS_ONLINE) { // contact is not on visible list
						if (wPermitId != 0) {
							currentAction = ACTION_REMOVEVISIBLE;
							wNewContactId = wPermitId;
							AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Deleting %s from visible list..."), str, MAX_PATH), strUID(dwUin, szUid));
							currentSequence = sendUploadBuddy(ppro, hContact, ICQ_LISTS_REMOVEFROMLIST, dwUin, szUid, wNewContactId, 0, SSI_ITEM_PERMIT);
							break;
						}
					}
					if (wApparentMode != ID_STATUS_OFFLINE) { // contact is not on invisible list
						if (wDenyId != 0) {
							currentAction = ACTION_REMOVEINVISIBLE;
							wNewContactId = wDenyId;
							AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Deleting %s from invisible list..."), str, MAX_PATH), strUID(dwUin, szUid));
							currentSequence = sendUploadBuddy(ppro, hContact, ICQ_LISTS_REMOVEFROMLIST, dwUin, szUid, wNewContactId, 0, SSI_ITEM_DENY);
							break;
						}
					}
					hContact = db_find_next(hContact);
				}
				if (!hContact) {
					currentState = STATE_CONSOLIDATE;
					AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Cleaning groups"), str, MAX_PATH));
					EnableDlgItem(hwndDlg, IDCANCEL, FALSE);
					enumServerGroups(ppro);
					PostMessage(hwndDlg, M_UPLOADMORE, 0, 0);
				}
				break;

			case STATE_CONSOLIDATE: // updage group data, remove redundant groups
				if (currentAction == ACTION_UPDATESTATE)
					DeleteLastUploadLogLine(hwndDlg);

				if (cbGroupIds) { // some groups in the list
					void* groupData;
					int groupSize;

					cbGroupIds--;
					wNewGroupId = pwGroupIds[cbGroupIds];

					if (groupData = ppro->collectBuddyGroup(wNewGroupId, &groupSize)) { // the group is still not empty, just update it
						char* pszGroup = ppro->getServListGroupName(wNewGroupId);
						cookie_servlist_action* ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));

						ack->dwAction = SSA_SERVLIST_ACK;
						ack->wGroupId = wNewGroupId;
						currentSequence = ppro->AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_UPDATEGROUP, 0, ack);
						ack->lParam = currentSequence;
						currentAction = ACTION_UPDATESTATE;
						AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Updating group \"%s\"..."), str, MAX_PATH), pszGroup);

						ppro->icq_sendServerGroup(currentSequence, ICQ_LISTS_UPDATEGROUP, wNewGroupId, pszGroup, groupData, groupSize, 0);

						SAFE_FREE((void**)&pszGroup);
					}
					else { // the group is empty, delete it if it does not have sub-groups
						if (!ppro->CheckServerID((WORD)(wNewGroupId + 1), 0) || ppro->getServListGroupLevel((WORD)(wNewGroupId + 1)) == 0) { // is next id an sub-group, if yes, we cannot delete this group
							char *pszGroup = ppro->getServListGroupName(wNewGroupId);
							currentAction = ACTION_REMOVEGROUP;
							AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Deleting group \"%s\"..."), str, MAX_PATH), pszGroup);
							currentSequence = sendUploadGroup(ppro, ICQ_LISTS_REMOVEFROMLIST, wNewGroupId, pszGroup);
							SAFE_FREE((void**)&pszGroup);
						}
						else { // update empty group
							char *pszGroup = ppro->getServListGroupName(wNewGroupId);
							cookie_servlist_action *ack = (cookie_servlist_action*)SAFE_MALLOC(sizeof(cookie_servlist_action));

							ack->dwAction = SSA_SERVLIST_ACK;
							ack->wGroupId = wNewGroupId;
							currentSequence = ppro->AllocateCookie(CKT_SERVERLIST, ICQ_LISTS_UPDATEGROUP, 0, ack);
							ack->lParam = currentSequence;
							currentAction = ACTION_UPDATESTATE;
							AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("Updating group \"%s\"..."), str, MAX_PATH), pszGroup);

							ppro->icq_sendServerGroup(currentSequence, ICQ_LISTS_UPDATEGROUP, wNewGroupId, pszGroup, 0, 0, 0);

							SAFE_FREE((void**)&pszGroup);
						}
					}
					SAFE_FREE((void**)&groupData); // free the memory
				}
				else { // all groups processed
					SAFE_FREE((void**)&pwGroupIds);
					currentState = STATE_READY;
				}
				break;
			}

			if (currentState == STATE_READY) {
				// All contacts are in sync
				AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("All operations complete"), str, MAX_PATH));
				EnableDlgItem(hwndDlg, IDCANCEL, TRUE);
				SetDlgItemText(hwndDlg, IDCANCEL, TranslateT("Close"));

				// end server modifications here
				ppro->servlistPostPacket(NULL, 0, SSO_END_OPERATION, 100);
				working = 0;
				UpdateCheckmarks(hwndList, ppro, hItemAll);
				if (hProtoAckHook)
					UnhookEvent(hProtoAckHook);
			}
		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			SendDlgItemMessage(hwndDlg, IDC_LOG, LB_RESETCONTENT, 0, 0);
			if (!ppro->icqOnline()) {
				char str[MAX_PATH];
				AppendToUploadLog(hwndDlg, ICQTranslateUtfStatic(LPGEN("You have to be online to synchronize the server-list!"), str, MAX_PATH));
				break;
			}
			working = 1;
			hCurrentContact = NULL;
			currentState = STATE_REGROUP;
			currentAction = ACTION_NONE;
			icq_ShowMultipleControls(hwndDlg, settingsControls, SIZEOF(settingsControls), SW_HIDE);
			//        SendDlgItemMessage(hwndDlg, IDC_CLIST, CLM_SETGREYOUTFLAGS, 0xFFFFFFFF, 0);
			//        InvalidateRect(GetDlgItem(hwndDlg, IDC_CLIST), NULL, FALSE);
			EnableDlgItem(hwndDlg, IDC_CLIST, FALSE);
			hProtoAckHook = HookEventMessage(ME_PROTO_ACK, hwndDlg, M_PROTOACK);
			// start server modifications here
			ppro->servlistPostPacket(NULL, 0, SSO_BEGIN_OPERATION | SSOF_IMPORT_OPERATION, 100);
			PostMessage(hwndDlg, M_UPLOADMORE, 0, 0);
			break;

		case IDCANCEL: // TODO: this must be clean
			DestroyWindow(hwndDlg);
			break;
		}
		break;

	case WM_NOTIFY:
		switch (((NMHDR*)lParam)->idFrom) {
		case IDC_CLIST:
			{
				HWND hClist = GetDlgItem(hwndDlg, IDC_CLIST);

				switch (((NMHDR*)lParam)->code) {
				case CLN_OPTIONSCHANGED:
					ResetCListOptions(hClist);
					break;

				case CLN_NEWCONTACT:
				case CLN_CONTACTMOVED:
					// Delete non-icq contacts
					DeleteOtherContactsFromControl(hClist, ppro);
					if (hItemAll)
						UpdateAllContactsCheckmark(hClist, ppro, hItemAll);
					break;

				case CLN_LISTREBUILT:
					{
						int bCheck = false;

						// Delete non-icq contacts
						if (ppro) {
							DeleteOtherContactsFromControl(hClist, ppro);
							if (!bListInit) // do not enter twice
								bCheck = UpdateCheckmarks(hClist, ppro, NULL);
						}

						if (!hItemAll) { // Add the "All contacts" item
							CLCINFOITEM cii = { 0 };
							cii.cbSize = sizeof(cii);
							cii.flags = CLCIIF_GROUPFONT | CLCIIF_CHECKBOX;
							cii.pszText = TranslateT(LPGEN("** All contacts **"));
							hItemAll = (HANDLE)SendMessage(hClist, CLM_ADDINFOITEM, 0, (LPARAM)&cii);
						}

						SendMessage(hClist, CLM_SETCHECKMARK, (WPARAM)hItemAll, bCheck);
					}
					break;

				case CLN_CHECKCHANGED:
					if (!bListInit) {
						NMCLISTCONTROL *nm = (NMCLISTCONTROL*)lParam;
						if (nm->flags&CLNF_ISINFO) {
							int check = SendMessage(hClist, CLM_GETCHECKMARK, (WPARAM)hItemAll, 0);

							MCONTACT hContact = db_find_first(ppro->m_szModuleName);
							while (hContact) {
								HANDLE hItem = (HANDLE)SendMessage(hClist, CLM_FINDCONTACT, hContact, 0);
								if (hItem)
									SendMessage(hClist, CLM_SETCHECKMARK, (WPARAM)hItem, check);
								hContact = db_find_next(hContact, ppro->m_szModuleName);
							}
						}
						else UpdateAllContactsCheckmark(hClist, ppro, hItemAll);
					}
					break;
				}
			}
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;

	case WM_DESTROY:
		if (hProtoAckHook)
			UnhookEvent(hProtoAckHook);
		if (working) // end server modifications here
			ppro->servlistPostPacket(NULL, 0, SSO_END_OPERATION, 100);

		hwndUploadContacts = NULL;
		working = 0;
		break;
	}

	return FALSE;
}

void CIcqProto::ShowUploadContactsDialog(void)
{
	if (hwndUploadContacts == NULL) {
		hItemAll = NULL;
		hwndUploadContacts = CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_ICQUPLOADLIST), NULL, DlgProcUploadList, LPARAM(this));
	}

	SetForegroundWindow(hwndUploadContacts);
}

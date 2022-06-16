/*
	Name: NewEventNotify - Plugin for Miranda IM
	File: popup.c - Displays a popup using the popup-plugin by hrk
	Version: 2.2.3
	Description: Notifies you about some events
	Author: icebreaker, <icebreaker@newmail.net>
	Date: 21.07.02 15:46 / Update: 16.09.02 17:45
	Copyright: (C) 2002 Starzinger Michael

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "stdafx.h"

extern int g_IsServiceAvail;

static mir_cs csPopupList;
static OBJLIST<PLUGIN_DATA> arPopupList(10, PtrKeySortT);

PLUGIN_DATA* PU_GetByContact(MCONTACT hContact, UINT eventType)
{
	for (auto &it : arPopupList)
		if (it->hContact == hContact && (eventType == -1 || it->eventType == (UINT)eventType))
			return it;

	return nullptr;
}

static INT_PTR CALLBACK DoDefaultHandling(void *param)
{
	auto *pdata = (PLUGIN_DATA *)param;
	if (pdata->hContact) {
		auto *pDlg = Srmm_FindDialog(pdata->hContact);
		if (pDlg && IsWindow(pDlg->GetHwnd()))
			CallService(MS_MSG_SENDMESSAGE, pdata->hContact, 0);
		else
			Clist_ContactDoubleClicked(pdata->hContact);
	}
	return 0;
}

int PopupAct(HWND hWnd, UINT mask, PLUGIN_DATA *pdata)
{
	if (mask & MASK_OPEN) {
		if (pdata) {
			if (g_plugin.bMsgReplyWindow && pdata->eventType == EVENTTYPE_MESSAGE)
				CallFunctionSync(DoDefaultHandling, pdata);
			else {
				if (pdata->events.empty())
					return 0;

				MEVENT hEvent = pdata->events[0];
				for (int idx = 0;; idx++) {
					CLISTEVENT *cle = g_clistApi.pfnGetEvent(pdata->hContact, idx);
					if (cle == nullptr) {
						CallFunctionSync(DoDefaultHandling, pdata);
						break;
					}

					if (cle->hDbEvent == hEvent) {
						if (ServiceExists(cle->pszService))
							CallServiceSync(cle->pszService, 0, (LPARAM)cle); // JK, use core (since 0.3.3+)
						break;
					}
				}
			}
		}
	}

	if (mask & MASK_REMOVE) {
		if (pdata) {
			for (auto &it: pdata->events) {
				g_clistApi.pfnRemoveEvent(pdata->hContact, it);
				db_event_markRead(pdata->hContact, it);
			}
		}
	}

	if (mask & MASK_DISMISS) {
		KillTimer(hWnd, TIMER_TO_ACTION);
		PUDeletePopup(hWnd);
	}

	return 0;
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	auto *pdata = (PLUGIN_DATA*)PUGetPluginData(hWnd);
	if (pdata == nullptr)
		return FALSE;

	RECT rc;
	POINT	pt;
	switch (message) {
	case WM_COMMAND:
		PopupAct(hWnd, g_plugin.maskActL, pdata);
		break;
	case WM_CONTEXTMENU:
		PopupAct(hWnd, g_plugin.maskActR, pdata);
		break;
	case UM_FREEPLUGINDATA:
		{
			mir_cslock lck(csPopupList);
			arPopupList.remove(pdata);
		}
		return TRUE;
	case UM_INITPOPUP:
		pdata->hWnd = hWnd;
		if (pdata->iSeconds > 0)
			SetTimer(hWnd, TIMER_TO_ACTION, pdata->iSeconds * 1000, nullptr);
		break;
	case WM_SETCURSOR:
		SetFocus(hWnd);
		break;
	case WM_TIMER:
		if (wParam != TIMER_TO_ACTION)
			break;

		GetCursorPos(&pt);
		GetWindowRect(hWnd, &rc);
		if (PtInRect(&rc, pt))
			break;

		if (pdata->iSeconds > 0)
			KillTimer(hWnd, TIMER_TO_ACTION);
		PopupAct(hWnd, g_plugin.maskActTE, pdata);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

/////////////////////////////////////////////////////////////////////////////////////////

static wchar_t* ShortenPreview(DBEVENTINFO *dbe)
{
	bool	fAddEllipsis = false;
	size_t iPreviewLimit = g_plugin.iLimitPreview;
	if (iPreviewLimit > 500 || iPreviewLimit == 0)
		iPreviewLimit = 500;

	wchar_t *buf = DbEvent_GetTextW(dbe, CP_ACP);
	if (mir_wstrlen(buf) > iPreviewLimit) {
		fAddEllipsis = true;
		size_t iIndex = iPreviewLimit;
		size_t iWordThreshold = 20;
		while (iIndex && buf[iIndex] != ' ' && iWordThreshold--)
			buf[iIndex--] = 0;

		buf[iIndex] = 0;
	}
	if (fAddEllipsis) {
		buf = (wchar_t *)mir_realloc(buf, (mir_wstrlen(buf) + 5) * sizeof(wchar_t));
		mir_wstrcat(buf, L"...");
	}
	return buf;
}

static wchar_t* GetEventPreview(DBEVENTINFO *dbei)
{
	wchar_t *comment1 = nullptr;
	wchar_t *comment2 = nullptr;
	char *commentFix = nullptr;
	char *pBlob = (char *)dbei->pBlob;

	// now get text
	switch (dbei->eventType) {
	case EVENTTYPE_MESSAGE:
		if (pBlob)
			comment1 = ShortenPreview(dbei);
		commentFix = POPUP_COMMENT_MESSAGE;
		break;

	case EVENTTYPE_FILE:
		if (pBlob) {
			char *p = pBlob + sizeof(uint32_t);
			// filenames
			comment2 = DbEvent_GetString(dbei, p);
			// description
			p += mir_strlen(p) + 1;
			if (*p)
				comment1 = DbEvent_GetString(dbei, p);
		}
		commentFix = POPUP_COMMENT_FILE;
		break;

	case EVENTTYPE_CONTACTS:
		// blob format is:
		// ASCIIZ    nick
		// ASCIIZ    UID
		if (pBlob) {
			// count contacts in event
			char* pcBlob = pBlob;
			char* pcEnd = pBlob + dbei->cbBlob;
			int nContacts;
			wchar_t szBuf[512];

			for (nContacts = 1; ; nContacts++) {
				// Nick
				pcBlob += mir_strlen(pcBlob) + 1;
				// UIN
				pcBlob += mir_strlen(pcBlob) + 1;
				// check for end of contacts
				if (pcBlob >= pcEnd)
					break;
			}
			mir_snwprintf(szBuf, TranslateT("Received %d contacts."), nContacts);
			comment1 = mir_wstrdup(szBuf);
		}
		commentFix = POPUP_COMMENT_CONTACTS;
		break;

	case EVENTTYPE_ADDED:
		// blob format is:
		// uint32_t     numeric uin (ICQ only afaik)
		// uint32_t     HANDLE to contact
		// ASCIIZ    nick (or text UID)
		// ASCIIZ    first name
		// ASCIIZ    last name
		// ASCIIZ    email (or YID)
		if (pBlob) {
			char szUin[16];
			wchar_t szBuf[2048];
			wchar_t* szNick = nullptr;
			char *pszNick = pBlob + 8;
			char *pszFirst = pszNick + mir_strlen(pszNick) + 1;
			char *pszLast = pszFirst + mir_strlen(pszFirst) + 1;
			char *pszEmail = pszLast + mir_strlen(pszLast) + 1;

			mir_snprintf(szUin, "%d", *((uint32_t*)pBlob));
			if (mir_strlen(pszNick) > 0) {
				if (dbei->flags & DBEF_UTF)
					szNick = mir_utf8decodeW(pszNick);
				else
					szNick = mir_a2u(pszNick);
			}
			else if (mir_strlen(pszEmail) > 0) {
				if (dbei->flags & DBEF_UTF)
					szNick = mir_utf8decodeW(pszEmail);
				else
					szNick = mir_a2u(pszEmail);
			}
			else if (*((uint32_t*)pBlob) > 0)
				szNick = mir_a2u(szUin);

			if (szNick) {
				mir_wstrcpy(szBuf, szNick);
				mir_wstrcat(szBuf, TranslateT(" added you to the contact list"));
				mir_free(szNick);
				comment1 = mir_wstrdup(szBuf);
			}
		}
		commentFix = POPUP_COMMENT_ADDED;
		break;

	case EVENTTYPE_AUTHREQUEST:
		if (pBlob) {
			DB::AUTH_BLOB blob(dbei->pBlob);

			wchar_t *szNick = nullptr;
			if (blob.get_nick())
				szNick = dbei->getString(blob.get_nick());
			else if (blob.get_email())
				szNick = dbei->getString(blob.get_email());
			else if (blob.get_uin()) {
				char szUin[16];
				szNick = mir_a2u(itoa(blob.get_uin(), szUin, 10));
			}

			if (szNick)
				comment1 = CMStringW(FORMAT, L"%s%s", szNick, TranslateT(" requested authorization")).Detach();
		}
		commentFix = POPUP_COMMENT_AUTH;
		break;

	default:
		// support for custom database event types
		DBEVENTTYPEDESCR *pei = DbEvent_GetType(dbei->szModule, dbei->eventType);
		if (pei && pBlob) {
			comment1 = DbEvent_GetTextW(dbei, CP_ACP);
			commentFix = pei->descr;
		}
		else commentFix = POPUP_COMMENT_OTHER;
	}

	if (mir_wstrlen(comment1) > 0) {
		mir_free(comment2);
		return comment1;
	}
	if (mir_wstrlen(comment2) > 0) {
		mir_free(comment1);
		return comment2;
	}
	mir_free(comment1);
	mir_free(comment2);
	return TranslateW(mir_a2u(commentFix));
}

int PopupShow(MCONTACT hContact, MEVENT hEvent, UINT eventType)
{
	wchar_t *sampleEvent;
	long iSeconds;

	// check if we should report this kind of event
	// get the prefered icon as well
	// CHANGE: iSeconds is -1 because I use my timer to hide popup
	POPUPDATAW pudw = {};
	pudw.iSeconds = -1;

	switch (eventType) {
	case EVENTTYPE_MESSAGE:
		if (!(g_plugin.maskNotify & MASK_MESSAGE)) return 1;
		pudw.lchIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
		pudw.colorBack = g_plugin.msg.bDefault ? 0 : g_plugin.msg.backColor;
		pudw.colorText = g_plugin.msg.bDefault ? 0 : g_plugin.msg.textColor;
		iSeconds = g_plugin.msg.iDelay;
		sampleEvent = TranslateT("This is a sample message event :-)");
		break;

	case EVENTTYPE_FILE:
		if (!(g_plugin.maskNotify & MASK_FILE)) return 1;
		pudw.lchIcon = Skin_LoadIcon(SKINICON_EVENT_FILE);
		pudw.colorBack = g_plugin.file.bDefault ? 0 : g_plugin.file.backColor;
		pudw.colorText = g_plugin.file.bDefault ? 0 : g_plugin.file.textColor;
		iSeconds = g_plugin.file.iDelay;
		sampleEvent = TranslateT("This is a sample file event :-D");
		break;

	case EVENTTYPE_ERRMSG:
		if (!(g_plugin.maskNotify & MASK_ERROR)) return 1;
		pudw.lchIcon = Skin_LoadIcon(SKINICON_OTHER_DELETE);
		pudw.colorBack = g_plugin.err.bDefault ? 0 : g_plugin.err.backColor;
		pudw.colorText = g_plugin.err.bDefault ? 0 : g_plugin.err.textColor;
		iSeconds = g_plugin.err.iDelay;
		sampleEvent = TranslateT("This is a sample error event :-D");
		break;

	default:
		if (!(g_plugin.maskNotify & MASK_OTHER)) return 1;
		pudw.lchIcon = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
		pudw.colorBack = g_plugin.other.bDefault ? 0 : g_plugin.other.backColor;
		pudw.colorText = g_plugin.other.bDefault ? 0 : g_plugin.other.textColor;
		iSeconds = g_plugin.other.iDelay;
		sampleEvent = TranslateT("This is a sample other event ;-D");
		break;
	}

	// get DBEVENTINFO with pBlob if preview is needed (when is test then is off)
	DB::EventInfo dbe;
	if (hEvent) {
		if ((g_plugin.bPreview || eventType == EVENTTYPE_ADDED || eventType == EVENTTYPE_AUTHREQUEST))
			dbe.cbBlob = -1;
		db_event_get(hEvent, &dbe);
	}

	// retrieve correct hContact for AUTH events
	if (dbe.pBlob && (eventType == EVENTTYPE_ADDED || eventType == EVENTTYPE_AUTHREQUEST))
		hContact = DbGetAuthEventContact(&dbe);

	// set plugin_data ... will be usable within PopupDlgProc
	PLUGIN_DATA *pdata = new PLUGIN_DATA();
	pdata->eventType = eventType;
	pdata->hContact = hContact;
	pdata->events.push_back(hEvent);
	switch (iSeconds) {
	case -1: pdata->iSeconds = 65535; break; // kinda forrever
	case 0:  pdata->iSeconds = g_plugin.iDelayDefault; break;
	default: pdata->iSeconds = iSeconds; break;
	}

	// finally create the popup
	pudw.lchContact = hContact;
	pudw.PluginWindowProc = PopupDlgProc;
	pudw.PluginData = pdata;

	// if hContact is NULL, && hEvent is NULL then popup is only Test
	if (hContact == 0 && hEvent == 0) {
		wcsncpy(pudw.lpwzContactName, TranslateT("Plugin Test"), MAX_CONTACTNAME);
		wcsncpy(pudw.lpwzText, TranslateW(sampleEvent), MAX_SECONDLINE);
	}
	else { // get the needed event data
		wcsncpy(pudw.lpwzContactName, Clist_GetContactDisplayName(hContact), MAX_CONTACTNAME);
		wcsncpy(pudw.lpwzText, ptrW(GetEventPreview(&dbe)), MAX_SECONDLINE);
	}

	// send data to popup plugin
	if (PUAddPopupW(&pudw) < 0)
		delete pdata;

	mir_cslock lck(csPopupList);
	arPopupList.insert(pdata);
	return 0;
}

int PopupUpdate(PLUGIN_DATA &pdata, MEVENT hEvent)
{
	if (hEvent) {
		pdata.events.push_back(hEvent);
		
		// re-init timer delay
		KillTimer(pdata.hWnd, TIMER_TO_ACTION);
		SetTimer(pdata.hWnd, TIMER_TO_ACTION, pdata.iSeconds * 1000, nullptr);
	}

	CMStringW wszText;
	if (g_plugin.bShowHeaders)
		wszText.AppendFormat(TranslateT("[b]Number of new message(s): %d[/b]\n"), pdata.events.size());

	size_t iStart, iEnd, iGap = (g_plugin.iNumberMsg == 0) ? 1 : g_plugin.iNumberMsg;
	if (pdata.events.size() < iGap) {
		iStart = 0; iEnd = pdata.events.size();
	}
	else if (g_plugin.bShowON) { // reverse sorting
		iStart = 0; iEnd = iGap;
	}
	else {
		iEnd = pdata.events.size(); iStart = iEnd - g_plugin.iNumberMsg;
	}

	// take the active event as starting one
	for (size_t i = iStart; i < iEnd; i++) {
		if (!wszText.IsEmpty())
			wszText.AppendChar('\n');

		// get DBEVENTINFO with pBlob if preview is needed (when is test then is off)
		DB::EventInfo dbe;
		if (g_plugin.bPreview)
			dbe.cbBlob = -1;
		db_event_get(pdata.events[i], &dbe);

		CMStringW wszFormat;
		if (g_plugin.bShowDate)
			wszFormat.Append(L"%Y.%m.%d ");
		if (g_plugin.bShowTime)
			wszFormat.Append(L"%H:%M");
		if (!wszFormat.IsEmpty()) {
			wchar_t timestamp[MAX_DATASIZE];
			time_t localTime = dbe.timestamp;
			wcsftime(timestamp, _countof(timestamp), wszFormat, localtime(&localTime));
			wszText.AppendFormat(L"[b][i]%s[/i][/b]\n", timestamp);
		}

		// prepare event preview
		wszText.Append(ptrW(GetEventPreview(&dbe)));
		wszText.AppendChar('\n');
	}

	if (pdata.events.empty())
		wszText.Append(L"\n...");

	PUChangeTextW(pdata.hWnd, wszText);
	return 0;
}

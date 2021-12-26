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

static int PopupCount = 0;

PLUGIN_DATA* PopupList[MAX_POPUPS];

int NumberPopupData(MCONTACT hContact, int eventType)
{
	for (int n = 0; n < MAX_POPUPS; n++) {
		if (!PopupList[n] && !hContact && eventType == -1)
			return n;

		if (PopupList[n] && (PopupList[n]->hContact == hContact) && (PopupList[n]->iLock == 0) && (eventType == -1 || PopupList[n]->eventType == (UINT)eventType))
			return n;
	}
	return -1;
}

static int FindPopupData(PLUGIN_DATA* pdata)
{
	for (int n = 0; n < MAX_POPUPS; n++)
		if (PopupList[n] == pdata)
			return n;

	return -1;
}

static void FreePopupEventData(PLUGIN_DATA* pdata)
{
	pdata->iLock = 1;
	EVENT_DATA_EX* eventData = pdata->firstEventData;
	while (eventData) {
		if (eventData->next) {
			eventData = eventData->next;
			mir_free(eventData->prev);
			eventData->prev = nullptr;
		}
		else {
			mir_free(eventData);
			eventData = nullptr;
		}
	}
	pdata->lastEventData = pdata->firstEventData = pdata->firstShowEventData = nullptr;
	// remove from popup list if present
	if (FindPopupData(pdata) != -1)
		PopupList[FindPopupData(pdata)] = nullptr;
}

int PopupAct(HWND hWnd, UINT mask, PLUGIN_DATA* pdata)
{
	if (mask & MASK_OPEN) {
		if (pdata) {
			// do MS_MSG_SENDMESSAGE instead if wanted to reply and not read!
			if (g_plugin.bMsgReplyWindow && pdata->eventType == EVENTTYPE_MESSAGE)
				CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)pdata->hContact, 0); // JK, use core (since 0.3.3+)
			else {
				EVENT_DATA_EX *eventData = pdata->firstEventData;
				if (eventData == nullptr)
					return 0;

				for (int idx = 0;; idx++) {
					CLISTEVENT *cle = g_clistApi.pfnGetEvent(pdata->hContact, idx);
					if (cle == nullptr)
						break;

					if (cle->hDbEvent == eventData->hEvent) {
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
			EVENT_DATA_EX *eventData = pdata->firstEventData;
			pdata->iLock = 1;
			while (eventData) {
				g_clistApi.pfnRemoveEvent(pdata->hContact, eventData->hEvent);
				db_event_markRead(pdata->hContact, eventData->hEvent);
				eventData = eventData->next;
			}
			FreePopupEventData(pdata);
		}
	}

	if (mask & MASK_DISMISS) {
		KillTimer(hWnd, TIMER_TO_ACTION);
		FreePopupEventData(pdata);
		PUDeletePopup(hWnd);
	}

	return 0;
}

static LRESULT CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PLUGIN_DATA *pdata = (PLUGIN_DATA*)PUGetPluginData(hWnd);
	if (pdata == nullptr)
		return FALSE;

	switch (message) {
	case WM_COMMAND:
		PopupAct(hWnd, g_plugin.maskActL, pdata);
		break;
	case WM_CONTEXTMENU:
		PopupAct(hWnd, g_plugin.maskActR, pdata);
		break;
	case UM_FREEPLUGINDATA:
		PopupCount--;
		mir_free(pdata);
		return TRUE;
	case UM_INITPOPUP:
		pdata->hWnd = hWnd;
		SetTimer(hWnd, TIMER_TO_ACTION, pdata->iSeconds * 1000, nullptr);
		break;
	case WM_MOUSEWHEEL:
		if ((short)HIWORD(wParam) > 0 && pdata->firstShowEventData->prev &&
			(g_plugin.bShowON || pdata->firstShowEventData->number >= g_plugin.iNumberMsg)) {
			pdata->firstShowEventData = pdata->firstShowEventData->prev;
			PopupUpdate(pdata->hContact, NULL);
		}
		if ((short)HIWORD(wParam) < 0 && pdata->firstShowEventData->next &&
			(!g_plugin.bShowON || pdata->countEvent - pdata->firstShowEventData->number >= g_plugin.iNumberMsg)) {
			pdata->firstShowEventData = pdata->firstShowEventData->next;
			PopupUpdate(pdata->hContact, NULL);
		}
		break;
	case WM_SETCURSOR:
		SetFocus(hWnd);
		break;
	case WM_TIMER:
		if (wParam != TIMER_TO_ACTION)
			break;
		PopupAct(hWnd, g_plugin.maskActTE, pdata);
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

static wchar_t* GetEventPreview(DBEVENTINFO *dbei)
{
	wchar_t *comment1 = nullptr;
	wchar_t *comment2 = nullptr;
	char *commentFix = nullptr;

	// now get text
	switch (dbei->eventType) {
	case EVENTTYPE_MESSAGE:
		if (dbei->pBlob) {
			if (dbei->flags & DBEF_UTF) {
				// utf-8 in blob
				comment1 = mir_utf8decodeW((char*)dbei->pBlob);
			}
			else if (dbei->cbBlob == (mir_wstrlen((wchar_t *)dbei->pBlob) + 1)*(sizeof(wchar_t) + 1)) {
				// wchar in blob (the old hack)
				comment1 = mir_wstrdup((wchar_t*)dbei->pBlob);
			}
			else comment1 = mir_a2u((char *)dbei->pBlob);
		}
		commentFix = POPUP_COMMENT_MESSAGE;
		break;

	case EVENTTYPE_FILE:
		if (dbei->pBlob) {
			char *p = (char*)dbei->pBlob + sizeof(uint32_t);
			// filenames
			comment2 = (dbei->flags & DBEF_UTF) ? mir_utf8decodeW(p) : mir_a2u(p);
			p += mir_strlen(p) + 1;
			// description
			comment1 = (dbei->flags & DBEF_UTF) ? mir_utf8decodeW(p) : mir_a2u(p);
		}
		commentFix = POPUP_COMMENT_FILE;
		break;

	case EVENTTYPE_CONTACTS:
		// blob format is:
		// ASCIIZ    nick
		// ASCIIZ    UID
		if (dbei->pBlob) {
			// count contacts in event
			char* pcBlob = (char *)dbei->pBlob;
			char* pcEnd = (char *)(dbei->pBlob + dbei->cbBlob);
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
		if (dbei->pBlob) {
			char szUin[16];
			wchar_t szBuf[2048];
			wchar_t* szNick = nullptr;
			char *pszNick = (char *)dbei->pBlob + 8;
			char *pszFirst = pszNick + mir_strlen(pszNick) + 1;
			char *pszLast = pszFirst + mir_strlen(pszFirst) + 1;
			char *pszEmail = pszLast + mir_strlen(pszLast) + 1;

			mir_snprintf(szUin, "%d", *((uint32_t*)dbei->pBlob));
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
			else if (*((uint32_t*)dbei->pBlob) > 0)
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
		if (dbei->pBlob) {
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
		DBEVENTTYPEDESCR *pei = DbEvent_GetType(dbei->szModule, dbei->eventType);
		// support for custom database event types
		if (pei && dbei->pBlob) {
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

	// there has to be a maximum number of popups shown at the same time
	if (PopupCount >= MAX_POPUPS)
		return 2;

	// check if we should report this kind of event
	// get the prefered icon as well
	// CHANGE: iSeconds is -1 because I use my timer to hide popup
	POPUPDATAW pudw = {};
	pudw.iSeconds = -1;

	switch (eventType) {
	case EVENTTYPE_MESSAGE:
		if (!(g_plugin.maskNotify & MASK_MESSAGE)) return 1;
		pudw.lchIcon = Skin_LoadIcon(SKINICON_EVENT_MESSAGE);
		pudw.colorBack = g_plugin.bDefaultColorMsg ? 0 : g_plugin.colBackMsg;
		pudw.colorText = g_plugin.bDefaultColorMsg ? 0 : g_plugin.colTextMsg;
		iSeconds = g_plugin.iDelayMsg;
		sampleEvent = TranslateT("This is a sample message event :-)");
		break;

	case EVENTTYPE_FILE:
		if (!(g_plugin.maskNotify & MASK_FILE)) return 1;
		pudw.lchIcon = Skin_LoadIcon(SKINICON_EVENT_FILE);
		pudw.colorBack = g_plugin.bDefaultColorFile ? 0 : g_plugin.colBackFile;
		pudw.colorText = g_plugin.bDefaultColorFile ? 0 : g_plugin.colTextFile;
		iSeconds = g_plugin.iDelayFile;
		sampleEvent = TranslateT("This is a sample file event :-D");
		break;

	default:
		if (!(g_plugin.maskNotify & MASK_OTHER)) return 1;
		pudw.lchIcon = Skin_LoadIcon(SKINICON_OTHER_MIRANDA);
		pudw.colorBack = g_plugin.bDefaultColorOthers ? 0 : g_plugin.colBackOthers;
		pudw.colorText = g_plugin.bDefaultColorOthers ? 0 : g_plugin.colTextOthers;
		iSeconds = g_plugin.iDelayOthers;
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

	EVENT_DATA_EX *eventData = (EVENT_DATA_EX*)mir_alloc(sizeof(EVENT_DATA_EX));
	eventData->hEvent = hEvent;
	eventData->number = 1;
	eventData->next = nullptr;
	eventData->prev = nullptr;

	// retrieve correct hContact for AUTH events
	if (dbe.pBlob && (eventType == EVENTTYPE_ADDED || eventType == EVENTTYPE_AUTHREQUEST))
		hContact = DbGetAuthEventContact(&dbe);

	// set plugin_data ... will be usable within PopupDlgProc
	PLUGIN_DATA *pdata = (PLUGIN_DATA*)mir_calloc(sizeof(PLUGIN_DATA));
	pdata->eventType = eventType;
	pdata->hContact = hContact;
	pdata->countEvent = 1;
	pdata->iLock = 0;
	pdata->iSeconds = (iSeconds > 0) ? iSeconds : g_plugin.iDelayDefault;
	pdata->firstEventData = pdata->firstShowEventData = pdata->lastEventData = eventData;

	// finally create the popup
	pudw.lchContact = hContact;
	pudw.PluginWindowProc = PopupDlgProc;
	pudw.PluginData = pdata;

	// if hContact is NULL, && hEvent is NULL then popup is only Test
	if ((hContact == NULL) && (hEvent == NULL)) {
		wcsncpy(pudw.lpwzContactName, TranslateT("Plugin Test"), MAX_CONTACTNAME);
		wcsncpy(pudw.lpwzText, TranslateW(sampleEvent), MAX_SECONDLINE);
	}
	else { // get the needed event data
		wcsncpy(pudw.lpwzContactName, Clist_GetContactDisplayName(hContact), MAX_CONTACTNAME);
		wcsncpy(pudw.lpwzText, ptrW(GetEventPreview(&dbe)), MAX_SECONDLINE);
	}

	PopupCount++;

	PopupList[NumberPopupData(NULL, -1)] = pdata;
	// send data to popup plugin

	// popup creation failed, release popupdata
	if (PUAddPopupW(&pudw) < 0) {
		FreePopupEventData(pdata);
		mir_free(pdata);
	}

	return 0;
}


int PopupUpdate(MCONTACT hContact, MEVENT hEvent)
{
	// merge only message popups
	PLUGIN_DATA *pdata = (PLUGIN_DATA*)PopupList[NumberPopupData(hContact, EVENTTYPE_MESSAGE)];

	if (hEvent) {
		pdata->countEvent++;

		pdata->lastEventData->next = (EVENT_DATA_EX *)mir_alloc(sizeof(EVENT_DATA_EX));
		pdata->lastEventData->next->prev = pdata->lastEventData;
		pdata->lastEventData = pdata->lastEventData->next;
		pdata->lastEventData->hEvent = hEvent;
		pdata->lastEventData->number = pdata->lastEventData->prev->number + 1;
		pdata->lastEventData->next = nullptr;
		if (!g_plugin.bShowON && pdata->countEvent > g_plugin.iNumberMsg && g_plugin.iNumberMsg)
			pdata->firstShowEventData = pdata->firstShowEventData->next;
		// re-init timer delay
		KillTimer(pdata->hWnd, TIMER_TO_ACTION);
		SetTimer(pdata->hWnd, TIMER_TO_ACTION, pdata->iSeconds * 1000, nullptr);
	}

	wchar_t lpzText[MAX_SECONDLINE * 2] = L"\0\0";
	if (g_plugin.bShowHeaders)
		mir_snwprintf(lpzText, TranslateT("[b]Number of new message(s): %d[/b]\n"), pdata->countEvent);

	int doReverse = g_plugin.bShowON;

	if ((pdata->firstShowEventData != pdata->firstEventData && doReverse) || (pdata->firstShowEventData != pdata->lastEventData && !doReverse))
		mir_snwprintf(lpzText, L"%s...\n", lpzText);

	// take the active event as starting one
	EVENT_DATA_EX *eventData = pdata->firstShowEventData;

	int iEvent = 0;
	while (true) {
		if (iEvent)
			eventData = (doReverse) ? eventData->next : eventData->prev;
		iEvent++;

		// get DBEVENTINFO with pBlob if preview is needed (when is test then is off)
		DB::EventInfo dbe;
		if (eventData->hEvent) {
			if (g_plugin.bPreview)
				dbe.cbBlob = -1;
			db_event_get(eventData->hEvent, &dbe);
		}

		if (g_plugin.bShowDate || g_plugin.bShowTime) {
			wchar_t timestamp[MAX_DATASIZE];
			wchar_t formatTime[MAX_DATASIZE];
			if (g_plugin.bShowDate)
				wcsncpy(formatTime, L"%Y.%m.%d", _countof(formatTime));
			else if (g_plugin.bShowTime)
				mir_wstrncat(formatTime, L" %H:%M", _countof(formatTime) - mir_wstrlen(formatTime));
			time_t localTime = dbe.timestamp;
			wcsftime(timestamp, _countof(timestamp), formatTime, localtime(&localTime));
			mir_snwprintf(lpzText, L"%s[b][i]%s[/i][/b]\n", lpzText, timestamp);
		}

		// prepare event preview
		wchar_t* szEventPreview = GetEventPreview(&dbe);
		mir_snwprintf(lpzText, L"%s%s", lpzText, szEventPreview);
		mir_free(szEventPreview);

		if (doReverse) {
			if ((iEvent >= g_plugin.iNumberMsg && g_plugin.iNumberMsg) || !eventData->next)
				break;
		}
		else if ((iEvent >= g_plugin.iNumberMsg && g_plugin.iNumberMsg) || !eventData->prev)
			break;

		mir_snwprintf(lpzText, L"%s\n", lpzText);
	}

	if ((doReverse && eventData->next) || (!doReverse && eventData->prev))
		mir_snwprintf(lpzText, L"%s\n...", lpzText);

	PUChangeTextW(pdata->hWnd, lpzText);
	return 0;
}

int PopupPreview()
{
	PopupShow(0, 0, EVENTTYPE_MESSAGE);
	PopupShow(0, 0, EVENTTYPE_FILE);
	PopupShow(0, 0, -1);
	return 0;
}

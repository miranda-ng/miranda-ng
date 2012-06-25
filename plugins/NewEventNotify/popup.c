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

#include "neweventnotify.h"
#include <m_skin.h>
#include <m_clist.h>
#include <m_system.h>
#include <m_protocols.h>
//needed for ICQEVENTTYPE_* (Webpager & Emailexpress)
#include <m_protosvc.h>
#include <m_icq.h>
//needed for reply instead of read
#include <m_message.h>
#include <m_popup.h>
#include <string.h>
#include <time.h>

#define SIZEOF(X) (sizeof(X)/(g_UnicodeCore?sizeof(WCHAR):sizeof(char)))

extern int g_IsServiceAvail;

static int PopupCount = 0;

PLUGIN_DATA* PopUpList[MAX_POPUPS];

/*
TIME NowTime()
{
	time_t actTime;
	TIME endTime;
	time(&actTime);
	strftime(endTime.time,sizeof(endTime.time), "%H:%M", localtime(&actTime));
	strftime(endTime.date,sizeof(endTime.date), "%Y.%m.%d", localtime(&actTime));
	strftime(endTime.all,sizeof(endTime.all), "%Y.%m.%d %H:%M", localtime(&actTime));
	return endTime;
}
*/

int NumberPopupData(HANDLE hContact, int eventType)
{
	int n;

	for (n=0;n<MAX_POPUPS;n++)
	{
		if (!PopUpList[n] && !hContact && eventType == -1)
			return n;

		if (PopUpList[n] && (PopUpList[n]->hContact == hContact) && (PopUpList[n]->iLock == 0) && (eventType == -1 || PopUpList[n]->eventType == (UINT)eventType))
			return n;
	}
	return -1;
}



static int FindPopupData(PLUGIN_DATA* pdata)
{
	int n;

	for (n=0;n<MAX_POPUPS;n++)
	{
		if (PopUpList[n] == pdata) 
			return n;
	}
	return -1;
}



static void FreePopupEventData(PLUGIN_DATA* pdata)
{
	EVENT_DATA_EX* eventData;

	pdata->iLock = 1;
	eventData = pdata->firstEventData;
	while (eventData)
	{
		if (eventData->next)
		{
			eventData = eventData->next;
			mir_free(eventData->prev);
			eventData->prev = NULL;
		}
		else
		{
			mir_free(eventData);
			eventData = NULL;
		}
	}		
	pdata->lastEventData = pdata->firstEventData = pdata->firstShowEventData = NULL;
	// remove from popup list if present
	if (FindPopupData(pdata) != -1)
		PopUpList[FindPopupData(pdata)] = NULL;
}



int PopupAct(HWND hWnd, UINT mask, PLUGIN_DATA* pdata)
{
	EVENT_DATA_EX* eventData;

	if (mask & MASK_OPEN)
	{
		if (pdata)
		{
			// do MS_MSG_SENDMESSAGE instead if wanted to reply and not read!
			if (pdata->pluginOptions->bMsgReplyWindow && pdata->eventType == EVENTTYPE_MESSAGE)
			{
				CallServiceSync(MS_MSG_SENDMESSAGE, (WPARAM)pdata->hContact, (LPARAM)NULL); // JK, use core (since 0.3.3+)
			}
			else
			{
				CLISTEVENT* cle;
				int idx = 0;

				eventData = pdata->firstEventData;
				if (eventData)
				{ 
					do { //try to find the correct clist event //JK
						cle = (CLISTEVENT*)CallService(MS_CLIST_GETEVENT, (WPARAM)pdata->hContact, idx);
						if (cle && cle->hDbEvent == eventData->hEvent)
						{
							if (ServiceExists(cle->pszService))
								CallServiceSync(cle->pszService, (WPARAM)NULL, (LPARAM)cle); // JK, use core (since 0.3.3+)
							break;
						}
						idx++;
					} while (cle);
				}
			}
		}
	}

	if (mask & MASK_REMOVE)
	{
		if (pdata)
		{
			eventData = pdata->firstEventData;
			pdata->iLock = 1;
			while (eventData)
			{
				CallService(MS_CLIST_REMOVEEVENT, (WPARAM)pdata->hContact, (LPARAM)eventData->hEvent);
				CallService(MS_DB_EVENT_MARKREAD, (WPARAM)pdata->hContact, (LPARAM)eventData->hEvent);			
				eventData = eventData->next;
			}		
			FreePopupEventData(pdata);
		}
	}

	if (mask & MASK_DISMISS)
	{
		KillTimer(hWnd, TIMER_TO_ACTION);
		FreePopupEventData(pdata);
		PUDeletePopUp(hWnd);
	}

	return 0;
}

static BOOL CALLBACK PopupDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PLUGIN_DATA* pdata = NULL;

  pdata = (PLUGIN_DATA*)CallService(MS_POPUP_GETPLUGINDATA, (WPARAM)hWnd, (LPARAM)pdata);
  if (!pdata) return FALSE;

  switch (message)
  {
    case WM_COMMAND:
			PopupAct(hWnd, pdata->pluginOptions->maskActL, pdata);
      break;
    case WM_CONTEXTMENU:
			PopupAct(hWnd, pdata->pluginOptions->maskActR, pdata);
      break;
		case UM_FREEPLUGINDATA:
			PopupCount--;
      mir_free(pdata);
			return TRUE;
		case UM_INITPOPUP:
			pdata->hWnd = hWnd;
			SetTimer(hWnd, TIMER_TO_ACTION, pdata->iSeconds * 1000, NULL);
			break;
		case WM_MOUSEWHEEL:
			if ((short)HIWORD(wParam) > 0 && pdata->firstShowEventData->prev &&
        (pdata->pluginOptions->bShowON || pdata->firstShowEventData->number >= pdata->pluginOptions->iNumberMsg))
			{
				pdata->firstShowEventData = pdata->firstShowEventData->prev;
				PopupUpdate(pdata->hContact, NULL);
			}
			if ((short)HIWORD(wParam) < 0 && pdata->firstShowEventData->next && 
				(!pdata->pluginOptions->bShowON || pdata->countEvent - pdata->firstShowEventData->number >= pdata->pluginOptions->iNumberMsg))
			{
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
			PopupAct(hWnd, pdata->pluginOptions->maskActTE, pdata);
			break;
		default:
			break;
  }
  if (g_UnicodeCore)
    return DefWindowProcW(hWnd, message, wParam, lParam);
  else
    return DefWindowProcA(hWnd, message, wParam, lParam);
}

static TCHAR* event_to_tchar(char* pBlob, DWORD flags)
{
//  if (flags & DBEF_UTF)
//    return utf8_to_tchar(pBlob);
//  else
    return ansi_to_tchar(pBlob);
}

static TCHAR* GetEventPreview(DBEVENTINFO *dbei)
{
  TCHAR* comment1 = NULL;
  TCHAR* comment2 = NULL;
  TCHAR* commentFix = NULL;

  //now get text
  switch (dbei->eventType)
  {
    case EVENTTYPE_MESSAGE:
    {
      if (dbei->pBlob)
      {
        if (dbei->flags & DBEF_UTF)
        { // utf-8 in blob
          comment1 = utf8_to_tchar(dbei->pBlob);
        } 
        else if (dbei->cbBlob == (strlennull((char *)dbei->pBlob)+1)*(sizeof(WCHAR)+1))
        { // wchar in blob (the old hack)
          if (g_UnicodeCore)
            comment1 = strdupT((TCHAR*)(dbei->pBlob + strlennull((char *)dbei->pBlob) + 1));
          else
            comment1 = strdupT((TCHAR*)dbei->pBlob);
        } 
        else
          comment1 = (TCHAR *)(dbei->pBlob);
      }
      commentFix = POPUP_COMMENT_MESSAGE;
      break;
    }

		case EVENTTYPE_URL:
      // url
			if (dbei->pBlob) comment2 = (TCHAR *)(dbei->pBlob);
      // comment
			if (dbei->pBlob) comment1 = (TCHAR *)(dbei->pBlob + strlennull((char *)dbei->pBlob) + 1);
			commentFix = POPUP_COMMENT_URL;
			break;

		case EVENTTYPE_FILE:
      // filenames
			if (dbei->pBlob) comment2 = event_to_tchar((char *)dbei->pBlob + 4, dbei->flags);
      // description
			if (dbei->pBlob) comment1 = event_to_tchar((char *)dbei->pBlob + strlennull((char *)dbei->pBlob + 4) + 1, dbei->flags);
			commentFix = POPUP_COMMENT_FILE;
			break;

//blob format is:
//ASCIIZ    nick
//ASCIIZ    UID
    case EVENTTYPE_CONTACTS:
      if (dbei->pBlob)
      { // count contacts in event
        char* pcBlob = (char *)dbei->pBlob;
        char* pcEnd = (char *)dbei->pBlob + dbei->cbBlob;
        int nContacts;
        TCHAR szBuf[512];
        WCHAR szTmp[512];

        for (nContacts = 1; ; nContacts++)
        { // Nick
          pcBlob += strlennull(pcBlob) + 1;
          // UIN
          pcBlob += strlennull(pcBlob) + 1;
          // check for end of contacts
          if (pcBlob >= pcEnd)
            break;
        }
        if (g_UnicodeCore)
        {
          _sntprintf((WCHAR*)szBuf, SIZEOF(szBuf), (WCHAR*)NENTranslateT("Received %d contacts.", szTmp), nContacts);
          ((WCHAR*)szBuf)[255] = 0;
        }
        else
          _snprintfT(szBuf, SIZEOF(szBuf), (char *)NENTranslateT("Received %d contacts.", szTmp), nContacts);
        comment1 = strdupT(szBuf);
      }
      commentFix = POPUP_COMMENT_CONTACTS;
      break;

//blob format is:
//DWORD     numeric uin (ICQ only afaik)
//DWORD     HANDLE to contact
//ASCIIZ    nick (or text UID)
//ASCIIZ    first name
//ASCIIZ    last name
//ASCIIZ    email (or YID)
    case EVENTTYPE_ADDED:
      if (dbei->pBlob)
      {
        TCHAR szUin[16];
        TCHAR szBuf[2048];
        TCHAR* szNick = NULL;
        TCHAR *pszNick = (TCHAR *)dbei->pBlob + 8;
        TCHAR *pszFirst = pszNick + strlennull((char *)pszNick) + 1;
        TCHAR *pszLast = pszFirst + strlennull((char *)pszFirst) + 1;
        TCHAR *pszEmail = pszLast + strlennull((char *)pszLast) + 1;

        _sntprintf(szUin, 16, _T("%d"), *((DWORD*)dbei->pBlob));
        if (strlennull((char *)pszNick) > 0)
          szNick = pszNick;
        else if (strlennull((char *)pszEmail) > 0)
          szNick = pszEmail;
        else if (*((DWORD*)dbei->pBlob) > 0)
          szNick = szUin;

        if (szNick)
        {
          strcpyT(szBuf, szNick);
          strcatT(szBuf, NENTranslateT(" added you to the contact list", (WCHAR*)szBuf + strlenT(szBuf) + 1));
          mir_free(szNick);
          comment1 = strdupT(szBuf);
        }
      }
      commentFix = POPUP_COMMENT_ADDED;
		  break;

		case EVENTTYPE_AUTHREQUEST:
      if (dbei->pBlob)
      {
        TCHAR szUin[16];
        TCHAR szBuf[2048];
        TCHAR* szNick = NULL;
        TCHAR *pszNick = (TCHAR *)dbei->pBlob + 8;
        TCHAR *pszFirst = pszNick + strlennull((char *)pszNick) + 1;
        TCHAR *pszLast = pszFirst + strlennull((char *)pszFirst) + 1;
        TCHAR *pszEmail = pszLast + strlennull((char *)pszLast) + 1;

        _snwprintf(szUin, 16, _T("%d"), *((DWORD*)dbei->pBlob));
        if (strlennull((char *)pszNick) > 0)
          szNick = pszNick;
        else if (strlennull((char *)pszEmail) > 0)
          szNick = pszEmail;
        else if (*((DWORD*)dbei->pBlob) > 0)
          szNick = szUin;

        if (szNick)
        {
          strcpyT(szBuf, szNick);
          strcatT(szBuf, NENTranslateT(" requested authorization", (WCHAR*)szBuf + strlenT(szBuf) + 1));
          mir_free(szNick);
          comment1 = strdupT(szBuf);
        }
      }
		  commentFix = POPUP_COMMENT_AUTH;
		  break;

//blob format is:
//ASCIIZ    text, usually "Sender IP: xxx.xxx.xxx.xxx\r\n%s"
//ASCIIZ    from name
//ASCIIZ    from e-mail
    case ICQEVENTTYPE_WEBPAGER:
			if (dbei->pBlob) comment1 = (TCHAR *)(dbei->pBlob);
//			if (dbei->pBlob) comment1 = dbei->pBlob + strlennull(comment2) + 1;
		  commentFix = POPUP_COMMENT_WEBPAGER;
		  break;

//blob format is:
//ASCIIZ    text, usually of the form "Subject: %s\r\n%s"
//ASCIIZ    from name
//ASCIIZ    from e-mail
    case ICQEVENTTYPE_EMAILEXPRESS:
			if (dbei->pBlob) comment1 = (TCHAR *)(dbei->pBlob);
//			if (dbei->pBlob) comment1 = dbei->pBlob + strlennull(comment2) + 1;
		  commentFix = POPUP_COMMENT_EMAILEXP;
      break;

    default:
    {
      if (ServiceExists(MS_DB_EVENT_GETTYPE))
      {
        DBEVENTTYPEDESCR *pei = (DBEVENTTYPEDESCR *)CallService(MS_DB_EVENT_GETTYPE, (WPARAM)dbei->szModule, (LPARAM)dbei->eventType);
		if ((pei && pei->cbSize) >= DBEVENTTYPEDESCR_SIZE)
        { // support for custom database event types
          if (dbei->pBlob)
          { // preview requested
            DBEVENTGETTEXT svc = {dbei, g_UnicodeCore ? DBVT_WCHAR : DBVT_ASCIIZ, CP_ACP};
            char *pet = (char *)CallService(MS_DB_EVENT_GETTEXT, 0, (LPARAM)&svc);
            if (pet)
            { // we've got event text, move to our memory space
              comment1 = strdupT((TCHAR *)pet);
              mir_free(&pet);
            }
          }
          commentFix = (TCHAR *)pei->descr;
        }
        else
          commentFix = POPUP_COMMENT_OTHER;
		
		break;
	  }
     // else
       // commentFix = POPUP_COMMENT_OTHER;


    }
  }

  if (strlenT(comment1) > 0)
  {
    mir_free(comment2);
    return comment1;
  }
  if (strlenT(comment2) > 0)
  {
    mir_free(comment1);
    return comment2;
  }
  mir_free(comment1);
  mir_free(comment2);

  {
    WCHAR tmp[MAX_PATH];

    return strdupT(NENTranslateT((char *)commentFix, tmp));
  }
}

int PopupShow(PLUGIN_OPTIONS* pluginOptions, HANDLE hContact, HANDLE hEvent, UINT eventType)
{
  POPUPDATAEX puda;
  POPUPDATAW pudw;
  PLUGIN_DATA* pdata;
  DBEVENTINFO dbe = {0};
	EVENT_DATA_EX* eventData;
	char* sampleEvent;
	long iSeconds;
  
	//there has to be a maximum number of popups shown at the same time
  if (PopupCount >= MAX_POPUPS)
    return 2;

	//check if we should report this kind of event
    //get the prefered icon as well
	//CHANGE: iSeconds is -1 because I use my timer to hide popup
  pudw.iSeconds = -1; 

	switch (eventType)
    {
      case EVENTTYPE_MESSAGE:
        if (!(pluginOptions->maskNotify&MASK_MESSAGE)) return 1;
        pudw.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_MESSAGE);
				pudw.colorBack = pluginOptions->bDefaultColorMsg ? 0 : pluginOptions->colBackMsg;
				pudw.colorText = pluginOptions->bDefaultColorMsg ? 0 : pluginOptions->colTextMsg;
				iSeconds = pluginOptions->iDelayMsg;
				sampleEvent = LPGEN("This is a sample message event :-)");
        break;
      case EVENTTYPE_URL:
        if (!(pluginOptions->maskNotify&MASK_URL)) return 1;
        pudw.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_URL);
				pudw.colorBack = pluginOptions->bDefaultColorUrl ? 0 : pluginOptions->colBackUrl;
				pudw.colorText = pluginOptions->bDefaultColorUrl ? 0 : pluginOptions->colTextUrl;
				iSeconds = pluginOptions->iDelayUrl;
				sampleEvent = LPGEN("This is a sample URL event ;-)");
        break;
      case EVENTTYPE_FILE:
        if (!(pluginOptions->maskNotify&MASK_FILE)) return 1;
        pudw.lchIcon = LoadSkinnedIcon(SKINICON_EVENT_FILE);
				pudw.colorBack = pluginOptions->bDefaultColorFile ? 0 : pluginOptions->colBackFile;
				pudw.colorText = pluginOptions->bDefaultColorFile ? 0 : pluginOptions->colTextFile;
				iSeconds = pluginOptions->iDelayFile;
				sampleEvent = LPGEN("This is a sample file event :-D");
        break;
      default:
        if (!(pluginOptions->maskNotify&MASK_OTHER)) return 1;
   			pudw.lchIcon = LoadSkinnedIcon(SKINICON_OTHER_MIRANDA);
				pudw.colorBack = pluginOptions->bDefaultColorOthers ? 0 : pluginOptions->colBackOthers;
				pudw.colorText = pluginOptions->bDefaultColorOthers ? 0 : pluginOptions->colTextOthers;
				iSeconds = pluginOptions->iDelayOthers;
				sampleEvent = LPGEN("This is a sample other event ;-D");
   			break;
    }

  //get DBEVENTINFO with pBlob if preview is needed (when is test then is off)
  dbe.cbSize = sizeof(dbe);
    
	if ((pluginOptions->bPreview || eventType == EVENTTYPE_ADDED || eventType == EVENTTYPE_AUTHREQUEST) && hEvent)
  {
    dbe.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)hEvent, 0);
    dbe.pBlob = (PBYTE)malloc(dbe.cbBlob);
  }
  if (hEvent)
    CallService(MS_DB_EVENT_GET, (WPARAM)hEvent, (LPARAM)&dbe);
	
  eventData = (EVENT_DATA_EX*)malloc(sizeof(EVENT_DATA_EX));
	eventData->hEvent = hEvent;
	eventData->number = 1;
	eventData->next = NULL;
	eventData->prev = NULL;

	// retrieve correct hContact for AUTH events
	if (dbe.pBlob && (eventType == EVENTTYPE_ADDED || eventType == EVENTTYPE_AUTHREQUEST))
		hContact = *((PHANDLE)(dbe.pBlob + sizeof(DWORD)));

	// set plugin_data ... will be usable within PopupDlgProc
  pdata = (PLUGIN_DATA*)malloc(sizeof(PLUGIN_DATA));
  pdata->eventType = eventType;
  pdata->hContact = hContact;
  pdata->pluginOptions = pluginOptions;
	pdata->countEvent = 1;
	pdata->iLock = 0;
	pdata->iSeconds = (iSeconds > 0) ? iSeconds : pluginOptions->iDelayDefault;
	pdata->firstEventData = pdata->firstShowEventData = pdata->lastEventData = eventData;
    
	// finally create the popup
	pudw.lchContact = hContact;
	pudw.PluginWindowProc = (WNDPROC)PopupDlgProc;
  pudw.PluginData = pdata;
	
	// if hContact is NULL, && hEvent is NULL then popup is only Test
	if ((hContact == NULL) && (hEvent == NULL))
	{
		strncpyT((TCHAR*)pudw.lptzContactName, NENTranslateT("Plugin Test", pudw.lpwzContactName), MAX_CONTACTNAME);
		strncpyT((TCHAR*)pudw.lptzText, NENTranslateT((char *)sampleEvent, pudw.lpwzText), MAX_SECONDLINE);
	}
	else
	{	// get the needed event data
    TCHAR* szEventPreview;

    strncpyT((TCHAR*)pudw.lptzContactName, (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)hContact, g_UnicodeCore ? GCDNF_UNICODE : 0), MAX_CONTACTNAME);
    szEventPreview = GetEventPreview(&dbe);
		strncpyT((TCHAR*)pudw.lptzText, szEventPreview, MAX_SECONDLINE);
    mir_free(szEventPreview);
	}
    
	PopupCount++;

	PopUpList[NumberPopupData(NULL, -1)] = pdata;
	// send data to popup plugin

	if (g_UnicodeCore && ServiceExists(MS_POPUP_ADDPOPUPW))
	{
		pdata->isUnicode = 1;
		if (CallService(MS_POPUP_ADDPOPUPW, (WPARAM)&pudw, 0) < 0)
		{ // popup creation failed, release popupdata
			FreePopupEventData(pdata);
			mir_free(pdata);
		}
	}
	else // convert to ansi
	{
		pdata->isUnicode = 0;
		puda.iSeconds = pudw.iSeconds;
    puda.lchIcon = pudw.lchIcon;
    puda.colorBack = pudw.colorBack;
    puda.colorText = pudw.colorText;
    puda.lchContact = pudw.lchContact;
    puda.PluginWindowProc = pudw.PluginWindowProc;
    puda.PluginData = pudw.PluginData;
    {
      char* szAnsi;

      szAnsi = tchar_to_ansi((TCHAR*)pudw.lptzContactName);
      if (szAnsi)
        strncpy(puda.lpzContactName, szAnsi, MAX_CONTACTNAME);
      else
        strcpy(puda.lpzContactName, "");
      mir_free(szAnsi);
      szAnsi = tchar_to_ansi((TCHAR*)pudw.lptzText);
      if (szAnsi)
        strncpy(puda.lpzText, szAnsi, MAX_SECONDLINE);
      else
        strcpy(puda.lpzText, "");
      mir_free(szAnsi);
    }
		if (CallService(MS_POPUP_ADDPOPUPEX, (WPARAM)&puda, 0) < 0)
		{ // popup creation failed, release popupdata
			FreePopupEventData(pdata);
			mir_free(pdata);
		}
	}
	if (dbe.pBlob)
		mir_free(dbe.pBlob);

	return 0;
}


int PopupUpdate(HANDLE hContact, HANDLE hEvent)
{
	PLUGIN_DATA* pdata;
	DBEVENTINFO dbe = {0};
	EVENT_DATA_EX* eventData;
	TCHAR lpzText[MAX_SECONDLINE*2] = _T("\0\0");
	char timestamp[MAX_DATASIZE] = "";
	char formatTime[MAX_DATASIZE] = "";
	int iEvent = 0;
	int doReverse = 0;

  // merge only message popups
	pdata = (PLUGIN_DATA*)PopUpList[NumberPopupData(hContact, EVENTTYPE_MESSAGE)];

	if (hEvent)
	{
		pdata->countEvent++;

		pdata->lastEventData->next = malloc(sizeof(EVENT_DATA_EX));
		pdata->lastEventData->next->prev = pdata->lastEventData;
		pdata->lastEventData = pdata->lastEventData->next;
		pdata->lastEventData->hEvent = hEvent;
		pdata->lastEventData->number = pdata->lastEventData->prev->number + 1;
		pdata->lastEventData->next = NULL;
		if (!pdata->pluginOptions->bShowON && pdata->countEvent > pdata->pluginOptions->iNumberMsg && pdata->pluginOptions->iNumberMsg)
			pdata->firstShowEventData = pdata->firstShowEventData->next;
		//re-init timer delay
		KillTimer(pdata->hWnd, TIMER_TO_ACTION);
		SetTimer(pdata->hWnd, TIMER_TO_ACTION, pdata->iSeconds * 1000, NULL);
	}

	if (pdata->pluginOptions->bShowHeaders)
	{
		WCHAR tmp[MAX_PATH];

		_snprintfT(lpzText, SIZEOF(lpzText), "[b]%s %d[/b]\n", NENTranslateT("Number of new message: ", tmp), pdata->countEvent);
	}

	doReverse = pdata->pluginOptions->bShowON;

	if ((pdata->firstShowEventData != pdata->firstEventData && doReverse) ||
		  (pdata->firstShowEventData != pdata->lastEventData && !doReverse))
		_snprintfT(lpzText, SIZEOF(lpzText), "%s...\n", lpzText);


	//take the active event as starting one
	eventData = pdata->firstShowEventData;

	while (TRUE)
	{
		if (iEvent)
		{
			if (doReverse)
			{
				eventData = eventData->next;
			}
			else
			{
				eventData = eventData->prev;
			}
		}
		iEvent++;
		//get DBEVENTINFO with pBlob if preview is needed (when is test then is off)
		dbe.cbSize = sizeof(dbe);
		dbe.pBlob = NULL;
		dbe.cbBlob = 0;
		if (pdata->pluginOptions->bPreview && eventData->hEvent)
		{
			dbe.cbBlob = CallService(MS_DB_EVENT_GETBLOBSIZE, (WPARAM)eventData->hEvent, 0);
			dbe.pBlob = (PBYTE)malloc(dbe.cbBlob);
		}
		if (eventData->hEvent)
			CallService(MS_DB_EVENT_GET, (WPARAM)eventData->hEvent, (LPARAM)&dbe);
		if (pdata->pluginOptions->bShowDate || pdata->pluginOptions->bShowTime)
		{
			strncpy(formatTime,"",sizeof(formatTime));
			if (pdata->pluginOptions->bShowDate)
				strncpy(formatTime, "%Y.%m.%d ", sizeof(formatTime));
			if (pdata->pluginOptions->bShowTime)
				strncat(formatTime, "%H:%M", sizeof(formatTime));
			strftime(timestamp,sizeof(timestamp), formatTime, localtime((time_t *)&dbe.timestamp));
			if (g_UnicodeCore)
				_snprintfT(lpzText, SIZEOF(lpzText), "%s[b][i]%S[/i][/b]\n", lpzText, timestamp);
			else
				_snprintfT(lpzText, SIZEOF(lpzText), "%s[b][i]%s[/i][/b]\n", lpzText, timestamp);
		}
    { // prepare event preview
			TCHAR* szEventPreview = GetEventPreview(&dbe);

			_snprintfT(lpzText, SIZEOF(lpzText), "%s%s", lpzText, szEventPreview);
			mir_free(szEventPreview);
		}
		if (dbe.pBlob)
			mir_free(dbe.pBlob);
		if (doReverse)
		{
			if ((iEvent >= pdata->pluginOptions->iNumberMsg && pdata->pluginOptions->iNumberMsg) || !eventData->next)
				break;
		}
		else
		{
			if ((iEvent >= pdata->pluginOptions->iNumberMsg && pdata->pluginOptions->iNumberMsg) || !eventData->prev)
				break;
		}

		_snprintfT(lpzText, SIZEOF(lpzText), "%s\n", lpzText);
	}
	if ((doReverse && eventData->next) || (!doReverse && eventData->prev))
	{
		_snprintfT(lpzText, SIZEOF(lpzText), "%s\n...", lpzText);
	}
	if (pdata->isUnicode)
	{
		CallService(MS_POPUP_CHANGETEXTW, (WPARAM)pdata->hWnd, (LPARAM)lpzText);
	}
	else
	{
		char* szAnsi = tchar_to_ansi(lpzText);
		CallService(MS_POPUP_CHANGETEXT, (WPARAM)pdata->hWnd, (LPARAM)szAnsi);
		mir_free(szAnsi);
	}
	return 0;
}

int PopupPreview(PLUGIN_OPTIONS* pluginOptions)
{
  PopupShow(pluginOptions, NULL, NULL, EVENTTYPE_MESSAGE);
  PopupShow(pluginOptions, NULL, NULL, EVENTTYPE_URL);
  PopupShow(pluginOptions, NULL, NULL, EVENTTYPE_FILE);
  PopupShow(pluginOptions, NULL, NULL, -1);

  return 0;
}


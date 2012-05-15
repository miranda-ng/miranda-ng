/*
Scriver

Copyright 2000-2009 Miranda ICQ/IM project,

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
#include "commonheaders.h"

extern HINSTANCE g_hInst;

static MessageSendQueueItem *global_sendQueue = NULL;
static CRITICAL_SECTION queueMutex;
static char *MsgServiceName(HANDLE hContact)
{
#ifdef _UNICODE
    char szServiceName[100];
    char *szProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hContact, 0);
    if (szProto == NULL)
        return PSS_MESSAGE;

    mir_snprintf(szServiceName, sizeof(szServiceName), "%s%sW", szProto, PSS_MESSAGE);
    if (ServiceExists(szServiceName))
        return PSS_MESSAGE "W";
#endif
    return PSS_MESSAGE;
}

TCHAR * GetSendBufferMsg(MessageSendQueueItem *item) {
    TCHAR *szMsg = NULL;
    size_t len = strlen(item->sendBuffer);
#if defined( _UNICODE )
    if (item->flags & PREF_UTF) {
        szMsg = mir_utf8decodeW(item->sendBuffer);
    } else {
        szMsg = (TCHAR *)mir_alloc(item->sendBufferSize - len - 1);
        memcpy(szMsg, item->sendBuffer + len + 1, item->sendBufferSize - len - 1);
    }
#else
    if (item->flags & PREF_UTF) {
        szMsg = mir_utf8decodecp(mir_strdup(item->sendBuffer), item->codepage, NULL);
    } else {
		szMsg = (char *)mir_alloc(len + 1);
		memcpy(szMsg, item->sendBuffer, len + 1);
    }
#endif
    return szMsg;
}

void InitSendQueue() {
	InitializeCriticalSection(&queueMutex);
}

void DestroySendQueue() {
	DeleteCriticalSection(&queueMutex);
}

MessageSendQueueItem* CreateSendQueueItem(HWND hwndSender) {
	MessageSendQueueItem *item = (MessageSendQueueItem *) mir_alloc(sizeof(MessageSendQueueItem));
	EnterCriticalSection(&queueMutex);
	ZeroMemory(item, sizeof(MessageSendQueueItem));
	item->hwndSender = hwndSender;
	item->next = global_sendQueue;
	if (global_sendQueue != NULL) {
		global_sendQueue->prev = item;
	}
	global_sendQueue = item;
	LeaveCriticalSection(&queueMutex);
	return item;
}

MessageSendQueueItem* FindOldestPendingSendQueueItem(HWND hwndSender, HANDLE hContact) {
	MessageSendQueueItem *item, *found = NULL;
	EnterCriticalSection(&queueMutex);
	for (item = global_sendQueue; item != NULL; item = item->next) {
		if (item->hwndSender == hwndSender && item->hContact == hContact && item->hwndErrorDlg == NULL) {
			found = item;
		}
	}
	LeaveCriticalSection(&queueMutex);
	return found;
}

MessageSendQueueItem* FindSendQueueItem(HANDLE hContact, HANDLE hSendId) {
	MessageSendQueueItem *item;
	EnterCriticalSection(&queueMutex);
	for (item = global_sendQueue; item != NULL; item = item->next) {
		if (item->hContact == hContact && item->hSendId == hSendId) {
			break;
		}
	}
	LeaveCriticalSection(&queueMutex);
	return item;
}

BOOL RemoveSendQueueItem(MessageSendQueueItem* item) {
	BOOL result = TRUE;
	HWND hwndSender = item->hwndSender;
//	logInfo(" removing [%s] next: [%s] prev: [%s]", item->sendBuffer, item->next != NULL ? item->next->sendBuffer : "", item->prev != NULL ? item->prev->sendBuffer : "");
	EnterCriticalSection(&queueMutex);
	if (item->prev != NULL) {
		item->prev->next = item->next;
	} else {
		global_sendQueue = item->next;
	}
	if (item->next != NULL) {
		item->next->prev = item->prev;
	}
	if (item->sendBuffer) {
 		mir_free(item->sendBuffer);
	}
	if (item->proto) {
 		mir_free(item->proto);
	}
	mir_free(item);
	for (item = global_sendQueue; item != NULL; item = item->next) {
		if (item->hwndSender == hwndSender) {
			result = FALSE;
		}
	}
	LeaveCriticalSection(&queueMutex);
	return result;
}

void ReportSendQueueTimeouts(HWND hwndSender) {
	MessageSendQueueItem *item, *item2;
	int timeout = DBGetContactSettingDword(NULL, SRMMMOD, SRMSGSET_MSGTIMEOUT, SRMSGDEFSET_MSGTIMEOUT);
	EnterCriticalSection(&queueMutex);
	for (item = global_sendQueue; item != NULL; item = item2) {
		item2 = item->next;
//		logInfo(" item in the queue [%s] next: [%s] prev: [%s]", item->sendBuffer, item->next != NULL ? item->next->sendBuffer : "", item->prev != NULL ? item->prev->sendBuffer : "");
		if (item->timeout < timeout) {
			item->timeout += 1000;
			if (item->timeout >= timeout) {
				if (item->hwndSender == hwndSender && item->hwndErrorDlg == NULL) {
					if (hwndSender != NULL) {
						ErrorWindowData *ewd = (ErrorWindowData *) mir_alloc(sizeof(ErrorWindowData));
						ewd->szName = GetNickname(item->hContact, item->proto);
						ewd->szDescription = mir_tstrdup(TranslateT("The message send timed out."));
						ewd->szText = GetSendBufferMsg(item);
						ewd->hwndParent = hwndSender;
						ewd->queueItem = item;
						PostMessage(hwndSender, DM_SHOWERRORMESSAGE, 0, (LPARAM)ewd);
					} else {
						/* TODO: Handle errors outside messaging window in a better way */
						RemoveSendQueueItem(item);
					}
				}
			}
		}
	}
	LeaveCriticalSection(&queueMutex);
}

void ReleaseSendQueueItems(HWND hwndSender) {
	MessageSendQueueItem *item;
	EnterCriticalSection(&queueMutex);
	for (item = global_sendQueue; item != NULL; item = item->next) {
		if (item->hwndSender == hwndSender) {
			item->hwndSender = NULL;
			if (item->hwndErrorDlg != NULL) {
				DestroyWindow(item->hwndErrorDlg);
			}
			item->hwndErrorDlg = NULL;
		}
	}
	LeaveCriticalSection(&queueMutex);
}

int ReattachSendQueueItems(HWND hwndSender, HANDLE hContact) {
	int count = 0;
	MessageSendQueueItem *item;
	EnterCriticalSection(&queueMutex);
	for (item = global_sendQueue; item != NULL; item = item->next) {
		if (item->hContact == hContact && item->hwndSender == NULL) {
			item->hwndSender = hwndSender;
			item->timeout = 0;
			count++;
//			logInfo(" reattaching [%s]", item->sendBuffer);
		}
	}
	LeaveCriticalSection(&queueMutex);
	return count;
}


void RemoveAllSendQueueItems() {
	MessageSendQueueItem *item, *item2;
	EnterCriticalSection(&queueMutex);
	for (item = global_sendQueue; item != NULL; item = item2) {
		item2 = item->next;
		RemoveSendQueueItem(item);
	}
	LeaveCriticalSection(&queueMutex);
}

void SendSendQueueItem(MessageSendQueueItem* item) {
	EnterCriticalSection(&queueMutex);
	item->timeout = 0;
//	logInfo(" sending item  [%s] next: [%s] prev: [%s]", item->sendBuffer, item->next != NULL ? item->next->sendBuffer : "", item->prev != NULL ? item->prev->sendBuffer : "");
	if (item->prev != NULL) {
		item->prev->next = item->next;
		if (item->next != NULL) {
			item->next->prev = item->prev;
		}
		item->next = global_sendQueue;
		item->prev = NULL;
		if (global_sendQueue != NULL) {
			global_sendQueue->prev = item;
		}
		global_sendQueue = item;
	}
//	logInfo(" item sent [%s] next: [%s] prev: [%s]", item->sendBuffer, item->next != NULL ? item->next->sendBuffer : "", item->prev != NULL ? item->prev->sendBuffer : "");
	LeaveCriticalSection(&queueMutex);
	item->hSendId = (HANDLE) CallContactService(item->hContact, MsgServiceName(item->hContact), item->flags, (LPARAM) item->sendBuffer);
}

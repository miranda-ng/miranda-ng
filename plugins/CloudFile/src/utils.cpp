#include "stdafx.h"

void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, MCONTACT hContact)
{
	if (Miranda_IsTerminated())
	{
		return;
	}

	if (ServiceExists(MS_POPUP_ADDPOPUPT) && db_get_b(NULL, "Popup", "ModuleIsEnabled", 1))
	{
		POPUPDATAT ppd = { 0 };
		ppd.lchContact = hContact;
		wcsncpy(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		wcsncpy(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = IcoLib_GetIcon("Slack_main");

		if (!PUAddPopupT(&ppd))
			return;
	}

	MessageBox(NULL, message, caption, MB_OK | flags);
}

void ShowNotification(const wchar_t *message, int flags, MCONTACT hContact)
{
	ShowNotification(_A2W(MODULE), message, flags, hContact);
}

MEVENT AddEventToDb(MCONTACT hContact, WORD type, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei = {};
	dbei.szModule = MODULE;
	dbei.timestamp = time(NULL);
	dbei.eventType = type;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = flags;
	return db_event_add(hContact, &dbei);
}

bool CanSendToContact(MCONTACT hContact)
{
	if (!hContact)
		return false;

	const char *proto = GetContactProto(hContact);
	if (!proto)
		return false;

	bool canSend = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND) != 0;
	if (!canSend)
		return false;

	bool isProtoOnline = CallProtoService(proto, PS_GETSTATUS, 0, 0) > ID_STATUS_OFFLINE;
	if (isProtoOnline)
		return true;

	bool isContactOnline = db_get_w(hContact, proto, "Status", ID_STATUS_OFFLINE) > ID_STATUS_OFFLINE;
	if (isContactOnline)
		return true;

	return CallProtoService(proto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE;
}

void SendToContact(MCONTACT hContact, const wchar_t *data)
{
	const char *szProto = GetContactProto(hContact);
	if (db_get_b(hContact, szProto, "ChatRoom", 0) == TRUE) {
		ptrW tszChatRoom(db_get_wsa(hContact, szProto, "ChatRoomID"));
		Chat_SendUserMessage(szProto, tszChatRoom, data);
		return;
	}

	char *message = mir_utf8encodeW(data);
	if (ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)message) != ACKRESULT_FAILED)
		AddEventToDb(hContact, EVENTTYPE_MESSAGE, DBEF_UTF | DBEF_SENT, (DWORD)mir_strlen(message), (PBYTE)message);
}

void PasteToInputArea(MCONTACT hContact, const wchar_t *data)
{
	CallService(MS_MSG_SENDMESSAGEW, hContact, (LPARAM)data);
}

void PasteToClipboard(const wchar_t *data)
{
	if (OpenClipboard(NULL)) {
		EmptyClipboard();

		size_t size = sizeof(wchar_t) * (mir_wstrlen(data) + 1);
		HGLOBAL hClipboardData = GlobalAlloc(NULL, size);
		if (hClipboardData) {
			wchar_t *pchData = (wchar_t*)GlobalLock(hClipboardData);
			if (pchData) {
				memcpy(pchData, (wchar_t*)data, size);
				GlobalUnlock(hClipboardData);
				SetClipboardData(CF_UNICODETEXT, hClipboardData);
			}
		}
		CloseClipboard();
	}
}

#include "stdafx.h"

void ShowNotification(const wchar_t *caption, const wchar_t *message, int flags, MCONTACT hContact)
{
	if (Miranda_IsTerminated())
		return;

	if (Popup_Enabled()) {
		POPUPDATAW ppd;
		ppd.lchContact = hContact;
		wcsncpy(ppd.lpwzContactName, caption, MAX_CONTACTNAME);
		wcsncpy(ppd.lpwzText, message, MAX_SECONDLINE);
		ppd.lchIcon = IcoLib_GetIcon("Slack_main");
		if (!PUAddPopupW(&ppd))
			return;
	}

	MessageBox(nullptr, message, caption, MB_OK | flags);
}

void ShowNotification(const wchar_t *message, int flags, MCONTACT hContact)
{
	ShowNotification(_A2W(MODULENAME), message, flags, hContact);
}

MEVENT AddEventToDb(MCONTACT hContact, uint16_t type, uint32_t flags, uint32_t cbBlob, uint8_t *pBlob)
{
	DBEVENTINFO dbei = {};
	dbei.szModule = MODULENAME;
	dbei.timestamp = time(0);
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

	const char *proto = Proto_GetBaseAccountName(hContact);
	if (!proto)
		return false;

	bool isCtrlPressed = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
	if (isCtrlPressed)
		return true;

	bool canSend = (CallProtoService(proto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IMSEND) != 0;
	if (!canSend)
		return false;

	bool isProtoOnline = Proto_GetStatus(proto) > ID_STATUS_OFFLINE;
	if (!isProtoOnline)
		return false;

	bool isContactOnline = Contact::GetStatus(hContact) > ID_STATUS_OFFLINE;
	if (isContactOnline)
		return true;

	return CallProtoService(proto, PS_GETCAPS, PFLAGNUM_4, 0) & PF4_IMSENDOFFLINE;
}

void SendToContact(MCONTACT hContact, const wchar_t *data)
{
	const char *szProto = Proto_GetBaseAccountName(hContact);
	if (Contact::IsGroupChat(hContact, szProto)) {
		ptrW tszChatRoom(db_get_wsa(hContact, szProto, "ChatRoomID"));
		Chat_SendUserMessage(szProto, tszChatRoom, data);
		return;
	}

	char *message = mir_utf8encodeW(data);
	if (ProtoChainSend(hContact, PSS_MESSAGE, 0, (LPARAM)message) != ACKRESULT_FAILED)
		AddEventToDb(hContact, EVENTTYPE_MESSAGE, DBEF_UTF | DBEF_SENT, (uint32_t)mir_strlen(message), (uint8_t*)message);
}

void PasteToInputArea(MCONTACT hContact, const wchar_t *data)
{
	CallService(MS_MSG_SENDMESSAGEW, hContact, (LPARAM)data);
}

void PasteToClipboard(const wchar_t *data)
{
	if (OpenClipboard(nullptr)) {
		EmptyClipboard();

		size_t size = sizeof(wchar_t) * (mir_wstrlen(data) + 1);
		HGLOBAL hClipboardData = GlobalAlloc(NULL, size);
		if (hClipboardData) {
			wchar_t *pchData = (wchar_t*)GlobalLock(hClipboardData);
			mir_wstrcpy(pchData, data);
			GlobalUnlock(hClipboardData);
			SetClipboardData(CF_UNICODETEXT, hClipboardData);
		}
		CloseClipboard();
	}
}

void Report(MCONTACT hContact, const wchar_t *data)
{
	if (g_plugin.getByte("UrlAutoSend", 1))
		SendToContact(hContact, data);

	if (g_plugin.getByte("UrlPasteToMessageInputArea", 0))
		PasteToInputArea(hContact, data);

	if (g_plugin.getByte("UrlCopyToClipboard", 0))
		PasteToClipboard(data);
}

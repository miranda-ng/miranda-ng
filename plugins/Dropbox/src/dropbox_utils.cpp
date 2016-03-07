#include "stdafx.h"

char* CDropbox::PreparePath(const char *oldPath, char *newPath)
{
	if (oldPath == NULL)
		mir_strcpy(newPath, "");
	else if (*oldPath != '/')
	{
		CMStringA result("/");
		result.Append(oldPath);
		result.Replace("\\", "/");
		mir_strcpy(newPath, result);
	}
	else
		mir_strcpy(newPath, oldPath);
	return newPath;
}

char* CDropbox::PreparePath(const TCHAR *oldPath, char *newPath)
{
	return PreparePath(ptrA(mir_utf8encodeW(oldPath)), newPath);
}

bool CDropbox::IsAccountIntercepted(const char *module)
{
	const char *interceptedAccounts = db_get_sa(NULL, MODULE, "InterceptedAccounts");
	if (interceptedAccounts == NULL)
		interceptedAccounts = db_get_sa(NULL, MODULE, "InterceptedProtos");
	return interceptedAccounts && strstr(interceptedAccounts, module);
}

char* CDropbox::HttpStatusToText(HTTP_STATUS status)
{
	switch (status) {
	case HTTP_STATUS_ERROR:
		return "Server does not respond";
	case HTTP_STATUS_OK:
		return "OK";
	case HTTP_STATUS_BAD_REQUEST:
		return "Bad input parameter. Error message should indicate which one and why";
	case HTTP_STATUS_UNAUTHORIZED:
		return "Bad or expired token. This can happen if the user or Dropbox revoked or expired an access token. To fix, you should re-authenticate the user";
	case HTTP_STATUS_FORBIDDEN:
		return "Bad OAuth request (wrong consumer key, bad nonce, expired timestamp...). Unfortunately, re-authenticating the user won't help here";
	case HTTP_STATUS_NOT_FOUND:
		return "File or folder not found at the specified path";
	case HTTP_STATUS_METHOD_NOT_ALLOWED:
		return "Request method not expected (generally should be GET or POST)";
	case HTTP_STATUS_TOO_MANY_REQUESTS:
		return "Your app is making too many requests and is being rate limited. 429s can trigger on a per-app or per-user basis";
	case HTTP_STATUS_SERVICE_UNAVAILABLE:
		return "If the response includes the Retry-After header, this means your OAuth 1.0 app is being rate limited. Otherwise, this indicates a transient server error, and your app should retry its request.";
	case HTTP_STATUS_INSUFICIENTE_STORAGE:
		return "User is over Dropbox storage quota";
	}

	return "Unknown error";
}

void CDropbox::HandleJsonResponseError(NETLIBHTTPREQUEST *response)
{
	if (response == NULL)
		throw DropboxException(HttpStatusToText(HTTP_STATUS_ERROR));

	if (response->resultCode == HTTP_STATUS_OK)
		return;

	if (response->resultCode != HTTP_STATUS_CONFLICT) {
		if (response->dataLength)
			throw DropboxException(response->pData);
		throw DropboxException(HttpStatusToText((HTTP_STATUS)response->resultCode));
	}

	JSONNode root = JSONNode::parse(response->pData);
	if (root.empty())
		throw DropboxException(HttpStatusToText(HTTP_STATUS_ERROR));

	JSONNode error = root.at("error_summary");
	if (error.empty())
		return;

	throw DropboxException(error.as_string().c_str());
}

MEVENT CDropbox::AddEventToDb(MCONTACT hContact, WORD type, DWORD flags, DWORD cbBlob, PBYTE pBlob)
{
	DBEVENTINFO dbei;
	dbei.cbSize = sizeof(dbei);
	dbei.szModule = MODULE;
	dbei.timestamp = time(NULL);
	dbei.eventType = type;
	dbei.cbBlob = cbBlob;
	dbei.pBlob = pBlob;
	dbei.flags = flags;
	return db_event_add(hContact, &dbei);
}

void CDropbox::SendToContact(MCONTACT hContact, const TCHAR *data)
{
	if (hContact == GetDefaultContact()) {
		char *message = mir_utf8encodeT(data);
		AddEventToDb(hContact, EVENTTYPE_MESSAGE, DBEF_UTF, (DWORD)mir_strlen(message), (PBYTE)message);
		return;
	}

	const char *szProto = GetContactProto(hContact);
	if (db_get_b(hContact, szProto, "ChatRoom", 0) == TRUE) {
		ptrT tszChatRoom(db_get_tsa(hContact, szProto, "ChatRoomID"));
		GCDEST gcd = { szProto, tszChatRoom, GC_EVENT_SENDMESSAGE };
		GCEVENT gce = { sizeof(gce), &gcd };
		gce.bIsMe = TRUE;
		gce.dwFlags = GCEF_ADDTOLOG;
		gce.ptszText = mir_tstrdup(data);
		gce.time = time(NULL);
		CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE, (LPARAM)&gce);
		mir_free((void*)gce.ptszText);
		return;
	}

	if (CallContactService(hContact, PSS_MESSAGE, 0, T2Utf(data)) != ACKRESULT_FAILED) {
		char *message = mir_utf8encodeT(data);
		AddEventToDb(hContact, EVENTTYPE_MESSAGE, DBEF_UTF | DBEF_SENT, (DWORD)mir_strlen(message), (PBYTE)message);
	}
}

void CDropbox::PasteToInputArea(MCONTACT hContact, const TCHAR *data)
{
	MessageWindowInputData mwid = { sizeof(MessageWindowInputData) };
	mwid.hContact = hContact;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	MessageWindowData mwd = { sizeof(MessageWindowData) };
	if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd)) {
		HWND hEdit = GetDlgItem(mwd.hwndWindow, 1002 /*IDC_MESSAGE*/);
		if (!hEdit) hEdit = GetDlgItem(mwd.hwndWindow, 1009 /*IDC_CHATMESSAGE*/);

		SendMessage(hEdit, EM_REPLACESEL, TRUE, (LPARAM)data);
	}
}

void CDropbox::PasteToClipboard(const TCHAR *data)
{
	if (OpenClipboard(NULL)) {
		EmptyClipboard();

		size_t size = sizeof(TCHAR) * (mir_tstrlen(data) + 1);
		HGLOBAL hClipboardData = GlobalAlloc(NULL, size);
		if (hClipboardData) {
			TCHAR *pchData = (TCHAR*)GlobalLock(hClipboardData);
			if (pchData) {
				memcpy(pchData, (TCHAR*)data, size);
				GlobalUnlock(hClipboardData);
				SetClipboardData(CF_UNICODETEXT, hClipboardData);
			}
		}
		CloseClipboard();
	}
}

void CDropbox::Report(MCONTACT hContact, const TCHAR *data)
{
	if (db_get_b(NULL, MODULE, "UrlAutoSend", 1))
		SendToContact(hContact, data);

	if (db_get_b(NULL, MODULE, "UrlPasteToMessageInputArea", 0))
		PasteToInputArea(hContact, data);

	if (db_get_b(NULL, MODULE, "UrlCopyToClipboard", 0))
		PasteToClipboard(data);
}

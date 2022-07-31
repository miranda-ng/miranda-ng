#include "stdafx.h"

mir_cs lib_cs;

void lib_cs_lock()
{
	mir_cslock lck(lib_cs);
}

MCONTACT find_contact(const char* userid, const char* protocol)
{
	for (auto &hContact : Contacts()) {
		const char *proto = Proto_GetBaseAccountName(hContact);
		if (proto && mir_strcmp(proto, protocol) == 0) {
			ptrA name(contact_get_id(hContact));
			if (name && mir_strcmp(name, userid) == 0)
				return hContact;
		}
	}

	return 0;
}

/* Look up a connection context by hContact from the given
* OtrlUserState.  If add_if_missing is true, allocate and return a new
* context if one does not currently exist.  In that event, call
* add_app_data(data, context) so that app_data and app_data_free can be
* filled in by the application, and set *addedp to 1. */
ConnContext* otrl_context_find_miranda(OtrlUserState us, MCONTACT hContact)
{
	const char *proto = Proto_GetBaseAccountName(hContact);
	return otrl_context_find(us, ptrA(contact_get_id(hContact)), proto, proto, OTRL_INSTAG_BEST, 0, nullptr, nullptr, nullptr);
}

/* What level of trust do we have in the privacy of this ConnContext? */
TrustLevel otr_context_get_trust(ConnContext *context)
{
	TrustLevel level = TRUST_NOT_PRIVATE;

	if (context && context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {
		level = TRUST_UNVERIFIED;
		if (context->active_fingerprint)
			if (context->active_fingerprint->trust && context->active_fingerprint->trust[0])
				level = TRUST_PRIVATE;
	}
	else if (context && context->msgstate == OTRL_MSGSTATE_FINISHED)
		level = TRUST_FINISHED;

	return level;
}

/* Set verification of fingerprint */
void VerifyFingerprint(ConnContext *context, bool verify)
{
	lib_cs_lock();
	otrl_context_set_trust(context->active_fingerprint, (verify) ? "verified" : nullptr);
	otrl_privkey_write_fingerprints(otr_user_state, g_fingerprint_store_filename);
	VerifyFingerprintMessage(context, verify);
}

void VerifyFingerprintMessage(ConnContext *context, bool verify)
{
	MCONTACT hContact = (UINT_PTR)context->app_data;

	wchar_t msg[1024];
	mir_snwprintf(msg, (verify) ? TranslateW(LANG_FINGERPRINT_VERIFIED) : TranslateW(LANG_FINGERPRINT_NOT_VERIFIED), contact_get_nameT(hContact));
	ShowMessage(hContact, msg);
	SetEncryptionStatus(hContact, otr_context_get_trust(context));
}

/* Convert a 20-byte hash value to a 45-byte human-readable value */
void otrl_privkey_hash_to_humanT(wchar_t human[45], const unsigned char hash[20])
{
	int word, byte;
	wchar_t *p = human;

	for (word = 0; word < 5; ++word) {
		for (byte = 0; byte < 4; ++byte) {
			swprintf(p, L"%02X", hash[word * 4 + byte]);
			p += 2;
		}
		*(p++) = ' ';
	}
	/* Change that last ' ' to a '\0' */
	--p;
	*p = '\0';
}

char* contact_get_id(MCONTACT hContact)
{
	ptrW pszUniqueID(Contact::GetInfo(CNF_UNIQUEID, hContact));
	if (!pszUniqueID)
		pszUniqueID = mir_wstrdup(Clist_GetContactDisplayName(hContact));

	return mir_utf8encodeW(pszUniqueID);
}

__inline const wchar_t* contact_get_nameT(MCONTACT hContact)
{
	return Clist_GetContactDisplayName(hContact);
}

char* GetDlgItemTextUtf(HWND hwndDlg, int ctrlId)
{
	wchar_t buf[1024];
	int len = GetDlgItemTextW(hwndDlg, ctrlId, buf, _countof(buf));
	buf[len] = 0;
	return mir_utf8encodeW(buf);
}

wchar_t* ProtoGetNickname(const char* proto)
{
	wchar_t *p = Contact::GetInfo(CNF_NICK, NULL, proto);
	return (p != nullptr) ? p : mir_wstrdup(L"");
}

void ShowPopup(const wchar_t* line1, const wchar_t* line2, int timeout, const MCONTACT hContact)
{
	if (Miranda_IsTerminated())
		return;

	POPUPDATAW ppd;
	ppd.lchContact = hContact;
	ppd.lchIcon = nullptr;

	if (line1 && line2) {
		wcsncpy(ppd.lpwzContactName, line1, MAX_CONTACTNAME - 1);
		wcsncpy(ppd.lpwzText, line2, MAX_SECONDLINE - 1);
	}
	else if (line1)
		wcsncpy(ppd.lpwzText, line1, MAX_SECONDLINE - 1);
	else if (line2)
		wcsncpy(ppd.lpwzText, line2, MAX_SECONDLINE - 1);

	ppd.iSeconds = timeout;
	ppd.PluginWindowProc = nullptr;
	ppd.PluginData = nullptr;
	PUAddPopupW(&ppd);
}

void ShowWarning(wchar_t *msg)
{
	wchar_t buffer[512];
	mir_snwprintf(buffer, L"%s Warning", _A2W(MODULENAME));

	switch (options.err_method) {
	case ED_POP:
		{
			int size = int(mir_wstrlen(msg) + 515);
			wchar_t *message = new wchar_t[size]; // newline and null terminator
			mir_snwprintf(message, size, L"%s\r\n%s", buffer, msg);
			PUShowMessageW(message, SM_WARNING);
			delete[] message;
		}
		break;
	case ED_MB:
		MessageBox(nullptr, msg, buffer, MB_OK | MB_ICONWARNING);
		break;
	case ED_BAL:
		Clist_TrayNotifyW(MODULENAME, buffer, msg, NIIF_WARNING, 10000);
		break;
	}
}

void ShowError(wchar_t *msg)
{
	wchar_t buffer[512];
	mir_snwprintf(buffer, L"%s Error", _A2W(MODULENAME));

	wchar_t *message;
	switch (options.err_method) {
	case ED_POP:
		{
			int size = int(mir_wstrlen(msg) + 515);
			message = new wchar_t[size]; // newline and null terminator
			mir_snwprintf(message, size, L"%s\r\n%s", buffer, msg);
			PUShowMessageW(message, SM_WARNING);
			delete[] message;
		}
		break;
	case ED_MB:
		MessageBox(nullptr, msg, buffer, MB_OK | MB_ICONERROR);
		break;
	case ED_BAL:
		Clist_TrayNotifyW(MODULENAME, buffer, msg, NIIF_ERROR, 10000);
		break;
	}
}

void CopyToClipboard(const wchar_t *pwszMsg)
{
	if (pwszMsg == nullptr)
		return;

	HGLOBAL hglbCopy = GlobalAlloc(GMEM_MOVEABLE, (mir_wstrlen(pwszMsg) + 1) * sizeof(wchar_t));
	LPWSTR lptstrCopy = (LPWSTR)GlobalLock(hglbCopy);
	mir_wstrcpy(lptstrCopy, pwszMsg);
	GlobalUnlock(hglbCopy);

	if (OpenClipboard(nullptr)) {
		EmptyClipboard();
		SetClipboardData(CF_UNICODETEXT, hglbCopy);
		CloseClipboard();
	}
}

void ShowPopupUtf(const char* line1, const char* line2, int timeout, const MCONTACT hContact)
{
	wchar_t* l1 = (line1) ? mir_utf8decodeW(line1) : nullptr;
	wchar_t* l2 = (line2) ? mir_utf8decodeW(line2) : nullptr;
	ShowPopup(l1, l2, timeout, hContact);
	if (l1) mir_free(l1);
	if (l2) mir_free(l2);
}

void ShowWarningUtf(char* msg)
{
	wchar_t* m = (msg) ? mir_utf8decodeW(msg) : nullptr;
	ShowWarning(m);
	if (m) mir_free(m);
}
void ShowErrorUtf(char* msg)
{
	wchar_t* m = (msg) ? mir_utf8decodeW(msg) : nullptr;
	ShowError(m);
	if (m) mir_free(m);
}

void ShowMessageInline(const MCONTACT hContact, const wchar_t *msg)
{
	wchar_t buff[1024];
	mir_snwprintf(buff, L"%s%s", _A2W(LANG_INLINE_PREFIX), msg);
	T2Utf utf(buff);

	PROTORECVEVENT pre = { 0 };
	pre.timestamp = time(0);
	pre.szMessage = utf;
	pre.flags = PREF_BYPASS_OTR;
	ProtoChainRecvMsg(hContact, &pre);
}

void ShowMessageInlineUtf(const MCONTACT hContact, const char *msg)
{
	char buff[1024];
	mir_snprintf(buff, "%s%s", LANG_INLINE_PREFIX, msg);

	PROTORECVEVENT pre = { 0 };
	pre.timestamp = time(0);
	pre.szMessage = buff;
	pre.flags = PREF_BYPASS_OTR;
	ProtoChainRecvMsg(hContact, &pre);
}

void ShowMessageUtf(const MCONTACT hContact, const char *msg)
{
	if (options.msg_inline)
		ShowMessageInlineUtf(hContact, msg);
	if (options.msg_popup)
		ShowPopupUtf(Translate(LANG_OTR_INFO), msg, 0, hContact);
}

void ShowMessage(const MCONTACT hContact, const wchar_t *msg)
{
	if (options.msg_inline)
		ShowMessageInline(hContact, msg);
	if (options.msg_popup)
		ShowPopup(TranslateT(LANG_OTR_INFO), msg, 0, hContact);
}

const wchar_t *policy_to_string(OtrlPolicy policy)
{
	switch (policy) {
	case OTRL_POLICY_NEVER:
		return TranslateW(LANG_POLICY_NEVER);
	case OTRL_POLICY_OPPORTUNISTIC:
		return TranslateW(LANG_POLICY_OPP);
	case OTRL_POLICY_MANUAL:
	case OTRL_POLICY_MANUAL_MOD:
		return TranslateW(LANG_POLICY_MANUAL);
	case OTRL_POLICY_ALWAYS:
		return TranslateW(LANG_POLICY_ALWAYS);
	default:
		return TranslateW(LANG_POLICY_DEFAULT);
	}
}

OtrlPolicy policy_from_string(const wchar_t *polstring)
{
	if (mir_wstrcmp(polstring, TranslateW(LANG_POLICY_NEVER)) == 0)
		return OTRL_POLICY_NEVER;
	else if (mir_wstrcmp(polstring, TranslateW(LANG_POLICY_OPP)) == 0)
		return OTRL_POLICY_OPPORTUNISTIC;
	else if (mir_wstrcmp(polstring, TranslateW(LANG_POLICY_MANUAL)) == 0)
		return OTRL_POLICY_MANUAL_MOD;
	else if (mir_wstrcmp(polstring, TranslateW(LANG_POLICY_ALWAYS)) == 0)
		return OTRL_POLICY_ALWAYS;
	else
		return CONTACT_DEFAULT_POLICY;
}

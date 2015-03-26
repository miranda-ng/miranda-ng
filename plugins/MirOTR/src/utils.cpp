#include "stdafx.h"

CRITICAL_SECTION lib_cs;

void InitUtils()
{
	InitializeCriticalSection(&lib_cs);
}

void DeinitUtils()
{
	DeleteCriticalSection(&lib_cs);
}

void lib_cs_lock()
{
	EnterCriticalSection(&lib_cs);
}

void lib_cs_unlock()
{
	LeaveCriticalSection(&lib_cs);
}

MCONTACT find_contact(const char* userid, const char* protocol)
{
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		const char *proto = GetContactProto(hContact);
		if(proto && strcmp(proto, protocol) == 0) {
			char *name = contact_get_id(hContact);
			if(name && strcmp(name, userid) == 0) {
				mir_free(name);
				return hContact;
			}
			mir_free(name);
		}
	}
	
	return 0;
}

/* Look up a connection context by hContact from the given
* OtrlUserState.  If add_if_missing is true, allocate and return a new
* context if one does not currently exist.  In that event, call
* add_app_data(data, context) so that app_data and app_data_free can be
* filled in by the application, and set *addedp to 1. */
ConnContext * otrl_context_find_miranda(OtrlUserState us, MCONTACT hContact)
{
	const char *proto = GetContactProto(hContact);
	char *username = contact_get_id(hContact);
	ConnContext* ret = otrl_context_find(us, username, proto, proto, OTRL_INSTAG_BEST, 0, NULL, NULL, NULL);
	mir_free(username);
	return ret;
}

/* What level of trust do we have in the privacy of this ConnContext? */
TrustLevel otr_context_get_trust(ConnContext *context)
{
	TrustLevel level = TRUST_NOT_PRIVATE;

	if (context && context->msgstate == OTRL_MSGSTATE_ENCRYPTED) {
		if (context->active_fingerprint->trust && context->active_fingerprint->trust[0]) {
			level = TRUST_PRIVATE;
		} else {
			level = TRUST_UNVERIFIED;
		}
	} else if (context && context->msgstate == OTRL_MSGSTATE_FINISHED) {
		level = TRUST_FINISHED;
	}

	return level;
}

/* Set verification of fingerprint */
void VerifyFingerprint(ConnContext *context, bool verify) {
	lib_cs_lock();
	otrl_context_set_trust(context->active_fingerprint, (verify)?"verified":NULL);
	otrl_privkey_write_fingerprints(otr_user_state, g_fingerprint_store_filename);
	lib_cs_unlock();
	VerifyFingerprintMessage(context, verify);
}

void VerifyFingerprintMessage(ConnContext *context, bool verify) {
	MCONTACT hContact = (MCONTACT)context->app_data;
	TCHAR msg[1024];

	mir_sntprintf(msg, SIZEOF(msg), (verify)?TranslateT(LANG_FINGERPRINT_VERIFIED):TranslateT(LANG_FINGERPRINT_NOT_VERIFIED), contact_get_nameT(hContact));
	ShowMessage(hContact, msg);
	SetEncryptionStatus(hContact, otr_context_get_trust(context));
}

/* Convert a 20-byte hash value to a 45-byte human-readable value */
void otrl_privkey_hash_to_humanT(TCHAR human[45], const unsigned char hash[20])
{
	int word, byte;
	TCHAR *p = human;

	for(word=0; word<5; ++word) {
	for(byte=0; byte<4; ++byte) {
		_stprintf(p, _T("%02X"), hash[word*4+byte]); //!!!!!!!!!!!!!!
		p += 2;
	}
	*(p++) = ' ';
	}
	/* Change that last ' ' to a '\0' */
	--p;
	*p = '\0';
}

char* contact_get_id(MCONTACT hContact, bool bNameOnError) {
	char* pszUniqueID = NULL;
	CONTACTINFO ci;
	memset(&ci, 0, sizeof(ci));
	ci.cbSize = sizeof(ci);
	ci.hContact = hContact;
	ci.dwFlag = CNF_UNIQUEID;

	if (CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci) == 0)
	{
		if (ci.type == CNFT_ASCIIZ) {   
			pszUniqueID = (char*)ci.pszVal; // MS_CONTACT_GETCONTACTINFO uses mir_alloc
		} else if (ci.type == CNFT_DWORD)  {
			pszUniqueID = (char*)mir_alloc(15);
			if (pszUniqueID) 
				mir_snprintf(pszUniqueID, 15, ("%u"), ci.dVal);
		} else if (ci.type == CNFT_WORD)  {
			pszUniqueID = (char*)mir_alloc(15);
			if (pszUniqueID)
				mir_snprintf(pszUniqueID, 15, ("%u"), ci.wVal);
		} else if (ci.type == CNFT_BYTE)  {
			pszUniqueID = (char*)mir_alloc(15);
			if (pszUniqueID)
				mir_snprintf(pszUniqueID, 15, ("%u"), ci.bVal);
		}
	}
	if (!pszUniqueID && bNameOnError) {
		char *name = (char *)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, 0);
		if (name) pszUniqueID = mir_strdup(name);
	}
	return pszUniqueID;
}

__inline const TCHAR* contact_get_nameT(MCONTACT hContact) {
	return (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, hContact, GCDNF_TCHAR);
}


__inline const char* contact_get_account(MCONTACT hContact) {
	char *uacc = (char *)CallService(MS_PROTO_GETCONTACTBASEACCOUNT, hContact, 0);
	return uacc;
}

TCHAR* ProtoGetNickname(const char* proto)
{
	CONTACTINFO ci = {sizeof(ci)};
	ci.dwFlag = CNF_TCHAR | CNF_NICK;
	ci.szProto = (char*)proto;
	if (!CallService(MS_CONTACT_GETCONTACTINFO, 0, (LPARAM)&ci)) {
		switch (ci.type) {
		case CNFT_ASCIIZ:
			return ci.pszVal;
		case CNFT_DWORD:
			mir_free(ci.pszVal);
			ci.pszVal=(TCHAR*)mir_alloc(12*sizeof(TCHAR)); // long can only have up to 11 characters (unsigned = 10)
			if(ci.pszVal)
				_ltot(ci.dVal, ci.pszVal, 10);
			return ci.pszVal;
		default:
			mir_free(ci.pszVal);
		}
	}
	return mir_tstrdup(TranslateT("'(Unknown contact)'"));
}

void ShowPopup(const TCHAR* line1, const TCHAR* line2, int timeout, const MCONTACT hContact) {
	if(CallService(MS_SYSTEM_TERMINATED, 0, 0)) return;

	if ( !options.bHavePopups) {	
		TCHAR title[256];
		mir_sntprintf(title, SIZEOF(title), _T("%s Message"), _T(MODULENAME));

		if(line1 && line2) {
			int size = int(_tcslen(line1) + _tcslen(line2) + 3);
			TCHAR *message = new TCHAR[size]; // newline and null terminator
			mir_sntprintf(message, size, _T("%s\r\n%s"), line1, line2);
			MessageBox( NULL, message, title, MB_OK | MB_ICONINFORMATION );
			delete[] message;
		} else if(line1) {
			MessageBox( NULL, line1, title, MB_OK | MB_ICONINFORMATION );
		} else if(line2) {
			MessageBox( NULL, line2, title, MB_OK | MB_ICONINFORMATION );
		}
		return;
	}

	POPUPDATAT ppd = {0};
	//memset((void *)&ppd, 0, sizeof(POPUPDATAT));

	ppd.lchContact = hContact;
	ppd.lchIcon = NULL;

	if(line1 && line2) {
		_tcsncpy( ppd.lptzContactName, line1, MAX_CONTACTNAME-1 );
		_tcsncpy( ppd.lptzText, line2, MAX_SECONDLINE-1 );
	} else if(line1)
		_tcsncpy( ppd.lptzText, line1, MAX_SECONDLINE-1 );
	else if(line2)
		_tcsncpy( ppd.lptzText, line2, MAX_SECONDLINE-1 );

	ppd.iSeconds = timeout;

	ppd.PluginWindowProc = NULL;
	ppd.PluginData = NULL;

	PUAddPopupT(&ppd);

}

void ShowWarning(TCHAR *msg) {
	TCHAR buffer[512];
	ErrorDisplay disp = options.err_method;
	// funny logic :) ... try to avoid message boxes
	// if want baloons but no balloons, try popups
	// if want popups but no popups, try baloons
	// if, after that, you want balloons but no balloons, revert to message boxes
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_POP; 
	if(disp == ED_POP && !options.bHavePopups) disp = ED_BAL;
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_MB;

	mir_sntprintf(buffer, SIZEOF(buffer), _T("%s Warning"), _T(MODULENAME));

	TCHAR *message;
	switch(disp) {
		case ED_POP:
			{
				int size = int(_tcslen(msg) + 515);
				message = new TCHAR[size]; // newline and null terminator
				mir_sntprintf(message, size, _T("%s\r\n%s"), buffer, msg);
				PUShowMessageT(message, SM_WARNING);
				delete message;
			}
			break;
		case ED_MB:
			MessageBox(0, msg, buffer, MB_OK | MB_ICONWARNING);
			break;
		case ED_BAL:
			{
				MIRANDASYSTRAYNOTIFY sn = {0};
				sn.cbSize = sizeof(sn);
				sn.szProto= MODULENAME;
				sn.tszInfoTitle = buffer;
				sn.tszInfo = msg;

				sn.dwInfoFlags = NIIF_WARNING | NIIF_INTERN_UNICODE;

				sn.uTimeout = 10;

				CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&sn);
			}

			break;
	}
}

void ShowError(TCHAR *msg) {
	TCHAR buffer[512];
	ErrorDisplay disp = options.err_method;
	// funny logic :) ... try to avoid message boxes
	// if want baloons but no balloons, try popups
	// if want popups but no popups, try baloons
	// if, after that, you want balloons but no balloons, revert to message boxes
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_POP;
	if(disp == ED_POP && !options.bHavePopups) disp = ED_BAL;
	if(disp == ED_BAL && !ServiceExists(MS_CLIST_SYSTRAY_NOTIFY)) disp = ED_MB;

	mir_sntprintf(buffer, SIZEOF(buffer), _T("%s Error"), _T(MODULENAME));


	TCHAR *message;
	switch(disp) {
		case ED_POP:
			{
				int size = int(_tcslen(msg) + 515);
				message = new TCHAR[size]; // newline and null terminator
				mir_sntprintf(message, size, _T("%s\r\n%s"), buffer, msg);
				PUShowMessageT(message, SM_WARNING);
				delete message;
			}
			break;
		case ED_MB:
			MessageBox(0, msg, buffer, MB_OK | MB_ICONERROR);
			break;
		case ED_BAL:
			{
				MIRANDASYSTRAYNOTIFY sn = {0};
				sn.cbSize = sizeof(sn);
				sn.szProto = MODULENAME;
				sn.tszInfoTitle = buffer;
				sn.tszInfo = msg;

				sn.dwInfoFlags = NIIF_ERROR | NIIF_INTERN_UNICODE;

				sn.uTimeout = 10;

				CallService(MS_CLIST_SYSTRAY_NOTIFY, 0, (LPARAM)&sn);
			}

			break;
	}
}


void ShowPopupUtf(const char* line1, const char* line2, int timeout, const MCONTACT hContact) {
	TCHAR* l1 = (line1) ? mir_utf8decodeT(line1) : NULL;
	TCHAR* l2 = (line2) ? mir_utf8decodeT(line2) : NULL;
	ShowPopup(l1, l2, timeout, hContact);
	if (l1) mir_free(l1);
	if (l2) mir_free(l2);
}

void ShowWarningUtf(char* msg) {
	TCHAR* m = (msg) ? mir_utf8decodeT(msg) : NULL;
	ShowWarning(m);
	if (m) mir_free(m);
}
void ShowErrorUtf(char* msg) {
	TCHAR* m = (msg) ? mir_utf8decodeT(msg) : NULL;
	ShowError(m);
	if (m) mir_free(m);
}

void ShowMessageInline(const MCONTACT hContact, const TCHAR *msg) {
	TCHAR buff[1024];
	mir_sntprintf(buff, SIZEOF(buff), _T("%s%s"), TranslateT(LANG_INLINE_PREFIX), msg);

	PROTORECVEVENT pre = {0};
	pre.timestamp = time(0);
	char *utf = mir_utf8encodeT(buff);
	pre.szMessage = utf;
	pre.flags = PREF_UTF|PREF_BYPASS_OTR;
	ProtoChainRecvMsg(hContact, &pre);	

	mir_free(utf);
}

void ShowMessageInlineUtf(const MCONTACT hContact, const char *msg) {
	char buff[1024];
	mir_snprintf(buff, SIZEOF(buff), "%s%s", Translate(LANG_INLINE_PREFIX), msg);

	PROTORECVEVENT pre = {0};
	pre.timestamp = time(0);
	pre.szMessage = buff;
	pre.flags = PREF_UTF|PREF_BYPASS_OTR;
	ProtoChainRecvMsg(hContact, &pre);
}

void ShowMessageUtf(const MCONTACT hContact, const char *msg) {
	if(options.msg_inline)
		ShowMessageInlineUtf(hContact, msg);
	if(options.msg_popup)
		ShowPopupUtf(Translate(LANG_OTR_INFO), msg, 0, hContact);
}

void ShowMessage(const MCONTACT hContact, const TCHAR *msg) {
	if(options.msg_inline)
		ShowMessageInline(hContact, msg);
	if(options.msg_popup)
		ShowPopup(TranslateT(LANG_OTR_INFO), msg, 0, hContact);
}

const TCHAR *policy_to_string(OtrlPolicy policy) {
	switch (policy) {
		case OTRL_POLICY_NEVER:
			return TranslateT(LANG_POLICY_NEVER);
			break;
		case OTRL_POLICY_OPPORTUNISTIC:
			return TranslateT(LANG_POLICY_OPP);
			break;
		case OTRL_POLICY_MANUAL:
		case OTRL_POLICY_MANUAL_MOD:
			return TranslateT(LANG_POLICY_MANUAL);
			break;
		case OTRL_POLICY_ALWAYS:
			return TranslateT(LANG_POLICY_ALWAYS);
			break;
	}
	return TranslateT(LANG_POLICY_DEFAULT);
}

OtrlPolicy policy_from_string(const TCHAR *polstring) {
	if (_tcscmp(polstring, TranslateT(LANG_POLICY_NEVER)) == 0)
		return OTRL_POLICY_NEVER;
	else if (_tcscmp(polstring, TranslateT(LANG_POLICY_OPP)) == 0)
		return OTRL_POLICY_OPPORTUNISTIC;
	else if (_tcscmp(polstring, TranslateT(LANG_POLICY_MANUAL)) == 0)
		return OTRL_POLICY_MANUAL_MOD;
	else if (_tcscmp(polstring, TranslateT(LANG_POLICY_ALWAYS)) == 0)
		return OTRL_POLICY_ALWAYS;
	else 
		return CONTACT_DEFAULT_POLICY;
}

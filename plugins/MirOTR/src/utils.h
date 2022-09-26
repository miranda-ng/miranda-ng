#pragma once
#ifndef __HEADER_UTILS_H
#define __HEADER_UTILS_H

extern mir_cs lib_cs;

MCONTACT find_contact(const char* userid, const char* protocol);
ConnContext * otrl_context_find_miranda(OtrlUserState us, MCONTACT hContact);

typedef enum {
	TRUST_NOT_PRIVATE,
	TRUST_FINISHED,
	TRUST_UNVERIFIED,
	TRUST_PRIVATE
} TrustLevel;
TrustLevel otr_context_get_trust(ConnContext *context);
void VerifyFingerprint(ConnContext *context, bool verify);
void VerifyFingerprintMessage(ConnContext *context, bool verify);

void otrl_privkey_hash_to_humanT(wchar_t human[45], const unsigned char hash[20]);

char* contact_get_id(MCONTACT hContact);
extern __inline const wchar_t* contact_get_nameT(MCONTACT hContact);

wchar_t* ProtoGetNickname(const char* proto);

char* GetDlgItemTextUtf(HWND hwndDlg, int ctrlId);

void ShowPopup(const wchar_t* line1, const wchar_t* line2, int timeout, const MCONTACT hContact = NULL);
void ShowWarning(wchar_t* msg);
void ShowError(wchar_t* msg);
void ShowPopupUtf(const char* line1, const char* line2, int timeout, const MCONTACT hContact = NULL);
void ShowWarningUtf(char* msg);
void ShowErrorUtf(char* msg);

void ShowMessageInline(const MCONTACT hContact, const wchar_t *msg);
void ShowMessageInlineUtf(const MCONTACT hContact, const char *msg);

void ShowMessage(const MCONTACT hContact, const wchar_t *msg);
void ShowMessageUtf(const MCONTACT hContact, const char *msg);

const wchar_t *policy_to_string(OtrlPolicy policy);
OtrlPolicy policy_from_string(const wchar_t *polstring);

#endif

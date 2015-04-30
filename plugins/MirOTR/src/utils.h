#pragma once
#ifndef __HEADER_UTILS_H
#define __HEADER_UTILS_H
#include "stdafx.h"


void lib_cs_lock();

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

void otrl_privkey_hash_to_humanT(TCHAR human[45], const unsigned char hash[20]);

char* contact_get_id(MCONTACT hContact, bool bNameOnError=true);
__inline const char* contact_get_account(MCONTACT hContact);
extern __inline const TCHAR* contact_get_nameT(MCONTACT hContact);

TCHAR* ProtoGetNickname(const char* proto);

void ShowPopup(const TCHAR* line1, const TCHAR* line2, int timeout, const MCONTACT hContact = NULL);
void ShowWarning(TCHAR* msg);
void ShowError(TCHAR* msg);
void ShowPopupUtf(const char* line1, const char* line2, int timeout, const MCONTACT hContact = NULL);
void ShowWarningUtf(char* msg);
void ShowErrorUtf(char* msg);

void ShowMessageInline(const MCONTACT hContact, const TCHAR *msg);
void ShowMessageInlineUtf(const MCONTACT hContact, const char *msg);

void ShowMessage(const MCONTACT hContact, const TCHAR *msg);
void ShowMessageUtf(const MCONTACT hContact, const char *msg);

const TCHAR *policy_to_string(OtrlPolicy policy);
OtrlPolicy policy_from_string(const TCHAR *polstring);

#endif

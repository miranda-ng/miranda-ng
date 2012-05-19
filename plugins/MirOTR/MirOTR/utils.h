#pragma once
#ifndef __HEADER_UTILS_H
#define __HEADER_UTILS_H
#include "stdafx.h"


void lib_cs_lock();
void lib_cs_unlock();

HANDLE find_contact(const char* userid, const char* protocol);
ConnContext * otrl_context_find_miranda(OtrlUserState us, HANDLE hContact);

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

char* contact_get_id(HANDLE hContact, bool bNameOnError=true);
__inline const char* contact_get_proto(HANDLE hContact);
__inline const char* contact_get_account(HANDLE hContact);
extern __inline const TCHAR* contact_get_nameT(HANDLE hContact);

void ShowPopup(const TCHAR* line1, const TCHAR* line2, int timeout, const HANDLE hContact = NULL);
void ShowWarning(TCHAR* msg);
void ShowError(TCHAR* msg);
void ShowPopupUtf(const char* line1, const char* line2, int timeout, const HANDLE hContact = NULL);
void ShowWarningUtf(char* msg);
void ShowErrorUtf(char* msg);

void ShowMessageInline(const HANDLE hContact, const TCHAR *msg);
void ShowMessageInlineUtf(const HANDLE hContact, const char *msg);

void ShowMessage(const HANDLE hContact, const TCHAR *msg);
void ShowMessageUtf(const HANDLE hContact, const char *msg);

void InitUtils();
void DeinitUtils();

const TCHAR *policy_to_string(OtrlPolicy policy);
OtrlPolicy policy_from_string(const TCHAR *polstring);

#endif
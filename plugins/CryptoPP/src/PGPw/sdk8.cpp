#include "../commonheaders.h"

#define PGP_WIN32 0x800

#include "sdk8/include/pgpEncode.h"
#include "sdk8/include/pgpErrors.h"
#include "sdk8/include/pgpKeys.h"
#include "sdk8/include/pgpOptionList.h"
#include "sdk8/include/pgpUserInterface.h"
#include "sdk8/include/pgpUtilities.h"
#include "sdk8/include/pgpFeatures.h"

#include "pgpw.h"

PGPContextRef pgpContext;
PGPKeyDBRef pgpKeyDB = 0;
LPSTR pszPassphrase = 0;
LPSTR pgpErrMsg = 0;
#define pgpErrMsgLen 512
extern UINT pgpVer;

#if !defined(_WIN64)

#pragma comment(lib, "pgpsdk.lib")
#pragma comment(lib, "pgpsdknl.lib")
#pragma comment(lib, "pgpsdkui.lib")

void ClearPGPError()
{
	*pgpErrMsg = '\0';
}


bool CheckPGPError(PGPError err)
{
	if (IsPGPError(err)) {
		PGPSize ErrMsgLen = pgpErrMsgLen;
		PGPGetErrorString(err, ErrMsgLen, pgpErrMsg);
		return 1;
	}
	else {
		*pgpErrMsg = '\0';
		return 0;
	}
}

#endif

#define _pgp_memcpy memmove

int __cdecl _pgp_init()
{
	if (!pgpErrMsg)
		pgpErrMsg = (LPSTR) LocalAlloc(LPTR,pgpErrMsgLen);

	pgpVer = 0;
	#if defined(_WIN64)
		return 0;
	#else
		__try {
			if (CheckPGPError(PGPsdkInit(0)) || CheckPGPError(PGPsdkUILibInit(0))) return 0;
			PGPNewContext(kPGPsdk20APIVersion, &pgpContext);
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
			return 0;
		}

		pgpVer = PGPGetPGPsdkVersion();
		return 1;
	#endif
}


int __cdecl _pgp_done()
{
	pgpVer = 0;
	#if defined(_WIN64)
		return 0;
	#else
		__try {
			if(pgpErrMsg) LocalFree(pgpErrMsg);
			if (pszPassphrase) PGPFreeData(pszPassphrase);
			if (pgpKeyDB) PGPFreeKeyDB(pgpKeyDB);
			PGPFreeContext(pgpContext);
			PGPsdkUILibCleanup();
			PGPsdkCleanup();
			pszPassphrase = pgpErrMsg = 0;
			pgpKeyDB = 0;
			pgpContext = 0;
		}
		__except ( EXCEPTION_EXECUTE_HANDLER ) {
			return 0;
		}
		return 1;
	#endif
}


int __cdecl _pgp_open_keyrings(LPSTR PubRingPath, LPSTR SecRingPath)
{
	#if defined(_WIN64)
		return 0;
	#else
		_pgp_close_keyrings();

		if ((!PubRingPath || !*PubRingPath) && !ShowSelectKeyringsDlg(0,PubRingPath,SecRingPath))
			return 0;

		PGPFileSpecRef PubKeyRing, SecKeyRing;
		PGPNewFileSpecFromFullPath(pgpContext, PubRingPath, &PubKeyRing);
		PGPNewFileSpecFromFullPath(pgpContext, SecRingPath, &SecKeyRing);

		PGPError err = PGPOpenKeyDBFile(pgpContext, kPGPOpenKeyDBFileOptions_None, PubKeyRing, SecKeyRing, &pgpKeyDB);
		PGPFreeFileSpec(SecKeyRing);
		PGPFreeFileSpec(PubKeyRing);

		if (CheckPGPError(err))
			return 0;

		return 1;
	#endif
}


int __cdecl _pgp_close_keyrings()
{
	#if !defined(_WIN64)
		if (pgpKeyDB) {
			PGPFreeKeyDB(pgpKeyDB);
			pgpKeyDB = 0;
		}
	#endif
	return 1;
}


int __cdecl _pgp_get_version()
{
	return pgpVer;
}


LPSTR __cdecl _pgp_get_error()
{
	return pgpErrMsg;
}


LPSTR __cdecl _pgp_encrypt_keydb(LPCSTR szPlainMsg, PVOID pgpKeyID)
{
	#if defined(_WIN64)
		return 0;
	#else
		PGPKeyID *RemoteKeyID = (PGPKeyID *) pgpKeyID;
		LPSTR szEncMsg = 0;
		PGPSize dwEncMsgLen;

		ClearPGPError();
		if (!pgpKeyDB)
			return 0;

		PGPKeyDBObjRef PublicKey;
		PGPFindKeyByKeyID(pgpKeyDB, RemoteKeyID, &PublicKey);

		PGPError err = PGPEncode(pgpContext,
			PGPOInputBuffer(pgpContext, szPlainMsg, mir_tstrlen(szPlainMsg)),
			PGPOArmorOutput(pgpContext, TRUE),
			PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szEncMsg, 16384, &dwEncMsgLen),
			PGPOEncryptToKeyDBObj(pgpContext, PublicKey),
			PGPOVersionString(pgpContext, szVersionStr),
			PGPOLastOption(pgpContext));

		if (CheckPGPError(err))
			return 0;

		LPSTR szMsg = (LPSTR) LocalAlloc(LPTR,dwEncMsgLen+1);
		_pgp_memcpy(szMsg, szEncMsg, dwEncMsgLen);
		szMsg[dwEncMsgLen] = 0;
		PGPFreeData((LPVOID)szEncMsg);

		return szMsg;
	#endif
}

LPSTR __cdecl _pgp_decrypt_keydb(LPCSTR szEncMsg)
{
	#if defined(_WIN64)
		return 0;
	#else
		LPSTR szPlainMsg = 0;
		PGPSize dwPlainMsgLen;

		ClearPGPError();
		if (!pgpKeyDB)
			return 0;

		int iTry = 0;

		do {
			if (!pszPassphrase &&
				PGPPassphraseDialog(pgpContext,
				PGPOUIOutputPassphrase(pgpContext, &pszPassphrase),
				PGPOLastOption(pgpContext)) == kPGPError_UserAbort) {
					iTry = 3;
					break;
			}

			PGPError err = PGPDecode(pgpContext,
				PGPOInputBuffer(pgpContext, szEncMsg, mir_tstrlen(szEncMsg)),
				PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szPlainMsg, 16384, &dwPlainMsgLen),
				PGPOKeyDBRef(pgpContext, pgpKeyDB),
				PGPOPassphrase(pgpContext, pszPassphrase),
				PGPOLastOption(pgpContext));

			if (CheckPGPError(err))
				iTry = 3;
			else
				if (!dwPlainMsgLen) {
					PGPFreeData(pszPassphrase);
					pszPassphrase = 0;
					iTry++;
				}

		} while(!dwPlainMsgLen && iTry<3);

		if(iTry == 3) return 0;

		LPSTR szMsg = (LPSTR) LocalAlloc(LPTR,dwPlainMsgLen+1);
		_pgp_memcpy(szMsg, szPlainMsg, dwPlainMsgLen);
		szMsg[dwPlainMsgLen] = 0;
		PGPFreeData((LPVOID)szPlainMsg);

		return szMsg;
	#endif
}


PGPError _pgp_import_key(PGPKeyDBRef *keyDB, LPCSTR pgpKey)
{
	#if defined(_WIN64)
		return 0;
	#else
		return PGPImport( pgpContext,
			keyDB,
			PGPOInputBuffer( pgpContext,
			pgpKey,
			mir_tstrlen(pgpKey) ),
			PGPOLastOption(pgpContext ));
	#endif
}


LPSTR __cdecl _pgp_encrypt_key(LPCSTR szPlainMsg, LPCSTR pgpKey)
{
	#if defined(_WIN64)
		return 0;
	#else
		LPSTR szEncMsg = 0;
		PGPSize dwEncMsgLen;

		PGPUInt32 dwKeys;
		PGPKeyDBRef PublicKeyDB;
		if (CheckPGPError(_pgp_import_key(&PublicKeyDB,pgpKey)))
			return 0;

		PGPKeyIterRef KeyIterRef;
		PGPNewKeyIterFromKeyDB(PublicKeyDB, &KeyIterRef);

		PGPKeyDBObjRef PublicKey;
		PGPKeyIterNextKeyDBObj(KeyIterRef, kPGPKeyDBObjType_Key, &PublicKey);

		PGPCountKeysInKeyDB(PublicKeyDB, &dwKeys);

		if(dwKeys==0) {
			PGPFreeKeyIter(KeyIterRef);
			PGPFreeKeyDB(PublicKeyDB);
			return 0;
		}

		PGPError err = PGPEncode(pgpContext,
			PGPOInputBuffer(pgpContext, szPlainMsg, mir_tstrlen(szPlainMsg)),
			PGPOArmorOutput(pgpContext, TRUE),
			PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szEncMsg, 16384, &dwEncMsgLen),
			PGPOEncryptToKeyDBObj(pgpContext, PublicKey),
			PGPOVersionString(pgpContext, szVersionStr),
			PGPOLastOption(pgpContext));

		PGPFreeKeyIter(KeyIterRef);
		PGPFreeKeyDB(PublicKeyDB);

		if (CheckPGPError(err))
			return 0;

		LPSTR szMsg = (LPSTR) LocalAlloc(LPTR,dwEncMsgLen+1);
		_pgp_memcpy(szMsg, szEncMsg, dwEncMsgLen);
		szMsg[dwEncMsgLen] = 0;
		PGPFreeData((LPVOID)szEncMsg);

		return szMsg;
	#endif
}


LPSTR __cdecl _pgp_decrypt_key(LPCSTR szEncMsg, LPCSTR pgpKey)
{
	#if defined(_WIN64)
		return 0;
	#else
		LPSTR szPlainMsg = 0;
		PGPSize dwPlainMsgLen;

		PGPUInt32 dwKeys;
		PGPKeyDBRef PrivateKeyDB;
		if (CheckPGPError(_pgp_import_key(&PrivateKeyDB,pgpKey)))
			return 0;
		PGPCountKeysInKeyDB(PrivateKeyDB, &dwKeys);

		if(dwKeys==0) {
			PGPFreeKeyDB(PrivateKeyDB);
			return 0;
		}

		int iTry = 0;

		do {
			if (!pszPassphrase &&
				PGPPassphraseDialog(pgpContext,
				PGPOUIOutputPassphrase(pgpContext, &pszPassphrase),
				PGPOLastOption(pgpContext)) == kPGPError_UserAbort) {
					iTry = 3;
					break;
			}

			PGPError err = PGPDecode(pgpContext,
				PGPOInputBuffer(pgpContext, szEncMsg, mir_tstrlen(szEncMsg)),
				PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szPlainMsg, 16384, &dwPlainMsgLen),
				PGPOKeyDBRef(pgpContext, PrivateKeyDB),
				PGPOPassphrase(pgpContext, pszPassphrase),
				PGPOLastOption(pgpContext));

			if (CheckPGPError(err))
				iTry = 3;
			else
				if (!dwPlainMsgLen) {
					PGPFreeData(pszPassphrase);
					pszPassphrase = 0;
					iTry++;
				}

		} while(!dwPlainMsgLen && iTry<3);

		PGPFreeKeyDB(PrivateKeyDB);

		if(iTry == 3) return 0;

		LPSTR szMsg = (LPSTR) LocalAlloc(LPTR,dwPlainMsgLen+1);
		_pgp_memcpy(szMsg, szPlainMsg, dwPlainMsgLen);
		szMsg[dwPlainMsgLen] = 0;
		PGPFreeData((LPVOID)szPlainMsg);

		return szMsg;
	#endif
}

int __cdecl _pgp_size_keyid()
{
	return sizeof(PGPKeyID);
}


PVOID __cdecl _pgp_select_keyid(HWND hDlg,LPSTR szKeyID)
{
	#if defined(_WIN64)
		return 0;
	#else
		PGPKeyDBRef ContactKeyDB;
		PGPError err;
		err = PGPRecipientDialog(pgpContext, pgpKeyDB, TRUE, &ContactKeyDB,
			PGPOUIParentWindowHandle(pgpContext, hDlg),
			PGPOUIWindowTitle(pgpContext, "Select Contact's Key"),
			PGPOLastOption(pgpContext));
		if (err == kPGPError_UserAbort)
			return 0;

		PGPUInt32 dwKeys;
		PGPCountKeysInKeyDB(ContactKeyDB, &dwKeys);
		if (!dwKeys)
			return 0;
		if (dwKeys > 1)
			MessageBox(hDlg, "You selected more than one key. Only the first key will be used.", szModuleName, MB_ICONINFORMATION);

		static PGPKeyID KeyID;

		PGPKeyIterRef KeyIterRef;
		PGPNewKeyIterFromKeyDB(ContactKeyDB, &KeyIterRef);

		PGPKeyDBObjRef KeyDBObjRef;
		PGPKeyIterNextKeyDBObj(KeyIterRef, kPGPKeyDBObjType_Key, &KeyDBObjRef);

		PGPSize dwFilled;
		PGPGetKeyDBObjDataProperty(KeyDBObjRef, kPGPKeyProperty_KeyID, &KeyID, sizeof(PGPKeyID), &dwFilled);
		PGPGetKeyIDString(&KeyID, kPGPKeyIDString_Abbreviated, szKeyID);

		PGPFreeKeyIter(KeyIterRef);
		PGPFreeKeyDB(ContactKeyDB);
		return (PVOID)&KeyID;
	#endif
}


BOOL ShowSelectKeyringsDlg(HWND hParent, LPSTR PubRingPath, LPSTR SecRingPath)
{
	// set keyring paths
	OPENFILENAME ofn={0};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hParent;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;

	ofn.lpstrFile = PubRingPath;
	ofn.lpstrFilter = "Public key rings (pubring.pkr)\0pubring.pkr\0All files (*.*)\0*.*\0";
	ofn.lpstrTitle = "Open Public Keyring";
	if (!GetOpenFileName(&ofn)) return FALSE;

	ofn.lpstrFile = SecRingPath;
	ofn.lpstrFilter = "Secret key rings (secring.skr)\0secring.skr\0All files (*.*)\0*.*\0";
	ofn.lpstrTitle = "Open Secret Keyring";
	if (!GetOpenFileName(&ofn)) return FALSE;

	return TRUE;
}

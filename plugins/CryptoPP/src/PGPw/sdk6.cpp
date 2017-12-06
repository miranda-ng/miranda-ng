#include "../commonheaders.h"

#define PGP_WIN32 0x600

#include "sdk6/include/pgpEncode.h"
#include "sdk6/include/pgpErrors.h"
#include "sdk6/include/pgpKeys.h"
#include "sdk6/include/pgpOptionList.h"
#include "sdk6/include/pgpUserInterface.h"
#include "sdk6/include/pgpUtilities.h"
#include "sdk6/include/pgpFeatures.h"

#include "pgpw.h"

// pgpsdk.lib pgpsdknl.lib pgpsdkui.lib libcmt.lib
// /delayload:pgpsdk.dll,pgpsdknl.dll,pgpsdkui.dll

PGPContextRef pgpContext;
PGPKeySetRef pgpKeyDB = 0;
LPSTR pszPassphrase = 0;
LPSTR pgpErrMsg = 0;
#define pgpErrMsgLen 512
UINT pgpVer;


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

#define _pgp_memcpy memmove
/*
void _pgp_memcpy(LPSTR dst, LPSTR src, UINT size)
{
	for(UINT i=0;i<size;i++)
		dst[i]=src[i];
}
*/

int __cdecl _pgp_init()
{
	if (!pgpErrMsg)
		pgpErrMsg = (LPSTR) LocalAlloc(LPTR,pgpErrMsgLen);

	pgpVer = 0;

 	__try {
		if (CheckPGPError(PGPsdkInit()) || CheckPGPError(PGPsdkUILibInit()))  return 0;
		PGPNewContext(kPGPsdkAPIVersion, &pgpContext);
	}
	__except ( EXCEPTION_EXECUTE_HANDLER ) {
		return 0;
	}

	PGPGetSDKVersion(&pgpVer);
	int minor = (pgpVer & 0x000FF000 ) >> 12;
	switch(PGPMajorVersion(pgpVer)) {
	case 2:
		pgpVer = 1<<24 | ((minor+1)<<16) | (PGPRevVersion(pgpVer)<<8);
		break;
	case 3:
		pgpVer = 1<<24 | ((minor+5)<<16) | (PGPRevVersion(pgpVer)<<8);
		break;
	default:
		pgpVer = 1<<24 | (minor<<16) | (PGPRevVersion(pgpVer)<<8);
		break;
	}

	return 1;
}


int __cdecl _pgp_done()
{
    pgpVer = 0;
	__try {
	    if(pgpErrMsg) LocalFree(pgpErrMsg);
		if (pszPassphrase) PGPFreeData(pszPassphrase);
		if (pgpKeyDB) PGPFreeKeySet(pgpKeyDB);
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
}


int __cdecl _pgp_open_keyrings(LPSTR PubRingPath, LPSTR SecRingPath)
{
	_pgp_close_keyrings();

    PGPError err = PGPOpenDefaultKeyRings(pgpContext, kPGPKeyRingOpenFlags_None, &pgpKeyDB);
	if (CheckPGPError(err)) {
		return 0;
	}
	return 1;
}


int __cdecl _pgp_close_keyrings()
{
	if (pgpKeyDB) {
		PGPFreeKeySet(pgpKeyDB);
		pgpKeyDB = 0;
	}
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
   	PGPKeyID *RemoteKeyID = (PGPKeyID *) pgpKeyID;
    LPSTR szEncMsg = 0;
    DWORD dwEncMsgLen;

	ClearPGPError();
	if (!pgpKeyDB)
		return 0;

    PGPFilterRef IDFilter;
    PGPNewKeyIDFilter(pgpContext, RemoteKeyID, &IDFilter);

    PGPKeySetRef PublicKey;
    PGPFilterKeySet(pgpKeyDB, IDFilter, &PublicKey);

    PGPError err = PGPEncode(pgpContext,
      PGPOInputBuffer(pgpContext, szPlainMsg, lstrlen(szPlainMsg)),
      PGPOArmorOutput(pgpContext, TRUE),
      PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szEncMsg, 16384, (PGPUInt32 *)&dwEncMsgLen),
      PGPOEncryptToKeySet(pgpContext, PublicKey),
      PGPOVersionString(pgpContext, szVersionStr),
      PGPOLastOption(pgpContext));

	PGPFreeKeySet(PublicKey);
	PGPFreeFilter(IDFilter);

    if (CheckPGPError(err))
       return 0;

    LPSTR szMsg = (LPSTR) LocalAlloc(LPTR,dwEncMsgLen+1);
    _pgp_memcpy(szMsg, szEncMsg, dwEncMsgLen);
    szMsg[dwEncMsgLen] = 0;
    PGPFreeData((LPVOID)szEncMsg);

    return szMsg;
}


LPSTR __cdecl _pgp_decrypt_keydb(LPCSTR szEncMsg)
{
    LPSTR szPlainMsg = 0;
    DWORD dwPlainMsgLen;

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
	       PGPOInputBuffer(pgpContext, szEncMsg, lstrlen(szEncMsg)),
	       PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szPlainMsg, 16384, (PGPUInt32 *)&dwPlainMsgLen),
		   PGPOKeySetRef(pgpContext, pgpKeyDB),
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
}


PGPError _pgp_import_key(PGPKeySetRef *keySet, LPCSTR pgpKey)
{
    return PGPImportKeySet( pgpContext,
                           keySet,
                           PGPOInputBuffer( pgpContext,
                                            pgpKey,
                                            lstrlen(pgpKey) ),
                           PGPOLastOption( pgpContext ) );
}

LPSTR __cdecl _pgp_encrypt_key(LPCSTR szPlainMsg, LPCSTR pgpKey)
{
    LPSTR szEncMsg = 0;
    DWORD dwEncMsgLen;

    PGPUInt32 dwKeys;
	PGPKeySetRef PublicKey;
    if (CheckPGPError(_pgp_import_key(&PublicKey,pgpKey)))
       return 0;
	PGPCountKeys(PublicKey, &dwKeys);
	if(dwKeys==0) {
		PGPFreeKeySet(PublicKey);
		return 0;
	}

    PGPError err = PGPEncode(pgpContext,
      PGPOInputBuffer(pgpContext, szPlainMsg, lstrlen(szPlainMsg)),
      PGPOArmorOutput(pgpContext, TRUE),
      PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szEncMsg, 16384, (PGPUInt32 *)&dwEncMsgLen),
      PGPOEncryptToKeySet(pgpContext, PublicKey),
      PGPOVersionString(pgpContext, szVersionStr),
      PGPOLastOption(pgpContext));

	PGPFreeKeySet(PublicKey);
    if (CheckPGPError(err))
       return 0;

    LPSTR szMsg = (LPSTR) LocalAlloc(LPTR,dwEncMsgLen+1);
    _pgp_memcpy(szMsg, szEncMsg, dwEncMsgLen);
    szMsg[dwEncMsgLen] = 0;
    PGPFreeData((LPVOID)szEncMsg);

    return szMsg;
}


LPSTR __cdecl _pgp_decrypt_key(LPCSTR szEncMsg, LPCSTR pgpKey)
{
    LPSTR szPlainMsg = 0;
    DWORD dwPlainMsgLen;

    PGPUInt32 dwKeys;
	PGPKeySetRef PrivateKeyDB;
    if (CheckPGPError(_pgp_import_key(&PrivateKeyDB,pgpKey)))
       return 0;
	PGPCountKeys(PrivateKeyDB, &dwKeys);
	if(dwKeys==0) {
		PGPFreeKeySet(PrivateKeyDB);
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
	       PGPOInputBuffer(pgpContext, szEncMsg, lstrlen(szEncMsg)),
	       PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szPlainMsg, 16384, (PGPUInt32 *)&dwPlainMsgLen),
		   PGPOKeySetRef(pgpContext, PrivateKeyDB),
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

	PGPFreeKeySet(PrivateKeyDB);

	if(iTry == 3) return 0;

    LPSTR szMsg = (LPSTR) LocalAlloc(LPTR,dwPlainMsgLen+1);
    _pgp_memcpy(szMsg, szPlainMsg, dwPlainMsgLen);
    szMsg[dwPlainMsgLen] = 0;
    PGPFreeData((LPVOID)szPlainMsg);

    return szMsg;
}


int __cdecl _pgp_size_keyid()
{
	return sizeof(PGPKeyID);
}


PVOID __cdecl _pgp_select_keyid(HWND hDlg,LPSTR szKeyID)
{
	PGPKeySetRef ContactKeyDB;
    PGPError err;
    err = PGPRecipientDialog(pgpContext, pgpKeyDB, TRUE, &ContactKeyDB,
          PGPOUIParentWindowHandle(pgpContext, hDlg),
          PGPOUIWindowTitle(pgpContext, "Select Contact's Key"),
          PGPOLastOption(pgpContext));
    if (err == kPGPError_UserAbort)
       return 0;

    PGPUInt32 dwKeys;
	PGPCountKeys(ContactKeyDB, &dwKeys);
    if (!dwKeys) {
       return 0;
    } else if (dwKeys > 1)
       MessageBox(hDlg, "You selected more than one key. Only the first key will be used.", szModuleName, MB_ICONINFORMATION);

	static PGPKeyID KeyID;

    PGPKeyListRef ContactKeyList;
    PGPOrderKeySet(ContactKeyDB, kPGPKeyIDOrdering, &ContactKeyList);

    PGPKeyIterRef KeyIterRef;
    PGPNewKeyIter(ContactKeyList, &KeyIterRef);

    PGPKeyRef ContactKey;
    PGPKeyIterNext(KeyIterRef, &ContactKey);

    PGPGetKeyIDFromKey(ContactKey, &KeyID);
	PGPGetKeyIDString(&KeyID, kPGPKeyIDString_Abbreviated, szKeyID);

    PGPFreeKeyList(ContactKeyList);
    PGPFreeKeyIter(KeyIterRef);
    PGPFreeKeySet(ContactKeyDB);
    return (PVOID)&KeyID;
}

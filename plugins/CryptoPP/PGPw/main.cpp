#include "commonheaders.h"

// pgpsdk.lib pgpsdknl.lib pgpsdkui.lib libcmt.lib
// /delayload:pgpsdk.dll,pgpsdknl.dll,pgpsdkui.dll

PGPContextRef pgpContext;
#if (PGP_WIN32 < 0x700)
PGPKeySetRef pgpKeyDB = 0;
#else
PGPKeyDBRef pgpKeyDB = 0;
#endif
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
	if(!pgpErrMsg)
		pgpErrMsg = (LPSTR) LocalAlloc(LPTR,pgpErrMsgLen);

	pgpVer = 0;

 	__try {
#if (PGP_WIN32 < 0x700)
		if (CheckPGPError(PGPsdkInit()) || CheckPGPError(PGPsdkUILibInit()))  return 0;
		PGPNewContext(kPGPsdkAPIVersion, &pgpContext);
#else
		if (CheckPGPError(PGPsdkInit(0)) || CheckPGPError(PGPsdkUILibInit(0))) return 0;
		PGPNewContext(kPGPsdk20APIVersion, &pgpContext);
#endif
	}
	__except ( EXCEPTION_EXECUTE_HANDLER ) {
		return 0;
	}

#if (PGP_WIN32 < 0x700)
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
#else
	pgpVer = PGPGetPGPsdkVersion();
#endif

	return 1;
}


int __cdecl _pgp_done()
{
    pgpVer = 0;
	__try {
	    if(pgpErrMsg) LocalFree(pgpErrMsg);
		if (pszPassphrase) PGPFreeData(pszPassphrase);
#if (PGP_WIN32 < 0x700)
		if (pgpKeyDB) PGPFreeKeySet(pgpKeyDB);
#else
		if (pgpKeyDB) PGPFreeKeyDB(pgpKeyDB);
#endif
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

#if (PGP_WIN32 < 0x700)
    PGPError err = PGPOpenDefaultKeyRings(pgpContext, kPGPKeyRingOpenFlags_None, &pgpKeyDB);
#else
    if ((!PubRingPath || !*PubRingPath) && !ShowSelectKeyringsDlg(0,PubRingPath,SecRingPath)) {
		return 0;
    }

    PGPFileSpecRef PubKeyRing, SecKeyRing;
    PGPNewFileSpecFromFullPath(pgpContext, PubRingPath, &PubKeyRing);
    PGPNewFileSpecFromFullPath(pgpContext, SecRingPath, &SecKeyRing);

    PGPError err = PGPOpenKeyDBFile(pgpContext, kPGPOpenKeyDBFileOptions_None, PubKeyRing, SecKeyRing, &pgpKeyDB);
    PGPFreeFileSpec(SecKeyRing);
    PGPFreeFileSpec(PubKeyRing);
#endif
	if (CheckPGPError(err)) {
		return 0;
	}
	return 1;
}


int __cdecl _pgp_close_keyrings()
{
#if (PGP_WIN32 < 0x700)
	if (pgpKeyDB) {
		PGPFreeKeySet(pgpKeyDB);
		pgpKeyDB = 0;
	}
#else
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
   	PGPKeyID *RemoteKeyID = (PGPKeyID *) pgpKeyID;
    LPSTR szEncMsg = 0;
    DWORD dwEncMsgLen;

	ClearPGPError();
	if(!pgpKeyDB)
		return 0;

#if (PGP_WIN32 < 0x700)
    PGPFilterRef IDFilter;
    PGPNewKeyIDFilter(pgpContext, RemoteKeyID, &IDFilter);

    PGPKeySetRef PublicKey;
    PGPFilterKeySet(pgpKeyDB, IDFilter, &PublicKey);
#else
    PGPKeyDBObjRef PublicKey;
    PGPFindKeyByKeyID(pgpKeyDB, RemoteKeyID, &PublicKey);
#endif

    PGPError err = PGPEncode(pgpContext,
      PGPOInputBuffer(pgpContext, szPlainMsg, lstrlen(szPlainMsg)),
      PGPOArmorOutput(pgpContext, TRUE),
      PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szEncMsg, 16384, (PGPUInt32 *)&dwEncMsgLen),
#if (PGP_WIN32 < 0x700)
      PGPOEncryptToKeySet(pgpContext, PublicKey),
#else
      PGPOEncryptToKeyDBObj(pgpContext, PublicKey),
#endif
      PGPOVersionString(pgpContext, szVersionStr),
      PGPOLastOption(pgpContext));

#if (PGP_WIN32 < 0x700)
	PGPFreeKeySet(PublicKey);
	PGPFreeFilter(IDFilter);
#endif

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
	if(!pgpKeyDB)
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
#if (PGP_WIN32 < 0x700)
		   PGPOKeySetRef(pgpContext, pgpKeyDB),
#else
	       PGPOKeyDBRef(pgpContext, pgpKeyDB),
#endif
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


#if (PGP_WIN32 < 0x700)
PGPError _pgp_import_key(PGPKeySetRef *keySet, LPCSTR pgpKey)
{
    return PGPImportKeySet( pgpContext,
                           keySet,
                           PGPOInputBuffer( pgpContext,
                                            pgpKey,
                                            lstrlen(pgpKey) ),
                           PGPOLastOption( pgpContext ) );
}
#else
PGPError _pgp_import_key(PGPKeyDBRef *keyDB, LPCSTR pgpKey)
{
    return PGPImport( pgpContext,
                     keyDB,
                     PGPOInputBuffer( pgpContext,
                                      pgpKey,
                                      lstrlen(pgpKey) ),
                     PGPOLastOption( pgpContext ) );
}
#endif

/*
int __cdecl _pgp_check_key(LPCSTR pgpKey)
{
#if (PGP_WIN32 < 0x700)
	PGPKeySetRef PrivateKey;
    if (CheckPGPError(_pgp_import_key(&PrivateKey,pgpKey)))
       return 0;
	PGPFreeKeySet(PrivateKey);
#else
	PGPKeyDBRef PrivateKey;
    if (CheckPGPError(_pgp_import_key(&PrivateKey,pgpKey)))
       return 0;
	PGPFreeKeyDB(PrivateKey);
#endif
	return 1;
}
*/

LPSTR __cdecl _pgp_encrypt_key(LPCSTR szPlainMsg, LPCSTR pgpKey)
{
    LPSTR szEncMsg = 0;
    DWORD dwEncMsgLen;

    PGPUInt32 dwKeys;
#if (PGP_WIN32 < 0x700)
	PGPKeySetRef PublicKey;
    if (CheckPGPError(_pgp_import_key(&PublicKey,pgpKey)))
       return 0;
	PGPCountKeys(PublicKey, &dwKeys);
#else
	PGPKeyDBRef PublicKeyDB;
    if (CheckPGPError(_pgp_import_key(&PublicKeyDB,pgpKey)))
       return 0;

    PGPKeyIterRef KeyIterRef;
    PGPNewKeyIterFromKeyDB(PublicKeyDB, &KeyIterRef);

	PGPKeyDBObjRef PublicKey;
	PGPKeyIterNextKeyDBObj(KeyIterRef, kPGPKeyDBObjType_Key, &PublicKey);

	PGPCountKeysInKeyDB(PublicKeyDB, &dwKeys);
#endif
	if(dwKeys==0) {
#if (PGP_WIN32 < 0x700)
		PGPFreeKeySet(PublicKey);
#else
	    PGPFreeKeyIter(KeyIterRef);
		PGPFreeKeyDB(PublicKeyDB);
#endif
		return 0;
	}

    PGPError err = PGPEncode(pgpContext,
      PGPOInputBuffer(pgpContext, szPlainMsg, lstrlen(szPlainMsg)),
      PGPOArmorOutput(pgpContext, TRUE),
      PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szEncMsg, 16384, (PGPUInt32 *)&dwEncMsgLen),
#if (PGP_WIN32 < 0x700)
      PGPOEncryptToKeySet(pgpContext, PublicKey),
#else
      PGPOEncryptToKeyDBObj(pgpContext, PublicKey),
#endif
      PGPOVersionString(pgpContext, szVersionStr),
      PGPOLastOption(pgpContext));

#if (PGP_WIN32 < 0x700)
	PGPFreeKeySet(PublicKey);
#else
    PGPFreeKeyIter(KeyIterRef);
	PGPFreeKeyDB(PublicKeyDB);
#endif

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
#if (PGP_WIN32 < 0x700)
	PGPKeySetRef PrivateKeyDB;
    if (CheckPGPError(_pgp_import_key(&PrivateKeyDB,pgpKey)))
       return 0;
	PGPCountKeys(PrivateKeyDB, &dwKeys);
#else
	PGPKeyDBRef PrivateKeyDB;
    if (CheckPGPError(_pgp_import_key(&PrivateKeyDB,pgpKey)))
       return 0;
	PGPCountKeysInKeyDB(PrivateKeyDB, &dwKeys);
#endif
	if(dwKeys==0) {
#if (PGP_WIN32 < 0x700)
		PGPFreeKeySet(PrivateKeyDB);
#else
		PGPFreeKeyDB(PrivateKeyDB);
#endif
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

/*
#if (PGP_WIN32 < 0x700)
	    PGPKeyListRef PrivateKeyList;
	    PGPOrderKeySet(PrivateKeyDB, kPGPKeyIDOrdering, &PrivateKeyList);

	    PGPKeyIterRef KeyIterRef;
	    PGPNewKeyIter(PrivateKeyList, &KeyIterRef);

		PGPKeyRef PrivateKey;
	    for(int i=0;i<dwKeys;i++) {
			PGPKeyIterNext(KeyIterRef, &PrivateKey);
			PGPOPassphraseIsValid(PrivateKey,
							      PGPOPassphrase(pgpContext, pszPassphrase),
							      PGPOLastOption(pgpContext));
		}

	    PGPFreeKeyList(PrivateKeyList);
	    PGPFreeKeyIter(KeyIterRef);
#else
	    PGPKeyIterRef KeyIterRef;
	    PGPNewKeyIterFromKeyDB(PrivateKeyDB, &KeyIterRef);

		PGPKeyDBObjRef KeyDBObjRef;
	    for(int i=0;i<dwKeys;i++) {
			PGPKeyIterNextKeyDBObj(KeyIterRef, kPGPKeyDBObjType_Key, &KeyDBObjRef);
			PGPOPassphraseIsValid(PrivateKey,
							      PGPOPassphrase(pgpContext, pszPassphrase),
							      PGPOLastOption(pgpContext));
		}

	    PGPFreeKeyIter(KeyIterRef);
#endif
*/
		PGPError err = PGPDecode(pgpContext,
	       PGPOInputBuffer(pgpContext, szEncMsg, lstrlen(szEncMsg)),
	       PGPOAllocatedOutputBuffer(pgpContext, (LPVOID *)&szPlainMsg, 16384, (PGPUInt32 *)&dwPlainMsgLen),
#if (PGP_WIN32 < 0x700)
		   PGPOKeySetRef(pgpContext, PrivateKeyDB),
#else
	       PGPOKeyDBRef(pgpContext, PrivateKeyDB),
#endif
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
    
#if (PGP_WIN32 < 0x700)
	PGPFreeKeySet(PrivateKeyDB);
#else
	PGPFreeKeyDB(PrivateKeyDB);
#endif

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
#if (PGP_WIN32 < 0x700)
	PGPKeySetRef ContactKeyDB;
#else
	PGPKeyDBRef ContactKeyDB;
#endif
    PGPError err;
    err = PGPRecipientDialog(pgpContext, pgpKeyDB, TRUE, &ContactKeyDB,
          PGPOUIParentWindowHandle(pgpContext, hDlg),
          PGPOUIWindowTitle(pgpContext, "Select Contact's Key"),
          PGPOLastOption(pgpContext));
    if (err == kPGPError_UserAbort)
       return 0;

    PGPUInt32 dwKeys;
#if (PGP_WIN32 < 0x700)
	PGPCountKeys(ContactKeyDB, &dwKeys);
#else
	PGPCountKeysInKeyDB(ContactKeyDB, &dwKeys);
#endif
    if (!dwKeys) {
       return 0;
    } else if (dwKeys > 1)
       MessageBox(hDlg, "You selected more than one key. Only the first key will be used.", szModuleName, MB_ICONINFORMATION);

	static PGPKeyID KeyID;

#if (PGP_WIN32 < 0x700)
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
#else
    PGPKeyIterRef KeyIterRef;
    PGPNewKeyIterFromKeyDB(ContactKeyDB, &KeyIterRef);

    PGPKeyDBObjRef KeyDBObjRef;
    PGPKeyIterNextKeyDBObj(KeyIterRef, kPGPKeyDBObjType_Key, &KeyDBObjRef);

    PGPSize dwFilled;
    PGPGetKeyDBObjDataProperty(KeyDBObjRef, kPGPKeyProperty_KeyID, &KeyID, sizeof(PGPKeyID), &dwFilled);
	PGPGetKeyIDString(&KeyID, kPGPKeyIDString_Abbreviated, szKeyID);

    PGPFreeKeyIter(KeyIterRef);
    PGPFreeKeyDB(ContactKeyDB);
#endif

    return (PVOID)&KeyID;
}


#if (PGP_WIN32 >= 0x700)
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
#endif


// dllmain
BOOL WINAPI dllmain(HINSTANCE hInst, DWORD dwReason, LPVOID) {
	g_hInst = hInst;
	return TRUE;
}


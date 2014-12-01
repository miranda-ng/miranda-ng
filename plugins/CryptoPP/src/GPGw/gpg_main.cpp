#include "../commonheaders.h"
#include "gpgw.h"

BOOL ShowSelectExecDlg(LPSTR);
BOOL ShowSelectHomeDlg(LPSTR);

char temporarydirectory[fullfilenamesize];
char logfile[fullfilenamesize];
/*
char *txtbeginpgppublickeyblock="-----BEGIN PGP PUBLIC KEY BLOCK-----";
char *txtendpgppublickeyblock="-----END PGP PUBLIC KEY BLOCK-----";
*/
char *txtbeginpgpmessage="-----BEGIN PGP MESSAGE-----";
char *txtendpgpmessage="-----END PGP MESSAGE-----";

void __cdecl ErrorMessage(const char *alevel, const char *atext, const char *ahint)
{
	char buffer[errormessagesize];

	strcpy(buffer, atext);
	strcat(buffer, " ");
	strcat(buffer, ahint);
	MessageBox(NULL, buffer, alevel, MB_OK);
}


void __cdecl LogMessage(const char *astart, const char *atext, const char *aend)
{
	FILE *log;

	if(logfile[0]=='\0') return;

	log=fopen(logfile, "a");
	if(log!=NULL)
	{
		fputs(astart, log);
		fputs(atext, log);
		fputs(aend, log);
		fclose(log);
	}
}

int __cdecl _gpg_init()
{
	GetTempPath(sizeof(temporarydirectory),temporarydirectory);
	logfile[0]='\0';
	initPassphrases();
	initKeyUserIDs(publickeyuserid);
	initKeyUserIDs(secretkeyuserid);
	return 1;
}


int __cdecl _gpg_done()
{
	releaseKeyUserIDs(secretkeyuserid);
	releaseKeyUserIDs(publickeyuserid);
	releasePassphrases();
	return 1;
}


int __cdecl _gpg_open_keyrings(LPSTR ExecPath, LPSTR HomePath)
{
	if ( !ExecPath || (!*ExecPath && !ShowSelectExecDlg(ExecPath)) ) {
		return 0;
	}
	if ( !HomePath || (!*HomePath && !ShowSelectHomeDlg(HomePath)) ) {
		return 0;
	}
	if ( !existsFile(ExecPath) ) {
		//	ErrorMessage(txtwarning, txtinvalidexecutable, txtverifyoptions);
		return 0;
	}
	strcpy(gpgExecutable, ExecPath);
	strcpy(gpgHomeDirectory, HomePath);
	updateKeyUserIDs(publickeyuserid);
	updateKeyUserIDs(secretkeyuserid);
	return 1;
}


int __cdecl _gpg_close_keyrings()
{
	return 1;
}


LPSTR __cdecl _gpg_get_error()
{
	return 0;
}


void __cdecl _gpg_set_log(LPCSTR LogPath)
{
	if(LogPath)	strncpy(logfile,LogPath,sizeof(logfile));
	else logfile[0]='\0';
}


void __cdecl _gpg_set_tmp(LPCSTR TmpPath)
{
	if(TmpPath)	strncpy(temporarydirectory,TmpPath,sizeof(temporarydirectory));
	else GetTempPath(sizeof(temporarydirectory),temporarydirectory);
}


LPSTR __cdecl _gpg_get_passphrases()
{
	size_t i; char *b, x;

	b = (char *) LocalAlloc(LPTR,(keyuseridsize+passphrasesize)*passphrasecount+1); *b = '\0';

	for(i=0; i<(size_t)passphrasecount; i++) {
		strcat(b,passphrases[i].keyuserid); strcat(b,"\x01");
		strcat(b,passphrases[i].passphrase); strcat(b,"\x02");
	}

	// encrypt
	for(i=0; i<strlen(b); i++)
		if ( b[i]>2 ) {
			x = b[i] ^ ( (i&0x7f) ^ 13);
			if ( x>2 ) b[i]=x;
		}

		return b;
}


void __cdecl _gpg_set_passphrases(LPCSTR buffer)
{
	size_t i, l = strlen(buffer); char *t, *p, *b, x;

	if ( !l ) return;

	b = (char *) LocalAlloc(LPTR,l+1);
	strcpy(b, buffer);

	// decrypt
	for(i=0; i<strlen(b); i++)
		if ( b[i]>2 ) {
			x = b[i] ^ ( (i&0x7f) ^ 13);
			if ( x>2 ) b[i]=x;
		}

	while(*b) {
		t = strchr(b, '\x02');
		if(t) {
			*t = '\0';
			p = strchr(b, '\x01');
			*p = '\0';
			addPassphrase(b, p+1);
			t++;
		}
		b = t;
	}
	LocalFree(b);
}


LPSTR __cdecl _gpg_encrypt(LPCSTR message, LPCSTR keyid)
{
	char buffer[ciphertextsize];
	char *encmessage = 0;
	int encmessagelen;
	gpgResult gpgresult;

	if(strlen(keyid))
	{
		memset(buffer, 0, sizeof(buffer));
		gpgresult=gpgEncrypt(buffer, keyid, message);

		if(gpgresult!=gpgSuccess)
			return 0;

		encmessagelen = strlen(buffer)+1;
		encmessage = (char *) LocalAlloc(LPTR,encmessagelen);
		MoveMemory(encmessage, buffer, encmessagelen);
	}

	return encmessage;
}

LPSTR __cdecl _gpg_decrypt(LPCSTR message)
{
	char buffer[ciphertextsize];
	char plaintext[plaintextsize];
	char keyuserid[keyuseridsize];
	int dlgresult;
	BOOL useridvalid;
	char *storedpassphrase;
	char passphrase[passphrasesize];
	char *decmessage = 0;
	int decmessagelen;
	gpgResult gpgresult;

	const char *begin = strstr(message, txtbeginpgpmessage);
	const char *end = strstr(message, txtendpgpmessage);

	if ((begin!=NULL)&&(end!=NULL))
	{
		strcpy(buffer, "");
		strncat(buffer, begin, end-begin+strlen(txtendpgpmessage));
		replace(buffer, "\r", "");
		replace(buffer, "\n", txtcrlf);

		memset(keyuserid, 0, sizeof(keyuserid));
		gpgresult=gpgDetectUserID(keyuserid, buffer);
		storedpassphrase=NULL;

		if(gpgresult!=gpgSuccess)
		{
			//        ErrorMessage(txtwarning, txtdetectuseridfailed, txtverifyoptions);
			strcpy(keyuserid, txtunknownuserid);
			useridvalid=FALSE;
		}
		else
		{
			storedpassphrase=getPassphrase(keyuserid);
			useridvalid=TRUE;
		}

		if(storedpassphrase!=NULL)
		{
			strcpy(passphrase, storedpassphrase);
			memset(plaintext, 0, sizeof(plaintext));
			gpgresult=gpgDecrypt(plaintext, buffer, passphrase);
		}
		else gpgresult=gpgUnknownError;

		dlgresult=IDOK;
		while((gpgresult!=gpgSuccess)&&(dlgresult!=IDCANCEL))
		{
			dlgresult=DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_PASSPHRASE), NULL, PassphraseDialogProcedure, (LPARAM)keyuserid);

			if(dlgresult==IDOK)
			{
				strcpy(passphrase, dlgpassphrase);
				memset(dlgpassphrase, 0, passphrasesize);
				strcat(passphrase, txtcrlf);
				memset(plaintext, 0, sizeof(plaintext));
				gpgresult=gpgDecrypt(plaintext, buffer, passphrase);
			}
		}

		if(gpgresult==gpgSuccess)
		{
			strcpy(buffer, plaintext);
		}

		if ( gpgresult==gpgSuccess && useridvalid==TRUE)
			addPassphrase(keyuserid, passphrase);

		memset(passphrase, 0, sizeof(passphrase));

		decmessagelen = strlen(buffer)+1;
		decmessage = (char *) LocalAlloc(LPTR,decmessagelen);
		MoveMemory(decmessage, buffer, decmessagelen);
	}

	return decmessage;
}


int __cdecl _gpg_size_keyid()
{
	return keyidsize;
}


int __cdecl _gpg_select_keyid(HWND hdlg, LPSTR keyid)
{
	int dlgresult;

	memset(keyid, 0, keyidsize);
	dlgresult=DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SELECTKEY), hdlg, UserIdDialogProcedure, (LPARAM)keyid);

	if(dlgresult!=IDOK)
		memset(keyid, 0, keyidsize);

	return (dlgresult==IDOK);
}


void noBackslash(LPSTR path)
{
	LPSTR ptr = path + strlen(path) - 1;
	if ( *ptr=='\\' ) *ptr = '\0';
}


static char buf[MAX_PATH];


LPSTR GetRegValue(HKEY hKey , LPCSTR szPath, LPCSTR szName)
{
	DWORD len=MAX_PATH,type;
	LPSTR ret=0;

	RegOpenKey(hKey,szPath,&hKey);
	if ( RegQueryValueEx(hKey,szName,NULL,&type,(LPBYTE)&buf,&len)==ERROR_SUCCESS ) {
		noBackslash((LPSTR)&buf);
		ret = (LPSTR)&buf;
	}
	RegCloseKey(hKey);

	return ret;
}


LPSTR GetEnvValue(LPCSTR szName)
{
	LPSTR ret=0;

	if ( GetEnvironmentVariable(szName, buf, MAX_PATH) > 0 ) {
		noBackslash((LPSTR)&buf);
		ret = (LPSTR)&buf;
	}

	return ret;
}


BOOL ShowSelectExecDlg(LPSTR path)
{
	OPENFILENAME ofn;
	memset(&ofn, 0, sizeof(ofn));

	ofn.lpstrFile = GetRegValue(HKEY_CURRENT_USER,"Software\\GNU\\GnuPG","gpgProgram");
	if ( ofn.lpstrFile && existsFile(ofn.lpstrFile) ) {
		strcpy(path, ofn.lpstrFile);
		return TRUE;
	}
	ofn.lpstrFile = GetRegValue(HKEY_LOCAL_MACHINE,"Software\\GNU\\GnuPG","Install Directory");
	if ( ofn.lpstrFile ) {
		strcat(ofn.lpstrFile,"\\gpg.exe");
		if ( existsFile(ofn.lpstrFile) ) {
			strcpy(path, ofn.lpstrFile);
			return TRUE;
		}
	}

	ofn.lStructSize = sizeof(ofn);
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;

	ofn.lpstrFile = path;
	ofn.lpstrFilter = "GnuPG executable (gpg.exe)\0gpg.exe\0All files (*.*)\0*.*\0";
	ofn.lpstrTitle = "Select GnuPG executable";
	if (!GetOpenFileName(&ofn)) return FALSE;

	return TRUE;
}


BOOL ShowSelectHomeDlg(LPSTR path)
{
	int i;
	OPENFILENAME ofn;

	ofn.lpstrFile = GetEnvValue("GNUPGHOME");
	if ( ofn.lpstrFile && existsPath(ofn.lpstrFile) ) {
		strcpy(path, ofn.lpstrFile);
		return TRUE;
	}
	ofn.lpstrFile = GetRegValue(HKEY_CURRENT_USER,"Software\\GNU\\GnuPG","HomeDir");
	if ( ofn.lpstrFile && existsPath(ofn.lpstrFile) ) {
		strcpy(path, ofn.lpstrFile);
		return TRUE;
	}
	ofn.lpstrFile = GetEnvValue("APPDATA");
	if ( ofn.lpstrFile ) {
		strcat(ofn.lpstrFile,"\\gnupg");
		if ( existsPath(ofn.lpstrFile) ) {
			strcpy(path, ofn.lpstrFile);
			return TRUE;
		}
	}

	ofn.lStructSize = sizeof(ofn);
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_NONETWORKBUTTON;

	ofn.lpstrFile = path;
	ofn.lpstrFilter = "Public key rings (pubring.gpg)\0pubring.gpg\0All files (*.*)\0*.*\0";
	ofn.lpstrTitle = "Open Public Keyring";
	if (!GetOpenFileName(&ofn)) return FALSE;

	for(i=strlen(path);i && path[i]!='\\';i--);
	path[i] = 0;

	return TRUE;
}

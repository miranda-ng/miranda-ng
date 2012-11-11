#include "skype.h"
#include "skype_proto.h"

int hLangpack;
HINSTANCE g_hInstance;

CSkype* g_skype;

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {9C448C61-FC3F-42F9-B9F0-4A30E1CF8671}
	{0x9c448c61, 0xfc3f, 0x42f9, {0xb9, 0xf0, 0x4a, 0x30, 0xe1, 0xcf, 0x86, 0x71}}
};

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

int port = 8963;

void *buf;

static const char base64Fillchar = '='; // used to mark partial words at the end

const unsigned char base64DecodeTable[] = {
	99, 98, 98, 98, 98, 98, 98, 98, 98, 97,  97, 98, 98, 97, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  //00 -29
	98, 98, 97, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 62, 98, 98, 98, 63, 52, 53,  54, 55, 56, 57, 58, 59, 60, 61, 98, 98,  //30 -59
	98, 96, 98, 98, 98, 0, 1, 2, 3, 4,   5, 6, 7, 8, 9, 10, 11, 12, 13, 14,  15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  //60 -89
	25, 98, 98, 98, 98, 98, 98, 26, 27, 28,  29, 30, 31, 32, 33, 34, 35, 36, 37, 38,  39, 40, 41, 42, 43, 44, 45, 46, 47, 48,  //90 -119
	49, 50, 51, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  //120 -149
	98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  //150 -179
	98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  //180 -209
	98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  //210 -239
	98, 98, 98, 98, 98, 98, 98, 98, 98, 98,  98, 98, 98, 98, 98, 98                                               //240 -255
};

unsigned int decodeSize(char * data)
{
	if ( !data) return 0;
	int size = 0;
	unsigned char c;
	//skip any extra characters (e.g. newlines or spaces)
	while (*data)
	{
		if (*data>255) { return 0; }
		c = base64DecodeTable[(unsigned char)(*data)];
		if (c<97) size++;
		else if (c == 98) { return 0; }
		data++;
	}
	if (size == 0) return 0;
	do { data--; size--; } while (*data == base64Fillchar); size++;
	return (unsigned int)((size*3)/4);
}

unsigned char decode(char * data, unsigned char *buf, int len)
{
	if ( !data) return 0;
	int i=0, p = 0;
	unsigned char d, c;
	for (;;)
	{

	#define BASE64DECODE_READ_NEXT_CHAR(c)                                              \
	do {                                                                        \
	if (data[i]>255) { c = 98; break; }                                        \
	c = base64DecodeTable[(unsigned char)data[i++]];                       \
	}while (c == 97);                                                             \
	if(c == 98) { return 0; }

		BASE64DECODE_READ_NEXT_CHAR(c)
			if (c == 99) { return 2; }
			if (c == 96)
			{
				if (p == (int)len) return 2;
				return 1;
			}

			BASE64DECODE_READ_NEXT_CHAR(d)
				if ((d == 99) || (d == 96)) { return 1; }
				if (p == (int)len) { return 0; }
				buf[p++] = (unsigned char)((c<<2)|((d>>4)&0x3));

				BASE64DECODE_READ_NEXT_CHAR(c)
					if (c == 99) { return 1; }
					if (p == (int)len)
					{
						if (c == 96) return 2;
						return 0;
					}
					if (c == 96) { return 1; }
					buf[p++] = (unsigned char)(((d<<4)&0xf0)|((c>>2)&0xf));

					BASE64DECODE_READ_NEXT_CHAR(d)
						if (d == 99) { return 1; }
						if (p == (int)len)
						{
							if (d == 96) return 2;
							return 0;
						}
						if (d == 96) { return 1; }
						buf[p++] = (unsigned char)(((c<<6)&0xc0)|d);
	}
}
#undef BASE64DECODE_READ_NEXT_CHAR

unsigned char *decode(char * data, int *outlen)
{
	if ( !data) { *outlen = 0; return (unsigned char*)""; }
	unsigned int len = decodeSize(data);
	if (outlen) *outlen = len;
	if ( !len) return NULL;
	malloc(len+1);
	if( !decode(data, (unsigned char*)buf, len)) { return NULL; }
	return (unsigned char*)buf;
}

char* LoadKeyPair()
{
	HRSRC hRes = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_KEY), _T("BIN"));
	if (hRes) {
		HGLOBAL hResource = LoadResource(g_hInstance, hRes);
		if (hResource) {
			aes_context ctx;

			int basedecodedkey = decodeSize((char*)MY_KEY);
			unsigned char *tmpK = (unsigned char*)malloc(basedecodedkey + 1);
			decode((char*)MY_KEY, tmpK, basedecodedkey);
			tmpK[basedecodedkey] = 0;


			aes_set_key( &ctx, tmpK, 128);
			int dwResSize = SizeofResource(g_hInstance, hRes);
			char *pData = (char*)GlobalLock(hResource);
			pData[dwResSize] = 0;
			int basedecoded = decodeSize(pData);
			GlobalUnlock(hResource);
			unsigned char *bufD = (unsigned char*)mir_alloc(basedecoded + 1);
			unsigned char *tmpD = (unsigned char*)mir_alloc(basedecoded + 1);
			decode(pData, tmpD, basedecoded);
			for (int i = 0; i < basedecoded; i += 16) {
				aes_decrypt(&ctx, tmpD+i, bufD+i);
			}
			mir_free(tmpD);
			bufD[basedecoded] = 0; //cert should be null terminated
			return (char*)bufD;
		}
		return NULL;
	}
	return NULL;
}

//
//   FUNCTION: IsRunAsAdmin()
//
//   PURPOSE: The function checks whether the current process is run as 
//   administrator. In other words, it dictates whether the primary access 
//   token of the process belongs to user account that is a member of the 
//   local Administrators group and it is elevated.
//
//   RETURN VALUE: Returns TRUE if the primary access token of the process 
//   belongs to user account that is a member of the local Administrators 
//   group and it is elevated. Returns FALSE if the token does not.
//
//   EXCEPTION: If this function fails, it throws a C++ DWORD exception which 
//   contains the Win32 error code of the failure.
//
//   EXAMPLE CALL:
//     try 
//     {
//         if (IsRunAsAdmin())
//             wprintf (L"Process is run as administrator\n");
//         else
//             wprintf (L"Process is not run as administrator\n");
//     }
//     catch (DWORD dwError)
//     {
//         wprintf(L"IsRunAsAdmin failed w/err %lu\n", dwError);
//     }
//
BOOL IsRunAsAdmin()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if (!AllocateAndInitializeSid(
		&NtAuthority, 
		2, 
		SECURITY_BUILTIN_DOMAIN_RID, 
		DOMAIN_ALIAS_RID_ADMINS, 
		0, 0, 0, 0, 0, 0, 
		&pAdministratorsGroup))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	// Determine whether the SID of administrators group is enabled in 
	// the primary access token of the process.
	if (!CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

Cleanup:
	// Centralized cleanup for all allocated resources.
	if (pAdministratorsGroup)
	{
		FreeSid(pAdministratorsGroup);
		pAdministratorsGroup = NULL;
	}

	// Throw the error if something failed in the function.
	if (ERROR_SUCCESS != dwError)
	{
		throw dwError;
	}

	return fIsRunAsAdmin;
}

int StartSkypeRuntime()
{
	// loading skype runtime
	// shitcode
	STARTUPINFO cif;
	PROCESS_INFORMATION pi;
	TCHAR param[128];

	ZeroMemory(&cif,sizeof(STARTUPINFO));	
	cif.cb = sizeof(STARTUPINFO);
	cif.dwFlags = STARTF_USESHOWWINDOW;
	cif.wShowWindow = SW_HIDE;

	HRSRC 	hRes;
	HGLOBAL	hResource;
	TCHAR	szFilename[MAX_PATH];

	hRes = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_RUNTIME), _T("BIN"));

	if (hRes) {
		hResource = LoadResource(g_hInstance, hRes);
		if (hResource) {
			HANDLE  hFile;
			char 	*pData = (char *)LockResource(hResource);
			DWORD	dwSize = SizeofResource(g_hInstance, hRes), written = 0;
			GetModuleFileName(g_hInstance, szFilename, MAX_PATH);
			TCHAR *SkypeKitPath = _tcsrchr(szFilename, '\\');
			if (SkypeKitPath != NULL)
				*SkypeKitPath = '\0';
			mir_sntprintf(szFilename, SIZEOF(szFilename), _T("%s\\%s"), szFilename, _T("SkypeKit.exe"));
			if (!PathFileExists(szFilename))
			{
				if ((hFile = CreateFile(szFilename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0)) != INVALID_HANDLE_VALUE) {
					WriteFile(hFile, (void *)pData, dwSize, &written, NULL);
					CloseHandle(hFile);
				}
				else
				{
					// Check the current process's "run as administrator" status.
					// Elevate the process if it is not run as administrator.
					if (!IsRunAsAdmin())
					{
						wchar_t szPath[MAX_PATH], cmdLine[100];
						GetModuleFileName(NULL, szPath, ARRAYSIZE(szPath));
						TCHAR *profilename = Utils_ReplaceVarsT(_T("%miranda_profilename%"));
						mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T(" /restart:%d /profile=%s"), GetCurrentProcessId(), profilename);
						// Launch itself as administrator.
						SHELLEXECUTEINFO sei = { sizeof(sei) };
						sei.lpVerb = L"runas";
						sei.lpFile = szPath;
						sei.lpParameters = cmdLine;
						//sei.hwnd = hDlg;
						sei.nShow = SW_NORMAL;

						if (!ShellExecuteEx(&sei))
						{
							DWORD dwError = GetLastError();
							if (dwError == ERROR_CANCELLED)
							{
								// The user refused to allow privileges elevation.
								// Do nothing ...
							}
						}
						else
						{
							//DestroyWindow(hDlg);  // Quit itself
							CallService("CloseAction", 0, 0);
						}
					}
					return 0;
				}
			}
		}
	}

	if (FindWindow(NULL, szFilename))
		port += rand() % 100;
	
	mir_sntprintf(param, SIZEOF(param), L"-p -p %d", port);

	int startingrt = CreateProcess(szFilename, param, NULL,
					NULL, FALSE, CREATE_NEW_CONSOLE,
					NULL, NULL, &cif, &pi);
	return startingrt;
}

extern "C" int __declspec(dllexport) Load(void)
{
	if (!StartSkypeRuntime())
		return 1;

	g_skype = new CSkype();
	char *keyBuf = LoadKeyPair();
	g_skype->init(keyBuf, "127.0.0.1", port);
	mir_free(keyBuf);
	g_skype->start();	

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CSkypeProto::InitSkypeProto;
	pd.fnUninit = (pfnUninitProto)CSkypeProto::UninitSkypeProto;
	CallService(MS_PROTO_REGISTERMODULE, 0, reinterpret_cast<LPARAM>(&pd));

	CSkypeProto::InitIcons();
	CSkypeProto::InitServiceList();
	CSkypeProto::InitMenus();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	CSkypeProto::UninitMenus();
	CSkypeProto::UninitIcons();

	g_skype->stop();
	delete g_skype;

	return 0;
}
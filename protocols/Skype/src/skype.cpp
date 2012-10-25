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
	{ 0x9c448c61, 0xfc3f, 0x42f9, { 0xb9, 0xf0, 0x4a, 0x30, 0xe1, 0xcf, 0x86, 0x71 } }
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

char* keyBuf = 0;
int port = 8963;

int LoadKeyPair()
{
	FILE* f = 0;
	size_t fsize = 0;
	int keyLen = 0;

	f = fopen(g_keyFileName, "r");

	if (f != 0)
	{
		fseek(f, 0, SEEK_END);
		fsize = ftell(f);
		rewind(f);
		keyLen = fsize + 1;
		keyBuf = new char[keyLen];
		size_t read = fread(keyBuf, 1, fsize, f);
		if (read != fsize) 
		{ 
			printf("Error reading %s\n", g_keyFileName);
			return 0;
		};
		keyBuf[fsize] = 0; //cert should be null terminated
		fclose(f);
		return keyLen;		
	};
	
	printf("Error opening app token file: %s\n", g_keyFileName);

	return 0;
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

	hRes = FindResource(g_hInstance, MAKEINTRESOURCE(IDR_RUNTIME), _T("EXE"));

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

	int startingrt = CreateProcess(
		szFilename,
		param,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&cif, 
		&pi);
	return startingrt;
}

extern "C" int __declspec(dllexport) Load(void)
{
	LoadKeyPair();
	if (!StartSkypeRuntime())
		return 1;

	g_skype = new CSkype();
	g_skype->init(keyBuf, "127.0.0.1", port);
	delete[] keyBuf;
	g_skype->start();	

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CSkypeProto::InitSkypeProto;
	pd.fnUninit = (pfnUninitProto)CSkypeProto::UninitSkypeProto;
	CallService(MS_PROTO_REGISTERMODULE, 0, reinterpret_cast<LPARAM>(&pd));

	//CallService(
	//	MS_UTILS_GETCOUNTRYLIST, 
	//	(WPARAM)&CSkypeProto::countriesCount, 
	//	(LPARAM)&CSkypeProto::countryList);

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
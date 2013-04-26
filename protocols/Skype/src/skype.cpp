//#include "skype.h"
#include "skype_proto.h"

#include "aes\aes.h" 
#include "base64\base64.h"
//#include "skypekit\skype.h"

#include "..\..\..\skypekit\key.h"

int hLangpack;
CSkype *g_skype;
HINSTANCE g_hInstance;

TIME_API tmi = {0};

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

// ---

BOOL IsRunAsAdmin()
{
	BOOL fIsRunAsAdmin = FALSE;
	DWORD dwError = ERROR_SUCCESS;
	PSID pAdministratorsGroup = NULL;

	// Allocate and initialize a SID of the administrators group.
	SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
	if ( !AllocateAndInitializeSid(
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
	if ( !CheckTokenMembership(NULL, pAdministratorsGroup, &fIsRunAsAdmin))
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

char *LoadKeyPair(HINSTANCE hInstance)
{
	HRSRC hRes = FindResource(hInstance, MAKEINTRESOURCE(IDR_KEY), L"BIN");
	if (hRes)
	{
		HGLOBAL hResource = LoadResource(hInstance, hRes);
		if (hResource)
		{
			aes_context ctx;
			unsigned char key[128];

			int basedecoded = Base64::Decode(MY_KEY, (char *)key, MAX_PATH);
			::aes_set_key(&ctx, key, 128);
			memset(key, 0, sizeof(key));

			basedecoded = ::SizeofResource(hInstance, hRes);
			char *pData = (char *)hResource;
			if (!pData)
				return NULL;

			unsigned char *bufD = (unsigned char*)::malloc(basedecoded + 1);
			unsigned char *tmpD = (unsigned char*)::malloc(basedecoded + 1);
			basedecoded = Base64::Decode(pData, (char *)tmpD, basedecoded);

			for (int i = 0; i < basedecoded; i += 16)
				aes_decrypt(&ctx, tmpD+i, bufD+i);

			::free(tmpD);
			bufD[basedecoded] = 0; //cert should be null terminated
			return (char *)bufD;
		}
		return NULL;
	}
	return NULL;
}

int StartSkypeRuntime(HINSTANCE hInstance, const wchar_t *profileName, int &port)
{
	STARTUPINFO cif = {0};
	cif.cb = sizeof(STARTUPINFO);
	cif.dwFlags = STARTF_USESHOWWINDOW;
	cif.wShowWindow = SW_HIDE;

	wchar_t	fileName[MAX_PATH];
	::GetModuleFileName(hInstance, fileName, MAX_PATH);

	wchar_t *skypeKitPath = ::wcsrchr(fileName, '\\');
	if (skypeKitPath != NULL)
		*skypeKitPath = 0;
	::swprintf(fileName, SIZEOF(fileName), L"%s\\%s", fileName, L"SkypeKit.exe");
	if ( !::PathFileExists(fileName))
	{
		HRSRC hRes = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_RUNTIME), L"BIN");
		if (hRes)
		{
			HGLOBAL hResource = ::LoadResource(hInstance, hRes);
			if (hResource)
			{
				HANDLE hFile;
				char *pData = (char *)LockResource(hResource);
				DWORD dwSize = SizeofResource(hInstance, hRes), written = 0;
				if ((hFile = ::CreateFile(
					fileName,
					GENERIC_WRITE,
					0,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					0)) != INVALID_HANDLE_VALUE)
				{
					::WriteFile(hFile, (void *)pData, dwSize, &written, NULL);
					::CloseHandle(hFile);
				}
				else
				{
					// Check the current process's "run as administrator" status.
					// Elevate the process if it is not run as administrator.
					if (!IsRunAsAdmin())
					{
						wchar_t path[MAX_PATH], cmdLine[100];
						::GetModuleFileName(NULL, path, ARRAYSIZE(path));

						if (profileName)
							::swprintf(
							cmdLine,
							SIZEOF(cmdLine),
							L" /restart:%d /profile=%s",
							::GetCurrentProcessId(),
							profileName);
						else
							::swprintf(
							cmdLine,
							SIZEOF(cmdLine),
							L" /restart:%d",
							::GetCurrentProcessId());

						// Launch itself as administrator.
						SHELLEXECUTEINFO sei = { sizeof(sei) };
						sei.lpVerb = L"runas";
						sei.lpFile = path;
						sei.lpParameters = cmdLine;
						//sei.hwnd = hDlg;
						sei.nShow = SW_NORMAL;

						if ( !::ShellExecuteEx(&sei))
						{
							DWORD dwError = ::GetLastError();
							if (dwError == ERROR_CANCELLED)
							{
								// The user refused to allow privileges elevation.
								// Do nothing ...
							}
						}
					}
					else
						return 0;
				}
			}
			else
				return 0;
		}
		else
			return 0;
	}

	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (::Process32First(snapshot, &entry) == TRUE) {
		while (::Process32Next(snapshot, &entry) == TRUE) {
			if (::wcsicmp(entry.szExeFile, L"SkypeKit.exe") == 0) {
				HANDLE hProcess = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, entry.th32ProcessID);
				port += rand() % 8963 + 1000;
				::CloseHandle(hProcess);
				break;
			}
		}
	}
	::CloseHandle(snapshot);

	wchar_t param[128], path[MAX_PATH];
	PROCESS_INFORMATION pi;
	VARST dbPath( _T("%miranda_userdata%\\SkypeKit"));
	_tcslwr(dbPath);
	mir_sntprintf(path, SIZEOF(path), _T("\"%s\""), dbPath);
	::swprintf(param, SIZEOF(param), L"-p -P %d -f %s", port, path);
	//::swprintf(param, SIZEOF(param), L"-p -P %d", port);
	int startingrt = ::CreateProcess(
		fileName, param,
		NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE,
		NULL, NULL, &cif, &pi);

	return startingrt;
}

// ---

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);

	int port = 8963;
	VARST profilename( _T("%miranda_profilename%"));

	if ( !StartSkypeRuntime(g_hInstance, (TCHAR *)profilename, port))
	{
		::MessageBox(NULL, TranslateT("Proccess SkypeKit.exe did not start."), _T(MODULE), MB_OK | MB_ICONERROR);
		return 1;
	}

	char *keyPair = LoadKeyPair(g_hInstance);
	if ( !keyPair)
	{
		::MessageBox(NULL, TranslateT("Initialization key corrupted or not valid."), _T(MODULE), MB_OK | MB_ICONERROR);
		return 1;
	}

	g_skype = new CSkype();
	TransportInterface::Status status = g_skype->init(keyPair, "127.0.0.1", port);
	if (status != TransportInterface::OK)
	{
		::MessageBox(NULL, TranslateT("SkypeKit did not initialize."), _T(MODULE), MB_OK | MB_ICONERROR);
		return 1;
	}

	free(keyPair);

	if ( !g_skype->start())
	{
		::MessageBox(NULL, TranslateT("SkypeKit did not start."), _T(MODULE), MB_OK | MB_ICONERROR);
		return 1;
	}

	// ---

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "SKYPE";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CSkypeProto::InitSkypeProto;
	pd.fnUninit = (pfnUninitProto)CSkypeProto::UninitSkypeProto;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

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
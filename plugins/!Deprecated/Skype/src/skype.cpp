#include "skype.h"

int hLangpack;
HINSTANCE g_hInstance;

TIME_API tmi = {0};

int g_cbCountries;
struct CountryListEntry* g_countries;

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

int UnpackSkypeRuntime(HINSTANCE hInstance, const wchar_t *profileName)
{
	wchar_t	fileName[MAX_PATH];
	::GetModuleFileName(hInstance, fileName, MAX_PATH);

	wchar_t *skypeKitPath = ::wcsrchr(fileName, '\\');
	if (skypeKitPath != NULL)
		*skypeKitPath = 0;
	::mir_snwprintf(fileName, SIZEOF(fileName), L"%s\\%s", fileName, L"SkypeKit.exe");
	if ( ::GetFileAttributes(fileName) == DWORD(-1))
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
							::mir_snwprintf(
							cmdLine,
							SIZEOF(cmdLine),
							L" /restart:%d /profile=%s",
							::GetCurrentProcessId(),
							profileName);
						else
							::mir_snwprintf(
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

	return 1;
}

// ---

extern "C" int __declspec(dllexport) Load(void)
{
	VARST profilename( _T("%miranda_profilename%"));

	if ( !UnpackSkypeRuntime(g_hInstance, (TCHAR *)profilename))
	{
		::MessageBox(NULL, TranslateT("Did not unpack SkypeKit.exe."), _T(MODULE), MB_OK | MB_ICONERROR);
		return 1;
	}

	mir_getTMI(&tmi);
	mir_getLP(&pluginInfo);

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "SKYPE";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CSkypeProto::InitSkypeProto;
	pd.fnUninit = (pfnUninitProto)CSkypeProto::UninitSkypeProto;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&g_cbCountries, (LPARAM)&g_countries);

	CSkypeProto::InitIcons();
	CSkypeProto::InitMenus();
	CSkypeProto::InitHookList();
	CSkypeProto::InitLanguages();
	CSkypeProto::InitServiceList();

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	CSkypeProto::UninitIcons();
	CSkypeProto::UninitMenus();
	CSkypeProto::UninitInstances();
	return 0;
}
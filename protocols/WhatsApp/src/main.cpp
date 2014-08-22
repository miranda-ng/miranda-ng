#include "common.h"

CLIST_INTERFACE* pcli;
int hLangpack;

HINSTANCE g_hInstance;
std::string g_strUserAgent;
DWORD g_mirandaVersion;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	"WhatsApp Protocol",
	__VERSION_DWORD,
	"Provides basic support for WhatsApp.",
	"Uli Hecht",
	"uli.hecht@gmail.com",
	"© 2013-14 Uli Hecht",
	"http://example.com",
	UNICODE_AWARE, //not transient
	// {4f1ff7fa-4d75-44b9-93b0-2ced2e4f9e3e}
	{ 0x4f1ff7fa, 0x4d75, 0x44b9, { 0x93, 0xb0, 0x2c, 0xed, 0x2e, 0x4f, 0x9e, 0x3e } }

};

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

int UnpackRegisterUtility(HINSTANCE hInstance, const wchar_t *profileName)
{
	wchar_t	fileName[MAX_PATH];
	::GetModuleFileName(hInstance, fileName, MAX_PATH);

	wchar_t *RegisterUtilityPath = ::wcsrchr(fileName, '\\');
	if (RegisterUtilityPath != NULL)
		*RegisterUtilityPath = 0;
	::mir_snwprintf(fileName, SIZEOF(fileName), L"%s\\%s", fileName, L"WART-1.7.1.0.exe");
	if ( ::GetFileAttributes(fileName) == DWORD(-1))
	{
		HRSRC hRes = ::FindResource(hInstance, MAKEINTRESOURCE(IDR_REGISTERUTILITY), L"BIN");
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

/////////////////////////////////////////////////////////////////////////////
// Protocol instances
static int compare_protos(const WhatsAppProto *p1, const WhatsAppProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

OBJLIST<WhatsAppProto> g_Instances(1, compare_protos);

DWORD WINAPI DllMain(HINSTANCE hInstance,DWORD,LPVOID)
{
	g_hInstance = hInstance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	g_mirandaVersion = mirandaVersion;
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = {MIID_PROTOCOL, MIID_LAST};

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static PROTO_INTERFACE* protoInit(const char *proto_name,const TCHAR *username )
{
	WhatsAppProto *proto = new WhatsAppProto(proto_name,username);
	g_Instances.insert(proto);
	return proto;
}

static int protoUninit(PROTO_INTERFACE* proto)
{
	g_Instances.remove(( WhatsAppProto* )proto);
	return EXIT_SUCCESS;
}

static HANDLE g_hEvents[1];

extern "C" int __declspec(dllexport) Load(void)
{
	VARST profilename( _T("%miranda_profilename%"));

	if ( !UnpackRegisterUtility(g_hInstance, (TCHAR *)profilename))
	{
		::MessageBox(NULL, TranslateT("Did not unpack WART (registration utility)."), _T("WhatsApp"), MB_OK | MB_ICONERROR);
		return 1;
	}

	mir_getLP(&pluginInfo);
	mir_getCLI();

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "WhatsApp";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = protoInit;
	pd.fnUninit = protoUninit;
	CallService(MS_PROTO_REGISTERMODULE,0,reinterpret_cast<LPARAM>(&pd));

	InitIcons();
	//InitContactMenus();

	// Init native User-Agent
	{
		std::stringstream agent;
//		DWORD mir_ver = ( DWORD )CallService( MS_SYSTEM_GETVERSION, NULL, NULL );
		agent << "MirandaNG/";
		agent << (( g_mirandaVersion >> 24) & 0xFF);
		agent << ".";
		agent << (( g_mirandaVersion >> 16) & 0xFF);
		agent << ".";
		agent << (( g_mirandaVersion >>  8) & 0xFF);
		agent << ".";
		agent << (( g_mirandaVersion		) & 0xFF);
	#ifdef _WIN64
		agent << " WhatsApp Protocol x64/";
	#else
		agent << " WhatsApp Protocol/";
	#endif
		agent << __VERSION_STRING;
		g_strUserAgent = agent.str( );
	}

  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

extern "C" int __declspec(dllexport) Unload(void)
{
	//UninitContactMenus();
	for(size_t i=0; i<SIZEOF(g_hEvents); i++)
		UnhookEvent(g_hEvents[i]);

	g_Instances.destroy();

	delete FMessage::generating_lock;
	WASocketConnection::quitNetwork();

	return 0;
}

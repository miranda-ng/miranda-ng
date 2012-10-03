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

void StartSkypeRuntime()
{
	// loading skype runtime
	// shitcode
	wchar_t* bsp;
	STARTUPINFO cif;
	PROCESS_INFORMATION pi;
	wchar_t runtimePath[MAX_PATH];
	TCHAR param[128];

	GetModuleFileName(g_hInstance, runtimePath, MAX_PATH);
	bsp = wcsrchr(runtimePath, '\\' );
	runtimePath[wcslen(runtimePath) - wcslen(bsp)] = '\0';
	bsp = wcsrchr(runtimePath, '\\' );
	runtimePath[wcslen(runtimePath) - wcslen(bsp)] = '\0';
	bsp = wcsrchr(runtimePath, '\\' );
	runtimePath[wcslen(runtimePath) - wcslen(bsp)] = '\0';
	bsp = wcsrchr(runtimePath, '\\' );
	runtimePath[wcslen(runtimePath) - wcslen(bsp)] = '\0';
	bsp = wcsrchr(runtimePath, '\\' );
	runtimePath[wcslen(runtimePath) - wcslen(bsp)] = '\0';
	//\\..\\..\\..\\..
	wcscat(runtimePath, L"\\SkypeKit\\SDK\\bin\\windows-x86\\windows-x86-skypekit.exe");
	
	ZeroMemory(&cif,sizeof(STARTUPINFOA));	
	cif.cb = sizeof(STARTUPINFO);
	cif.dwFlags = STARTF_USESHOWWINDOW;
	cif.wShowWindow = SW_HIDE;

	
	if ( FindWindow(NULL, runtimePath))
		port += rand() % 100;
	
	mir_sntprintf(param, SIZEOF(param), L"-p -p %d", port);

	CreateProcess(
		runtimePath,
		param,
		NULL,
		NULL,
		FALSE,
		CREATE_NEW_CONSOLE,
		NULL,
		NULL,
		&cif, 
		&pi);
}

extern "C" int __declspec(dllexport) Load(void)
{
	LoadKeyPair();
	StartSkypeRuntime();

	g_skype = new CSkype();
	g_skype->init(keyBuf, "127.0.0.1", port);
	g_skype->start();	

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = MODULE;
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)CSkypeProto::InitSkypeProto;
	pd.fnUninit = (pfnUninitProto)CSkypeProto::UninitSkypeProto;
	CallService(MS_PROTO_REGISTERMODULE, 0, reinterpret_cast<LPARAM>(&pd));

	CSkypeProto::InitIcons();
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
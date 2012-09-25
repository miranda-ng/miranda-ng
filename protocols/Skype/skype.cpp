#include "skype.h"
#include "version.h"
#include "skype_proto.h"

int hLangpack;
HINSTANCE g_hInstance;

PLUGININFOEX pluginInfo =
{
	sizeof(PLUGININFOEX),
	"Skype Plugin",
	__VERSION_DWORD,
	"Provides basic support for Skype protocol.",
	"unsane",
	"",
	"© 2012 unsane",
	"http://miranda-ng.org/",
	UNICODE_AWARE,
	// {9C448C61-FC3F-42F9-B9F0-4A30E1CF8671}
	{ 0x9c448c61, 0xfc3f, 0x42f9, { 0xb9, 0xf0, 0x4a, 0x30, 0xe1, 0xcf, 0x86, 0x71 } }
};

static int compare_protos(const CSkypeProto *p1, const CSkypeProto *p2)
{
	return _tcscmp(p1->m_tszUserName, p2->m_tszUserName);
}

OBJLIST<CSkypeProto> g_Instances(1, compare_protos);

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

static CSkypeProto* SkypeProtoInit(const char* pszProtoName, const TCHAR* tszUserName)
{
	//CSkypeProto *ppro = new CSkypeProto(pszProtoName, tszUserName);
	//g_Instances.insert(ppro);
	//return ppro;
	return 0;
}

static int SkypeProtoUninit(CSkypeProto* ppro)
{
	g_Instances.remove(ppro);
	delete ppro;
	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	//mir_getLP(&pluginInfo);

	PROTOCOLDESCRIPTOR pd = { sizeof(pd) };
	pd.szName = "Skype";
	pd.type = PROTOTYPE_PROTOCOL;
	pd.fnInit = (pfnInitProto)SkypeProtoInit;
	pd.fnUninit = (pfnUninitProto)SkypeProtoUninit;
	CallService(MS_PROTO_REGISTERMODULE, 0, reinterpret_cast<LPARAM>(&pd));

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

#if !defined(MIID_CONNECTIONNOTIFY)
	#define MIID_CONNECTIONNOTIFY  {0x4bb5b4aa, 0xc364, 0x4f23, { 0x97, 0x46, 0xd5, 0xb7, 0x8, 0xa2, 0x86, 0xa5 } }
#endif
// 4BB5B4AA-C364-4F23-9746-D5B708A286A5


void showMsg(TCHAR *pName,DWORD pid,TCHAR *intIp,TCHAR *extIp,int intPort,int extPort,int state);
//int __declspec(dllexport) Load(PLUGINLINK *link);
extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved);
static unsigned __stdcall checkthread(void *dummy);
struct CONNECTION * LoadSettingsConnections();
void saveSettingsConnections(struct CONNECTION *connHead);

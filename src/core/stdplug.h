
#ifdef _STATIC
	#define MIRANDA_EXE_FUNC(T) extern "C" T __declspec(dllexport)
#else
	#define MIRANDA_EXE_FUNC(T) extern "C" T __declspec(dllimport)
#endif

MIRANDA_EXE_FUNC(HICON)  LoadSkinProtoIcon(const char *szProto, int status, bool big = false);
MIRANDA_EXE_FUNC(HICON)  LoadSkinIcon(int idx, bool big = false);
MIRANDA_EXE_FUNC(HANDLE) GetSkinIconHandle(int idx);

MIRANDA_EXE_FUNC(HANDLE) IcoLib_AddNewIcon(int hLangpack, SKINICONDESC* sid);
MIRANDA_EXE_FUNC(HICON)  IcoLib_GetIcon(const char* pszIconName, bool big);
MIRANDA_EXE_FUNC(HICON)  IcoLib_GetIconByHandle(HANDLE hItem, bool big);
MIRANDA_EXE_FUNC(HANDLE) IcoLib_IsManaged(HICON hIcon);
MIRANDA_EXE_FUNC(int)    IcoLib_ReleaseIcon(HICON hIcon, char* szIconName, bool big=false);

MIRANDA_EXE_FUNC(void)   Button_SetIcon_IcoLib(HWND hDlg, int itemId, int iconId, const char* tooltip);
MIRANDA_EXE_FUNC(void)   Button_FreeIcon_IcoLib(HWND hDlg, int itemId);

MIRANDA_EXE_FUNC(void)   Window_SetIcon_IcoLib(HWND hWnd, int iconId);
MIRANDA_EXE_FUNC(void)   Window_SetProtoIcon_IcoLib(HWND hWnd, const char *szProto, int iconId);
MIRANDA_EXE_FUNC(void)   Window_FreeIcon_IcoLib(HWND hWnd);

MIRANDA_EXE_FUNC(int)    GetPluginLangByInstance(HINSTANCE hInstance);

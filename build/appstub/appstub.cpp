#include <Windows.h>

#include <delayimp.h>
#pragma comment(lib, "delayimp.lib")

static HANDLE hUcrtDll = nullptr;

EXTERN_C HANDLE WINAPI hook(unsigned mode, PDelayLoadInfo)
{
	if (mode == dliNotePreLoadLibrary && hUcrtDll == nullptr) {
		wchar_t wszPath[MAX_PATH];
		GetModuleFileNameW(nullptr, wszPath, _countof(wszPath));

		// if current dir isn't set
		for (int i = lstrlenW(wszPath); i >= 0; i--)
			if (wszPath[i] == '\\') {
				wszPath[i] = 0;
				break;
			}

		SetCurrentDirectoryW(wszPath);

		lstrcatW(wszPath, L"\\libs");
		SetDllDirectoryW(wszPath);

#ifdef _DEBUG
		lstrcatW(wszPath, L"\\ucrtbased.dll");
#else
		lstrcatW(wszPath, L"\\ucrtbase.dll");
#endif
		hUcrtDll = LoadLibraryW(wszPath);
	}

	return 0;
}

EXTERN_C const PfnDliHook  __pfnDliNotifyHook2 = (PfnDliHook)&hook;

#include "stdafx.h"

using namespace Microsoft::WRL;

wchar_t* GetShortcutPath()
{
	wchar_t path[MAX_PATH];
	SHGetSpecialFolderPathW(nullptr, path, CSIDL_STARTMENU, FALSE);
	wcscat_s(path, L"\\Programs\\Miranda NG.lnk");
	return mir_wstrdup(path);
}

HRESULT InstallShortcut(_In_z_ wchar_t *shortcutPath)
{
	wchar_t exePath[MAX_PATH];
	GetModuleFileName(nullptr, exePath, MAX_PATH);

	ComPtr<IShellLink> shellLink;
	CHECKHR(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink)));

	CHECKHR(shellLink->SetPath(exePath));

	CHECKHR(shellLink->SetArguments(L""));

	ComPtr<IPropertyStore> propertyStore;
	CHECKHR(shellLink.As(&propertyStore));

	PROPVARIANT appIdPropVar;
	HRESULT hr = InitPropVariantFromString(AppUserModelID, &appIdPropVar);
	if (SUCCEEDED(hr)) {
		hr = propertyStore->SetValue(PKEY_AppUserModel_ID, appIdPropVar);
		if (SUCCEEDED(hr)) {
			hr = propertyStore->Commit();
			if (SUCCEEDED(hr)) {
				ComPtr<IPersistFile> persistFile;
				hr = shellLink.As(&persistFile);
				if (SUCCEEDED(hr)) {
					hr = persistFile->Save(shortcutPath, TRUE);
				}
			}
		}
		PropVariantClear(&appIdPropVar);
	}
	return hr;
}

bool ShortcutExists()
{
	return (GetFileAttributes(ptrW(GetShortcutPath())) < 0xFFFFFFF);
}

HRESULT TryCreateShortcut()
{
	return (ShortcutExists() ? S_OK : InstallShortcut(ptrW(GetShortcutPath())));
}

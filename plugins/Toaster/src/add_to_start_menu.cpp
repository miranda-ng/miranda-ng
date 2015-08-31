#include "stdafx.h"

#define SHORTCUT_PATH "\\Microsoft\\Windows\\Start Menu\\Programs\\Miranda NG.lnk"

using namespace Microsoft::WRL;

wchar_t* GetShortcutPath()
{
	wchar_t path[MAX_PATH];
	GetEnvironmentVariable(_T("APPDATA"), path, MAX_PATH);
	wcscat_s(path, _T(SHORTCUT_PATH));
	return mir_wstrdup(path);
}

HRESULT InstallShortcut(_In_z_ wchar_t *shortcutPath)
{
	wchar_t exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);

	ComPtr<IShellLink> shellLink;
	CHECKHR(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink)));

	CHECKHR(shellLink->SetPath(exePath));

	CHECKHR(shellLink->SetArguments(L""));

	ComPtr<IPropertyStore> propertyStore;
	CHECKHR(shellLink.As(&propertyStore));

	PROPVARIANT appIdPropVar;
	HRESULT hr = InitPropVariantFromString(AppUserModelID, &appIdPropVar);
	if (SUCCEEDED(hr))
	{
		CHECKHR(propertyStore->SetValue(PKEY_AppUserModel_ID, appIdPropVar));

		CHECKHR(propertyStore->Commit());

		ComPtr<IPersistFile> persistFile;
		CHECKHR(hr = shellLink.As(&persistFile))

		hr = persistFile->Save(shortcutPath, TRUE);
	}
	PropVariantClear(&appIdPropVar);

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

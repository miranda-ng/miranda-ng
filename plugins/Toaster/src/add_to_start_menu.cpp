#include "stdafx.h"

using namespace Microsoft::WRL;

wchar_t* GetShortcutPath()
{
	wchar_t shortcutPath[MAX_PATH];
	GetEnvironmentVariable(_T("APPDATA"), shortcutPath, MAX_PATH);
	wcscat_s(shortcutPath, ARRAYSIZE(shortcutPath), L"\\Microsoft\\Windows\\Start Menu\\Programs\\Miranda NG.lnk");

	return mir_wstrdup(shortcutPath);
}

HRESULT ShortcutExists()
{
	HRESULT hr;
	DWORD attributes = GetFileAttributes(ptrW(GetShortcutPath()));
	bool fileExists = attributes < 0xFFFFFFF;

	if (!fileExists)
	{
		hr = S_OK;
	}
	else
	{
		hr = S_FALSE;
	}
	return hr;
}

HRESULT TryCreateShortcut()
{
	if (!ShortcutExists())
		return InstallShortcut(ptrW(GetShortcutPath()));
	return S_OK;
}

HRESULT InstallShortcut(_In_z_ wchar_t *shortcutPath)
{
	wchar_t exePath[MAX_PATH];
	GetModuleFileName(NULL, exePath, MAX_PATH);

	ComPtr<IShellLink> shellLink;
	HRESULT hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));
	if (SUCCEEDED(hr))
	{
		hr = shellLink->SetPath(exePath);
		if (SUCCEEDED(hr))
		{
			hr = shellLink->SetArguments(L"");
			if (SUCCEEDED(hr))
			{
				ComPtr<IPropertyStore> propertyStore;
				hr = shellLink.As(&propertyStore);
				if (SUCCEEDED(hr))
				{
					PROPVARIANT appIdPropVar;
					hr = InitPropVariantFromString(AppUserModelID, &appIdPropVar);
					if (SUCCEEDED(hr))
					{
						hr = propertyStore->SetValue(PKEY_AppUserModel_ID, appIdPropVar);
						if (SUCCEEDED(hr))
						{
							hr = propertyStore->Commit();
							if (SUCCEEDED(hr))
							{
								ComPtr<IPersistFile> persistFile;
								hr = shellLink.As(&persistFile);
								if (SUCCEEDED(hr))
								{
									hr = persistFile->Save(shortcutPath, TRUE);
								}
							}
						}
						PropVariantClear(&appIdPropVar);
					}
				}
			}
		}
	}
	return hr;
}

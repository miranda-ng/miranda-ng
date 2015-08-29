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

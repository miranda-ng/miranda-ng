#include "stdafx.h"

using namespace Microsoft::WRL;

int hLangpack;
HINSTANCE g_hInst;

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
	// {61D5BB60-2249-4E3D-B23E-8FB86F04ED40}
	{ 0x61d5bb60, 0x2249, 0x4e3d, { 0xb2, 0x3e, 0x8f, 0xb8, 0x6f, 0x4, 0xed, 0x40 } }

};

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInst = hInstance;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	return &pluginInfo;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	CreateServiceFunction(MODULENAME "/Add", Service);

	if (!ShortcutExists())
	{
		CMenuItem mi;
		mi.position = -0x7FFFFFFF;
		//mi.hIcolibItem = icon.hIcolib;
		mi.name.a = LPGEN("Add to start menu");
		mi.pszService = MODULENAME "/Add";
		Menu_AddMainMenuItem(&mi);
	}

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

INT_PTR Service(WPARAM, LPARAM)
{
	return (INT_PTR)TryCreateShortcut();
}

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
	return S_FALSE;
}

HRESULT InstallShortcut(_In_z_ wchar_t *shortcutPath)
{
	wchar_t exePath[MAX_PATH];

	DWORD charWritten = GetModuleFileNameEx(GetCurrentProcess(), nullptr, exePath, ARRAYSIZE(exePath));

	HRESULT hr = charWritten > 0 ? S_OK : E_FAIL;

	if (SUCCEEDED(hr))
	{
		ComPtr<IShellLink> shellLink;
		hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));

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
	}
	return hr;
}

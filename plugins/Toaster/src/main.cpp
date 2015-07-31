#include "stdafx.h"

int hLangpack;

HINSTANCE g_hInstance;

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
	// {1FDBD8F0-3929-41BC-92D1-020779460769}
	{ 0x1fdbd8f0, 0x3929, 0x41bc, { 0x92, 0xd1, 0x2, 0x7, 0x79, 0x46, 0x7, 0x69 } }
};

DWORD WINAPI DllMain(HINSTANCE hInstance, DWORD, LPVOID)
{
	g_hInstance = hInstance;

	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD)
{
	OSVERSIONINFO osvi = { sizeof(OSVERSIONINFO) };
	//GetVersionEx(&osvi);
	//if (osvi.dwMajorVersion >= 6 && osvi.dwMinorVersion >= 2)
		return &pluginInfo;

	//MessageBox(NULL, _T(MODULE" supports Windows 8 or higher"), _T(MODULE), MB_OK | MB_ICONERROR);
	//return NULL;

}

static void __cdecl OnToastNotificationClicked(void* arg)
{
	MCONTACT hContact = (MCONTACT)arg;
	if (hContact)
		CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, (LPARAM)"");
}

static void ShowToastNotification(TCHAR* text, TCHAR* title, MCONTACT hContact)
{
	ptrT imagePath;
	ToastEventHandler *eventHandler;
	/*if (hContact)
	{
		eventHandler = new ToastEventHandler(OnToastNotificationClicked, (void*)hContact);

		TCHAR avatarPath[MAX_PATH] = { 0 };
		const char* szProto = GetContactProto(hContact);
		if (ProtoServiceExists(szProto, PS_GETMYAVATAR))
			if (!CallProtoService(szProto, PS_GETMYAVATAR, (WPARAM)avatarPath, (LPARAM)MAX_PATH))
				imagePath = mir_tstrdup(avatarPath);
	}
	else*/
		eventHandler = new ToastEventHandler(nullptr);

	ToastNotification notification(text, title, imagePath);
	notification.Show(eventHandler);
}

static INT_PTR CreatePopup(WPARAM wParam, LPARAM)
{
	POPUPDATA *ppd = (POPUPDATA*)wParam;
	ptrW text(mir_a2u(ppd->lpzText));
	ptrW contactName(mir_a2u(ppd->lpzContactName));

	ShowToastNotification(text, contactName, ppd->lchContact);

	return 0;
}

static INT_PTR CreatePopupW(WPARAM wParam, LPARAM)
{
	POPUPDATAW *ppd = (POPUPDATAW*)wParam;

	ShowToastNotification(ppd->lpwzText, ppd->lpwzContactName, ppd->lchContact);

	return 0;
}

static INT_PTR CreatePopup2(WPARAM wParam, LPARAM)
{
	POPUPDATA2 *ppd = (POPUPDATA2*)wParam;

	ptrW text, title;
	if (ppd->flags & PU2_UNICODE)
	{
		text = mir_wstrdup(ppd->lpwzText);
		title = mir_wstrdup(ppd->lpwzTitle);
	}
	else
	{
		text = mir_a2u(ppd->lpzText);
		title = mir_a2u(ppd->lpzTitle);
	}

	ShowToastNotification(text, title, ppd->lchContact);

	return 0;
}

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);

	CreateServiceFunction(MS_POPUP_ADDPOPUP, CreatePopup);
	CreateServiceFunction(MS_POPUP_ADDPOPUPW, CreatePopupW);
	CreateServiceFunction(MS_POPUP_ADDPOPUP2, CreatePopup2);

	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	return 0;
}

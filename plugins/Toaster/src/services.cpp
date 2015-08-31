#include "stdafx.h"

mir_cs csNotifications;
OBJLIST<ToastNotification> lstNotifications(1);

__forceinline bool isChatRoom(MCONTACT hContact)
{	return (db_get_b(hContact, GetContactProto(hContact), "ChatRoom", 0) == 1);
}

wchar_t* SaveBitmap(HBITMAP bmp, const char *szProto)
{
	wchar_t wszSavePath[MAX_PATH], wszRelativePath[MAX_PATH];
	GetEnvironmentVariableW(L"TEMP", wszSavePath, MAX_PATH);
	mir_snwprintf(wszRelativePath, L"\\MirandaToaster.%s.png", _A2T(szProto));
	wcscat_s(wszSavePath, wszRelativePath);

	if (!(GetFileAttributes(wszSavePath) < 0xFFFFFFF))
	{
		IMGSRVC_INFO isi = { sizeof(isi) };
		isi.wszName = mir_wstrdup(wszSavePath);
		isi.hbm = bmp;
		isi.dwMask = IMGI_HBITMAP;
		isi.fif = FREE_IMAGE_FORMAT::FIF_PNG;
		CallService(MS_IMG_SAVE, (WPARAM)&isi, IMGL_WCHAR);
	}

	return mir_wstrdup(wszSavePath);
}

wchar_t* ProtoIcon(const char *szProto)
{
	HICON hIcon = Skin_LoadProtoIcon(szProto, ID_STATUS_ONLINE, 1);
	wchar_t *wszResult = NULL;
	ICONINFO icon;
	if (GetIconInfo(hIcon, &icon))
	{
		wszResult = SaveBitmap(icon.hbmColor, szProto);

		DeleteObject(icon.hbmMask);
		DeleteObject(icon.hbmColor);
	}
	return wszResult;
}

static void __cdecl OnToastNotificationClicked(void* arg)
{
	callbackArg *cb = (callbackArg*)arg;
	MCONTACT hContact = cb->hContact;
	if (hContact)
	{
		if (!isChatRoom(hContact))
		{
			CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact);
		}
		else
		{
			const char *szProto = GetContactProto(hContact);
			ptrT tszChatRoom(db_get_tsa(hContact, szProto, "ChatRoomID"));
			GCDEST gcd = { szProto, tszChatRoom, GC_EVENT_CONTROL };
			GCEVENT gce = { sizeof(gce), &gcd };
			CallServiceSync(MS_GC_EVENT, WINDOW_VISIBLE, (LPARAM)&gce);
		}
	}
}

static void ShowToastNotification(TCHAR* text, TCHAR* title, MCONTACT hContact)
{
	if (!db_get_b(0, "Popup", "ModuleIsEnabled", 1))
		return;

	ptrT imagePath;
	callbackArg *arg = (callbackArg*)mir_calloc(sizeof(callbackArg));
	if (hContact)
	{
		arg->hContact = hContact;
		const char* szProto = GetContactProto(hContact);

		if (ProtoServiceExists(szProto, PS_GETAVATARINFO))
		{
			PROTO_AVATAR_INFORMATION pai = { 0 };
			pai.hContact = hContact;
			if (CallProtoService(szProto, PS_GETAVATARINFO, 0, (LPARAM)&pai) == GAIR_SUCCESS)
			{
				imagePath = mir_tstrdup(pai.filename);
			}
			else
			{
				imagePath = ProtoIcon(szProto);
			}
		}
		else
		{
			imagePath = ProtoIcon(szProto);
		}
	}

	arg->notification = new ToastNotification(text, title, imagePath);
		
	HRESULT hr = arg->notification->Initialize();
	if (SUCCEEDED(hr))
	{
		arg->notification->Show(new ToastEventHandler(OnToastNotificationClicked, arg));
		lstNotifications.insert(arg->notification);
	}
	else
	{
		OutputDebugStringA(CMStringA(FORMAT, "Toaster error: HRESULT = %lld", hr));
#ifdef _DEBUG	
		DebugBreak();
#else
		Netlib_Logf(NULL, "Toaster: " __FUNCTION__ " failed: HRESULT = %lld", hr);
#endif
		delete arg->notification;
	}
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

static INT_PTR PopupQuery(WPARAM wParam, LPARAM)
{
	switch (wParam) {
	case PUQS_ENABLEPOPUPS:
	{
		bool enabled = db_get_b(0, "Popup", "ModuleIsEnabled", 1) != 0;
		if (!enabled) db_set_b(0, "Popup", "ModuleIsEnabled", 1);
		return !enabled;
	}
	break;
	case PUQS_DISABLEPOPUPS:
	{
		bool enabled = db_get_b(0, "Popup", "ModuleIsEnabled", 1) != 0;
		if (enabled) db_set_b(0, "Popup", "ModuleIsEnabled", 0);
		HideAllToasts();
		return enabled;
	}
	break;

	case PUQS_GETSTATUS:
		return db_get_b(0, "Popup", "ModuleIsEnabled", 1);

	default:
		return 1;
	}
}

void HideAllToasts()
{
	mir_cslock lck(csNotifications);
	while (lstNotifications.getCount())
	{
		lstNotifications[0].Hide();
		lstNotifications.remove(0);
	}
}

void InitServices()
{
	CreateServiceFunction(MS_POPUP_ADDPOPUP, CreatePopup);
	CreateServiceFunction(MS_POPUP_ADDPOPUPW, CreatePopupW);
	CreateServiceFunction(MS_POPUP_ADDPOPUP2, CreatePopup2);
	CreateServiceFunction(MS_POPUP_QUERY, PopupQuery);
}

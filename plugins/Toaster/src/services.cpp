#include "stdafx.h"

mir_cs csNotifications;
OBJLIST<ToastNotification> lstNotifications(1);

__forceinline bool isChatRoom(MCONTACT hContact)
{	return (db_get_b(hContact, GetContactProto(hContact), "ChatRoom", 0) == 1);
}

static void __cdecl OnToastNotificationClicked(void* arg)
{
	callbackArg *cb = (callbackArg*)arg;
	MCONTACT hContact = cb->hContact;
	if (hContact)
	{
		if (!isChatRoom(hContact))
		{
			CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, (LPARAM)"");
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
	lstNotifications.remove(cb->notification);
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
			CallProtoService(szProto, PS_GETAVATARINFO, (WPARAM)0, (LPARAM)&pai);
			if (pai.format != PA_FORMAT_PNG)
			{
				wchar_t dir[MAX_PATH];
				FoldersGetCustomPathT(g_hTempAvatarsFolder, dir, _countof(dir), VARSW(L"%miranda_userdata%\\Temp"));
				ToasterAvatar *ta = new ToasterAvatar(&pai);
				CMStringW wszPath(FORMAT, L"%s\\%lld.%d.png", dir, hContact, db_get_dw(hContact, "ContactPhoto", "ImagePath"));
				ta->Save(wszPath);
				imagePath = wszPath.Detach();
				delete ta;
			}
			imagePath = pai.filename[0] ? mir_tstrdup(pai.filename) : nullptr;
		}
	}

	ToastNotification* notification = new ToastNotification (text, title, imagePath);
	arg->notification = notification;
	notification->Show(new ToastEventHandler(OnToastNotificationClicked, arg));
	lstNotifications.insert(notification);
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

		mir_cslock lck(csNotifications);
		while (lstNotifications.getCount())
		{
			lstNotifications[0].Hide();
			lstNotifications.remove(0);
		}

		return enabled;
	}
	break;

	case PUQS_GETSTATUS:
		return db_get_b(0, "Popup", "ModuleIsEnabled", 1);

	default:
		return 1;
	}
}

void InitServices()
{
	CreateServiceFunction(MS_POPUP_ADDPOPUP, CreatePopup);
	CreateServiceFunction(MS_POPUP_ADDPOPUPW, CreatePopupW);
	CreateServiceFunction(MS_POPUP_ADDPOPUP2, CreatePopup2);
	CreateServiceFunction(MS_POPUP_QUERY, PopupQuery);
}

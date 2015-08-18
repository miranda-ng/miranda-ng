#include "stdafx.h"

static void __cdecl OnToastNotificationClicked(void* arg)
{
	MCONTACT hContact = (MCONTACT)arg;
	if (hContact)
		CallService(MS_MSG_SENDMESSAGE, (WPARAM)hContact, (LPARAM)"");
}

static void ShowToastNotification(TCHAR* text, TCHAR* title, MCONTACT hContact)
{
	if (!db_get_b(0, "Popup", "ModuleIsEnabled", 1))
		return;

	ptrT imagePath;
	ToastEventHandler *eventHandler;
	if (hContact)
	{
		eventHandler = new ToastEventHandler(OnToastNotificationClicked, (void*)hContact);

		TCHAR avatarPath[MAX_PATH] = { 0 };
		const char* szProto = GetContactProto(hContact);
		if (ProtoServiceExists(szProto, PS_GETMYAVATAR))
			if (!CallProtoService(szProto, PS_GETMYAVATAR, (WPARAM)avatarPath, (LPARAM)MAX_PATH))
				imagePath = mir_tstrdup(avatarPath);
	}
	else
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
		return enabled;
	}
	break;

	case PUQS_GETSTATUS:
		return db_get_b(0, "Popup", "ModuleIsEnabled", 1);

	default:
		return 1;
	}

	return 0;
}

void InitServices()
{
	CreateServiceFunction(MS_POPUP_ADDPOPUP, CreatePopup);
	CreateServiceFunction(MS_POPUP_ADDPOPUPW, CreatePopupW);
	CreateServiceFunction(MS_POPUP_ADDPOPUP2, CreatePopup2);
	CreateServiceFunction(MS_POPUP_QUERY, PopupQuery);
}

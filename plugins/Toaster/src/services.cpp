#include "stdafx.h"

mir_cs csNotifications;
OBJLIST<ToastNotification> lstNotifications(1);
std::map<std::string, ClassData*> mp_Classes;
wchar_t wszTempDir[MAX_PATH];

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

void __stdcall ShowToastNotification(void* p)
{
	std::unique_ptr<ToastData> td((ToastData*)p);

	if (!db_get_b(0, "Popup", "ModuleIsEnabled", 1))
		return;

	ptrT imagePath;
	callbackArg *arg = (callbackArg*)mir_calloc(sizeof(callbackArg));
	if (td->hContact != NULL && td->hContact != INVALID_CONTACT_ID)
	{
		arg->hContact = td->hContact;
		const char* szProto = GetContactProto(td->hContact);

		if (ProtoServiceExists(szProto, PS_GETAVATARINFO))
		{
			PROTO_AVATAR_INFORMATION pai = { td->hContact };
			if (CallProtoService(szProto, PS_GETAVATARINFO, 0, (LPARAM)&pai) == GAIR_SUCCESS)
			{
				imagePath = mir_tstrdup(pai.filename);
			}
		}

		if (imagePath == NULL)
		{
			if (szProto)
			{
				imagePath = ProtoIcon(szProto);
			}
			else if (td->hIcon)
			{
				imagePath = SaveHIcon(td->hIcon, CMStringA(FORMAT, "%p", td->hIcon));
			}
		}
	}
	else
	{
		if (td->hIcon)
		{
			imagePath = SaveHIcon(td->hIcon, CMStringA(FORMAT, "%p", td->hIcon));
		}
	}

	arg->notification = new ToastNotification(td->tszText, td->tszTitle, imagePath);
		
	HRESULT hr = arg->notification->Initialize();
	if (SUCCEEDED(hr))
	{
		arg->notification->Show(new ToastEventHandler(OnToastNotificationClicked, arg));
		lstNotifications.insert(arg->notification);
	}
	else
	{
		delete arg->notification;
		mir_free(arg);
	}
}

static INT_PTR CreatePopup(WPARAM wParam, LPARAM)
{
	POPUPDATA *ppd = (POPUPDATA*)wParam;
	ptrW text(mir_a2u(ppd->lpzText));
	ptrW contactName(mir_a2u(ppd->lpzContactName));

	CallFunctionAsync(&ShowToastNotification, new ToastData(ppd->lchContact, contactName, text, ppd->lchIcon));

	return 0;
}

static INT_PTR CreatePopupW(WPARAM wParam, LPARAM)
{
	POPUPDATAW *ppd = (POPUPDATAW*)wParam;
	CallFunctionAsync(&ShowToastNotification, new ToastData(ppd->lchContact, ppd->lpwzContactName, ppd->lpwzText, ppd->lchIcon));
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

	CallFunctionAsync(&ShowToastNotification, new ToastData(ppd->lchContact, title, text, ppd->lchIcon));

	return 0;
}

static INT_PTR RegisterClass(WPARAM, LPARAM lParam)
{
	POPUPCLASS *pc = (POPUPCLASS*)lParam;

	ClassData *cd = new ClassData(pc->flags, pc->hIcon);
	mp_Classes[pc->pszName] = cd;

	return (INT_PTR)cd->handle;
}

static INT_PTR CreateClassPopup(WPARAM, LPARAM lParam)
{
	POPUPDATACLASS *ppc = (POPUPDATACLASS*)lParam;

	auto it = mp_Classes.find(ppc->pszClassName);
	if (it != mp_Classes.end())
	{
		if (it->second->iFlags & PCF_TCHAR)
		{
			CallFunctionAsync(&ShowToastNotification, new ToastData(ppc->hContact, ppc->ptszTitle, ppc->ptszText, it->second->hIcon));
		}
		else
		{
			CallFunctionAsync(&ShowToastNotification, new ToastData(ppc->hContact, mir_utf8decodeT(ppc->pszTitle), mir_utf8decodeT(ppc->pszText), it->second->hIcon));
		}
	}

	return 0;
}

static INT_PTR UnRegisterClass(WPARAM, LPARAM lParam)
{
	HANDLE h = (HANDLE)lParam;

	for (auto it = mp_Classes.begin(); it != mp_Classes.end(); it++)
	{
		if (it->second->handle == h)
		{
			delete it->second;
			mp_Classes.erase(it);
		}
	}
	return 0;
}

void CleanupClasses()
{
	for (auto it = mp_Classes.begin(); it != mp_Classes.end(); it++)
	{
		delete it->second;
		mp_Classes.erase(it);
	}
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
		CallFunctionAsync(&HideAllToasts, NULL);
		return enabled;
	}
	break;

	case PUQS_GETSTATUS:
		return db_get_b(0, "Popup", "ModuleIsEnabled", 1);

	default:
		return 1;
	}
}

void __stdcall HideAllToasts(void*)
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
	CreateServiceFunction(MS_POPUP_ADDPOPUPCLASS, CreateClassPopup);
	CreateServiceFunction(MS_POPUP_REGISTERCLASS, RegisterClass);
	CreateServiceFunction(MS_POPUP_UNREGISTERCLASS, UnRegisterClass);
}
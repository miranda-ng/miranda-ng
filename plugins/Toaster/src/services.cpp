#include "stdafx.h"

std::map<std::string, ClassData*> mp_Classes;
wchar_t wszTempDir[MAX_PATH];

void __stdcall ShowToastNotification(void *p)
{
	std::unique_ptr<ToastData> td((ToastData*)p);

	if (!Popup_Enabled())
		return;

	ptrW imagePath;
	if (td->hContact != NULL && td->hContact != INVALID_CONTACT_ID) {
		const char *szProto = Proto_GetBaseAccountName(td->hContact);
		if (ProtoServiceExists(szProto, PS_GETAVATARINFO)) {
			PROTO_AVATAR_INFORMATION pai = { td->hContact };
			if (CallProtoService(szProto, PS_GETAVATARINFO, 0, (LPARAM)&pai) == GAIR_SUCCESS)
				imagePath = mir_wstrdup(pai.filename);
		}

		if (imagePath == NULL) {
			if (szProto)
				imagePath = ToasterImage(szProto);
			else {
				if (td->iType == 1 && td->hBitmap)		imagePath = ToasterImage(td->hBitmap);
				else if (td->iType == 2 && td->hIcon)	imagePath = ToasterImage(td->hIcon);
			}
		}
	}
	else if (td->hIcon)
		imagePath = ToasterImage(td->hIcon);

	new (std::nothrow) ToastNotification(td->tszText, td->tszTitle, imagePath, td->hContact, td->pPopupProc, td->vPopupData);
}

static INT_PTR GetPopupData(WPARAM wParam, LPARAM)
{
	return (INT_PTR)((ToastNotification*)wParam)->GetPluginData();
}

static INT_PTR GetPopupContact(WPARAM wParam, LPARAM)
{
	return (INT_PTR)((ToastNotification*)wParam)->GetContact();
}

static INT_PTR CreatePopup(WPARAM wParam, LPARAM)
{
	POPUPDATA *ppd = (POPUPDATA*)wParam;
	ptrW text(mir_a2u(ppd->lpzText));
	ptrW contactName(mir_a2u(ppd->lpzContactName));

	ToastData *td = new ToastData(ppd->lchContact, contactName, text, ppd->lchIcon);
	td->vPopupData = ppd->PluginData;
	td->pPopupProc = ppd->PluginWindowProc;

	CallFunctionAsync(&ShowToastNotification, td);

	return 0;
}

static INT_PTR CreatePopupW(WPARAM wParam, LPARAM)
{
	POPUPDATAW *ppd = (POPUPDATAW*)wParam;

	ToastData *td = new ToastData(ppd->lchContact, ppd->lpwzContactName, ppd->lpwzText, ppd->lchIcon);
	td->vPopupData = ppd->PluginData;
	td->pPopupProc = ppd->PluginWindowProc;

	CallFunctionAsync(ShowToastNotification, td);
	return 0;
}

static INT_PTR CreatePopup2(WPARAM wParam, LPARAM)
{
	POPUPDATA2 *ppd = (POPUPDATA2*)wParam;

	ptrW text, title;
	if (ppd->flags & PU2_UNICODE) {
		text = mir_wstrdup(ppd->szText.w);
		title = mir_wstrdup(ppd->szTitle.w);
	}
	else {
		text = mir_a2u(ppd->szText.a);
		title = mir_a2u(ppd->szTitle.a);
	}

	ToastData *td = nullptr;

	if (ppd->hbmAvatar)
		td = new ToastData(ppd->lchContact, title, text, ppd->hbmAvatar);
	else
		td = new ToastData(ppd->lchContact, title, text, ppd->lchIcon);

	td->vPopupData = ppd->PluginData;
	td->pPopupProc = ppd->PluginWindowProc;

	CallFunctionAsync(ShowToastNotification, td);
	return 0;
}

static INT_PTR RegisterPopupClass(WPARAM, LPARAM lParam)
{
	POPUPCLASS *pc = (POPUPCLASS*)lParam;

	ClassData *cd = new ClassData(pc->flags, pc->hIcon);
	cd->pPopupProc = pc->PluginWindowProc;

	mp_Classes[pc->pszName] = cd;

	return (INT_PTR)cd;
}

static INT_PTR CreateClassPopup(WPARAM, LPARAM lParam)
{
	POPUPDATACLASS *ppc = (POPUPDATACLASS*)lParam;

	auto it = mp_Classes.find(ppc->pszClassName);
	if (it != mp_Classes.end()) {
		ToastData *td;
		if (it->second->iFlags & PCF_UNICODE)
			td = new ToastData(ppc->hContact, ppc->szTitle.w, ppc->szText.w, it->second->hIcon);
		else
			td = new ToastData(ppc->hContact, ptrW(mir_utf8decodeW(ppc->szTitle.a)), ptrW(mir_utf8decodeW(ppc->szText.a)), it->second->hIcon);

		td->vPopupData = ppc->PluginData;
		td->pPopupProc = it->second->pPopupProc;

		CallFunctionAsync(ShowToastNotification, td);
	}

	return 0;
}

static INT_PTR UnRegisterPopupClass(WPARAM, LPARAM lParam)
{
	for (auto it = mp_Classes.begin(); it != mp_Classes.end(); it++) {
		if (it->second == (void *)lParam) {
			delete it->second;
			mp_Classes.erase(it);
			break;
		}
	}
	return 0;
}

void CleanupClasses()
{
	for (auto it = mp_Classes.begin(); it != mp_Classes.end(); ++it)
		delete it->second;
	mp_Classes.clear();
}

static INT_PTR ShowMessageW(WPARAM wParam, LPARAM lParam)
{
	HICON hIcon = nullptr;
	switch (lParam) {
	case SM_WARNING:
		hIcon = Skin_LoadIcon(SKINICON_WARNING, 1);
		break;
	case SM_ERROR:
		hIcon = Skin_LoadIcon(SKINICON_ERROR, 1);
		break;
	case SM_NOTIFY:
		hIcon = Skin_LoadIcon(SKINICON_INFORMATION, 1);
		break;
	}

	ToastData *td = new ToastData(NULL, nullptr, (wchar_t *)wParam, hIcon);
	CallFunctionAsync(ShowToastNotification, td);

	return 0;
}

static INT_PTR ShowMessage(WPARAM wParam, LPARAM lParam)
{
	ptrW tszText(mir_utf8decodeW((char *)wParam));
	return ShowMessageW(tszText, lParam);
}

static INT_PTR HideToast(WPARAM, LPARAM lParam)
{
	auto *pNotification = (ToastNotification*)lParam;
	if (pNotification)
		pNotification->Destroy();
	return 0;
}

void InitServices()
{
	CreateServiceFunction(MS_POPUP_SHOWMESSAGE, ShowMessage);
	CreateServiceFunction(MS_POPUP_SHOWMESSAGEW, ShowMessageW);

	CreateServiceFunction(MS_POPUP_ADDPOPUP, CreatePopup);
	CreateServiceFunction(MS_POPUP_ADDPOPUPW, CreatePopupW);
	CreateServiceFunction(MS_POPUP_ADDPOPUP2, CreatePopup2);

	CreateServiceFunction(MS_POPUP_ADDPOPUPCLASS, CreateClassPopup);
	CreateServiceFunction(MS_POPUP_REGISTERCLASS, RegisterPopupClass);
	CreateServiceFunction(MS_POPUP_UNREGISTERCLASS, UnRegisterPopupClass);

	CreateServiceFunction(MS_POPUP_GETPLUGINDATA, GetPopupData);
	CreateServiceFunction(MS_POPUP_GETCONTACT, GetPopupContact);

	CreateServiceFunction(MS_POPUP_DESTROYPOPUP, HideToast);
}

#include "stdafx.h"

HICON hIconNotSecure, hIconFinished, hIconPrivate, hIconUnverified;
BBButton OTRButton;

int SVC_IconPressed(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;

	if (mir_strcmp(sicd->szModule, MODULENAME) == 0) {
		char *proto = GetContactProto(hContact);
		if (proto && db_get_b(hContact, proto, "ChatRoom", 0))
			return 0;
		ShowOTRMenu(hContact, sicd->clickLocation);
	}

	return 0;
}

// set SRMM icon status, if applicable
void SetEncryptionStatus(MCONTACT hContact, TrustLevel level)
{
	char *proto = GetContactProto(hContact);
	bool chat_room = (proto && db_get_b(hContact, proto, "ChatRoom", 0));

	BBButton button = OTRButton;

	StatusIconData sid = {}, sid2 = {};
	sid.szModule = MODULENAME;
	sid.dwId = 0;
	sid.flags = MBF_HIDDEN;

	sid2.szModule = MODULENAME;
	sid2.dwId = 1;
	sid2.flags = MBF_HIDDEN;

	if (!chat_room) {
		switch (level) {
		case TRUST_FINISHED:
			sid.flags = 0;
			button.pwszTooltip = TranslateW(LANG_STATUS_FINISHED);
			button.hIcon = IcoLib_GetIconHandle(ICON_FINISHED);
			break;
		case TRUST_UNVERIFIED:
			sid2.flags = MBF_DISABLED;
			button.pwszTooltip = TranslateW(LANG_STATUS_UNVERIFIED);
			button.hIcon = IcoLib_GetIconHandle(ICON_UNVERIFIED);
			break;
		case TRUST_PRIVATE:
			sid2.flags = 0;
			button.pwszTooltip = TranslateW(LANG_STATUS_PRIVATE);
			button.hIcon = IcoLib_GetIconHandle(ICON_PRIVATE);
			break;
		default:
			sid.flags = MBF_DISABLED;
			button.pwszTooltip = TranslateW(LANG_STATUS_DISABLED);
			button.hIcon = IcoLib_GetIconHandle(ICON_NOT_PRIVATE);
			break;
		}
		button.bbbFlags = 0;
	}
	else button.bbbFlags = BBSF_HIDDEN;

	Srmm_ModifyIcon(hContact, &sid);
	Srmm_ModifyIcon(hContact, &sid2);
	Srmm_SetButtonState(hContact, &button);

	db_set_dw(hContact, MODULENAME, "TrustLevel", level);

	if (!chat_room) {
		MCONTACT hMeta = db_mc_getMeta(hContact);
		if(hMeta){
			MCONTACT hMostOnline = db_mc_getMostOnline(hMeta);
			if(hContact == hMostOnline) {
				SetEncryptionStatus(hMeta, level);
			} else {
				/* in case the new most online contact has changed
				(e.g. when the otr subcontact goes offline) */
				ConnContext *context = otrl_context_find_miranda(otr_user_state, hMostOnline);
				TrustLevel encrypted = otr_context_get_trust(context);
				SetEncryptionStatus(hMeta, encrypted);
			}
		}
	}
}

int SVC_ButtonsBarLoaded(WPARAM, LPARAM)
{
	Srmm_AddButton(&OTRButton);
	return 0;
}

int SVC_ButtonsBarPressed(WPARAM w, LPARAM l)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)l;
	if (cbcd->dwButtonId == 0 && !mir_strcmp(cbcd->pszModule, MODULENAME)) {
		MCONTACT hContact = (MCONTACT)w;
	
		char *proto = GetContactProto(hContact);
		if(proto && db_get_b(hContact, proto, "ChatRoom", 0))
			return 0;
		ShowOTRMenu(hContact, cbcd->pt);

	}
	return 0;
}

void InitSRMM()
{
	// add icon to srmm status icons
	InitMirOTRMenu();

	hIconNotSecure = IcoLib_GetIcon(ICON_NOT_PRIVATE, 0);
	hIconFinished = IcoLib_GetIcon(ICON_FINISHED, 0);
	hIconPrivate = IcoLib_GetIcon(ICON_PRIVATE, 0);
	hIconUnverified = IcoLib_GetIcon(ICON_UNVERIFIED, 0);

	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.hIcon = hIconFinished;
	sid.hIconDisabled = hIconNotSecure;
	sid.flags = MBF_DISABLED | MBF_HIDDEN;
	sid.szTooltip = LANG_OTR_TOOLTIP;
	Srmm_AddIcon(&sid);

	sid.dwId = 1;
	sid.hIcon = hIconPrivate;
	sid.hIconDisabled = hIconUnverified;
	Srmm_AddIcon(&sid);
		
	// hook the window events so that we can can change the status of the icon
	HookEvent(ME_MSG_ICONPRESSED, SVC_IconPressed);

	memset(&OTRButton, 0, sizeof(OTRButton));
	OTRButton.pszModuleName = MODULENAME;
	OTRButton.dwDefPos = 200;
	OTRButton.bbbFlags = BBBF_ISRSIDEBUTTON | BBBF_CANBEHIDDEN | BBBF_ISIMBUTTON;
	OTRButton.pwszTooltip = TranslateT(LANG_OTR_TOOLTIP);
	OTRButton.hIcon = IcoLib_GetIconHandle(ICON_NOT_PRIVATE);

	HookEvent(ME_MSG_BUTTONPRESSED, SVC_ButtonsBarPressed);

	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, SVC_ButtonsBarLoaded);
}

void DeinitSRMM()
{
	IcoLib_Release(ICON_NOT_PRIVATE, 0);
	IcoLib_Release(ICON_FINISHED, 0);
	IcoLib_Release(ICON_PRIVATE, 0);
	IcoLib_Release(ICON_UNVERIFIED, 0);
	hIconNotSecure = hIconFinished = hIconPrivate = hIconUnverified =0;

	UninitMirOTRMenu();
}

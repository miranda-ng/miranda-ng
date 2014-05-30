#include "stdafx.h"

HICON hIconNotSecure, hIconFinished, hIconPrivate, hIconUnverified;
BBButton OTRButton;

int SVC_IconPressed(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if(sicd->cbSize < (int)sizeof(StatusIconClickData))
		return 0;

	if(strcmp(sicd->szModule, MODULENAME) == 0) {
		char *proto = GetContactProto(hContact);
		if(proto && db_get_b(hContact, proto, "ChatRoom", 0))
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

	StatusIconData sid = { sizeof(sid) }, sid2 = { sizeof(sid) };
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
			button.ptszTooltip = TranslateT(LANG_STATUS_FINISHED);
			button.hIcon = GetIconHandle(ICON_FINISHED);
			break;
		case TRUST_UNVERIFIED:
			sid2.flags = MBF_DISABLED;
			button.ptszTooltip = TranslateT(LANG_STATUS_UNVERIFIED);
			button.hIcon = GetIconHandle(ICON_UNVERIFIED);
			break;
		case TRUST_PRIVATE:
			sid2.flags = 0;
			button.ptszTooltip = TranslateT(LANG_STATUS_PRIVATE);
			button.hIcon = GetIconHandle(ICON_PRIVATE);
			break;
		default:
			sid.flags = MBF_DISABLED;
			button.ptszTooltip = TranslateT(LANG_STATUS_DISABLED);
			button.hIcon = GetIconHandle(ICON_NOT_PRIVATE);
			break;
		}
		button.bbbFlags = 0;
	}
	else button.bbbFlags = BBSF_HIDDEN;

	Srmm_ModifyIcon(hContact, &sid);
	Srmm_ModifyIcon(hContact, &sid2);

	if (options.bHaveButtonsBar) CallService(MS_BB_SETBUTTONSTATE, hContact, (LPARAM)&button);
	db_set_dw(hContact, MODULENAME, "TrustLevel", level);

	if (!chat_room) {
		MCONTACT hMeta = db_mc_getMeta(hContact);
		MCONTACT hMostOnline = db_mc_getMostOnline(hMeta);
		if(hMeta && hContact == hMostOnline)
			SetEncryptionStatus(hMeta, level);
		else if(hMeta) {
			/* in case the new most online contact has changed
			(e.g. when the otr subcontact goes offline) */
			ConnContext *context = otrl_context_find_miranda(otr_user_state, hMostOnline);
			TrustLevel encrypted = otr_context_get_trust(context);
			SetEncryptionStatus(hMeta, encrypted);
		}
	}
}

int SVC_ButtonsBarLoaded(WPARAM, LPARAM)
{
	CallService(MS_BB_ADDBUTTON, 0, (LPARAM)&OTRButton);
	return 0;
}

int SVC_ButtonsBarPressed(WPARAM w, LPARAM l)
{
	CustomButtonClickData* cbcd = (CustomButtonClickData *)l;
	if (cbcd->cbSize == (int)sizeof(CustomButtonClickData) && cbcd->dwButtonId == 0 && strcmp(cbcd->pszModule, MODULENAME)==0) {
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

	hIconNotSecure = LoadIcon(ICON_NOT_PRIVATE, 0);
	hIconFinished = LoadIcon(ICON_FINISHED, 0);
	hIconPrivate = LoadIcon(ICON_PRIVATE, 0);
	hIconUnverified = LoadIcon(ICON_UNVERIFIED, 0);

	StatusIconData sid = { sizeof(sid) };
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

	if (options.bHaveButtonsBar) {
		ZeroMemory(&OTRButton, sizeof(OTRButton));
		OTRButton.cbSize = sizeof(OTRButton);
		OTRButton.dwButtonID = 0;
		OTRButton.pszModuleName = MODULENAME;
		OTRButton.dwDefPos = 200;
		OTRButton.bbbFlags = BBBF_ISRSIDEBUTTON|BBBF_CANBEHIDDEN|BBBF_ISIMBUTTON;
		OTRButton.ptszTooltip = TranslateT(LANG_OTR_TOOLTIP);
		OTRButton.hIcon = GetIconHandle(ICON_NOT_PRIVATE);
		HookEvent(ME_MSG_TOOLBARLOADED, SVC_ButtonsBarLoaded);
		HookEvent(ME_MSG_BUTTONPRESSED, SVC_ButtonsBarPressed);
	}
}
void DeinitSRMM()
{
	ReleaseIcon(ICON_NOT_PRIVATE, 0);
	ReleaseIcon(ICON_FINISHED, 0);
	ReleaseIcon(ICON_PRIVATE, 0);
	ReleaseIcon(ICON_UNVERIFIED, 0);
	hIconNotSecure = hIconFinished = hIconPrivate = hIconUnverified =0;

	UninitMirOTRMenu();
}
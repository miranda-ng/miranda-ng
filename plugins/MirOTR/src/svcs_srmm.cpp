#include "stdafx.h"

HICON hIconNotSecure, hIconFinished, hIconPrivate, hIconUnverified;
BBButton OTRButton;

int SVC_IconPressed(WPARAM hContact, LPARAM lParam)
{
	StatusIconClickData *sicd = (StatusIconClickData *)lParam;

	if (mir_strcmp(sicd->szModule, MODULENAME) == 0) {
		if (!Contact::IsGroupChat(hContact))
			ShowOTRMenu(hContact, sicd->clickLocation);
	}

	return 0;
}

// set SRMM icon status, if applicable
void SetEncryptionStatus(MCONTACT hContact, TrustLevel level)
{
	bool chat_room = Contact::IsGroupChat(hContact);

	BBButton button = OTRButton;
	int flags1 = MBF_HIDDEN, flags2 = MBF_HIDDEN;

	if (!chat_room) {
		switch (level) {
		case TRUST_FINISHED:
			flags1 = 0;
			button.pwszTooltip = TranslateW(LANG_STATUS_FINISHED);
			button.hIcon = iconList[ICON_FINISHED].hIcolib;
			break;
		case TRUST_UNVERIFIED:
			flags2 = MBF_DISABLED;
			button.pwszTooltip = TranslateW(LANG_STATUS_UNVERIFIED);
			button.hIcon = iconList[ICON_UNVERIFIED].hIcolib;
			break;
		case TRUST_PRIVATE:
			flags2 = 0;
			button.pwszTooltip = TranslateW(LANG_STATUS_PRIVATE);
			button.hIcon = iconList[ICON_PRIVATE].hIcolib;
			break;
		default:
			flags1 = MBF_DISABLED;
			button.pwszTooltip = TranslateW(LANG_STATUS_DISABLED);
			button.hIcon = iconList[ICON_NOT_PRIVATE].hIcolib;
			break;
		}
		button.bbbFlags = 0;
	}
	else button.bbbFlags = BBSF_HIDDEN;

	Srmm_SetIconFlags(hContact, MODULENAME, 0, flags1);
	Srmm_SetIconFlags(hContact, MODULENAME, 1, flags2);
	Srmm_SetButtonState(hContact, &button);

	g_plugin.setDword(hContact, "TrustLevel", level);

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
	Srmm_AddButton(&OTRButton, &g_plugin);
	return 0;
}

int SVC_ButtonsBarPressed(WPARAM w, LPARAM l)
{
	CustomButtonClickData *cbcd = (CustomButtonClickData *)l;
	
	if (cbcd->dwButtonId == 0 && !mir_strcmp(cbcd->pszModule, MODULENAME)) {
		MCONTACT hContact = (MCONTACT)w;
		if (!Contact::IsGroupChat(hContact))
			ShowOTRMenu(hContact, cbcd->pt);

	}
	return 0;
}

void InitSRMM()
{
	// add icon to srmm status icons
	InitMirOTRMenu();

	hIconNotSecure = IcoLib_GetIconByHandle(iconList[ICON_NOT_PRIVATE].hIcolib, 0);
	hIconFinished = IcoLib_GetIconByHandle(iconList[ICON_FINISHED].hIcolib, 0);
	hIconPrivate = IcoLib_GetIconByHandle(iconList[ICON_PRIVATE].hIcolib, 0);
	hIconUnverified = IcoLib_GetIconByHandle(iconList[ICON_UNVERIFIED].hIcolib, 0);

	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.hIcon = hIconFinished;
	sid.hIconDisabled = hIconNotSecure;
	sid.flags = MBF_DISABLED | MBF_HIDDEN;
	sid.szTooltip.a = LANG_OTR_TOOLTIP;
	Srmm_AddIcon(&sid, &g_plugin);

	sid.dwId = 1;
	sid.hIcon = hIconPrivate;
	sid.hIconDisabled = hIconUnverified;
	Srmm_AddIcon(&sid, &g_plugin);
		
	// hook the window events so that we can can change the status of the icon
	HookEvent(ME_MSG_ICONPRESSED, SVC_IconPressed);

	memset(&OTRButton, 0, sizeof(OTRButton));
	OTRButton.pszModuleName = MODULENAME;
	OTRButton.dwDefPos = 200;
	OTRButton.bbbFlags = BBBF_ISRSIDEBUTTON | BBBF_CANBEHIDDEN | BBBF_ISIMBUTTON;
	OTRButton.pwszTooltip = TranslateT(LANG_OTR_TOOLTIP);
	OTRButton.hIcon = iconList[ICON_NOT_PRIVATE].hIcolib;

	HookEvent(ME_MSG_BUTTONPRESSED, SVC_ButtonsBarPressed);

	HookTemporaryEvent(ME_MSG_TOOLBARLOADED, SVC_ButtonsBarLoaded);
}

void DeinitSRMM()
{
	IcoLib_ReleaseIcon(hIconNotSecure);
	IcoLib_ReleaseIcon(hIconFinished);
	IcoLib_ReleaseIcon(hIconPrivate);
	IcoLib_ReleaseIcon(hIconUnverified);
	hIconNotSecure = hIconFinished = hIconPrivate = hIconUnverified =nullptr;

	UninitMirOTRMenu();
}

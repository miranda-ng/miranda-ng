#include "commonheaders.h"

int __cdecl onWindowEvent(WPARAM, LPARAM lParam)
{
	MessageWindowEventData *mwd = (MessageWindowEventData *)lParam;
	if (mwd->uType == MSG_WINDOW_EVT_OPEN || mwd->uType == MSG_WINDOW_EVT_OPENING)
		ShowStatusIcon(mwd->hContact);

	return 0;
}

int __cdecl onIconPressed(WPARAM wParam, LPARAM lParam)
{
	HANDLE hContact = (HANDLE)wParam;
	if (isProtoMetaContacts(hContact))
		hContact = getMostOnline(hContact); // возьмем тот, через который пойдет сообщение

	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (strcmp(sicd->szModule, MODULENAME) != 0 || !isSecureProtocol(hContact))
		return 0; // not our event

	if (!isContactPGP(hContact) && !isContactGPG(hContact) && !isChatRoom(hContact)) {
		if (isContactSecured(hContact) & SECURED)
			Service_DisableIM(wParam,0);
		else
			Service_CreateIM(wParam,0);
	}

	return 0;
}

void InitSRMMIcons()
{
	// add icon to srmm status icons
	if (ServiceExists(MS_MSG_ADDICON)) {
		StatusIconData sid = { sizeof(sid) };
		sid.szModule = MODULENAME;
		sid.flags = MBF_DISABLED|MBF_HIDDEN;
		// Native
		sid.dwId = MODE_NATIVE;
		sid.hIcon = mode2icon(MODE_NATIVE|SECURED,3);
		sid.hIconDisabled = mode2icon(MODE_NATIVE,3);
		sid.szTooltip = Translate("SecureIM [Native]");
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
		// PGP
		sid.dwId = MODE_PGP;
		sid.hIcon = mode2icon(MODE_PGP|SECURED,3);
		sid.hIconDisabled = mode2icon(MODE_PGP,3);
		sid.szTooltip = Translate("SecureIM [PGP]");
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
		// GPG
		sid.dwId = MODE_GPG;
		sid.hIcon = mode2icon(MODE_GPG|SECURED,3);
		sid.hIconDisabled = mode2icon(MODE_GPG,3);
		sid.szTooltip = Translate("SecureIM [GPG]");
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);
		// RSAAES
		sid.dwId = MODE_RSAAES;
		sid.hIcon = mode2icon(MODE_RSAAES|SECURED,3);
		sid.hIconDisabled = mode2icon(MODE_RSAAES,3);
		sid.szTooltip = Translate("SecureIM [RSA/AES]");
		CallService(MS_MSG_ADDICON, 0, (LPARAM)&sid);

		// hook the window events so that we can can change the status of the icon
		HookEvent(ME_MSG_WINDOWEVENT, onWindowEvent);
		HookEvent(ME_MSG_ICONPRESSED, onIconPressed);
	}
}



void DeinitSRMMIcons()
{
	// remove srmm status icons
	if (ServiceExists(MS_MSG_REMOVEICON)) {
		StatusIconData sid = { sizeof(sid) };
		sid.szModule = MODULENAME;
		// Native
		sid.dwId = MODE_NATIVE;
		CallService(MS_MSG_REMOVEICON, 0, (LPARAM)&sid);
		// PGP
		sid.dwId = MODE_PGP;
		CallService(MS_MSG_REMOVEICON, 0, (LPARAM)&sid);
		// GPG
		sid.dwId = MODE_GPG;
		CallService(MS_MSG_REMOVEICON, 0, (LPARAM)&sid);
		// RSAAES
		sid.dwId = MODE_RSAAES;
		CallService(MS_MSG_REMOVEICON, 0, (LPARAM)&sid);
	}
}

// EOF

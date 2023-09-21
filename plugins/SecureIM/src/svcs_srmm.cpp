#include "commonheaders.h"

int __cdecl onWindowEvent(WPARAM uType, LPARAM lParam)
{
	auto *pDlg = (CSrmmBaseDialog *)lParam;

	if (uType == MSG_WINDOW_EVT_OPEN || uType == MSG_WINDOW_EVT_OPENING)
		ShowStatusIcon(pDlg->m_hContact);
	return 0;
}

int __cdecl onIconPressed(WPARAM hContact, LPARAM lParam)
{
	if (db_mc_isMeta(hContact))
		hContact = db_mc_getMostOnline(hContact); // возьмем тот, через который пойдет сообщение

	StatusIconClickData *sicd = (StatusIconClickData *)lParam;
	if (mir_strcmp(sicd->szModule, MODULENAME) != 0 || !isSecureProtocol(hContact))
		return 0; // not our event

	if (!isContactPGP(hContact) && !isContactGPG(hContact) && !isChatRoom(hContact)) {
		if (isContactSecured(hContact) & SECURED)
			Service_DisableIM(hContact, 0);
		else
			Service_CreateIM(hContact, 0);
	}

	return 0;
}

void InitSRMMIcons()
{
	// add icon to srmm status icons
	StatusIconData sid = {};
	sid.szModule = MODULENAME;
	sid.flags = MBF_DISABLED | MBF_HIDDEN;

	// Native
	sid.dwId = MODE_NATIVE;
	sid.hIcon = mode2icon(MODE_NATIVE | SECURED, 3);
	sid.hIconDisabled = mode2icon(MODE_NATIVE, 3);
	sid.szTooltip.a = LPGEN("SecureIM [Native]");
	Srmm_AddIcon(&sid, &g_plugin);

	// PGP
	sid.dwId = MODE_PGP;
	sid.hIcon = mode2icon(MODE_PGP | SECURED, 3);
	sid.hIconDisabled = mode2icon(MODE_PGP, 3);
	sid.szTooltip.a = LPGEN("SecureIM [PGP]");
	Srmm_AddIcon(&sid, &g_plugin);

	// GPG
	sid.dwId = MODE_GPG;
	sid.hIcon = mode2icon(MODE_GPG | SECURED, 3);
	sid.hIconDisabled = mode2icon(MODE_GPG, 3);
	sid.szTooltip.a = LPGEN("SecureIM [GPG]");
	Srmm_AddIcon(&sid, &g_plugin);

	// RSAAES
	sid.dwId = MODE_RSAAES;
	sid.hIcon = mode2icon(MODE_RSAAES | SECURED, 3);
	sid.hIconDisabled = mode2icon(MODE_RSAAES, 3);
	sid.szTooltip.a = LPGEN("SecureIM [RSA/AES]");
	Srmm_AddIcon(&sid, &g_plugin);

	// hook the window events so that we can can change the status of the icon
	HookEvent(ME_MSG_WINDOWEVENT, onWindowEvent);
	HookEvent(ME_MSG_ICONPRESSED, onIconPressed);
}

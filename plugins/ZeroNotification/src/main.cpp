/*
A small Miranda plugin, by bidyut, updated by Maat.
Original plugin idea (NoSound) by Anders Nilsson.

Miranda can be found here:
https://miranda-ng.org/
*/

#include "stdafx.h"

CMPlugin g_plugin;
HGENMENU noSoundMenu;

int OptionsInitialize(WPARAM, LPARAM);

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {47D489D3-310D-4EF6-BD05-699FFFD5A4AA}
	{ 0x47d489d3, 0x310d, 0x4ef6, { 0xbd, 0x5, 0x69, 0x9f, 0xff, 0xd5, 0xa4, 0xaa } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// Update the name on the menu

static void UpdateMenuItem()
{
	Menu_ModifyItem(noSoundMenu, db_get_b(0, "Skin", "UseSound", 1) ? DISABLE_SOUND : ENABLE_SOUND);
}

// Called when the sound setting in the database is changed
static int SoundSettingChanged(WPARAM, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (!strcmp(cws->szModule, "Skin") && !strcmp(cws->szSetting, "UseSound"))
		UpdateMenuItem();
	return 0;
}

int SetNotify(const long status)
{
	db_set_b(0, "Skin", "UseSound", (uint8_t)!(g_plugin.getDword("NoSound", DEFAULT_NOSOUND) & status));
	db_set_b(0, "CList", "DisableTrayFlash", (uint8_t)(g_plugin.getDword("NoBlink", DEFAULT_NOBLINK) & status));
	db_set_b(0, "CList", "NoIconBlink", (uint8_t)(g_plugin.getDword("NoCLCBlink", DEFAULT_NOCLCBLINK) & status));

	UpdateMenuItem();
	return 0;
}

// Called whenever a change in status is detected
static int ProtoAck(WPARAM, LPARAM lParam)
{
	// quit if not status event
	ACKDATA *ack = (ACKDATA*)lParam;
	if (ack->type == ACKTYPE_STATUS && ack->result == ACKRESULT_SUCCESS) {
		long status = 0;
		for (auto &pa : Accounts())
			status = status | Proto_Status2Flag(Proto_GetStatus(pa->szModuleName));

		SetNotify(status);
	}

	return 0;
}

static INT_PTR NoSoundMenuCommand(WPARAM, LPARAM)
{
	bool useSound = db_get_b(0, "Skin", "UseSound", true);
	db_set_b(0, "Skin", "UseSound", !useSound);

	return 0;
}

int CMPlugin::Load()
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x6bd635eb, 0xc4bb, 0x413b, 0xb9, 0x3, 0x81, 0x6d, 0x8f, 0xf1, 0x9b, 0xb0);
	mi.position = -0x7FFFFFFF;
	mi.flags = CMIF_UNICODE;
	mi.pszService = MODULENAME "/MenuCommand";
	noSoundMenu = Menu_AddMainMenuItem(&mi);
	CreateServiceFunction(mi.pszService, NoSoundMenuCommand);

	int bHideMenu = g_plugin.getByte("HideMenu", 100);
	if (bHideMenu != 100) {
		Menu_SetVisible(noSoundMenu, !bHideMenu);
		g_plugin.delSetting("HideMenu");
	}

	UpdateMenuItem();

	HookEvent(ME_PROTO_ACK, ProtoAck);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SoundSettingChanged);
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);

	return 0;
}

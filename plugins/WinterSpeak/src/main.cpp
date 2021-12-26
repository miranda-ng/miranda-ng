#include "stdafx.h"

CMPlugin g_plugin;

SpeakConfig   *g_speak_config = nullptr;
SpeakAnnounce *g_speak_announce = nullptr;
HANDLE         g_event_status_change;
HANDLE         g_event_message_added;
HANDLE         g_protocol_ack;

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {81E189DC-C251-45F6-9EDF-A0F3A05C4248}
	{ 0x81e189dc, 0xc251, 0x45f6, { 0x9e, 0xdf, 0xa0, 0xf3, 0xa0, 0x5c, 0x42, 0x48 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("speak_config", pluginInfoEx)
{
}

//-----------------------------------------------------------------------------
// Description : External hook
//-----------------------------------------------------------------------------
INT_PTR status(WPARAM wParam, LPARAM lParam)
{
	return g_speak_config->status(reinterpret_cast<wchar_t *>(lParam), MCONTACT(wParam));
}

INT_PTR message(WPARAM wParam, LPARAM lParam)
{
	return g_speak_config->message(reinterpret_cast<wchar_t *>(lParam), MCONTACT(wParam));
}

//-----------------------------------------------------------------------------
// Description : an status change event occured
//-----------------------------------------------------------------------------
int eventStatusChange(WPARAM wParam, LPARAM lParam)
{
	g_speak_announce->statusChange(reinterpret_cast<DBCONTACTWRITESETTING *>(lParam), MCONTACT(wParam));
	return 0;
}

//-----------------------------------------------------------------------------
// Description : a message event occured
//-----------------------------------------------------------------------------
int eventMessageAdded(WPARAM wParam, LPARAM lParam)
{
	g_speak_announce->incomingEvent(wParam, lParam);
	return 0;
}

//-----------------------------------------------------------------------------
// Description : a messaging protocol changed state
//-----------------------------------------------------------------------------
int protocolAck(WPARAM, LPARAM lParam)
{
	g_speak_announce->protocolAck(reinterpret_cast<ACKDATA *>(lParam));

	return 0;
}

int dialogOptionsInitialise(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.szGroup.w = LPGENW("Speak");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;

	if (g_speak_config) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_CONFIG);
		odp.szTitle.w = LPGENW("Engine/Voice");
		odp.pfnDlgProc = DialogConfigEngine::process;
		g_plugin.addOptions(wParam, &odp);

		odp.pszTemplate = MAKEINTRESOURCEA(IDD_ACTIVEMODES);
		odp.szTitle.w = LPGENW("Active Modes");
		odp.pfnDlgProc = DialogConfigActive::process;
		g_plugin.addOptions(wParam, &odp);
	}

	if (g_speak_announce) {
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_ANNOUNCE);
		odp.szTitle.w = LPGENW("Announce");
		odp.pfnDlgProc = AnnounceDialog::process;
		g_plugin.addOptions(wParam, &odp);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	if (!g_speak_config) {
		g_speak_config = new SpeakConfig(g_plugin.getInst());

		// expose to allow miranda + plugins to access my speak routines
		CreateServiceFunction(MS_SPEAK_STATUS, status);
		CreateServiceFunction(MS_SPEAK_MESSAGE, message);
	}

	if (!g_speak_announce) {
		g_speak_announce = new SpeakAnnounce(g_plugin.getInst());

		// tap into contact setting change event
		g_event_status_change = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, eventStatusChange);

		// a new message event has occured
		g_event_message_added = HookEvent(ME_DB_EVENT_ADDED, eventMessageAdded);

		// a messaging protocol changed state
		g_protocol_ack = HookEvent(ME_PROTO_ACK, protocolAck);

	}

	// a option dialog box request has occured
	HookEvent(ME_OPT_INITIALISE, dialogOptionsInitialise);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	if (g_speak_config) {
		delete g_speak_config;
		g_speak_config = nullptr;
	}

	if (g_speak_announce) {
		UnhookEvent(g_event_status_change);
		UnhookEvent(g_event_message_added);
		UnhookEvent(g_protocol_ack);

		delete g_speak_announce;
		g_speak_announce = nullptr;
	}

	return 0;
}

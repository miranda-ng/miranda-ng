#include "stdafx.h"

CMPlugin g_plugin;

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
	// D3E24A92-E2C3-440F-BC87-00 2A B8 22 ABFB
	{0xD3E24A92, 0xE2C3, 0x440F, {0xbc, 0x87, 0x00, 0x2A, 0xb8, 0x22, 0xab, 0xf8}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>("Jingle", pluginInfoEx)
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load (hook ModulesLoaded)

static INT_PTR FakeService(WPARAM, LPARAM)
{
	g_assertion_message(0, 0, 0, 0, 0);
	g_object_unref(0);
	gst_bin_add(0, 0);
	gst_rtp_header_extension_set_id(0, 0);
	gst_sdp_message_as_text(0);
	gst_webrtc_session_description_free(0);
	return 0;
}

int CMPlugin::Load()
{
	CreateServiceFunction("Jingle/StartSession", &FakeService);
	return 0;
}

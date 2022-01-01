/*

WhatsAppWeb plugin for Miranda NG
Copyright © 2019-22 George Hazan

*/

#include "stdafx.h"
#include "version.h"

CMPlugin g_plugin;

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	UNICODE_AWARE, //not transient
	// {008B9CE1-154B-44E4-9823-97C1AAB00C3C}
	{ 0x8b9ce1, 0x154b, 0x44e4, { 0x98, 0x23, 0x97, 0xc1, 0xaa, 0xb0, 0xc, 0x3c }}
};

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

/////////////////////////////////////////////////////////////////////////////////////////

CMPlugin::CMPlugin() :
	ACCPROTOPLUGIN<WhatsAppProto>(MODULENAME, pluginInfo)
{
	SetUniqueId(DBKEY_ID);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load

static int hmac_sha256_init(void **hmac_context, const uint8_t *key, size_t key_len, void *)
{
	HMAC_CTX *ctx = HMAC_CTX_new();
	*hmac_context = ctx;
	HMAC_Init(ctx, key, (int)key_len, EVP_sha256());
	return 0;
}

int hmac_sha256_update(void *hmac_context, const uint8_t *data, size_t data_len, void *)
{
	return HMAC_Update((HMAC_CTX *)hmac_context, data, data_len);
}

int hmac_sha256_final(void *hmac_context, signal_buffer **output, void *)
{
	BYTE data[200];
	unsigned len = 0;
	if (!HMAC_Final((HMAC_CTX *)hmac_context, data, &len))
		return 1;

	*output = signal_buffer_create(data, len);
	return 0;
}

void hmac_sha256_cleanup(void *hmac_context, void *)
{
	HMAC_CTX_free((HMAC_CTX *)hmac_context);
}

static int random_func(uint8_t *pData, size_t size, void *)
{
	Utils_GetRandom(pData, size);
	return 0;
}

int CMPlugin::Load()
{
	// InitIcons();
	// InitContactMenus();

	NETLIBUSER nlu = {};
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_UNICODE;
	nlu.szSettingsModule = "WhatsApp";
	nlu.szDescriptiveName.w = TranslateT("WhatsApp HTTP connection");
	hAvatarUser = Netlib_RegisterUser(&nlu);

	//////////////////////////////////////////////////////////////////////////////////////
	signal_context_create(&pCtx, nullptr);

	signal_crypto_provider prov;
	memset(&prov, 0xFF, sizeof(prov));
	prov.hmac_sha256_init_func = hmac_sha256_init;
	prov.hmac_sha256_final_func = hmac_sha256_final;
	prov.hmac_sha256_update_func = hmac_sha256_update;
	prov.hmac_sha256_cleanup_func = hmac_sha256_cleanup;
	prov.random_func = random_func;
	signal_context_set_crypto_provider(pCtx, &prov);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

int CMPlugin::Unload()
{
	Netlib_CloseHandle(hAvatarConn);
	Netlib_CloseHandle(hAvatarUser);

	signal_context_destroy(pCtx);
	return 0;
}

#include "commonheaders.h"

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
	// {1B2A39E5-E2F6-494D-958D-1808FD110DD5}
	{ 0x1B2A39E5, 0xE2F6, 0x494D, { 0x95, 0x8D, 0x18, 0x08, 0xFD, 0x11, 0x0D, 0xD5 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// basic events: onModuleLoad, onModulesLoad, onShutdown

LPCWSTR sim311[] = {
	LPGENW("SecureIM mode (Native)"),
	LPGENW("SecureIM mode (PGP)"),
	LPGENW("SecureIM mode (GPG)"),
	LPGENW("SecureIM mode (RSA/AES)"),
	LPGENW("SecureIM mode (RSA)")
};

LPCWSTR sim312[] = {
	LPGENW("SecureIM status (disabled)"),
	LPGENW("SecureIM status (enabled)"),
	LPGENW("SecureIM status (always try)")
};

LPCSTR sim231[] = { LPGEN("Native"), "PGP", "GPG", "RSA/AES", "RSA" };
LPCSTR sim232[] = { LPGEN("Disabled"), LPGEN("Enabled"), LPGEN("Always try") };

static HGENMENU MyAddMenuItem(LPCWSTR name, int pos, LPCSTR szUid, HICON hicon, LPCSTR service, int flags = 0, WPARAM wParam = 0)
{
	CMenuItem mi(&g_plugin);
	mi.flags = flags | CMIF_HIDDEN | CMIF_UNICODE;
	mi.position = pos;
	mi.hIcolibItem = hicon;
	mi.name.w = (wchar_t*)name;
	mi.pszService = (char*)service;
	HGENMENU res = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(res, MCI_OPT_UID, szUid);
	return res;
}

static HGENMENU MyAddSubItem(HGENMENU hRoot, LPCSTR name, int pos, int poppos, LPCSTR service, WPARAM wParam = 0)
{
	CMenuItem mi(&g_plugin);
	mi.flags = CMIF_HIDDEN | CMIF_SYSTEM;
	mi.position = pos;
	mi.name.a = (char*)name;
	mi.root = hRoot;
	mi.pszService = (char*)service;
	HGENMENU res = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(res, MCI_OPT_EXECPARAM, poppos);
	return res;
}

static int onModulesLoaded(WPARAM, LPARAM)
{
	InitNetlib();

	g_hFolders = FoldersRegisterCustomPath(LPGEN("SecureIM"), LPGEN("Icons"), MIRANDA_PATH "\\icons");

	InitIcons();
	GetFlags();

	// RSA/AES
	Sent_NetLog("rsa_init");
	rsa_init(&mir_exp, &imp);

	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;

	if (db_get(0, MODULENAME, "rsa_priv", &dbv) == 0) {
		mir_exp->rsa_set_keypair(CPP_MODE_RSA_4096, dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
		rsa_4096 = 1;
	}
	else if (db_get(0, MODULENAME, "rsa_priv_4096", &dbv) == 0) {
		mir_exp->rsa_set_keypair(CPP_MODE_RSA_4096 | CPP_MODE_RSA_BER, dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);

		char priv_key[4096]; int priv_len;
		char pub_key[4096]; int pub_len;

		mir_exp->rsa_get_keypair(CPP_MODE_RSA_4096, (uint8_t*)&priv_key, &priv_len, (uint8_t*)&pub_key, &pub_len);
		db_set_blob(0, MODULENAME, "rsa_priv", priv_key, priv_len);
		db_set_blob(0, MODULENAME, "rsa_pub", &pub_key, pub_len);
		g_plugin.delSetting("rsa_priv_2048");
		g_plugin.delSetting("rsa_pub_2048");
		rsa_4096 = 1;
	}

	if (!rsa_4096)
		mir_forkthread(sttGenerateRSA);

	mir_exp->rsa_set_timeout(g_plugin.getWord("ket", 10));

	Sent_NetLog("pgp_init");

	bPGP = g_plugin.getByte("pgp", 0);
	if (bPGP) { //PGP
		bPGPloaded = pgp_init();
		bUseKeyrings = g_plugin.getByte("ukr", 1);
		LPSTR priv = g_plugin.getStringA("pgpPrivKey");
		if (priv) {
			bPGPprivkey = true;
			if (bPGPloaded)
				pgp_set_priv_key(priv);
			mir_free(priv);
		}
		if (bPGPloaded && bUseKeyrings) {
			char PubRingPath[MAX_PATH], SecRingPath[MAX_PATH];
			PubRingPath[0] = '\0'; SecRingPath[0] = '\0';
			if (pgp_get_version() < 0x02000000) { // 6xx
				bPGPkeyrings = pgp_open_keyrings(PubRingPath, SecRingPath);
			}
			else {
				LPSTR tmp = g_plugin.getStringA("pgpPubRing");
				if (tmp) {
					strncpy(PubRingPath, tmp, sizeof(PubRingPath));
					mir_free(tmp);
				}
				if (tmp = g_plugin.getStringA("pgpSecRing")) {
					strncpy(SecRingPath, tmp, sizeof(SecRingPath));
					mir_free(tmp);
				}
				if (PubRingPath[0] && SecRingPath[0]) {
					bPGPkeyrings = pgp_open_keyrings(PubRingPath, SecRingPath);
					if (bPGPkeyrings) {
						g_plugin.setString("pgpPubRing", PubRingPath);
						g_plugin.setString("pgpSecRing", SecRingPath);
					}
					else {
						g_plugin.delSetting("pgpPubRing");
						g_plugin.delSetting("pgpSecRing");
					}
				}
			}
		}
	}

	Sent_NetLog("gpg_init");

	bGPG = g_plugin.getByte("gpg", 0);
	if (bGPG) { //GPG
		bGPGloaded = gpg_init();

		char gpgexec[MAX_PATH], gpghome[MAX_PATH];
		gpgexec[0] = '\0'; gpghome[0] = '\0';

		LPSTR tmp = g_plugin.getStringA("gpgExec");
		if (tmp) {
			strncpy(gpgexec, tmp, sizeof(gpgexec)-1);
			mir_free(tmp);
		}
		if (tmp = g_plugin.getStringA("gpgHome")) {
			strncpy(gpghome, tmp, sizeof(gpghome)-1);
			mir_free(tmp);
		}

		if (g_plugin.getByte("gpgLogFlag", 0)) {
			if (tmp = g_plugin.getStringA("gpgLog")) {
				gpg_set_log(tmp);
				mir_free(tmp);
			}
		}

		if (g_plugin.getByte("gpgTmpFlag", 0)) {
			if (tmp = g_plugin.getStringA("gpgTmp")) {
				gpg_set_tmp(tmp);
				mir_free(tmp);
			}
		}

		bGPGkeyrings = gpg_open_keyrings(gpgexec, gpghome);
		if (bGPGkeyrings) {
			g_plugin.setString("gpgExec", gpgexec);
			g_plugin.setString("gpgHome", gpghome);
		}
		else {
			g_plugin.delSetting("gpgExec");
			g_plugin.delSetting("gpgHome");
		}

		bSavePass = g_plugin.getByte("gpgSaveFlag", 0);
		if (bSavePass) {
			if (tmp = g_plugin.getStringA("gpgSave")) {
				gpg_set_passphrases(tmp);
				mir_free(tmp);
			}
		}
	}

	Sent_NetLog("loadContactList");
	loadContactList();

	// add new skin sound
	g_plugin.addSound("IncomingSecureMessage", LPGENW("SecureIM"), LPGENW("Incoming Secure Message"), L"Sounds\\iSecureMessage.wav");
	g_plugin.addSound("OutgoingSecureMessage", LPGENW("SecureIM"), LPGENW("Outgoing Secure Message"), L"Sounds\\oSecureMessage.wav");

	// init extra icons
	for (int i = 0; i < _countof(g_IEC); i++)
		g_IEC[i] = (HANDLE)-1;

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onRebuildContactMenu);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);
	HookEvent(ME_DB_CONTACT_ADDED, onContactAdded);
	HookEvent(ME_DB_CONTACT_DELETED, onContactDeleted);

	// hook message transport
	CreateProtoServiceFunction(MODULENAME, PSR_MESSAGE, onRecvMsg);
	CreateProtoServiceFunction(MODULENAME, PSS_MESSAGE, onSendMsg);

	// create a menu item for creating a secure im connection to the user.
	g_hMenu[0] = MyAddMenuItem(LPGENW("Create SecureIM connection"), 110000, "5A8C2F35-4699-43A4-A820-516DEB83FCA1", g_hICO[ICO_CM_EST], MODULENAME"/SIM_EST", CMIF_NOTOFFLINE);
	g_hMenu[1] = MyAddMenuItem(LPGENW("Disable SecureIM connection"), 110001, "0B092254-DA91-42D6-A89D-365981BB3D91", g_hICO[ICO_CM_DIS], MODULENAME"/SIM_DIS", CMIF_NOTOFFLINE);

	g_hMenu[2] = MyAddMenuItem(sim312[0], 110002, "635576BB-A927-4F64-B205-DD464F57CC99", nullptr, nullptr);
	g_hMenu[3] = MyAddSubItem(g_hMenu[2], sim232[0], 110003, 110002, MODULENAME"/SIM_ST_DIS");
	g_hMenu[4] = MyAddSubItem(g_hMenu[2], sim232[1], 110004, 110002, MODULENAME"/SIM_ST_ENA");
	g_hMenu[5] = MyAddSubItem(g_hMenu[2], sim232[2], 110005, 110002, MODULENAME"/SIM_ST_TRY");

	if (bPGPloaded) {
		g_hMenu[6] = MyAddMenuItem(LPGENW("Load PGP Key"), 110006, "33829541-85B9-499E-9605-4DDADE1A4B33", mode2icon(MODE_PGP | SECURED, 2), MODULENAME"/PGP_SET", 0);
		g_hMenu[7] = MyAddMenuItem(LPGENW("Unload PGP Key"), 110007, "25530E70-8349-419D-9F4F-FA748E485E2B", mode2icon(MODE_PGP, 2), MODULENAME"/PGP_DEL", 0);
	}

	if (bGPGloaded) {
		g_hMenu[8] = MyAddMenuItem(LPGENW("Load GPG Key"), 110008, "D8BD7B70-3E6C-4A09-9612-4E4E2FCBBB8A", mode2icon(MODE_GPG | SECURED, 2), MODULENAME"/GPG_SET", 0);
		g_hMenu[9] = MyAddMenuItem(LPGENW("Unload GPG Key"), 110009, "5C60AD6F-6B1B-4758-BB68-C008168BF32B", mode2icon(MODE_GPG, 2), MODULENAME"/GPG_DEL", 0);
	}

	g_hMenu[10] = MyAddMenuItem(sim311[0], 110010, "D56DD118-863B-4069-9A6A-C0057BA99CC6", nullptr, nullptr);
	g_hMenu[11] = MyAddSubItem(g_hMenu[10], sim231[0], 110011, 110010, MODULENAME"/MODE_NAT");
	g_hMenu[12] = MyAddSubItem(g_hMenu[10], sim231[1], 110012, 110010, MODULENAME"/MODE_PGP");
	g_hMenu[13] = MyAddSubItem(g_hMenu[10], sim231[2], 110013, 110010, MODULENAME"/MODE_GPG");
	g_hMenu[14] = MyAddSubItem(g_hMenu[10], sim231[3], 110014, 110010, MODULENAME"/MODE_RSA");

	InitSRMMIcons();
	return 0;
}

static int onShutdown(WPARAM, LPARAM)
{
	if (bSavePass) {
		LPSTR tmp = gpg_get_passphrases();
		g_plugin.setString("gpgSave", tmp);
		LocalFree(tmp);
	}
	else g_plugin.delSetting("gpgSave");

	if (bPGPloaded) pgp_done();
	if (bGPGloaded) gpg_done();
	rsa_done();

	DestroyHookableEvent(g_hEvent[0]);
	DestroyHookableEvent(g_hEvent[1]);

	freeContactList();

	DeinitNetlib();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load

int CMPlugin::Load(void)
{
	INITCOMMONCONTROLSEX icce = { sizeof(icce), ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES };
	InitCommonControlsEx(&icce);

	DisableThreadLibraryCalls(g_plugin.getInst());

	char temp[MAX_PATH];
	GetTempPath(sizeof(temp), temp);
	GetLongPathName(temp, TEMP, sizeof(TEMP));
	TEMP_SIZE = (int)mir_strlen(TEMP);
	if (TEMP[TEMP_SIZE - 1] == '\\') {
		TEMP_SIZE--;
		TEMP[TEMP_SIZE] = '\0';
	}

	// check for support TrueColor Icons
	BOOL bIsComCtl6 = FALSE;
	HMODULE hComCtlDll = LoadLibrary("comctl32.dll");
	if (hComCtlDll) {
		typedef HRESULT(CALLBACK *PFNDLLGETVERSION)(DLLVERSIONINFO*);
		PFNDLLGETVERSION pfnDllGetVersion = (PFNDLLGETVERSION)GetProcAddress(hComCtlDll, "DllGetVersion");
		if (pfnDllGetVersion) {
			DLLVERSIONINFO dvi;
			memset(&dvi, 0, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
			HRESULT hRes = (*pfnDllGetVersion)(&dvi);
			if (SUCCEEDED(hRes) && dvi.dwMajorVersion >= 6) {
				bIsComCtl6 = TRUE;
			}
		}
		FreeLibrary(hComCtlDll);
	}
	if (bIsComCtl6)	iBmpDepth = ILC_COLOR32 | ILC_MASK;  // 32-bit images are supported
	else		iBmpDepth = ILC_COLOR24 | ILC_MASK;

	// load crypo++ dll
	if (!loadlib()) {
		msgbox(nullptr, LPGEN("SecureIM won't be loaded because cryptopp.dll is missing or wrong version!"), MODULENAME, MB_OK | MB_ICONSTOP);
		return 1;
	}

	// register plugin module
	Proto_RegisterModule(PROTOTYPE_ENCRYPTION, MODULENAME);

	// hook events
	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, onShutdown);
	HookEvent(ME_OPT_INITIALISE, onRegisterOptions);

	g_hEvent[0] = CreateHookableEvent(MODULENAME"/Disabled");
	g_hEvent[1] = CreateHookableEvent(MODULENAME"/Established");

	CreateServiceFunction(MODULENAME"/IsContactSecured", Service_IsContactSecured);
	CreateServiceFunction(MODULENAME"/SIM_EST", Service_CreateIM);
	CreateServiceFunction(MODULENAME"/SIM_DIS", Service_DisableIM);
	CreateServiceFunction(MODULENAME"/SIM_ST_DIS", Service_StatusDis);
	CreateServiceFunction(MODULENAME"/SIM_ST_ENA", Service_StatusEna);
	CreateServiceFunction(MODULENAME"/SIM_ST_TRY", Service_StatusTry);
	CreateServiceFunction(MODULENAME"/PGP_SET", Service_PGPsetKey);
	CreateServiceFunction(MODULENAME"/PGP_DEL", Service_PGPdelKey);
	CreateServiceFunction(MODULENAME"/GPG_SET", Service_GPGsetKey);
	CreateServiceFunction(MODULENAME"/GPG_DEL", Service_GPGdelKey);
	CreateServiceFunction(MODULENAME"/MODE_NAT", Service_ModeNative);
	CreateServiceFunction(MODULENAME"/MODE_PGP", Service_ModePGP);
	CreateServiceFunction(MODULENAME"/MODE_GPG", Service_ModeGPG);
	CreateServiceFunction(MODULENAME"/MODE_RSA", Service_ModeRSAAES);

	g_hCLIcon = ExtraIcon_RegisterCallback(MODULENAME, LPGEN("SecureIM status"), "sim_cm_est", onExtraImageListRebuilding, onExtraImageApplying);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

int CMPlugin::Unload()
{
	freelib();
	return 0;
}

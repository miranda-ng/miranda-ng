#include "commonheaders.h"

int hLangpack = 0;

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {1B2A39E5-E2F6-494D-958D-1808FD110DD5}
	{ 0x1B2A39E5, 0xE2F6, 0x494D, { 0x95, 0x8D, 0x18, 0x08, 0xFD, 0x11, 0x0D, 0xD5 } }
};

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

/////////////////////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID)
{
	g_hInst = hInst;
	if (dwReason == DLL_PROCESS_ATTACH) {
		INITCOMMONCONTROLSEX icce = { sizeof(icce), ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES };
		InitCommonControlsEx(&icce);
	}
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////////////////
// basic events: onModuleLoad, onModulesLoad, onShutdown

static HGENMENU AddMenuItem(LPCSTR name, int pos, HICON hicon, LPCSTR service, int flags = 0, WPARAM wParam = 0)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = flags | CMIF_HIDDEN;
	mi.position = pos;
	mi.hIcon = hicon;
	mi.pszName = (char*)name;
	mi.pszPopupName = (char*)-1;
	mi.pszService = (char*)service;
	return Menu_AddContactMenuItem(&mi);
}

static HGENMENU AddSubItem(HANDLE rootid, LPCSTR name, int pos, int poppos, LPCSTR service, WPARAM wParam = 0)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_CHILDPOPUP | CMIF_HIDDEN;
	mi.position = pos;
	mi.popupPosition = poppos;
	mi.pszName = (char*)name;
	mi.pszPopupName = (char*)rootid;
	mi.pszService = (char*)service;
	return Menu_AddContactMenuItem(&mi);
}

static int onModuleLoad(WPARAM, LPARAM)
{
	bPopupExists = ServiceExists(MS_POPUP_ADDPOPUPT);
	return 0;
}

static int onModulesLoaded(WPARAM, LPARAM)
{
	InitNetlib();

	g_hFolders = FoldersRegisterCustomPathT(LPGEN("SecureIM"), LPGEN("Icons"), _T(MIRANDA_PATH"\\icons"));

	InitIcons();
	GetFlags();
	onModuleLoad(0, 0);

	// RSA/AES
	Sent_NetLog("rsa_init");
	rsa_init(&exp, &imp);

	DBVARIANT dbv;
	dbv.type = DBVT_BLOB;

	if (db_get(0, MODULENAME, "rsa_priv", &dbv) == 0) {
		exp->rsa_set_keypair(CPP_MODE_RSA_4096, dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);
		rsa_4096 = 1;
	}
	else if (db_get(0, MODULENAME, "rsa_priv_4096", &dbv) == 0) {
		exp->rsa_set_keypair(CPP_MODE_RSA_4096 | CPP_MODE_RSA_BER, dbv.pbVal, dbv.cpbVal);
		db_free(&dbv);

		char priv_key[4096]; int priv_len;
		char pub_key[4096]; int pub_len;

		exp->rsa_get_keypair(CPP_MODE_RSA_4096, (PBYTE)&priv_key, &priv_len, (PBYTE)&pub_key, &pub_len);
		db_set_blob(NULL, MODULENAME, "rsa_priv", priv_key, priv_len);
		db_set_blob(NULL, MODULENAME, "rsa_pub", &pub_key, pub_len);
		db_unset(NULL, MODULENAME, "rsa_priv_2048");
		db_unset(NULL, MODULENAME, "rsa_pub_2048");
		rsa_4096 = 1;
	}

	if (!rsa_4096)
		mir_forkthread(sttGenerateRSA, NULL);

	exp->rsa_set_timeout(db_get_w(0, MODULENAME, "ket", 10));

	Sent_NetLog("pgp_init");

	bPGP = db_get_b(0, MODULENAME, "pgp", 0);
	if (bPGP) { //PGP
		bPGPloaded = pgp_init();
		bUseKeyrings = db_get_b(0, MODULENAME, "ukr", 1);
		LPSTR priv = db_get_sa(0, MODULENAME, "pgpPrivKey");
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
				LPSTR tmp = db_get_sa(0, MODULENAME, "pgpPubRing");
				if (tmp) {
					strncpy(PubRingPath, tmp, sizeof(PubRingPath));
					mir_free(tmp);
				}
				if (tmp = db_get_sa(0, MODULENAME, "pgpSecRing")) {
					strncpy(SecRingPath, tmp, sizeof(SecRingPath));
					mir_free(tmp);
				}
				if (PubRingPath[0] && SecRingPath[0]) {
					bPGPkeyrings = pgp_open_keyrings(PubRingPath, SecRingPath);
					if (bPGPkeyrings) {
						db_set_s(0, MODULENAME, "pgpPubRing", PubRingPath);
						db_set_s(0, MODULENAME, "pgpSecRing", SecRingPath);
					}
					else {
						db_unset(0, MODULENAME, "pgpPubRing");
						db_unset(0, MODULENAME, "pgpSecRing");
					}
				}
			}
		}
	}

	Sent_NetLog("gpg_init");

	bGPG = db_get_b(0, MODULENAME, "gpg", 0);
	if (bGPG) { //GPG
		bGPGloaded = gpg_init();

		char gpgexec[MAX_PATH], gpghome[MAX_PATH];
		gpgexec[0] = '\0'; gpghome[0] = '\0';

		LPSTR tmp = db_get_sa(0, MODULENAME, "gpgExec");
		if (tmp) {
			strncpy(gpgexec, tmp, sizeof(gpgexec)-1);
			mir_free(tmp);
		}
		if (tmp = db_get_sa(0, MODULENAME, "gpgHome")) {
			strncpy(gpghome, tmp, sizeof(gpghome)-1);
			mir_free(tmp);
		}

		if (db_get_b(0, MODULENAME, "gpgLogFlag", 0)) {
			if (tmp = db_get_sa(0, MODULENAME, "gpgLog")) {
				gpg_set_log(tmp);
				mir_free(tmp);
			}
		}

		if (db_get_b(0, MODULENAME, "gpgTmpFlag", 0)) {
			if (tmp = db_get_sa(0, MODULENAME, "gpgTmp")) {
				gpg_set_tmp(tmp);
				mir_free(tmp);
			}
		}

		bGPGkeyrings = gpg_open_keyrings(gpgexec, gpghome);
		if (bGPGkeyrings) {
			db_set_s(0, MODULENAME, "gpgExec", gpgexec);
			db_set_s(0, MODULENAME, "gpgHome", gpghome);
		}
		else {
			db_unset(0, MODULENAME, "gpgExec");
			db_unset(0, MODULENAME, "gpgHome");
		}

		bSavePass = db_get_b(0, MODULENAME, "gpgSaveFlag", 0);
		if (bSavePass) {
			if (tmp = db_get_sa(0, MODULENAME, "gpgSave")) {
				gpg_set_passphrases(tmp);
				mir_free(tmp);
			}
		}
	}

	Sent_NetLog("loadContactList");
	loadContactList();

	// add new skin sound
	SkinAddNewSound("IncomingSecureMessage", LPGEN("Incoming Secure Message"), "Sounds\\iSecureMessage.wav");
	SkinAddNewSound("OutgoingSecureMessage", LPGEN("Outgoing Secure Message"), "Sounds\\oSecureMessage.wav");

	// init extra icons
	for (int i = 0; i < SIZEOF(g_IEC); i++)
		g_IEC[i] = (HANDLE)-1;

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, onRebuildContactMenu);

	// hook init options
	HookEvent(ME_OPT_INITIALISE, onRegisterOptions);
	if (bPopupExists)
		HookEvent(ME_OPT_INITIALISE, onRegisterPopOptions);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);
	HookEvent(ME_DB_CONTACT_ADDED, onContactAdded);
	HookEvent(ME_DB_CONTACT_DELETED, onContactDeleted);

	// hook message transport
	CreateProtoServiceFunction(MODULENAME, PSR_MESSAGE, onRecvMsg);
	CreateProtoServiceFunction(MODULENAME, PSS_MESSAGE, onSendMsg);

	// create a menu item for creating a secure im connection to the user.
	g_hMenu[0] = AddMenuItem(sim301, 110000, g_hICO[ICO_CM_EST], MODULENAME"/SIM_EST", CMIF_NOTOFFLINE);
	g_hMenu[1] = AddMenuItem(sim302, 110001, g_hICO[ICO_CM_DIS], MODULENAME"/SIM_DIS", CMIF_NOTOFFLINE);

	if (ServiceExists(MS_CLIST_MENUBUILDSUBGROUP)) {
		g_hMenu[2] = AddMenuItem(sim312[0], 110002, NULL, NULL, CMIF_ROOTPOPUP);
		g_hMenu[3] = AddSubItem(g_hMenu[2], sim232[0], 110003, 110002, MODULENAME"/SIM_ST_DIS");
		g_hMenu[4] = AddSubItem(g_hMenu[2], sim232[1], 110004, 110002, MODULENAME"/SIM_ST_ENA");
		g_hMenu[5] = AddSubItem(g_hMenu[2], sim232[2], 110005, 110002, MODULENAME"/SIM_ST_TRY");
	}
	else {
		g_hMenu[2] = 0;
		g_hMenu[3] = AddMenuItem(sim232[0], 110003, NULL, MODULENAME"/SIM_ST_DIS");
		g_hMenu[4] = AddMenuItem(sim232[1], 110004, NULL, MODULENAME"/SIM_ST_ENA");
		g_hMenu[5] = AddMenuItem(sim232[2], 110005, NULL, MODULENAME"/SIM_ST_TRY");
	}

	if (bPGPloaded) {
		g_hMenu[6] = AddMenuItem(sim306, 110006, mode2icon(MODE_PGP | SECURED, 2), MODULENAME"/PGP_SET", 0);
		g_hMenu[7] = AddMenuItem(sim307, 110007, mode2icon(MODE_PGP, 2), MODULENAME"/PGP_DEL", 0);
	}

	if (bGPGloaded) {
		g_hMenu[8] = AddMenuItem(sim308, 110008, mode2icon(MODE_GPG | SECURED, 2), MODULENAME"/GPG_SET", 0);
		g_hMenu[9] = AddMenuItem(sim309, 110009, mode2icon(MODE_GPG, 2), MODULENAME"/GPG_DEL", 0);
	}

	if (ServiceExists(MS_CLIST_MENUBUILDSUBGROUP)) {
		g_hMenu[10] = AddMenuItem(sim311[0], 110010, NULL, NULL, CMIF_ROOTPOPUP);
		g_hMenu[11] = AddSubItem(g_hMenu[10], sim231[0], 110011, 110010, MODULENAME"/MODE_NAT");
		g_hMenu[12] = AddSubItem(g_hMenu[10], sim231[1], 110012, 110010, MODULENAME"/MODE_PGP");
		g_hMenu[13] = AddSubItem(g_hMenu[10], sim231[2], 110013, 110010, MODULENAME"/MODE_GPG");
		g_hMenu[14] = AddSubItem(g_hMenu[10], sim231[3], 110014, 110010, MODULENAME"/MODE_RSA");
	}
	else {
		g_hMenu[10] = 0;
		g_hMenu[11] = AddMenuItem(sim231[0], 110011, NULL, MODULENAME"/MODE_NAT");
		g_hMenu[12] = AddMenuItem(sim231[1], 110012, NULL, MODULENAME"/MODE_PGP");
		g_hMenu[13] = AddMenuItem(sim231[2], 110013, NULL, MODULENAME"/MODE_GPG");
		g_hMenu[14] = AddMenuItem(sim231[3], 110014, NULL, MODULENAME"/MODE_RSA");
	}

	InitSRMMIcons();
	return 0;
}

static int onShutdown(WPARAM, LPARAM)
{
	if (bSavePass) {
		LPSTR tmp = gpg_get_passphrases();
		db_set_s(0, MODULENAME, "gpgSave", tmp);
		LocalFree(tmp);
	}
	else db_unset(0, MODULENAME, "gpgSave");

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

extern "C" __declspec(dllexport) int __cdecl Load(void)
{
	mir_getLP(&pluginInfoEx);

	DisableThreadLibraryCalls(g_hInst);

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
	HMODULE hComCtlDll = LoadLibrary(_T("comctl32.dll"));
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
		msgbox1(0, sim107, MODULENAME, MB_OK | MB_ICONSTOP);
		return 1;
	}

	// register plugin module
	PROTOCOLDESCRIPTOR pd = { 0 };
	pd.cbSize = sizeof(pd);
	pd.szName = (char*)MODULENAME;
	pd.type = PROTOTYPE_ENCRYPTION;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// hook events
	HookEvent(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	HookEvent(ME_SYSTEM_PRESHUTDOWN, onShutdown);
	HookEvent(ME_SYSTEM_MODULELOAD, onModuleLoad);
	HookEvent(ME_SYSTEM_MODULEUNLOAD, onModuleLoad);

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

	g_hCLIcon = ExtraIcon_Register(MODULENAME, LPGEN("SecureIM status"), "sim_cm_est", onExtraImageListRebuilding, onExtraImageApplying);

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Unload

extern "C" __declspec(dllexport) int __cdecl Unload()
{
	freelib();
	return 0;
}

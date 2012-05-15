#include "commonheaders.h"

PLUGINLINK *pluginLink;
MM_INTERFACE mmi = {0};
int hLangpack = 0;

extern "C" BOOL APIENTRY DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID) {
	g_hInst = hInst;
	if (dwReason == DLL_PROCESS_ATTACH) {
		INITCOMMONCONTROLSEX icce = {
			sizeof(icce), ICC_LISTVIEW_CLASSES | ICC_TAB_CLASSES
		};
		InitCommonControlsEx(&icce);
	}
	return TRUE;
}


PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion) {
	return &pluginInfoEx;
}


MUUID* MirandaPluginInterfaces(void) {
	return interfaces;
}


void AddServiceFunction(LPCSTR serviceName, MIRANDASERVICE serviceFunction) {
	
	g_hService = (HANDLE*) mir_realloc(g_hService,sizeof(HANDLE)*(iService+1));
	g_hService[iService] = CreateServiceFunction(serviceName, serviceFunction);
	iService++;
}


void AddProtoServiceFunction(LPCSTR serviceName, MIRANDASERVICE serviceFunction) {
	
	g_hService = (HANDLE*) mir_realloc(g_hService,sizeof(HANDLE)*(iService+1));
 	g_hService[iService] = CreateProtoServiceFunction(szModuleName, serviceName, serviceFunction);
	iService++;
}


void AddHookFunction(LPCSTR eventName, MIRANDAHOOK hookFunction) {
	
	g_hHook = (HANDLE*) mir_realloc(g_hHook,sizeof(HANDLE)*(iHook+1));
	g_hHook[iHook] = HookEvent(eventName, hookFunction);
	iHook++;
}


HANDLE AddMenuItem(LPCSTR name,int pos,HICON hicon,LPCSTR service,int flags=0,WPARAM wParam=0) {

	CLISTMENUITEM mi;
	memset(&mi,0,sizeof(mi));
	mi.cbSize=sizeof(mi);
	mi.flags=flags | CMIF_HIDDEN;
	mi.position=pos;
	mi.hIcon=hicon;
	mi.pszName= (char*)name;
	mi.pszPopupName=(char*)-1;
	mi.pszService=(char*)service;
	return((HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,wParam,(LPARAM)&mi));
}


HANDLE AddSubItem(HANDLE rootid,LPCSTR name,int pos,int poppos,LPCSTR service,WPARAM wParam=0) {

    CLISTMENUITEM mi;
    memset(&mi,0,sizeof(mi));
    mi.cbSize=sizeof(mi);
    mi.flags=CMIF_CHILDPOPUP | CMIF_HIDDEN;
    mi.position=pos;
    mi.popupPosition=poppos;
    mi.hIcon=NULL;
    mi.pszName=(char*)name;
    mi.pszPopupName=(char*)rootid;
    mi.pszService=(char*)service;
    return((HANDLE)CallService(MS_CLIST_ADDCONTACTMENUITEM,wParam,(LPARAM)&mi));
}


int __cdecl Load(PLUGINLINK *link) {

	pluginLink = link;
	DisableThreadLibraryCalls(g_hInst);
	InitializeCriticalSection(&localQueueMutex);

	{
		char temp[MAX_PATH];
		GetTempPath(sizeof(temp),temp);
		GetLongPathName(temp,TEMP,sizeof(TEMP));
		TEMP_SIZE = (int)strlen(TEMP);
		if(TEMP[TEMP_SIZE-1]=='\\') {
			TEMP_SIZE--;
			TEMP[TEMP_SIZE]='\0';
		}
	}

	// get memoryManagerInterface address
	mir_getMMI( &mmi );
	//get per-plugin langpack interface
	mir_getLP(&pluginInfoEx);

	// check for support TrueColor Icons
	BOOL bIsComCtl6 = FALSE;
	HMODULE hComCtlDll = LoadLibrary("comctl32.dll");
	if ( hComCtlDll ) {
		typedef HRESULT (CALLBACK *PFNDLLGETVERSION)(DLLVERSIONINFO*);
		PFNDLLGETVERSION pfnDllGetVersion = (PFNDLLGETVERSION) GetProcAddress(hComCtlDll,"DllGetVersion");
		if ( pfnDllGetVersion ) {
			DLLVERSIONINFO dvi;
			memset(&dvi,0,sizeof(dvi));
			dvi.cbSize = sizeof(dvi);
			HRESULT hRes = (*pfnDllGetVersion)( &dvi );
			if ( SUCCEEDED(hRes) && dvi.dwMajorVersion >= 6 ) {
				bIsComCtl6 = TRUE;
			}
		}
		FreeLibrary(hComCtlDll);
	}
	if (bIsComCtl6)	iBmpDepth = ILC_COLOR32 | ILC_MASK;  // 32-bit images are supported
	else		iBmpDepth = ILC_COLOR24 | ILC_MASK;

//	iBmpDepth = ILC_COLOR32 | ILC_MASK;

	char version[512];
	CallService(MS_SYSTEM_GETVERSIONTEXT, sizeof(version), (LPARAM)&version);
	bCoreUnicode = strstr(version, "Unicode")!=0;
	iCoreVersion = CallService(MS_SYSTEM_GETVERSION,0,0);

	// load crypo++ dll
	if( !loadlib() ) {
		msgbox1(0,sim107,szModuleName,MB_OK|MB_ICONSTOP);
		return 1;
	}

	load_rtfconv();

	// register plugin module
	PROTOCOLDESCRIPTOR pd;
	memset(&pd,0,sizeof(pd));
	pd.cbSize = sizeof(pd);
	pd.szName = (char*)szModuleName;
	pd.type = PROTOTYPE_ENCRYPTION;
	CallService(MS_PROTO_REGISTERMODULE, 0, (LPARAM)&pd);

	// hook events
	AddHookFunction(ME_SYSTEM_MODULESLOADED, onModulesLoaded);
	AddHookFunction(ME_SYSTEM_OKTOEXIT, onSystemOKToExit); 

	g_hEvent[0] = CreateHookableEvent(MODULENAME"/Disabled");
	g_hEvent[1] = CreateHookableEvent(MODULENAME"/Established");

	AddServiceFunction(MODULENAME"/IsContactSecured",Service_IsContactSecured);
	AddServiceFunction(MODULENAME"/SIM_EST",Service_CreateIM);
	AddServiceFunction(MODULENAME"/SIM_DIS",Service_DisableIM);
	AddServiceFunction(MODULENAME"/SIM_ST_DIS",Service_StatusDis);
	AddServiceFunction(MODULENAME"/SIM_ST_ENA",Service_StatusEna);
	AddServiceFunction(MODULENAME"/SIM_ST_TRY",Service_StatusTry);
	AddServiceFunction(MODULENAME"/PGP_SET",Service_PGPsetKey);
	AddServiceFunction(MODULENAME"/PGP_DEL",Service_PGPdelKey);
	AddServiceFunction(MODULENAME"/GPG_SET",Service_GPGsetKey);
	AddServiceFunction(MODULENAME"/GPG_DEL",Service_GPGdelKey);
	AddServiceFunction(MODULENAME"/MODE_NAT",Service_ModeNative);
	AddServiceFunction(MODULENAME"/MODE_PGP",Service_ModePGP);
	AddServiceFunction(MODULENAME"/MODE_GPG",Service_ModeGPG);
	AddServiceFunction(MODULENAME"/MODE_RSA",Service_ModeRSAAES);

	return 0;
}


int __cdecl Unload() {
	DeleteCriticalSection(&localQueueMutex);
	return 0;
}


int __cdecl onModulesLoaded(WPARAM wParam,LPARAM lParam) {

#if defined(_DEBUG) || defined(NETLIB_LOG)
    InitNetlib();
    Sent_NetLog("onModuleLoaded begin");
#endif

    bMetaContacts = ServiceExists(MS_MC_GETMETACONTACT)!=0;
    bPopupExists = ServiceExists(MS_POPUP_ADDPOPUPEX)!=0;
    bPopupUnicode = ServiceExists(MS_POPUP_ADDPOPUPW)!=0;

    g_hFolders = FoldersRegisterCustomPath(szModuleName, "Icons", MIRANDA_PATH"\\icons");
    if( g_hFolders==(HANDLE)CALLSERVICE_NOTFOUND ) g_hFolders = 0;

    InitIcons();
    GetFlags();

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("rsa_init");
#endif
	{ // RSA/AES
		rsa_init(&exp,&imp);

		DBVARIANT dbv;
		dbv.type = DBVT_BLOB;

		if( DBGetContactSetting(0,szModuleName,"rsa_priv",&dbv) == 0 ) {
			exp->rsa_set_keypair(CPP_MODE_RSA_4096,dbv.pbVal,dbv.cpbVal);
			DBFreeVariant(&dbv);
			rsa_4096=1;
		}
		else
		if( DBGetContactSetting(0,szModuleName,"rsa_priv_4096",&dbv) == 0 ) {
			exp->rsa_set_keypair(CPP_MODE_RSA_4096|CPP_MODE_RSA_BER,dbv.pbVal,dbv.cpbVal);
			DBFreeVariant(&dbv);

			char priv_key[4096]; int priv_len;
			char pub_key[4096]; int pub_len;

			DBCONTACTWRITESETTING cws;
			cws.szModule = szModuleName;
			cws.value.type = DBVT_BLOB;

			exp->rsa_get_keypair(CPP_MODE_RSA_4096,(PBYTE)&priv_key,&priv_len,(PBYTE)&pub_key,&pub_len);

			cws.szSetting = "rsa_priv";
			cws.value.pbVal = (PBYTE)&priv_key;
			cws.value.cpbVal = priv_len;
			CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)0, (LPARAM)&cws);

			cws.szSetting = "rsa_pub";
			cws.value.pbVal = (PBYTE)&pub_key;
			cws.value.cpbVal = pub_len;
			CallService(MS_DB_CONTACT_WRITESETTING, (WPARAM)0, (LPARAM)&cws);

			DBDeleteContactSetting(0, szModuleName, "rsa_priv_2048");
			DBDeleteContactSetting(0, szModuleName, "rsa_pub_2048");
//			DBDeleteContactSetting(0, szModuleName, "rsa_priv_4096");
//			DBDeleteContactSetting(0, szModuleName, "rsa_pub_4096");

			rsa_4096=1;
		}	

		if( !rsa_4096 ) {
			unsigned int tID;
			CloseHandle( (HANDLE) _beginthreadex(NULL, 0, sttGenerateRSA, NULL, 0, &tID) );
		}

		exp->rsa_set_timeout( DBGetContactSettingWord(0,szModuleName,"ket",10) );
	}

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("pgp_init");
#endif
	bPGP = DBGetContactSettingByte(0, szModuleName, "pgp", 0);
	if(bPGP) { //PGP
	    bPGPloaded = pgp_init();
   	    bUseKeyrings = DBGetContactSettingByte(0,szModuleName,"ukr",1);
   	    LPSTR priv = myDBGetStringDecode(0,szModuleName,"pgpPrivKey");
   	    if(priv) {
	   	    bPGPprivkey = true;
		    if(bPGPloaded)
			pgp_set_priv_key(priv);
	   	    mir_free(priv);
	    }// if(priv)
            if(bPGPloaded && bUseKeyrings) {
    		char PubRingPath[MAX_PATH], SecRingPath[MAX_PATH];
    		PubRingPath[0]='\0'; SecRingPath[0]='\0';
    		if(pgp_get_version()<0x02000000) { // 6xx
    		    bPGPkeyrings = pgp_open_keyrings(PubRingPath,SecRingPath);
		}
        	else {
        		LPSTR tmp;
        		tmp = myDBGetString(0,szModuleName,"pgpPubRing");
        		if(tmp) {
        			strncpy(PubRingPath,tmp,sizeof(PubRingPath));
        			mir_free(tmp);
        		}
        		tmp = myDBGetString(0,szModuleName,"pgpSecRing");
        		if(tmp) {
        			strncpy(SecRingPath,tmp,sizeof(SecRingPath));
        			mir_free(tmp);
        		}
        	   	if(PubRingPath[0] && SecRingPath[0]) {
        			bPGPkeyrings = pgp_open_keyrings(PubRingPath,SecRingPath);
        			if(bPGPkeyrings) {
        				DBWriteContactSettingString(0,szModuleName,"pgpPubRing",PubRingPath);
        				DBWriteContactSettingString(0,szModuleName,"pgpSecRing",SecRingPath);
        			}
        			else {
        				DBDeleteContactSetting(0, szModuleName, "pgpPubRing");
        				DBDeleteContactSetting(0, szModuleName, "pgpSecRing");
        			}
        		}
    		}
    	    }// if(bPGPloaded && bUseKeyrings)
   	}// if(bPGP)

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("gpg_init");
#endif
	bGPG = DBGetContactSettingByte(0, szModuleName, "gpg", 0);
	if(bGPG) { //GPG

		LPSTR tmp;

		bGPGloaded = gpg_init();

   		char gpgexec[MAX_PATH], gpghome[MAX_PATH];
   		gpgexec[0]='\0'; gpghome[0]='\0';

		tmp = myDBGetString(0,szModuleName,"gpgExec");
		if(tmp) {
			strncpy(gpgexec,tmp,sizeof(gpgexec));
			mir_free(tmp);
		}
		tmp = myDBGetString(0,szModuleName,"gpgHome");
		if(tmp) {
			strncpy(gpghome,tmp,sizeof(gpghome));
			mir_free(tmp);
		}

		if(DBGetContactSettingByte(0, szModuleName, "gpgLogFlag",0)) {
			tmp = myDBGetString(0,szModuleName,"gpgLog");
			if(tmp) {
				gpg_set_log(tmp);
				mir_free(tmp);
			}
		}

		if(DBGetContactSettingByte(0, szModuleName, "gpgTmpFlag",0)) {
			tmp = myDBGetString(0,szModuleName,"gpgTmp");
			if(tmp) {
				gpg_set_tmp(tmp);
				mir_free(tmp);
			}
		}

		bGPGkeyrings = gpg_open_keyrings(gpgexec,gpghome);
		if(bGPGkeyrings) {
			DBWriteContactSettingString(0,szModuleName,"gpgExec",gpgexec);
			DBWriteContactSettingString(0,szModuleName,"gpgHome",gpghome);
		}
		else {
			DBDeleteContactSetting(0, szModuleName, "gpgExec");
			DBDeleteContactSetting(0, szModuleName, "gpgHome");
		}

		bSavePass = DBGetContactSettingByte(0,szModuleName,"gpgSaveFlag",0);
		if(bSavePass) {
			tmp = myDBGetString(0,szModuleName,"gpgSave");
			if(tmp) {
				gpg_set_passphrases(tmp);
				mir_free(tmp);
			}
		}
	}

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("loadContactList");
#endif
	loadContactList();

	// add new skin sound
	SkinAddNewSound("IncomingSecureMessage","Incoming Secure Message","Sounds\\iSecureMessage.wav");
	SkinAddNewSound("OutgoingSecureMessage","Outgoing Secure Message","Sounds\\oSecureMessage.wav");

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("init extra icons");
#endif
	// init extra icons
	for(int i=0;i<1+MODE_CNT*IEC_CNT;i++) {
		g_IEC[i].cbSize = sizeof(g_IEC[i]);
		g_IEC[i].ColumnType = bADV;
		g_IEC[i].hImage = (HANDLE)-1;
	}

	// build extra imagelist
	//onExtraImageListRebuilding(0,0);

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("hook events");
#endif
	AddHookFunction(ME_CLIST_PREBUILDCONTACTMENU, onRebuildContactMenu);
//	g_hMC = HookEvent(ME_MC_SUBCONTACTSCHANGED, onMC);

	if( ServiceExists(MS_EXTRAICON_REGISTER) ) {
		g_hCLIcon = ExtraIcon_Register(szModuleName, Translate("SecureIM status"), "sim_cm_est",
						onExtraImageListRebuilding,
						onExtraImageApplying);
	}
	else {
		AddHookFunction(ME_CLIST_EXTRA_LIST_REBUILD, onExtraImageListRebuilding);
		AddHookFunction(ME_CLIST_EXTRA_IMAGE_APPLY, onExtraImageApplying);
	}

	// hook init options
	AddHookFunction(ME_OPT_INITIALISE, onRegisterOptions);
	if(bPopupExists)
	AddHookFunction(ME_OPT_INITIALISE, onRegisterPopOptions);
	AddHookFunction(ME_PROTO_ACK, onProtoAck);
	AddHookFunction(ME_DB_CONTACT_SETTINGCHANGED, onContactSettingChanged);
	AddHookFunction(ME_DB_CONTACT_ADDED, onContactAdded);
	AddHookFunction(ME_DB_CONTACT_DELETED, onContactDeleted);

	// hook message transport
	AddProtoServiceFunction(PSR_MESSAGE, onRecvMsg);
	AddProtoServiceFunction(PSS_MESSAGE, (MIRANDASERVICE)onSendMsg);
	AddProtoServiceFunction(PSS_MESSAGE"W", (MIRANDASERVICE)onSendMsgW);
	AddProtoServiceFunction(PSS_FILE, (MIRANDASERVICE)onSendFile);

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("create Native/RSA menu");
#endif
	// create a menu item for creating a secure im connection to the user.
	g_hMenu[0] = AddMenuItem(sim301,110000,g_hICO[ICO_CM_EST],MODULENAME"/SIM_EST",CMIF_NOTOFFLINE);
	g_hMenu[1] = AddMenuItem(sim302,110001,g_hICO[ICO_CM_DIS],MODULENAME"/SIM_DIS",CMIF_NOTOFFLINE);

	if(ServiceExists(MS_CLIST_ADDSUBGROUPMENUITEM)) {
	    g_hMenu[2] = AddMenuItem(sim312[0],110002,NULL,NULL,CMIF_ROOTPOPUP);
	    g_hMenu[3] = AddSubItem(g_hMenu[2],sim232[0],110003,110002,MODULENAME"/SIM_ST_DIS");
	    g_hMenu[4] = AddSubItem(g_hMenu[2],sim232[1],110004,110002,MODULENAME"/SIM_ST_ENA");
	    g_hMenu[5] = AddSubItem(g_hMenu[2],sim232[2],110005,110002,MODULENAME"/SIM_ST_TRY");
	}
	else {
	    g_hMenu[2] = 0;
	    g_hMenu[3] = AddMenuItem(sim232[0],110003,NULL,MODULENAME"/SIM_ST_DIS");
	    g_hMenu[4] = AddMenuItem(sim232[1],110004,NULL,MODULENAME"/SIM_ST_ENA");
	    g_hMenu[5] = AddMenuItem(sim232[2],110005,NULL,MODULENAME"/SIM_ST_TRY");
	}

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("create PGP/GPG menu");
#endif
	HICON icon;
	if( bPGPloaded ) {
		icon=mode2icon(MODE_PGP|SECURED,2);
		g_hMenu[6] = AddMenuItem(sim306,110006,icon,MODULENAME"/PGP_SET",0);
		icon=mode2icon(MODE_PGP,2);
		g_hMenu[7] = AddMenuItem(sim307,110007,icon,MODULENAME"/PGP_DEL",0);
	}

    	if(bGPGloaded) {
		icon=mode2icon(MODE_GPG|SECURED,2);
		g_hMenu[8] = AddMenuItem(sim308,110008,icon,MODULENAME"/GPG_SET",0);
		icon=mode2icon(MODE_GPG,2);
		g_hMenu[9] = AddMenuItem(sim309,110009,icon,MODULENAME"/GPG_DEL",0);
    	}

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("create Mode menu");
#endif
	if(ServiceExists(MS_CLIST_ADDSUBGROUPMENUITEM)) {
	    g_hMenu[10] = AddMenuItem(sim311[0],110010,NULL,NULL,CMIF_ROOTPOPUP);
	    g_hMenu[11] = AddSubItem(g_hMenu[10],sim231[0],110011,110010,MODULENAME"/MODE_NAT");
	    g_hMenu[12] = AddSubItem(g_hMenu[10],sim231[1],110012,110010,MODULENAME"/MODE_PGP");
	    g_hMenu[13] = AddSubItem(g_hMenu[10],sim231[2],110013,110010,MODULENAME"/MODE_GPG");
	    g_hMenu[14] = AddSubItem(g_hMenu[10],sim231[3],110014,110010,MODULENAME"/MODE_RSA");
	}
	else {
	    g_hMenu[10] = 0;
	    g_hMenu[11] = AddMenuItem(sim231[0],110011,NULL,MODULENAME"/MODE_NAT");
	    g_hMenu[12] = AddMenuItem(sim231[1],110012,NULL,MODULENAME"/MODE_PGP");
	    g_hMenu[13] = AddMenuItem(sim231[2],110013,NULL,MODULENAME"/MODE_GPG");
	    g_hMenu[14] = AddMenuItem(sim231[3],110014,NULL,MODULENAME"/MODE_RSA");
	}
	
    	// updater plugin support
        if(ServiceExists(MS_UPDATE_REGISTERFL)) {
		CallService(MS_UPDATE_REGISTERFL, (WPARAM)2445, (LPARAM)&pluginInfoEx);
	}

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("create srmm icons");
#endif
	// add icon to srmm status icons
	if(ServiceExists(MS_MSG_ADDICON)) {

		StatusIconData sid;
		memset(&sid,0,sizeof(sid));
		sid.cbSize = sizeof(sid);
		sid.szModule = (char*)szModuleName;
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
		AddHookFunction(ME_MSG_WINDOWEVENT, onWindowEvent);
		AddHookFunction(ME_MSG_ICONPRESSED, onIconPressed);
	}

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("onModuleLoaded end");
#endif
	return 0;
}


int __cdecl onSystemOKToExit(WPARAM wParam,LPARAM lParam) {

    if(bSavePass) {
	LPSTR tmp = gpg_get_passphrases();
	DBWriteContactSettingString(0,szModuleName,"gpgSave",tmp);
	LocalFree(tmp);
    }
    else {
	DBDeleteContactSetting(0,szModuleName,"gpgSave");
    }
	if(bPGPloaded) pgp_done();
	if(bGPGloaded) gpg_done();
	rsa_done();
	while(iHook--) UnhookEvent(g_hHook[iHook]);
	mir_free(g_hHook);
	while(iService--) DestroyServiceFunction(g_hService[iService]);
	mir_free(g_hService);
	DestroyHookableEvent(g_hEvent[0]);
	DestroyHookableEvent(g_hEvent[1]);
	freeContactList();
	free_rtfconv();
#if defined(_DEBUG) || defined(NETLIB_LOG)
	DeinitNetlib();
#endif
	return 0;
}

// EOF

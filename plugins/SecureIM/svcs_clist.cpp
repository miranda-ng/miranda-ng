#include "commonheaders.h"


int __cdecl onContactSettingChanged(WPARAM wParam,LPARAM lParam) {

	HANDLE hContact = (HANDLE)wParam;
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	if(!hContact || strcmp(cws->szSetting,"Status")) return 0;

	pUinKey ptr = getUinKey(hContact);
	int stat = getContactStatus(hContact);
	if(!ptr || stat==-1) return 0;

//	HANDLE hMetaContact = getMetaContact(hContact);
//	if(hMetaContact) {
//		ptr = getUinKey(hMetaContact);
//		if(!ptr) return 0;
//	}

	if (stat==ID_STATUS_OFFLINE) { // go offline
		if (ptr->mode==MODE_NATIVE && cpp_keyx(ptr->cntx)) { // have active context
			cpp_delete_context(ptr->cntx); ptr->cntx=0; // reset context
//			if(hMetaContact) { // is subcontact of metacontact
//				showPopUpDC(hMetaContact);
//				ShowStatusIconNotify(hMetaContact);
//				if(getMostOnline(hMetaContact)) { // make handover
//					CallContactService(hMetaContact,PSS_MESSAGE,0,(LPARAM)SIG_INIT);
//				}
//			}
//			else { // is contact or metacontact (not subcontact)
				showPopUpDC(hContact);	// show popup "Disabled"
				ShowStatusIconNotify(hContact); // change icon in CL
//			}
		}
		else
		if (ptr->mode==MODE_RSAAES && exp->rsa_get_state(ptr->cntx)==7) {
			deleteRSAcntx(ptr);
//			if(hMetaContact) { // is subcontact of metacontact
//				showPopUpDC(hMetaContact);
//				ShowStatusIconNotify(hMetaContact);
//				if(getMostOnline(hMetaContact)) { // make handover
//					CallContactService(hMetaContact,PSS_MESSAGE,0,(LPARAM)SIG_INIT);
//				}
//			}
//			else { // is contact or metacontact (not subcontact)
				showPopUpDC(hContact);	// show popup "Disabled"
				ShowStatusIconNotify(hContact); // change icon in CL
//			}
		}
	}
	else { // go not offline
//		if(!hMetaContact) { // is contact or metacontact (not subcontact)
			if (ptr->offlineKey) {
				cpp_reset_context(ptr->cntx);
				ptr->offlineKey = false;
			}
			ShowStatusIconNotify(hContact); // change icon in CL
//		}
	}
	return 0;
}


//  wParam=(WPARAM)(HANDLE)hContact
//  lParam=0
int __cdecl onContactAdded(WPARAM wParam,LPARAM lParam) {
	addContact((HANDLE)wParam);
	return 0;
}


//  wParam=(WPARAM)(HANDLE)hContact
//  lParam=0
int __cdecl onContactDeleted(WPARAM wParam,LPARAM lParam) {
	delContact((HANDLE)wParam);
	return 0;
}


int __cdecl onExtraImageListRebuilding(WPARAM, LPARAM) {

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("onExtraImageListRebuilding");
#endif
	if( (bADV || g_hCLIcon) && ServiceExists(MS_CLIST_EXTRA_ADD_ICON) ) {
		RefreshContactListIcons();
	}
	return 0;
}


int __cdecl onExtraImageApplying(WPARAM wParam, LPARAM) {

	if( (bADV || g_hCLIcon) && ServiceExists(MS_CLIST_EXTRA_SET_ICON) && isSecureProtocol((HANDLE)wParam) ) {
		IconExtraColumn iec = mode2iec(isContactSecured((HANDLE)wParam));
		if( g_hCLIcon ) {
			ExtraIcon_SetIcon(g_hCLIcon, (HANDLE)wParam, iec.hImage);
		}
		else {
			CallService(MS_CLIST_EXTRA_SET_ICON, wParam, (LPARAM)&iec);
		}
	}
	return 0;
}


int __cdecl onRebuildContactMenu(WPARAM wParam,LPARAM lParam) {

#if defined(_DEBUG) || defined(NETLIB_LOG)
	Sent_NetLog("onRebuildContactMenu");
#endif
	HANDLE hContact = (HANDLE)wParam;
	BOOL bMC = isProtoMetaContacts(hContact);
	if( bMC ) hContact = getMostOnline(hContact); // возьмем тот, через который пойдет сообщение
	pUinKey ptr = getUinKey(hContact);
	int i;

	CLISTMENUITEM mi;
	memset(&mi,0,sizeof(mi));
	mi.cbSize = sizeof(CLISTMENUITEM);

	ShowStatusIconNotify(hContact);

	// check offline/online
	if(!ptr) {
		// hide menu bars
		mi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE | CMIF_HIDDEN;
		for(i=0;i<SIZEOF(g_hMenu);i++) {
			if( g_hMenu[i] )
				CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)g_hMenu[i],(LPARAM)&mi);
		}
		return 0;
	}

//	char *szProto = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
//	if (szProto==NULL) // || DBGetContactSettingDword(hContact, szProto, "Status", ID_STATUS_OFFLINE) == ID_STATUS_OFFLINE)
//		return 0;

	BOOL isSecureProto = isSecureProtocol(hContact);
	BOOL isPGP = isContactPGP(hContact);
	BOOL isGPG = isContactGPG(hContact);
//	BOOL isRSAAES = isContactRSAAES(hContact);
	BOOL isSecured = isContactSecured(hContact)&SECURED;
	BOOL isChat = isChatRoom(hContact);
	BOOL isMiranda = isClientMiranda(hContact);

	// hide all menu bars
	mi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE | CMIF_HIDDEN;
	for(i=0;i<SIZEOF(g_hMenu);i++) {
		if( g_hMenu[i] )
			CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)g_hMenu[i],(LPARAM)&mi);
	}

	if ( isSecureProto && !isChat && isMiranda && 
	    (ptr->mode==MODE_NATIVE || ptr->mode==MODE_RSAAES) ) {
		// Native/RSAAES
		mi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE | CMIM_ICON;
		if( !isSecured ) {
			// create secureim connection
			mi.hIcon = mode2icon(ptr->mode|SECURED,2);
			CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)g_hMenu[0],(LPARAM)&mi);
		}
		else {
			// disable secureim connection
			mi.hIcon = mode2icon(ptr->mode,2);
			CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)g_hMenu[1],(LPARAM)&mi);
		}
		// set status menu
		if( bSCM && !bMC &&
		    ( !isSecured || ptr->mode==MODE_PGP || ptr->mode==MODE_GPG ) ) {

			mi.flags = CMIM_FLAGS | CMIM_NAME | CMIM_ICON;
			mi.hIcon = g_hICO[ICO_ST_DIS+ptr->status];
			mi.pszName = (LPSTR)sim312[ptr->status];
			CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)g_hMenu[2],(LPARAM)&mi);

			mi.flags = CMIM_FLAGS | CMIM_ICON;
			for(i=0;i<=(ptr->mode==MODE_RSAAES?1:2);i++) {
				mi.hIcon = (i == ptr->status) ? g_hICO[ICO_ST_DIS+ptr->status] : NULL;
				CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)g_hMenu[3+i],(LPARAM)&mi);
			}
		}
	}
	else
	if( isSecureProto && !isChat && (ptr->mode==MODE_PGP || ptr->mode==MODE_GPG) ) {
		// PGP, GPG
		if( ptr->mode==MODE_PGP && bPGPloaded ) {
			if((bPGPkeyrings || bPGPprivkey) && !isGPG) {
				mi.flags = CMIM_FLAGS;
				CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)g_hMenu[isPGP+6],(LPARAM)&mi);
			}
		}
		if( ptr->mode==MODE_GPG && bGPGloaded ) {
			if(bGPGkeyrings && !isPGP) {
				mi.flags = CMIM_FLAGS;
				CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)g_hMenu[isGPG+8],(LPARAM)&mi);
			}
		}
	}
	if( isSecureProto && !isChat && isMiranda ) {
		// set mode menu
		if( bMCM && !bMC &&
	            ( !isSecured || ptr->mode==MODE_PGP || ptr->mode==MODE_GPG ) ) {

			mi.flags = CMIM_FLAGS | CMIM_NAME | CMIM_ICON;
			mi.hIcon = g_hICO[ICO_OV_NAT+ptr->mode];
			mi.pszName = (LPSTR)sim311[ptr->mode];
			CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)g_hMenu[10],(LPARAM)&mi);

			mi.flags = CMIM_FLAGS | CMIM_ICON;
			for(i=0;i<MODE_CNT;i++) {
				if( i==MODE_PGP && ptr->mode!=MODE_PGP && !bPGP ) continue;
				if( i==MODE_GPG && ptr->mode!=MODE_GPG && !bGPG ) continue;
				mi.hIcon = (i == ptr->mode) ? g_hICO[ICO_ST_ENA] : NULL;
				CallService(MS_CLIST_MODIFYMENUITEM,(WPARAM)g_hMenu[11+i],(LPARAM)&mi);
			}
		}
	}

	return 0;
}


// EOF

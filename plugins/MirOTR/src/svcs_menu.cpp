#include "stdafx.h"

static HGENMENU hStopItem, hStartItem;

////////////////////////////////
///////// Menu Services ////////
///////////////////////////////
int StartOTR(MCONTACT hContact) {
	const char *proto = GetContactProto(hContact);
	if (!proto) return 1; // error
	char *uname = contact_get_id(hContact);
	if (!uname) return 1; // error
	DWORD pol = db_get_dw(hContact, MODULENAME, "Policy", CONTACT_DEFAULT_POLICY);
	if(pol == CONTACT_DEFAULT_POLICY) pol = options.default_policy;
	
	lib_cs_lock();
	char *msg = otrl_proto_default_query_msg(MODULENAME, pol);
	otr_gui_inject_message((void*)hContact, proto, proto, uname, msg ? msg : MIROTR_PROTO_HELLO);
	free(msg);
	lib_cs_unlock();
	mir_free(uname);
	return 0;
}

INT_PTR SVC_StartOTR(WPARAM hContact, LPARAM lParam)
{
	TCHAR buff[512];

	MCONTACT hSub = db_mc_getMostOnline(hContact);
	if(hSub != 0)
		hContact = hSub;

	if ( options.bHaveSecureIM && CallService("SecureIM/IsContactSecured", hContact, 0) != 0 ) {
		mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_OTR_SECUREIM_STARTED), contact_get_nameT(hContact));
		ShowError(buff);
		return 0;
	}

	mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_REQUEST_OTR), contact_get_nameT(hContact));
	ShowMessage(hContact, buff);

	return StartOTR(hContact);
}

INT_PTR SVC_RefreshOTR(WPARAM hContact, LPARAM lParam)
{
	TCHAR buff[512];

	MCONTACT hSub = db_mc_getMostOnline(hContact);
	if(hSub != 0)
		hContact = hSub;

	if ( options.bHaveSecureIM && CallService("SecureIM/IsContactSecured", hContact, 0) != 0 ) {
		mir_sntprintf(buff, 512, TranslateT(LANG_OTR_SECUREIM_STARTED), contact_get_nameT(hContact));
		ShowError(buff);
		return 0;
	}
	
	mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_TRY_CONTINUE_OTR), contact_get_nameT(hContact));
	ShowMessage(hContact, buff);

	int res = StartOTR(hContact);
	if (res) return res;

	return 0;
}

int otr_disconnect_contact(MCONTACT hContact)
{
	MCONTACT hSub = db_mc_getMostOnline(hContact);
	if (hSub != 0)
		hContact = hSub;
	
	const char *proto = GetContactProto(hContact);
	if (!proto) return 1; // error
	char *uname = contact_get_id(hContact);
	if (!uname) return 1; // error

	lib_cs_lock();
	otrl_message_disconnect_all_instances(otr_user_state, &ops, (void*)hContact, proto, proto, uname);
	lib_cs_unlock();
	mir_free(uname);
	return 0;
}

INT_PTR SVC_StopOTR(WPARAM hContact, LPARAM lParam)
{
	// prevent this filter from acting on injeceted messages for metas, when they are passed though the subcontact's proto send chain
	if (otr_disconnect_contact(hContact)) return 0;

	SetEncryptionStatus(hContact, TRUST_NOT_PRIVATE);

	TCHAR buff[512];
	mir_sntprintf(buff, SIZEOF(buff), TranslateT(LANG_SESSION_TERMINATED_OTR), contact_get_nameT(hContact));
	ShowMessage(hContact, buff);
	return 0;
}

INT_PTR SVC_VerifyOTR(WPARAM hContact, LPARAM lParam)
{
	MCONTACT hSub = db_mc_getMostOnline(hContact);
	if(hSub != 0)
		hContact = hSub;

	ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);
	if (!context)
		return 1;
	
	//VerifyContextDialog(context);	
	SMPInitDialog(context);
	return 0;
}

INT_PTR SVC_ToggleHTMLOTR(WPARAM hContact, LPARAM lParam)
{
	MCONTACT hSub = db_mc_getMostOnline(hContact);
	if (hSub != 0)
		hContact = hSub;
	
	if (db_get_b(hContact, MODULENAME, "HTMLConv", 0))
		db_set_b(hContact, MODULENAME, "HTMLConv", 0);
	else
		db_set_b(hContact, MODULENAME, "HTMLConv", 1);

	return 0;
}

void InitMenu()
{
	CreateServiceFunction(MS_OTR_MENUSTART, SVC_StartOTR);
	CreateServiceFunction(MS_OTR_MENUSTOP, SVC_StopOTR);
	CreateServiceFunction(MS_OTR_MENUREFRESH, SVC_RefreshOTR);
	CreateServiceFunction(MS_OTR_MENUVERIFY, SVC_VerifyOTR);
	CreateServiceFunction(MS_OTR_MENUTOGGLEHTML, SVC_ToggleHTMLOTR);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIF_NOTOFFLINE | CMIF_TCHAR;
	mi.position = -400000;

	mi.ptszName = _T(LANG_STOP_OTR);
	mi.pszService = MS_OTR_MENUSTOP;
	mi.icolibItem = GetIconHandle(ICON_PRIVATE);

	hStopItem = Menu_AddContactMenuItem(&mi);
	
	mi.ptszName = _T(LANG_START_OTR);
	mi.pszService = MS_OTR_MENUSTART;
	mi.icolibItem = GetIconHandle(ICON_NOT_PRIVATE);

	hStartItem = Menu_AddContactMenuItem(&mi);

	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, SVC_PrebuildContactMenu);
}

int SVC_PrebuildContactMenu(WPARAM hContact, LPARAM lParam)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.flags = CMIM_FLAGS | CMIF_NOTOFFLINE | CMIF_TCHAR;
	
	const char *proto = GetContactProto(hContact);
	DWORD pol = CONTACT_DEFAULT_POLICY;
	
	if (!proto || db_get_b(hContact, proto, "ChatRoom", 0) == 1) {
hide_all:
		Menu_ShowItem(hStartItem, false);
		Menu_ShowItem(hStopItem, false);
		return 0;
	}
	
	if(proto && strcmp(proto, META_PROTO) == 0) {
		// make menu act as per most online subcontact
		hContact = db_mc_getMostOnline(hContact);
		if (!hContact)
			goto hide_all;
		proto = GetContactProto(hContact);
	}

	pol = db_get_dw(hContact, MODULENAME, "Policy", CONTACT_DEFAULT_POLICY);
	if(pol == CONTACT_DEFAULT_POLICY) pol = options.default_policy;
	
	if(pol == OTRL_POLICY_NEVER || pol == OTRL_POLICY_ALWAYS)
		goto hide_all;

	ConnContext *context = otrl_context_find_miranda(otr_user_state, hContact);
	TrustLevel encrypted = otr_context_get_trust(context);
	Menu_ShowItem(hStartItem, encrypted == TRUST_NOT_PRIVATE);
	Menu_ShowItem(hStopItem, encrypted != TRUST_NOT_PRIVATE);
	return 0;
}

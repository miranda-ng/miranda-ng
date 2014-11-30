#include "headers.h"

int hLangpack;
HINSTANCE hInst;
HANDLE hWindowList;

PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {A9E9C114-84B9-434B-A3D5-89921D39DDFF}
	{0xa9e9c114, 0x84b9, 0x434b, {0xa3, 0xd5, 0x89, 0x92, 0x1d, 0x39, 0xdd, 0xff}}
};

//========================
//  WINAPI DllMain
//========================

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}
//========================
//  MirandaPluginInfo
//========================


extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

//===================
// MainInit
//===================

int MainInit(WPARAM wParam, LPARAM lParam)
{
	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position=10;
	mi.flags=0;
	mi.hIcon= LoadIcon(hInst,MAKEINTRESOURCE(IDI_POUNCE));
	mi.pszName=LPGEN("&Buddy Pounce");
	mi.pszService="BuddyPounce/MenuCommand";
	mi.pszContactOwner=NULL;

	Menu_AddContactMenuItem(&mi);
	return 0;
}

int MsgAck(WPARAM wParam, LPARAM lParam) 
{ 
	ACKDATA *ack=(ACKDATA*)lParam; 

    if (ack && ack->cbSize==sizeof(ACKDATA) 
        && ack->type==ACKTYPE_MESSAGE 
        && ack->hProcess==(HANDLE)WindowList_Find(hWindowList,ack->hContact)) { 
		if (db_get_b(NULL, modname, "ShowDeliveryMessages", 1))
			CreateMessageAcknowlegedWindow(ack->hContact,ack->result == ACKRESULT_SUCCESS);
		if (ack->result == ACKRESULT_SUCCESS) {
			// wrtie it to the DB
			DBEVENTINFO dbei = { 0 };
			DBVARIANT dbv;
			int reuse = db_get_b(ack->hContact,modname, "Reuse", 0);
			if ( !db_get_ts(ack->hContact, modname, "PounceMsg", &dbv) && (dbv.ptszVal[0] != '\0')) {
				ptrA pszUtf( mir_utf8encodeT(dbv.ptszVal));
				dbei.cbSize = sizeof(dbei);
				dbei.eventType = EVENTTYPE_MESSAGE;
				dbei.flags = DBEF_UTF | DBEF_SENT;
				dbei.szModule = (char*)ack->szModule;
				dbei.timestamp = time(NULL);
				dbei.cbBlob = mir_strlen(pszUtf) + 1;
				dbei.pBlob = (PBYTE)(char*)pszUtf;
				db_event_add(ack->hContact, &dbei);
			}
			// check to reuse
			if (reuse > 1)
				db_set_b(ack->hContact, modname, "Reuse", (BYTE)(reuse-1));
			else {
				db_set_b(ack->hContact,modname, "Reuse", 0);
				db_set_ws(ack->hContact, modname, "PounceMsg", _T(""));
			}
		}
		WindowList_Remove(hWindowList,(HWND)ack->hProcess);
   } 
   return 0; 
} 

int BuddyPounceOptInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.flags = ODPF_BOLDGROUPS | ODPF_TCHAR;
	odp.hInstance = hInst;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.ptszGroup = LPGENT("Message Sessions");
	odp.ptszTitle = LPGENT("Buddy Pounce");
	odp.pfnDlgProc = BuddyPounceOptionsDlgProc;
	Options_AddPage(wParam, &odp);
	return 0;
}

int statusCheck(int statusFlag, int status)
{
	if (statusFlag == ANY || !statusFlag) return 1;
	switch(status) {
		case ID_STATUS_OFFLINE:
			return 0;
		case ID_STATUS_ONLINE:
			return (statusFlag & ONLINE);
		case ID_STATUS_AWAY:
			return (statusFlag & AWAY);
		case ID_STATUS_NA:
			return (statusFlag & NA);
		case ID_STATUS_OCCUPIED:
			return (statusFlag & OCCUPIED);
		case ID_STATUS_DND:
			return (statusFlag & DND);
		case ID_STATUS_FREECHAT:
			return (statusFlag & FFC);
		case ID_STATUS_INVISIBLE:
			return (statusFlag & INVISIBLE);
		case ID_STATUS_OUTTOLUNCH:
			return (statusFlag & LUNCH);
		case ID_STATUS_ONTHEPHONE:
			return (statusFlag & PHONE);
	}
	return 0;
}
int CheckDate(MCONTACT hContact)
{
	time_t curtime = time (NULL);
	if(!db_get_b(hContact,modname,"GiveUpDays",0))
		return 1;
	if(db_get_b(hContact,modname,"GiveUpDays",0) && ( abs((time_t)db_get_dw(hContact,modname,"GiveUpDate",0)) > curtime))
		return 1;
	return 0;
}

void SendPounce(TCHAR *text, MCONTACT hContact)
{
	char* pszUtf = mir_utf8encodeT(text);
	if (HANDLE hSendId = (HANDLE)CallContactService(hContact, PSS_MESSAGE, PREF_UTF, (LPARAM)pszUtf)) 
		WindowList_Add(hWindowList, (HWND)hSendId, hContact);

}

int UserOnlineSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;

	char *szProto = GetContactProto(hContact);
	if(hContact == NULL || strcmp(cws->szSetting,"Status")) return 0;
	if (szProto && (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM)) {
		int newStatus = cws->value.wVal;
		int oldStatus = db_get_w(hContact,"UserOnline","OldStatus",ID_STATUS_OFFLINE);
		
		if (newStatus != oldStatus && hContact != NULL && newStatus != ID_STATUS_OFFLINE) {
			DBVARIANT dbv;
			if (!db_get_ts(hContact, modname, "PounceMsg", &dbv) && (dbv.ptszVal[0] != '\0')) {
				// check my status
				if (statusCheck(db_get_w(hContact, modname, "SendIfMyStatusIsFLAG", 0), CallProtoService(szProto, PS_GETSTATUS,0,0)) 
				// check the contacts status
				&& statusCheck(db_get_w(hContact, modname, "SendIfTheirStatusIsFLAG", 0), newStatus)) {
					// check if we r giving up after x days
					if (CheckDate(hContact)) {
						if (db_get_w(hContact, modname, "ConfirmTimeout", 0)) {
							SendPounceDlgProcStruct *spdps = (SendPounceDlgProcStruct *)mir_alloc(sizeof(SendPounceDlgProcStruct));
							TCHAR *message = mir_tstrdup(dbv.ptszVal); // will get free()ed in the send confirm window proc
							spdps->hContact = hContact;
							spdps->message = message;
							CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_CONFIRMSEND), 0, SendPounceDlgProc, (LPARAM)spdps);
							// set the confirmation window to send the msg when the timeout is done
							mir_free(message);
						}
						else SendPounce(dbv.ptszVal, hContact);
					}
				}
				db_free(&dbv);
			}
		}
	}
	return 0;
}

INT_PTR BuddyPounceMenuCommand(WPARAM hContact, LPARAM lParam)
{
	if (db_get_b(NULL, modname, "UseAdvanced", 0) || db_get_b(hContact, modname, "UseAdvanced", 0))
		CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_POUNCE),0,BuddyPounceDlgProc, hContact);
	else
		CreateDialogParam(hInst,MAKEINTRESOURCE(IDD_POUNCE_SIMPLE),0,BuddyPounceSimpleDlgProc, hContact);
	return 0;
}

INT_PTR AddSimpleMessage(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	TCHAR* message = (TCHAR*)lParam;
	time_t today = time(NULL);
	db_set_ws(hContact, modname, "PounceMsg", message);
	db_set_w(hContact, modname, "SendIfMyStatusIsFLAG", (WORD)db_get_w(NULL, modname, "SendIfMyStatusIsFLAG",1));
	db_set_w(hContact, modname, "SendIfTheirStatusIsFLAG", (WORD)db_get_w(NULL, modname, "SendIfTheirStatusIsFLAG",1));
	db_set_b(hContact, modname, "Reuse", (BYTE)db_get_b(NULL, modname, "Reuse",0));
	db_set_b(hContact, modname, "GiveUpDays", (BYTE)db_get_b(NULL, modname, "GiveUpDays",0));
	db_set_dw(hContact, modname, "GiveUpDate", (DWORD)(db_get_b(hContact, modname, "GiveUpDays",0)*SECONDSINADAY));
	return 0;
}

INT_PTR AddToPounce(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	TCHAR* message = (TCHAR*)lParam;
	DBVARIANT dbv;
	if (!db_get_ts(hContact, modname, "PounceMsg",&dbv))
	{
		TCHAR* newPounce = (TCHAR*)mir_alloc(mir_tstrlen(dbv.ptszVal) + mir_tstrlen(message) + 1);
		if (!newPounce) return 1;
		_tcscpy(newPounce, dbv.ptszVal);
		_tcscat(newPounce, message);
		db_set_ws(hContact, modname, "PounceMsg", newPounce);
		mir_free(newPounce);
		db_free(&dbv);
	}
	else AddSimpleMessage(hContact, (LPARAM)message);
	return 0;
}

//===========================
// Load (hook ModulesLoaded)
//===========================
extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfo);
	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, UserOnlineSettingChanged); 
	HookEvent(ME_OPT_INITIALISE, BuddyPounceOptInit);
	HookEvent(ME_PROTO_ACK, MsgAck);
	CreateServiceFunction("BuddyPounce/MenuCommand", BuddyPounceMenuCommand);
	hWindowList = WindowList_Create();

	/*     service funcitons for other devs...					*/
	CreateServiceFunction("BuddyPounce/AddSimplePounce", AddSimpleMessage); // add a simple pounce to a contact
	CreateServiceFunction("BuddyPounce/AddToPounce", AddToPounce); // add to the exsisitng pounce, if there isnt 1 then add a new simple pounce.
	/* ******************************************************** */

	return 0; 
}


extern "C" __declspec(dllexport) int Unload(void)
{
	WindowList_Destroy(hWindowList);
	return 0;
}

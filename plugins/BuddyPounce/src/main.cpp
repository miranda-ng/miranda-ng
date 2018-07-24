#include "stdafx.h"

CMPlugin g_plugin;
MWindowList hWindowList;

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
	// {A9E9C114-84B9-434B-A3D5-89921D39DDFF}
	{0xa9e9c114, 0x84b9, 0x434b, {0xa3, 0xd5, 0x89, 0x92, 0x1d, 0x39, 0xdd, 0xff}}
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////
// MainInit

int MainInit(WPARAM, LPARAM)
{
	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x719c1596, 0xb0fd, 0x4c74, 0xb7, 0xe4, 0xeb, 0x22, 0xf4, 0x99, 0xd7, 0x68);
	mi.position = 10;
	mi.hIcolibItem = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_POUNCE));
	mi.name.a = LPGEN("&Buddy Pounce");
	mi.pszService = "BuddyPounce/MenuCommand";
	Menu_AddContactMenuItem(&mi);
	return 0;
}

int MsgAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (ack && ack->type == ACKTYPE_MESSAGE) {
		if (ack->hProcess == (HANDLE)WindowList_Find(hWindowList, ack->hContact)) {
			if (db_get_b(NULL, MODULENAME, "ShowDeliveryMessages", 1))
				CreateMessageAcknowlegedWindow(ack->hContact, ack->result == ACKRESULT_SUCCESS);
			if (ack->result == ACKRESULT_SUCCESS) {
				// wrtie it to the DB
				DBEVENTINFO dbei = {};
				DBVARIANT dbv;
				int reuse = db_get_b(ack->hContact, MODULENAME, "Reuse", 0);
				if (!db_get_ws(ack->hContact, MODULENAME, "PounceMsg", &dbv) && (dbv.pwszVal[0] != '\0')) {
					T2Utf pszUtf(dbv.pwszVal);
					dbei.eventType = EVENTTYPE_MESSAGE;
					dbei.flags = DBEF_UTF | DBEF_SENT;
					dbei.szModule = (char*)ack->szModule;
					dbei.timestamp = time(0);
					dbei.cbBlob = (int)mir_strlen(pszUtf) + 1;
					dbei.pBlob = (PBYTE)(char*)pszUtf;
					db_event_add(ack->hContact, &dbei);
				}
				// check to reuse
				if (reuse > 1)
					db_set_b(ack->hContact, MODULENAME, "Reuse", (BYTE)(reuse - 1));
				else {
					db_set_b(ack->hContact, MODULENAME, "Reuse", 0);
					db_set_ws(ack->hContact, MODULENAME, "PounceMsg", L"");
				}
			}
			WindowList_Remove(hWindowList, (HWND)ack->hProcess);
		}
	}
	return 0;
}

int BuddyPounceOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szGroup.w = LPGENW("Message sessions");
	odp.szTitle.w = LPGENW("Buddy Pounce");
	odp.pfnDlgProc = BuddyPounceOptionsDlgProc;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

int statusCheck(int statusFlag, int status)
{
	if (statusFlag == ANY || !statusFlag) return 1;
	switch (status) {
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
	time_t curtime = time(nullptr);
	if (!db_get_b(hContact, MODULENAME, "GiveUpDays", 0))
		return 1;
	if (db_get_b(hContact, MODULENAME, "GiveUpDays", 0) && (abs((time_t)db_get_dw(hContact, MODULENAME, "GiveUpDate", 0)) > curtime))
		return 1;
	return 0;
}

void SendPounce(wchar_t *text, MCONTACT hContact)
{
	if (HANDLE hSendId = (HANDLE)ProtoChainSend(hContact, PSS_MESSAGE, 0, T2Utf(text)))
		WindowList_Add(hWindowList, (HWND)hSendId, hContact);
}

int UserOnlineSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	char *szProto = GetContactProto(hContact);
	if (hContact == NULL || strcmp(cws->szSetting, "Status")) return 0;
	if (szProto && (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM)) {
		int newStatus = cws->value.wVal;
		int oldStatus = db_get_w(hContact, "UserOnline", "OldStatus", ID_STATUS_OFFLINE);

		if (newStatus != oldStatus && hContact != NULL && newStatus != ID_STATUS_OFFLINE) {
			DBVARIANT dbv;
			if (!db_get_ws(hContact, MODULENAME, "PounceMsg", &dbv) && (dbv.pwszVal[0] != '\0')) {
				// check my status
				if (statusCheck(db_get_w(hContact, MODULENAME, "SendIfMyStatusIsFLAG", 0), Proto_GetStatus(szProto))
					// check the contacts status
					&& statusCheck(db_get_w(hContact, MODULENAME, "SendIfTheirStatusIsFLAG", 0), newStatus)) {
					// check if we r giving up after x days
					if (CheckDate(hContact)) {
						if (db_get_w(hContact, MODULENAME, "ConfirmTimeout", 0)) {
							SendPounceDlgProcStruct *spdps = (SendPounceDlgProcStruct *)mir_alloc(sizeof(SendPounceDlgProcStruct));
							wchar_t *message = mir_wstrdup(dbv.pwszVal); // will get free()ed in the send confirm window proc
							spdps->hContact = hContact;
							spdps->message = message;
							CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_CONFIRMSEND), nullptr, SendPounceDlgProc, (LPARAM)spdps);
							// set the confirmation window to send the msg when the timeout is done
							mir_free(message);
						}
						else SendPounce(dbv.pwszVal, hContact);
					}
				}
				db_free(&dbv);
			}
		}
	}
	return 0;
}

INT_PTR BuddyPounceMenuCommand(WPARAM hContact, LPARAM)
{
	if (db_get_b(NULL, MODULENAME, "UseAdvanced", 0) || db_get_b(hContact, MODULENAME, "UseAdvanced", 0))
		CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_POUNCE), nullptr, BuddyPounceDlgProc, hContact);
	else
		CreateDialogParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_POUNCE_SIMPLE), nullptr, BuddyPounceSimpleDlgProc, hContact);
	return 0;
}

INT_PTR AddSimpleMessage(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	wchar_t* message = (wchar_t*)lParam;
	db_set_ws(hContact, MODULENAME, "PounceMsg", message);
	db_set_w(hContact, MODULENAME, "SendIfMyStatusIsFLAG", (WORD)db_get_w(NULL, MODULENAME, "SendIfMyStatusIsFLAG", 1));
	db_set_w(hContact, MODULENAME, "SendIfTheirStatusIsFLAG", (WORD)db_get_w(NULL, MODULENAME, "SendIfTheirStatusIsFLAG", 1));
	db_set_b(hContact, MODULENAME, "Reuse", (BYTE)db_get_b(NULL, MODULENAME, "Reuse", 0));
	db_set_b(hContact, MODULENAME, "GiveUpDays", (BYTE)db_get_b(NULL, MODULENAME, "GiveUpDays", 0));
	db_set_dw(hContact, MODULENAME, "GiveUpDate", (DWORD)(db_get_b(hContact, MODULENAME, "GiveUpDays", 0)*SECONDSINADAY));
	return 0;
}

INT_PTR AddToPounce(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	wchar_t* message = (wchar_t*)lParam;
	DBVARIANT dbv;
	if (!db_get_ws(hContact, MODULENAME, "PounceMsg", &dbv))
	{
		wchar_t* newPounce = (wchar_t*)mir_alloc(mir_wstrlen(dbv.pwszVal) + mir_wstrlen(message) + 1);
		if (!newPounce) return 1;
		mir_wstrcpy(newPounce, dbv.pwszVal);
		mir_wstrcat(newPounce, message);
		db_set_ws(hContact, MODULENAME, "PounceMsg", newPounce);
		mir_free(newPounce);
		db_free(&dbv);
	}
	else AddSimpleMessage(hContact, (LPARAM)message);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Load (hook ModulesLoaded)

int CMPlugin::Load()
{
	HookEvent(ME_SYSTEM_MODULESLOADED, MainInit);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, UserOnlineSettingChanged);
	HookEvent(ME_OPT_INITIALISE, BuddyPounceOptInit);
	HookEvent(ME_PROTO_ACK, MsgAck);

	CreateServiceFunction("BuddyPounce/MenuCommand", BuddyPounceMenuCommand);

	hWindowList = WindowList_Create();

	// service funcitons for other devs...					*/
	CreateServiceFunction("BuddyPounce/AddSimplePounce", AddSimpleMessage); // add a simple pounce to a contact
	CreateServiceFunction("BuddyPounce/AddToPounce", AddToPounce); // add to the exsisitng pounce, if there isnt 1 then add a new simple pounce.
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	WindowList_Destroy(hWindowList);
	return 0;
}

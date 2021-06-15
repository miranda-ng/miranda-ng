#include "stdafx.h"

CMPlugin g_plugin;
MWindowList hWindowList;

static HGENMENU g_hMenuItem;

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
	PLUGIN<CMPlugin>("BuddyPounce", pluginInfoEx),
	bUseAdvanced(m_szModuleName, "UseAdvanced", false),
	bShowDelivery(m_szModuleName, "ShowDeliveryMessages", true)
{
}

/////////////////////////////////////////////////////////////////////////////////////////

int MsgAck(WPARAM, LPARAM lParam)
{
	ACKDATA *ack = (ACKDATA*)lParam;

	if (ack && ack->type == ACKTYPE_MESSAGE) {
		if (ack->hProcess == (HANDLE)WindowList_Find(hWindowList, ack->hContact)) {
			if (g_plugin.bShowDelivery)
				CreateMessageAcknowlegedWindow(ack->hContact, ack->result == ACKRESULT_SUCCESS);
			
			if (ack->result == ACKRESULT_SUCCESS) {
				// wrtie it to the DB
				DBVARIANT dbv;
				int reuse = g_plugin.getByte(ack->hContact, "Reuse");
				if (!g_plugin.getWString(ack->hContact, "PounceMsg", &dbv) && (dbv.pwszVal[0] != '\0')) {
					T2Utf pszUtf(dbv.pwszVal);

					DBEVENTINFO dbei = {};
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
					g_plugin.setByte(ack->hContact, "Reuse", (BYTE)(reuse - 1));
				else {
					g_plugin.setByte(ack->hContact, "Reuse", 0);
					g_plugin.setWString(ack->hContact, "PounceMsg", L"");
				}
			}
			WindowList_Remove(hWindowList, (HWND)ack->hProcess);
		}
	}
	return 0;
}

static int PrebuildContactMenu(WPARAM hContact, LPARAM)
{
	Menu_ShowItem(g_hMenuItem, (CallProtoService(Proto_GetBaseAccountName(hContact), PS_GETCAPS, PFLAGNUM_1) & PF1_IM) != 0);
	return 0;
}

static int BuddyPounceOptInit(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.flags = ODPF_BOLDGROUPS;
	odp.szGroup.a = LPGEN("Message sessions");
	odp.szTitle.a = LPGEN("Buddy Pounce");
	odp.pDialog = new COptionsDlg(IDD_OPTIONS);
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
	}
	return 0;
}
int CheckDate(MCONTACT hContact)
{
	time_t curtime = time(nullptr);
	if (!g_plugin.getByte(hContact, "GiveUpDays", 0))
		return 1;
	if (g_plugin.getByte(hContact, "GiveUpDays", 0) && (abs((time_t)g_plugin.getDword(hContact, "GiveUpDate", 0)) > curtime))
		return 1;
	return 0;
}

void SendPounce(wchar_t *text, MCONTACT hContact)
{
	if (HANDLE hSendId = (HANDLE)ProtoChainSend(hContact, PSS_MESSAGE, 0, T2Utf(text)))
		WindowList_Add(hWindowList, (HWND)hSendId, hContact);
}

/////////////////////////////////////////////////////////////////////////////////////////

class CSendConfirmDlg : public CDlgBase
{
	int timeout;
	MCONTACT hContact;
	ptrW wszMessage;

	CTimer timer;

public:
	CSendConfirmDlg(MCONTACT _1, wchar_t *_2) :
		CDlgBase(g_plugin, IDD_CONFIRMSEND),
		hContact(_1),
		wszMessage(_2),
		timer(this, 1)
	{
		timeout = g_plugin.getWord(hContact, "ConfirmTimeout");
	}

	bool OnInitDialog() override
	{
		SetDlgItemText(m_hwnd, IDC_MESSAGE, wszMessage);
		timer.Start(1000);
		OnTimer(0);
		return true;
	}

	bool OnApply() override
	{
		SendPounce(wszMessage, hContact);
		return true;
	}

	void onTimer(CTimer *)
	{
		wchar_t message[1024];
		mir_snwprintf(message, TranslateT("Pounce being sent to %s in %d seconds"), Clist_GetContactDisplayName(hContact), timeout);
		SetDlgItemText(m_hwnd, LBL_CONTACT, message);

		timeout--;
		if (timeout < 0) {
			timer.Stop();
			SendPounce(message, hContact);
			Close();
		}
	}
};

static int UserOnlineSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;

	char *szProto = Proto_GetBaseAccountName(hContact);
	if (hContact == NULL || strcmp(cws->szSetting, "Status")) return 0;
	if (szProto && (CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_IM)) {
		int newStatus = cws->value.wVal;
		int oldStatus = db_get_w(hContact, "UserOnline", "OldStatus", ID_STATUS_OFFLINE);

		if (newStatus != oldStatus && newStatus != ID_STATUS_OFFLINE) {
			ptrW wszMessage(g_plugin.getWStringA(hContact, "PounceMsg"));
			if (mir_wstrlen(wszMessage)) {
				// check my status
				if (statusCheck(g_plugin.getWord(hContact, "SendIfMyStatusIsFLAG", 0), Proto_GetStatus(szProto))
					// check the contacts status
					&& statusCheck(g_plugin.getWord(hContact, "SendIfTheirStatusIsFLAG", 0), newStatus)) {
					// check if we r giving up after x days
					if (CheckDate(hContact)) {
						if (g_plugin.getWord(hContact, "ConfirmTimeout"))
							(new CSendConfirmDlg(hContact, wszMessage.detach()))->Create();
						else
							SendPounce(wszMessage, hContact);
					}
				}
			}
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

INT_PTR BuddyPounceMenuCommand(WPARAM hContact, LPARAM)
{
	if (g_plugin.bUseAdvanced || g_plugin.getByte(hContact, "UseAdvanced"))
		(new CBuddyPounceDlg(hContact))->Create();
	else
		(new CBuddyPounceSimpleDlg(hContact))->Create();
	return 0;
}

INT_PTR AddSimpleMessage(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	wchar_t* message = (wchar_t*)lParam;
	g_plugin.setWString(hContact, "PounceMsg", message);
	g_plugin.setWord(hContact, "SendIfMyStatusIsFLAG", (WORD)g_plugin.getWord("SendIfMyStatusIsFLAG", 1));
	g_plugin.setWord(hContact, "SendIfTheirStatusIsFLAG", (WORD)g_plugin.getWord("SendIfTheirStatusIsFLAG", 1));
	g_plugin.setByte(hContact, "Reuse", (BYTE)g_plugin.getByte("Reuse", 0));
	g_plugin.setByte(hContact, "GiveUpDays", (BYTE)g_plugin.getByte("GiveUpDays", 0));
	g_plugin.setDword(hContact, "GiveUpDate", (DWORD)(g_plugin.getByte(hContact, "GiveUpDays", 0)*SECONDSINADAY));
	return 0;
}

INT_PTR AddToPounce(WPARAM wParam, LPARAM lParam)
{
	MCONTACT hContact = wParam;
	wchar_t* message = (wchar_t*)lParam;
	DBVARIANT dbv;
	if (!g_plugin.getWString(hContact, "PounceMsg", &dbv)) {
		wchar_t* newPounce = (wchar_t*)mir_alloc(mir_wstrlen(dbv.pwszVal) + mir_wstrlen(message) + 1);
		if (!newPounce) return 1;
		mir_wstrcpy(newPounce, dbv.pwszVal);
		mir_wstrcat(newPounce, message);
		g_plugin.setWString(hContact, "PounceMsg", newPounce);
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
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PrebuildContactMenu);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, UserOnlineSettingChanged);
	HookEvent(ME_OPT_INITIALISE, BuddyPounceOptInit);
	HookEvent(ME_PROTO_ACK, MsgAck);

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0x719c1596, 0xb0fd, 0x4c74, 0xb7, 0xe4, 0xeb, 0x22, 0xf4, 0x99, 0xd7, 0x68);
	mi.position = 10;
	mi.hIcolibItem = LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_POUNCE));
	mi.name.a = LPGEN("&Buddy Pounce");
	mi.pszService = "BuddyPounce/MenuCommand";
	g_hMenuItem = Menu_AddContactMenuItem(&mi);
	CreateServiceFunction(mi.pszService, BuddyPounceMenuCommand);

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

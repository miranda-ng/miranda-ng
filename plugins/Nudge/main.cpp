#include "headers.h"
#include "main.h"
#include "shake.h"
#include "m_msg_buttonsbar.h"


int nProtocol = 0;
static HANDLE g_hEventModulesLoaded = NULL, hEventOptionsInitialize = NULL, g_hIcon = NULL, g_hEventDbWindowEvent = NULL, g_hEventToolbarLoaded = NULL, g_hEventButtonPressed = NULL, g_hEventAccountsChanged = NULL;
HINSTANCE hInst;

NudgeElementList *NudgeList = NULL;
CNudgeElement DefaultNudge;
CShake shake;
CNudge GlobalNudge;



int hLangpack = 0;


//========================
//  MirandaPluginInfo
//========================
PLUGININFOEX pluginInfo={
	sizeof(PLUGININFOEX),
	"Nudge",
	 PLUGIN_MAKE_VERSION(0,0,2,0),
	"Plugin to shake the clist and chat window",
	"Tweety/GouZ, FREAK_THEMIGHTY",
	"francois.mean@skynet.be, Sylvain.gougouzian@gmail.com, wishmaster51@googlemail.com",
	"copyright to the miranda community",
	"http://addons.miranda-im.org/",		// www
	UNICODE_AWARE,
	0,		//doesn't replace anything built-in
	{ 0xe47cc215, 0xd28, 0x462d, { 0xa0, 0xf6, 0x3a, 0xe4, 0x44, 0x3d, 0x29, 0x26 } } // {E47CC215-0D28-462D-A0F6-3AE4443D2926}
};

INT_PTR NudgeShowMenu(WPARAM wParam,LPARAM lParam)
{

	for(NudgeElementList *n = NudgeList;n != NULL; n = n->next)
	{
		if (!strcmp((char *) wParam,n->item.ProtocolName))
		{
			return n->item.ShowContactMenu(lParam != 0);
		}
	}
	return 0;
}

INT_PTR NudgeSend(WPARAM wParam,LPARAM lParam)
{

	char *protoName = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);
	int diff = time(NULL) - DBGetContactSettingDword((HANDLE) wParam, "Nudge", "LastSent", time(NULL)-30);

	if(diff < GlobalNudge.sendTimeSec)
	{
		TCHAR msg[500];
		mir_sntprintf(msg,500, TranslateT("You are not allowed to send too much nudge (only 1 each %d sec, %d sec left)"),GlobalNudge.sendTimeSec, 30 - diff);
		//MessageBox(NULL,msg,NULL,0);
		if(GlobalNudge.useByProtocol)
		{
			for(NudgeElementList *n = NudgeList;n != NULL; n = n->next)
			{
				if (!strcmp(protoName,n->item.ProtocolName))
				{
					Nudge_ShowPopup(n->item, (HANDLE) wParam, msg);
				}
			}
		}
		else
		{
			Nudge_ShowPopup(DefaultNudge, (HANDLE) wParam, msg);
		}
		return 0;
	}

	DBWriteContactSettingDword((HANDLE) wParam, "Nudge", "LastSent", time(NULL));

	if(GlobalNudge.useByProtocol)
	{
		NudgeElementList *n;
		for(n = NudgeList;n != NULL; n = n->next)
		{
			if (!strcmp(protoName,n->item.ProtocolName))
			{
				//if(n->item.showPopup)
				//	Nudge_ShowPopup(n->item, (HANDLE) wParam, n->item.senText);
				if(n->item.showStatus)
					Nudge_SentStatus(n->item, (HANDLE) wParam);
			}
		}
	}
	else
	{
		//if(DefaultNudge.showPopup)
		//	Nudge_ShowPopup(DefaultNudge, (HANDLE) wParam, DefaultNudge.senText);
		if(DefaultNudge.showStatus)
			Nudge_SentStatus(DefaultNudge, (HANDLE) wParam);
	}

	CallProtoService(protoName,"/SendNudge",wParam,lParam);
	return 0;
}

void OpenContactList()
{
	HWND hWnd = (HWND) CallService(MS_CLUI_GETHWND,0,0);
	ShowWindow(hWnd, SW_RESTORE);
	ShowWindow(hWnd, SW_SHOW);
}

int NudgeRecieved(WPARAM wParam,LPARAM lParam)
{

	char *protoName = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO,wParam,0);

	DWORD currentTimestamp = time(NULL);
	DWORD nudgeSentTimestamp = lParam ? (DWORD)lParam : currentTimestamp;

	int diff = currentTimestamp - DBGetContactSettingDword((HANDLE) wParam, "Nudge", "LastReceived", currentTimestamp-30);
	int diff2 = nudgeSentTimestamp - DBGetContactSettingDword((HANDLE) wParam, "Nudge", "LastReceived2", nudgeSentTimestamp-30);

	if(diff >= GlobalNudge.recvTimeSec)
		DBWriteContactSettingDword((HANDLE) wParam, "Nudge", "LastReceived", currentTimestamp);
	if(diff2 >= GlobalNudge.recvTimeSec)
		DBWriteContactSettingDword((HANDLE) wParam, "Nudge", "LastReceived2", nudgeSentTimestamp);

	if(GlobalNudge.useByProtocol)
	{
		for(NudgeElementList *n = NudgeList;n != NULL; n = n->next)
		{
			if (!strcmp(protoName,n->item.ProtocolName))
			{

				if(n->item.enabled)
				{
					if(n->item.useIgnoreSettings && CallService(MS_IGNORE_ISIGNORED,wParam,IGNOREEVENT_USERONLINE))
						return 0;

					DWORD Status = CallProtoService(protoName,PS_GETSTATUS,0,0);

					if ( ((n->item.statusFlags & NUDGE_ACC_ST0) && (Status<=ID_STATUS_OFFLINE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST1) && (Status==ID_STATUS_ONLINE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST2) && (Status==ID_STATUS_AWAY)) ||
						((n->item.statusFlags & NUDGE_ACC_ST3) && (Status==ID_STATUS_DND)) ||
						((n->item.statusFlags & NUDGE_ACC_ST4) && (Status==ID_STATUS_NA)) ||
						((n->item.statusFlags & NUDGE_ACC_ST5) && (Status==ID_STATUS_OCCUPIED)) ||
						((n->item.statusFlags & NUDGE_ACC_ST6) && (Status==ID_STATUS_FREECHAT)) ||
						((n->item.statusFlags & NUDGE_ACC_ST7) && (Status==ID_STATUS_INVISIBLE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST8) && (Status==ID_STATUS_ONTHEPHONE)) ||
						((n->item.statusFlags & NUDGE_ACC_ST9) && (Status==ID_STATUS_OUTTOLUNCH)))
					{
						if(diff >= GlobalNudge.recvTimeSec)
						{
							if(n->item.showPopup)
								Nudge_ShowPopup(n->item, (HANDLE) wParam, n->item.recText);
							if(n->item.openContactList)
								OpenContactList();
							if(n->item.shakeClist)
								ShakeClist(wParam,lParam);
							if(n->item.openMessageWindow)
								CallService(MS_MSG_SENDMESSAGET,wParam,0);
							if(n->item.shakeChat)
								ShakeChat(wParam,lParam);
							if(n->item.autoResend)
								mir_forkthread(AutoResendNudge,(void *)wParam);

							SkinPlaySound( n->item.NudgeSoundname );
						}
					}
					if(diff2 >= GlobalNudge.recvTimeSec)
					{
						if(n->item.showStatus)
							Nudge_ShowStatus(n->item, (HANDLE) wParam, nudgeSentTimestamp);
					}

				}
				break;
			}
		}
	}
	else
	{
		if(DefaultNudge.enabled)
		{
			if(DefaultNudge.useIgnoreSettings && CallService(MS_IGNORE_ISIGNORED,wParam,IGNOREEVENT_USERONLINE))
				return 0;
			DWORD Status = CallService(MS_CLIST_GETSTATUSMODE,0,0);
			if ( ((DefaultNudge.statusFlags & NUDGE_ACC_ST0) && (Status<=ID_STATUS_OFFLINE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST1) && (Status==ID_STATUS_ONLINE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST2) && (Status==ID_STATUS_AWAY)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST3) && (Status==ID_STATUS_DND)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST4) && (Status==ID_STATUS_NA)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST5) && (Status==ID_STATUS_OCCUPIED)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST6) && (Status==ID_STATUS_FREECHAT)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST7) && (Status==ID_STATUS_INVISIBLE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST8) && (Status==ID_STATUS_ONTHEPHONE)) ||
				((DefaultNudge.statusFlags & NUDGE_ACC_ST9) && (Status==ID_STATUS_OUTTOLUNCH)))
			{
				if(diff >= GlobalNudge.recvTimeSec)
				{
					if(DefaultNudge.showPopup)
						Nudge_ShowPopup(DefaultNudge, (HANDLE) wParam, DefaultNudge.recText);
					if(DefaultNudge.openContactList)
						OpenContactList();
					if(DefaultNudge.shakeClist)
						ShakeClist(wParam,lParam);
					if(DefaultNudge.openMessageWindow)
						CallService(MS_MSG_SENDMESSAGET,wParam,0);
					if(DefaultNudge.shakeChat)
						ShakeChat(wParam,lParam);
					if(DefaultNudge.autoResend)
						mir_forkthread(AutoResendNudge,(void *)wParam);

					SkinPlaySound( DefaultNudge.NudgeSoundname );
				}
			}
			if(diff2 >= GlobalNudge.recvTimeSec)
			{
				if(DefaultNudge.showStatus)
					Nudge_ShowStatus(DefaultNudge, (HANDLE) wParam, nudgeSentTimestamp);
			}
		}
	}
	return 0;
}

extern "C" BOOL WINAPI DllMain(HINSTANCE hinstDLL,DWORD fdwReason,LPVOID lpvReserved)
{
	hInst=hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) PLUGININFOEX* MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

static INT_PTR CALLBACK DlgProcOptsTrigger(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

    switch (msg) {
	case WM_INITDIALOG: {
		// lParam = (LPARAM)(DWORD)actionID or 0 if this is a new trigger entry
		BOOL bshakeClist,bshakeChat;

		DWORD actionID = (DWORD)lParam;
        TranslateDialogDefault(hwnd);
		// Initialize the dialog according to the action ID
		bshakeClist = DBGetActionSettingByte(actionID, NULL, "Nudge", "ShakeClist",FALSE);
		bshakeChat = DBGetActionSettingByte(actionID, NULL, "Nudge", "ShakeChat",FALSE);
		CheckDlgButton(hwnd, IDC_TRIGGER_SHAKECLIST, bshakeClist ? BST_CHECKED : BST_UNCHECKED);
		CheckDlgButton(hwnd, IDC_TRIGGER_SHAKECHAT, bshakeChat ? BST_CHECKED : BST_UNCHECKED);
        break;
						}

	case TM_ADDACTION: {
		// save your settings
		// wParam = (WPARAM)(DWORD)actionID
		bool bshakeClist,bshakeChat;

		DWORD actionID = (DWORD)wParam;
		bshakeClist = (IsDlgButtonChecked(hwnd,IDC_TRIGGER_SHAKECLIST)==BST_CHECKED);
		bshakeChat = (IsDlgButtonChecked(hwnd,IDC_TRIGGER_SHAKECHAT)==BST_CHECKED);
		DBWriteActionSettingByte(actionID, NULL, "Nudge", "ShakeClist",bshakeClist);
		DBWriteActionSettingByte(actionID, NULL, "Nudge", "ShakeChat",bshakeChat);
		break;
					   }
	}

    return FALSE;
}

int TriggerActionRecv( DWORD actionID, REPORTINFO *ri)
{
	// check how to process this call
	if (ri->flags&ACT_PERFORM) {
		BOOL bshakeClist,bshakeChat;
		HANDLE hContact = ((ri->td!=NULL)&&(ri->td->dFlags&DF_CONTACT))?ri->td->hContact:NULL;
		bshakeClist = DBGetActionSettingByte(actionID, NULL, "Nudge", "ShakeClist",FALSE);
		bshakeChat = DBGetActionSettingByte(actionID, NULL, "Nudge", "ShakeChat",FALSE);

		if(bshakeClist)
			ShakeClist(NULL,NULL);
		if(bshakeChat && (hContact != NULL))
			ShakeChat((WPARAM)hContact,NULL);

	/*	// Actually show the message box
		DBVARIANT dbv;
		TCHAR *tszMsg;

		// Retrieve the correct settings for this action ID
		if (!DBGetActionSettingTString(actionID, NULL, MODULENAME, SETTING_TEXT, &dbv)) {
			// Parse by Variables, if available (notice extratext and subject are given).
			tszMsg = variables_parsedup(dbv.ptszVal, ((ri->td!=NULL)&&(ri->td->dFlags&DF_TEXT))?ri->td->tszText:NULL, ((ri->td!=NULL)&&(ri->td->dFlags&DF_CONTACT))?ri->td->hContact:NULL);
			if (tszMsg != NULL) {
				// Show the message box
				MessageBox(NULL, tszMsg, TranslateT("ExampleAction"), MB_OK);
				free(tszMsg);
			}
			DBFreeVariant(&dbv);
		}
		*/
	}
	if (ri->flags&ACT_CLEANUP) { // request to delete all associated settings
		RemoveAllActionSettings(actionID, "Nudge");
	}
	return FALSE;
}

int TriggerActionSend( DWORD actionID, REPORTINFO *ri)
{
	if (ri->flags&ACT_PERFORM) {
		HANDLE hContact = ((ri->td!=NULL)&&(ri->td->dFlags&DF_CONTACT))?ri->td->hContact:NULL;
		if(hContact != NULL)
			NudgeSend((WPARAM)hContact,NULL);
	}

	return FALSE;
}

void LoadProtocols(void)
{
	//Load the default nudge
	mir_snprintf(DefaultNudge.ProtocolName,sizeof(DefaultNudge.ProtocolName),"Default");
	mir_snprintf(DefaultNudge.NudgeSoundname,sizeof(DefaultNudge.NudgeSoundname),"Nudge : Default");
	SkinAddNewSoundEx( DefaultNudge.NudgeSoundname, LPGEN("Nudge"), LPGEN("Default Nudge"));
	DefaultNudge.Load();

	GlobalNudge.Load();

	int numberOfProtocols = 0;
	PROTOACCOUNT **ppProtocolDescriptors;
	INT_PTR ret = ProtoEnumAccounts(&numberOfProtocols,&ppProtocolDescriptors);
	if(ret == 0)
	{
		for(int i = 0; i < numberOfProtocols ; i++)
		{
			Nudge_AddAccount(ppProtocolDescriptors[i]);
		}

	}

	shake.Load();

	/*CNudgeElement *n;
	for(n = NudgeList;n != NULL; n = n->next)
	{
		MessageBox(NULL,n->ProtocolName,n->NudgeSoundname,0);
	}*/
}

void RegisterToTrigger(void)
{
	if ( ServiceExists(MS_TRIGGER_REGISTERACTION))
	{
		ACTIONREGISTER ar;
		ZeroMemory(&ar, sizeof(ar));
		ar.cbSize = sizeof(ar);
		ar.hInstance = hInst;
		ar.flags = ARF_TCHAR|ARF_FUNCTION;
		ar.actionFunction = TriggerActionRecv;
		ar.pfnDlgProc = DlgProcOptsTrigger;
		ar.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TRIGGER);
		ar.pszName = Translate("Nudge : Shake contact list/chat window");

		// register the action
		CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);

		ar.actionFunction = TriggerActionSend;
		ar.pszName = Translate("Nudge : Send a nudge");
		ar.pfnDlgProc = NULL;
		ar.pszTemplate = NULL;

		// register the action
		CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ar);
	}
}

void RegisterToDbeditorpp(void)
{
    // known modules list
    if (ServiceExists("DBEditorpp/RegisterSingleModule"))
        CallService("DBEditorpp/RegisterSingleModule", (WPARAM)"Nudge", 0);
}

void LoadIcons(void)
{
	//Load icons
	SKINICONDESC sid = {0};
	TCHAR szFilename[MAX_PATH];
	GetModuleFileName(hInst,szFilename,MAX_PATH);

	sid.cbSize = sizeof(sid);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszSection = LPGENT("Nudge");
	sid.ptszDefaultFile = szFilename;

	sid.pszName = "Nudge_Default";
	sid.ptszDescription = LPGENT("Nudge as Default");
	sid.iDefaultIndex = -IDI_NUDGE;
	sid.hDefaultIcon =  LoadIcon(hInst,MAKEINTRESOURCE(IDI_NUDGE));
	g_hIcon = Skin_AddIcon(&sid);
}

// Nudge support
static int TabsrmmButtonPressed(WPARAM wParam, LPARAM lParam)
{
	CustomButtonClickData *cbcd=(CustomButtonClickData *)lParam;

	if (!strcmp(cbcd->pszModule,"Nudge"))
		NudgeSend(wParam, 0);

	return 0;
}

static int TabsrmmButtonInit(WPARAM wParam, LPARAM lParam)
{
	BBButton bbd = {0};

	bbd.cbSize = sizeof(BBButton);
	bbd.pszModuleName = "Nudge";
	bbd.ptszTooltip = LPGENT("Send Nudge");
	bbd.dwDefPos = 300;
	bbd.bbbFlags = BBBF_ISIMBUTTON|BBBF_ISLSIDEBUTTON|BBBF_CANBEHIDDEN;
	bbd.hIcon = g_hIcon;
	bbd.dwButtonID = 6000;
	bbd.iButtonWidth = 0;
	CallService (MS_BB_ADDBUTTON, 0, (LPARAM)&bbd);

	return 0;
}

void HideNudgeButton(HANDLE hContact)
{
	char str[MAXMODULELABELLENGTH + 12] = {0};
	char *szProto = (char*) CallService(MS_PROTO_GETCONTACTBASEPROTO,(WPARAM)hContact,0);
	mir_snprintf(str,MAXMODULELABELLENGTH + 12,"%s/SendNudge", szProto);

	if (!ServiceExists(str))
    {
      BBButton bbd={0};
      bbd.cbSize=sizeof(BBButton);
      bbd.bbbFlags=BBSF_HIDDEN|BBSF_DISABLED;
      bbd.pszModuleName="Nudge";
	  bbd.dwButtonID = 6000;
      CallService(MS_BB_SETBUTTONSTATE, (WPARAM)hContact, (LPARAM)&bbd);
    }
}

static int ContactWindowOpen(WPARAM wparam,LPARAM lParam)
{
   MessageWindowEventData *MWeventdata = (MessageWindowEventData*)lParam;

   if(MWeventdata->uType == MSG_WINDOW_EVT_OPENING&&MWeventdata->hContact)
   {
      HideNudgeButton(MWeventdata->hContact);
   }
   return 0;
}

int ModulesLoaded(WPARAM,LPARAM)
{
	RegisterToTrigger();
	RegisterToDbeditorpp();
	LoadProtocols();
	LoadIcons();
	LoadPopupClass();

	g_hEventToolbarLoaded = HookEvent(ME_MSG_TOOLBARLOADED, TabsrmmButtonInit);
	if (g_hEventToolbarLoaded)
	{
		g_hEventButtonPressed = HookEvent(ME_MSG_BUTTONPRESSED, TabsrmmButtonPressed);
		g_hEventDbWindowEvent = HookEvent(ME_MSG_WINDOWEVENT,ContactWindowOpen);
	}
	return 0;
}

int AccListChanged(WPARAM wParam,LPARAM lParam)
{
	PROTOACCOUNT *proto = (PROTOACCOUNT*) wParam;
	if (proto==NULL)
		return 0;

	switch (lParam)
	{
		case PRAC_ADDED:
			Nudge_AddAccount(proto);
			break;
	}
	return 0;
}

HANDLE hShakeClist=NULL,hShakeChat=NULL,hNudgeSend=NULL,hNudgeShowMenu=NULL;
extern "C" int __declspec(dllexport) Load(void)
{

	mir_getLP(&pluginInfo);

	g_hEventModulesLoaded = HookEvent(ME_SYSTEM_MODULESLOADED,ModulesLoaded);
	g_hEventAccountsChanged = HookEvent(ME_PROTO_ACCLISTCHANGED,AccListChanged);
	hEventOptionsInitialize = HookEvent(ME_OPT_INITIALISE, NudgeOptInit);

	//Create function for plugins
	hShakeClist=CreateServiceFunction(MS_SHAKE_CLIST,ShakeClist);
	hShakeChat=CreateServiceFunction(MS_SHAKE_CHAT,ShakeChat);
	hNudgeSend=CreateServiceFunction(MS_NUDGE_SEND,NudgeSend);
	hNudgeShowMenu=CreateServiceFunction(MS_NUDGE_SHOWMENU,NudgeShowMenu);
	return 0;
}

extern "C" int __declspec(dllexport) Unload(void)
{
	if(g_hEventToolbarLoaded) UnhookEvent(g_hEventToolbarLoaded);
	if(g_hEventDbWindowEvent) UnhookEvent(g_hEventButtonPressed);
	if(g_hEventDbWindowEvent) UnhookEvent(g_hEventDbWindowEvent);

	UnhookEvent(g_hEventModulesLoaded);
	UnhookEvent(g_hEventAccountsChanged);
	UnhookEvent(hEventOptionsInitialize);

	DestroyServiceFunction(hShakeClist);
	DestroyServiceFunction(hShakeChat);
	DestroyServiceFunction(hNudgeSend);
	DestroyServiceFunction(hNudgeShowMenu);

	NudgeElementList* p = NudgeList;
	while ( p != NULL )
	{
		if(p->item.hEvent) UnhookEvent(p->item.hEvent);
		NudgeElementList* p1 = p->next;
		//free( p );
		delete p;
		p = p1;
	}
	return 0;
}

LRESULT CALLBACK NudgePopUpProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
	{
		case WM_COMMAND:
		{
			HANDLE hContact = PUGetContact(hWnd);
			CallService(MS_MSG_SENDMESSAGET, (WPARAM)hContact, 0);
			PUDeletePopUp(hWnd);
			break;
		}

		case WM_CONTEXTMENU:
			PUDeletePopUp(hWnd);
			break;
		case UM_FREEPLUGINDATA:
			//Here we'd free our own data, if we had it.
			return FALSE;
		case UM_INITPOPUP:
			break;
		case UM_DESTROYPOPUP:
			break;
		case WM_NOTIFY:
		default:
			break;
	}
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

void LoadPopupClass()
{
	if(ServiceExists(MS_POPUP_REGISTERCLASS))
	{
		POPUPCLASS ppc = {0};
		ppc.cbSize = sizeof(ppc);
		ppc.flags = PCF_TCHAR;
		ppc.pszName = "nudge";
		ppc.ptszDescription = LPGENT("Show Nudge");
		ppc.hIcon = (HICON) CallService(MS_SKIN2_GETICONBYHANDLE,0,(LPARAM)g_hIcon);
		ppc.colorBack = NULL;
		ppc.colorText = NULL;
		ppc.iSeconds = 0;
		ppc.PluginWindowProc = NudgePopUpProc;
		CallService(MS_POPUP_REGISTERCLASS,0,(LPARAM)&ppc);
	}
}

int Preview()
{
	HANDLE hContact = (HANDLE) CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	if ( GlobalNudge.useByProtocol )
	{
		for(NudgeElementList *n = NudgeList;n != NULL; n = n->next)
		{
			if(n->item.enabled)
			{
				SkinPlaySound( n->item.NudgeSoundname );
				if(n->item.showPopup)
					Nudge_ShowPopup(n->item, hContact, n->item.recText);
				if(n->item.openContactList)
					OpenContactList();
				if(n->item.shakeClist)
					ShakeClist(0,0);
				if(n->item.openMessageWindow)
					CallService(MS_MSG_SENDMESSAGET,(WPARAM)hContact,NULL);
				if(n->item.shakeChat)
					ShakeChat((WPARAM)hContact,(LPARAM)time(NULL));
			}
		}
	}
	else
	{
		if(DefaultNudge.enabled)
		{
			SkinPlaySound( DefaultNudge.NudgeSoundname );
			if(DefaultNudge.showPopup)
				Nudge_ShowPopup(DefaultNudge, hContact, DefaultNudge.recText);
			if(DefaultNudge.openContactList)
				OpenContactList();
			if(DefaultNudge.shakeClist)
				ShakeClist(0,0);
			if(DefaultNudge.openMessageWindow)
				CallService(MS_MSG_SENDMESSAGET,(WPARAM)hContact,NULL);
			if(DefaultNudge.shakeChat)
				ShakeChat((WPARAM)hContact,(LPARAM)time(NULL));
		}
	}
	return 0;
}

void Nudge_ShowPopup(CNudgeElement n, HANDLE hContact, TCHAR * Message)
{
	hContact = Nudge_GethContact(hContact);
	TCHAR * lpzContactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,(WPARAM)hContact,GCDNF_TCHAR);

	if(ServiceExists(MS_POPUP_ADDPOPUPCLASS))
	{
		POPUPDATACLASS NudgePopUp = {0};
		NudgePopUp.cbSize = sizeof(NudgePopUp);
		NudgePopUp.hContact = hContact;
		NudgePopUp.ptszText = Message;
		NudgePopUp.ptszTitle = lpzContactName;
		NudgePopUp.pszClassName = "nudge";
		CallService(MS_POPUP_ADDPOPUPCLASS, 0, (LPARAM)&NudgePopUp);
	}
	else if(ServiceExists(MS_POPUP_ADDPOPUPT))
	{
		POPUPDATAT NudgePopUp = {0};

		NudgePopUp.lchContact = hContact;
		NudgePopUp.lchIcon = (HICON) CallService(MS_SKIN2_GETICON,0,(LPARAM) n.hIcoLibItem );
		NudgePopUp.colorBack = 0;
		NudgePopUp.colorText = 0;
		NudgePopUp.iSeconds = 0;
		NudgePopUp.PluginWindowProc = NudgePopUpProc;
		NudgePopUp.PluginData = (void *)1;

		//lstrcpy(NudgePopUp.lpzText, Translate(Message));
		lstrcpy(NudgePopUp.lptzText, Message);

		lstrcpy(NudgePopUp.lptzContactName, lpzContactName);

		CallService(MS_POPUP_ADDPOPUPT,(WPARAM)&NudgePopUp,0);
	}
	else
	{
		MessageBox(NULL,Message,lpzContactName,0);
	}
}

void Nudge_SentStatus(CNudgeElement n, HANDLE hContact)
{
	DBEVENTINFO NudgeEvent = { 0 };

	NudgeEvent.cbSize = sizeof(NudgeEvent);
	NudgeEvent.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

	char *buff = mir_utf8encodeT(n.senText);
	NudgeEvent.flags = DBEF_SENT | DBEF_UTF;

	NudgeEvent.timestamp = ( DWORD )time(NULL);
	NudgeEvent.eventType = EVENTTYPE_STATUSCHANGE;
	NudgeEvent.cbBlob = (DWORD)strlen(buff) + 1;
	NudgeEvent.pBlob = ( PBYTE ) buff;

	INT_PTR res = CallService( MS_MC_GETMETACONTACT, (WPARAM)hContact, 0 ); //try to retrieve the metacontact if some
	if(res != CALLSERVICE_NOTFOUND)
	{
		HANDLE hMetaContact = (HANDLE) res;
		if(hMetaContact != NULL) //metacontact
			CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&NudgeEvent);
	}


	CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&NudgeEvent);
}

void Nudge_ShowStatus(CNudgeElement n, HANDLE hContact, DWORD timestamp)
{
	DBEVENTINFO NudgeEvent = { 0 };

	NudgeEvent.cbSize = sizeof(NudgeEvent);
	NudgeEvent.szModule = (char*)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)hContact, 0);

	char *buff = mir_utf8encodeT(n.recText);
	NudgeEvent.flags = DBEF_UTF;

	NudgeEvent.timestamp = timestamp;
	NudgeEvent.eventType = EVENTTYPE_STATUSCHANGE;
	NudgeEvent.cbBlob = (DWORD)strlen(buff) + 1;
	NudgeEvent.pBlob = ( PBYTE ) buff;


	INT_PTR res = CallService( MS_MC_GETMETACONTACT, (WPARAM)hContact, 0 ); //try to retrieve the metacontact if some
	if(res != CALLSERVICE_NOTFOUND)
	{
		HANDLE hMetaContact = (HANDLE) res;
		if(hMetaContact != NULL) //metacontact
		{
			CallService(MS_DB_EVENT_ADD,(WPARAM)hMetaContact,(LPARAM)&NudgeEvent);
			NudgeEvent.flags |= DBEF_READ;
		}
	}

	CallService(MS_DB_EVENT_ADD,(WPARAM)hContact,(LPARAM)&NudgeEvent);
}

HANDLE Nudge_GethContact(HANDLE hContact)
{
	INT_PTR res = CallService( MS_MC_GETMETACONTACT, (WPARAM)hContact, 0 );
	if(res!=CALLSERVICE_NOTFOUND)
	{
		HANDLE hMetaContact = (HANDLE) res;
		if(hMetaContact!=NULL)
			return hMetaContact;
	}

	return hContact;
}

void Nudge_AddAccount(PROTOACCOUNT *proto)
{
	char str[MAXMODULELABELLENGTH + 10];
	mir_snprintf(str,sizeof(str),"%s/Nudge",proto->szModuleName);
	HANDLE hevent = HookEvent(str, NudgeRecieved);
	if(hevent == NULL)
		return;

	nProtocol ++;

	//Add a specific sound per protocol
	NudgeElementList *newNudge = new NudgeElementList;
	//newNudge = (NudgeElementList*) malloc(sizeof(NudgeElementList));
	mir_snprintf(newNudge->item.NudgeSoundname,sizeof(newNudge->item.NudgeSoundname),"%s: Nudge",proto->szModuleName);

	strcpy( newNudge->item.ProtocolName, proto->szProtoName );
	_tcscpy(newNudge->item.AccountName, proto->tszAccountName );

	newNudge->item.Load();

	newNudge->item.hEvent = hevent;

	TCHAR soundDesc[MAXMODULELABELLENGTH + 10];
	mir_sntprintf(soundDesc,sizeof(soundDesc),_T("Nudge for %s"),proto->tszAccountName);
	SkinAddNewSoundExT(newNudge->item.NudgeSoundname, LPGENT("Nudge"), soundDesc);

	newNudge->next = NudgeList;
	NudgeList = newNudge;

	char iconName[MAXMODULELABELLENGTH + 10];
	mir_snprintf(iconName,sizeof(iconName),"Nudge_%s",proto->szModuleName);

	TCHAR szFilename[MAX_PATH], iconDesc[MAXMODULELABELLENGTH + 10];
	GetModuleFileName(hInst,szFilename,MAX_PATH);
	mir_sntprintf(iconDesc, SIZEOF(iconDesc), TranslateT("Nudge for %s"), proto->tszAccountName);

	SKINICONDESC sid = {0};
	sid.cbSize = sizeof(sid);
	sid.flags = SIDF_ALL_TCHAR;
	sid.ptszSection = LPGENT("Nudge");
	sid.ptszDefaultFile = szFilename;
	sid.pszName = iconName;
	sid.ptszDescription = iconDesc;
	sid.iDefaultIndex = -IDI_NUDGE;
	sid.hDefaultIcon =  LoadIcon(hInst,MAKEINTRESOURCE(IDI_NUDGE));
	newNudge->item.hIcoLibItem = Skin_AddIcon(&sid);

	//Add contact menu entry
	CLISTMENUITEM mi = {0};
	mi.cbSize = sizeof(mi);
	mi.popupPosition = 500085000;
	mi.pszContactOwner = proto->szModuleName;
	mi.pszPopupName = proto->szModuleName;
	mi.flags = CMIF_NOTOFFLINE | CMIF_TCHAR | CMIF_ICONFROMICOLIB;
	mi.position = -500050004;
	mi.icolibItem = newNudge->item.hIcoLibItem;
	mi.ptszName = LPGENT( "Send &Nudge" );
	mi.pszService = MS_NUDGE_SEND;
	newNudge->item.hContactMenu = Menu_AddContactMenuItem(&mi);
}

void AutoResendNudge(void *wParam)
{

	Sleep(GlobalNudge.resendDelaySec * 1000);
	NudgeSend((WPARAM) wParam,NULL);
}

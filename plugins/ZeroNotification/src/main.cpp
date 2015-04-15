/*
A small Miranda plugin, by bidyut, updated by Maat.

Original plugin idea Anders Nilsson.
His plugin NoSound can be found at:
http://anders.nilsson.net/programs/miranda

Miranda can be found here:
http://miranda-ng.org/
*/

#include "common.h"

HINSTANCE hInst;

static HANDLE hEventSoundSettingChange, hEventStatusModeChange, hEventOptionsInitialize, hAckEvent, hSoundMenu;
HGENMENU noSoundMenu;
int hLangpack;

struct CheckBoxValues_t {
	DWORD style;
	TCHAR *szDescr;
};

static const struct CheckBoxValues_t statusValues[]={
	{PF2_ONLINE, TEXT("Online")},
	{PF2_SHORTAWAY, TEXT("Away")},
	{PF2_LONGAWAY, TEXT("NA")},
	{PF2_LIGHTDND, TEXT("Occupied")},
	{PF2_HEAVYDND, TEXT("DND")},
	{PF2_FREECHAT, TEXT("Free for chat")},
	{PF2_INVISIBLE, TEXT("Invisible")},
	{PF2_OUTTOLUNCH, TEXT("Out to lunch")},
	{PF2_ONTHEPHONE, TEXT("On the phone")}
};

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
	// {47D489D3-310D-4EF6-BD05-699FFFD5A4AA}
	{0x47d489d3, 0x310d, 0x4ef6, {0xbd, 0x5, 0x69, 0x9f, 0xff, 0xd5, 0xa4, 0xaa}}
};

extern "C" __declspec(dllexport) PLUGININFOEX * MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfoEx;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	hInst = hinstDLL;
	return TRUE;
}

static void FillCheckBoxTree(HWND hwndTree,const struct CheckBoxValues_t *values,int nValues,DWORD style)
{
	TVINSERTSTRUCT tvis;
	int i;

	tvis.hParent=NULL;
	tvis.hInsertAfter=TVI_LAST;
	tvis.item.mask=TVIF_PARAM|TVIF_TEXT|TVIF_STATE;
	for(i=0;i<nValues;i++) {
		tvis.item.lParam=values[i].style;
		tvis.item.pszText = TranslateTS(values[i].szDescr);
		tvis.item.stateMask=TVIS_STATEIMAGEMASK;
		tvis.item.state=INDEXTOSTATEIMAGEMASK((style&tvis.item.lParam)!=0?2:1);
		TreeView_InsertItem(hwndTree,&tvis);
	}
}

static DWORD MakeCheckBoxTreeFlags(HWND hwndTree)
{
	DWORD flags=0;

	TVITEM tvi;
	tvi.mask=TVIF_HANDLE|TVIF_PARAM|TVIF_STATE;
	tvi.hItem=TreeView_GetRoot(hwndTree);
	while(tvi.hItem) {
		TreeView_GetItem(hwndTree,&tvi);
		if (((tvi.state & TVIS_STATEIMAGEMASK)>>12 == 2))
			flags |= tvi.lParam;
		tvi.hItem = TreeView_GetNextSibling(hwndTree,tvi.hItem);
	}
	return flags;
}

//Update the name on the menu
static void UpdateMenuItem()
{
	CLISTMENUITEM mi = { sizeof(mi) };
	if (db_get_b(NULL, "Skin", "UseSound", 1))
		mi.ptszName = DISABLE_SOUND;
	else
		mi.ptszName = ENABLE_SOUND;
	mi.flags |= CMIM_NAME | CMIF_TCHAR;
	Menu_ModifyItem(noSoundMenu, &mi);
}

//Called when the sound setting in the database is changed
static int SoundSettingChanged(WPARAM hContact,LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws=(DBCONTACTWRITESETTING*)lParam;
	if(mir_strcmp(cws->szModule, "Skin") || mir_strcmp(cws->szSetting, "UseSound"))
		return 0;

	UpdateMenuItem();
	return 0;
}

static int SetNotify(const long status)
{
	db_set_b(NULL,"Skin","UseSound", (BYTE) !(db_get_dw(NULL, MODNAME, "NoSound",DEFAULT_NOSOUND) & status));
	db_set_b(NULL,"CList","DisableTrayFlash", (BYTE) (db_get_dw(NULL, MODNAME, "NoBlink",DEFAULT_NOBLINK) & status));
	db_set_b(NULL,"CList","NoIconBlink", (BYTE) (db_get_dw(NULL, MODNAME, "NoCLCBlink",DEFAULT_NOCLCBLINK) & status));

	UpdateMenuItem();
	return 0;
}

//Called whenever a change in status is detected
static int ProtoAck(WPARAM wParam, LPARAM lParam)
{
	ACKDATA *ack=(ACKDATA*)lParam;
	PROTOACCOUNT **protos;

	//quit if not status event
	if ( ack->type == ACKTYPE_STATUS && ack->result == ACKRESULT_SUCCESS ) {
		long status = 0;
		int count;
		ProtoEnumAccounts(&count, &protos);

		for (int i=0; i < count; i++)
			status = status | Proto_Status2Flag(CallProtoService(protos[i]->szModuleName, PS_GETSTATUS, 0, 0));

		SetNotify(status);
	}

	return 0;
}

static INT_PTR CALLBACK DlgProcNoSoundOpts(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	DWORD test;
	switch (msg)
	{
	case WM_INITDIALOG:
		TranslateDialogDefault(hwndDlg);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_NOSOUND),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_NOSOUND),GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_NOBLINK),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_NOBLINK),GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);
		SetWindowLongPtr(GetDlgItem(hwndDlg,IDC_NOCLCBLINK),GWL_STYLE,GetWindowLongPtr(GetDlgItem(hwndDlg,IDC_NOCLCBLINK),GWL_STYLE)|TVS_NOHSCROLL|TVS_CHECKBOXES);
		CheckDlgButton(hwndDlg, IDC_HIDEMENU, db_get_b(NULL, MODNAME, "HideMenu",1) ? BST_CHECKED : BST_UNCHECKED);

		FillCheckBoxTree(GetDlgItem(hwndDlg,IDC_NOSOUND),statusValues,sizeof(statusValues)/sizeof(statusValues[0]),db_get_dw(NULL, MODNAME, "NoSound",DEFAULT_NOSOUND));
		FillCheckBoxTree(GetDlgItem(hwndDlg,IDC_NOBLINK),statusValues,sizeof(statusValues)/sizeof(statusValues[0]),db_get_dw(NULL, MODNAME, "NoBlink",DEFAULT_NOBLINK));
		FillCheckBoxTree(GetDlgItem(hwndDlg,IDC_NOCLCBLINK),statusValues,sizeof(statusValues)/sizeof(statusValues[0]),db_get_dw(NULL, MODNAME, "NoCLCBlink",DEFAULT_NOCLCBLINK));
		return TRUE;
	case WM_COMMAND:
		SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		break;
	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case IDC_NOSOUND:
		case IDC_NOBLINK:
		case IDC_NOCLCBLINK:
			if (((LPNMHDR)lParam)->code==NM_CLICK) {
				TVHITTESTINFO hti;
				hti.pt.x=(short)LOWORD(GetMessagePos());
				hti.pt.y=(short)HIWORD(GetMessagePos());
				ScreenToClient(((LPNMHDR)lParam)->hwndFrom,&hti.pt);
				if(TreeView_HitTest(((LPNMHDR)lParam)->hwndFrom,&hti)) {
					if(hti.flags&TVHT_ONITEMSTATEICON) {
						TVITEM tvi;
						tvi.mask=TVIF_HANDLE|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
						tvi.hItem=hti.hItem;
						TreeView_GetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
						tvi.iImage=tvi.iSelectedImage=tvi.iImage==1?2:1;
						TreeView_SetItem(((LPNMHDR)lParam)->hwndFrom,&tvi);
						SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
					}
				}
			}
			break;
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_APPLY:
				db_set_b(NULL, MODNAME, "HideMenu",(BYTE)IsDlgButtonChecked(hwndDlg,IDC_HIDEMENU));

				db_set_dw(NULL, MODNAME, "NoSound",MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg,IDC_NOSOUND)));
				db_set_dw(NULL, MODNAME, "NoBlink",MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg,IDC_NOBLINK)));
				db_set_dw(NULL, MODNAME, "NoCLCBlink",MakeCheckBoxTreeFlags(GetDlgItem(hwndDlg,IDC_NOCLCBLINK)));

				test = db_get_w(NULL,"CList","Status",0);
				SetNotify(Proto_Status2Flag(db_get_w(NULL,"CList","Status",0)));
				return TRUE;
			}
			break;
		}
	}
	return FALSE;
}
//Called when the user opened the options dialog
static int OptionsInitialize(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.hInstance = hInst;
	odp.flags = ODPF_TCHAR;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_NOSOUND);
	odp.ptszTitle = LPGENT("Zero Notifications");
	odp.ptszGroup = LPGENT("Plugins");
	odp.groupPosition = 100000000;
	odp.pfnDlgProc = DlgProcNoSoundOpts;
	Options_AddPage(wParam, &odp);
	return 0;
}

static INT_PTR NoSoundMenuCommand(WPARAM wParam,LPARAM lParam)
{
	if (db_get_b(NULL,"Skin","UseSound",1))
		db_set_b(NULL,"Skin","UseSound",0);
//	else
//		db_set_b(NULL,"Skin","UseSound",1);

	return 0;
}

extern "C" __declspec(dllexport) int Load(void)
{
	mir_getLP(&pluginInfoEx);

	//The menu item - begin
	if (!db_get_b(NULL, MODNAME, "HideMenu", 1)) {
		hSoundMenu = CreateServiceFunction(MODNAME "/MenuCommand", NoSoundMenuCommand);
	
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.position = -0x7FFFFFFF;
		mi.flags = CMIF_TCHAR;
		UpdateMenuItem();

		mi.pszService = MODNAME "/MenuCommand";
		noSoundMenu = Menu_AddMainMenuItem(&mi);
	}
	//The menu item - end

	//The hooks
	hAckEvent = HookEvent(ME_PROTO_ACK, ProtoAck);
	hEventSoundSettingChange = HookEvent(ME_DB_CONTACT_SETTINGCHANGED, SoundSettingChanged);
	hEventOptionsInitialize = HookEvent(ME_OPT_INITIALISE, OptionsInitialize);

	return 0;
}

extern "C" __declspec(dllexport) int Unload(void)
{
	UnhookEvent(hEventSoundSettingChange);
	UnhookEvent(hEventOptionsInitialize);
	UnhookEvent(hAckEvent);
	DestroyServiceFunction(hSoundMenu);
	return 0;
}
#include "../commonstatus.h"
#include "keepstatus.h"
#include "../resource.h"
#include "../../NewTriggerPlugin/m_trigger.h"
#include "../../helpers/db_helpers.h"

extern HINSTANCE hInst;

#define STR_PROTOFROMTRIGGER	"PROTO_FT"
#define SETTING_PROTO_ENABLED	"proto_Enabled"

static BOOL ProtocolEnabled(DWORD id, char *szPrefix, REPORTINFO *ri, char *szProto)
{
	char dbSetting[128];
	_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s_%s", szPrefix, id, szProto, SETTING_PROTO_ENABLED);
	if (db_getb(dbSetting, 0))
		return TRUE;

	if ( ri != NULL && ri->td != NULL && (ri->td->dFlags & DF_PROTO) && !strcmp(szProto, ri->td->szProto)) {
		_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s_%s", szPrefix, id, STR_PROTOFROMTRIGGER, SETTING_PROTO_ENABLED);
		return db_getb(dbSetting, 0);
	}

	return FALSE;
}

static void BuildProtoList(HWND hList, BOOL bFromTrigger, char* szPrefix, DWORD id)
{
	ListView_SetExtendedListViewStyleEx(hList, LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES, LVS_EX_FULLROWSELECT|LVS_EX_CHECKBOXES);

	LVCOLUMN lvCol = { 0 };
	lvCol.mask = LVCF_TEXT;
	lvCol.pszText = TranslateT("Protocol");
	ListView_InsertColumn(hList, 0, &lvCol);

	LVITEM lvItem = { 0 };
	lvItem.mask = LVIF_TEXT|LVIF_PARAM;
	lvItem.iItem = 0;
	lvItem.iSubItem = 0;

	int count;
	PROTOACCOUNT** protos;
	ProtoEnumAccounts( &count, &protos );

	char dbSetting[128];

	for( int i=0; i < count; i++ ) {
		if ( !IsSuitableProto( protos[i] ))
			continue;

		lvItem.pszText = Hlp_GetProtocolName( protos[i]->szModuleName );
		lvItem.lParam = (LPARAM)protos[i]->szModuleName;
		ListView_InsertItem(hList, &lvItem);

		_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s_%s", szPrefix, id, protos[i]->szModuleName, SETTING_PROTO_ENABLED);
		ListView_SetCheckState(hList, lvItem.iItem, db_getb(dbSetting, 0));
		ListView_SetItem(hList, &lvItem);
		lvItem.iItem++;
		if (lvItem.pszText != NULL)
			free(lvItem.pszText);
	}
	if (bFromTrigger) {
		lvItem.pszText = TranslateT("<from trigger>");
		ListView_InsertItem(hList, &lvItem);
		lvItem.lParam = (LPARAM)STR_PROTOFROMTRIGGER;
		_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s_%s", szPrefix, id, STR_PROTOFROMTRIGGER, SETTING_PROTO_ENABLED);
		ListView_SetCheckState(hList, lvItem.iItem, db_getb(dbSetting, 0));
		ListView_SetItem(hList, &lvItem);
		lvItem.iItem++;
	}
	ListView_SetColumnWidth(hList, 0, LVSCW_AUTOSIZE);
}

static void SaveProtoList(HWND hList, char *szPrefix, DWORD id)
{
	LVITEM lvItem = { 0 };
	lvItem.mask = LVIF_PARAM;
	lvItem.iSubItem=0;
	for ( int i=0; i < ListView_GetItemCount(hList); i++ ) {
		lvItem.iItem=i;
		if (ListView_GetItem(hList, &lvItem) == FALSE)
			continue;

		char dbSetting[128];
		_snprintf(dbSetting, sizeof(dbSetting), "%s%u_%s_%s", szPrefix, id, (const char*)lvItem.lParam, SETTING_PROTO_ENABLED);
		if (ListView_GetCheckState(hList, lvItem.iItem))
			db_setb(dbSetting, TRUE);
		else {
			log_debugA("deleting: %s", dbSetting);
			db_del(dbSetting);
		}
	}
}

static INT_PTR CALLBACK DlgProcOptsActionKeepStatus(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {
	case WM_INITDIALOG: {
		DWORD actionID;
		TRIGGERINFO ti;
		
		TranslateDialogDefault(hwndDlg);
		actionID = (DWORD)lParam;
		ZeroMemory(&ti, sizeof(TRIGGERINFO));
		ti.cbSize = sizeof(TRIGGERINFO);
		SendMessage(GetParent(hwndDlg), TM_GETTRIGGERINFO, 0, (LPARAM)&ti);
		BuildProtoList(GetDlgItem(hwndDlg, IDC_PROTOLIST), ti.dFlags&DF_PROTO?TRUE:FALSE, PREFIX_ACTIONID, actionID);
		CheckRadioButton(hwndDlg, IDC_ENABLECHECKING, IDC_DISABLECHECKING, DBGetActionSettingByte(actionID, NULL, MODULENAME, SETTING_ENABLECHECKING, FALSE)?IDC_ENABLECHECKING:IDC_DISABLECHECKING);
		break;
	}

	case TM_ADDACTION: { // save the settings
		DWORD actionID;
		
		actionID = (DWORD)wParam;
		DBWriteActionSettingByte(actionID, NULL, MODULENAME, SETTING_ENABLECHECKING, (BYTE)IsDlgButtonChecked(hwndDlg, IDC_ENABLECHECKING));
		SaveProtoList(GetDlgItem(hwndDlg, IDC_PROTOLIST), PREFIX_ACTIONID, actionID);
		break;
	}

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDC_ENABLECHECKING:
		case IDC_DISABLECHECKING:
			CheckRadioButton(hwndDlg, IDC_ENABLECHECKING, IDC_DISABLECHECKING, LOWORD(wParam));
			break;
		}
		break;
	}

	return FALSE;
}

static int TriggerChecking(DWORD actionID, REPORTINFO *ri)
{
	if (ri->flags & ACT_PERFORM) {
		BOOL checking = DBGetActionSettingByte(actionID, NULL, MODULENAME, SETTING_ENABLECHECKING, FALSE);

		int count;
		PROTOACCOUNT** protos;
		ProtoEnumAccounts( &count, &protos );

		for( int i=0; i <count; i++ ) {
			if ( !IsSuitableProto( protos[i] ))
				continue;

			if (!ProtocolEnabled(actionID, PREFIX_ACTIONID, ri, protos[i]->szModuleName))
				continue;

			CallService(MS_KS_ENABLEPROTOCOL, (WPARAM)checking, (LPARAM)protos[i]->szModuleName);
		}
	}
	if (ri->flags & ACT_CLEANUP) {
		REMOVETRIGGERSETTINGS ras;
		ras.cbSize = sizeof(REMOVETRIGGERSETTINGS);
		ras.prefix = PREFIX_ACTIONID;
		ras.id = actionID;
		ras.hContact = NULL;
		ras.szModule = MODULENAME;
		CallService(MS_TRIGGER_REMOVESETTINGS, 0, (LPARAM)&ras);
	}

	return 0;
}

int RegisterAction()
{
	if (!ServiceExists(MS_TRIGGER_REGISTERACTION))
		return -1;

	ACTIONREGISTER ai;
	ai.cbSize = sizeof(ai);
	ai.pszName = "KeepStatus: Enable/disable checking";
	ai.hInstance = hInst;
	ai.pfnDlgProc = DlgProcOptsActionKeepStatus;
	ai.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_KS_ACTION);
	ai.flags = ARF_FUNCTION;
	ai.actionFunction = TriggerChecking;
	return CallService(MS_TRIGGER_REGISTERACTION, 0, (LPARAM)&ai);
}

int DeInitAction() {

	return 0;
}

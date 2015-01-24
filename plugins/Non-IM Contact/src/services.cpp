#include "commonheaders.h"

//=======================================================
//db_get_static
//=======================================================
int db_get_static(MCONTACT hContact, const char *szModule, const char *szSetting, char *value)
{
	DBVARIANT dbv;
	if (!db_get(hContact, szModule, szSetting, &dbv))
	{
		strcpy(value, dbv.pszVal);
		db_free(&dbv);
		return 1;
	}
	else 
	{
		db_free(&dbv);
		return 0;
	}
	
	return 0;
}

//=======================================================
//GetCaps
//=======================================================

INT_PTR GetLCCaps(WPARAM wParam,LPARAM lParam)
{
	if (wParam==PFLAGNUM_1)
		return 0;
	if (wParam==PFLAGNUM_2)
		return PF2_ONLINE|PF2_LONGAWAY|PF2_SHORTAWAY|PF2_LIGHTDND|PF2_HEAVYDND|PF2_FREECHAT|PF2_INVISIBLE|PF2_OUTTOLUNCH|PF2_ONTHEPHONE; // add the possible statuses here.
	if (wParam==PFLAGNUM_3)
		return 0;
	return 0;
}

//=======================================================
//GetName
//=======================================================
INT_PTR GetLCName(WPARAM wParam,LPARAM lParam)
{
	mir_strncpy((char*)lParam, MODNAME, wParam);
	return 0;
}

//=======================================================
//BPLoadIcon
//=======================================================
INT_PTR LoadLCIcon(WPARAM wParam,LPARAM lParam)
{
	if (LOWORD(wParam) == PLI_PROTOCOL) {
		if (wParam & PLIF_ICOLIBHANDLE)
			return (INT_PTR)icoList[0].hIcolib;

		HICON hIcon = Skin_GetIconByHandle(icoList[0].hIcolib, (wParam & PLIF_SMALL) == 0);
		if (wParam & PLIF_ICOLIB)
			return (INT_PTR)hIcon;

		HICON hIcon2 = CopyIcon(hIcon);
		Skin_ReleaseIcon(hIcon);
		return (INT_PTR)hIcon2;
	}

	return NULL;	
}

//=======================================================
//SetFStatus			
//=======================================================
int SetLCStatus(WPARAM wParam,LPARAM lParam) 
{ 
	int oldStatus = LCStatus;
	LCStatus = wParam;
	db_set_w(NULL, MODNAME, "Status", (WORD)wParam);
	db_set_w(NULL, MODNAME, "timerCount",0);
	if (LCStatus == ID_STATUS_OFFLINE || (LCStatus == ID_STATUS_AWAY && !db_get_b(NULL, MODNAME, "AwayAsStatus", 0)) || !db_get_w(NULL, MODNAME, "Timer",1)) killTimer();
	else if (db_get_w(NULL, MODNAME, "Timer",1))
		startTimer(TIMER); 
	
	for (MCONTACT hContact = db_find_first(MODNAME); hContact; hContact = db_find_next(hContact,MODNAME)) {
		if (LCStatus != ID_STATUS_OFFLINE)
			replaceAllStrings(hContact);

		switch (LCStatus) {
		case ID_STATUS_OFFLINE:
			if (db_get_b(hContact, MODNAME, "AlwaysVisible",0) && !db_get_b(hContact, MODNAME, "VisibleUnlessOffline",1))
				db_set_w(hContact, MODNAME, "Status",(WORD)db_get_w(hContact, MODNAME, "Icon",ID_STATUS_ONLINE));	
			else
				db_set_w(hContact, MODNAME, "Status", ID_STATUS_OFFLINE);
			break;

		case ID_STATUS_ONLINE:
			db_set_w(hContact, MODNAME, "Status",(WORD)db_get_w(hContact, MODNAME, "Icon",ID_STATUS_ONLINE));
			break;

		case ID_STATUS_AWAY:
			if (db_get_b(NULL, MODNAME, "AwayAsStatus", 0) && (db_get_b(hContact, MODNAME, "AlwaysVisible",0) || (db_get_w(hContact, MODNAME, "Icon",ID_STATUS_ONLINE)==ID_STATUS_AWAY)) )
				db_set_w(hContact, MODNAME, "Status",(WORD)db_get_w(hContact, MODNAME, "Icon",ID_STATUS_ONLINE));	
			else if (!db_get_b(NULL, MODNAME, "AwayAsStatus", 0))
				db_set_w(hContact, MODNAME, "Status",(WORD)db_get_w(hContact, MODNAME, "Icon",ID_STATUS_ONLINE));
			else
				db_set_w(hContact, MODNAME, "Status", ID_STATUS_OFFLINE);
			break;

		default:
			if (db_get_b(hContact, MODNAME, "AlwaysVisible",0) || LCStatus == db_get_w(hContact, MODNAME, "Icon",ID_STATUS_ONLINE))
				db_set_w(hContact, MODNAME, "Status",(WORD)db_get_w(hContact, MODNAME, "Icon",ID_STATUS_ONLINE));
			break;
		}
	}

	ProtoBroadcastAck(MODNAME,NULL,ACKTYPE_STATUS,ACKRESULT_SUCCESS,(HANDLE)oldStatus,wParam);
	return 0; 
} 

//=======================================================
//GetStatus
//=======================================================
INT_PTR GetLCStatus(WPARAM wParam,LPARAM lParam)
{
	if ((LCStatus >= ID_STATUS_ONLINE) && (LCStatus <= ID_STATUS_OUTTOLUNCH))
		return LCStatus;
	else
		return ID_STATUS_OFFLINE;
}


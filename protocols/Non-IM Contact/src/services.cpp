#include "stdafx.h"

//=======================================================
// GetCaps
//=======================================================
//
INT_PTR GetLCCaps(WPARAM wParam, LPARAM)
{
	if (wParam == PFLAGNUM_1)
		return 0;
	if (wParam == PFLAGNUM_2)
		return PF2_ONLINE | PF2_LONGAWAY | PF2_SHORTAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT | PF2_INVISIBLE; // add the possible statuses here.
	if (wParam == PFLAGNUM_3)
		return 0;
	return 0;
}

//=======================================================
// GetName
//=======================================================
//
INT_PTR GetLCName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char*)lParam, MODNAME, wParam);
	return 0;
}

//=======================================================
// BPLoadIcon
//=======================================================
//
INT_PTR LoadLCIcon(WPARAM wParam, LPARAM)
{
	if (LOWORD(wParam) == PLI_PROTOCOL) {
		if (wParam & PLIF_ICOLIBHANDLE)
			return (INT_PTR)iconList[0].hIcolib;

		HICON hIcon = IcoLib_GetIconByHandle(iconList[0].hIcolib, (wParam & PLIF_SMALL) == 0);
		if (wParam & PLIF_ICOLIB)
			return (INT_PTR)hIcon;

		HICON hIcon2 = CopyIcon(hIcon);
		IcoLib_ReleaseIcon(hIcon);
		return (INT_PTR)hIcon2;
	}

	return NULL;
}

//=======================================================
// SetFStatus			
//=======================================================
//
int SetLCStatus(WPARAM wParam, LPARAM)
{
	int oldStatus = LCStatus;
	LCStatus = wParam;
	g_plugin.setWord("Status", (uint16_t)wParam);
	g_plugin.setWord("timerCount", 0);
	if (LCStatus == ID_STATUS_OFFLINE || (LCStatus == ID_STATUS_AWAY && !g_plugin.getByte("AwayAsStatus", 0)) || !g_plugin.getWord("Timer", 1))
		killTimer();
	else if (g_plugin.getWord("Timer", 1))
		startTimer(TIMER);

	for (auto &hContact : Contacts(MODNAME)) {
		if (LCStatus != ID_STATUS_OFFLINE)
			replaceAllStrings(hContact);

		switch (LCStatus) {
		case ID_STATUS_OFFLINE:
			if (g_plugin.getByte(hContact, "AlwaysVisible", 0) && !g_plugin.getByte(hContact, "VisibleUnlessOffline", 1))
				g_plugin.setWord(hContact, "Status", (uint16_t)g_plugin.getWord(hContact, "Icon", ID_STATUS_ONLINE));
			else
				g_plugin.setWord(hContact, "Status", ID_STATUS_OFFLINE);
			break;

		case ID_STATUS_ONLINE:
			g_plugin.setWord(hContact, "Status", (uint16_t)g_plugin.getWord(hContact, "Icon", ID_STATUS_ONLINE));
			break;

		case ID_STATUS_AWAY:
			if (g_plugin.getByte("AwayAsStatus", 0) && (g_plugin.getByte(hContact, "AlwaysVisible", 0) || (g_plugin.getWord(hContact, "Icon", ID_STATUS_ONLINE) == ID_STATUS_AWAY)))
				g_plugin.setWord(hContact, "Status", (uint16_t)g_plugin.getWord(hContact, "Icon", ID_STATUS_ONLINE));
			else if (!g_plugin.getByte("AwayAsStatus", 0))
				g_plugin.setWord(hContact, "Status", (uint16_t)g_plugin.getWord(hContact, "Icon", ID_STATUS_ONLINE));
			else
				g_plugin.setWord(hContact, "Status", ID_STATUS_OFFLINE);
			break;

		default:
			if (g_plugin.getByte(hContact, "AlwaysVisible", 0) || LCStatus == g_plugin.getWord(hContact, "Icon", ID_STATUS_ONLINE))
				g_plugin.setWord(hContact, "Status", (uint16_t)g_plugin.getWord(hContact, "Icon", ID_STATUS_ONLINE));
			break;
		}
	}

	ProtoBroadcastAck(MODNAME, NULL, ACKTYPE_STATUS, ACKRESULT_SUCCESS, (HANDLE)oldStatus, wParam);
	return 0;
}

//=======================================================
// GetStatus
//=======================================================
//
INT_PTR GetLCStatus(WPARAM, LPARAM)
{
	if (LCStatus >= ID_STATUS_ONLINE && LCStatus <= ID_STATUS_MAX)
		return LCStatus;
	else
		return ID_STATUS_OFFLINE;
}


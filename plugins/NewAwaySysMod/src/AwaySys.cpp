/*
	New Away System - plugin for Miranda IM
	Copyright (c) 2005-2007 Chervov Dmitry
	Copyright (c) 2004-2005 Iksaif Entertainment
	Copyright (c) 2002-2003 Goblineye Entertainment

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
  Thanx to Faith Healer for icons and help, Unregistered for his plugins (variables, AAA :p),
   mistag for GamerStatus,  BigMuscle for his code to use AAA
  Thanx to Tornado, orignal developer of AwaySys.
  Please note that some code from the Miranda's original away module (SRAway) is used around
  AwaySys. i tried to mention it wherever possible, but i might have forgotten a few. Kudos to Miranda's authors.
  The Read-Away-Msg part was practically copied from Miranda, not proud of it, but since i really can't see how can i make it better, there
  was no point in rewriting it all.
*/

#include "Common.h"
#include "m_genmenu.h"
#include "m_idle.h"
#include "m_statusplugins.h"
#include "m_NewAwaySys.h"
#include "MsgTree.h"
#include "ContactList.h"
#include "Properties.h"
#include "Path.h"
#include "Services.h"
#include "version.h"

#include <m_modernopt.h>

HINSTANCE g_hInstance;
CLIST_INTERFACE *pcli;

int hLangpack;
HANDLE g_hContactMenuItem, g_hReadStatMenuItem, g_hTopToolbarbutton;
HGENMENU g_hToggleSOEMenuItem, g_hToggleSOEContactMenuItem;
HGENMENU g_hAutoreplyOnContactMenuItem, g_hAutoreplyOffContactMenuItem, g_hAutoreplyUseDefaultContactMenuItem;
bool g_fNoProcessing = false; // tells the status change proc not to do anything
int g_bIsIdle = false;
VAR_PARSE_DATA VarParseData;

static IconItem iconList[] = {
	{ LPGEN("Toggle On"),  "on",  IDI_SOE_DISABLED },
	{ LPGEN("Toggle Off"), "off", IDI_SOE_ENABLED  }
};

PLUGININFOEX pluginInfo = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__AUTHOREMAIL,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0xb2dd9270, 0xce5e, 0x11df, { 0xbd, 0x3d, 0x8, 0x0, 0x20, 0xc, 0x9a, 0x66 } }
};

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	g_hInstance = hinstDLL;
	return TRUE;
}

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SRAWAY, MIID_LAST }; // TODO: add MIID_WHOISREADING here if there'll be any some time in future..

extern "C" __declspec(dllexport) PLUGININFOEX *MirandaPluginInfoEx(DWORD mirandaVersion)
{
	return &pluginInfo;
}

/////////////////////////////////////////////////////////////////////////////////////////

TCString GetDynamicStatMsg(MCONTACT hContact, char *szProto, DWORD UIN, int iStatus)
{
	// hContact is the contact that requests the status message
	if (hContact != INVALID_CONTACT_ID)
		VarParseData.Message = CContactSettings(iStatus, hContact).GetMsgFormat(GMF_ANYCURRENT, NULL, szProto);
	else // contact is unknown
		VarParseData.Message = CProtoSettings(szProto, iStatus).GetMsgFormat(iStatus ? GMF_LASTORDEFAULT : GMF_ANYCURRENT);

	TCString sTime;
	VarParseData.szProto = szProto ? szProto : ((hContact && hContact != INVALID_CONTACT_ID) ? GetContactProto(hContact) : NULL);
	VarParseData.UIN = UIN;
	VarParseData.Flags = 0;
	if (ServiceExists(MS_VARS_FORMATSTRING) && !g_SetAwayMsgPage.GetDBValueCopy(IDS_SAWAYMSG_DISABLEVARIABLES)) {
		TCHAR *szResult = variables_parse(VarParseData.Message,  0, hContact);
		if (szResult) {
			VarParseData.Message = szResult;
			mir_free(szResult);
		}
	}
	return VarParseData.Message = VarParseData.Message.Left(AWAY_MSGDATA_MAX);
}


int StatusMsgReq(WPARAM wParam, LPARAM lParam, CString &szProto)
{
	_ASSERT(szProto != NULL);
	LogMessage("ME_ICQ_STATUSMSGREQ called. szProto=%s, Status=%d, UIN=%d", (char*)szProto, wParam, lParam);
	// find the contact
	char *szFoundProto;
	MCONTACT hFoundContact = NULL; // if we'll find the contact only on some other protocol, but not on szProto, then we'll use that hContact.
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)) {
		char *szCurProto = GetContactProto(hContact);
		if (db_get_dw(hContact, szCurProto, "UIN", 0) == lParam) {
			szFoundProto = szCurProto;
			hFoundContact = hContact;
			if (!strcmp(szCurProto, szProto))
				break;
		}
	}

	int iMode = ICQStatusToGeneralStatus(wParam);
	if (!hFoundContact)
		hFoundContact = INVALID_CONTACT_ID;
	else if (iMode >= ID_STATUS_ONLINE && iMode <= ID_STATUS_OUTTOLUNCH)
		// don't count xstatus requests
		db_set_w(hFoundContact, MOD_NAME, DB_REQUESTCOUNT, db_get_w(hFoundContact, MOD_NAME, DB_REQUESTCOUNT, 0) + 1);

	MCONTACT hContactForSettings = hFoundContact; // used to take into account not-on-list contacts when getting contact settings, but at the same time allows to get correct contact info for contacts that are in the DB
	if (hContactForSettings != INVALID_CONTACT_ID && db_get_b(hContactForSettings, "CList", "NotOnList", 0))
		hContactForSettings = INVALID_CONTACT_ID; // INVALID_HANDLE_VALUE means the contact is not-on-list

	if (g_SetAwayMsgPage.GetWnd()) {
		CallAllowedPS_SETAWAYMSG(szProto, iMode, NULL); // we can set status messages to NULL here, as they'll be changed again when the SAM dialog closes.
		return 0;
	}
	if (CContactSettings(iMode, hContactForSettings).Ignore) {
		CallAllowedPS_SETAWAYMSG(szProto, iMode, ""); // currently NULL makes ICQ to ignore _any_ further status message requests until the next PS_SETAWAYMSG, so i can't use it here..
		return 0; // move along, sir
	}

	if (iMode) { // if it's not an xstatus message request
		CallAllowedPS_SETAWAYMSG(szProto, iMode, (char*)_T2A(GetDynamicStatMsg(hFoundContact, szProto, lParam)));
	}
	//	COptPage PopupNotifyData(g_PopupOptPage);
	//	PopupNotifyData.DBToMem();
	VarParseData.szProto = szProto;
	VarParseData.UIN = lParam;
	VarParseData.Flags = 0;
	if (!iMode)
		VarParseData.Flags |= VPF_XSTATUS;

	return 0;
}

// Here is an ugly workaround to support multiple ICQ accounts
// hope 5 icq accounts will be sufficient for everyone ;)
#define MAXICQACCOUNTS 5
CString ICQProtoList[MAXICQACCOUNTS];
#define StatusMsgReqN(N) int StatusMsgReq##N(WPARAM wParam, LPARAM lParam) {return StatusMsgReq(wParam, lParam, ICQProtoList[N - 1]);}
StatusMsgReqN(1) StatusMsgReqN(2) StatusMsgReqN(3) StatusMsgReqN(4) StatusMsgReqN(5)
MIRANDAHOOK StatusMsgReqHooks[] = { StatusMsgReq1, StatusMsgReq2, StatusMsgReq3, StatusMsgReq4, StatusMsgReq5 };

int IsAnICQProto(char *szProto)
{
	for (int i = 0; i < MAXICQACCOUNTS; i++)
		if (ICQProtoList[i] == (const char*)szProto)
			return true;

	return false;
}

// wParam = iMode
// lParam = (char*)szProto

struct
{
	int Status, DisableReplyCtlID, DontShowDialogCtlID;
}
static StatusModeList[] = {
	ID_STATUS_ONLINE, IDC_REPLYDLG_DISABLE_ONL, IDC_MOREOPTDLG_DONTPOPDLG_ONL,
	ID_STATUS_AWAY, IDC_REPLYDLG_DISABLE_AWAY, IDC_MOREOPTDLG_DONTPOPDLG_AWAY,
	ID_STATUS_NA, IDC_REPLYDLG_DISABLE_NA, IDC_MOREOPTDLG_DONTPOPDLG_NA,
	ID_STATUS_OCCUPIED, IDC_REPLYDLG_DISABLE_OCC, IDC_MOREOPTDLG_DONTPOPDLG_OCC,
	ID_STATUS_DND, IDC_REPLYDLG_DISABLE_DND, IDC_MOREOPTDLG_DONTPOPDLG_DND,
	ID_STATUS_FREECHAT, IDC_REPLYDLG_DISABLE_FFC, IDC_MOREOPTDLG_DONTPOPDLG_FFC,
	ID_STATUS_INVISIBLE, IDC_REPLYDLG_DISABLE_INV, IDC_MOREOPTDLG_DONTPOPDLG_INV,
	ID_STATUS_ONTHEPHONE, IDC_REPLYDLG_DISABLE_OTP, IDC_MOREOPTDLG_DONTPOPDLG_OTP,
	ID_STATUS_OUTTOLUNCH, IDC_REPLYDLG_DISABLE_OTL, IDC_MOREOPTDLG_DONTPOPDLG_OTL
};

int StatusChanged(WPARAM wParam, LPARAM lParam)
{
	LogMessage("MS_CLIST_SETSTATUSMODE called. szProto=%s, Status=%d", lParam ? (char*)lParam : "NULL", wParam);
	g_ProtoStates[(char*)lParam].Status = wParam;
	// let's check if we handle this thingy
	if (g_fNoProcessing) { // we're told not to do anything
		g_fNoProcessing = false; // take it off
		return 0;
	}
	DWORD Flag1 = 0;
	DWORD Flag3 = 0;
	if (lParam) {
		Flag1 = CallProtoService((char*)lParam, PS_GETCAPS, PFLAGNUM_1, 0);
		Flag3 = CallProtoService((char*)lParam, PS_GETCAPS, PFLAGNUM_3, 0);
	}
	else {
		PROTOACCOUNT **accs;
		int numAccs = 0;
		ProtoEnumAccounts(&numAccs, &accs);
		for (int i = 0; i < numAccs; i++) {
			Flag1 |= CallProtoService(accs[i]->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
			Flag3 |= CallProtoService(accs[i]->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0);
		}
	}

	if (!(Flag1 & PF1_MODEMSGSEND || Flag3 & Proto_Status2Flag(wParam) || (Flag1 & PF1_IM) == PF1_IM))
		return 0; // there are no protocols with changed status that support autoreply or away messages for this status

	if (g_SetAwayMsgPage.GetWnd()) {
		SetForegroundWindow(g_SetAwayMsgPage.GetWnd());
		return 0;
	}

	int i;
	for (i = SIZEOF(StatusModeList) - 1; i >= 0; i--)
		if (wParam == StatusModeList[i].Status)
			break;
	if (i < 0)
		return 0;

	BOOL bScreenSaverRunning;
	SystemParametersInfo(SPI_GETSCREENSAVERRUNNING, 0, &bScreenSaverRunning, 0);
	if (bScreenSaverRunning || g_MoreOptPage.GetDBValueCopy(StatusModeList[i].DontShowDialogCtlID)) {
		CProtoSettings((char*)lParam).SetMsgFormat(SMF_PERSONAL, CProtoSettings((char*)lParam).GetMsgFormat(GMF_LASTORDEFAULT));
		ChangeProtoMessages((char*)lParam, wParam, TCString());
	}
	else {
		SetAwayMsgData *dat = new SetAwayMsgData;
		ZeroMemory(dat, sizeof(SetAwayMsgData));
		dat->szProtocol = (char*)lParam;
		dat->IsModeless = false;
		DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SETAWAYMSG), NULL, SetAwayMsgDlgProc, (LPARAM)dat);
	}
	return 0;
}


#define ID_STATUS_LAST 40081 // yes, 40081 means internal CommonStatus' ID_STATUS_LAST here, not ID_STATUS_IDLE :-S
#define ID_STATUS_CURRENT 40082
#define ID_STATUS_DISABLED 41083

// wParam = PROTOCOLSETTINGEX** protoSettings
int CSStatusChange(WPARAM wParam, LPARAM lParam) // CommonStatus plugins (StartupStatus and AdvancedAutoAway)
{
	PROTOCOLSETTINGEX **ps = *(PROTOCOLSETTINGEX***)wParam;
	if (!ps)
		return -1;

	LogMessage("ME_CS_STATUSCHANGEEX event:");

	for (int i = 0; i < lParam; i++) {
		LogMessage("%d: cbSize=%d, szProto=%s, status=%d, lastStatus=%d, szMsg:", 
			i + 1, ps[i]->cbSize, ps[i]->szName ? (char*)ps[i]->szName : "NULL", ps[i]->status, ps[i]->lastStatus, ps[i]->szMsg ? ps[i]->szMsg : _T("NULL"));
		if (ps[i]->status != ID_STATUS_DISABLED) {
			if (ps[i]->status != ID_STATUS_CURRENT)
				g_ProtoStates[ps[i]->szName].Status = (ps[i]->status == ID_STATUS_LAST) ? ps[i]->lastStatus : ps[i]->status;

			CProtoSettings(ps[i]->szName).SetMsgFormat(SMF_TEMPORARY, ps[i]->szMsg ? ps[i]->szMsg : CProtoSettings(ps[i]->szName).GetMsgFormat(GMF_LASTORDEFAULT));
		}
	}
	return 0;
}

static int IdleChangeEvent(WPARAM, LPARAM lParam)
{
	LogMessage("ME_IDLE_CHANGED event. lParam=0x%x", lParam); // yes, we don't do anything with status message changes on idle.. there seems to be no any good solution for the wrong status message issue :(
	g_bIsIdle = lParam & IDF_ISIDLE;
	return 0;
}

int PreBuildContactMenu(WPARAM hContact, LPARAM)
{
	char *szProto = GetContactProto(hContact);
	CLISTMENUITEM miSetMsg = { sizeof(miSetMsg) };
	miSetMsg.flags = CMIM_FLAGS | CMIF_TCHAR | CMIF_HIDDEN;
	CLISTMENUITEM miReadMsg = { sizeof(miReadMsg) };
	miReadMsg.flags = CMIM_FLAGS | CMIF_TCHAR | CMIF_HIDDEN;
	int iMode = szProto ? CallProtoService(szProto, PS_GETSTATUS, 0, 0) : 0;
	int Flag1 = szProto ? CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) : 0;
	int iContactMode = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
	TCHAR szSetStr[256], szReadStr[256];
	if (szProto) {
		int i;
		for (i = SIZEOF(StatusModeList) - 1; i >= 0; i--)
			if (iMode == StatusModeList[i].Status)
				break;

		// the protocol supports status message sending for current status, or autoreplying
		if ((Flag1 & PF1_MODEMSGSEND && CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(iMode)) || ((Flag1 & PF1_IM) == PF1_IM && (i < 0 || !g_AutoreplyOptPage.GetDBValueCopy(StatusModeList[i].DisableReplyCtlID)))) {
			mir_sntprintf(szSetStr, SIZEOF(szSetStr), TranslateT("Set %s message for the contact"), pcli->pfnGetStatusModeDescription(iMode, 0), pcli->pfnGetContactDisplayName(hContact, 0));
			miSetMsg.ptszName = szSetStr;
			miSetMsg.flags = CMIM_FLAGS | CMIF_TCHAR | CMIM_NAME;
		}

		// the protocol supports status message reading for contact's status
		if (Flag1 & PF1_MODEMSGRECV && CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(iContactMode)) {
			mir_sntprintf(szReadStr, SIZEOF(szReadStr), TranslateT("Re&ad %s message"), pcli->pfnGetStatusModeDescription(iContactMode, 0));
			miReadMsg.ptszName = szReadStr;
			miReadMsg.flags = CMIM_FLAGS | CMIF_TCHAR | CMIM_NAME | CMIM_ICON;
			miReadMsg.hIcon = LoadSkinnedProtoIcon(szProto, iContactMode);
		}
	}
	if (g_hContactMenuItem) {
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hContactMenuItem, (LPARAM)&miSetMsg);

		// if this contact supports sending/receiving messages
		if ((Flag1 & PF1_IM) == PF1_IM) {
			int iAutoreply = CContactSettings(g_ProtoStates[szProto].Status, hContact).Autoreply;
			CLISTMENUITEM mi = { sizeof(mi) };
			mi.flags = CMIM_ICON | CMIM_FLAGS | CMIF_TCHAR;
			switch (iAutoreply) {
				case VAL_USEDEFAULT: mi.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DOT)); break;
				case 0: mi.icolibItem = iconList[0].hIcolib; break;
				default: iAutoreply = 1; mi.icolibItem = iconList[1].hIcolib; break;
			}
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hToggleSOEContactMenuItem, (LPARAM)&mi);
			mi.flags = CMIM_FLAGS | CMIF_TCHAR | (iAutoreply == 1 ? CMIF_CHECKED : 0);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hAutoreplyOnContactMenuItem, (LPARAM)&mi);
			mi.flags = CMIM_FLAGS | CMIF_TCHAR | (iAutoreply == 0 ? CMIF_CHECKED : 0);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hAutoreplyOffContactMenuItem, (LPARAM)&mi);
			mi.flags = CMIM_FLAGS | CMIF_TCHAR | (iAutoreply == VAL_USEDEFAULT ? CMIF_CHECKED : 0);
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hAutoreplyUseDefaultContactMenuItem, (LPARAM)&mi);
		}
		else { // hide the Autoreply menu item
			CLISTMENUITEM mi = { sizeof(mi) };
			mi.flags = CMIM_FLAGS | CMIF_TCHAR | CMIF_HIDDEN;
			CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hToggleSOEContactMenuItem, (LPARAM)&mi);
		}
	}
	CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hReadStatMenuItem, (LPARAM)&miReadMsg);
	return 0;
}


static INT_PTR SetContactStatMsg(WPARAM hContact, LPARAM)
{
	if (g_SetAwayMsgPage.GetWnd()) { // already setting something
		SetForegroundWindow(g_SetAwayMsgPage.GetWnd());
		return 0;
	}
	
	SetAwayMsgData *dat = new SetAwayMsgData;
	ZeroMemory(dat, sizeof(SetAwayMsgData));
	dat->hInitContact = hContact;
	dat->szProtocol = GetContactProto(hContact);
	dat->IsModeless = false;
	DialogBoxParam(g_hInstance, MAKEINTRESOURCE(IDD_SETAWAYMSG), NULL, SetAwayMsgDlgProc, (LPARAM)dat);
	return 0;
}

INT_PTR ToggleSendOnEvent(WPARAM hContact, LPARAM)
{
	// used only for the global setting
	CContactSettings(g_ProtoStates[hContact ? GetContactProto(hContact) : NULL].Status, hContact).Autoreply.Toggle();

	if (hContact == NULL) {
		int SendOnEvent = CContactSettings(g_ProtoStates[(LPSTR)NULL].Status).Autoreply;
		
		CLISTMENUITEM mi = { sizeof(mi) };
		mi.flags = CMIM_ICON | CMIM_NAME | CMIF_TCHAR;
		if (SendOnEvent) {
			mi.ptszName = ENABLE_SOE_COMMAND;
			mi.icolibItem = iconList[1].hIcolib;
		}
		else {
			mi.ptszName = DISABLE_SOE_COMMAND;
			mi.icolibItem = iconList[0].hIcolib;
		}
		CallService(MS_CLIST_MODIFYMENUITEM, (WPARAM)g_hToggleSOEMenuItem, (LPARAM)&mi);

		if (g_hTopToolbarbutton)
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)g_hTopToolbarbutton, SendOnEvent ? TTBST_PUSHED : 0);
	}

	return 0;
}

INT_PTR srvAutoreplyOn(WPARAM hContact, LPARAM)
{
	CContactSettings(g_ProtoStates[GetContactProto(hContact)].Status, hContact).Autoreply = 1;
	return 0;
}

INT_PTR srvAutoreplyOff(WPARAM hContact, LPARAM)
{
	CContactSettings(g_ProtoStates[GetContactProto(hContact)].Status, hContact).Autoreply = 0;
	return 0;
}

INT_PTR srvAutoreplyUseDefault(WPARAM hContact, LPARAM)
{
	CContactSettings(g_ProtoStates[GetContactProto(hContact)].Status, hContact).Autoreply = VAL_USEDEFAULT;
	return 0;
}

static int Create_TopToolbar(WPARAM, LPARAM)
{
	int SendOnEvent = CContactSettings(g_ProtoStates[(char*)NULL].Status).Autoreply;
	if (ServiceExists(MS_TTB_REMOVEBUTTON)) {
		TTBButton ttbb = { sizeof(ttbb) };
		ttbb.name = LPGEN("Toggle autoreply on/off");;
		ttbb.hIconHandleUp = iconList[0].hIcolib;
		ttbb.hIconHandleDn = iconList[1].hIcolib;
		ttbb.pszService = MS_AWAYSYS_AUTOREPLY_TOGGLE;
		ttbb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON | TTBBF_VISIBLE;
		ttbb.pszTooltipDn = LPGEN("Toggle autoreply off");
		ttbb.pszTooltipUp = LPGEN("Toggle autoreply on");
		g_hTopToolbarbutton = TopToolbar_AddButton(&ttbb);
		
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)g_hTopToolbarbutton, SendOnEvent ? TTBST_PUSHED : 0);
	}
	return 0;
}

static int IconsChanged(WPARAM, LPARAM)
{
	g_IconList.ReloadIcons();
	
	if (g_MessagesOptPage.GetWnd())
		SendMessage(g_MessagesOptPage.GetWnd(), UM_ICONSCHANGED, 0, 0);

	if (g_MoreOptPage.GetWnd())
		SendMessage(g_MoreOptPage.GetWnd(), UM_ICONSCHANGED, 0, 0);

	if (g_AutoreplyOptPage.GetWnd())
		SendMessage(g_AutoreplyOptPage.GetWnd(), UM_ICONSCHANGED, 0, 0);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// variables

struct
{
	TCHAR *Name;
	char *Descr;
	int Flags;
}
static Variables[] =
{
	{ VAR_AWAYSINCE_TIME, LPGEN("New Away System")"\t(x)\t"LPGEN("Away since time in default format; ?nas_awaysince_time(x) in format x"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_AWAYSINCE_DATE, LPGEN("New Away System")"\t(x)\t"LPGEN("Away since date in default format; ?nas_awaysince_date(x) in format x"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_STATDESC, LPGEN("New Away System")"\t"LPGEN("Status description"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_MYNICK, LPGEN("New Away System")"\t"LPGEN("Your nick for current protocol"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_REQUESTCOUNT, LPGEN("New Away System")"\t"LPGEN("Number of status message requests from the contact"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_MESSAGENUM, LPGEN("New Away System")"\t"LPGEN("Number of messages from the contact"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_TIMEPASSED, LPGEN("New Away System")"\t"LPGEN("Time passed until request"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_PREDEFINEDMESSAGE, LPGEN("New Away System")"\t(x)\t"LPGEN("Returns one of your predefined messages by its title: ?nas_predefinedmessage(creepy)"), TRF_FUNCTION },
	{ VAR_PROTOCOL, LPGEN("New Away System")"\t"LPGEN("Current protocol name"), TRF_FIELD | TRF_FUNCTION }
};

INT_PTR srvVariablesHandler(WPARAM, LPARAM lParam)
{
	ARGUMENTSINFO *ai = (ARGUMENTSINFO*)lParam;
	ai->flags = AIF_DONTPARSE;
	TCString Result;
	if (!lstrcmp(ai->targv[0], VAR_AWAYSINCE_TIME)) {
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, g_ProtoStates[VarParseData.szProto].AwaySince, (ai->argc > 1 && *ai->targv[1]) ? ai->targv[1] : _T("H:mm"), Result.GetBuffer(256), 256);
		Result.ReleaseBuffer();
	}
	else if (!lstrcmp(ai->targv[0], VAR_AWAYSINCE_DATE)) {
		GetDateFormat(LOCALE_USER_DEFAULT, 0, g_ProtoStates[VarParseData.szProto].AwaySince, (ai->argc > 1 && *ai->targv[1]) ? ai->targv[1] : NULL, Result.GetBuffer(256), 256);
		Result.ReleaseBuffer();
	}
	else if (!lstrcmp(ai->targv[0], VAR_STATDESC)) {
		Result = (VarParseData.Flags & VPF_XSTATUS) ? STR_XSTATUSDESC : pcli->pfnGetStatusModeDescription(g_ProtoStates[VarParseData.szProto].Status, 0);
	}
	else if (!lstrcmp(ai->targv[0], VAR_MYNICK)) {
		if (g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_MYNICKPERPROTO) && VarParseData.szProto)
			Result = db_get_s(NULL, VarParseData.szProto, "Nick", (TCHAR*)NULL);

		if (Result == NULL)
			Result = pcli->pfnGetContactDisplayName(NULL, 0);

		if (Result == NULL)
			Result = TranslateT("Stranger");
	}
	else if (!lstrcmp(ai->targv[0], VAR_REQUESTCOUNT)) {
		mir_sntprintf(Result.GetBuffer(16), 16, _T("%d"), db_get_w(ai->fi->hContact, MOD_NAME, DB_REQUESTCOUNT, 0));
		Result.ReleaseBuffer();
	}
	else if (!lstrcmp(ai->targv[0], VAR_MESSAGENUM)) {
		mir_sntprintf(Result.GetBuffer(16), 16, _T("%d"), db_get_w(ai->fi->hContact, MOD_NAME, DB_MESSAGECOUNT, 0));
		Result.ReleaseBuffer();
	}
	else if (!lstrcmp(ai->targv[0], VAR_TIMEPASSED)) {
		ULARGE_INTEGER ul_AwaySince, ul_Now;
		SYSTEMTIME st;
		GetLocalTime(&st);
		SystemTimeToFileTime(&st, (LPFILETIME)&ul_Now);
		SystemTimeToFileTime(g_ProtoStates[VarParseData.szProto].AwaySince, (LPFILETIME)&ul_AwaySince);
		ul_Now.QuadPart -= ul_AwaySince.QuadPart;
		ul_Now.QuadPart /= 10000000; // now it's in seconds
		Result.GetBuffer(256);
		if (ul_Now.LowPart >= 7200) // more than 2 hours
			mir_sntprintf(Result, 256, TranslateT("%d hours"), ul_Now.LowPart / 3600);
		else if (ul_Now.LowPart >= 120) // more than 2 minutes
			mir_sntprintf(Result, 256, TranslateT("%d minutes"), ul_Now.LowPart / 60);
		else
			mir_sntprintf(Result, 256, TranslateT("%d seconds"), ul_Now.LowPart);
		Result.ReleaseBuffer();
	}
	else if (!lstrcmp(ai->targv[0], VAR_PREDEFINEDMESSAGE)) {
		ai->flags = 0; // reset AIF_DONTPARSE flag
		if (ai->argc != 2)
			return NULL;

		COptPage MsgTreeData(g_MsgTreePage);
		COptItem_TreeCtrl *TreeCtrl = (COptItem_TreeCtrl*)MsgTreeData.Find(IDV_MSGTREE);
		TreeCtrl->DBToMem(CString(MOD_NAME));

		for (int i = 0; i < TreeCtrl->Value.GetSize(); i++) {
			if (!(TreeCtrl->Value[i].Flags & TIF_GROUP) && !_tcsicmp(TreeCtrl->Value[i].Title, ai->targv[1])) {
				Result = TreeCtrl->Value[i].User_Str1;
				break;
			}
		}
		if (Result == NULL) // if we didn't find a message with specified title
			return NULL; // return it now, as later we change NULL to ""
	}
	else if (!lstrcmp(ai->targv[0], VAR_PROTOCOL)) {
		if (VarParseData.szProto) {
			CString AnsiResult;
			CallProtoService(VarParseData.szProto, PS_GETNAME, 256, (LPARAM)AnsiResult.GetBuffer(256));
			AnsiResult.ReleaseBuffer();
			Result = _A2T(AnsiResult);
		}
		if (Result == NULL) // if we didn't find a message with specified title
			return NULL; // return it now, as later we change NULL to ""
	}
	TCHAR *szResult = (TCHAR*)malloc((Result.GetLen() + 1) * sizeof(TCHAR));
	if (!szResult)
		return NULL;

	_tcscpy(szResult, (Result != NULL) ? Result : _T(""));
	return (INT_PTR)szResult;
}

INT_PTR srvFreeVarMem(WPARAM, LPARAM lParam)
{
	if (!lParam)
		return -1;

	free((void*)lParam);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int MirandaLoaded(WPARAM, LPARAM)
{
	LoadMsgTreeModule();
	LoadCListModule();
	InitUpdateMsgs();
	g_IconList.ReloadIcons();

	int numAccs = 0;
	PROTOACCOUNT **accs;
	ProtoEnumAccounts(&numAccs, &accs);
	for (int i = 0, CurProtoIndex = 0; i < numAccs && CurProtoIndex < MAXICQACCOUNTS; i++) {
		HANDLE hHook = HookEvent(CString(accs[i]->szModuleName) + ME_ICQ_STATUSMSGREQ, StatusMsgReqHooks[CurProtoIndex]);
		if (hHook) {
			ICQProtoList[CurProtoIndex] = accs[i]->szModuleName;
			CurProtoIndex++;
		}
	}
	
	CreateServiceFunction(MS_AWAYSYS_SETCONTACTSTATMSG, SetContactStatMsg);
	CreateServiceFunction(MS_AWAYSYS_AUTOREPLY_TOGGLE, ToggleSendOnEvent);
	CreateServiceFunction(MS_AWAYSYS_AUTOREPLY_ON, srvAutoreplyOn);
	CreateServiceFunction(MS_AWAYSYS_AUTOREPLY_OFF, srvAutoreplyOff);
	CreateServiceFunction(MS_AWAYSYS_AUTOREPLY_USEDEFAULT, srvAutoreplyUseDefault);
	CreateServiceFunction(MS_AWAYMSG_GETSTATUSMSGW, GetStatusMsgW);

	CreateServiceFunction(MS_NAS_GETSTATEA, GetStateA);
	CreateServiceFunction(MS_NAS_SETSTATEA, SetStateA);
	CreateServiceFunction(MS_NAS_GETSTATEW, GetStateW);
	CreateServiceFunction(MS_NAS_SETSTATEW, SetStateW);
	CreateServiceFunction(MS_NAS_INVOKESTATUSWINDOW, InvokeStatusWindow);
	

	// and old AwaySysMod service, for compatibility reasons
	CreateServiceFunction(MS_AWAYSYS_SETSTATUSMODE, SetStatusMode);

	HookEvent(ME_TTB_MODULELOADED, Create_TopToolbar);
	HookEvent(ME_OPT_INITIALISE, OptsDlgInit);
	HookEvent(ME_CLIST_STATUSMODECHANGE, StatusChanged);
	HookEvent(ME_CS_STATUSCHANGEEX, CSStatusChange); // for compatibility with StartupStatus and AdvancedAutoAway
	HookEvent(ME_DB_EVENT_FILTER_ADD, MsgEventAdded);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);
	HookEvent(ME_IDLE_CHANGED, IdleChangeEvent);
	
	g_hReadWndList = WindowList_Create();
	
	int SendOnEvent = CContactSettings(g_ProtoStates[(char*)NULL].Status).Autoreply;

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.position = 1000020000;
	mi.flags = CMIF_TCHAR | CMIF_NOTOFFLINE;
	mi.icolibItem = iconList[SendOnEvent ? 1 : 0].hIcolib;
	mi.ptszName = SendOnEvent ? DISABLE_SOE_COMMAND : ENABLE_SOE_COMMAND;
	mi.pszService = MS_AWAYSYS_AUTOREPLY_TOGGLE;
	g_hToggleSOEMenuItem = Menu_AddMainMenuItem(&mi);

	ZeroMemory(&mi, sizeof(mi));
	mi.cbSize = sizeof(mi);
	mi.position = -2000005000;
	mi.flags = CMIF_TCHAR | CMIF_NOTOFFLINE | CMIF_HIDDEN;
	mi.ptszName = LPGENT("Read status message"); // never seen...
	mi.pszService = MS_AWAYMSG_SHOWAWAYMSG;
	g_hReadStatMenuItem = Menu_AddContactMenuItem(&mi);
	
	if (g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_USEMENUITEM)) {
		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.flags = CMIF_TCHAR | CMIF_HIDDEN;
		mi.ptszName = LPGENT("Set status message"); // will never be shown
		mi.position = 1000020000;
		mi.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_MSGICON));
		mi.pszService = MS_AWAYSYS_SETCONTACTSTATMSG;
		g_hContactMenuItem = Menu_AddContactMenuItem(&mi);

		ZeroMemory(&mi, sizeof(mi));
		mi.cbSize = sizeof(mi);
		mi.flags = CMIF_TCHAR | CMIF_ROOTPOPUP;
		mi.hIcon = NULL;
		mi.pszPopupName = (char*)-1;
		mi.position = 1000020000;
		mi.ptszName = LPGENT("Autoreply");
		g_hToggleSOEContactMenuItem = Menu_AddContactMenuItem(&mi);

		mi.flags = CMIF_TCHAR | CMIF_CHILDPOPUP;
		mi.hParentMenu = g_hToggleSOEContactMenuItem;
		mi.popupPosition = 1000020000;
		mi.position = 1000020000;

		mi.icolibItem = iconList[1].hIcolib;
		mi.ptszName = LPGENT("On");
		mi.pszService = MS_AWAYSYS_AUTOREPLY_ON;
		g_hAutoreplyOnContactMenuItem = Menu_AddContactMenuItem(&mi);

		mi.icolibItem = iconList[0].hIcolib;
		mi.ptszName = LPGENT("Off");
		mi.pszService = MS_AWAYSYS_AUTOREPLY_OFF;
		g_hAutoreplyOffContactMenuItem = Menu_AddContactMenuItem(&mi);

		mi.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_DOT));
		mi.ptszName = LPGENT("Use the default setting");
		mi.pszService = MS_AWAYSYS_AUTOREPLY_USEDEFAULT;
		g_hAutoreplyUseDefaultContactMenuItem = Menu_AddContactMenuItem(&mi);
	}
	// add that funky thingy (just tweaked a bit, was spotted in Miranda's src code)
	// we have to read the status message from contacts too... err
	CreateServiceFunction(MS_AWAYMSG_SHOWAWAYMSG, GetContactStatMsg);

	SkinAddNewSoundEx(AWAYSYS_STATUSMSGREQUEST_SOUND, NULL, LPGEN("NewAwaySys: Incoming status message request"));

	if (ServiceExists(MS_VARS_REGISTERTOKEN)) {
		CreateServiceFunction(MS_AWAYSYS_FREEVARMEM, srvFreeVarMem);
		CreateServiceFunction(MS_AWAYSYS_VARIABLESHANDLER, srvVariablesHandler);

		TOKENREGISTER tr = { sizeof(tr) };
		tr.szService = MS_AWAYSYS_VARIABLESHANDLER;
		tr.szCleanupService = MS_AWAYSYS_FREEVARMEM;
		tr.memType = TR_MEM_OWNER;
		for (int i = 0; i < SIZEOF(Variables); i++) {
			tr.flags = Variables[i].Flags | TRF_CALLSVC | TRF_TCHAR;
			tr.tszTokenString = Variables[i].Name;
			tr.szHelpText = Variables[i].Descr;
			CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM)&tr);
		}
	}

	HookEvent(ME_MODERNOPT_INITIALIZE, ModernOptInitialise);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Load(void)
{
	mir_getLP(&pluginInfo);
	mir_getCLI();

	HookEvent(ME_SYSTEM_MODULESLOADED, MirandaLoaded);

	Icon_Register(g_hInstance, MOD_NAME, iconList, SIZEOF(iconList), "nas");

	InitCommonControls();
	InitOptions(); // must be called before we hook CallService

	if (db_get_b(NULL, MOD_NAME, DB_SETTINGSVER, 0) < 1) { // change all %nas_message% variables to %extratext% if it wasn't done before
		TCString Str = db_get_s(NULL, MOD_NAME, "PopupsFormat", _T(""));
		if (Str.GetLen())
			db_set_ts(NULL, MOD_NAME, "PopupsFormat", Str.Replace(_T("nas_message"), _T("extratext")));

		Str = db_get_s(NULL, MOD_NAME, "ReplyPrefix", _T(""));
		if (Str.GetLen())
			db_set_ts(NULL, MOD_NAME, "ReplyPrefix", Str.Replace(_T("nas_message"), _T("extratext")));
	}
	if (db_get_b(NULL, MOD_NAME, DB_SETTINGSVER, 0) < 2) { // disable autoreply for not-on-list contacts, as such contact may be a spam bot
		db_set_b(NULL, MOD_NAME, ContactStatusToDBSetting(0, DB_ENABLEREPLY, 0, INVALID_CONTACT_ID), 0);
		db_set_b(NULL, MOD_NAME, DB_SETTINGSVER, 2);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" int __declspec(dllexport) Unload()
{
	WindowList_Destroy(g_hReadWndList);
	return 0;
}

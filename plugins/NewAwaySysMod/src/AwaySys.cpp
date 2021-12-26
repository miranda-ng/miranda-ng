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

#include "stdafx.h"
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

CMPlugin g_plugin;

HANDLE g_hTopToolbarbutton;
HGENMENU g_hToggleSOEMenuItem, g_hToggleSOEContactMenuItem, g_hContactMenuItem, g_hReadStatMenuItem;
HGENMENU g_hAutoreplyOnContactMenuItem, g_hAutoreplyOffContactMenuItem, g_hAutoreplyUseDefaultContactMenuItem;
bool g_fNoProcessing = false; // tells the status change proc not to do anything
int g_bIsIdle = false;
VAR_PARSE_DATA VarParseData;

static IconItem iconList[] =
{
	{ LPGEN("Toggle Off"),         "off",        IDI_SOE_DISABLED },
	{ LPGEN("Toggle On"),          "on",         IDI_SOE_ENABLED  },
	{ LPGEN("New message"),        "new_msg",    IDI_NEWMESSAGE   },
	{ LPGEN("New category"),       "new_cat",    IDI_NEWCATEGORY  },
	{ LPGEN("Delete"),             "delete",     IDI_DELETE       },
	{ LPGEN("Dot"),                "dot",        IDI_DOT          },
	{ LPGEN("Ignore"),             "ignore",     IDI_IGNORE       },
	{ LPGEN("Indefinite"),         "indefinite", IDI_INDEFINITE   },
	{ LPGEN("Set status message"), "msg_icon",   IDI_MSGICON      },
	{ LPGEN("Add"),                "add",        IDI_SAVEASNEW    },
	{ LPGEN("Save"),               "save",       IDI_SAVE         },
	{ LPGEN("Settings"),           "settings",   IDI_SETTINGS     },
};

/////////////////////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx =
{
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	{ 0xb2dd9270, 0xce5e, 0x11df, { 0xbd, 0x3d, 0x8, 0x0, 0x20, 0xc, 0x9a, 0x66 } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{}

/////////////////////////////////////////////////////////////////////////////////////////

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_SRAWAY, MIID_LAST }; 

/////////////////////////////////////////////////////////////////////////////////////////

TCString GetDynamicStatMsg(MCONTACT hContact, char *szProto, uint32_t UIN, int iStatus)
{
	// hContact is the contact that requests the status message
	if (hContact != INVALID_CONTACT_ID)
		VarParseData.Message = CContactSettings(iStatus, hContact).GetMsgFormat(GMF_ANYCURRENT, nullptr, szProto);
	else // contact is unknown
		VarParseData.Message = CProtoSettings(szProto, iStatus).GetMsgFormat(iStatus ? GMF_LASTORDEFAULT : GMF_ANYCURRENT);

	TCString sTime;
	VarParseData.szProto = szProto ? szProto : ((hContact && hContact != INVALID_CONTACT_ID) ? Proto_GetBaseAccountName(hContact) : nullptr);
	VarParseData.UIN = UIN;
	VarParseData.Flags = 0;
	if (ServiceExists(MS_VARS_FORMATSTRING) && !g_SetAwayMsgPage.GetDBValueCopy(IDS_SAWAYMSG_DISABLEVARIABLES)) {
		wchar_t *szResult = variables_parse(VarParseData.Message, nullptr, hContact);
		if (szResult) {
			VarParseData.Message = szResult;
			mir_free(szResult);
		}
	}
	return VarParseData.Message = VarParseData.Message.Left(AWAY_MSGDATA_MAX);
}

// wParam = iMode
// lParam = (char*)szProto

struct
{
	int Status, DisableReplyCtlID, DontShowDialogCtlID;
}
static StatusModeList[] = {
	{ ID_STATUS_ONLINE, IDC_REPLYDLG_DISABLE_ONL, IDC_MOREOPTDLG_DONTPOPDLG_ONL },
	{ ID_STATUS_AWAY, IDC_REPLYDLG_DISABLE_AWAY, IDC_MOREOPTDLG_DONTPOPDLG_AWAY },
	{ ID_STATUS_NA, IDC_REPLYDLG_DISABLE_NA, IDC_MOREOPTDLG_DONTPOPDLG_NA },
	{ ID_STATUS_OCCUPIED, IDC_REPLYDLG_DISABLE_OCC, IDC_MOREOPTDLG_DONTPOPDLG_OCC },
	{ ID_STATUS_DND, IDC_REPLYDLG_DISABLE_DND, IDC_MOREOPTDLG_DONTPOPDLG_DND },
	{ ID_STATUS_FREECHAT, IDC_REPLYDLG_DISABLE_FFC, IDC_MOREOPTDLG_DONTPOPDLG_FFC },
	{ ID_STATUS_INVISIBLE, IDC_REPLYDLG_DISABLE_INV, IDC_MOREOPTDLG_DONTPOPDLG_INV },
};

int StatusChanged(WPARAM wParam, LPARAM lParam)
{
	LogMessage("MS_CLIST_SETSTATUSMODE called. szProto=%s, Status=%d", lParam ? (char*)lParam : "NULL", wParam);
	g_ProtoStates[(char*)lParam].m_status = wParam;
	// let's check if we handle this thingy
	if (g_fNoProcessing) { // we're told not to do anything
		g_fNoProcessing = false; // take it off
		return 0;
	}
	uint32_t Flag1 = 0;
	uint32_t Flag3 = 0;
	if (lParam) {
		Flag1 = CallProtoService((char*)lParam, PS_GETCAPS, PFLAGNUM_1, 0);
		Flag3 = CallProtoService((char*)lParam, PS_GETCAPS, PFLAGNUM_3, 0);
	}
	else {
		for (auto &pa : Accounts()) {
			Flag1 |= CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_1, 0);
			Flag3 |= CallProtoService(pa->szModuleName, PS_GETCAPS, PFLAGNUM_3, 0);
		}
	}

	if (!(Flag1 & PF1_MODEMSGSEND || Flag3 & Proto_Status2Flag(wParam) || (Flag1 & PF1_IM) == PF1_IM))
		return 0; // there are no protocols with changed status that support autoreply or away messages for this status

	if (g_SetAwayMsgPage.GetWnd()) {
		SetForegroundWindow(g_SetAwayMsgPage.GetWnd());
		return 0;
	}

	int i;
	for (i = _countof(StatusModeList) - 1; i >= 0; i--)
		if ((int)wParam == StatusModeList[i].Status)
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
		memset(dat, 0, sizeof(SetAwayMsgData));
		dat->szProtocol = (char*)lParam;
		dat->IsModeless = false;
		DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SETAWAYMSG), nullptr, SetAwayMsgDlgProc, (LPARAM)dat);
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
		LogMessage("%d: szProto=%s, status=%d, lastStatus=%d, szMsg:", 
			i + 1, ps[i]->m_szName ? (char*)ps[i]->m_szName : "NULL", ps[i]->m_status, ps[i]->m_lastStatus, ps[i]->m_szMsg ? ps[i]->m_szMsg : L"NULL");
		if (ps[i]->m_status != ID_STATUS_DISABLED) {
			if (ps[i]->m_status != ID_STATUS_CURRENT)
				g_ProtoStates[ps[i]->m_szName].m_status = (ps[i]->m_status == ID_STATUS_LAST) ? ps[i]->m_lastStatus : ps[i]->m_status;

			CProtoSettings(ps[i]->m_szName).SetMsgFormat(SMF_TEMPORARY, ps[i]->m_szMsg ? ps[i]->m_szMsg : CProtoSettings(ps[i]->m_szName).GetMsgFormat(GMF_LASTORDEFAULT));
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
	char *szProto = Proto_GetBaseAccountName(hContact);
	int iMode = szProto ? Proto_GetStatus(szProto) : 0;
	int Flag1 = szProto ? CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0) : 0;
	int iContactMode = db_get_w(hContact, szProto, "Status", ID_STATUS_OFFLINE);
	wchar_t szSetStr[256], szReadStr[256];
	szSetStr[0] = szReadStr[0] = 0;
	HANDLE hReadMsgIcon = nullptr;

	if (szProto) {
		int i;
		for (i = _countof(StatusModeList) - 1; i >= 0; i--)
			if (iMode == StatusModeList[i].Status)
				break;

		// the protocol supports status message sending for current status, or autoreplying
		if ((Flag1 & PF1_MODEMSGSEND && CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(iMode)) || 
			((Flag1 & PF1_IM) == PF1_IM && (i < 0 || !g_AutoreplyOptPage.GetDBValueCopy(StatusModeList[i].DisableReplyCtlID))))
			mir_snwprintf(szSetStr, TranslateT("Set %s message for the contact"), Clist_GetStatusModeDescription(iMode, 0), Clist_GetContactDisplayName(hContact));

		// the protocol supports status message reading for contact's status
		if (Flag1 & PF1_MODEMSGRECV && CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_3, 0) & Proto_Status2Flag(iContactMode)) {
			mir_snwprintf(szReadStr, TranslateT("Re&ad %s message"), Clist_GetStatusModeDescription(iContactMode, 0));
			hReadMsgIcon = Skin_GetProtoIcon(szProto, iContactMode);
		}
	}
	if (g_hContactMenuItem) {
		if (szSetStr[0] == 0)
			Menu_ShowItem(g_hContactMenuItem, false);
		else 
			Menu_ModifyItem(g_hContactMenuItem, szSetStr, iconList[8].hIcolib, 0);

		// if this contact supports sending/receiving messages
		if ((Flag1 & PF1_IM) == PF1_IM) {
			int iAutoreply = CContactSettings(g_ProtoStates[szProto].m_status, hContact).Autoreply;
			HANDLE hIcon;
			switch (iAutoreply) {
				case VAL_USEDEFAULT: hIcon = Skin_GetIconHandle(IDI_DOT); break;
				case 0: hIcon = iconList[0].hIcolib; break;
				default: iAutoreply = 1; hIcon = iconList[1].hIcolib; break;
			}
			Menu_ModifyItem(g_hToggleSOEContactMenuItem, nullptr, hIcon, 0);
			Menu_ModifyItem(g_hAutoreplyOnContactMenuItem, nullptr, iconList[1].hIcolib, (iAutoreply == 1) ? CMIF_CHECKED : 0);
			Menu_ModifyItem(g_hAutoreplyOffContactMenuItem, nullptr, iconList[0].hIcolib, (iAutoreply == 0) ? CMIF_CHECKED : 0);
			Menu_ModifyItem(g_hAutoreplyUseDefaultContactMenuItem, nullptr, iconList[5].hIcolib, (iAutoreply == VAL_USEDEFAULT) ? CMIF_CHECKED : 0);
		}
		else // hide the Autoreply menu item
			Menu_ShowItem(g_hToggleSOEContactMenuItem, false);
	}

	if (szReadStr[0] == 0)
		Menu_ShowItem(g_hReadStatMenuItem, false);
	else
		Menu_ModifyItem(g_hReadStatMenuItem, szReadStr, hReadMsgIcon, 0);
	return 0;
}

static int DBSettingChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING *cws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact != NULL && !strcmp(cws->szSetting, "Status"))
		db_set_w(hContact, "UserOnline", "OldStatus", cws->value.wVal);
	
	return 0;
}

static INT_PTR SetContactStatMsg(WPARAM hContact, LPARAM)
{
	if (g_SetAwayMsgPage.GetWnd()) { // already setting something
		SetForegroundWindow(g_SetAwayMsgPage.GetWnd());
		return 0;
	}
	
	SetAwayMsgData *dat = new SetAwayMsgData;
	memset(dat, 0, sizeof(SetAwayMsgData));
	dat->hInitContact = hContact;
	dat->szProtocol = Proto_GetBaseAccountName(hContact);
	dat->IsModeless = false;
	DialogBoxParam(g_plugin.getInst(), MAKEINTRESOURCE(IDD_SETAWAYMSG), nullptr, SetAwayMsgDlgProc, (LPARAM)dat);
	return 0;
}

INT_PTR ToggleSendOnEvent(WPARAM hContact, LPARAM)
{
	// used only for the global setting
	CContactSettings(g_ProtoStates[hContact ? Proto_GetBaseAccountName(hContact) : nullptr].m_status, hContact).Autoreply.Toggle();

	if (hContact == NULL) {
		int SendOnEvent = CContactSettings(g_ProtoStates[(LPSTR)NULL].m_status).Autoreply;
		
		if (SendOnEvent)
			Menu_ModifyItem(g_hToggleSOEMenuItem, DISABLE_SOE_COMMAND, iconList[1].hIcolib);
		else
			Menu_ModifyItem(g_hToggleSOEMenuItem, ENABLE_SOE_COMMAND, iconList[0].hIcolib);

		if (g_hTopToolbarbutton)
			CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)g_hTopToolbarbutton, SendOnEvent ? TTBST_PUSHED : 0);
	}

	return 0;
}

INT_PTR srvAutoreplyOn(WPARAM hContact, LPARAM)
{
	CContactSettings(g_ProtoStates[Proto_GetBaseAccountName(hContact)].m_status, hContact).Autoreply = 1;
	return 0;
}

INT_PTR srvAutoreplyOff(WPARAM hContact, LPARAM)
{
	CContactSettings(g_ProtoStates[Proto_GetBaseAccountName(hContact)].m_status, hContact).Autoreply = 0;
	return 0;
}

INT_PTR srvAutoreplyUseDefault(WPARAM hContact, LPARAM)
{
	CContactSettings(g_ProtoStates[Proto_GetBaseAccountName(hContact)].m_status, hContact).Autoreply = VAL_USEDEFAULT;
	return 0;
}

static int Create_TopToolbar(WPARAM, LPARAM)
{
	int SendOnEvent = CContactSettings(g_ProtoStates[(char*)NULL].m_status).Autoreply;
	if (ServiceExists(MS_TTB_REMOVEBUTTON)) {
		TTBButton ttbb = { 0 };
		ttbb.name = LPGEN("Toggle autoreply on/off");;
		ttbb.hIconHandleUp = iconList[0].hIcolib;
		ttbb.hIconHandleDn = iconList[1].hIcolib;
		ttbb.pszService = MS_AWAYSYS_AUTOREPLY_TOGGLE;
		ttbb.dwFlags = TTBBF_SHOWTOOLTIP | TTBBF_ASPUSHBUTTON | TTBBF_VISIBLE;
		ttbb.pszTooltipDn = LPGEN("Toggle autoreply off");
		ttbb.pszTooltipUp = LPGEN("Toggle autoreply on");
		g_hTopToolbarbutton = g_plugin.addTTB(&ttbb);
		
		CallService(MS_TTB_SETBUTTONSTATE, (WPARAM)g_hTopToolbarbutton, SendOnEvent ? TTBST_PUSHED : 0);
	}
	return 0;
}

static int IconsChanged(WPARAM, LPARAM)
{
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
	wchar_t *Name;
	char *Descr;
	int Flags;
}
static Variables[] =
{
	{ VAR_AWAYSINCE_TIME, LPGEN("New Away System") "\t(x)\t" LPGEN("Away since time in default format; ?nas_awaysince_time(x) in format x"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_AWAYSINCE_DATE, LPGEN("New Away System") "\t(x)\t" LPGEN("Away since date in default format; ?nas_awaysince_date(x) in format x"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_STATDESC, LPGEN("New Away System") "\t" LPGEN("Status description"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_MYNICK, LPGEN("New Away System") "\t" LPGEN("Your nick for current protocol"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_REQUESTCOUNT, LPGEN("New Away System") "\t" LPGEN("Number of status message requests from the contact"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_MESSAGENUM, LPGEN("New Away System") "\t" LPGEN("Number of messages from the contact"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_TIMEPASSED, LPGEN("New Away System") "\t" LPGEN("Time passed until request"), TRF_FIELD | TRF_FUNCTION },
	{ VAR_PREDEFINEDMESSAGE, LPGEN("New Away System") "\t(x)\t" LPGEN("Returns one of your predefined messages by its title: ?nas_predefinedmessage(creepy)"), TRF_FUNCTION },
	{ VAR_PROTOCOL, LPGEN("New Away System") "\t" LPGEN("Current protocol name"), TRF_FIELD | TRF_FUNCTION }
};

INT_PTR srvVariablesHandler(WPARAM, LPARAM lParam)
{
	ARGUMENTSINFO *ai = (ARGUMENTSINFO*)lParam;
	ai->flags = AIF_DONTPARSE;
	TCString Result;
	if (!mir_wstrcmp(ai->argv.w[0], VAR_AWAYSINCE_TIME)) {
		GetTimeFormat(LOCALE_USER_DEFAULT, 0, g_ProtoStates[VarParseData.szProto].m_awaySince, (ai->argc > 1 && *ai->argv.w[1]) ? ai->argv.w[1] : L"H:mm", Result.GetBuffer(256), 256);
		Result.ReleaseBuffer();
	}
	else if (!mir_wstrcmp(ai->argv.w[0], VAR_AWAYSINCE_DATE)) {
		GetDateFormat(LOCALE_USER_DEFAULT, 0, g_ProtoStates[VarParseData.szProto].m_awaySince, (ai->argc > 1 && *ai->argv.w[1]) ? ai->argv.w[1] : nullptr, Result.GetBuffer(256), 256);
		Result.ReleaseBuffer();
	}
	else if (!mir_wstrcmp(ai->argv.w[0], VAR_STATDESC)) {
		Result = (VarParseData.Flags & VPF_XSTATUS) ? STR_XSTATUSDESC : Clist_GetStatusModeDescription(g_ProtoStates[VarParseData.szProto].m_status, 0);
	}
	else if (!mir_wstrcmp(ai->argv.w[0], VAR_MYNICK)) {
		if (g_MoreOptPage.GetDBValueCopy(IDC_MOREOPTDLG_MYNICKPERPROTO) && VarParseData.szProto)
			Result = db_get_s(0, VarParseData.szProto, "Nick", (wchar_t*)nullptr);

		if (Result == nullptr)
			Result = Clist_GetContactDisplayName(0);

		if (Result == nullptr)
			Result = TranslateT("Stranger");
	}
	else if (!mir_wstrcmp(ai->argv.w[0], VAR_REQUESTCOUNT)) {
		mir_snwprintf(Result.GetBuffer(16), 16, L"%d", db_get_w(ai->fi->hContact, MODULENAME, DB_REQUESTCOUNT, 0));
		Result.ReleaseBuffer();
	}
	else if (!mir_wstrcmp(ai->argv.w[0], VAR_MESSAGENUM)) {
		mir_snwprintf(Result.GetBuffer(16), 16, L"%d", db_get_w(ai->fi->hContact, MODULENAME, DB_MESSAGECOUNT, 0));
		Result.ReleaseBuffer();
	}
	else if (!mir_wstrcmp(ai->argv.w[0], VAR_TIMEPASSED)) {
		ULARGE_INTEGER ul_AwaySince, ul_Now;
		SYSTEMTIME st;
		GetLocalTime(&st);
		SystemTimeToFileTime(&st, (LPFILETIME)&ul_Now);
		SystemTimeToFileTime(g_ProtoStates[VarParseData.szProto].m_awaySince, (LPFILETIME)&ul_AwaySince);
		ul_Now.QuadPart -= ul_AwaySince.QuadPart;
		ul_Now.QuadPart /= 10000000; // now it's in seconds
		Result.GetBuffer(256);
		if (ul_Now.LowPart >= 7200) // more than 2 hours
			mir_snwprintf(Result, 256, TranslateT("%d hours"), ul_Now.LowPart / 3600);
		else if (ul_Now.LowPart >= 120) // more than 2 minutes
			mir_snwprintf(Result, 256, TranslateT("%d minutes"), ul_Now.LowPart / 60);
		else
			mir_snwprintf(Result, 256, TranslateT("%d seconds"), ul_Now.LowPart);
		Result.ReleaseBuffer();
	}
	else if (!mir_wstrcmp(ai->argv.w[0], VAR_PREDEFINEDMESSAGE)) {
		ai->flags = 0; // reset AIF_DONTPARSE flag
		if (ai->argc != 2)
			return NULL;

		COptPage MsgTreeData(g_MsgTreePage);
		COptItem_TreeCtrl *TreeCtrl = (COptItem_TreeCtrl*)MsgTreeData.Find(IDV_MSGTREE);
		TreeCtrl->DBToMem(CString(MODULENAME));

		for (int i = 0; i < TreeCtrl->m_value.GetSize(); i++) {
			if (!(TreeCtrl->m_value[i].Flags & TIF_GROUP) && !mir_wstrcmpi(TreeCtrl->m_value[i].Title, ai->argv.w[1])) {
				Result = TreeCtrl->m_value[i].User_Str1;
				break;
			}
		}
		if (Result == nullptr) // if we didn't find a message with specified title
			return NULL; // return it now, as later we change NULL to ""
	}
	else if (!mir_wstrcmp(ai->argv.w[0], VAR_PROTOCOL)) {
		if (VarParseData.szProto) {
			CString AnsiResult;
			CallProtoService(VarParseData.szProto, PS_GETNAME, 256, (LPARAM)AnsiResult.GetBuffer(256));
			AnsiResult.ReleaseBuffer();
			Result = _A2T(AnsiResult);
		}
		if (Result == nullptr) // if we didn't find a message with specified title
			return NULL; // return it now, as later we change NULL to ""
	}
	wchar_t *szResult = (wchar_t*)malloc((Result.GetLen() + 1) * sizeof(wchar_t));
	if (!szResult)
		return NULL;

	mir_wstrcpy(szResult, (Result != nullptr) ? Result : L"");
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
	HookEvent(ME_CS_STATUSCHANGEEX, CSStatusChange); // for compatibility with StartupStatus and AdvancedAutoAway
	HookEvent(ME_IDLE_CHANGED, IdleChangeEvent);
	
	g_hReadWndList = WindowList_Create();
	
	int SendOnEvent = CContactSettings(g_ProtoStates[(char*)NULL].m_status).Autoreply;

	CMenuItem mi(&g_plugin);
	SET_UID(mi, 0xa379c361, 0x9e3f, 0x468d, 0xb2, 0xac, 0xc4, 0x89, 0xbb, 0xfc, 0x81, 0x15);
	mi.position = 1000020000;
	mi.flags = CMIF_UNICODE | CMIF_NOTOFFLINE;
	mi.hIcolibItem = iconList[SendOnEvent ? 1 : 0].hIcolib;
	mi.name.w = SendOnEvent ? DISABLE_SOE_COMMAND : ENABLE_SOE_COMMAND;
	mi.pszService = MS_AWAYSYS_AUTOREPLY_TOGGLE;
	g_hToggleSOEMenuItem = Menu_AddMainMenuItem(&mi);

	// contact menu items	
	memset(&mi, 0, sizeof(mi));
	SET_UID(mi, 0xd3282acc, 0x9ff1, 0x4ede, 0x8a, 0x1e, 0x36, 0x72, 0x3f, 0x44, 0x4f, 0x84);
	mi.position = -2000005000;
	mi.flags = CMIF_UNICODE | CMIF_NOTOFFLINE | CMIF_HIDDEN;
	mi.name.w = LPGENW("Read status message"); // never seen...
	mi.pszService = MS_AWAYMSG_SHOWAWAYMSG;
	g_hReadStatMenuItem = Menu_AddContactMenuItem(&mi);

	memset(&mi, 0, sizeof(mi));
	SET_UID(mi, 0xc42a4fdb, 0x51b8, 0x4bbe, 0x83, 0xee, 0x2d, 0x32, 0x29, 0x5c, 0x2, 0xb3);
	mi.flags = CMIF_UNICODE | CMIF_HIDDEN;
	mi.name.w = LPGENW("Set status message"); // will never be shown
	mi.position = 1000020000;
	mi.hIcolibItem = iconList[8].hIcolib;
	mi.pszService = MS_AWAYSYS_SETCONTACTSTATMSG;
	g_hContactMenuItem = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(g_hContactMenuItem, MCI_OPT_DISABLED, TRUE);

	memset(&mi, 0, sizeof(mi));
	SET_UID(mi, 0x47a3c631, 0x8ca9, 0x4b7e, 0x84, 0x6e, 0x29, 0xbf, 0x53, 0x30, 0x6f, 0x83);
	mi.flags = CMIF_UNICODE;
	mi.hIcolibItem = nullptr;
	mi.position = 1000020000;
	mi.name.w = LPGENW("Autoreply");
	g_hToggleSOEContactMenuItem = Menu_AddContactMenuItem(&mi);
	Menu_ConfigureItem(g_hToggleSOEContactMenuItem, MCI_OPT_DISABLED, TRUE);
	UNSET_UID(mi);

	mi.flags = CMIF_UNICODE | CMIF_SYSTEM;
	mi.root = g_hToggleSOEContactMenuItem;
	mi.position = 1000020000;

	mi.hIcolibItem = iconList[1].hIcolib;
	mi.name.w = LPGENW("On");
	mi.pszService = MS_AWAYSYS_AUTOREPLY_ON;
	g_hAutoreplyOnContactMenuItem = Menu_AddContactMenuItem(&mi);

	mi.hIcolibItem = iconList[0].hIcolib;
	mi.name.w = LPGENW("Off");
	mi.pszService = MS_AWAYSYS_AUTOREPLY_OFF;
	g_hAutoreplyOffContactMenuItem = Menu_AddContactMenuItem(&mi);

	mi.hIcolibItem = iconList[5].hIcolib;
	mi.name.w = LPGENW("Use the default setting");
	mi.pszService = MS_AWAYSYS_AUTOREPLY_USEDEFAULT;
	g_hAutoreplyUseDefaultContactMenuItem = Menu_AddContactMenuItem(&mi);

	// add that funky thingy (just tweaked a bit, was spotted in Miranda's src code)
	// we have to read the status message from contacts too... err
	CreateServiceFunction(MS_AWAYMSG_SHOWAWAYMSG, GetContactStatMsg);

	g_plugin.addSound(AWAYSYS_STATUSMSGREQUEST_SOUND, nullptr, LPGENW("NewAwaySys: Incoming status message request"));

	if (ServiceExists(MS_VARS_REGISTERTOKEN)) {
		CreateServiceFunction(MS_AWAYSYS_FREEVARMEM, srvFreeVarMem);
		CreateServiceFunction(MS_AWAYSYS_VARIABLESHANDLER, srvVariablesHandler);

		TOKENREGISTER tr = { sizeof(tr) };
		tr.szService = MS_AWAYSYS_VARIABLESHANDLER;
		tr.szCleanupService = MS_AWAYSYS_FREEVARMEM;
		tr.memType = TR_MEM_OWNER;
		for (auto &it: Variables) {
			tr.flags = it.Flags | TRF_CALLSVC | TRF_TCHAR;
			tr.szTokenString.w = it.Name;
			tr.szHelpText = it.Descr;
			CallService(MS_VARS_REGISTERTOKEN, 0, (LPARAM)&tr);
		}
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Load()
{
	HookEvent(ME_CLIST_STATUSMODECHANGE, StatusChanged);
	HookEvent(ME_CLIST_PREBUILDCONTACTMENU, PreBuildContactMenu);
	HookEvent(ME_DB_CONTACT_SETTINGCHANGED, DBSettingChanged);
	HookEvent(ME_DB_EVENT_FILTER_ADD, MsgEventAdded);
	HookEvent(ME_OPT_INITIALISE, OptsDlgInit);
	HookEvent(ME_SKIN_ICONSCHANGED, IconsChanged);
	HookEvent(ME_SYSTEM_MODULESLOADED, MirandaLoaded);

	g_plugin.registerIcon(MODULENAME, iconList, "nas");

	InitCommonControls();
	InitOptions(); // must be called before we hook CallService

	if (g_plugin.getByte(DB_SETTINGSVER, 0) < 1) { // change all %nas_message% variables to %extratext% if it wasn't done before
		TCString Str = db_get_s(0, MODULENAME, "PopupsFormat", L"");
		if (Str.GetLen())
			g_plugin.setWString("PopupsFormat", Str.Replace(L"nas_message", L"extratext"));

		Str = db_get_s(0, MODULENAME, "ReplyPrefix", L"");
		if (Str.GetLen())
			g_plugin.setWString("ReplyPrefix", Str.Replace(L"nas_message", L"extratext"));
	}
	if (g_plugin.getByte(DB_SETTINGSVER, 0) < 2) { // disable autoreply for not-on-list contacts, as such contact may be a spam bot
		g_plugin.setByte(ContactStatusToDBSetting(0, DB_ENABLEREPLY, 0, INVALID_CONTACT_ID), 0);
		g_plugin.setByte(DB_SETTINGSVER, 2);
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int CMPlugin::Unload()
{
	WindowList_Destroy(g_hReadWndList);
	return 0;
}

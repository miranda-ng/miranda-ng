//////////////////////////////////////////////////////////////////////////
// AMD local protocol main file

#include "stdafx.h"

CMPlugin	g_plugin;

//////////////////////////////////////////////////////////////////////////

CMLan* g_lan = nullptr;

bool g_InitOptions = false;

#ifdef VERBOSE
std::fstream emlanLog("EmLanLog.txt", std::ios::out|std::ios::app);
#endif

//////////////////////////////////////////////////////////////////////////

PLUGININFOEX pluginInfoEx = {
	sizeof(PLUGININFOEX),
	__PLUGIN_NAME,
	PLUGIN_MAKE_VERSION(__MAJOR_VERSION, __MINOR_VERSION, __RELEASE_NUM, __BUILD_NUM),
	__DESCRIPTION,
	__AUTHOR,
	__COPYRIGHT,
	__AUTHORWEB,
	UNICODE_AWARE,
	// {E08CE7C4-9EEB-4272-B544-0D32E18D90DE}
	{ 0xe08ce7c4, 0x9eeb, 0x4272, { 0xb5, 0x44, 0xd, 0x32, 0xe1, 0x8d, 0x90, 0xde } }
};

CMPlugin::CMPlugin() :
	PLUGIN<CMPlugin>(MODULENAME, pluginInfoEx)
{
	RegisterProtocol(PROTOTYPE_PROTOCOL);
	SetUniqueId("Nick");
}

/////////////////////////////////////////////////////////////////////////////////////////
// Interface information

extern "C" __declspec(dllexport) const MUUID MirandaInterfaces[] = { MIID_PROTOCOL, MIID_LAST };

//////////////////////////////////////////////////////////////////////////

BOOL APIENTRY DllMain(HINSTANCE hInstDLL, uint32_t reason, LPVOID)
{
	if (reason == DLL_PROCESS_ATTACH) {
		EMLOG("EmLan Started");
		DisableThreadLibraryCalls(hInstDLL);
	}
	else if (reason == DLL_PROCESS_DETACH) {
		EMLOG("EmLan Stopped");
	}

	return TRUE;
}

//////////////////////////////////////////////////////////////////////////

static INT_PTR __cdecl EMPGetCaps(WPARAM wParam, LPARAM)
{
	switch (wParam) {
	case PFLAGNUM_1:
		return PF1_IM | PF1_BASICSEARCH | PF1_ADDSEARCHRES | PF1_PEER2PEER | PF1_INDIVSTATUS |
			PF1_MODEMSG | PF1_FILE | PF1_CANRENAMEFILE | PF1_FILERESUME;
	case PFLAGNUM_2:
		return PF2_ONLINE | PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAGNUM_3:
		return PF2_SHORTAWAY | PF2_LONGAWAY | PF2_LIGHTDND | PF2_HEAVYDND | PF2_FREECHAT;
	case PFLAG_UNIQUEIDTEXT:
		return (INT_PTR)TranslateT("User name, IP address or '*'");
	default:
		return 0;
	}
}

static INT_PTR __cdecl EMPGetName(WPARAM wParam, LPARAM lParam)
{
	mir_strncpy((char*)lParam, "EmLan", wParam);
	return 0;
}

static INT_PTR __cdecl EMPLoadIcon(WPARAM wParam, LPARAM)
{
	if ((wParam & 0xFFFF) != PLI_PROTOCOL)
		return 0;
	
	return (INT_PTR)LoadIcon(g_plugin.getInst(), MAKEINTRESOURCE(IDI_ICON_ONLINE));
}

static INT_PTR __cdecl EMPGetStatus(WPARAM, LPARAM)
{
	return g_lan->GetMirandaStatus();
}

INT_PTR __cdecl EMPSetStatus(WPARAM new_status, LPARAM)
{
	g_lan->SetMirandaStatus(new_status);
	return 0;
}

static INT_PTR __cdecl EMPSendMessage(WPARAM, LPARAM lParam)
{
	return g_lan->SendMessageUrl((CCSDATA*)lParam);
}

static INT_PTR __cdecl EMPRecvMessage(WPARAM, LPARAM lParam)
{
	g_lan->RecvMessageUrl((CCSDATA*)lParam);
	return 0;
}

static INT_PTR __cdecl EMPAddToList(WPARAM flags, LPARAM lParam)
{
	return g_lan->AddToContactList((u_int)flags, (EMPSEARCHRESULT*)lParam);
}

static INT_PTR __cdecl EMPBasicSearch(WPARAM, LPARAM lParam)
{
	const wchar_t *wszName = (const wchar_t*)lParam;
	return g_lan->Search(_T2A(wszName));
}

static INT_PTR __cdecl EMPGetAwayMsg(WPARAM, LPARAM lParam)
{
	return g_lan->GetAwayMsg((CCSDATA*)lParam);
}

static INT_PTR __cdecl EMPRecvAwayMessage(WPARAM, LPARAM lParam)
{
	return g_lan->RecvAwayMsg((CCSDATA*)lParam);
}

static INT_PTR __cdecl EMPSetAwayMsg(WPARAM wParam, LPARAM lParam)
{
	return g_lan->SetAwayMsg((u_int)wParam, (char*)lParam);
}

static INT_PTR __cdecl EMPFileResume(WPARAM wParam, LPARAM lParam)
{
	return g_lan->FileResume((int)wParam, (PROTOFILERESUME*)lParam);
}

static INT_PTR __cdecl EMPSendFileAllow(WPARAM, LPARAM lParam)
{
	return g_lan->FileAllow((CCSDATA*)lParam);
}

static INT_PTR __cdecl EMPSendFileDeny(WPARAM, LPARAM lParam)
{
	return g_lan->FileDeny((CCSDATA*)lParam);
}

static INT_PTR __cdecl EMPSendFileCancel(WPARAM, LPARAM lParam)
{
	return g_lan->FileCancel((CCSDATA*)lParam);
}

static INT_PTR __cdecl EMPSendFile(WPARAM, LPARAM lParam)
{
	return g_lan->SendFile((CCSDATA*)lParam);
}

static INT_PTR __cdecl EMPRecvFile(WPARAM, LPARAM lParam)
{
	g_lan->RecvFile((CCSDATA*)lParam);
	return 0;
}

//////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK EMPDlgProcMainOpts(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		g_InitOptions = true;
		TranslateDialogDefault(hwndDlg);
		{
			int count = g_lan->GetHostAddrCount();
			in_addr caddr = g_lan->GetCurHostAddress();
			int cind = 0;
			for (int i = 0; i < count; i++) {
				in_addr addr = g_lan->GetHostAddress(i);
				wchar_t* ipStr = mir_a2u(inet_ntoa(addr));
				if (addr.S_un.S_addr == caddr.S_un.S_addr)
					cind = i;
				SendDlgItemMessage(hwndDlg, IDC_LIST_IP, LB_ADDSTRING, 0, (LPARAM)ipStr);
			}
			SendDlgItemMessage(hwndDlg, IDC_LIST_IP, LB_SETCURSEL, cind, 0);
			SetDlgItemText(hwndDlg, IDC_EDIT_NAME, g_lan->GetName());
			if (g_lan->GetUseHostName()) {
				CheckDlgButton(hwndDlg, IDC_RADIO_USECOMPNAME, BST_CHECKED);
				CheckDlgButton(hwndDlg, IDC_RADIO_USEOWN, BST_UNCHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_NAME), FALSE);
			}
			else {
				CheckDlgButton(hwndDlg, IDC_RADIO_USECOMPNAME, BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_RADIO_USEOWN, BST_CHECKED);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_NAME), TRUE);
			}
		}
		g_InitOptions = false;
		return TRUE;

	case WM_COMMAND:
		{
			if (g_InitOptions)
				break;

			bool changed = false;
			switch (LOWORD(wParam)) {
			case IDC_RADIO_USECOMPNAME:
				g_lan->SetUseHostName(true);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_NAME), FALSE);
				changed = true;
				break;
			case IDC_RADIO_USEOWN:
				g_lan->SetUseHostName(false);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_NAME), TRUE);
				changed = true;
				break;
			case IDC_LIST_IP:
				{
					u_long addr = g_lan->GetHostAddress(SendDlgItemMessage(hwndDlg, IDC_LIST_IP, LB_GETCURSEL, 0, 0)).S_un.S_addr;
					if (addr != g_lan->GetCurHostAddress().S_un.S_addr) {
						g_lan->SetRequiredIp(addr);
						changed = true;
					}
				}
				break;
			case IDC_EDIT_NAME:
				if (HIWORD(wParam) == EN_CHANGE)
					changed = true;
				break;
			}
			if (changed)
				SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
		break;
	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->idFrom) {
		case 0:
			switch (((LPNMHDR)lParam)->code) {
			case PSN_RESET:
				g_lan->LoadSettings();
				return TRUE;
			case PSN_APPLY:
				{
					int status = g_lan->GetMirandaStatus();
					g_lan->SetMirandaStatus(ID_STATUS_OFFLINE);

					GetDlgItemText(hwndDlg, IDC_EDIT_NAME, g_lan->GetName(), MAX_HOSTNAME_LEN);

					g_lan->SaveSettings();
					g_lan->LoadSettings();

					g_lan->SetMirandaStatus(status);
				}
				return TRUE;
			}
		}
		break;
	}
	return FALSE;
}

int __cdecl EMPCreateOptionsDlg(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = {};
	odp.position = 100000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_EMP_FORM_OPT);
	odp.szTitle.a = LPGEN("E-mage LAN protocol");
	odp.szGroup.a = LPGEN("Network");
	odp.flags = ODPF_BOLDGROUPS;
	odp.pfnDlgProc = EMPDlgProcMainOpts;
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

//////////////////////////////////////////////////////////////////////////

INT_PTR CALLBACK EMPDlgProcMessage(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM)
{
	HWND hwndOwner;
	RECT rc, rcDlg, rcOwner;

	switch (uMsg) {
	case WM_INITDIALOG:
		if ((hwndOwner = GetParent(hwndDlg)) == nullptr)
			hwndOwner = GetDesktopWindow();

		GetWindowRect(hwndOwner, &rcOwner);
		GetWindowRect(hwndDlg, &rcDlg);
		CopyRect(&rc, &rcOwner);

		OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
		OffsetRect(&rc, -rc.left, -rc.top);
		OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);
		SetWindowPos(hwndDlg, HWND_TOP, rcOwner.left + (rc.right / 2), rcOwner.top + (rc.bottom / 2), 0, 0,  SWP_NOSIZE);

		if (GetDlgCtrlID((HWND)wParam) != IDOK) {
			SetFocus(GetDlgItem(hwndDlg, IDOK));
			return FALSE;
		}
		return TRUE;

	case WM_CLOSE:
		EndDialog(hwndDlg, 0);
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			EndDialog(hwndDlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}

int CMPlugin::Load()
{
	g_lan = new CMLan();

	CreateProtoServiceFunction(MODULENAME, PS_GETCAPS, EMPGetCaps);
	CreateProtoServiceFunction(MODULENAME, PS_GETNAME, EMPGetName);
	CreateProtoServiceFunction(MODULENAME, PS_LOADICON, EMPLoadIcon);
	CreateProtoServiceFunction(MODULENAME, PS_SETSTATUS, EMPSetStatus);
	CreateProtoServiceFunction(MODULENAME, PS_GETSTATUS, EMPGetStatus);
	CreateProtoServiceFunction(MODULENAME, PS_BASICSEARCH, EMPBasicSearch);
	CreateProtoServiceFunction(MODULENAME, PS_ADDTOLIST, EMPAddToList);
	CreateProtoServiceFunction(MODULENAME, PSS_MESSAGE, EMPSendMessage);
	CreateProtoServiceFunction(MODULENAME, PSR_MESSAGE, EMPRecvMessage);
	CreateProtoServiceFunction(MODULENAME, PSS_GETAWAYMSG, EMPGetAwayMsg);
	CreateProtoServiceFunction(MODULENAME, PS_SETAWAYMSG, EMPSetAwayMsg);
	CreateProtoServiceFunction(MODULENAME, PSR_AWAYMSG, EMPRecvAwayMessage);
	CreateProtoServiceFunction(MODULENAME, PS_FILERESUME, EMPFileResume);
	CreateProtoServiceFunction(MODULENAME, PSS_FILEALLOW, EMPSendFileAllow);
	CreateProtoServiceFunction(MODULENAME, PSS_FILEDENY, EMPSendFileDeny);
	CreateProtoServiceFunction(MODULENAME, PSS_FILECANCEL, EMPSendFileCancel);
	CreateProtoServiceFunction(MODULENAME, PSS_FILE, EMPSendFile);
	CreateProtoServiceFunction(MODULENAME, PSR_FILE, EMPRecvFile);

	HookEvent(ME_OPT_INITIALISE, EMPCreateOptionsDlg);
	return 0;
}

int CMPlugin::Unload()
{
	delete g_lan;
	return 0;
}

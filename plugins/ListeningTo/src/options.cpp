/* 
Copyright (C) 2006 Ricardo Pescuma Domenecci

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#include "commons.h"

// Prototypes /////////////////////////////////////////////////////////////////////////////////////

HANDLE hOptHook = NULL;

Options opts;

extern std::vector<ProtocolInfo> proto_items;
extern HANDLE hExtraIcon;

BOOL ListeningToEnabled(char *proto, BOOL ignoreGlobal = FALSE);

static INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK PlayersDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);
static INT_PTR CALLBACK FormatDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


static OptPageControl optionsControls[] = { 
	{ &opts.enable_sending,				CONTROL_CHECKBOX,	IDC_ENABLE_SEND,				"EnableSend", TRUE },
	{ &opts.enable_music,				CONTROL_CHECKBOX,	IDC_ENABLE_MUSIC,				"EnableMusic", TRUE },
	{ &opts.enable_radio,				CONTROL_CHECKBOX,	IDC_ENABLE_RADIO,				"EnableRadio", TRUE },
	{ &opts.enable_video,				CONTROL_CHECKBOX,	IDC_ENABLE_VIDEO,				"EnableVideo", TRUE },
	{ &opts.enable_others,				CONTROL_CHECKBOX,	IDC_ENABLE_OTHERS,				"EnableOthers", TRUE },
	{ &opts.xstatus_set,				CONTROL_RADIO,		IDC_SET_XSTATUS,				"XStatusSet", 0, SET_XSTATUS },
	{ &opts.xstatus_set,				CONTROL_RADIO,		IDC_CHECK_XSTATUS,				"XStatusSet", 0, CHECK_XSTATUS },
	{ &opts.xstatus_set,				CONTROL_RADIO,		IDC_CHECK_XSTATUS_MUSIC,		"XStatusSet", 0, CHECK_XSTATUS_MUSIC },
	{ &opts.xstatus_set,				CONTROL_RADIO,		IDC_IGNORE_XSTATUS,				"XStatusSet", 0, IGNORE_XSTATUS },
	{ &opts.override_contact_template,	CONTROL_CHECKBOX,	IDC_OVERRIDE_CONTACTS_TEMPLATE,	"OverrideContactsTemplate", FALSE},
	{ &opts.show_adv_icon,				CONTROL_CHECKBOX,	IDC_SHOW_ADV_ICON,				"ShowAdvancedIcon", FALSE},
	{ &opts.adv_icon_slot,				CONTROL_COMBO,		IDC_ADV_ICON,					"AdvancedIconSlot", 1}
};

static OptPageControl formatControls[] = { 
	{ &opts.templ,					CONTROL_TEXT,		IDC_TEMPLATE,			"Template", (DWORD) _T("%artist% - %title%") },
	{ &opts.unknown,				CONTROL_TEXT,		IDC_UNKNOWN,			"Unknown", (DWORD) LPGENT("<Unknown>"), 0, 0, 128 },
	{ &opts.xstatus_name,			CONTROL_TEXT,		IDC_XSTATUS_NAME,		"XStatusName", (DWORD) LPGENT("Listening to") },
	{ &opts.xstatus_message,		CONTROL_TEXT,		IDC_XSTATUS_MESSAGE,	"XStatusMessage", (DWORD) _T("%listening%") },
	{ &opts.nothing,				CONTROL_TEXT,		IDC_NOTHING,			"Nothing", (DWORD) LPGENT("<Nothing is playing now>"), 0, 0, 128 }
};

static OptPageControl playersControls[] = { 
	{ NULL,							CONTROL_CHECKBOX,	IDC_WATRACK,		"GetInfoFromWATrack", FALSE },
	{ &opts.time_to_pool,			CONTROL_SPIN,		IDC_POLL_TIMER,		"TimeToPool", (WORD) 5, IDC_POLL_TIMER_SPIN, (WORD) 1, (WORD) 255 },
	{ NULL,							CONTROL_CHECKBOX,	IDC_WINAMP,			"EnableWinamp", TRUE },
	{ NULL,							CONTROL_CHECKBOX,	IDC_ITUNES,			"EnableITunes", TRUE },
	{ NULL,							CONTROL_CHECKBOX,	IDC_WMP,			"EnableWMP", TRUE },
	{ NULL,							CONTROL_CHECKBOX,	IDC_FOOBAR,			"EnableFoobar", TRUE },
	{ NULL,							CONTROL_CHECKBOX,	IDC_MRADIO,			"EnableMRadio", TRUE },
	{ &opts.enable_other_players,	CONTROL_CHECKBOX,	IDC_OTHER,			"EnableOtherPlayers", TRUE },
	{ &opts.enable_code_injection,	CONTROL_CHECKBOX,	IDC_CODE_INJECTION,	"EnableCodeInjection", TRUE }
};

// Functions //////////////////////////////////////////////////////////////////////////////////////

int InitOptionsCallback(WPARAM wParam,LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = hInst;
	odp.flags = ODPF_BOLDGROUPS;

	odp.pszGroup = LPGEN("Status");
	odp.pszTitle = LPGEN("Listening info");
	odp.pszTab = LPGEN("General");
	odp.pfnDlgProc = OptionsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Format");
	odp.pfnDlgProc = FormatDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_FORMAT);
	odp.flags = ODPF_BOLDGROUPS;
	Options_AddPage(wParam, &odp);

	odp.pszTab = LPGEN("Players");
	odp.pfnDlgProc = PlayersDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_PLAYERS);
	Options_AddPage(wParam, &odp);
	return 0;
}


void InitOptions()
{
	playersControls[0].var = &players[WATRACK]->enabled;
	playersControls[2].var = &players[WINAMP]->enabled;
	playersControls[3].var = &players[ITUNES]->enabled;
	playersControls[4].var = &players[WMP]->enabled;
	playersControls[5].var = &players[FOOBAR]->enabled;
	playersControls[6].var = &players[MRADIO]->enabled;

	LoadOptions();

	hOptHook = HookEvent(ME_OPT_INITIALISE, InitOptionsCallback);
}


void DeInitOptions()
{
	UnhookEvent(hOptHook);
}


void LoadOptions()
{
	LoadOpts(optionsControls, SIZEOF(optionsControls), MODULE_NAME);
	LoadOpts(formatControls, SIZEOF(formatControls), MODULE_NAME);
	LoadOpts(playersControls, SIZEOF(playersControls), MODULE_NAME);
}


BOOL IsTypeEnabled(LISTENINGTOINFO *lti)
{
	if (lti == NULL)
		return TRUE;


	if (lti->dwFlags & LTI_UNICODE) {
		if (mir_tstrcmpi(lti->ptszType, LPGENT("Music")) == 0)
			return opts.enable_music;
		if (mir_tstrcmpi(lti->ptszType, LPGENT("Radio")) == 0)
			return opts.enable_radio;
		if (mir_tstrcmpi(lti->ptszType, LPGENT("Video")) == 0)
			return opts.enable_video;
		return opts.enable_others;
	}
	else

	{
		if (strcmpi(lti->pszType, "Music") == 0)
			return opts.enable_music;
		if (strcmpi(lti->pszType, "Radio") == 0)
			return opts.enable_radio;
		if (strcmpi(lti->pszType, "Video") == 0)
			return opts.enable_video;
		return opts.enable_others;
	}
}


static void OptionsEnableDisableCtrls(HWND hwndDlg)
{
	BOOL enabled = IsDlgButtonChecked(hwndDlg, IDC_ENABLE_SEND);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_MUSIC), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_RADIO), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_VIDEO), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_OTHERS), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ENABLE_MENU), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_XSTATUS_L), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SET_XSTATUS), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_XSTATUS), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CHECK_XSTATUS_MUSIC), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_IGNORE_XSTATUS), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CONTACTS_G), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_OVERRIDE_CONTACTS_TEMPLATE), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_SHOW_ADV_ICON), enabled);
	EnableWindow(GetDlgItem(hwndDlg, IDC_ADV_ICON), enabled);
}


static INT_PTR CALLBACK OptionsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	BOOL ret;
	if (msg != WM_INITDIALOG)
		ret = SaveOptsDlgProc(optionsControls, SIZEOF(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

	switch (msg) {
	case WM_INITDIALOG:
		if (hExtraIcon != NULL) {
			ShowWindow(GetDlgItem(hwndDlg, IDC_SHOW_ADV_ICON), SW_HIDE);
			ShowWindow(GetDlgItem(hwndDlg, IDC_ADV_ICON), SW_HIDE);
		}
		else {
			// Init combo
			int total = EXTRA_ICON_COUNT, first = 0;

			SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) _T("1"));
			SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) _T("2"));

			if (total > 0) {
				TCHAR tmp[10];
				for (int i = first; i <= total; i++)
					SendDlgItemMessage(hwndDlg, IDC_ADV_ICON, CB_ADDSTRING, 0, (LPARAM) _itot(i - first + 3, tmp, 10));
			}
		}

		ret = SaveOptsDlgProc(optionsControls, SIZEOF(optionsControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
		OptionsEnableDisableCtrls(hwndDlg);
		break;

	case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY) {
				RebuildMenu();
				StartTimer();
			}

		}
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ENABLE_SEND:
			if (HIWORD(wParam) == BN_CLICKED)
				OptionsEnableDisableCtrls(hwndDlg);

			break;
		}
		break;
	}

	return ret;
}


int playerDlgs[] = {
	WINAMP, IDC_WINAMP, 
	WMP, IDC_WMP, 
	ITUNES, IDC_ITUNES,
	FOOBAR, IDC_FOOBAR,
	MRADIO, IDC_MRADIO
};


static void PlayersEnableDisableCtrls(HWND hwndDlg)
{
	BOOL watrack_found = ServiceExists(MS_WAT_GETMUSICINFO);
	EnableWindow(GetDlgItem(hwndDlg, IDC_WATRACK), watrack_found);

	BOOL enabled = !IsDlgButtonChecked(hwndDlg, IDC_WATRACK) || !watrack_found;
	EnableWindow(GetDlgItem(hwndDlg, IDC_PLAYERS_L), enabled);

	BOOL needPoll = FALSE;
	for (int i = 0; i < SIZEOF(playerDlgs); i += 2) {
		EnableWindow(GetDlgItem(hwndDlg, playerDlgs[i+1]), enabled);
		if (players[playerDlgs[i]]->needPoll && IsDlgButtonChecked(hwndDlg, playerDlgs[i+1]))
			needPoll = TRUE;
	}

	EnableWindow(GetDlgItem(hwndDlg, IDC_OTHER), enabled);

	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_L), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_SPIN), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_POLL_TIMER_S_L), enabled && needPoll);
	EnableWindow(GetDlgItem(hwndDlg, IDC_CODE_INJECTION), enabled);
}

static INT_PTR CALLBACK PlayersDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	BOOL ret = SaveOptsDlgProc(playersControls, SIZEOF(playersControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);

	switch (msg) {
	case WM_INITDIALOG:
		PlayersEnableDisableCtrls(hwndDlg);
		break;

	case WM_COMMAND:
		if (HIWORD(wParam) == BN_CLICKED)
			PlayersEnableDisableCtrls(hwndDlg);
		break;

	case WM_NOTIFY:
		{
			LPNMHDR lpnmhdr = (LPNMHDR)lParam;
			if (lpnmhdr->idFrom == 0 && lpnmhdr->code == PSN_APPLY) {
				EnableDisablePlayers();
				StartTimer();
			}
		}
		break;
	}

	return ret;
}

static INT_PTR CALLBACK FormatDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) 
{
	return SaveOptsDlgProc(formatControls, SIZEOF(formatControls), MODULE_NAME, hwndDlg, msg, wParam, lParam);
}


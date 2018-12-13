#include "stdafx.h"

Options options;

static int OptionsInitialize(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
	odp.position = 100000000;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
	odp.szTitle.w = LPGENW("Newstory");
	odp.flags = ODPF_BOLDGROUPS | ODPF_UNICODE;

	//	odp.szTab.w = LPGENW("General");
	//	odp.pfnDlgProc = 0;
	//	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_GENERAL);
	//	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Templates");
	odp.pfnDlgProc = OptTemplatesDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_TEMPLATES);
	g_plugin.addOptions(wParam, &odp);

	odp.szTab.w = LPGENW("Passwords (not ready yet)");
	odp.pfnDlgProc = OptPasswordsDlgProc;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_PASSWORDS);
	g_plugin.addOptions(wParam, &odp);
	return 0;
}

void InitOptions()
{
	HookEvent(ME_OPT_INITIALISE, OptionsInitialize);
}

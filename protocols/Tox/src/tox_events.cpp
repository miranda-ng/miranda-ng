#include "common.h"

int CToxProto::OnPreShutdown(WPARAM, LPARAM)
{
	UninitNetlib();

	return 0;
}

INT_PTR CToxProto::OnAccountManagerInit(WPARAM, LPARAM lParam)
{
	return (INT_PTR)CreateDialogParam(
		g_hInstance,
		MAKEINTRESOURCE(IDD_ACCOUNT_MANAGER),
		(HWND)lParam,
		CToxProto::MainOptionsProc,
		(LPARAM)this);
}

int CToxProto::OnOptionsInit(WPARAM wParam, LPARAM)
{
	char *title = mir_t2a(m_tszUserName);

	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.hInstance = g_hInstance;
	odp.pszTitle = title;
	odp.dwInitParam = (LPARAM)this;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszGroup = LPGEN("Network");

	odp.pszTab = LPGEN("Account");
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_MAIN);
	odp.pfnDlgProc = MainOptionsProc;
	Options_AddPage(wParam, &odp);

	mir_free(title);

	return 0;
}

int CToxProto::OnSettingsChanged(WPARAM hContact, LPARAM lParam)
{
	DBCONTACTWRITESETTING* dbcws = (DBCONTACTWRITESETTING*)lParam;
	if (hContact == NULL && !strcmp(dbcws->szModule, m_szModuleName))
	{
		if (!strcmp(dbcws->szSetting, "Nick") && dbcws->value.pszVal)
		{
			if (tox_set_name(tox, (uint8_t*)dbcws->value.pszVal, (uint16_t)strlen(dbcws->value.pszVal)))
			{
				SaveToxProfile();
			}
		}

		/*if (!strcmp(dbcws->szSetting, "StatusMsg") || !strcmp(dbcws->szSetting, "StatusNote"))
		{
		if (tox_set_status_message(tox, (uint8_t*)(char*)ptrA(mir_utf8encodeW(dbcws->value.ptszVal)), (uint16_t)_tcslen(dbcws->value.ptszVal)))
		{
		SaveToxData();
		}
		}*/
	}

	return 0;
}
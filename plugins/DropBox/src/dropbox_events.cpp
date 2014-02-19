#include "dropbox.h"

int CDropbox::OnModulesLoaded(WPARAM wParam, LPARAM lParam)
{
	NETLIBUSER nlu = { sizeof(nlu) };
	nlu.flags = NUF_INCOMING | NUF_OUTGOING | NUF_HTTPCONNS | NUF_TCHAR;
	nlu.szSettingsModule = MODULE;
	nlu.szSettingsModule = MODULE;
	nlu.ptszDescriptiveName = L"Dropbox";

	g_dropbox->hNetlibUser = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

	MCONTACT hContact = GetDefaultContact();
	if (!hContact)
	{
		hContact = (MCONTACT)CallService(MS_DB_CONTACT_ADD, 0, 0);
		if (!CallService(MS_PROTO_ADDTOCONTACT, hContact, (LPARAM)MODULE))
		{
			db_set_s(hContact, MODULE, "Nick", MODULE);
		}
	}

	return 0;
}

int CDropbox::OnOptionsInit(WPARAM wParam, LPARAM lParam)
{
	OPTIONSDIALOGPAGE odp = { sizeof(odp) };
	odp.position = 100000000;
	odp.hInstance = g_hInstance;
	odp.flags = ODPF_BOLDGROUPS;
	odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS_MAIN);
	odp.pszGroup = LPGEN("Network");
	odp.pszTitle = LPGEN("Dropbox");
	odp.pfnDlgProc = MainOptionsProc;

	//Options_AddPage(wParam, &odp);

	return 0;
}
#include "commonheaders.h"


INT_PTR __cdecl Service_IsContactSecured(WPARAM wParam, LPARAM lParam)
{
	return (isContactSecured((MCONTACT)wParam)&SECURED) || isContactPGP((MCONTACT)wParam) || isContactGPG((MCONTACT)wParam);
}


INT_PTR __cdecl Service_CreateIM(WPARAM wParam,LPARAM lParam)
{
	CallContactService((MCONTACT)wParam, PSS_MESSAGE, PREF_METANODB, (LPARAM)SIG_INIT);
	return 1;
}

INT_PTR __cdecl Service_DisableIM(WPARAM wParam,LPARAM lParam)
{
	CallContactService((MCONTACT)wParam, PSS_MESSAGE, PREF_METANODB, (LPARAM)SIG_DEIN);
	return 1;
}

INT_PTR __cdecl Service_Status(WPARAM wParam, LPARAM lParam)
{
	switch(--lParam) {
	case STATUS_DISABLED:
	case STATUS_ENABLED:
	case STATUS_ALWAYSTRY:
		pUinKey ptr = getUinKey((MCONTACT)wParam);
		if (ptr) {
			ptr->status=ptr->tstatus=(BYTE)lParam;
			if (ptr->status == STATUS_ENABLED)	db_unset(ptr->hContact, MODULENAME, "StatusID");
			else 				db_set_b(ptr->hContact, MODULENAME, "StatusID", ptr->status);
		}
		break;
	}

	return 1;
}

INT_PTR __cdecl Service_StatusDis(WPARAM wParam, LPARAM lParam)
{
	return Service_Status(wParam,STATUS_DISABLED+1);
}

INT_PTR __cdecl Service_StatusEna(WPARAM wParam, LPARAM lParam)
{
	return Service_Status(wParam,STATUS_ENABLED+1);
}

INT_PTR __cdecl Service_StatusTry(WPARAM wParam, LPARAM lParam)
{
	return Service_Status(wParam,STATUS_ALWAYSTRY+1);
}

INT_PTR __cdecl Service_PGPdelKey(WPARAM wParam, LPARAM lParam)
{
	if (bPGPloaded) {
		db_unset((MCONTACT)wParam, MODULENAME, "pgp");
		db_unset((MCONTACT)wParam, MODULENAME, "pgp_mode");
		db_unset((MCONTACT)wParam, MODULENAME, "pgp_abbr");
	}
	{
		pUinKey ptr = getUinKey((MCONTACT)wParam);
		cpp_delete_context(ptr->cntx); ptr->cntx=0;
	}
	ShowStatusIconNotify((MCONTACT)wParam);
	return 1;
}

INT_PTR __cdecl Service_PGPsetKey(WPARAM wParam, LPARAM lParam)
{
	BOOL del = true;
	if (bPGPloaded) {
		if (bPGPkeyrings) {
			char szKeyID[128]; szKeyID[0]='\0';
			PVOID KeyID = pgp_select_keyid(GetForegroundWindow(),szKeyID);
			if (szKeyID[0]) {
				db_unset((MCONTACT)wParam, MODULENAME, "pgp");
				db_set_blob((MCONTACT)wParam, MODULENAME, "pgp", KeyID, pgp_size_keyid());
				db_set_b((MCONTACT)wParam, MODULENAME, "pgp_mode", 0);
				db_set_s((MCONTACT)wParam, MODULENAME, "pgp_abbr", szKeyID);
				del = false;
			}
		}
		else if (bPGPprivkey) {
			char KeyPath[MAX_PATH]; KeyPath[0]='\0';
			if (ShowSelectKeyDlg(0,KeyPath)) {
				char *publ = LoadKeys(KeyPath,false);
				if (publ) {
					db_unset((MCONTACT)wParam, MODULENAME, "pgp");
					db_set_s((MCONTACT)wParam, MODULENAME, "pgp", publ);
					db_set_b((MCONTACT)wParam, MODULENAME, "pgp_mode", 1);
					db_set_s((MCONTACT)wParam, MODULENAME, "pgp_abbr", "(binary)");
					mir_free(publ);
					del = false;
				}
			}
		}
	}

	if (del)
		Service_PGPdelKey(wParam,lParam);
	else {
		pUinKey ptr = getUinKey((MCONTACT)wParam);
		cpp_delete_context(ptr->cntx); ptr->cntx=0;
	}
	ShowStatusIconNotify((MCONTACT)wParam);
	return 1;
}

INT_PTR __cdecl Service_GPGdelKey(WPARAM wParam, LPARAM lParam)
{
	if (bGPGloaded)
		db_unset((MCONTACT)wParam, MODULENAME, "gpg");
	{
		pUinKey ptr = getUinKey((MCONTACT)wParam);
		cpp_delete_context(ptr->cntx); ptr->cntx=0;
	}
	ShowStatusIconNotify((MCONTACT)wParam);
	return 1;
}

INT_PTR __cdecl Service_GPGsetKey(WPARAM wParam, LPARAM lParam)
{
	bool del = true;
	if (bGPGloaded && bGPGkeyrings) {
		char szKeyID[128]; szKeyID[0]='\0';
		gpg_select_keyid(GetForegroundWindow(),szKeyID);
		if (szKeyID[0]) {
			db_set_s((MCONTACT)wParam, MODULENAME, "gpg", szKeyID);
			del = false;
		}
	}

	if (del) Service_GPGdelKey(wParam,lParam);
	else {
		pUinKey ptr = getUinKey((MCONTACT)wParam);
		cpp_delete_context(ptr->cntx); ptr->cntx=0;
	}
	ShowStatusIconNotify((MCONTACT)wParam);
	return 1;
}

INT_PTR __cdecl Service_Mode(WPARAM wParam, LPARAM lParam)
{
	pUinKey ptr = getUinKey((MCONTACT)wParam);

	switch(--lParam) {
	case MODE_NATIVE:
	case MODE_RSAAES:
		if (isContactSecured((MCONTACT)wParam)&SECURED) {
			msgbox(NULL, sim111, MODULENAME, MB_OK);
			return 0;
		}
		if (lParam != MODE_NATIVE && ptr->status>STATUS_ENABLED )
			Service_Status(wParam,STATUS_ENABLED+1);

	case MODE_PGP:
	case MODE_GPG:
		// нужно много проверок и отключение активного контекста если необходимо
		if (ptr) {
			if (ptr->cntx) {
				cpp_delete_context(ptr->cntx);
				ptr->cntx = 0;
				ptr->keyLoaded = 0;
			}
			ptr->mode=(BYTE)lParam;
			db_set_b((MCONTACT)wParam, MODULENAME, "mode", (BYTE)lParam);
		}
		ShowStatusIcon((MCONTACT)wParam);
		break;
	}

	return 1;
}

INT_PTR __cdecl Service_ModeNative(WPARAM wParam, LPARAM lParam)
{
	return Service_Mode(wParam, MODE_NATIVE+1);
}

INT_PTR __cdecl Service_ModePGP(WPARAM wParam, LPARAM lParam)
{
	return Service_Mode(wParam, MODE_PGP+1);
}

INT_PTR __cdecl Service_ModeGPG(WPARAM wParam, LPARAM lParam)
{
	return Service_Mode(wParam, MODE_GPG+1);
}
 
INT_PTR __cdecl Service_ModeRSAAES(WPARAM wParam, LPARAM lParam)
{
	return Service_Mode(wParam, MODE_RSAAES+1);
}


// EOF

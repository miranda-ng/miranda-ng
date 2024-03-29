/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "../stdafx.h"

/***********************************************************************************************************
 * internal functions
 ***********************************************************************************************************/

/**
 * name:	DisplayNameToFileName
 * desc:	convert contact's display name to valid filename
 * param:	hContact	- handle of contact to create the filename for
 *			pszFileName	- buffer, retrieving the converted filename
 *			cchFileName	- number of maximum characters the filename can be
 * return:	nothing
 **/

static void DisplayNameToFileName(lpExImParam ExImContact, wchar_t *pszFileName, size_t cchFileName)
{
	LPCSTR disp = nullptr;
	LPSTR	temp = nullptr;

	cchFileName--;
	pszFileName[0] = 0;

	switch (ExImContact->Typ) {
	case EXIM_ALL:
	case EXIM_GROUP:
		mir_wstrncpy(pszFileName, TranslateT("all Contacts"), cchFileName);
		return;

	case EXIM_CONTACT:
		if (ExImContact->hContact == NULL) {
			mir_wstrncpy(pszFileName, TranslateT("Owner"), cchFileName);
			return;
		}

		disp = temp = mir_u2a(Clist_GetContactDisplayName(ExImContact->hContact));
		break;

	case EXIM_SUBGROUP:
		temp = mir_u2a(ExImContact->ptszName);
		disp = temp;
		break;

	case EXIM_ACCOUNT:
		PROTOACCOUNT *acc = Proto_GetAccount(ExImContact->pszName);
		disp = temp = mir_u2a(acc->tszAccountName);
		break;
	}

	// replace unwanted characters
	while (*disp != 0 && cchFileName > 1) {
		switch (*disp) {
		case '?':	case '*':	case ':':
		case '\\':	case '|':	case '/':
		case '<':	case '>':	case '"':
			*(pszFileName++) = '_';
			break;
		default:
			*(pszFileName++) = *disp;
			break;
		}
		disp++;
		cchFileName--;
	}
	mir_free(temp);
	pszFileName[0] = 0;
}

static CMStringW FilterString(lpExImParam ExImContact)
{
	CMStringW str;

	switch (ExImContact->Typ) {
	case EXIM_SUBGROUP:
	case EXIM_ACCOUNT:
		str.Format(L"%s 1.0 (*.xml)\0*.xml\0", TranslateT("XMLCard"));
		__fallthrough;

	case EXIM_ALL:
	case EXIM_GROUP:
		str.AppendFormat(L"%s (*.ini)\0*.ini\0", TranslateT("DBEditor++ File"));
		__fallthrough;

	case EXIM_CONTACT:
		str.AppendFormat(L"%s 2.1 (*.vcf)\0*.vcf\0", TranslateT("Standard vCard"));
	}
	return str;
}

/**
 * name:	SvcExImport_Export
 * desc:	service function to export contact information
 * param:	wParam	- handle to contact or NULL
 *			lParam	- parent window
 * return:	0 always
 **/

INT_PTR SvcExImport_Export(lpExImParam ExImContact, HWND hwndParent)
{
	wchar_t szFileName[MAX_PATH]; szFileName[0] = 0;

	// create the filename to suggest the user for the to export contact
	DisplayNameToFileName(ExImContact, szFileName, _countof(szFileName));

	int nIndex = DlgExIm_SaveFileName(hwndParent, TranslateT("Select a destination file..."), FilterString(ExImContact), szFileName);
	switch (nIndex) {
	case 1: // .xml
		{
			CFileXml xmlFile;
			return xmlFile.Export(ExImContact, szFileName);
		}
	case 2: // .ini
		return SvcExImINI_Export(ExImContact, szFileName);

	case 3:		// .vcf
		CVCardFileVCF vcfFile;
		SetCursor(LoadCursor(nullptr, IDC_WAIT));
		if (vcfFile.Open(ExImContact->hContact, szFileName, L"wt")) {
			vcfFile.Export(FALSE);
			vcfFile.Close();
		}
		SetCursor(LoadCursor(nullptr, IDC_ARROW));
		return 0;
	}
	return 1;
}

/**
 * name:	SvcExImport_Import
 * desc:	service function to export contact information
 * param:	wParam	- handle to contact or NULL
 *			lParam	- parent window
 * return:	0 always
 **/

INT_PTR SvcExImport_Import(lpExImParam ExImContact, HWND hwndParent)
{
	wchar_t szFileName[MAX_PATH]; szFileName[0] = 0;

	// create the filename to suggest the user for the to export contact
	DisplayNameToFileName(ExImContact, szFileName, _countof(szFileName));

	int nIndex = DlgExIm_OpenFileName(hwndParent, TranslateT("Import User Details from VCard"), FilterString(ExImContact), szFileName);

	// Stop during develop
	if (ExImContact->Typ == EXIM_ACCOUNT ||
		ExImContact->Typ == EXIM_GROUP) return 1;

	switch (nIndex) {
	case 1:
		CFileXml().Import(ExImContact->hContact, szFileName);
		Clist_BroadcastAsync(CLM_AUTOREBUILD, 0, 0);
		return 0;

		// .ini
	case 2:
		return SvcExImINI_Import(ExImContact->hContact, szFileName);

		// .vcf
	case 3:
		CVCardFileVCF vcfFile;
		if (vcfFile.Open(ExImContact->hContact, szFileName, L"rt")) {
			SetCursor(LoadCursor(nullptr, IDC_WAIT));
			vcfFile.Import();
			vcfFile.Close();
			SetCursor(LoadCursor(nullptr, IDC_ARROW));
		}
		return 0;
	}
	return 1;
}

/***********************************************************************************************************
 * service functions
 ***********************************************************************************************************/

 /*********************************
  * Ex/import All (MainMenu)
  *********************************/

INT_PTR svcExIm_MainExport_Service(WPARAM, LPARAM lParam)
{
	ExImParam ExIm;
	memset(&ExIm, 0, sizeof(ExIm));
	ExIm.hContact = INVALID_CONTACT_ID;
	ExIm.Typ = EXIM_ALL;
	return SvcExImport_Export(&ExIm, (HWND)lParam);
}

INT_PTR svcExIm_MainImport_Service(WPARAM, LPARAM lParam)
{
	ExImParam ExIm;
	memset(&ExIm, 0, sizeof(ExIm));
	ExIm.hContact = INVALID_CONTACT_ID;
	ExIm.Typ = EXIM_ALL;
	return SvcExImport_Import(&ExIm, (HWND)lParam);
}

/*********************************
 * Ex/import Contact (ContactMenu)
 *********************************/

INT_PTR svcExIm_ContactExport_Service(WPARAM hContact, LPARAM lParam)
{
	ExImParam ExIm;
	memset(&ExIm, 0, sizeof(ExIm));
	ExIm.hContact = hContact;
	ExIm.Typ = EXIM_CONTACT;
	return SvcExImport_Export(&ExIm, (HWND)lParam);
}

INT_PTR svcExIm_ContactImport_Service(WPARAM hContact, LPARAM lParam)
{
	ExImParam ExIm;
	memset(&ExIm, 0, sizeof(ExIm));
	ExIm.hContact = hContact;
	ExIm.Typ = EXIM_CONTACT;
	return SvcExImport_Import(&ExIm, (HWND)lParam);
}

/*********************************
 *Ex/import (Sub)Group (GroupMenu)
 *********************************/

 /**
  * This service is call by (Sub)Group MenuItem Export and MenuItem Import
  *
  * @param	wParam				- gmp.wParam  = 0 ->Import
  * @param	wParam				- gmp.wParam != 0 ->Export
  * @param	lParam				- gmp.lParam not used
	*
  * @return	always 0
  **/

INT_PTR svcExIm_Group_Service(WPARAM wParam, LPARAM)
{
	ExImParam ExIm;
	INT_PTR hItem = 0, hRoot = 0, hParent = 0;
	wchar_t tszGroup[120], tszItem[120];
	memset(&tszGroup, 0, sizeof(tszGroup));
	memset(&tszItem, 0, sizeof(tszItem));
	memset(&ExIm, 0, sizeof(ExIm));
	LPTSTR ptszGroup = tszGroup;
	LPTSTR ptszItem = tszItem;

	HWND hClist = g_clistApi.hwndContactTree;
	// get clist selection
	hItem = SendMessage(hClist, CLM_GETSELECTION, 0, 0);
	hRoot = SendMessage(hClist, CLM_GETNEXTITEM, (WPARAM)CLGN_ROOT, (LPARAM)hItem);
	while (hItem) {
		if (SendMessage(hClist, CLM_GETITEMTYPE, (WPARAM)hItem, 0) == CLCIT_GROUP) {
			SendMessage(hClist, CLM_GETITEMTEXT, (WPARAM)hItem, (LPARAM)ptszItem);
			LPTSTR temp = mir_wstrdup(ptszGroup);
			mir_snwprintf(tszGroup, L"%s%s%s", ptszItem, mir_wstrlen(temp) ? L"\\" : L"", temp);
			mir_free(temp);
		}
		hParent = SendMessage(hClist, CLM_GETNEXTITEM, (WPARAM)CLGN_PARENT, (LPARAM)hItem);
		hItem = (hParent != hRoot) ? hParent : 0;
	}
	ExIm.ptszName = ptszGroup;
	ExIm.Typ = EXIM_SUBGROUP;

	if (wParam) {
		//Export	"/ExportGroup"
		SvcExImport_Export(&ExIm, hClist);
	}
	else {
		//Import	"/ImportGroup"
		SvcExImport_Import(&ExIm, hClist);
	}

	return 0;
};

/*********************************
 *Ex/Import Account (AccountMenu)
 *********************************/

 // neeed for MO_MENUITEMGETOWNERDATA
 // taken from core clistmenus.ccp

struct StatusMenuExecParam
{
	char *proto;			//This is unique protoname
	int protoindex;
	int status;

	BOOL custom;
	char *svc;
	HANDLE hMenuItem;
};

/**
 * This service is call by Account MenuItem Export and MenuItem Import
 *
 * @param	wParam				- not used
 * @param	lParam				- MenuItem from MS_CLIST_ADDSTATUSMENUITEM
 *
 * @return	always 0
 **/

INT_PTR svcExIm_Account_Service(WPARAM, LPARAM lParam)
{
	ExImParam ExIm;
	memset(&ExIm, 0, sizeof(ExIm));
	HWND hClist = g_clistApi.hwndContactTree;
	StatusMenuExecParam *smep = (StatusMenuExecParam*)Menu_GetItemData((HGENMENU)lParam);
	ExIm.pszName = mir_strdup(smep->proto);
	ExIm.Typ = EXIM_ACCOUNT;

	if (strstr(smep->svc, "/ExportAccount")) {
		//Export	"/ExportAccount"
		SvcExImport_Export(&ExIm, hClist);
	}
	else {
		//Import	"/ImportAccount"
		SvcExImport_Import(&ExIm, hClist);
	}
	mir_free(ExIm.pszName);
	return 0;
};

/**
 * name:	SvcExImport_LoadModule()
 * desc:	initializes the Ex/Import Services
 *
 * return:	0 or 1
 **/

void SvcExImport_LoadModule()
{
	CreateServiceFunction(MS_USERINFO_VCARD_EXPORTALL, svcExIm_MainExport_Service);
	CreateServiceFunction(MS_USERINFO_VCARD_IMPORTALL, svcExIm_MainImport_Service);
	CreateServiceFunction(MS_USERINFO_VCARD_EXPORT, svcExIm_ContactExport_Service);
	CreateServiceFunction(MS_USERINFO_VCARD_IMPORT, svcExIm_ContactImport_Service);
}

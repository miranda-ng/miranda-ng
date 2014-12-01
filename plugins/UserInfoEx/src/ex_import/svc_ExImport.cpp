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

#include "..\commonheaders.h"

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
static void DisplayNameToFileName(lpExImParam ExImContact, LPSTR pszFileName, WORD cchFileName)
{
	LPCSTR	disp = 0;
	LPSTR	temp = 0;

	cchFileName--;
	pszFileName[0] = 0;

	switch (ExImContact->Typ) {
		case EXIM_ALL:
		case EXIM_GROUP:
			mir_strncpy(pszFileName, Translate("all Contacts"), cchFileName);
			return;
		case EXIM_CONTACT:
			if (ExImContact->hContact == NULL) {
				mir_strncpy(pszFileName, Translate("Owner"), cchFileName);
				return;
			}
			else {
				disp = (LPCSTR)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ExImContact->hContact, NULL);
			}
			break;
		case EXIM_SUBGROUP:
			temp = mir_t2a(ExImContact->ptszName);
			disp = temp;
			break;
		case EXIM_ACCOUNT:
			PROTOACCOUNT* acc = ProtoGetAccount(ExImContact->pszName);
			temp = mir_t2a(acc->tszAccountName);
			disp = temp;
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

LPCSTR FilterString(lpExImParam ExImContact)
{
	LPCSTR pszFilter = 0;
	switch (ExImContact->Typ) {
		case EXIM_SUBGROUP:
		case EXIM_ACCOUNT:
			pszFilter = ("XMLCard 1.0 (*.xml)\0*.xml\0");
			break;
		case EXIM_ALL:
		case EXIM_GROUP:
			pszFilter = ("XMLCard 1.0 (*.xml)\0*.xml\0DBEditor++ File (*.ini)\0*.ini\0");
			break;
		case EXIM_CONTACT:
			pszFilter = ("XMLCard 1.0 (*.xml)\0*.xml\0DBEditor++ File (*.ini)\0*.ini\0Standard vCard 2.1 (*.vcf)\0*.vcf\0");
			break;
	}
	return pszFilter;
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
	CHAR szFileName[MAX_PATH] = { 0 };
	// create the filename to suggest the user for the to export contact
	DisplayNameToFileName(ExImContact, szFileName, SIZEOF(szFileName));
	int nIndex = DlgExIm_SaveFileName(hwndParent, 
		Translate("Select a destination file..."),
		FilterString(ExImContact),
		szFileName);

	switch (nIndex) {
		case 1:		// .xml
		{
			CFileXml xmlFile;
			return xmlFile.Export(ExImContact, szFileName);
		}
		case 2:		// .ini
		{
			return SvcExImINI_Export(ExImContact, szFileName);
		}
		case 3:		// .vcf
		{
			CVCardFileVCF vcfFile;
			SetCursor(LoadCursor(NULL, IDC_WAIT));
			if (vcfFile.Open(ExImContact->hContact, szFileName, "wt")) {
				vcfFile.Export(FALSE);
				vcfFile.Close();
			}
			SetCursor(LoadCursor(NULL, IDC_ARROW));
			return 0;
		}
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
	CHAR szFileName[MAX_PATH] = { 0 };

	// create the filename to suggest the user for the to export contact
	DisplayNameToFileName(ExImContact, szFileName, SIZEOF(szFileName));

	int nIndex = DlgExIm_OpenFileName(hwndParent, 
		Translate("Import User Details from VCard"),
		FilterString(ExImContact),
		szFileName);

// Stop during develop
if (ExImContact->Typ == EXIM_ACCOUNT || 
	ExImContact->Typ == EXIM_GROUP) return 1;

	switch (nIndex) {
		case 1:
		{
			CFileXml xmlFile;
			CallService(MS_CLIST_SETHIDEOFFLINE, -1, 0);	//workarround to refresh the clist....
			xmlFile.Import(ExImContact->hContact, szFileName);
			CallService(MS_CLIST_SETHIDEOFFLINE, -1, 0);	//...after import.
			//pcli->pfnClcBroadcast(CLM_AUTOREBUILD, 0, 0); //does not work
			return 0;
		}
		// .ini
		case 2:
			return SvcExImINI_Import(ExImContact->hContact, szFileName);

		// .vcf
		case 3:
		{
			CVCardFileVCF vcfFile;

			if (vcfFile.Open(ExImContact->hContact, szFileName, "rt")) {
				SetCursor(LoadCursor(NULL, IDC_WAIT));
				vcfFile.Import();
				vcfFile.Close();
				SetCursor(LoadCursor(NULL, IDC_ARROW));
			}
			return 0;
		}
	}
	return 1;
}

/***********************************************************************************************************
 * service functions
 ***********************************************************************************************************/

/*********************************
 * Ex/import All (MainMenu)
 *********************************/
INT_PTR svcExIm_MainExport_Service(WPARAM wParam, LPARAM lParam)
{
	ExImParam ExIm;
	memset(&ExIm, 0, sizeof(ExIm));
	ExIm.hContact = INVALID_CONTACT_ID;
	ExIm.Typ = EXIM_ALL;
	return SvcExImport_Export(&ExIm, (HWND)lParam);
}

INT_PTR svcExIm_MainImport_Service(WPARAM wParam, LPARAM lParam)
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
INT_PTR svcExIm_Group_Service(WPARAM wParam, LPARAM lParam)
{
	ExImParam ExIm;
	INT_PTR hItem = 0, hRoot = 0, hParent = 0;
	TCHAR tszGroup[120], tszItem[120];
	memset(&tszGroup, 0, sizeof(tszGroup));
	memset(&tszItem, 0, sizeof(tszItem));
	memset(&ExIm, 0, sizeof(ExIm));
	LPTSTR ptszGroup = tszGroup;
	LPTSTR ptszItem = tszItem;

	HWND hClist = (HWND)CallService(MS_CLUI_GETHWNDTREE,0,0);
	// get clist selection
	hItem = SendMessage(hClist,CLM_GETSELECTION,0,0);
	hRoot = SendMessage(hClist,CLM_GETNEXTITEM, (WPARAM)CLGN_ROOT, (LPARAM)hItem);
	while (hItem) {
		if (SendMessage(hClist,CLM_GETITEMTYPE, (WPARAM)hItem, 0) == CLCIT_GROUP) {
			SendMessage(hClist,CLM_GETITEMTEXT, (WPARAM)hItem, (LPARAM)ptszItem);
			LPTSTR temp = mir_tstrdup(ptszGroup);
			mir_sntprintf(tszGroup, SIZEOF(tszGroup),_T("%s%s%s"), ptszItem, _tcslen(temp)? _T("\\"):_T(""), temp);
			mir_free (temp);
		}
		hParent = SendMessage(hClist,CLM_GETNEXTITEM, (WPARAM)CLGN_PARENT, (LPARAM)hItem);
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
typedef struct
// neeed for MO_MENUITEMGETOWNERDATA
// taken from core clistmenus.ccp
{
	char *proto;			//This is unique protoname
	int protoindex;
	int status;

	BOOL custom;
	char *svc;
	HANDLE hMenuItem;
}	StatusMenuExecParam,*lpStatusMenuExecParam;

/**
 * This service is call by Account MenuItem Export and MenuItem Import
 *
 * @param	wParam				- not used
 * @param	lParam				- MenuItem from MS_CLIST_ADDSTATUSMENUITEM
 *
 * @return	always 0
 **/
INT_PTR svcExIm_Account_Service(WPARAM wParam, LPARAM lParam)
{
	ExImParam ExIm;
	memset(&ExIm, 0, sizeof(ExIm));
	HWND hClist = (HWND)CallService(MS_CLUI_GETHWNDTREE,0,0);
	lpStatusMenuExecParam smep = (lpStatusMenuExecParam) CallService(MO_MENUITEMGETOWNERDATA, (WPARAM) lParam, NULL);
	ExIm.pszName = mir_strdup(smep->proto);
	ExIm.Typ = EXIM_ACCOUNT;

	if (strstr( smep->svc, "/ExportAccount" )) {
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
	CreateServiceFunction(MS_USERINFO_VCARD_EXPORTALL,	svcExIm_MainExport_Service);
	CreateServiceFunction(MS_USERINFO_VCARD_IMPORTALL,	svcExIm_MainImport_Service);
	CreateServiceFunction(MS_USERINFO_VCARD_EXPORT,		svcExIm_ContactExport_Service);
	CreateServiceFunction(MS_USERINFO_VCARD_IMPORT,		svcExIm_ContactImport_Service);
	return;
}

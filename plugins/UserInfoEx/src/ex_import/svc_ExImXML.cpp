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

#define XMLCARD_VERSION	"1.1"

INT_PTR CALLBACK DlgProc_DataHistory(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) 
	{
		case WM_INITDIALOG:
		{
			const ICONCTRL idIcon[] = {
				{ ICO_DLG_EXPORT,	WM_SETICON,		NULL		},
				{ ICO_DLG_EXPORT,	STM_SETIMAGE,	ICO_DLGLOGO	},
				{ ICO_BTN_EXPORT,	BM_SETIMAGE,	IDOK		},
				{ ICO_BTN_CANCEL,	BM_SETIMAGE,	IDCANCEL	}
			};
			const int numIconsToSet = db_get_b(NULL, MODNAME, SET_ICONS_BUTTONS, 1) ? SIZEOF(idIcon) : 2;
			IcoLib_SetCtrlIcons(hDlg, idIcon, numIconsToSet);

			TranslateDialogDefault(hDlg);
			SendDlgItemMessage(hDlg, IDOK, BUTTONTRANSLATE, NULL, NULL);
			SendDlgItemMessage(hDlg, IDCANCEL, BUTTONTRANSLATE, NULL, NULL);
			break;
		}
		case WM_CTLCOLORSTATIC:
			switch (GetWindowLongPtr((HWND)lParam, GWLP_ID)) {
				case STATIC_WHITERECT:
				case ICO_DLGLOGO:
				case IDC_INFO:
					SetBkColor((HDC)wParam, RGB(255, 255, 255));
					return (INT_PTR)GetStockObject(WHITE_BRUSH);
			}
			return FALSE;
		case WM_COMMAND:
			if (HIWORD(wParam) == BN_CLICKED) {
				switch (LOWORD(wParam)) {
					case IDCANCEL:
						EndDialog(hDlg, 0);
						break;
					case IDOK: {
						WORD hiWord = 0;

						if (IsDlgButtonChecked(hDlg, IDC_CHECK1))
							hiWord |= EXPORT_DATA;
						if (IsDlgButtonChecked(hDlg, IDC_CHECK2))
							hiWord |= EXPORT_HISTORY;
						EndDialog(hDlg, (INT_PTR)MAKELONG(IDOK, hiWord));
						break;
					}
				}
			}
			break;
	}
	return FALSE;
}

/***********************************************************************************************************
 * exporting stuff
 ***********************************************************************************************************/

/**
 * name:	Export
 * desc:	globally accessible function which does the whole export stuff.
 * params:	hContact	- handle to the contact who is to export
 *			pszFileName	- full qualified path to the xml file which is destination for the export process
 * return:	0 on success, 1 otherwise
 **/
int CFileXml::Export(lpExImParam ExImContact, LPCSTR pszFileName)
{
	FILE *xmlfile;
	DB::CEnumList Modules;
	LONG cbHeader;
	SYSTEMTIME now;
	DWORD result;
	MCONTACT hContact;

	result = (DWORD)DialogBox(ghInst, 
							MAKEINTRESOURCE(IDD_EXPORT_DATAHISTORY),
							NULL, DlgProc_DataHistory);
	if (LOWORD(result) != IDOK)
	{
		return 0;
	}
	_wExport = HIWORD(result);

	// show dialog to enable user to select modules for export
	if (!(_wExport & EXPORT_DATA) ||
		!DlgExImModules_SelectModulesToExport(ExImContact, &Modules, NULL)) 
	{

		xmlfile = fopen(pszFileName, "wt");
		if (!xmlfile)
		{
			MsgErr(NULL, LPGENT("Can't create xml file!\n%S"), pszFileName);
			return 1;
		}
		
		GetLocalTime(&now);

		// write xml header raw as it is without using the tinyxml api
		fprintf(xmlfile, 
			"%c%c%c<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
			"<XMLCard ver=\""XMLCARD_VERSION"\" ref=\"%04d-%02d-%02d %02d:%02d:%02d\">\n",
			0xefU, 0xbbU, 0xbfU, now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond
		);
		// remember the header's size
		cbHeader = ftell(xmlfile);

		CExImContactXML vContact(this);

		// write data
		if ( ExImContact->Typ == EXIM_CONTACT) {
			// export single contact
			_hContactToWorkOn = ExImContact->hContact;
			if (vContact.fromDB(ExImContact->hContact)) {
				vContact.Export(xmlfile, &Modules);
			}
		}
		else {
			// other export mode
			_hContactToWorkOn = INVALID_CONTACT_ID;
#ifdef _DEBUG
			LARGE_INTEGER freq, t1, t2;

			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&t1);
#endif
			// export owner contact
			if (ExImContact->Typ == EXIM_ALL && vContact.fromDB(NULL)) {
				vContact.Export(xmlfile, &Modules);
			}
			// loop for all other contact
			for (hContact = db_find_first(); hContact != NULL; hContact = db_find_next(hContact))
			{
				switch (ExImContact->Typ)
				{
					case EXIM_ALL:
					case EXIM_GROUP:
						// dont export meta subcontacts by default
						if (!db_mc_isSub(hContact)) {
							if (vContact.fromDB(hContact)) {
								vContact.Export(xmlfile, &Modules);
							}
						}
						break;
					case EXIM_SUBGROUP:
						// dont export meta subcontacts by default and
						// export only contact with selectet group name
						if (!db_mc_isSub(hContact) && 
							mir_tcsncmp(ExImContact->ptszName, DB::Setting::GetTString(hContact, "CList", "Group"), mir_tcslen(ExImContact->ptszName))== 0)
							{
							if (vContact.fromDB(hContact)) {
								vContact.Export(xmlfile, &Modules);
							}
						}
						break;
					case EXIM_ACCOUNT:
						// export only contact with selectet account name
						if (!mir_strncmp(ExImContact->pszName, DB::Contact::Proto(hContact), mir_strlen(ExImContact->pszName))) {
							if (vContact.fromDB(hContact)) {
								vContact.Export(xmlfile, &Modules);
							}
						}
						break;
				}
			} // *end for
#ifdef _DEBUG
			QueryPerformanceCounter(&t2);
			MsgErr(NULL, LPGENT("Export took %f ms"),
				(long double)(t2.QuadPart - t1.QuadPart) / freq.QuadPart * 1000.);
#endif
		}// *end other export mode

		// nothing exported?
		if (cbHeader == ftell(xmlfile)) {
			fclose(xmlfile);
			DeleteFileA(pszFileName);
			return 1;
		}
		fputs("</XMLCard>\n", xmlfile);
		fclose(xmlfile);
	}
	return 0;
}


/***********************************************************************************************************
 * importing stuff
 ***********************************************************************************************************/

CFileXml::CFileXml()
{
	_numContactsTodo		= 0;
	_numContactsDone		= 0;
	_numSettingsTodo		= 0;
	_numSettingsDone		= 0;
	_numEventsTodo			= 0;
	_numEventsDone			= 0;
	_numEventsDuplicated	= 0;
}

/**
 * name:	ImportOwner
 * desc:	Interpretes an xmlnode as owner contact, finds a corresponding contact in database
 *			or adds a new one including all xml childnodes.
 * params:	xContact	- xmlnode representing the contact
 *			stat		- structure used to collect some statistics
 * return:	ERROR_OK on success or one other element of ImportError to tell the type of failure
 **/
int CFileXml::ImportOwner(TiXmlElement* xContact)
{
	CExImContactXML vContact(this);

	if (vContact = xContact) {
		vContact.Import();
		return ERROR_OK;
	}
	return ERROR_NOT_ADDED;
}

/**
 * name:	ImportContacts
 * desc:	Parse all child nodes of an given parent node and try to import all found contacts
 * params:	xmlParent	 - xmlnode representing the parent of the list of contacts
 *			hContact	 - handle to the contact, who is the owner of the setting to import
 *			stat		 - structure used to collect some statistics
 * return:	ERROR_OK if at least one contact was successfully imported
 **/
int CFileXml::ImportContacts(TiXmlElement* xmlParent)
{
	TiXmlElement *xContact;
	CExImContactXML vContact(this);
	int result;
	LPTSTR pszNick;

	// import contacts
	for (xContact = xmlParent->FirstChildElement(); xContact != NULL; xContact = xContact->NextSiblingElement()) {
		if (!mir_stricmp(xContact->Value(), XKEY_CONTACT)) {
			// update progressbar and abort if user clicked cancel
			pszNick = mir_utf8decodeT(xContact->Attribute("nick"));
			// user clicked abort button
			if (_progress.UpdateContact(LPGENT("Contact: %s (%S)"), pszNick, xContact->Attribute("proto"))) {
				result = vContact.LoadXmlElemnt(xContact);
				switch (result) {
					case ERROR_OK:
						// init contact class and import if matches the user desires
						if (_hContactToWorkOn == INVALID_CONTACT_ID || vContact.handle() == _hContactToWorkOn) {
							result = vContact.Import(_hContactToWorkOn != INVALID_CONTACT_ID);
							switch (result) {
								case ERROR_OK:
									_numContactsDone++;
									break;
								case ERROR_ABORTED:
									if (pszNick) mir_free(pszNick);
									return ERROR_ABORTED;
#ifdef _DEBUG
								default:
									MsgErr(NULL, LPGENT("Importing %s caused error %d"), pszNick, result);
									break;
#endif
							}
						}
						break;
					case ERROR_ABORTED:
						if (pszNick) mir_free(pszNick);
						return ERROR_ABORTED;
#ifdef _DEBUG
					default:
						MsgErr(NULL, LPGENT("Loading contact %s from xml failed with error %d"), pszNick, result);
						break;
#endif
				}
			}
			if (pszNick) mir_free(pszNick);
		}
		// import owner contact
		else if (_hContactToWorkOn == INVALID_CONTACT_ID && !mir_stricmp(xContact->Value(), XKEY_OWNER) && (vContact = xContact)) {
			result = vContact.Import();
			switch (result) {
				case ERROR_OK:
					_numContactsDone++;
					break;
				case ERROR_ABORTED:
					return ERROR_ABORTED;
#ifdef _DEBUG
				default:
					MsgErr(NULL, LPGENT("Importing Owner caused error %d"), result);
#endif
			}
		}
	}
	return ERROR_OK;
}

/**
 * name:	CountContacts
 * desc:	Counts the number of contacts stored in the file
 * params:	xContact	- the contact, who is the owner of the keys to count
 * return:	nothing
 **/
DWORD CFileXml::CountContacts(TiXmlElement* xmlParent)
{
	DWORD dwCount = 0;
	TiXmlNode *xContact;

	try {
		// count contacts in file for progress bar
		for (xContact = xmlParent->FirstChild(); xContact != NULL; xContact = xContact->NextSibling()) {
			if (!mir_stricmp(xContact->Value(), XKEY_CONTACT) || !mir_stricmp(xContact->Value(), XKEY_OWNER)) {
				dwCount += CountContacts(xContact->ToElement()) + 1;
			}
		}
	}
	catch(...) {
		return 0;
	}
	return dwCount;
}

/**
 * name:	Import
 * desc:	Interpretes an xmlnode as owner contact, finds a corresponding contact in database
 *			or adds a new one including all xml childnodes.
 * params:	hContact	- handle to the contact, who is the owner of the setting to import
 *			pszFileName	- full qualified path to the xml file which is to import
 * return:	ERROR_OK on success or one other element of ImportError to tell the type of failure
 **/
int CFileXml::Import(MCONTACT hContact, LPCSTR pszFileName)
{
	TiXmlDocument doc;
	TiXmlElement *xmlCard = NULL;

	try {
		_hContactToWorkOn = hContact;
		// load xml file
		if (!doc.LoadFile(pszFileName)) {
			MsgErr(NULL, LPGENT("Parser is unable to load XMLCard \"%s\"\nError: %d\nDescription: %s"),
				pszFileName, doc.ErrorId(), doc.ErrorDesc());
			return 1;
		}
		// is xmlfile a XMLCard ?
		if ((xmlCard = doc.FirstChildElement("XMLCard")) == NULL) {
			MsgErr(NULL, LPGENT("The selected file is no valid XMLCard"));
			return 1;
		}
		// check version
		if (mir_strcmp(xmlCard->Attribute("ver"), XMLCARD_VERSION)) {
			MsgErr(NULL, LPGENT("The version of the XMLCard is not supported by UserInfoEx"));
			return 1;
		}

		// is owner contact to import ?
		if (_hContactToWorkOn == NULL) {
			int ret;
			
			// disable database safty mode to speed up the operation
			CallService(MS_DB_SETSAFETYMODE, 0, 0);
			// import owner contact
			ret = ImportOwner(xmlCard->FirstChildElement(XKEY_OWNER));
			// as soon as possible enable safty mode again!
			CallService(MS_DB_SETSAFETYMODE, 1, 0);

			if (!ret) {
				MsgBox(NULL, MB_ICONINFORMATION, 
					LPGENT("Complete"),
					LPGENT("Import complete"),
					LPGENT("Owner contact successfully imported."));
				return 0;
			} else {
				MsgErr(NULL, LPGENT("Selected XMLCard does not contain an owner contact!"));
				return 1;
			}
		}
		else {
#ifdef _DEBUG
			LARGE_INTEGER freq, t1, t2;

			QueryPerformanceFrequency(&freq);
			QueryPerformanceCounter(&t1);
#endif
			// count contacts in file for progress bar
			_numContactsTodo = CountContacts(xmlCard);
			if (_numContactsTodo > 0) {
				_progress.SetContactCount(_numContactsTodo);
				// disable database safty mode to speed up the operation
				CallService(MS_DB_SETSAFETYMODE, 0, 0);
				// import the contacts
				ImportContacts(xmlCard);
				// as soon as possible enable safty mode again!
				CallService(MS_DB_SETSAFETYMODE, 1, 0);
			}
			// finally hide the progress dialog
			_progress.Hide();

#ifdef _DEBUG
			QueryPerformanceCounter(&t2);
			MsgErr(NULL, LPGENT("Import took %f ms"),
				(long double)(t2.QuadPart - t1.QuadPart) / freq.QuadPart * 1000.);
#endif
			// show results
			MsgBox(NULL, MB_ICONINFORMATION, LPGENT("Import complete"), LPGENT("Some basic statistics"), 
				LPGENT("added contacts: %u / %u\nadded settings: %u / %u\nadded events %u / %u\nduplicated events: %u"),
				_numContactsDone, _numContactsTodo,
				_numSettingsDone, _numSettingsTodo,
				_numEventsDone, _numEventsTodo,
				_numEventsDuplicated);
			
		}
	}	
	catch(...) {
		MsgErr(NULL, LPGENT("FATAL: An exception was thrown while importing contacts from xmlCard!"));
		return 1;
	}
	return 0;
}

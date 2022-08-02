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

#define XMLCARD_VERSION	"1.1"

INT_PTR CALLBACK DlgProc_DataHistory(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		TranslateDialogDefault(hDlg);
		{
			const ICONCTRL idIcon[] = {
				{ IDI_EXPORT,     WM_SETICON,   NULL        },
				{ IDI_EXPORT,     STM_SETIMAGE, ICO_DLGLOGO },
				{ IDI_EXPORT,     BM_SETIMAGE,  IDOK        },
				{ IDI_BTN_CLOSE,	BM_SETIMAGE,  IDCANCEL    }
			};
			const int numIconsToSet = g_plugin.bButtonIcons ? _countof(idIcon) : 2;
			IcoLib_SetCtrlIcons(hDlg, idIcon, numIconsToSet);

			SendDlgItemMessage(hDlg, IDOK, BUTTONTRANSLATE, NULL, NULL);
			SendDlgItemMessage(hDlg, IDCANCEL, BUTTONTRANSLATE, NULL, NULL);
		}
		break;

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

			case IDOK:
				uint16_t hiWord = 0;
				if (IsDlgButtonChecked(hDlg, IDC_CHECK1))
					hiWord |= EXPORT_DATA;
				if (IsDlgButtonChecked(hDlg, IDC_CHECK2))
					hiWord |= EXPORT_HISTORY;
				EndDialog(hDlg, (INT_PTR)MAKELONG(IDOK, hiWord));
				break;
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

int CFileXml::Export(lpExImParam ExImContact, const wchar_t *pszFileName)
{
	DB::CEnumList Modules;

	uint32_t result = (uint32_t)DialogBox(g_plugin.getInst(), MAKEINTRESOURCE(IDD_EXPORT_DATAHISTORY), nullptr, DlgProc_DataHistory);
	if (LOWORD(result) != IDOK)
		return 0;

	_wExport = HIWORD(result);

	// show dialog to enable user to select modules for export
	if (!(_wExport & EXPORT_DATA) ||
		!DlgExImModules_SelectModulesToExport(ExImContact, &Modules, nullptr)) {

		FILE *xmlfile = _wfopen(pszFileName, L"wt");
		if (!xmlfile) {
			MsgErr(nullptr, LPGENW("Can't create xml file!\n%S"), pszFileName);
			return 1;
		}

		SYSTEMTIME now;
		GetLocalTime(&now);

		// write xml header raw as it is without using the tinyxml api
		fprintf(xmlfile,
			"%c%c%c<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n"
			"<XMLCard ver=\"" XMLCARD_VERSION "\" ref=\"%04d-%02d-%02d %02d:%02d:%02d\">\n",
			0xefU, 0xbbU, 0xbfU, now.wYear, now.wMonth, now.wDay, now.wHour, now.wMinute, now.wSecond
		);
		// remember the header's size
		LONG cbHeader = ftell(xmlfile);

		CExImContactXML vContact(this);

		// write data
		if (ExImContact->Typ == EXIM_CONTACT) {
			// export single contact
			_hContactToWorkOn = ExImContact->hContact;
			if (vContact.fromDB(ExImContact->hContact))
				vContact.Export(xmlfile, &Modules);
		}
		else {
			// other export mode
			_hContactToWorkOn = INVALID_CONTACT_ID;

			// export owner contact
			if (ExImContact->Typ == EXIM_ALL && vContact.fromDB(NULL))
				vContact.Export(xmlfile, &Modules);

			// loop for all other contact
			for (auto &hContact: Contacts()) {
				switch (ExImContact->Typ) {
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
						mir_wstrncmp(ExImContact->ptszName, DB::Setting::GetWString(hContact, "CList", "Group"), mir_wstrlen(ExImContact->ptszName)) == 0) {
						if (vContact.fromDB(hContact)) {
							vContact.Export(xmlfile, &Modules);
						}
					}
					break;
				case EXIM_ACCOUNT:
					// export only contact with selectet account name
					if (!mir_strncmp(ExImContact->pszName, Proto_GetBaseAccountName(hContact), mir_strlen(ExImContact->pszName))) {
						if (vContact.fromDB(hContact)) {
							vContact.Export(xmlfile, &Modules);
						}
					}
					break;
				}
			}
		}
		// *end other export mode

		// nothing exported?
		if (cbHeader == ftell(xmlfile)) {
			fclose(xmlfile);
			DeleteFileW(pszFileName);
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
	_numContactsTodo = 0;
	_numContactsDone = 0;
	_numSettingsTodo = 0;
	_numSettingsDone = 0;
	_numEventsTodo = 0;
	_numEventsDone = 0;
	_numEventsDuplicated = 0;
	_hContactToWorkOn = INVALID_CONTACT_ID;
	_wExport = 0;
}

/**
 * name:	ImportOwner
 * desc:	Interpretes an xmlnode as owner contact, finds a corresponding contact in database
 *			or adds a new one including all xml childnodes.
 * params:	xContact	- xmlnode representing the contact
 *			stat		- structure used to collect some statistics
 * return:	ERROR_OK on success or one other element of ImportError to tell the type of failure
 **/

int CFileXml::ImportOwner(const TiXmlElement *xContact)
{
	CExImContactXML vContact(this);
	if (vContact.LoadXmlElement(xContact) == ERROR_OK) {
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

int CFileXml::ImportContacts(const TiXmlElement *xmlParent)
{
	CExImContactXML vContact(this);

	// import contacts
	for (auto *xContact : TiXmlEnum(xmlParent)) {
		if (!mir_strcmpi(xContact->Value(), XKEY_CONTACT)) {
			// update progressbar and abort if user clicked cancel
			LPTSTR pszNick = mir_utf8decodeW(xContact->Attribute("nick"));
			// user clicked abort button
			if (_progress.UpdateContact(LPGENW("Contact: %s (%S)"), pszNick, xContact->Attribute("proto"))) {
				int result = vContact.LoadXmlElement(xContact);
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
						}
					}
					break;
				case ERROR_ABORTED:
					if (pszNick) mir_free(pszNick);
					return ERROR_ABORTED;
				}
			}
			if (pszNick) mir_free(pszNick);
		}
		// import owner contact
		else if (_hContactToWorkOn == INVALID_CONTACT_ID && !mir_strcmpi(xContact->Value(), XKEY_OWNER) && vContact.LoadXmlElement(xContact) == ERROR_OK) {
			int result = vContact.Import();
			switch (result) {
			case ERROR_OK:
				_numContactsDone++;
				break;
			case ERROR_ABORTED:
				return ERROR_ABORTED;
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

uint32_t CFileXml::CountContacts(const TiXmlElement *xmlParent)
{
	uint32_t dwCount = 0;
	// count contacts in file for progress bar
	for (auto *xContact : TiXmlEnum(xmlParent))
		if (!mir_strcmpi(xContact->Name(), XKEY_CONTACT) || !mir_strcmpi(xContact->Name(), XKEY_OWNER))
			dwCount += CountContacts(xContact) + 1;

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

int CFileXml::Import(MCONTACT hContact, const wchar_t *pszFileName)
{
	_hContactToWorkOn = hContact;

	FILE *in = _wfopen(pszFileName, L"rb");
	if (in == nullptr) {
		MsgErr(nullptr, LPGENW("Parser is unable to load XMLCard \"%s\"\nError: %d\nDescription: %s"),
			pszFileName, 2, TranslateT("File not found"));
		return 1;
	}

	// load xml file
	TiXmlDocument doc;
	int errorCode = doc.LoadFile(in);
	fclose(in);
	if (errorCode) {
		MsgErr(nullptr, LPGENW("Parser is unable to load XMLCard \"%s\"\nError: %d\nDescription: %s"),
			pszFileName, errorCode, doc.Error());
		return 1;
	}
	// is xmlfile a XMLCard ?
	TiXmlElement *xmlCard = doc.FirstChildElement("XMLCard");
	if (xmlCard == nullptr) {
		MsgErr(nullptr, LPGENW("The selected file is no valid XMLCard"));
		return 1;
	}
	// check version
	if (mir_strcmp(xmlCard->Attribute("ver"), XMLCARD_VERSION)) {
		MsgErr(nullptr, LPGENW("The version of the XMLCard is not supported by UserInfoEx"));
		return 1;
	}

	// is owner contact to import ?
	if (_hContactToWorkOn == NULL) {
		int ret;

		// disable database safty mode to speed up the operation
		db_set_safety_mode(0);
		// import owner contact
		ret = ImportOwner(xmlCard->FirstChildElement(XKEY_OWNER));
		// as soon as possible enable safty mode again!
		db_set_safety_mode(1);

		if (!ret) {
			MsgBox(nullptr, MB_ICONINFORMATION,
				LPGENW("Complete"),
				LPGENW("Import complete"),
				LPGENW("Owner contact successfully imported."));
			return 0;
		}
		else {
			MsgErr(nullptr, LPGENW("Selected XMLCard does not contain an owner contact!"));
			return 1;
		}
	}
	else {
		// count contacts in file for progress bar
		_numContactsTodo = CountContacts(xmlCard);
		if (_numContactsTodo > 0) {
			_progress.SetContactCount(_numContactsTodo);
			// disable database safty mode to speed up the operation
			db_set_safety_mode(0);
			// import the contacts
			ImportContacts(xmlCard);
			// as soon as possible enable safty mode again!
			db_set_safety_mode(1);
		}
		// finally hide the progress dialog
		_progress.Hide();

		// show results
		MsgBox(nullptr, MB_ICONINFORMATION, LPGENW("Import complete"), LPGENW("Some basic statistics"),
			LPGENW("added contacts: %u / %u\nadded settings: %u / %u\nadded events %u / %u\nduplicated events: %u"),
			_numContactsDone, _numContactsTodo,
			_numSettingsDone, _numSettingsTodo,
			_numEventsDone, _numEventsTodo,
			_numEventsDuplicated);

	}
	return 0;
}

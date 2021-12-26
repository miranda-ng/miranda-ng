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
 * common stuff
 ***********************************************************************************************************/

/**
* name:	SortProc
* desc:	used for bsearch in CExImContactXML::IsContactInfo
* param:	item1	- item to compare
*			item2	- item to compare
* return:	the difference
**/

static int SortProc(const LPDWORD item1, const LPDWORD item2)
{
	return *item1 - *item2;
}

/**
* name:	CExImContactXML
* class:	CExImContactXML
* desc:	the constructor for the contact class
* param:	pXmlFile	- the owning xml file
* return:	nothing
**/

CExImContactXML::CExImContactXML(CFileXml *pXmlFile)
	: CExImContactBase()
{
	_xmlReader = nullptr;
	_pXmlFile = pXmlFile;
	_hEvent = NULL;
}

/**
* name:	IsContactInfo
* class:	CExImContactXML
* desc:	this function compares the given setting key to the list of known contact
*			information keys
* param:	pszKey	- the settings key to check
* return:	TRUE if pszKey is a valid contact information
**/

uint8_t CExImContactXML::IsContactInfo(LPCSTR pszKey)
{
	// This is a sorted list of all hashvalues of the contact information.
	// This is the same as the szCiKey[] array below but sorted
	const uint32_t dwCiHash[] = {
		0x6576F145,0x65780A70,0x6719120C,0x6776F145,0x67780A70,0x6EDB33D7,0x6F0466B5,
		0x739B6915,0x73B11E48,0x760D8AD5,0x786A70D0,0x8813C350,0x88641AF8,0x8ED5652D,
		0x96D64541,0x97768A14,0x9B786F9C,0x9B7889F9,0x9C26E6ED,0xA6675748,0xA813C350,
		0xA8641AF8,0xAC408FCC,0xAC40AFCC,0xAC40CFCC,0xAEC6EA4C,0xB813C350,0xB8641AF8,
		0xC5227954,0xCC68DE0E,0xCCD62E70,0xCCFBAAF4,0xCD715E13,0xD36182CF,0xD361C2CF,
		0xD361E2CF,0xD42638DE,0xD4263956,0xD426395E,0xD453466E,0xD778D233,0xDB59D87A,
		0xE406F60E,0xE406FA0E,0xE406FA4E,0xECF7E910,0xEF660441,0x00331041,0x0039AB3A,
		0x003D88A6,0x07ABA803,0x113D8227,0x113DC227,0x113DE227,0x2288784F,0x238643D6,
		0x2671C03E,0x275F720B,0x2EBDC0D6,0x3075C8C5,0x32674C9F,0x33EEAE73,0x40239C1C,
		0x44DB75D0,0x44FA69D0,0x4C76989B,0x4FF38979,0x544B2F44,0x55AFAF8C,0x567A6BC5,
		0x5A96C47F,0x6376F145,0x63780A70
	};
	if (pszKey && *pszKey) {
		char buf[MAXSETTING];
		// convert to hash and make bsearch as it is much faster then working with strings
		const uint32_t dwHash = mir_hashstr(_strlwr(mir_strncpy(buf, pszKey, _countof(buf))));
		return bsearch(&dwHash, dwCiHash, _countof(dwCiHash), sizeof(dwCiHash[0]), (int(*)(const void *, const void *))SortProc) != nullptr;
	}
	return FALSE;
}

/***********************************************************************************************************
 * exporting stuff
 ***********************************************************************************************************/

/**
* name:	CreateXmlNode
* class:	CExImContactXML
* desc:	creates a _xmlDoc.NewElement representing the contact
*			whose information are stored in this class
* param:	none
* return:	pointer to the newly created TiXmlElement
**/

TiXmlElement *CExImContactXML::CreateXmlElement()
{
	if (_hContact) {
		if (_pszProto) {
			_xmlDoc.InsertEndChild(_xmlWriter = _xmlDoc.NewElement(XKEY_CONTACT));

			LPSTR pszUID = uid2String(TRUE);
			_xmlWriter->SetAttribute("ampro", _pszAMPro);
			_xmlWriter->SetAttribute("proto", _pszProto);

			if (_pszDisp)  _xmlWriter->SetAttribute("disp", _pszDisp);
			if (_pszNick)  _xmlWriter->SetAttribute("nick", _pszNick);
			if (_pszGroup) _xmlWriter->SetAttribute("group", _pszGroup);

			if (pszUID) {
				if (_pszUIDKey) {
					_xmlWriter->SetAttribute("uidk", _pszUIDKey);
					_xmlWriter->SetAttribute("uidv", pszUID);
				}
				else {
					_xmlWriter->SetAttribute("uidk", "#NV");
					_xmlWriter->SetAttribute("uidv", "UNLOADED");
				}
				mir_free(pszUID);
			}
		}
		else _xmlWriter = nullptr;
	}
	else _xmlDoc.InsertEndChild(_xmlWriter = _xmlDoc.NewElement(XKEY_OWNER));

	return _xmlWriter;
}

/**
* name:	ExportContact
* class:	CExImContactXML
* desc:	exports a contact
* param:	none
* return:	ERROR_OK on success or any other on failure
**/

int CExImContactXML::ExportContact(DB::CEnumList *pModules)
{
	if (_pXmlFile->_wExport & EXPORT_DATA) {
		if (pModules) {
			for (auto &it : *pModules)
				ExportModule(it);
		}
		else {
			ExportModule(USERINFO);
			ExportModule(MOD_MBIRTHDAY);
		}
	}

	// export contact's events
	if (_pXmlFile->_wExport & EXPORT_HISTORY)
		ExportEvents();

	return ERROR_OK;
}

/**
* name:	ExportSubContact
* class:	CExImContactXML
* desc:	exports a meta sub contact
* param:	none
* return:	ERROR_OK on success or any other on failure
**/

int CExImContactXML::ExportSubContact(CExImContactXML *vMetaContact, DB::CEnumList *pModules)
{
	// create xmlNode
	if (!CreateXmlElement())
		return ERROR_INVALID_CONTACT;

	if (ExportContact(pModules) == ERROR_OK)
		if (!_xmlWriter->NoChildren() && vMetaContact->_xmlWriter->LinkEndChild(_xmlWriter))
			return ERROR_OK;

	return ERROR_NOT_ADDED;
}

/**
* name:	Export
* class:	CExImContactXML
* desc:	exports a contact
* param:	xmlfile		- handle to the open file to write the contact to
*			pModules	- list of modules to export for each contact
* return:	ERROR_OK on success or any other on failure
**/

int CExImContactXML::Export(FILE *xmlfile, DB::CEnumList *pModules)
{
	if (!xmlfile)
		return ERROR_INVALID_PARAMS;

	if (_hContact == INVALID_CONTACT_ID)
		return ERROR_INVALID_CONTACT;

	if (!CreateXmlElement())
		return ERROR_INVALID_CONTACT;

	// export meta
	if (isMeta()) {
		CExImContactXML vContact(_pXmlFile);

		const int cnt = db_mc_getSubCount(_hContact);
		const int def = db_mc_getDefaultNum(_hContact);
		MCONTACT hSubContact = db_mc_getSub(_hContact, def);

		// export default subcontact
		if (hSubContact && vContact.fromDB(hSubContact))
			vContact.ExportSubContact(this, pModules);

		for (int i = 0; i < cnt; i++) {
			if (i != def) {
				hSubContact = db_mc_getSub(_hContact, i);
				if (hSubContact && vContact.fromDB(hSubContact))
					vContact.ExportSubContact(this, pModules);
			}
		}
	}
	ExportContact(pModules);

	// add xContact to document
	if (_xmlWriter->NoChildren()) {
		_xmlWriter = nullptr;
		_xmlDoc.Clear();
		return ERROR_NOT_ADDED;
	}

	tinyxml2::XMLPrinter printer(xmlfile, false);
	_xmlDoc.Print(&printer);
	fputc('\n', xmlfile);

	_xmlWriter = nullptr;
	_xmlDoc.Clear();

	return ERROR_OK;
}

/**
* name:	ExportModule
* class:	CExImContactXML
* desc:	enumerates all settings of a database module and adds them to the xml tree
* params:	pszModule	- the module which is to export
* return:	ERROR_OK on success or any other on failure
**/

int CExImContactXML::ExportModule(LPCSTR pszModule)
{
	DB::CEnumList	Settings;
	if (!pszModule || !*pszModule)
		return ERROR_INVALID_PARAMS;

	if (!Settings.EnumSettings(_hContact, pszModule)) {
		TiXmlElement *xmod = _xmlDoc.NewElement(XKEY_MOD);
		if (!xmod)
			return ERROR_MEMORY_ALLOC;

		xmod->SetAttribute("key", pszModule);
		for (auto &it : Settings)
			ExportSetting(xmod, pszModule, it);

		if (!xmod->NoChildren() && _xmlWriter->LinkEndChild(xmod))
			return ERROR_OK;
	}

	return ERROR_EMPTY_MODULE;
}

/**
* name:	ExportSetting
* desc:	read a setting from database and add an xmlelement to contact node
* params:	xmlModule	- xml node to add the setting to
*			hContact	- handle of the contact whose event chain is to export
*			pszModule	- the module which is to export
*			pszSetting	- the setting which is to export
* return:	pointer to the added element
**/

int CExImContactXML::ExportSetting(TiXmlElement *xmlModule, LPCSTR pszModule, LPCSTR pszSetting)
{
	TiXmlElement *xmlEntry = nullptr;
	TiXmlText *xmlValue = nullptr;
	CHAR          buf[32];
	LPSTR         str = nullptr;

	DBVARIANT dbv;
	if (DB::Setting::GetAsIs(_hContact, pszModule, pszSetting, &dbv))
		return ERROR_INVALID_VALUE;

	switch (dbv.type) {
	case DBVT_BYTE:		//'b' bVal and cVal are valid
		buf[0] = 'b';
		_ultoa(dbv.bVal, buf + 1, 10);
		xmlValue = _xmlDoc.NewText(buf);
		break;
	case DBVT_WORD:		//'w' wVal and sVal are valid
		buf[0] = 'w';
		_ultoa(dbv.wVal, buf + 1, 10);
		xmlValue = _xmlDoc.NewText(buf);
		break;
	case DBVT_DWORD:	//'d' dVal and lVal are valid
		buf[0] = 'd';
		_ultoa(dbv.dVal, buf + 1, 10);
		xmlValue = _xmlDoc.NewText(buf);
		break;
	case DBVT_ASCIIZ:	//'s' pszVal is valid
		if (mir_IsEmptyA(dbv.pszVal)) break;
		DB::Variant::ConvertString(&dbv, DBVT_UTF8);
		if (str = (LPSTR)mir_alloc(mir_strlen(dbv.pszVal) + 2)) {
			str[0] = 's';
			mir_strcpy(&str[1], dbv.pszVal);
			xmlValue = _xmlDoc.NewText(str);
			mir_free(str);
		}
		break;
	case DBVT_UTF8:		//'u' pszVal is valid
		if (mir_IsEmptyA(dbv.pszVal)) break;
		if (str = (LPSTR)mir_alloc(mir_strlen(dbv.pszVal) + 2)) {
			str[0] = 'u';
			mir_strcpy(&str[1], dbv.pszVal);
			xmlValue = _xmlDoc.NewText(str);
			mir_free(str);
		}
		break;
	case DBVT_WCHAR:	//'u' pwszVal is valid
		if (mir_IsEmptyW(dbv.pwszVal)) break;
		DB::Variant::ConvertString(&dbv, DBVT_UTF8);
		if (str = (LPSTR)mir_alloc(mir_strlen(dbv.pszVal) + 2)) {
			str[0] = 'u';
			mir_strcpy(&str[1], dbv.pszVal);
			xmlValue = _xmlDoc.NewText(str);
			mir_free(str);
		}
		break;
	case DBVT_BLOB:		//'n' cpbVal and pbVal are valid
		{
			// new buffer for base64 encoded data
			INT_PTR baselen = mir_base64_encode_bufsize(dbv.cpbVal);
			str = (LPSTR)mir_alloc(baselen + 6);
			assert(str != nullptr);
			// encode data
			if (mir_base64_encodebuf(dbv.pbVal, dbv.cpbVal, str + 1, baselen)) {
				str[baselen + 1] = 0;
				str[0] = 'n';
				xmlValue = _xmlDoc.NewText(str);
			}
			mir_free(str);
		}
		break;

	case DBVT_DELETED:	//this setting just got deleted, no other values are valid
		break;

	default:
		; // nothing
	}
	db_free(&dbv);
	if (xmlValue) {
		xmlEntry = _xmlDoc.NewElement(XKEY_SET);
		if (xmlEntry) {
			xmlEntry->SetAttribute("key", pszSetting);
			if (xmlEntry->LinkEndChild(xmlValue) && xmlModule->LinkEndChild(xmlEntry))
				return ERROR_OK;
		}
	}
	return ERROR_MEMORY_ALLOC;
}

/**
* name:	ExportEvents
* desc:	adds the event chain for a given contact to the xml tree
* params:	xContact	- the xml node to add the events as childs to
*			hContact	- handle of the contact whose event chain is to export
* return:	TRUE on success, FALSE otherwise
**/

uint8_t CExImContactXML::ExportEvents()
{
	int dwNumEvents = db_event_count(_hContact);
	if (dwNumEvents == 0)
		return FALSE;

	int dwNumEventsAdded = 0;

	// read out all events for the current contact
	DB::ECPTR pCursor(DB::Events(_hContact));
	while (MEVENT hDbEvent = pCursor.FetchNext()) {
		DBEVENTINFO dbei = {};
		if (DB::Event::GetInfoWithData(hDbEvent, &dbei))
			continue;

		// encode data
		LPSTR pBase64Data = mir_base64_encode(dbei.pBlob, dbei.cbBlob);
		if (pBase64Data) {
			TiXmlElement *xmlEvent = _xmlDoc.NewElement("evt");
			if (xmlEvent) {
				xmlEvent->SetAttribute("type", dbei.eventType);
				xmlEvent->SetAttribute("time", (int)dbei.timestamp);
				xmlEvent->SetAttribute("flag", (int)dbei.flags);

				TiXmlText *xmlText = _xmlDoc.NewText(pBase64Data);
				xmlEvent->LinkEndChild(xmlText);

				// find module
				TiXmlElement *xmlModule = nullptr;
				for (auto *it : TiXmlEnum(xmlModule))
					if (!mir_strcmpi(it->Attribute("key"), dbei.szModule)) {
						xmlModule = (TiXmlElement *)it;
						break;
					}

				// create new module
				if (!xmlModule) {
					xmlModule = _xmlDoc.NewElement(XKEY_MOD);
					xmlModule->SetAttribute("key", dbei.szModule);
					_xmlWriter->InsertEndChild(xmlModule);
				}

				xmlModule->LinkEndChild(xmlEvent);
				dwNumEventsAdded++;
			}
		}
		MIR_FREE(dbei.pBlob);
	}

	return dwNumEventsAdded == dwNumEvents;
}

/***********************************************************************************************************
 * importing stuff
 ***********************************************************************************************************/

 /**
 * name:	CountKeys
 * desc:	Counts the number of events and settings stored for a contact
 * params:	xmlContact	- the contact, who is the owner of the keys to count
 * return:	nothing
 **/

void CExImContactXML::CountKeys(uint32_t &numSettings, uint32_t &numEvents)
{
	numSettings = numEvents = 0;

	for (auto *xmod : TiXmlFilter(_xmlReader, XKEY_MOD)) {
		for (auto *xkey : TiXmlEnum(xmod)) {
			if (!mir_strcmpi(xkey->Value(), XKEY_SET))
				numSettings++;
			else
				numEvents++;
		}
	}
}

/**
* name:	LoadXmlElemnt
* class:	CExImContactXML
* desc:	get contact information from XML-file
* param:	xContact	- TiXmlElement representing a contact
* return:	ERROR_OK if successful or any other error number otherwise
**/

int CExImContactXML::LoadXmlElement(const TiXmlElement *xContact)
{
	if (xContact == nullptr)
		return ERROR_INVALID_PARAMS;

	// delete last contact
	db_free(&_dbvUID);
	_hContact = INVALID_CONTACT_ID;

	_xmlReader = xContact;
	MIR_FREE(_pszAMPro); ampro(xContact->Attribute("ampro"));
	MIR_FREE(_pszNick);  nick(xContact->Attribute("nick"));
	MIR_FREE(_pszDisp);  disp(xContact->Attribute("disp"));
	MIR_FREE(_pszGroup); group(xContact->Attribute("group"));
	MIR_FREE(_pszProto);
	MIR_FREE(_pszProtoOld);
	MIR_FREE(_pszUIDKey);

	// is contact a metacontact
	if (_pszAMPro && !mir_strcmp(_pszAMPro, META_PROTO)) {
		proto(META_PROTO);

		// meta contact must be uniquelly identified by its subcontacts
		// the metaID may change during an export or import call
		for (auto *xSub : TiXmlFilter(xContact, XKEY_CONTACT)) {
			CExImContactXML vSub(_pXmlFile);
			if (vSub.LoadXmlElement(xSub) == ERROR_OK) {
				// identify metacontact by the first valid subcontact in xmlfile
				if (_hContact == INVALID_CONTACT_ID && vSub.handle() != INVALID_CONTACT_ID) {
					MCONTACT hMeta = db_mc_getMeta(vSub.handle());
					if (hMeta != NULL) {
						_hContact = hMeta;
						break;
					}
				}
			}
		}
		// if no handle was found, this is a new meta contact
		_isNewContact = _hContact == INVALID_CONTACT_ID;
	}
	// entry is a default contact
	else {
		proto(xContact->Attribute("proto"));
		uidk(xContact->Attribute("uidk"));
		if (!_pszProto) {
			// check if this is the owner contact
			if (mir_strcmpi(xContact->Value(), XKEY_OWNER))
				return ERROR_INVALID_PARAMS;
			_hContact = NULL;
			_xmlReader = xContact;
			return ERROR_OK;
		}

		if (_pszUIDKey && mir_strcmp("#NV", _pszUIDKey) != 0) {
			LPCSTR pUID = xContact->Attribute("uidv");

			if (pUID != nullptr) {
				switch (*(pUID++)) {
				case 'b':
					uid((uint8_t)atoi(pUID));
					break;
				case 'w':
					uid((uint16_t)atoi(pUID));
					break;
				case 'd':
					uid((uint32_t)_atoi64(pUID));
					break;
				case 's':
					// utf8 -> asci
					uida(pUID);
					break;
				case 'u':
					uidu(pUID);
					break;
				case 'n':
					size_t valLen;
					{
						uint8_t *pbVal = (uint8_t*)mir_base64_decode(pUID, &valLen);
						if (pbVal != nullptr)
							uidn(pbVal, (uint32_t)valLen);
					}
					break;
				default:
					uidu((LPCSTR)nullptr);
					break;
				}
			}
		}
		// finally try to find contact in contact list
		findHandle();
	}
	return ERROR_OK;
}

/**
* name:	ImportContact
* class:	CExImContactXML
* desc:	create the contact if neccessary and copy
*			all information from the xmlNode to database
* param:	none
* return:	ERROR_OK on success or any other error number otherwise
**/

int CExImContactXML::ImportContact()
{
	// create the contact if not yet exists
	if (toDB() != INVALID_CONTACT_ID) {
		_hEvent = NULL;

		// count settings and events and init progress dialog
		uint32_t numSettings, numEvents;
		CountKeys(numSettings, numEvents);
		_pXmlFile->_progress.SetSettingsCount(numSettings + numEvents);
		_pXmlFile->_numSettingsTodo += numSettings;
		_pXmlFile->_numEventsTodo += numEvents;

		// import all modules
		for (auto *xmod : TiXmlFilter(_xmlReader, XKEY_MOD)) {
			// import module
			if (ImportModule(xmod) == ERROR_ABORTED) {
				// ask to delete new incomplete contact
				if (_isNewContact && _hContact != NULL) {
					int result = MsgBox(nullptr, MB_YESNO | MB_ICONWARNING,
						LPGENW("Question"),
						LPGENW("Importing a new contact was aborted!"),
						LPGENW("You aborted import of a new contact.\nSome information may be missing for this contact.\n\nDo you want to delete the incomplete contact?"));
					if (result == IDYES) {
						db_delete_contact(_hContact);
						_hContact = INVALID_CONTACT_ID;
					}
				}
				return ERROR_ABORTED;
			}
		}
		return ERROR_OK;
	}
	return ERROR_NOT_ADDED;
}

/**
* name:	ImportNormalContact
* class:	CExImContactXML
* desc:	create the contact if neccessary and copy
*			all information from the xmlNode to database.
*			Remove contact from a metacontact if it is a subcontact
* param:	none
* return:	ERROR_OK on success or any other error number otherwise
**/

int CExImContactXML::ImportNormalContact()
{
	int err = ImportContact();

	// remove contact from a metacontact
	if (err == ERROR_OK && db_mc_getMeta(_hContact))
		db_mc_removeFromMeta(_hContact);

	return err;
}

/**
* name:	Import
* class:	CExImContactXML
* desc:	create the contact if neccessary and copy
*			all information from the xmlNode to database.
*			Remove contact from a metacontact if it is a subcontact
* param:	TRUE = keepMetaSubContact
* return:	ERROR_OK on success or any other error number otherwise
**/

int CExImContactXML::Import(uint8_t keepMetaSubContact)
{
	// xml contact contains subcontacts?
	const TiXmlElement *xContact = _xmlReader->FirstChildElement("CONTACT");
	if (xContact) {
		// contact is a metacontact and metacontacts plugin is installed?
		if (isMeta()) {
			// create object for first sub contact
			CExImContactXML vContact(_pXmlFile);

			// the contact does not yet exist
			if (_isNewContact) {
				// import default contact as normal contact and convert to meta contact
				if (vContact.LoadXmlElement(xContact) != ERROR_OK)
					return ERROR_CONVERT_METACONTACT;

				// import as normal contact
				int result = vContact.ImportContact();
				if (result != ERROR_OK)
					return result;

				// convert default subcontact to metacontact
				_hContact = db_mc_convertToMeta(vContact.handle());
				if (_hContact == NULL) {
					_hContact = INVALID_CONTACT_ID;
					return ERROR_CONVERT_METACONTACT;
				}

				_pXmlFile->_numContactsDone++;
				// do not load first meta contact twice
				xContact = xContact->NextSiblingElement("CONTACT");
			}
			// xml contact contains more than one subcontacts?
			if (xContact) {
				// load all subcontacts
				do {
					// update progressbar and abort if user clicked cancel
					int result = _pXmlFile->_progress.UpdateContact(L"Sub Contact: %s (%S)",
						ptrW(mir_utf8decodeW(xContact->Attribute("nick"))).get(), xContact->Attribute("proto"));

					// user clicked abort button
					if (!result)
						break;

					if (vContact.LoadXmlElement(xContact) == ERROR_OK) {
						if (vContact.ImportMetaSubContact(this) == ERROR_ABORTED)
							return ERROR_ABORTED;
						_pXmlFile->_numContactsDone++;
					}
				} while (xContact = xContact->NextSiblingElement("CONTACT"));
			}
			// load metacontact information (after subcontact for faster import)
			return ImportContact();
		}
		// import sub contacts as normal contacts
		return _pXmlFile->ImportContacts(_xmlReader);
	}

	// load contact information
	int result = ImportContact();
	if (result == ERROR_OK && !keepMetaSubContact)
		db_mc_removeFromMeta(_hContact);

	return result;
}

/**
* name:	ImportMetaSubContact
* class:	CExImContactXML
* desc:	create the contact if neccessary and copy
*			all information from the xmlNode to database.
*			Add this contact to an meta contact
* param:	pMetaContact	- the meta contact to add this one to
* return:
**/

int CExImContactXML::ImportMetaSubContact(CExImContactXML *pMetaContact)
{
	// abort here if contact was not imported correctly
	int err = ImportContact();
	if (err != ERROR_OK)
		return err;

	// check if contact is subcontact of the desired meta contact
	if (db_mc_getMeta(_hContact) != pMetaContact->handle()) {
		// add contact to the metacontact (this service returns TRUE if successful)	
		err = db_mc_addToMeta(_hContact, pMetaContact->handle());
		if (err == FALSE) {
			// ask to delete new contact
			if (_isNewContact && _hContact != NULL) {
				LPTSTR ptszNick = mir_utf8decodeW(_pszNick);
				LPTSTR ptszMetaNick = mir_utf8decodeW(pMetaContact->_pszNick);
				int result = MsgBox(nullptr, MB_YESNO | MB_ICONWARNING,
					LPGENW("Question"),
					LPGENW("Importing a new meta subcontact failed!"),
					LPGENW("The newly created meta subcontact '%s'\ncould not be added to metacontact '%s'!\n\nDo you want to delete this contact?"),
					ptszNick, ptszMetaNick);
				MIR_FREE(ptszNick);
				MIR_FREE(ptszMetaNick);
				if (result == IDYES) {
					db_delete_contact(_hContact);
					_hContact = INVALID_CONTACT_ID;
				}
			}
			return ERROR_ADDTO_METACONTACT;
		}
	}
	return ERROR_OK;
}

/**
* name:	ImportModule
* class:	CExImContactXML
* desc:	interprete an xmlnode as module and add the children to database.
* params:	hContact	- handle to the contact, who is the owner of the setting to import
*			xmlModule	- xmlnode representing the module
*			stat		- structure used to collect some statistics
* return:	ERROR_OK on success or one other element of ImportError to tell the type of failure
**/

int CExImContactXML::ImportModule(const TiXmlElement *xmlModule)
{
	// get module name
	LPCSTR pszModule = xmlModule->Attribute("key");
	if (!pszModule || !*pszModule)
		return ERROR_INVALID_PARAMS;

	// ignore Modul 'Protocol' as it would cause trouble
	if (!mir_strcmpi(pszModule, "Protocol"))
		return ERROR_OK;

	for (auto *xKey : TiXmlEnum(xmlModule)) {
		// import setting
		if (!mir_strcmpi(xKey->Value(), XKEY_SET)) {
			// check if the module to import is the contact's protocol module
			uint8_t isProtoModule = !mir_strcmpi(pszModule, _pszProto)/* || DB::Module::IsMeta(pszModule)*/;
			uint8_t isMetaModule = DB::Module::IsMeta(pszModule);

			// just ignore MetaModule on normal contact to avoid errors (only keys)
			if (!isProtoModule && isMetaModule)
				continue;

			// just ignore MetaModule on Meta to avoid errors (only import spetial keys)
			if (isProtoModule && isMetaModule) {
				if (!mir_strcmpi(xKey->Attribute("key"), "Nick") ||
					!mir_strcmpi(xKey->Attribute("key"), "TzName") ||
					!mir_strcmpi(xKey->Attribute("key"), "Timezone")) {
					if (ImportSetting(pszModule, xKey) == ERROR_OK)
						_pXmlFile->_numSettingsDone++;
				}
			}
			// just ignore some settings of protocol module to avoid errors (only keys)
			else if (isProtoModule && !isMetaModule) {
				if (!IsContactInfo(xKey->Attribute("key"))) {
					if (ImportSetting(pszModule, xKey) == ERROR_OK)
						_pXmlFile->_numSettingsDone++;
				}
			}
			// other module
			else if (ImportSetting(pszModule, xKey) == ERROR_OK) {
				_pXmlFile->_numSettingsDone++;
			}
			if (!_pXmlFile->_progress.UpdateSetting(LPGENW("Settings: %S"), pszModule))
				return ERROR_ABORTED;
		}
		// import event
		else if (!mir_strcmpi(xKey->Value(), XKEY_EVT)) {
			int error = ImportEvent(pszModule, xKey);
			switch (error) {
			case ERROR_OK:
				_pXmlFile->_numEventsDone++;
				break;
			case ERROR_DUPLICATED:
				_pXmlFile->_numEventsDuplicated++;
				break;
			}
			if (!_pXmlFile->_progress.UpdateSetting(LPGENW("Events: %S"), pszModule))
				return ERROR_ABORTED;
		}
	} //*end for
	return ERROR_OK;
}

/**
* name:	ImportSetting
* class:	CExImContactXML
* desc:	interprete an setting representing xmlnode and write the corresponding setting to database.
* params:	xmlModule	- xmlnode representing the module to write the setting to in the database
*			xmlEntry	- xmlnode representing the setting to import
* return:	ERROR_OK on success or one other element of ImportError to tell the type of failure
**/

int CExImContactXML::ImportSetting(LPCSTR pszModule, const TiXmlElement *xmlEntry)
{
	// validate parameter
	if (!xmlEntry || !pszModule || !*pszModule)
		return ERROR_INVALID_PARAMS;

	// validate value
	LPCSTR value = xmlEntry->GetText();
	if (!value)
		return ERROR_INVALID_VALUE;

	// convert data
	size_t baselen;
	DBVARIANT dbv = { 0 };

	switch (value[0]) {
	case 'b':			//'b' bVal and cVal are valid
		dbv.type = DBVT_BYTE;
		dbv.bVal = (uint8_t)atoi(value + 1);
		break;
	case 'w':			//'w' wVal and sVal are valid
		dbv.type = DBVT_WORD;
		dbv.wVal = (uint16_t)atoi(value + 1);
		break;
	case 'd':			//'d' dVal and lVal are valid
		dbv.type = DBVT_DWORD;
		dbv.dVal = (uint32_t)_atoi64(value + 1);
		break;
	case 's':			//'s' pszVal is valid
		dbv.type = DBVT_ASCIIZ;
		dbv.pszVal = (LPSTR)mir_utf8decodeA((LPSTR)(value + 1));
		break;
	case 'u':
		dbv.type = DBVT_UTF8;
		dbv.pszVal = (LPSTR)mir_strdup((LPSTR)(value + 1));
		break;
	case 'n':
		dbv.type = DBVT_BLOB;
		dbv.pbVal = (uint8_t*)mir_base64_decode(value + 1, &baselen);
		if (dbv.pbVal != nullptr)
			dbv.cpbVal = (uint16_t)baselen;
		else {
			mir_free(dbv.pbVal);
			return ERROR_NOT_ADDED;
		}
		break;
	default:
		return ERROR_INVALID_TYPE;
	}

	// write value to db
	if (db_set(_hContact, pszModule, xmlEntry->Attribute("key"), &dbv)) {
		//if (cws.value.pbVal>0)
		mir_free(dbv.pbVal);
		if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_UTF8) mir_free(dbv.pszVal);
		return ERROR_NOT_ADDED;
	}
	//if (dbv.pbVal>0)
	mir_free(dbv.pbVal);
	if (dbv.type == DBVT_ASCIIZ || dbv.type == DBVT_UTF8) mir_free(dbv.pszVal);
	return ERROR_OK;
}

/**
* name:	ImportEvent
* class:	CExImContactXML
* desc:	interprete an xmlnode and add the corresponding event to database.
* params:	hContact	- handle to the contact, who is the owner of the setting to import
*			xmlModule	- xmlnode representing the module to write the setting to in the database
*			xmlEvent	- xmlnode representing the event to import
* return:	ERROR_OK on success or one other element of ImportError to tell the type of failure
**/

int CExImContactXML::ImportEvent(LPCSTR pszModule, const TiXmlElement *xmlEvent)
{
	// dont import events from metacontact
	if (isMeta())
		return ERROR_DUPLICATED;

	if (!xmlEvent || !pszModule || !*pszModule)
		return ERROR_INVALID_PARAMS;

	if (_stricmp(xmlEvent->Value(), "evt"))
		return ERROR_NOT_ADDED;

	// timestamp must be valid
	DBEVENTINFO	dbei = {};
	dbei.timestamp = xmlEvent->IntAttribute("time");
	if (dbei.timestamp == 0)
		return ERROR_INVALID_TIMESTAMP;

	LPCSTR tmp = xmlEvent->GetText();
	if (!tmp || tmp[0] == 0)
		return ERROR_INVALID_VALUE;

	size_t baselen;
	mir_ptr<uint8_t> tmpVal((uint8_t*)mir_base64_decode(tmp, &baselen));
	if (tmpVal != NULL) {
		// event owning module
		dbei.pBlob = tmpVal;
		dbei.cbBlob = (uint16_t)baselen;
		dbei.szModule = (LPSTR)pszModule;
		dbei.eventType = xmlEvent->IntAttribute("type");
		dbei.flags = xmlEvent->IntAttribute("flag");
		if (dbei.flags == 0)
			dbei.flags = DBEF_READ;

		// search in new and existing contact for existing event to avoid duplicates
		if (DB::Event::Exists(_hContact, _hEvent, &dbei))
			return ERROR_DUPLICATED;

		if ((_hEvent = db_event_add(_hContact, &dbei)) != 0)
			return ERROR_OK;
	}

	return ERROR_NOT_ADDED;
}

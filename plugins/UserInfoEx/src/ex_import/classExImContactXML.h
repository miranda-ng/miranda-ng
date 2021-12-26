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

#ifndef _CLASS_EXIM_CONTACT_XML_INCLUDED_
#define _CLASS_EXIM_CONTACT_XML_INCLUDED_ 1

#define XKEY_MOD		"MOD"
#define XKEY_SET		"SET"
#define XKEY_EVT		"evt"
#define XKEY_CONTACT	"CONTACT"
#define XKEY_OWNER		"OWNER"

enum EError
{
	ERROR_OK = 0,
	ERROR_NOT_ADDED = 1,
	ERROR_INVALID_PARAMS = 2,
	ERROR_INVALID_VALUE = 3,
	ERROR_INVALID_TIMESTAMP = 4,
	ERROR_INVALID_TYPE = 5,
	ERROR_DUPLICATED = 6,
	ERROR_MEMORY_ALLOC = 7,
	ERROR_INVALID_CONTACT = 8,
	ERROR_INVALID_SIGNATURE = 9,
	ERROR_ABORTED = 10,
	ERROR_CONVERT_METACONTACT = 11,
	ERROR_ADDTO_METACONTACT = 12,
	ERROR_EMPTY_MODULE = 13
};

class CExImContactXML : public CExImContactBase
{
	CFileXml *_pXmlFile; // the xmlfile
	TiXmlDocument _xmlDoc; // whole document in memory
	union
	{
		TiXmlElement *_xmlWriter; // xml node being prepared for export
		const TiXmlElement *_xmlReader; // xml node being used for import
	};

	MEVENT _hEvent;

	uint8_t IsContactInfo(LPCSTR pszKey);

	// private importing methods
	int		ImportModule(const TiXmlElement *xmlModule);
	int		ImportSetting(LPCSTR pszModule, const TiXmlElement *xmlEntry);
	int		ImportEvent(LPCSTR pszModule, const TiXmlElement *xmlEvent);
	int		ImportContact();
	int		ImportNormalContact();
	int		ImportMetaSubContact(CExImContactXML *pMetaContact);
	void		CountKeys(uint32_t &numSettings, uint32_t &numEvents);

	// private exporting methods
	int		ExportModule(LPCSTR pszModule);
	int		ExportSetting(TiXmlElement *xmlModule, LPCSTR pszModule, LPCSTR pszSetting);
	uint8_t     ExportEvents();

	int		ExportContact(DB::CEnumList *pModules);
	int		ExportSubContact(CExImContactXML *vMetaContact, DB::CEnumList *pModules);

public:
	CExImContactXML(CFileXml *pXmlFile);

	// exporting stuff
	TiXmlElement *CreateXmlElement();
	int		Export(FILE *xmlfile, DB::CEnumList *pModules);

	// importing stuff
	int		LoadXmlElement(const TiXmlElement *xContact);
	int		Import(uint8_t keepMetaSubContact = FALSE);
};

#endif /* _CLASS_EXIM_CONTACT_XML_INCLUDED_ */

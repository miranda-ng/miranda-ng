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

enum EError {
	ERROR_OK					= 0,
	ERROR_NOT_ADDED				= 1,
	ERROR_INVALID_PARAMS		= 2,
	ERROR_INVALID_VALUE			= 3,
	ERROR_INVALID_TIMESTAMP		= 4,
	ERROR_INVALID_TYPE			= 5,
	ERROR_DUPLICATED			= 6,
	ERROR_MEMORY_ALLOC			= 7,
	ERROR_INVALID_CONTACT		= 8,
	ERROR_INVALID_SIGNATURE		= 9,
	ERROR_ABORTED				= 10,
	ERROR_CONVERT_METACONTACT	= 11,
	ERROR_ADDTO_METACONTACT		= 12,
	ERROR_EMPTY_MODULE			= 13
};

class CExImContactXML : public CExImContactBase {

	CFileXml      *_pXmlFile; // the xmlfile
	TiXmlElement  *_xmlNode;  // xmlnode with contact information
	MEVENT         _hEvent;

	BYTE IsContactInfo(LPCSTR pszKey);

	// private importing methods
	int		ImportModule(TiXmlNode* xmlModule);
	int		ImportSetting(LPCSTR pszModule, TiXmlElement *xmlEntry);
	int		ImportEvent(LPCSTR pszModule, TiXmlElement *xmlEvent);
	int		ImportContact();
	int		ImportNormalContact();
	int		ImportMetaSubContact(CExImContactXML * pMetaContact);
	void	CountKeys(DWORD &numSettings, DWORD &numEvents);

	// private exporting methods
	int		ExportModule(LPCSTR pszModule);
	int		ExportSetting(TiXmlElement *xmlModule, LPCSTR pszModule, LPCSTR pszSetting);
	BYTE	ExportEvents();

	int		ExportContact(DB::CEnumList* pModules);
	int		ExportSubContact(CExImContactXML *vMetaContact, DB::CEnumList* pModules);

public:
	CExImContactXML(CFileXml * pXmlFile);

	// exporting stuff
	TiXmlElement*	CreateXmlElement();
	int		Export(FILE *xmlfile, DB::CEnumList* pModules);

	// importing stuff
	int		LoadXmlElemnt(TiXmlElement *xContact);
	int		Import(BYTE keepMetaSubContact = FALSE);

	BYTE operator = (TiXmlElement* xmlContact)	{
		return LoadXmlElemnt(xmlContact) == ERROR_OK;	
	}
};

#endif /* _CLASS_EXIM_CONTACT_XML_INCLUDED_ */

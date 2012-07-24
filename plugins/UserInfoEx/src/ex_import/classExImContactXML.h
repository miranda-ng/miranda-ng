/*
UserinfoEx plugin for Miranda IM

Copyright:
� 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

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

===============================================================================

File name      : $HeadURL: https://userinfoex.googlecode.com/svn/trunk/ex_import/classExImContactXML.h $
Revision       : $Revision: 187 $
Last change on : $Date: 2010-09-08 16:05:54 +0400 (Ср, 08 сен 2010) $
Last change by : $Author: ing.u.horn $

===============================================================================
*/

#ifndef _CLASS_EXIM_CONTACT_XML_INCLUDED_
#define _CLASS_EXIM_CONTACT_XML_INCLUDED_ 1

#include "svc_ExImport.h"
#include "classExImContactBase.h"
#include "svc_ExImXML.h"
#include "dlg_ExImProgress.h"

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

	CFileXml*		_pXmlFile;		// the xmlfile
	TiXmlElement*	_xmlNode;		// xmlnode with contact information
	HANDLE			_hEvent;

	BOOLEAN IsContactInfo(LPCSTR pszKey);

	// private importing methods
	INT		ImportModule(TiXmlNode* xmlModule);
	INT		ImportSetting(LPCSTR pszModule, TiXmlElement *xmlEntry);
	INT		ImportEvent(LPCSTR pszModule, TiXmlElement *xmlEvent);
	INT		ImportContact();
	INT		ImportNormalContact();
	INT		ImportMetaSubContact(CExImContactXML * pMetaContact);
	VOID	CountKeys(DWORD &numSettings, DWORD &numEvents);

	// private exporting methods
	INT		ExportModule(LPCSTR pszModule);
	INT		ExportSetting(TiXmlElement *xmlModule, LPCSTR pszModule, LPCSTR pszSetting);
	BOOLEAN	ExportEvents();

	INT		ExportContact(DB::CEnumList* pModules);
	INT		ExportSubContact(CExImContactXML *vMetaContact, DB::CEnumList* pModules);

public:
	CExImContactXML(CFileXml * pXmlFile);

	// exporting stuff
	TiXmlElement*	CreateXmlElement();
	INT		Export(FILE *xmlfile, DB::CEnumList* pModules);

	// importing stuff
	INT		LoadXmlElemnt(TiXmlElement *xContact);
	INT		Import(BOOLEAN keepMetaSubContact = FALSE);

	BOOLEAN operator = (TiXmlElement* xmlContact)	{
		return LoadXmlElemnt(xmlContact) == ERROR_OK;	
	}
};

#endif /* _CLASS_EXIM_CONTACT_XML_INCLUDED_ */

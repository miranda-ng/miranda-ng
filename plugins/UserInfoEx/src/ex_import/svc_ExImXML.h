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

#ifndef _SVC_FILEXML_INCLUDED_
#define _SVC_FILEXML_INCLUDED_ 1

#define EXPORT_DATA		1
#define EXPORT_HISTORY	2
#define EXPORT_ALL		(EXPORT_DATA|EXPORT_HISTORY)

class CFileXml {
	friend class CExImContactXML;

	uint32_t	_numContactsTodo;
	uint32_t	_numContactsDone;
	uint32_t	_numSettingsTodo;
	uint32_t	_numSettingsDone;
	uint32_t	_numEventsTodo;
	uint32_t _numEventsDone;
	uint32_t _numEventsDuplicated;

	MCONTACT _hContactToWorkOn;	// contact to ex/import (NULL=owner|INVALID_HANDLE_VALUE=all|HADNLE=one user)

	uint16_t _wExport;

	CProgress _progress;

	int ImportOwner(const TiXmlElement* xmlContact);
	int ImportContacts(const TiXmlElement* xmlParent);
	
	uint32_t CountContacts(const TiXmlElement* xmlParent);

	/*
	int ExportOwner(FILE *xmlfile, uint8_t bExportEvents);
	int ExportContact(FILE *xmlfile, MCONTACT hContact, uint8_t bExportEvents, LPENUMLIST pModules);
	int ExportSubContact(TiXmlElement *xContact, MCONTACT hContact, uint8_t bExportEvents);
	*/

public:
	CFileXml();
	int Import(MCONTACT hContact, const wchar_t *pszFileName);
	int Export(lpExImParam ExImContact, const wchar_t *pszFileName);
};

#endif /* _SVC_FILEXML_INCLUDED_ */
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

	DWORD	_numContactsTodo;
	DWORD	_numContactsDone;
	DWORD	_numSettingsTodo;
	DWORD	_numSettingsDone;
	DWORD	_numEventsTodo;
	DWORD _numEventsDone;
	DWORD _numEventsDuplicated;

	MCONTACT _hContactToWorkOn;	// contact to ex/import (NULL=owner|INVALID_HANDLE_VALUE=all|HADNLE=one user)

	WORD _wExport;

	CProgress _progress;

	int ImportOwner(TiXmlElement* xmlContact);
	int ImportContacts(TiXmlElement* xmlParent);
	
	DWORD CountContacts(TiXmlElement* xmlParent);

	/*
	int ExportOwner(FILE *xmlfile, BYTE bExportEvents);
	int ExportContact(FILE *xmlfile, MCONTACT hContact, BYTE bExportEvents, LPENUMLIST pModules);
	int ExportSubContact(TiXmlElement *xContact, MCONTACT hContact, BYTE bExportEvents);
	*/

public:
	CFileXml();
	int Import(MCONTACT hContact, LPCSTR pszFileName);
	int Export(lpExImParam ExImContact, LPCSTR pszFileName);
};

#endif /* _SVC_FILEXML_INCLUDED_ */
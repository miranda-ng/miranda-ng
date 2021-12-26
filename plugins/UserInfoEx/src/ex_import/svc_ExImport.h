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

#ifndef _SVC_EXIMPORT_INCLUDED_
#define _SVC_EXIMPORT_INCLUDED_ 1

typedef struct
{
	uint8_t Typ;
	union {
		MCONTACT hContact;
		LPSTR pszName;
		LPTSTR ptszName;
	};
}
ExImParam, *lpExImParam;

enum ExImType
{
	EXIM_ALL = 1,
	EXIM_CONTACT = 2,
	EXIM_GROUP = 4,
	EXIM_SUBGROUP = 8,
	EXIM_ACCOUNT = 16
};

INT_PTR svcExIm_MainExport_Service(WPARAM wParam, LPARAM lParam);
INT_PTR svcExIm_MainImport_Service(WPARAM wParam, LPARAM lParam);
INT_PTR svcExIm_ContactExport_Service(WPARAM wParam, LPARAM lParam);
INT_PTR svcExIm_ContactImport_Service(WPARAM wParam, LPARAM lParam);
INT_PTR svcExIm_Group_Service(WPARAM wParam, LPARAM lParam);
INT_PTR svcExIm_Account_Service(WPARAM wParam, LPARAM lParam);

void SvcExImport_LoadModule();

#endif /* _SVC_EXIMPORT_INCLUDED_ */
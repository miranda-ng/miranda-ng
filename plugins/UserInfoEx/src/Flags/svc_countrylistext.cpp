/*
UserinfoEx plugin for Miranda IM

Copyright:
© 2006-2010 DeathAxe, Yasnovidyashii, Merlin, K. Romanov, Kreol

part of this code based on:
Miranda IM Country Flags Plugin Copyright ©2006-2007 H. Herkenrath

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

/************************* Services *******************************/

INT_PTR ServiceGetCountryByNumber(WPARAM wParam,LPARAM lParam)
{
	int i;
	UNREFERENCED_PARAMETER(lParam);
	for(i=0; i<SIZEOF(countries); ++i)
		if ((int)wParam==countries[i].id)
			return (INT_PTR)countries[i].szName;
	return NULL;
}

INT_PTR ServiceGetCountryList(WPARAM wParam,LPARAM lParam)
{
	if ((int*)wParam==NULL || (void*)lParam==NULL) return 1;
	*(int*)wParam=SIZEOF(countries);
	*(struct CountryListEntry**)lParam=countries;
	return 0;
}

/************************* Misc ***********************************/

VOID InitCountryListExt()
{
	/* hack to replace built-in country list */
	if (!myDestroyServiceFunction(MS_UTILS_GETCOUNTRYLIST))
		CreateServiceFunction(MS_UTILS_GETCOUNTRYLIST,ServiceGetCountryList);
	if (!myDestroyServiceFunction(MS_UTILS_GETCOUNTRYBYNUMBER))
		CreateServiceFunction(MS_UTILS_GETCOUNTRYBYNUMBER,ServiceGetCountryByNumber);
}

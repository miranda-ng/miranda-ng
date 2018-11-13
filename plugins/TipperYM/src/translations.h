/*
Copyright (C) 2006-2007 Scott Ellis
Copyright (C) 2007-2011 Jan Holub

This is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this file; see the file license.txt.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  
*/

#ifndef _TRANSLATIONS_INC
#define _TRANSLATIONS_INC

extern int iTransFuncsCount;
extern DBVTranslation *translations;

struct LISTTYPEDATAITEM
{
	int id;
	char *szValue;
};

void InitTranslations();
void DeinitTranslations();

wchar_t *TimestampToTimeDifference(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen);
wchar_t *EmptyXStatusToDefaultName(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen);
wchar_t *WordToStatusDesc(MCONTACT hContact, const char *szModuleName, const char *szSettingName, wchar_t *buff, int bufflen);

#endif

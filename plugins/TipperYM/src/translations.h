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

typedef struct {
  int id;
  char *szValue;
} LISTTYPEDATAITEM;

static LISTTYPEDATAITEM languages[] = {
  {0, LPGEN("None")},
  {55,LPGEN("Afrikaans")},
  {58,LPGEN("Albanian")},
  {1, LPGEN("Arabic")},
  {59,LPGEN("Armenian")},
  {68,LPGEN("Azerbaijani")},
  {72,LPGEN("Belorussian")},
  {2, LPGEN("Bhojpuri")},
  {56,LPGEN("Bosnian")},
  {3, LPGEN("Bulgarian")},
  {4, LPGEN("Burmese")},
  {5, LPGEN("Cantonese")},
  {6, LPGEN("Catalan")},
  {61,LPGEN("Chamorro")},
  {7, LPGEN("Chinese")},
  {8, LPGEN("Croatian")},
  {9, LPGEN("Czech")},
  {10,LPGEN("Danish")},
  {11,LPGEN("Dutch")},
  {12,LPGEN("English")},
  {13,LPGEN("Esperanto")},
  {14,LPGEN("Estonian")},
  {15,LPGEN("Farsi")},
  {16,LPGEN("Finnish")},
  {17,LPGEN("French")},
  {18,LPGEN("Gaelic")},
  {19,LPGEN("German")},
  {20,LPGEN("Greek")},
  {70,LPGEN("Gujarati")},
  {21,LPGEN("Hebrew")},
  {22,LPGEN("Hindi")},
  {23,LPGEN("Hungarian")},
  {24,LPGEN("Icelandic")},
  {25,LPGEN("Indonesian")},
  {26,LPGEN("Italian")},
  {27,LPGEN("Japanese")},
  {28,LPGEN("Khmer")},
  {29,LPGEN("Korean")},
  {69,LPGEN("Kurdish")},
  {30,LPGEN("Lao")},
  {31,LPGEN("Latvian")},
  {32,LPGEN("Lithuanian")},
  {65,LPGEN("Macedonian")},
  {33,LPGEN("Malay")},
  {63,LPGEN("Mandarin")},
  {62,LPGEN("Mongolian")},
  {34,LPGEN("Norwegian")},
  {57,LPGEN("Persian")},
  {35,LPGEN("Polish")},
  {36,LPGEN("Portuguese")},
  {60,LPGEN("Punjabi")},
  {37,LPGEN("Romanian")},
  {38,LPGEN("Russian")},
  {39,LPGEN("Serbo-Croatian")},
  {66,LPGEN("Sindhi")},
  {40,LPGEN("Slovak")},
  {41,LPGEN("Slovenian")},
  {42,LPGEN("Somali")},
  {43,LPGEN("Spanish")},
  {44,LPGEN("Swahili")},
  {45,LPGEN("Swedish")},
  {46,LPGEN("Tagalog")},
  {64,LPGEN("Taiwanese")},
  {71,LPGEN("Tamil")},
  {47,LPGEN("Tatar")},
  {48,LPGEN("Thai")},
  {49,LPGEN("Turkish")},
  {50,LPGEN("Ukrainian")},
  {51,LPGEN("Urdu")},
  {52,LPGEN("Vietnamese")},
  {67,LPGEN("Welsh")},
  {53,LPGEN("Yiddish")},
  {54,LPGEN("Yoruba")},
};

static char *days[7] = {
	LPGEN("Sunday"), LPGEN("Monday"), LPGEN("Tuesday"), LPGEN("Wednesday"), LPGEN("Thursday"), LPGEN("Friday"), LPGEN("Saturday")
};

static char *months[12] = {
	LPGEN("January"), LPGEN("February"), LPGEN("March"), LPGEN("April"), LPGEN("May"), LPGEN("June"),
	LPGEN("July"), LPGEN("August"), LPGEN("September"), LPGEN("October"), LPGEN("November"), LPGEN("December")
};

void InitTranslations();
void DeinitTranslations();

TCHAR *TimestampToTimeDifference(MCONTACT hContact, const char *szModuleName, const char *szSettingName, TCHAR *buff, int bufflen);
TCHAR *EmptyXStatusToDefaultName(MCONTACT hContact, const char *szModuleName, const char *szSettingName, TCHAR *buff, int bufflen);
TCHAR *WordToStatusDesc(MCONTACT hContact, const char *szModuleName, const char *szSettingName, TCHAR *buff, int bufflen);


#endif

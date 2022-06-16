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

#include "stdafx.h"

static IDSTRLIST TmplLanguages[] = {
	{ 0, LPGEN("Unspecified"), nullptr},
	{55, LPGEN("Afrikaans"), nullptr},
	{58, LPGEN("Albanian"), nullptr},
	{ 1, LPGEN("Arabic"), nullptr},
	{59, LPGEN("Armenian"), nullptr},
	{68, LPGEN("Azerbaijani"), nullptr},
	{72, LPGEN("Belorussian"), nullptr},
	{ 2, LPGEN("Bhojpuri"), nullptr},
	{56, LPGEN("Bosnian"), nullptr},
	{ 3, LPGEN("Bulgarian"), nullptr},
	{ 4, LPGEN("Burmese"), nullptr},
	{ 5, LPGEN("Cantonese"), nullptr},
	{ 6, LPGEN("Catalan"), nullptr},
	{61, LPGEN("Chamorro"), nullptr},
	{ 7, LPGEN("Chinese"), nullptr},
	{ 8, LPGEN("Croatian"), nullptr},
	{ 9, LPGEN("Czech"), nullptr},
	{10, LPGEN("Danish"), nullptr},
	{11, LPGEN("Dutch"), nullptr},
	{12, LPGEN("English"), nullptr},
	{13, LPGEN("Esperanto"), nullptr},
	{14, LPGEN("Estonian"), nullptr},
	{15, LPGEN("Farsi"), nullptr},
	{16, LPGEN("Finnish"), nullptr},
	{17, LPGEN("French"), nullptr},
	{18, LPGEN("Gaelic"), nullptr},
	{19, LPGEN("German"), nullptr},
	{20, LPGEN("Greek"), nullptr},
	{70, LPGEN("Gujarati"), nullptr},
	{21, LPGEN("Hebrew"), nullptr},
	{22, LPGEN("Hindi"), nullptr},
	{23, LPGEN("Hungarian"), nullptr},
	{24, LPGEN("Icelandic"), nullptr},
	{25, LPGEN("Indonesian"), nullptr},
	{26, LPGEN("Italian"), nullptr},
	{27, LPGEN("Japanese"), nullptr},
	{28, LPGEN("Khmer"), nullptr},
	{29, LPGEN("Korean"), nullptr},
	{69, LPGEN("Kurdish"), nullptr},
	{30, LPGEN("Lao"), nullptr},
	{31, LPGEN("Latvian"), nullptr},
	{32, LPGEN("Lithuanian"), nullptr},
	{65, LPGEN("Macedonian"), nullptr},
	{33, LPGEN("Malay"), nullptr},
	{63, LPGEN("Mandarin"), nullptr},
	{62, LPGEN("Mongolian"), nullptr},
	{34, LPGEN("Norwegian"), nullptr},
	{57, LPGEN("Persian"), nullptr},
	{35, LPGEN("Polish"), nullptr},
	{36, LPGEN("Portuguese"), nullptr},
	{60, LPGEN("Punjabi"), nullptr},
	{37, LPGEN("Romanian"), nullptr},
	{38, LPGEN("Russian"), nullptr},
	{39, LPGEN("Serbo-Croatian"), nullptr},
	{66, LPGEN("Sindhi"), nullptr},
	{40, LPGEN("Slovak"), nullptr},
	{41, LPGEN("Slovenian"), nullptr},
	{42, LPGEN("Somali"), nullptr},
	{43, LPGEN("Spanish"), nullptr},
	{44, LPGEN("Swahili"), nullptr},
	{45, LPGEN("Swedish"), nullptr},
	{46, LPGEN("Tagalog"), nullptr},
	{64, LPGEN("Taiwanese"), nullptr},
	{71, LPGEN("Tamil"), nullptr},
	{47, LPGEN("Tatar"), nullptr},
	{48, LPGEN("Thai"), nullptr},
	{49, LPGEN("Turkish"), nullptr},
	{50, LPGEN("Ukrainian"), nullptr},
	{51, LPGEN("Urdu"), nullptr},
	{52, LPGEN("Vietnamese"), nullptr},
	{67, LPGEN("Welsh"), nullptr},
	{53, LPGEN("Yiddish"), nullptr},
	{54, LPGEN("Yoruba"), nullptr},
};

static IDSTRLIST TmplOccupations[] = {
	{ 0, LPGEN("Unspecified"), nullptr},
	{ 1, LPGEN("Academic"), nullptr},
	{ 2, LPGEN("Administrative"), nullptr},
	{ 3, LPGEN("Art/Entertainment"), nullptr},
	{ 4, LPGEN("College Student"), nullptr},
	{ 5, LPGEN("Computers"), nullptr},
	{ 6, LPGEN("Community & Social"), nullptr},
	{ 7, LPGEN("Education"), nullptr},
	{ 8, LPGEN("Engineering"), nullptr},
	{ 9, LPGEN("Financial Services"), nullptr},
	{10, LPGEN("Government"), nullptr},
	{11, LPGEN("High School Student"), nullptr},
	{12, LPGEN("Home"), nullptr},
	{13, LPGEN("ICQ - Providing Help"), nullptr},
	{14, LPGEN("Law"), nullptr},
	{15, LPGEN("Managerial"), nullptr},
	{16, LPGEN("Manufacturing"), nullptr},
	{17, LPGEN("Medical/Health"), nullptr},
	{18, LPGEN("Military"), nullptr},
	{19, LPGEN("Non-Government Organization"), nullptr},
	{20, LPGEN("Professional"), nullptr},
	{21, LPGEN("Retail"), nullptr},
	{22, LPGEN("Retired"), nullptr},
	{23, LPGEN("Science & Research"), nullptr},
	{24, LPGEN("Sports"), nullptr},
	{25, LPGEN("Technical"), nullptr},
	{26, LPGEN("University Student"), nullptr},
	{27, LPGEN("Web Building"), nullptr},
	{99, LPGEN("Other Services"), nullptr}
};

static IDSTRLIST TmplInterests[] = {
	{	0,	LPGEN("Unspecified"), nullptr},
	{100, LPGEN("Art"), nullptr},
	{101, LPGEN("Cars"), nullptr},
	{102, LPGEN("Celebrity Fans"), nullptr},
	{103, LPGEN("Collections"), nullptr},
	{104, LPGEN("Computers"), nullptr},
	{105, LPGEN("Culture & Literature"), nullptr},
	{106, LPGEN("Fitness"), nullptr},
	{107, LPGEN("Games"), nullptr},
	{108, LPGEN("Hobbies"), nullptr},
	{109, LPGEN("ICQ - Providing Help"), nullptr},
	{110, LPGEN("Internet"), nullptr},
	{111, LPGEN("Lifestyle"), nullptr},
	{112, LPGEN("Movies/TV"), nullptr},
	{113, LPGEN("Music"), nullptr},
	{114, LPGEN("Outdoor Activities"), nullptr},
	{115, LPGEN("Parenting"), nullptr},
	{116, LPGEN("Pets/Animals"), nullptr},
	{117, LPGEN("Religion"), nullptr},
	{118, LPGEN("Science/Technology"), nullptr},
	{119, LPGEN("Skills"), nullptr},
	{120, LPGEN("Sports"), nullptr},
	{121, LPGEN("Web Design"), nullptr},
	{122, LPGEN("Nature and Environment"), nullptr},
	{123, LPGEN("News & Media"), nullptr},
	{124, LPGEN("Government"), nullptr},
	{125, LPGEN("Business & Economy"), nullptr},
	{126, LPGEN("Mystics"), nullptr},
	{127, LPGEN("Travel"), nullptr},
	{128, LPGEN("Astronomy"), nullptr},
	{129, LPGEN("Space"), nullptr},
	{130, LPGEN("Clothing"), nullptr},
	{131, LPGEN("Parties"), nullptr},
	{132, LPGEN("Women"), nullptr},
	{133, LPGEN("Social science"), nullptr},
	{134, LPGEN("60's"), nullptr},
	{135, LPGEN("70's"), nullptr},
	{136, LPGEN("80's"), nullptr},
	{137, LPGEN("50's"), nullptr},
	{138, LPGEN("Finance and corporate"), nullptr},
	{139, LPGEN("Entertainment"), nullptr},
	{140, LPGEN("Consumer electronics"), nullptr},
	{141, LPGEN("Retail stores"), nullptr},
	{142, LPGEN("Health and beauty"), nullptr},
	{143, LPGEN("Media"), nullptr},
	{144, LPGEN("Household products"), nullptr},
	{145, LPGEN("Mail order catalog"), nullptr},
	{146, LPGEN("Business services"), nullptr},
	{147, LPGEN("Audio and visual"), nullptr},
	{148, LPGEN("Sporting and athletic"), nullptr},
	{149, LPGEN("Publishing"), nullptr},
	{150, LPGEN("Home automation"), nullptr}
};

static IDSTRLIST TmplAffiliations[] = {
	{	0,	LPGEN("Unspecified"), nullptr},
	{200, LPGEN("Alumni Org."), nullptr},
	{201, LPGEN("Charity Org."), nullptr},
	{202, LPGEN("Club/Social Org."), nullptr},
	{203, LPGEN("Community Org."), nullptr},
	{204, LPGEN("Cultural Org."), nullptr},
	{205, LPGEN("Fan Clubs"), nullptr},
	{206, LPGEN("Fraternity/Sorority"), nullptr},
	{207, LPGEN("Hobbyists Org."), nullptr},
	{208, LPGEN("International Org."), nullptr},
	{209, LPGEN("Nature and Environment Org."), nullptr},
	{210, LPGEN("Professional Org."), nullptr},
	{211, LPGEN("Scientific/Technical Org."), nullptr},
	{212, LPGEN("Self Improvement Group"), nullptr},
	{213, LPGEN("Spiritual/Religious Org."), nullptr},
	{214, LPGEN("Sports Org."), nullptr},
	{215, LPGEN("Support Org."), nullptr},
	{216, LPGEN("Trade and Business Org."), nullptr},
	{217, LPGEN("Union"), nullptr},
	{218, LPGEN("Volunteer Org."), nullptr},
	{299, LPGEN("Other"), nullptr},
};

static IDSTRLIST TmplPast[] = {
	{	0,	LPGEN("Unspecified"), nullptr},
	{300, LPGEN("Elementary School"), nullptr},
	{301, LPGEN("High School"), nullptr},
	{302, LPGEN("College"), nullptr},
	{303, LPGEN("University"), nullptr},
	{304, LPGEN("Military"), nullptr},
	{305, LPGEN("Past Work Place"), nullptr},
	{306, LPGEN("Past Organization"), nullptr},
	{399, LPGEN("Other"), nullptr}
};

static IDSTRLIST TmplMarital[]={
	{ 0, LPGEN("Unspecified"), nullptr},
	{10, LPGEN("Single"), nullptr},
	{11, LPGEN("Close relationships"), nullptr},
	{12, LPGEN("Engaged"), nullptr},
	{20, LPGEN("Married"), nullptr},
	{30, LPGEN("Divorced"), nullptr},
	{31, LPGEN("Separated"), nullptr},
	{40, LPGEN("Widowed"), nullptr},
	{50, LPGEN("Actively searching"), nullptr },
	{60, LPGEN("In love"), nullptr },
	{70, LPGEN("It's complicated"), nullptr },
	{80, LPGEN("In a civil union"), nullptr }
};

static IDSTRLIST TmplPrefixes[]={
	{	0,	LPGEN("Unspecified"), nullptr},
	{'j', LPGEN("jun."), nullptr},
	{'s', LPGEN("sen."), nullptr}
};

static IDSTRLIST *MyCountries = nullptr;
static UINT MyCountriesCount = 0;

/**
* This is a sort procedure, which compares two items of an IDSTRLIST array.
* It is used by qsort in SvcConstantsTranslateList and cares about the
* locale, which was set up in OS. This prevents e.g. Ä,Ö to be put onto 
* the end of the list., but being sorted to the right position.
*
* @param	p1				- (LPIDSTRLIST) first item to compare
* @param	p2				- (LPIDSTRLIST) second item to compare
*
* returns -1, 0, 1			according to the comparison result of mir_wstrcmp.
**/

static int __cdecl ListSortProc(const LPIDSTRLIST p1, const LPIDSTRLIST p2)
{
	return mir_wstrcmpi(p1->ptszTranslated, p2->ptszTranslated);
}

/**
* Translates the text of each item of an IDStrinList to users locale
* language and saves result in szTranslated member for later use and 
* faster access to translated strings later.
*
* @param	pList			- pointer to list to translate
* @param	nListCount		- number of list items
*
* @return	nothing
**/

static void SvcConstantsTranslateList(LPIDSTRLIST pList, UINT nListCount)
{
	if (!pList[0].ptszTranslated) {
		for (UINT i = 0; i < nListCount; i++)	
			pList[i].ptszTranslated = Langpack_PcharToTchar(pList[i].pszText);

		// Ignore last item, if it is a "Other" item.
		if (!mir_strcmp(pList[nListCount-1].pszText, LPGEN("Other"))) nListCount--;

		// Sort list according translated text and ignore first item.
		qsort(pList+1, nListCount-1, sizeof(pList[0]), 
			(int (*)(const void*, const void*))ListSortProc);
	}
}

/**
* This function uses the country list provided by the core to create ower own one.
* The core's list is extended by a translated value. The cached translation is meant
* to improve speed upon adding items to a combobox.
*
* @param	pList			- LPIDSTRLIST pointer, which retrieves the list pointer.
* @param	pnListSize		- pointer to an unsigned integer, which retrieves the number of items.
*
* @retval	MIR_OK			- indicates success
* @retval	MIR_FAIL		- indicates error
**/

INT_PTR GetCountryList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	INT_PTR rc = MIR_OK;
	if (!MyCountries) {

		CountryListEntry *country;
		if (!CallService(MS_UTILS_GETCOUNTRYLIST, (WPARAM)&MyCountriesCount, (LPARAM)&country)) {
			MyCountries = (IDSTRLIST*)mir_alloc(MyCountriesCount * sizeof(IDSTRLIST));
			if (MyCountries) {
				for (UINT i = 0; i < MyCountriesCount; i++) {
					MyCountries[i].nID = country[i].id;
					MyCountries[i].pszText = country[i].szName;
					MyCountries[i].ptszTranslated = Langpack_PcharToTchar(country[i].szName);
				}
				// Sort list according translated text and ignore first item.
				qsort(MyCountries+1, MyCountriesCount-1, sizeof(MyCountries[0]), 
					(int (*)(const void*, const void*))ListSortProc);
			}
			else rc = MIR_FAIL;
		}
		else rc = MIR_FAIL;
	}
	*pnListSize = MyCountriesCount;
	*pList = MyCountries;
	return rc;
}

INT_PTR GetMaritalList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = _countof(TmplMarital);
	*pList = TmplMarital;
	SvcConstantsTranslateList(TmplMarital, *pnListSize);
	return MIR_OK;
}

INT_PTR GetLanguageList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = _countof(TmplLanguages);
	*pList = TmplLanguages;
	SvcConstantsTranslateList(TmplLanguages, *pnListSize);
	return MIR_OK;
}

INT_PTR GetOccupationList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = _countof(TmplOccupations);
	*pList = TmplOccupations;
	SvcConstantsTranslateList(TmplOccupations, *pnListSize);
	return MIR_OK;
}

INT_PTR GetInterestsList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = _countof(TmplInterests);
	*pList = TmplInterests;
	SvcConstantsTranslateList(TmplInterests, *pnListSize);
	return MIR_OK;
}

INT_PTR GetPastList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = _countof(TmplPast);
	*pList = TmplPast;
	SvcConstantsTranslateList(TmplPast, *pnListSize);
	return MIR_OK;
}

INT_PTR GetAffiliationsList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = _countof(TmplAffiliations);
	*pList = TmplAffiliations;
	SvcConstantsTranslateList(TmplAffiliations, *pnListSize);
	return MIR_OK;
}

INT_PTR GetNamePrefixList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = _countof(TmplPrefixes);
	*pList = TmplPrefixes;
	SvcConstantsTranslateList(TmplPrefixes, *pnListSize);
	return MIR_OK;
}

void SvcConstantsLoadModule(void)
{
	UINT nListSize;
	LPIDSTRLIST pList;

	// precache translation
	GetMaritalList(&nListSize, &pList);
	GetLanguageList(&nListSize, &pList);
	GetCountryList(&nListSize, &pList);
	GetOccupationList(&nListSize, &pList);
	GetInterestsList(&nListSize, &pList);
	GetPastList(&nListSize, &pList);
	GetAffiliationsList(&nListSize, &pList);
	GetNamePrefixList(&nListSize, &pList);
}

static void __forceinline SvcConstantsClearList(UINT pnListSize, LPIDSTRLIST pList)
{
	if (pList) 
		for (UINT i = 0; i < pnListSize; i++)
			MIR_FREE(pList[i].ptszTranslated);
}

void SvcConstantsUnloadModule(void)
{
	SvcConstantsClearList(_countof(TmplMarital), TmplMarital);
	SvcConstantsClearList(_countof(TmplLanguages), TmplLanguages);
	SvcConstantsClearList(_countof(TmplOccupations), TmplOccupations);
	SvcConstantsClearList(_countof(TmplInterests), TmplInterests);
	SvcConstantsClearList(_countof(TmplPast), TmplPast);
	SvcConstantsClearList(_countof(TmplAffiliations), TmplAffiliations);
	SvcConstantsClearList(_countof(TmplPrefixes), TmplPrefixes);
	SvcConstantsClearList(MyCountriesCount, MyCountries);
	MIR_FREE(MyCountries);
}

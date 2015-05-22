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

#include "commonheaders.h"

static IDSTRLIST TmplLanguages[] = {
	{ 0, LPGEN("Unspecified"), 0},
	{55, LPGEN("Afrikaans"), 0},
	{58, LPGEN("Albanian"), 0},
	{ 1, LPGEN("Arabic"), 0},
	{59, LPGEN("Armenian"), 0},
	{68, LPGEN("Azerbaijani"), 0},
	{72, LPGEN("Belorussian"), 0},
	{ 2, LPGEN("Bhojpuri"), 0},
	{56, LPGEN("Bosnian"), 0},
	{ 3, LPGEN("Bulgarian"), 0},
	{ 4, LPGEN("Burmese"), 0},
	{ 5, LPGEN("Cantonese"), 0},
	{ 6, LPGEN("Catalan"), 0},
	{61, LPGEN("Chamorro"), 0},
	{ 7, LPGEN("Chinese"), 0},
	{ 8, LPGEN("Croatian"), 0},
	{ 9, LPGEN("Czech"), 0},
	{10, LPGEN("Danish"), 0},
	{11, LPGEN("Dutch"), 0},
	{12, LPGEN("English"), 0},
	{13, LPGEN("Esperanto"), 0},
	{14, LPGEN("Estonian"), 0},
	{15, LPGEN("Farsi"), 0},
	{16, LPGEN("Finnish"), 0},
	{17, LPGEN("French"), 0},
	{18, LPGEN("Gaelic"), 0},
	{19, LPGEN("German"), 0},
	{20, LPGEN("Greek"), 0},
	{70, LPGEN("Gujarati"), 0},
	{21, LPGEN("Hebrew"), 0},
	{22, LPGEN("Hindi"), 0},
	{23, LPGEN("Hungarian"), 0},
	{24, LPGEN("Icelandic"), 0},
	{25, LPGEN("Indonesian"), 0},
	{26, LPGEN("Italian"), 0},
	{27, LPGEN("Japanese"), 0},
	{28, LPGEN("Khmer"), 0},
	{29, LPGEN("Korean"), 0},
	{69, LPGEN("Kurdish"), 0},
	{30, LPGEN("Lao"), 0},
	{31, LPGEN("Latvian"), 0},
	{32, LPGEN("Lithuanian"), 0},
	{65, LPGEN("Macedonian"), 0},
	{33, LPGEN("Malay"), 0},
	{63, LPGEN("Mandarin"), 0},
	{62, LPGEN("Mongolian"), 0},
	{34, LPGEN("Norwegian"), 0},
	{57, LPGEN("Persian"), 0},
	{35, LPGEN("Polish"), 0},
	{36, LPGEN("Portuguese"), 0},
	{60, LPGEN("Punjabi"), 0},
	{37, LPGEN("Romanian"), 0},
	{38, LPGEN("Russian"), 0},
	{39, LPGEN("Serbo-Croatian"), 0},
	{66, LPGEN("Sindhi"), 0},
	{40, LPGEN("Slovak"), 0},
	{41, LPGEN("Slovenian"), 0},
	{42, LPGEN("Somali"), 0},
	{43, LPGEN("Spanish"), 0},
	{44, LPGEN("Swahili"), 0},
	{45, LPGEN("Swedish"), 0},
	{46, LPGEN("Tagalog"), 0},
	{64, LPGEN("Taiwanese"), 0},
	{71, LPGEN("Tamil"), 0},
	{47, LPGEN("Tatar"), 0},
	{48, LPGEN("Thai"), 0},
	{49, LPGEN("Turkish"), 0},
	{50, LPGEN("Ukrainian"), 0},
	{51, LPGEN("Urdu"), 0},
	{52, LPGEN("Vietnamese"), 0},
	{67, LPGEN("Welsh"), 0},
	{53, LPGEN("Yiddish"), 0},
	{54, LPGEN("Yoruba"), 0},
};

static IDSTRLIST TmplOccupations[] = {
	{ 0, LPGEN("Unspecified"), 0},
	{ 1, LPGEN("Academic"), 0},
	{ 2, LPGEN("Administrative"), 0},
	{ 3, LPGEN("Art/Entertainment"), 0},
	{ 4, LPGEN("College Student"), 0},
	{ 5, LPGEN("Computers"), 0},
	{ 6, LPGEN("Community & Social"), 0},
	{ 7, LPGEN("Education"), 0},
	{ 8, LPGEN("Engineering"), 0},
	{ 9, LPGEN("Financial Services"), 0},
	{10, LPGEN("Government"), 0},
	{11, LPGEN("High School Student"), 0},
	{12, LPGEN("Home"), 0},
	{13, LPGEN("ICQ - Providing Help"), 0},
	{14, LPGEN("Law"), 0},
	{15, LPGEN("Managerial"), 0},
	{16, LPGEN("Manufacturing"), 0},
	{17, LPGEN("Medical/Health"), 0},
	{18, LPGEN("Military"), 0},
	{19, LPGEN("Non-Government Organization"), 0},
	{20, LPGEN("Professional"), 0},
	{21, LPGEN("Retail"), 0},
	{22, LPGEN("Retired"), 0},
	{23, LPGEN("Science & Research"), 0},
	{24, LPGEN("Sports"), 0},
	{25, LPGEN("Technical"), 0},
	{26, LPGEN("University Student"), 0},
	{27, LPGEN("Web Building"), 0},
	{99, LPGEN("Other Services"), 0}
};

static IDSTRLIST TmplInterests[] = {
	{	0,	LPGEN("Unspecified"), 0},
	{100, LPGEN("Art"), 0},
	{101, LPGEN("Cars"), 0},
	{102, LPGEN("Celebrity Fans"), 0},
	{103, LPGEN("Collections"), 0},
	{104, LPGEN("Computers"), 0},
	{105, LPGEN("Culture & Literature"), 0},
	{106, LPGEN("Fitness"), 0},
	{107, LPGEN("Games"), 0},
	{108, LPGEN("Hobbies"), 0},
	{109, LPGEN("ICQ - Providing Help"), 0},
	{110, LPGEN("Internet"), 0},
	{111, LPGEN("Lifestyle"), 0},
	{112, LPGEN("Movies/TV"), 0},
	{113, LPGEN("Music"), 0},
	{114, LPGEN("Outdoor Activities"), 0},
	{115, LPGEN("Parenting"), 0},
	{116, LPGEN("Pets/Animals"), 0},
	{117, LPGEN("Religion"), 0},
	{118, LPGEN("Science/Technology"), 0},
	{119, LPGEN("Skills"), 0},
	{120, LPGEN("Sports"), 0},
	{121, LPGEN("Web Design"), 0},
	{122, LPGEN("Nature and Environment"), 0},
	{123, LPGEN("News & Media"), 0},
	{124, LPGEN("Government"), 0},
	{125, LPGEN("Business & Economy"), 0},
	{126, LPGEN("Mystics"), 0},
	{127, LPGEN("Travel"), 0},
	{128, LPGEN("Astronomy"), 0},
	{129, LPGEN("Space"), 0},
	{130, LPGEN("Clothing"), 0},
	{131, LPGEN("Parties"), 0},
	{132, LPGEN("Women"), 0},
	{133, LPGEN("Social science"), 0},
	{134, LPGEN("60's"), 0},
	{135, LPGEN("70's"), 0},
	{136, LPGEN("80's"), 0},
	{137, LPGEN("50's"), 0},
	{138, LPGEN("Finance and corporate"), 0},
	{139, LPGEN("Entertainment"), 0},
	{140, LPGEN("Consumer electronics"), 0},
	{141, LPGEN("Retail stores"), 0},
	{142, LPGEN("Health and beauty"), 0},
	{143, LPGEN("Media"), 0},
	{144, LPGEN("Household products"), 0},
	{145, LPGEN("Mail order catalog"), 0},
	{146, LPGEN("Business services"), 0},
	{147, LPGEN("Audio and visual"), 0},
	{148, LPGEN("Sporting and athletic"), 0},
	{149, LPGEN("Publishing"), 0},
	{150, LPGEN("Home automation"), 0}
};

static IDSTRLIST TmplAffiliations[] = {
	{	0,	LPGEN("Unspecified"), 0},
	{200, LPGEN("Alumni Org."), 0},
	{201, LPGEN("Charity Org."), 0},
	{202, LPGEN("Club/Social Org."), 0},
	{203, LPGEN("Community Org."), 0},
	{204, LPGEN("Cultural Org."), 0},
	{205, LPGEN("Fan Clubs"), 0},
	{206, LPGEN("Fraternity/Sorority"), 0},
	{207, LPGEN("Hobbyists Org."), 0},
	{208, LPGEN("International Org."), 0},
	{209, LPGEN("Nature and Environment Org."), 0},
	{210, LPGEN("Professional Org."), 0},
	{211, LPGEN("Scientific/Technical Org."), 0},
	{212, LPGEN("Self Improvement Group"), 0},
	{213, LPGEN("Spiritual/Religious Org."), 0},
	{214, LPGEN("Sports Org."), 0},
	{215, LPGEN("Support Org."), 0},
	{216, LPGEN("Trade and Business Org."), 0},
	{217, LPGEN("Union"), 0},
	{218, LPGEN("Volunteer Org."), 0},
	{299, LPGEN("Other"), 0},
};

static IDSTRLIST TmplPast[] = {
	{	0,	LPGEN("Unspecified"), 0},
	{300, LPGEN("Elementary School"), 0},
	{301, LPGEN("High School"), 0},
	{302, LPGEN("College"), 0},
	{303, LPGEN("University"), 0},
	{304, LPGEN("Military"), 0},
	{305, LPGEN("TmplPast Work Place"), 0},
	{306, LPGEN("TmplPast Organization"), 0},
	{399, LPGEN("Other"), 0}
};

static IDSTRLIST TmplMarital[]={
	{ 0, LPGEN("Unspecified"), 0},
	{10, LPGEN("Single"), 0},
	{11, LPGEN("Close relationships"), 0},
	{12, LPGEN("Engaged"), 0},
	{20, LPGEN("Married"), 0},
	{30, LPGEN("Divorced"), 0},
	{31, LPGEN("Separated"), 0},
	{40, LPGEN("Widowed"), 0}
};

static IDSTRLIST TmplPrefixes[]={
	{	0,	LPGEN("Unspecified"), 0},
	{'j', LPGEN("jun."), 0},
	{'s', LPGEN("sen."), 0}
};

static IDSTRLIST *MyCountries = NULL;
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
* returns -1, 0, 1			according to the comparison result of _tcscmp.
**/

static int __cdecl ListSortProc(const LPIDSTRLIST p1, const LPIDSTRLIST p2)
{
	return mir_tstrcmpi(p1->ptszTranslated, p2->ptszTranslated);
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

static void SvcConstantsTranslateList(LPIDSTRLIST pList, UINT nListCount/*, SortedList *pSorted*/)
{
	if (!pList[0].ptszTranslated)
	{
		for (UINT i = 0; i < nListCount; i++)	
		{
			pList[i].ptszTranslated = (LPTSTR)CallService(MS_LANGPACK_PCHARTOTCHAR, 0, (LPARAM)pList[i].pszText);
		}
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
					MyCountries[i].ptszTranslated = (LPTSTR)CallService(MS_LANGPACK_PCHARTOTCHAR, 0, (LPARAM)country[i].szName);
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
	*pnListSize = SIZEOF(TmplMarital);
	*pList = TmplMarital;
	SvcConstantsTranslateList(TmplMarital, *pnListSize);
	return MIR_OK;
}

INT_PTR GetLanguageList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = SIZEOF(TmplLanguages);
	*pList = TmplLanguages;
	SvcConstantsTranslateList(TmplLanguages, *pnListSize);
	return MIR_OK;
}

INT_PTR GetOccupationList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = SIZEOF(TmplOccupations);
	*pList = TmplOccupations;
	SvcConstantsTranslateList(TmplOccupations, *pnListSize);
	return MIR_OK;
}

INT_PTR GetInterestsList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = SIZEOF(TmplInterests);
	*pList = TmplInterests;
	SvcConstantsTranslateList(TmplInterests, *pnListSize);
	return MIR_OK;
}

INT_PTR GetPastList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = SIZEOF(TmplPast);
	*pList = TmplPast;
	SvcConstantsTranslateList(TmplPast, *pnListSize);
	return MIR_OK;
}

INT_PTR GetAffiliationsList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = SIZEOF(TmplAffiliations);
	*pList = TmplAffiliations;
	SvcConstantsTranslateList(TmplAffiliations, *pnListSize);
	return MIR_OK;
}

INT_PTR GetNamePrefixList(LPUINT pnListSize, LPIDSTRLIST *pList)
{
	*pnListSize = SIZEOF(TmplPrefixes);
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

static void FORCEINLINE SvcConstantsClearList(UINT pnListSize, LPIDSTRLIST pList)
{
	if (pList) 
		for (UINT i = 0; i < pnListSize; i++)
			MIR_FREE(pList[i].ptszTranslated);
}

void SvcConstantsUnloadModule(void)
{
	SvcConstantsClearList(SIZEOF(TmplMarital), TmplMarital);
	SvcConstantsClearList(SIZEOF(TmplLanguages), TmplLanguages);
	SvcConstantsClearList(SIZEOF(TmplOccupations), TmplOccupations);
	SvcConstantsClearList(SIZEOF(TmplInterests), TmplInterests);
	SvcConstantsClearList(SIZEOF(TmplPast), TmplPast);
	SvcConstantsClearList(SIZEOF(TmplAffiliations), TmplAffiliations);
	SvcConstantsClearList(SIZEOF(TmplPrefixes), TmplPrefixes);
	SvcConstantsClearList(MyCountriesCount, MyCountries);
	MIR_FREE(MyCountries);
}

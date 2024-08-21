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

static IDSTRLIST TmplOccupations[] = {
	{ 0, LPGENW("Unspecified"), nullptr},
	{ 1, LPGENW("Academic"), nullptr},
	{ 2, LPGENW("Administrative"), nullptr},
	{ 3, LPGENW("Art/Entertainment"), nullptr},
	{ 4, LPGENW("College Student"), nullptr},
	{ 5, LPGENW("Computers"), nullptr},
	{ 6, LPGENW("Community & Social"), nullptr},
	{ 7, LPGENW("Education"), nullptr},
	{ 8, LPGENW("Engineering"), nullptr},
	{ 9, LPGENW("Financial Services"), nullptr},
	{10, LPGENW("Government"), nullptr},
	{11, LPGENW("High School Student"), nullptr},
	{12, LPGENW("Home"), nullptr},
	{13, LPGENW("Providing Help"), nullptr},
	{14, LPGENW("Law"), nullptr},
	{15, LPGENW("Managerial"), nullptr},
	{16, LPGENW("Manufacturing"), nullptr},
	{17, LPGENW("Medical/Health"), nullptr},
	{18, LPGENW("Military"), nullptr},
	{19, LPGENW("Non-Government Organization"), nullptr},
	{20, LPGENW("Professional"), nullptr},
	{21, LPGENW("Retail"), nullptr},
	{22, LPGENW("Retired"), nullptr},
	{23, LPGENW("Science & Research"), nullptr},
	{24, LPGENW("Sports"), nullptr},
	{25, LPGENW("Technical"), nullptr},
	{26, LPGENW("University Student"), nullptr},
	{27, LPGENW("Web Building"), nullptr},
	{99, LPGENW("Other Services"), nullptr}
};

static IDSTRLIST TmplInterests[] = {
	{	0,	LPGENW("Unspecified"), nullptr},
	{100, LPGENW("Art"), nullptr},
	{101, LPGENW("Cars"), nullptr},
	{102, LPGENW("Celebrity Fans"), nullptr},
	{103, LPGENW("Collections"), nullptr},
	{104, LPGENW("Computers"), nullptr},
	{105, LPGENW("Culture & Literature"), nullptr},
	{106, LPGENW("Fitness"), nullptr},
	{107, LPGENW("Games"), nullptr},
	{108, LPGENW("Hobbies"), nullptr},
	{109, LPGENW("Providing Help"), nullptr},
	{110, LPGENW("Internet"), nullptr},
	{111, LPGENW("Lifestyle"), nullptr},
	{112, LPGENW("Movies/TV"), nullptr},
	{113, LPGENW("Music"), nullptr},
	{114, LPGENW("Outdoor Activities"), nullptr},
	{115, LPGENW("Parenting"), nullptr},
	{116, LPGENW("Pets/Animals"), nullptr},
	{117, LPGENW("Religion"), nullptr},
	{118, LPGENW("Science/Technology"), nullptr},
	{119, LPGENW("Skills"), nullptr},
	{120, LPGENW("Sports"), nullptr},
	{121, LPGENW("Web Design"), nullptr},
	{122, LPGENW("Nature and Environment"), nullptr},
	{123, LPGENW("News & Media"), nullptr},
	{124, LPGENW("Government"), nullptr},
	{125, LPGENW("Business & Economy"), nullptr},
	{126, LPGENW("Mystics"), nullptr},
	{127, LPGENW("Travel"), nullptr},
	{128, LPGENW("Astronomy"), nullptr},
	{129, LPGENW("Space"), nullptr},
	{130, LPGENW("Clothing"), nullptr},
	{131, LPGENW("Parties"), nullptr},
	{132, LPGENW("Women"), nullptr},
	{133, LPGENW("Social science"), nullptr},
	{134, LPGENW("60's"), nullptr},
	{135, LPGENW("70's"), nullptr},
	{136, LPGENW("80's"), nullptr},
	{137, LPGENW("50's"), nullptr},
	{138, LPGENW("Finance and corporate"), nullptr},
	{139, LPGENW("Entertainment"), nullptr},
	{140, LPGENW("Consumer electronics"), nullptr},
	{141, LPGENW("Retail stores"), nullptr},
	{142, LPGENW("Health and beauty"), nullptr},
	{143, LPGENW("Media"), nullptr},
	{144, LPGENW("Household products"), nullptr},
	{145, LPGENW("Mail order catalog"), nullptr},
	{146, LPGENW("Business services"), nullptr},
	{147, LPGENW("Audio and visual"), nullptr},
	{148, LPGENW("Sporting and athletic"), nullptr},
	{149, LPGENW("Publishing"), nullptr},
	{150, LPGENW("Home automation"), nullptr}
};

static IDSTRLIST TmplAffiliations[] = {
	{	0,	LPGENW("Unspecified"), nullptr},
	{200, LPGENW("Alumni Org."), nullptr},
	{201, LPGENW("Charity Org."), nullptr},
	{202, LPGENW("Club/Social Org."), nullptr},
	{203, LPGENW("Community Org."), nullptr},
	{204, LPGENW("Cultural Org."), nullptr},
	{205, LPGENW("Fan Clubs"), nullptr},
	{206, LPGENW("Fraternity/Sorority"), nullptr},
	{207, LPGENW("Hobbyists Org."), nullptr},
	{208, LPGENW("International Org."), nullptr},
	{209, LPGENW("Nature and Environment Org."), nullptr},
	{210, LPGENW("Professional Org."), nullptr},
	{211, LPGENW("Scientific/Technical Org."), nullptr},
	{212, LPGENW("Self Improvement Group"), nullptr},
	{213, LPGENW("Spiritual/Religious Org."), nullptr},
	{214, LPGENW("Sports Org."), nullptr},
	{215, LPGENW("Support Org."), nullptr},
	{216, LPGENW("Trade and Business Org."), nullptr},
	{217, LPGENW("Union"), nullptr},
	{218, LPGENW("Volunteer Org."), nullptr},
	{299, LPGENW("Other"), nullptr},
};

static IDSTRLIST TmplPast[] = {
	{	0,	LPGENW("Unspecified"), nullptr},
	{300, LPGENW("Elementary School"), nullptr},
	{301, LPGENW("High School"), nullptr},
	{302, LPGENW("College"), nullptr},
	{303, LPGENW("University"), nullptr},
	{304, LPGENW("Military"), nullptr},
	{305, LPGENW("Past Work Place"), nullptr},
	{306, LPGENW("Past Organization"), nullptr},
	{399, LPGENW("Other"), nullptr}
};

static IDSTRLIST TmplMarital[]={
	{ 0, LPGENW("Unspecified"), nullptr},
	{10, LPGENW("Single"), nullptr},
	{11, LPGENW("Close relationships"), nullptr},
	{12, LPGENW("Engaged"), nullptr},
	{20, LPGENW("Married"), nullptr},
	{30, LPGENW("Divorced"), nullptr},
	{31, LPGENW("Separated"), nullptr},
	{40, LPGENW("Widowed"), nullptr},
	{50, LPGENW("Actively searching"), nullptr },
	{60, LPGENW("In love"), nullptr },
	{70, LPGENW("It's complicated"), nullptr },
	{80, LPGENW("In a civil union"), nullptr }
};

static IDSTRLIST TmplPrefixes[]={
	{	0,	LPGENW("Unspecified"), nullptr},
	{'j', LPGENW("jun."), nullptr},
	{'s', LPGENW("sen."), nullptr}
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
			pList[i].ptszTranslated = TranslateW(pList[i].pszText);

		// Ignore last item, if it is a "Other" item.
		if (!mir_wstrcmp(pList[nListCount-1].pszText, LPGENW("Other"))) nListCount--;

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
					MyCountries[i].pszText = mir_a2u(country[i].szName);
					MyCountries[i].ptszTranslated = TranslateW(MyCountries[i].pszText);
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
	GetCountryList(&nListSize, &pList);
	GetOccupationList(&nListSize, &pList);
	GetInterestsList(&nListSize, &pList);
	GetPastList(&nListSize, &pList);
	GetAffiliationsList(&nListSize, &pList);
	GetNamePrefixList(&nListSize, &pList);
}

void SvcConstantsUnloadModule(void)
{
	for (UINT i = 0; i < MyCountriesCount; i++)
		mir_free((void*)MyCountries[i].pszText);

	MIR_FREE(MyCountries);
}

/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-22 Miranda NG team (https://miranda-ng.org),
Copyright (c) 2000-12 Miranda IM project,
all portions of this codebase are copyrighted to the people
listed in contributors.txt.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "stdafx.h"

#define MS_SYSTEM_GET_MD5I	"Miranda/System/GetMD5I"

void InitClistCore(void);

static CountryListEntry countries[] = {
	{ 0,      "",   LPGEN("Unspecified") },
	{ 9999,   "",   LPGEN("Other") },
	{ 0xFFFF, "",   LPGEN("Unknown") },
	{ 93,     "AF", LPGEN("Afghanistan") },
	{ 358,    "AX", LPGEN("Aland Islands") },
	{ 355,    "AL", LPGEN("Albania") },
	{ 213,    "DZ", LPGEN("Algeria") },
	{ 1684,   "AS", LPGEN("American Samoa") },
	{ 376,    "AD", LPGEN("Andorra") },
	{ 244,    "AO", LPGEN("Angola") },
	{ 1264,   "AI", LPGEN("Anguilla") },
	{ 0xFFFE, "AQ", LPGEN("Antarctica") },
	{ 1268,   "AG", LPGEN("Antigua and Barbuda") },
	{ 54,     "AR", LPGEN("Argentina") },
	{ 374,    "AM", LPGEN("Armenia") },
	{ 297,    "AW", LPGEN("Aruba") },
	{ 61,     "AU", LPGEN("Australia") },
	{ 43,     "AT", LPGEN("Austria") },
	{ 994,    "AZ", LPGEN("Azerbaijan") },
	{ 1242,   "BS", LPGEN("Bahamas") },
	{ 973,    "BH", LPGEN("Bahrain") },
	{ 880,    "BD", LPGEN("Bangladesh") },
	{ 1246,   "BB", LPGEN("Barbados") },
	{ 375,    "BY", LPGEN("Belarus") },
	{ 32,     "BE", LPGEN("Belgium") },
	{ 501,    "BZ", LPGEN("Belize") },
	{ 229,    "BJ", LPGEN("Benin") },
	{ 1441,   "BM", LPGEN("Bermuda") },
	{ 975,    "BT", LPGEN("Bhutan") },
	{ 591,    "BO", LPGEN("Bolivia") },
	{ 5997,   "BQ", LPGEN("Bonaire, Sint Eustatius and Saba") },
	{ 387,    "BA", LPGEN("Bosnia and Herzegovina") },
	{ 267,    "BW", LPGEN("Botswana") },
	{ 55,     "BV", LPGEN("Bouvet Island") },
	{ 55,     "BR", LPGEN("Brazil") },
	{ 246,    "IO", LPGEN("British Indian Ocean Territory") },
	{ 673,    "BN", LPGEN("Brunei") },
	{ 359,    "BG", LPGEN("Bulgaria") },
	{ 226,    "BF", LPGEN("Burkina Faso") },
	{ 257,    "BI", LPGEN("Burundi") },
	{ 855,    "KH", LPGEN("Cambodia") },
	{ 237,    "CM", LPGEN("Cameroon") },
	{ 1,      "CA", LPGEN("Canada") },
	{ 238,    "CV", LPGEN("Cape Verde") },
	{ 1345,   "KY", LPGEN("Cayman Islands") },
	{ 236,    "CF", LPGEN("Central African Republic") },
	{ 235,    "TD", LPGEN("Chad") },
	{ 56,     "CL", LPGEN("Chile") },
	{ 86,     "CN", LPGEN("China") },
	{ 61,     "CX", LPGEN("Christmas Island") },
	{ 61,     "CC", LPGEN("Cocos (Keeling) Islands") },
	{ 57,     "CO", LPGEN("Colombia") },
	{ 269,    "KM", LPGEN("Comoros") },
	{ 242,    "CG", LPGEN("Congo, Republic of the") },
	{ 243,    "CD", LPGEN("Congo, Democratic Republic of the") },
	{ 682,    "CK", LPGEN("Cook Islands") },
	{ 506,    "CR", LPGEN("Costa Rica") },
	{ 225,    "CI", LPGEN("Cote d'Ivoire") },
	{ 385,    "HR", LPGEN("Croatia") },
	{ 53,     "CU", LPGEN("Cuba") },
	{ 5999,   "CW", LPGEN("Curacao") },
	{ 357,    "CY", LPGEN("Cyprus") },
	{ 420,    "CZ", LPGEN("Czech Republic") },
	{ 45,     "DK", LPGEN("Denmark") },
	{ 253,    "DJ", LPGEN("Djibouti") },
	{ 1767,   "DM", LPGEN("Dominica") },
	{ 1809,   "DO", LPGEN("Dominican Republic") },
	{ 670,    "TL", LPGEN("East Timor") },
	{ 593,    "EC", LPGEN("Ecuador") },
	{ 20,     "EG", LPGEN("Egypt") },
	{ 503,    "SV", LPGEN("El Salvador") },
	{ 240,    "GQ", LPGEN("Equatorial Guinea") },
	{ 291,    "ER", LPGEN("Eritrea") },
	{ 372,    "EE", LPGEN("Estonia") },
	{ 251,    "ET", LPGEN("Ethiopia") },
	{ 500,    "FK", LPGEN("Falkland Islands (Malvinas)") },
	{ 298,    "FO", LPGEN("Faroe Islands") },
	{ 679,    "FJ", LPGEN("Fiji") },
	{ 358,    "FI", LPGEN("Finland") },
	{ 33,     "FR", LPGEN("France") },
	{ 594,    "GF", LPGEN("French Guiana") },
	{ 689,    "PF", LPGEN("French Polynesia") },
	{ 0xFFFE, "TF", LPGEN("French Southern and Antarctic Lands") },
	{ 241,    "GA", LPGEN("Gabon") },
	{ 220,    "GM", LPGEN("Gambia") },
	{ 995,    "GE", LPGEN("Georgia") },
	{ 49,     "DE", LPGEN("Germany") },
	{ 233,    "GH", LPGEN("Ghana") },
	{ 350,    "GI", LPGEN("Gibraltar") },
	{ 30,     "GR", LPGEN("Greece") },
	{ 299,    "GL", LPGEN("Greenland") },
	{ 1473,   "GD", LPGEN("Grenada") },
	{ 590,    "GP", LPGEN("Guadeloupe") },
	{ 1671,   "GU", LPGEN("Guam") },
	{ 502,    "GT", LPGEN("Guatemala") },
	{ 44,     "GG", LPGEN("Guernsey") },
	{ 224,    "GN", LPGEN("Guinea") },
	{ 245,    "GW", LPGEN("Guinea-Bissau") },
	{ 592,    "GY", LPGEN("Guyana") },
	{ 509,    "HT", LPGEN("Haiti") },
	{ 0xFFFE, "HM", LPGEN("Heard Island and McDonald Islands") },
	{ 504,    "HN", LPGEN("Honduras") },
	{ 852,    "HK", LPGEN("Hong Kong") },
	{ 36,     "HU", LPGEN("Hungary") },
	{ 354,    "IS", LPGEN("Iceland") },
	{ 91,     "IN", LPGEN("India") },
	{ 62,     "ID", LPGEN("Indonesia") },
	{ 98,     "IR", LPGEN("Iran") },
	{ 964,    "IQ", LPGEN("Iraq") },
	{ 353,    "IE", LPGEN("Ireland") },
	{ 44,     "IM", LPGEN("Isle of Man") },
	{ 972,    "IL", LPGEN("Israel") },
	{ 39,     "IT", LPGEN("Italy") },
	{ 1876,   "JM", LPGEN("Jamaica") },
	{ 81,     "JP", LPGEN("Japan") },
	{ 44,     "JE", LPGEN("Jersey") },
	{ 962,    "JO", LPGEN("Jordan") },
	{ 76,     "KZ", LPGEN("Kazakhstan") },
	{ 254,    "KE", LPGEN("Kenya") },
	{ 686,    "KI", LPGEN("Kiribati") },
	{ 850,    "KP", LPGEN("North Korea") },
	{ 82,     "KR", LPGEN("South Korea") },
	{ 965,    "KW", LPGEN("Kuwait") },
	{ 996,    "KG", LPGEN("Kyrgyzstan") },
	{ 856,    "LA", LPGEN("Laos") },
	{ 371,    "LV", LPGEN("Latvia") },
	{ 961,    "LB", LPGEN("Lebanon") },
	{ 266,    "LS", LPGEN("Lesotho") },
	{ 231,    "LR", LPGEN("Liberia") },
	{ 218,    "LY", LPGEN("Libya") },
	{ 423,    "LI", LPGEN("Liechtenstein") },
	{ 370,    "LT", LPGEN("Lithuania") },
	{ 352,    "LU", LPGEN("Luxembourg") },
	{ 853,    "MO", LPGEN("Macau") },
	{ 389,    "MK", LPGEN("Macedonia") },
	{ 261,    "MG", LPGEN("Madagascar") },
	{ 265,    "MW", LPGEN("Malawi") },
	{ 60,     "MY", LPGEN("Malaysia") },
	{ 960,    "MV", LPGEN("Maldives") },
	{ 223,    "ML", LPGEN("Mali") },
	{ 356,    "MT", LPGEN("Malta") },
	{ 692,    "MH", LPGEN("Marshall Islands") },
	{ 596,    "MQ", LPGEN("Martinique") },
	{ 222,    "MR", LPGEN("Mauritania") },
	{ 230,    "MU", LPGEN("Mauritius") },
	{ 262,    "YT", LPGEN("Mayotte") },
	{ 52,     "MX", LPGEN("Mexico") },
	{ 691,    "FM", LPGEN("Micronesia, Federated States of") },
	{ 373,    "MD", LPGEN("Moldova") },
	{ 377,    "MC", LPGEN("Monaco") },
	{ 976,    "MN", LPGEN("Mongolia") },
	{ 382,    "ME", LPGEN("Montenegro") },
	{ 1664,   "MS", LPGEN("Montserrat") },
	{ 212,    "MA", LPGEN("Morocco") },
	{ 258,    "MZ", LPGEN("Mozambique") },
	{ 95,     "MM", LPGEN("Myanmar") },
	{ 264,    "NA", LPGEN("Namibia") },
	{ 674,    "NR", LPGEN("Nauru") },
	{ 977,    "NP", LPGEN("Nepal") },
	{ 31,     "NL", LPGEN("Netherlands") },
	{ 687,    "NC", LPGEN("New Caledonia") },
	{ 64,     "NZ", LPGEN("New Zealand") },
	{ 505,    "NI", LPGEN("Nicaragua") },
	{ 227,    "NE", LPGEN("Niger") },
	{ 234,    "NG", LPGEN("Nigeria") },
	{ 683,    "NU", LPGEN("Niue") },
	{ 672,    "NF", LPGEN("Norfolk Island") },
	{ 1670,   "MP", LPGEN("Northern Mariana Islands") },
	{ 47,     "NO", LPGEN("Norway") },
	{ 968,    "OM", LPGEN("Oman") },
	{ 92,     "PK", LPGEN("Pakistan") },
	{ 680,    "PW", LPGEN("Palau") },
	{ 970,    "PS", LPGEN("Palestinian Territories") },
	{ 507,    "PA", LPGEN("Panama") },
	{ 675,    "PG", LPGEN("Papua New Guinea") },
	{ 595,    "PY", LPGEN("Paraguay") },
	{ 51,     "PE", LPGEN("Peru") },
	{ 63,     "PH", LPGEN("Philippines") },
	{ 64,     "PN", LPGEN("Pitcairn Islands") },
	{ 48,     "PL", LPGEN("Poland") },
	{ 351,    "PT", LPGEN("Portugal") },
	{ 1787,   "PR", LPGEN("Puerto Rico") },
	{ 974,    "QA", LPGEN("Qatar") },
	{ 262,    "RE", LPGEN("Reunion") },
	{ 40,     "RO", LPGEN("Romania") },
	{ 7,      "RU", LPGEN("Russia") },
	{ 250,    "RW", LPGEN("Rwanda") },
	{ 590,    "BL", LPGEN("Saint Barthelemy") },
	{ 290,    "SH", LPGEN("Saint Helena, Ascension and Tristan da Cunha") },
	{ 1869,   "KN", LPGEN("Saint Kitts and Nevis") },
	{ 1758,   "LC", LPGEN("Saint Lucia") },
	{ 590,    "MF", LPGEN("Saint Martin (French part)") },
	{ 508,    "PM", LPGEN("Saint Pierre and Miquelon") },
	{ 1784,   "VC", LPGEN("Saint Vincent and the Grenadines") },
	{ 685,    "WS", LPGEN("Samoa") },
	{ 378,    "SM", LPGEN("San Marino") },
	{ 239,    "ST", LPGEN("Sao Tome and Principe") },
	{ 966,    "SA", LPGEN("Saudi Arabia") },
	{ 221,    "SN", LPGEN("Senegal") },
	{ 381,    "RS", LPGEN("Serbia") },
	{ 248,    "SC", LPGEN("Seychelles") },
	{ 232,    "SL", LPGEN("Sierra Leone") },
	{ 65,     "SG", LPGEN("Singapore") },
	{ 1721,   "SX", LPGEN("Sint Maarten (Dutch part)") },
	{ 421,    "SK", LPGEN("Slovakia") },
	{ 386,    "SI", LPGEN("Slovenia") },
	{ 677,    "SB", LPGEN("Solomon Islands") },
	{ 252,    "SO", LPGEN("Somalia") },
	{ 27,     "ZA", LPGEN("South Africa") },
	{ 500,    "GS", LPGEN("South Georgia and the South Sandwich Islands") },
	{ 211,    "SS", LPGEN("South Sudan") },
	{ 34,     "ES", LPGEN("Spain") },
	{ 94,     "LK", LPGEN("Sri Lanka") },
	{ 249,    "SD", LPGEN("Sudan") },
	{ 597,    "SR", LPGEN("Suriname") },
	{ 4779,   "SJ", LPGEN("Svalbard and Jan Mayen") },
	{ 268,    "SZ", LPGEN("Swaziland") },
	{ 46,     "SE", LPGEN("Sweden") },
	{ 41,     "CH", LPGEN("Switzerland") },
	{ 963,    "SY", LPGEN("Syria") },
	{ 886,    "TW", LPGEN("Taiwan") },
	{ 992,    "TJ", LPGEN("Tajikistan") },
	{ 255,    "TZ", LPGEN("Tanzania") },
	{ 66,     "TH", LPGEN("Thailand") },
	{ 228,    "TG", LPGEN("Togo") },
	{ 690,    "TK", LPGEN("Tokelau") },
	{ 676,    "TO", LPGEN("Tonga") },
	{ 1868,   "TT", LPGEN("Trinidad and Tobago") },
	{ 216,    "TN", LPGEN("Tunisia") },
	{ 90,     "TR", LPGEN("Turkey") },
	{ 993,    "TM", LPGEN("Turkmenistan") },
	{ 1649,   "TC", LPGEN("Turks and Caicos Islands") },
	{ 688,    "TV", LPGEN("Tuvalu") },
	{ 256,    "UG", LPGEN("Uganda") },
	{ 380,    "UA", LPGEN("Ukraine") },
	{ 971,    "AE", LPGEN("United Arab Emirates") },
	{ 44,     "GB", LPGEN("United Kingdom") },
	{ 1,      "US", LPGEN("United States") },
	{ 699,    "UM", LPGEN("United States Minor Outlying Islands") },
	{ 598,    "UY", LPGEN("Uruguay") },
	{ 998,    "UZ", LPGEN("Uzbekistan") },
	{ 678,    "VU", LPGEN("Vanuatu") },
	{ 379,    "VA", LPGEN("Vatican City") },
	{ 58,     "VE", LPGEN("Venezuela") },
	{ 84,     "VN", LPGEN("Vietnam") },
	{ 1284,   "VG", LPGEN("Virgin Islands (British)") },
	{ 1340,   "VI", LPGEN("Virgin Islands (United States)") },
	{ 681,    "WF", LPGEN("Wallis and Futuna") },
	{ 5289,   "EH", LPGEN("Western Sahara") },
	{ 967,    "YE", LPGEN("Yemen") },
	{ 260,    "ZM", LPGEN("Zambia") },
	{ 263,    "ZW", LPGEN("Zimbabwe") },
};

static INT_PTR GetCountryByNumber(WPARAM wParam, LPARAM)
{
	for (auto &it : countries)
		if ((int)wParam == it.id)
			return (INT_PTR)it.szName;

	return 0;
}

static INT_PTR GetCountryByISOCode(WPARAM wParam, LPARAM)
{
	for (auto &it : countries)
		if ( mir_strcmpi((char*)wParam, it.ISOcode) == 0)
			return (INT_PTR)it.szName;

	return 0;
}

static INT_PTR GetCountryList(WPARAM wParam, LPARAM lParam)
{
	*(int*)wParam = _countof(countries);
	*(CountryListEntry**)lParam = countries;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static void AddToFileList(wchar_t **&pppFiles, int &totalCount, const wchar_t *szFilename)
{
	pppFiles = (wchar_t **)mir_realloc(pppFiles, (++totalCount + 1) * sizeof(wchar_t *));
	pppFiles[totalCount] = nullptr;
	pppFiles[totalCount - 1] = mir_wstrdup(szFilename);

	if (GetFileAttributes(szFilename) & FILE_ATTRIBUTE_DIRECTORY) {
		WIN32_FIND_DATA fd;
		HANDLE hFind;
		wchar_t szPath[MAX_PATH];
		mir_wstrcpy(szPath, szFilename);
		mir_wstrcat(szPath, L"\\*");
		if (hFind = FindFirstFile(szPath, &fd)) {
			do {
				if (!mir_wstrcmp(fd.cFileName, L".") || !mir_wstrcmp(fd.cFileName, L".."))
					continue;
				mir_wstrcpy(szPath, szFilename);
				mir_wstrcat(szPath, L"\\");
				mir_wstrcat(szPath, fd.cFileName);
				AddToFileList(pppFiles, totalCount, szPath);
			} while (FindNextFile(hFind, &fd));
			FindClose(hFind);
		}
	}
}

bool ProcessFileDrop(HDROP hDrop, MCONTACT hContact)
{
	if (hDrop == nullptr || hContact == 0)
		return false;

	auto *szProto = Proto_GetBaseAccountName(hContact);
	if (szProto == nullptr)
		return false;

	int pcaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_1, 0);
	if (!(pcaps & PF1_FILESEND))
		return false;

	if (Contact::GetStatus(hContact) == ID_STATUS_OFFLINE) {
		pcaps = CallProtoService(szProto, PS_GETCAPS, PFLAGNUM_4, 0);
		if (!(pcaps & PF4_OFFLINEFILES))
			return false;
	}

	int fileCount = DragQueryFile(hDrop, -1, nullptr, 0), totalCount = 0;
	wchar_t **ppFiles = nullptr;
	for (int i = 0; i < fileCount; i++) {
		wchar_t szFilename[MAX_PATH];
		if (DragQueryFileW(hDrop, i, szFilename, _countof(szFilename)))
			AddToFileList(ppFiles, totalCount, szFilename);
	}

	CallService(MS_FILE_SENDSPECIFICFILEST, hContact, (LPARAM)ppFiles);

	for (int i=0; ppFiles[i]; i++)
		mir_free(ppFiles[i]);
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadUtilsModule(void)
{
	CreateServiceFunction(MS_UTILS_GETCOUNTRYBYNUMBER, GetCountryByNumber);
	CreateServiceFunction(MS_UTILS_GETCOUNTRYBYISOCODE, GetCountryByISOCode);
	CreateServiceFunction(MS_UTILS_GETCOUNTRYLIST, GetCountryList);

	InitClistCore();
	return 0;
}

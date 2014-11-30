/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
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

#include "..\..\core\commonheaders.h"

#define MS_SYSTEM_GET_MD5I	"Miranda/System/GetMD5I"

INT_PTR ResizeDialog(WPARAM wParam, LPARAM lParam);

int  InitOpenUrl(void);
int  InitWindowList(void);
int  InitPathUtils(void);
int  InitHyperlink(void);
int  InitColourPicker(void);
int  InitBitmapFilter(void);
void InitXmlApi(void);

void InitTimeZones(void);

int  InitCrypt(void);
void UninitCrypt(void);

INT_PTR __cdecl svcEnterString(WPARAM, LPARAM lParam);

static BOOL bModuleInitialized = FALSE;

static CountryListEntry countries[] = {
	{0,      LPGEN("Unspecified"), ""},
	{9999,   LPGEN("Other"), ""},
	{0xFFFF, LPGEN("Unknown"), ""},
	{93,     LPGEN("Afghanistan"), "AF"},
	{358,    LPGEN("Aland Islands"), "AX"},
	{355,    LPGEN("Albania"), "AL"},
	{213,    LPGEN("Algeria"), "DZ"},
	{1684,   LPGEN("American Samoa"), "AS"},
	{376,    LPGEN("Andorra"), "AD"},
	{244,    LPGEN("Angola"), "AO"},
	{1264,   LPGEN("Anguilla"), "AI"},
	{0xFFFE, LPGEN("Antarctica"), "AQ"},
	{1268,   LPGEN("Antigua and Barbuda"), "AG"},
	{54,     LPGEN("Argentina"), "AR"},
	{374,    LPGEN("Armenia"), "AM"},
	{297,    LPGEN("Aruba"), "AW"},
	{61,     LPGEN("Australia"), "AU"},
	{43,     LPGEN("Austria"), "AT"},
	{994,    LPGEN("Azerbaijan"), "AZ"},
	{1242,   LPGEN("Bahamas"), "BS"},
	{973,    LPGEN("Bahrain"), "BH"},
	{880,    LPGEN("Bangladesh"), "BD"},
	{1246,   LPGEN("Barbados"), "BB"},
	{375,    LPGEN("Belarus"), "BY"},
	{32,     LPGEN("Belgium"), "BE"},
	{501,    LPGEN("Belize"), "BZ"},
	{229,    LPGEN("Benin"), "BJ"},
	{1441,   LPGEN("Bermuda"), "BM"},
	{975,    LPGEN("Bhutan"), "BT"},
	{591,    LPGEN("Bolivia"), "BO"},
	{5997,   LPGEN("Bonaire, Sint Eustatius and Saba"), "BQ"},
	{387,    LPGEN("Bosnia and Herzegovina"), "BA"},
	{267,    LPGEN("Botswana"), "BW"},
	{55,     LPGEN("Bouvet Island"), "BV"},
	{55,     LPGEN("Brazil"), "BR"},
	{246,    LPGEN("British Indian Ocean Territory"), "IO"},
	{673,    LPGEN("Brunei"), "BN"},
	{359,    LPGEN("Bulgaria"), "BG"},
	{226,    LPGEN("Burkina Faso"), "BF"},
	{257,    LPGEN("Burundi"), "BI"},
	{855,    LPGEN("Cambodia"), "KH"},
	{237,    LPGEN("Cameroon"), "CM"},
	{1,      LPGEN("Canada"), "CA"},
	{238,    LPGEN("Cape Verde"), "CV"},
	{1345,   LPGEN("Cayman Islands"), "KY"},
	{236,    LPGEN("Central African Republic"), "CF"},
	{235,    LPGEN("Chad"), "TD"},
	{56,     LPGEN("Chile"), "CL"},
	{86,     LPGEN("China"), "CN"},
	{61,     LPGEN("Christmas Island"), "CX"},
	{61,     LPGEN("Cocos (Keeling) Islands"), "CC"},
	{57,     LPGEN("Colombia"), "CO"},
	{269,    LPGEN("Comoros"), "KM"},
	{242,    LPGEN("Congo, Republic of the"), "CG"},
	{243,    LPGEN("Congo, Democratic Republic of the"), "CD"},
	{682,    LPGEN("Cook Islands"), "CK"},
	{506,    LPGEN("Costa Rica"), "CR"},
	{225,    LPGEN("Cote d'Ivoire"), "CI"},
	{385,    LPGEN("Croatia"), "HR"},
	{53,     LPGEN("Cuba"), "CU"},
	{5999,   LPGEN("Curacao"), "CW"},
	{357,    LPGEN("Cyprus"), "CY"},
	{420,    LPGEN("Czech Republic"), "CZ"},
	{45,     LPGEN("Denmark"), "DK"},
	{253,    LPGEN("Djibouti"), "DJ"},
	{1767,   LPGEN("Dominica"), "DM"},
	{1809,   LPGEN("Dominican Republic"), "DO"},
	{670,    LPGEN("East Timor"), "TL"},
	{593,    LPGEN("Ecuador"), "EC"},
	{20,     LPGEN("Egypt"), "EG"},
	{503,    LPGEN("El Salvador"), "SV"},
	{240,    LPGEN("Equatorial Guinea"), "GQ"},
	{291,    LPGEN("Eritrea"), "ER"},
	{372,    LPGEN("Estonia"), "EE"},
	{251,    LPGEN("Ethiopia"), "ET"},
	{500,    LPGEN("Falkland Islands (Malvinas)"), "FK"},
	{298,    LPGEN("Faroe Islands"), "FO"},
	{679,    LPGEN("Fiji"), "FJ"},
	{358,    LPGEN("Finland"), "FI"},
	{33,     LPGEN("France"), "FR"},
	{594,    LPGEN("French Guiana"), "GF"},
	{689,    LPGEN("French Polynesia"), "PF"},
	{0xFFFE, LPGEN("French Southern and Antarctic Lands"), "TF"},
	{241,    LPGEN("Gabon"), "GA"},
	{220,    LPGEN("Gambia"), "GM"},
	{995,    LPGEN("Georgia"), "GE"},
	{49,     LPGEN("Germany"), "DE"},
	{233,    LPGEN("Ghana"), "GH"},
	{350,    LPGEN("Gibraltar"), "GI"},
	{30,     LPGEN("Greece"), "GR"},
	{299,    LPGEN("Greenland"), "GL"},
	{1473,   LPGEN("Grenada"), "GD"},
	{590,    LPGEN("Guadeloupe"), "GP"},
	{1671,   LPGEN("Guam"), "GU"},
	{502,    LPGEN("Guatemala"), "GT"},
	{44,     LPGEN("Guernsey"), "GG"},
	{224,    LPGEN("Guinea"), "GN"},
	{245,    LPGEN("Guinea-Bissau"), "GW"},
	{592,    LPGEN("Guyana"), "GY"},
	{509,    LPGEN("Haiti"), "HT"},
	{0xFFFE, LPGEN("Heard Island and McDonald Islands"), "HM"},
	{504,    LPGEN("Honduras"), "HN"},
	{852,    LPGEN("Hong Kong"), "HK"},
	{36,     LPGEN("Hungary"), "HU"},
	{354,    LPGEN("Iceland"), "IS"},
	{91,     LPGEN("India"), "IN"},
	{62,     LPGEN("Indonesia"), "ID"},
	{98,     LPGEN("Iran"), "IR"},
	{964,    LPGEN("Iraq"), "IQ"},
	{353,    LPGEN("Ireland"), "IE"},
	{44,     LPGEN("Isle of Man"), "IM"},
	{972,    LPGEN("Israel"), "IL"},
	{39,     LPGEN("Italy"), "IT"},
	{1876,   LPGEN("Jamaica"), "JM"},
	{81,     LPGEN("Japan"), "JP"},
	{44,     LPGEN("Jersey"), "JE"},
	{962,    LPGEN("Jordan"), "JO"},
	{76,     LPGEN("Kazakhstan"), "KZ"},
	{254,    LPGEN("Kenya"), "KE"},
	{686,    LPGEN("Kiribati"), "KI"},
	{850,    LPGEN("North Korea"), "KP"},
	{82,     LPGEN("South Korea"), "KR"},
	{965,    LPGEN("Kuwait"), "KW"},
	{996,    LPGEN("Kyrgyzstan"), "KG"},
	{856,    LPGEN("Laos"), "LA"},
	{371,    LPGEN("Latvia"), "LV"},
	{961,    LPGEN("Lebanon"), "LB"},
	{266,    LPGEN("Lesotho"), "LS"},
	{231,    LPGEN("Liberia"), "LR"},
	{218,    LPGEN("Libya"), "LY"},
	{423,    LPGEN("Liechtenstein"), "LI"},
	{370,    LPGEN("Lithuania"), "LT"},
	{352,    LPGEN("Luxembourg"), "LU"},
	{853,    LPGEN("Macau"), "MO"},
	{389,    LPGEN("Macedonia"), "MK"},
	{261,    LPGEN("Madagascar"), "MG"},
	{265,    LPGEN("Malawi"), "MW"},
	{60,     LPGEN("Malaysia"), "MY"},
	{960,    LPGEN("Maldives"), "MV"},
	{223,    LPGEN("Mali"), "ML"},
	{356,    LPGEN("Malta"), "MT"},
	{692,    LPGEN("Marshall Islands"), "MH"},
	{596,    LPGEN("Martinique"), "MQ"},
	{222,    LPGEN("Mauritania"), "MR"},
	{230,    LPGEN("Mauritius"), "MU"},
	{262,    LPGEN("Mayotte"), "YT"},
	{52,     LPGEN("Mexico"), "MX"},
	{691,    LPGEN("Micronesia, Federated States of"), "FM"},
	{373,    LPGEN("Moldova"), "MD"},
	{377,    LPGEN("Monaco"), "MC"},
	{976,    LPGEN("Mongolia"), "MN"},
	{382,    LPGEN("Montenegro"), "ME"},
	{1664,   LPGEN("Montserrat"), "MS"},
	{212,    LPGEN("Morocco"), "MA"},
	{258,    LPGEN("Mozambique"), "MZ"},
	{95,     LPGEN("Myanmar"), "MM"},
	{264,    LPGEN("Namibia"), "NA"},
	{674,    LPGEN("Nauru"), "NR"},
	{977,    LPGEN("Nepal"), "NP"},
	{31,     LPGEN("Netherlands"), "NL"},
	{687,    LPGEN("New Caledonia"), "NC"},
	{64,     LPGEN("New Zealand"), "NZ"},
	{505,    LPGEN("Nicaragua"), "NI"},
	{227,    LPGEN("Niger"), "NE"},
	{234,    LPGEN("Nigeria"), "NG"},
	{683,    LPGEN("Niue"), "NU"},
	{672,    LPGEN("Norfolk Island"), "NF"},
	{1670,   LPGEN("Northern Mariana Islands"), "MP"},
	{47,     LPGEN("Norway"), "NO"},
	{968,    LPGEN("Oman"), "OM"},
	{92,     LPGEN("Pakistan"), "PK"},
	{680,    LPGEN("Palau"), "PW"},
	{970,    LPGEN("Palestinian Territories"), "PS"},
	{507,    LPGEN("Panama"), "PA"},
	{675,    LPGEN("Papua New Guinea"), "PG"},
	{595,    LPGEN("Paraguay"), "PY"},
	{51,     LPGEN("Peru"), "PE"},
	{63,     LPGEN("Philippines"), "PH"},
	{64,     LPGEN("Pitcairn Islands"), "PN"},
	{48,     LPGEN("Poland"), "PL"},
	{351,    LPGEN("Portugal"), "PT"},
	{1787,   LPGEN("Puerto Rico"), "PR"},
	{974,    LPGEN("Qatar"), "QA"},
	{262,    LPGEN("Reunion"), "RE"},
	{40,     LPGEN("Romania"), "RO"},
	{7,      LPGEN("Russia"), "RU"},
	{250,    LPGEN("Rwanda"), "RW"},
	{590,    LPGEN("Saint Barthelemy"), "BL"},
	{290,    LPGEN("Saint Helena, Ascension and Tristan da Cunha"), "SH"},
	{1869,   LPGEN("Saint Kitts and Nevis"), "KN"},
	{1758,   LPGEN("Saint Lucia"), "LC"},
	{590,    LPGEN("Saint Martin (French part)"), "MF"},
	{508,    LPGEN("Saint Pierre and Miquelon"), "PM"},
	{1784,   LPGEN("Saint Vincent and the Grenadines"), "VC"},
	{685,    LPGEN("Samoa"), "WS"},
	{378,    LPGEN("San Marino"), "SM"},
	{239,    LPGEN("Sao Tome and Principe"), "ST"},
	{966,    LPGEN("Saudi Arabia"), "SA"},
	{221,    LPGEN("Senegal"), "SN"},
	{381,    LPGEN("Serbia"),  "RS"},
	{248,    LPGEN("Seychelles"), "SC"},
	{232,    LPGEN("Sierra Leone"), "SL"},
	{65,     LPGEN("Singapore"), "SG"},
	{1721,   LPGEN("Sint Maarten (Dutch part)"), "SX"},
	{421,    LPGEN("Slovakia"), "SK"},
	{386,    LPGEN("Slovenia"), "SI"},
	{677,    LPGEN("Solomon Islands"), "SB"},
	{252,    LPGEN("Somalia"), "SO"},
	{27,     LPGEN("South Africa"), "ZA"},
	{500,    LPGEN("South Georgia and the South Sandwich Islands"), "GS"},
	{211,    LPGEN("South Sudan"), "SS"},
	{34,     LPGEN("Spain"), "ES"},
	{94,     LPGEN("Sri Lanka"), "LK"},
	{249,    LPGEN("Sudan"), "SD"},
	{597,    LPGEN("Suriname"), "SR"},
	{4779,   LPGEN("Svalbard and Jan Mayen"), "SJ"},
	{268,    LPGEN("Swaziland"), "SZ"},
	{46,     LPGEN("Sweden"), "SE"},
	{41,     LPGEN("Switzerland"), "CH"},
	{963,    LPGEN("Syria"), "SY"},
	{886,    LPGEN("Taiwan"), "TW"},
	{992,    LPGEN("Tajikistan"), "TJ"},
	{255,    LPGEN("Tanzania"), "TZ"},
	{66,     LPGEN("Thailand"), "TH"},
	{228,    LPGEN("Togo"), "TG"},
	{690,    LPGEN("Tokelau"), "TK"},
	{676,    LPGEN("Tonga"), "TO"},
	{1868,   LPGEN("Trinidad and Tobago"), "TT"},
	{216,    LPGEN("Tunisia"), "TN"},
	{90,     LPGEN("Turkey"), "TR"},
	{993,    LPGEN("Turkmenistan"), "TM"},
	{1649,   LPGEN("Turks and Caicos Islands"), "TC"},
	{688,    LPGEN("Tuvalu"), "TV"},
	{256,    LPGEN("Uganda"), "UG"},
	{380,    LPGEN("Ukraine"), "UA"},
	{971,    LPGEN("United Arab Emirates"), "AE"},
	{44,     LPGEN("United Kingdom"), "GB"},
	{1,      LPGEN("United States"), "US"},
	{699,    LPGEN("United States Minor Outlying Islands"), "UM"},
	{598,    LPGEN("Uruguay"), "UY"},
	{998,    LPGEN("Uzbekistan"), "UZ"},
	{678,    LPGEN("Vanuatu"), "VU"},
	{379,    LPGEN("Vatican City"), "VA"},
	{58,     LPGEN("Venezuela"), "VE"},
	{84,     LPGEN("Vietnam"), "VN"},
	{1284,   LPGEN("Virgin Islands (British)"), "VG"},
	{1340,   LPGEN("Virgin Islands (United States)"), "VI"},
	{681,    LPGEN("Wallis and Futuna"), "WF"},
	{5289,   LPGEN("Western Sahara"), "EH"},
	{967,    LPGEN("Yemen"), "YE"},
	{260,    LPGEN("Zambia"), "ZM"},
	{263,    LPGEN("Zimbabwe"), "ZW"}
};

static INT_PTR GetCountryByNumber(WPARAM wParam, LPARAM)
{
	for (int i = 0; i < SIZEOF(countries); i++)
		if ((int)wParam == countries[i].id)
			return (INT_PTR)countries[i].szName;

	return NULL;
}

static INT_PTR GetCountryByISOCode(WPARAM wParam, LPARAM)
{
	for (int i = 0; i < SIZEOF(countries); i++)
		if ( mir_strcmpi((char*)wParam, countries[i].ISOcode) == 0)
			return (INT_PTR)countries[i].szName;

	return NULL;
}

static INT_PTR GetCountryList(WPARAM wParam, LPARAM lParam)
{
	*(int*)wParam = SIZEOF(countries);
	*(CountryListEntry**)lParam = countries;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR SaveWindowPosition(WPARAM, LPARAM lParam)
{
	SAVEWINDOWPOS *swp = (SAVEWINDOWPOS*)lParam;
	WINDOWPLACEMENT wp;
	char szSettingName[64];

	wp.length = sizeof(wp);
	GetWindowPlacement(swp->hwnd, &wp);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sx", swp->szNamePrefix);
	db_set_dw(swp->hContact, swp->szModule, szSettingName, wp.rcNormalPosition.left);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sy", swp->szNamePrefix);
	db_set_dw(swp->hContact, swp->szModule, szSettingName, wp.rcNormalPosition.top);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%swidth", swp->szNamePrefix);
	db_set_dw(swp->hContact, swp->szModule, szSettingName, wp.rcNormalPosition.right-wp.rcNormalPosition.left);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sheight", swp->szNamePrefix);
	db_set_dw(swp->hContact, swp->szModule, szSettingName, wp.rcNormalPosition.bottom-wp.rcNormalPosition.top);
	return 0;
}


static INT_PTR AssertInsideScreen(WPARAM wParam, LPARAM lParam)
{
	LPRECT rc = (LPRECT) wParam;
	if (rc == NULL)
		return -1;

	RECT rcScreen;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rcScreen, FALSE);

	if (MonitorFromRect(rc, MONITOR_DEFAULTTONULL))
		return 0;

	MONITORINFO mi = {0};
	HMONITOR hMonitor = MonitorFromRect(rc, MONITOR_DEFAULTTONEAREST);
	mi.cbSize = sizeof(mi);
	if (GetMonitorInfo(hMonitor, &mi))
		rcScreen = mi.rcWork;

	if (rc->top >= rcScreen.bottom)
		OffsetRect(rc, 0, rcScreen.bottom - rc->bottom);
	else if (rc->bottom <= rcScreen.top)
		OffsetRect(rc, 0, rcScreen.top - rc->top);
	if (rc->left >= rcScreen.right)
		OffsetRect(rc, rcScreen.right - rc->right, 0);
	else if (rc->right <= rcScreen.left)
		OffsetRect(rc, rcScreen.left - rc->left, 0);

	return 1;
}

static INT_PTR RestoreWindowPosition(WPARAM wParam, LPARAM lParam)
{
	SAVEWINDOWPOS *swp = (SAVEWINDOWPOS*)lParam;
	WINDOWPLACEMENT wp;
	char szSettingName[64];
	int x, y;

	wp.length = sizeof(wp);
	GetWindowPlacement(swp->hwnd, &wp);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sx", swp->szNamePrefix);
	x = db_get_dw(swp->hContact, swp->szModule, szSettingName, -1);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sy", swp->szNamePrefix);
	y = (int)db_get_dw(swp->hContact, swp->szModule, szSettingName, -1);
	if (x == -1) return 1;
	if (wParam&RWPF_NOSIZE) {
		OffsetRect(&wp.rcNormalPosition, x-wp.rcNormalPosition.left, y-wp.rcNormalPosition.top);
	}
	else {
		wp.rcNormalPosition.left = x;
		wp.rcNormalPosition.top = y;
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%swidth", swp->szNamePrefix);
		wp.rcNormalPosition.right = wp.rcNormalPosition.left+db_get_dw(swp->hContact, swp->szModule, szSettingName, -1);
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sheight", swp->szNamePrefix);
		wp.rcNormalPosition.bottom = wp.rcNormalPosition.top+db_get_dw(swp->hContact, swp->szModule, szSettingName, -1);
	}
	wp.flags = 0;
	if (wParam & RWPF_HIDDEN)
		wp.showCmd = SW_HIDE;
	if (wParam & RWPF_NOACTIVATE)
		wp.showCmd = SW_SHOWNOACTIVATE;

	if (!(wParam & RWPF_NOMOVE))
		AssertInsideScreen((WPARAM) &wp.rcNormalPosition, 0);

	SetWindowPlacement(swp->hwnd, &wp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
static INT_PTR RestartMiranda(WPARAM wParam, LPARAM)
{
	TCHAR mirandaPath[MAX_PATH], cmdLine[MAX_PATH];
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	GetModuleFileName(NULL, mirandaPath, SIZEOF(mirandaPath));
	if (wParam) {
		VARST profilename( _T("%miranda_profilename%"));
		mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d /profile=%s"), mirandaPath, GetCurrentProcessId(), (TCHAR*)profilename);
	}
	else mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d"), mirandaPath, GetCurrentProcessId());

	CallService("CloseAction", 0, 0);
	CreateProcess(mirandaPath, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef BOOL (APIENTRY *PGENRANDOM)(PVOID, ULONG);

static INT_PTR GenerateRandom(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 0 || lParam == 0) return 0;

	PGENRANDOM pfnRtlGenRandom = NULL;
	HMODULE hModule = GetModuleHandleA("advapi32");
	if (hModule)
	{
		pfnRtlGenRandom = (PGENRANDOM)GetProcAddress(hModule, "SystemFunction036");
		if (pfnRtlGenRandom)
		{
			if (!pfnRtlGenRandom((PVOID)lParam, wParam))
				pfnRtlGenRandom = NULL;
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadUtilsModule(void)
{
	bModuleInitialized = TRUE;

	CreateServiceFunction(MS_UTILS_RESIZEDIALOG, ResizeDialog);
	CreateServiceFunction(MS_UTILS_SAVEWINDOWPOSITION, SaveWindowPosition);
	CreateServiceFunction(MS_UTILS_RESTOREWINDOWPOSITION, RestoreWindowPosition);
	CreateServiceFunction(MS_UTILS_ASSERTINSIDESCREEN, AssertInsideScreen);
	CreateServiceFunction(MS_UTILS_GETCOUNTRYBYNUMBER, GetCountryByNumber);
	CreateServiceFunction(MS_UTILS_GETCOUNTRYBYISOCODE, GetCountryByISOCode);
	CreateServiceFunction(MS_UTILS_GETCOUNTRYLIST, GetCountryList);
	CreateServiceFunction(MS_UTILS_GETRANDOM, GenerateRandom);
	CreateServiceFunction(MS_UTILS_ENTERSTRING, svcEnterString);
	CreateServiceFunction(MS_SYSTEM_RESTART, RestartMiranda);

	InitOpenUrl();
	InitWindowList();
	InitHyperlink();
	InitPathUtils();
	InitColourPicker();
	InitBitmapFilter();
	InitXmlApi();
	InitTimeZones();
	InitCrypt();
	return 0;
}

void UnloadUtilsModule(void)
{
	if (!bModuleInitialized)
		return;

	UninitCrypt();
}

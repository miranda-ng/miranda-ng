/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2009 Miranda ICQ/IM project, 
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
int InitOpenUrl(void);
int InitWindowList(void);
int InitPathUtils(void);
void FreeWindowList(void);
int InitHyperlink(void);
int InitColourPicker(void);
int InitJson(void);
int InitBitmapFilter(void);
void InitXmlApi(void);
void InitTimeZones(void);
void UninitTimeZones(void);
int InitCrypt(void);

static BOOL bModuleInitialized = FALSE;

static CountryListEntry countries[] = {
	{0, "Unspecified", ""}, 
	{9999, "Other", ""}, 
	{0xFFFF, "Unknown", ""}, 
	{93, "Afghanistan", "AF"},
	{358, "Aland Islands", "AX"},
	{355, "Albania", "AL"},
	{213, "Algeria", "DZ"},
	{1684, "American Samoa", "AS"},
	{376, "Andorra", "AD"},
	{244, "Angola", "AO"},
	{1264, "Anguilla", "AI"},
	{0xFFFE, "Antarctica", "AQ"},
	{1268, "Antigua and Barbuda", "AG"},
	{54, "Argentina", "AR"},
	{374, "Armenia", "AM"},
	{297, "Aruba", "AW"},
	{61, "Australia", "AU"},
	{43, "Austria", "AT"},
	{994, "Azerbaijan", "AZ"},
	{1242, "Bahamas", "BS"},
	{973, "Bahrain", "BH"},
	{880, "Bangladesh", "BD"},
	{1246, "Barbados", "BB"},
	{375, "Belarus", "BY"},
	{32, "Belgium", "BE"},
	{501, "Belize", "BZ"},
	{229, "Benin", "BJ"},
	{1441, "Bermuda", "BM"},
	{975, "Bhutan", "BT"},
	{591, "Bolivia, Plurinational State of", "BO"},
	{5997, "Bonaire, Sint Eustatius and Saba", "BQ"},
	{387, "Bosnia and Herzegovina", "BA"},
	{267, "Botswana", "BW"},
	{55, "Bouvet Island", "BV"},
	{55, "Brazil", "BR"},
	{246, "British Indian Ocean Territory", "IO"},
	{673, "Brunei Darussalam", "BN"},
	{359, "Bulgaria", "BG"},
	{226, "Burkina Faso", "BF"},
	{257, "Burundi", "BI"},
	{855, "Cambodia", "KH"},
	{237, "Cameroon", "CM"},
	{1, "Canada", "CA"},
	{238, "Cape Verde", "CV"},
	{1345, "Cayman Islands", "KY"},
	{236, "Central African Republic", "CF"},
	{235, "Chad", "TD"},
	{56, "Chile", "CL"},
	{86, "China", "CN"},
	{61, "Christmas Island", "CX"},
	{61, "Cocos (Keeling) Islands", "CC"},
	{57, "Colombia", "CO"},
	{269, "Comoros", "KM"},
	{242, "Congo", "CG"},
	{243, "Congo, the Democratic Republic of the", "CD"},
	{682, "Cook Islands", "CK"},
	{506, "Costa Rica", "CR"},
	{225, "Cote d'Ivoire", "CI"},
	{385, "Croatia", "HR"},
	{53, "Cuba", "CU"},
	{5999, "Curacao", "CW"},
	{357, "Cyprus", "CY"},
	{420, "Czech Republic", "CZ"},
	{45, "Denmark", "DK"},
	{253, "Djibouti", "DJ"},
	{1767, "Dominica", "DM"},
	{1809, "Dominican Republic", "DO"},
	{593, "Ecuador", "EC"},
	{20, "Egypt", "EG"},
	{503, "El Salvador", "SV"},
	{240, "Equatorial Guinea", "GQ"},
	{291, "Eritrea", "ER"},
	{372, "Estonia", "EE"},
	{251, "Ethiopia", "ET"},
	{500, "Falkland Islands (Malvinas)", "FK"},
	{298, "Faroe Islands", "FO"},
	{679, "Fiji", "FJ"},
	{358, "Finland", "FI"},
	{33, "France", "FR"},
	{594, "French Guiana", "GF"},
	{689, "French Polynesia", "PF"},
	{0xFFFE, "French Southern Territories", "TF"},
	{241, "Gabon", "GA"},
	{220, "Gambia", "GM"},
	{995, "Georgia", "GE"},
	{49, "Germany", "DE"},
	{233, "Ghana", "GH"},
	{350, "Gibraltar", "GI"},
	{30, "Greece", "GR"},
	{299, "Greenland", "GL"},
	{1473, "Grenada", "GD"},
	{590, "Guadeloupe", "GP"},
	{1671, "Guam", "GU"},
	{502, "Guatemala", "GT"},
	{44, "Guernsey", "GG"},
	{224, "Guinea", "GN"},
	{245, "Guinea-Bissau", "GW"},
	{592, "Guyana", "GY"},
	{509, "Haiti", "HT"},
	{0xFFFE, "Heard Island and McDonald Islands", "HM"},
	{379, "Holy See (Vatican City State)", "VA"},
	{504, "Honduras", "HN"},
	{852, "Hong Kong", "HK"},
	{36, "Hungary", "HU"},
	{354, "Iceland", "IS"},
	{91, "India", "IN"},
	{62, "Indonesia", "ID"},
	{98, "Iran, Islamic Republic of", "IR"},
	{964, "Iraq", "IQ"},
	{353, "Ireland", "IE"},
	{44, "Isle of Man", "IM"},
	{972, "Israel", "IL"},
	{39, "Italy", "IT"},
	{1876, "Jamaica", "JM"},
	{81, "Japan", "JP"},
	{44, "Jersey", "JE"},
	{962, "Jordan", "JO"},
	{76, "Kazakhstan", "KZ"},
	{254, "Kenya", "KE"},
	{686, "Kiribati", "KI"},
	{850, "Korea, Democratic People's Republic of", "KP"},
	{82, "Korea, Republic of", "KR"},
	{965, "Kuwait", "KW"},
	{996, "Kyrgyzstan", "KG"},
	{856, "Lao People's Democratic Republic", "LA"},
	{371, "Latvia", "LV"},
	{961, "Lebanon", "LB"},
	{266, "Lesotho", "LS"},
	{231, "Liberia", "LR"},
	{218, "Libya", "LY"},
	{423, "Liechtenstein", "LI"},
	{370, "Lithuania", "LT"},
	{352, "Luxembourg", "LU"},
	{853, "Macao", "MO"},
	{389, "Macedonia, The Former Yugoslav Republic of", "MK"},
	{261, "Madagascar", "MG"},
	{265, "Malawi", "MW"},
	{60, "Malaysia", "MY"},
	{960, "Maldives", "MV"},
	{223, "Mali", "ML"},
	{356, "Malta", "MT"},
	{692, "Marshall Islands", "MH"},
	{596, "Martinique", "MQ"},
	{222, "Mauritania", "MR"},
	{230, "Mauritius", "MU"},
	{262, "Mayotte", "YT"},
	{52, "Mexico", "MX"},
	{691, "Micronesia, Federated States of", "FM"},
	{373, "Moldova, Republic of", "MD"},
	{377, "Monaco", "MC"},
	{976, "Mongolia", "MN"},
	{382, "Montenegro", "ME"},
	{1664, "Montserrat", "MS"},
	{212, "Morocco", "MA"},
	{258, "Mozambique", "MZ"},
	{95, "Myanmar", "MM"},
	{264, "Namibia", "NA"},
	{674, "Nauru", "NR"},
	{977, "Nepal", "NP"},
	{31, "Netherlands", "NL"},
	{687, "New Caledonia", "NC"},
	{64, "New Zealand", "NZ"},
	{505, "Nicaragua", "NI"},
	{227, "Niger", "NE"},
	{234, "Nigeria", "NG"},
	{683, "Niue", "NU"},
	{672, "Norfolk Island", "NF"},
	{1670, "Northern Mariana Islands", "MP"},
	{47, "Norway", "NO"},
	{968, "Oman", "OM"},
	{92, "Pakistan", "PK"},
	{680, "Palau", "PW"},
	{970, "Palestinian Territory, Occupied", "PS"},
	{507, "Panama", "PA"},
	{675, "Papua New Guinea", "PG"},
	{595, "Paraguay", "PY"},
	{51, "Peru", "PE"},
	{63, "Philippines", "PH"},
	{64, "Pitcairn", "PN"},
	{48, "Poland", "PL"},
	{351, "Portugal", "PT"},
	{1787, "Puerto Rico", "PR"},
	{974, "Qatar", "QA"},
	{262, "Reunion", "RE"},
	{40, "Romania", "RO"},
	{7, "Russian Federation", "RU"},
	{250, "Rwanda", "RW"},
	{590, "Saint Barthelemy", "BL"},
	{290, "Saint Helena, Ascension and Tristan da Cunha", "SH"},
	{1869, "Saint Kitts and Nevis", "KN"},
	{1758, "Saint Lucia", "LC"},
	{590, "Saint Martin (French part)", "MF"},
	{508, "Saint Pierre and Miquelon", "PM"},
	{1784, "Saint Vincent and the Grenadines", "VC"},
	{685, "Samoa", "WS"},
	{378, "San Marino", "SM"},
	{239, "Sao Tome and Principe", "ST"},
	{966, "Saudi Arabia", "SA"},
	{221, "Senegal", "SN"},
	{381, "Serbia", "RS"},
	{248, "Seychelles", "SC"},
	{232, "Sierra Leone", "SL"},
	{65, "Singapore", "SG"},
	{1721, "Sint Maarten (Dutch part)", "SX"},
	{421, "Slovakia", "SK"},
	{386, "Slovenia", "SI"},
	{677, "Solomon Islands", "SB"},
	{252, "Somalia", "SO"},
	{27, "South Africa", "ZA"},
	{500, "South Georgia and the South Sandwich Islands", "GS"},
	{211, "South Sudan", "SS"},
	{34, "Spain", "ES"},
	{94, "Sri Lanka", "LK"},
	{249, "Sudan", "SD"},
	{597, "Suriname", "SR"},
	{4779, "Svalbard and Jan Mayen", "SJ"},
	{268, "Swaziland", "SZ"},
	{46, "Sweden", "SE"},
	{41, "Switzerland", "CH"},
	{963, "Syrian Arab Republic", "SY"},
	{886, "Taiwan, Province of China", "TW"},
	{992, "Tajikistan", "TJ"},
	{255, "Tanzania, United Republic of", "TZ"},
	{66, "Thailand", "TH"},
	{670, "Timor-Leste", "TL"},
	{228, "Togo", "TG"},
	{690, "Tokelau", "TK"},
	{676, "Tonga", "TO"},
	{1868, "Trinidad and Tobago", "TT"},
	{216, "Tunisia", "TN"},
	{90, "Turkey", "TR"},
	{993, "Turkmenistan", "TM"},
	{1649, "Turks and Caicos Islands", "TC"},
	{688, "Tuvalu", "TV"},
	{256, "Uganda", "UG"},
	{380, "Ukraine", "UA"},
	{971, "United Arab Emirates", "AE"},
	{44, "United Kingdom", "GB"},
	{1, "United States", "US"},
	{699, "United States Minor Outlying Islands", "UM"},
	{598, "Uruguay", "UY"},
	{998, "Uzbekistan", "UZ"},
	{678, "Vanuatu", "VU"},
	{58, "Venezuela, Bolivarian Republic of", "VE"},
	{84, "Viet Nam", "VN"},
	{1284, "Virgin Islands, British", "VG"},
	{1340, "Virgin Islands, U.S.", "VI"},
	{681, "Wallis and Futuna", "WF"},
	{5289, "Western Sahara", "EH"},
	{967, "Yemen", "YE"},
	{260, "Zambia", "ZM"},
	{263, "Zimbabwe", "ZW"}
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
		if ( lstrcmpiA((char*)wParam, countries[i].ISOcode) == 0)
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

	if (MyMonitorFromWindow) 
	{
		if (MyMonitorFromRect(rc, MONITOR_DEFAULTTONULL))
			return 0;

		MONITORINFO mi = {0};
		HMONITOR hMonitor = MyMonitorFromRect(rc, MONITOR_DEFAULTTONEAREST);
		mi.cbSize = sizeof(mi);
		if (MyGetMonitorInfo(hMonitor, &mi))
			rcScreen = mi.rcWork;
	}
	else 
	{
		RECT rcDest;
		if (IntersectRect(&rcDest, &rcScreen, rc))
			return 0;
	}

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

	if ( !(wParam & RWPF_NOMOVE))
		AssertInsideScreen((WPARAM) &wp.rcNormalPosition, 0);

	SetWindowPlacement(swp->hwnd, &wp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
static INT_PTR RestartMiranda(WPARAM wParam, LPARAM)
{
	TCHAR mirandaPath[MAX_PATH], cmdLine[100];
	PROCESS_INFORMATION pi;
	STARTUPINFO si = {0};
	si.cb = sizeof(si);
	GetModuleFileName(NULL, mirandaPath, SIZEOF(mirandaPath));
	if (wParam) {
		TCHAR *profilename = Utils_ReplaceVarsT(_T("%miranda_profilename%"));
		mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d /profile=%s"), mirandaPath, GetCurrentProcessId(), profilename);
		mir_free(profilename);
	} else
		mir_sntprintf(cmdLine, SIZEOF(cmdLine), _T("\"%s\" /restart:%d"), mirandaPath, GetCurrentProcessId());
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
			if ( !pfnRtlGenRandom((PVOID)lParam, wParam))
				pfnRtlGenRandom = NULL;
		}
	}
	if (pfnRtlGenRandom == NULL)
	{
		srand(GetTickCount());
		unsigned short* buf = (unsigned short*)lParam;
		for (; (long)(wParam-=2) >= 0;)
			*(buf++) = (unsigned short)rand();
		if (lParam < 0)
			*(char*)buf = (char)(rand() & 0xFF);
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
	CreateServiceFunction(MS_SYSTEM_RESTART, RestartMiranda);

	InitOpenUrl();
	InitWindowList();
	InitHyperlink();
	InitPathUtils();
	InitColourPicker();
	InitBitmapFilter();
	InitXmlApi();
	InitJson();
	InitTimeZones();
	InitCrypt();
	return 0;
}

void UnloadUtilsModule(void)
{
	if ( !bModuleInitialized)
		return;

	FreeWindowList();
	UninitTimeZones();
}

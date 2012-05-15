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
#include "commonheaders.h"

INT_PTR ResizeDialog(WPARAM wParam,LPARAM lParam);
int InitOpenUrl(void);
int InitWindowList(void);
void FreeWindowList(void);
int InitHyperlink(void);
int InitColourPicker(void);
int InitBitmapFilter(void);
void InitXmlApi(void);
void InitTimeZones(void);
void UninitTimeZones(void);

INT_PTR GetMD5Interface(WPARAM, LPARAM);
INT_PTR GetSHA1Interface(WPARAM, LPARAM);

static BOOL bModuleInitialized = FALSE;

static struct CountryListEntry countries[]={
	{0   ,"Unspecified"},
	{9999,"Other"},
	{0xFFFF,"Unknown"},
	{93  ,"Afghanistan"},
	{355 ,"Albania"},
	{213 ,"Algeria"},
	{376 ,"Andorra"},
	{244 ,"Angola"},
	{1264,"Anguilla"},	/* change county code to NANP (from 101) */
	{1268,"Antigua and Barbuda"},	/* change county code to NANP (from 1021) */
//	{5902,"Antilles"},	/* removed: it is not a country, it's a group of islands from diffrent countries (all are included in the list)*/
	{54  ,"Argentina"},
	{374 ,"Armenia"},
	{297 ,"Aruba"},
	{247 ,"Ascension Island"},
	{61   ,"Australia"},
	{6720 ,"Australia, Antarctic Territory"},	/* added country code 672(0)*/
	{614  ,"Australia, Christmas Island"},	/* rename (from Christmas Island) and change to official county code 61(4) (from 672) */
	{61891,"Australia, Cocos (Keeling) Islands"},	/* rename and change to official county code 61(891) (from 6102) */
	{6723 ,"Australia, Norfolk Island"},	/* rename (from Norfolk Island) and change to official county code 672(3) (from 6722) */
	{43  ,"Austria"},
	{994 ,"Azerbaijan"},
	{1242,"Bahamas"},	/* change county code to NANP (from 103) */
	{973 ,"Bahrain"},
	{880 ,"Bangladesh"},
	{1246,"Barbados"},	/* change county code to NANP (from 103) */
//	{120 ,"Barbuda"},	/* removed: it is not a country and no special island, see Antigua and Barbuda*/
	{375 ,"Belarus"},
	{32  ,"Belgium"},
	{501 ,"Belize"},
	{229 ,"Benin"},
	{1441,"Bermuda"},	/* change county code to NANP (from 105) */
	{975 ,"Bhutan"},
	{591 ,"Bolivia"},
	{387 ,"Bosnia and Herzegovina"},
	{267 ,"Botswana"},
	{55  ,"Brazil"},
	{673 ,"Brunei"},
	{359 ,"Bulgaria"},
	{226 ,"Burkina Faso"},
	{257 ,"Burundi"},
	{855 ,"Cambodia"},
	{237 ,"Cameroon"},
	{1002,"Canada"},	/* change county code to NANP (from 107 to virtual 1(002) -> reflect NANP*/
	{238 ,"Cape Verde Islands"},
	{1345,"Cayman Islands"},	/* change county code to NANP (from 108) */
	{236 ,"Central African Republic"},
	{235 ,"Chad"},
	{56  ,"Chile, Republic of"},
	{86  ,"China"},
//	{6101,"Cocos-Keeling Islands"},	/* removed (double): see Australia, Cocos (Keeling) Islands */
	{57  ,"Colombia"},
	{269 ,"Comoros"},	/* change county code (from 2691) */
	{243 ,"Congo, Democratic Republic of (Zaire)"},
	{242 ,"Congo, Republic of the"},
	{682 ,"Cook Islands"},
	{506 ,"Costa Rica"},
	{225 ,"Cote d'Ivoire (Ivory Coast)"},
	{385 ,"Croatia"},
	{53  ,"Cuba"},
	{357 ,"Greek, Republic of South Cyprus"},	/* rename coz Turkey, Republic of Northern Cyprus */
	{420 ,"Czech Republic"},
	{45  ,"Denmark"},
	{246 ,"Diego Garcia"},
	{253 ,"Djibouti"},
	{1767,"Dominica"},	/* change county code to NANP (from 109) */
	{1809,"Dominican Republic"},	/* change county code to NANP 809, 829, 849 (from 110) */
	{593 ,"Ecuador"},
	{20  ,"Egypt"},
	{503 ,"El Salvador"},
	{240 ,"Equatorial Guinea"},
	{291 ,"Eritrea"},
	{372 ,"Estonia"},
	{251 ,"Ethiopia"},
	{3883,"Europe"},	/* add county code  +388 3 official European Telephony Numbering Space*/
	{298 ,"Faeroe Islands"},
	{500 ,"Falkland Islands"},
	{679 ,"Fiji"},
	{358 ,"Finland"},
	{33  ,"France"},
	{5901,"French Antilles"},
	{594 ,"French Guiana"},
	{689 ,"French Polynesia"},
	{241 ,"Gabon"},
	{220 ,"Gambia"},
	{995 ,"Georgia"},
	{49  ,"Germany"},
	{233 ,"Ghana"},
	{350 ,"Gibraltar"},
	{30  ,"Greece"},
	{299 ,"Greenland"},
	{1473,"Grenada"},	/* change county code to NANP (from 111) */
	{590 ,"Guadeloupe"},
	{1671,"Guam, US Territory of"},	/* change county code to NANP (from 671) */
	{502 ,"Guatemala"},
	{224 ,"Guinea"},
	{245 ,"Guinea-Bissau"},
	{592 ,"Guyana"},
	{509 ,"Haiti"},
	{504 ,"Honduras"},
	{852 ,"Hong Kong"},
	{36  ,"Hungary"},
	{354 ,"Iceland"},
	{91  ,"India"},
	{62  ,"Indonesia"},
	{98  ,"Iran (Islamic Republic of)"},
	{964 ,"Iraq"},
	{353 ,"Ireland"},
	{972 ,"Israel"},
	{39  ,"Italy"},
	{1876,"Jamaica"},	/* change county code to NANP (from 112) */
	{81  ,"Japan"},
	{962 ,"Jordan"},
	{705 ,"Kazakhstan"},
	{254 ,"Kenya"},
	{686 ,"Kiribati"},
	{850 ,"Korea, North"},
	{82  ,"Korea, South"},
	{965 ,"Kuwait"},
	{996 ,"Kyrgyzstan"},	/* change county code (from 706) */
	{856 ,"Laos"},
	{371 ,"Latvia"},
	{961 ,"Lebanon"},
	{266 ,"Lesotho"},
	{231 ,"Liberia"},
	{218 ,"Libyan Arab Jamahiriya"},
	{423 ,"Liechtenstein"},	/* change county code (from 4101) */
	{370 ,"Lithuania"},
	{352 ,"Luxembourg"},
	{853 ,"Macau"},
	{389 ,"Macedonia, Republic of"},	/* rename coz war */
	{261 ,"Madagascar"},
	{265 ,"Malawi"},
	{60  ,"Malaysia"},
	{960 ,"Maldives"},
	{223 ,"Mali"},
	{356 ,"Malta"},
	{692 ,"Marshall Islands"},
	{596 ,"Martinique"},
	{222 ,"Mauritania"},
	{230 ,"Mauritius"},
	{262 ,"Mayotte Island"},	/* change county code coz bug (from 269) */
	{52  ,"Mexico"},
	{691 ,"Micronesia, Federated States of"},
	{373 ,"Moldova, Republic of"},
	{377 ,"Monaco"},
	{976 ,"Mongolia"},
	{1664,"Montserrat"},	/* change county code to NANP (from 113) */
	{212 ,"Morocco"},
	{258 ,"Mozambique"},
	{95  ,"Myanmar"},
	{264 ,"Namibia"},
	{674 ,"Nauru"},
	{977 ,"Nepal"},
	{31  ,"Netherlands"},
	{599  ,"Netherlands Antilles"},	/* dissolved 2010 */
	{5995 ,"St. Maarten"},	/* add new country in 2010 (from Netherlands Antilles) */
	{5999 ,"Curacao"},	/* add new country in 2010 (from Netherlands Antilles) */
	{5997 ,"Netherlands (Bonaire Island)"},	/* add new Part of Netherlands in 2010 (from Netherlands Antilles) */
	{59946,"Netherlands (Saba Island)"},	/* add new Part of Netherlands in 2010 (from Netherlands Antilles) */
	{59938,"Netherlands (St. Eustatius Island)"},	/* add new Part of Netherlands in 2010 (from Netherlands Antilles) */
	//	{114 ,"Nevis"},	/* removed: it is not a country, it's part of Saint Kitts and Nevis*/
	{687 ,"New Caledonia"},
	{64  ,"New Zealand"},
	{505 ,"Nicaragua"},
	{227 ,"Niger"},
	{234 ,"Nigeria"},
	{683 ,"Niue"},
	{1670,"Northern Mariana Islands, US Territory of"},	/* added NANP */
	{47  ,"Norway"},
	{968 ,"Oman"},
	{92  ,"Pakistan"},
	{680 ,"Palau"},
	{507 ,"Panama"},
	{675 ,"Papua New Guinea"},
	{595 ,"Paraguay"},
	{51  ,"Peru"},
	{63  ,"Philippines"},
	{48  ,"Poland"},
	{351 ,"Portugal"},
	{1939,"Puerto Rico"},	/* change county code to NANP 939, 787 (from 121) */
	{974 ,"Qatar"},
	{262 ,"Reunion Island"},
	{40  ,"Romania"},
//	{6701,"Rota Island"},	/* removed: it is not a country it is part of Northern Mariana Islands, US Territory of */
	{7   ,"Russia"},
	{250 ,"Rwanda"},
	{1684,"Samoa (USA)"},	/* rename (from American Samoa) change county code to NANP (from 684) */
	{685 ,"Samoa, Western"},	/* rename (from Western Samoa) */
	{290 ,"Saint Helena"},
//	{115 ,"Saint Kitts"},	/* removed: it is not a country it is part of Saint Kitts and Nevis*/
	{1869,"Saint Kitts and Nevis"},	/* change county code to NANP (from 1141) */
	{1758,"Saint Lucia"},	/* change county code to NANP (from 122) */
	{508 ,"Saint Pierre and Miquelon"},
	{1784,"Saint Vincent and the Grenadines"},	/* change county code to NANP (from 116) */
//	{670 ,"Saipan Island"},	/* removed: it is not a country it is part of Northern Mariana Islands, US Territory of */
	{378 ,"San Marino"},
	{239 ,"Sao Tome and Principe"},
	{966 ,"Saudi Arabia"},
	{442 ,"Scotland"},
	{221 ,"Senegal"},
	{248 ,"Seychelles"},
	{232 ,"Sierra Leone"},
	{65  ,"Singapore"},
	{421 ,"Slovakia"},
	{386 ,"Slovenia"},
	{677 ,"Solomon Islands"},
	{252 ,"Somalia"},
	{27  ,"South Africa"},
	{34  ,"Spain"},
	{3492,"Spain, Canary Islands"},	/*rename and change county code to 34(92) spain + canary code*/
	{94  ,"Sri Lanka"},
	{249 ,"Sudan"},
	{597 ,"Suriname"},
	{268 ,"Swaziland"},
	{46  ,"Sweden"},
	{41  ,"Switzerland"},
	{963 ,"Syrian Arab Republic"},
	{886 ,"Taiwan"},
	{992 ,"Tajikistan"},	/* change county code (from 708) */
	{255 ,"Tanzania"},
	{66  ,"Thailand"},
//	{6702,"Tinian Island"},	/* removed: it is not a country it is part of Northern Mariana Islands, US Territory of */
	{670 ,"Timor, East"},	/* added (is part off Northern Mariana Islands but not US Territory*/
	{228 ,"Togo"},
	{690 ,"Tokelau"},
	{676 ,"Tonga"},
	{1868,"Trinidad and Tobago"},	/* change county code to NANP (from 1141) */
	{216 ,"Tunisia"},
	{90   ,"Turkey"},
	{90392,"Turkey, Republic of Northern Cyprus"},	/* added (is diffrent from Greek part)*/
	{993 ,"Turkmenistan"},	/* change county code (from 709) */
	{1649,"Turks and Caicos Islands"},	/* change county code to NANP (from 118) */
	{688 ,"Tuvalu"},
	{256 ,"Uganda"},
	{380 ,"Ukraine"},
	{971 ,"United Arab Emirates"},
	{44  ,"United Kingdom"},
	{598 ,"Uruguay"},
	{1   ,"USA"},
	{998 ,"Uzbekistan"},	/* change county code (from 711) */
	{678 ,"Vanuatu"},
	{379 ,"Vatican City"},
	{58  ,"Venezuela"},
	{84  ,"Vietnam"},
	{1284,"Virgin Islands (UK)"},	/* change county code to NANP (from 105) - rename coz Virgin Islands (USA) */
	{1340,"Virgin Islands (USA)"},	/* change county code to NANP (from 123) */
	{441 ,"Wales"},
	{681 ,"Wallis and Futuna Islands"},
	{967 ,"Yemen"},
	{38  ,"Yugoslavia"},	/* added for old values like birth-country */
	{381 ,"Serbia, Republic of"},	/* rename need (from Yugoslavia)*/
	{383 ,"Kosovo, Republic of"},	/*change country code (from 3811),  rename need (from Yugoslavia - Serbia) */
	{382 ,"Montenegro, Republic of"},	/* rename need (from Yugoslavia - Montenegro) */
	{260 ,"Zambia"},
	{263 ,"Zimbabwe"},
};

static INT_PTR GetCountryByNumber(WPARAM wParam, LPARAM)
{
	int i;

	for(i=0; i < SIZEOF(countries); i++ )
		if((int)wParam==countries[i].id) return (INT_PTR)countries[i].szName;
	return (INT_PTR)NULL;
}

static INT_PTR GetCountryList(WPARAM wParam,LPARAM lParam)
{
	*(int*)wParam = SIZEOF(countries);
	*(struct CountryListEntry**)lParam=countries;
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR SaveWindowPosition(WPARAM, LPARAM lParam)
{
	SAVEWINDOWPOS *swp=(SAVEWINDOWPOS*)lParam;
	WINDOWPLACEMENT wp;
	char szSettingName[64];

	wp.length=sizeof(wp);
	GetWindowPlacement(swp->hwnd,&wp);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sx", swp->szNamePrefix);
	DBWriteContactSettingDword(swp->hContact,swp->szModule,szSettingName,wp.rcNormalPosition.left);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sy", swp->szNamePrefix);
	DBWriteContactSettingDword(swp->hContact,swp->szModule,szSettingName,wp.rcNormalPosition.top);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%swidth", swp->szNamePrefix);
	DBWriteContactSettingDword(swp->hContact,swp->szModule,szSettingName,wp.rcNormalPosition.right-wp.rcNormalPosition.left);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sheight", swp->szNamePrefix);
	DBWriteContactSettingDword(swp->hContact,swp->szModule,szSettingName,wp.rcNormalPosition.bottom-wp.rcNormalPosition.top);
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


static INT_PTR RestoreWindowPosition(WPARAM wParam,LPARAM lParam)
{
	SAVEWINDOWPOS *swp=(SAVEWINDOWPOS*)lParam;
	WINDOWPLACEMENT wp;
	char szSettingName[64];
	int x,y;

	wp.length=sizeof(wp);
	GetWindowPlacement(swp->hwnd,&wp);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sx", swp->szNamePrefix);
	x=DBGetContactSettingDword(swp->hContact,swp->szModule,szSettingName,-1);
	mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sy", swp->szNamePrefix);
	y=(int)DBGetContactSettingDword(swp->hContact,swp->szModule,szSettingName,-1);
	if(x==-1) return 1;
	if(wParam&RWPF_NOSIZE) {
		OffsetRect(&wp.rcNormalPosition,x-wp.rcNormalPosition.left,y-wp.rcNormalPosition.top);
	}
	else {
		wp.rcNormalPosition.left=x;
		wp.rcNormalPosition.top=y;
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%swidth", swp->szNamePrefix);
		wp.rcNormalPosition.right=wp.rcNormalPosition.left+DBGetContactSettingDword(swp->hContact,swp->szModule,szSettingName,-1);
		mir_snprintf(szSettingName, SIZEOF(szSettingName), "%sheight", swp->szNamePrefix);
		wp.rcNormalPosition.bottom=wp.rcNormalPosition.top+DBGetContactSettingDword(swp->hContact,swp->szModule,szSettingName,-1);
	}
	wp.flags=0;
	if (wParam & RWPF_HIDDEN)
		wp.showCmd = SW_HIDE;
	if (wParam & RWPF_NOACTIVATE)
		wp.showCmd = SW_SHOWNOACTIVATE;

	if (!(wParam & RWPF_NOMOVE))
		AssertInsideScreen((WPARAM) &wp.rcNormalPosition, 0);

	SetWindowPlacement(swp->hwnd,&wp);
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

static INT_PTR RestartMiranda(WPARAM, LPARAM)
{
	TCHAR mirandaPath[ MAX_PATH ], cmdLine[ 100 ];
	PROCESS_INFORMATION pi;
	STARTUPINFO si = { 0 };
	si.cb = sizeof(si);
	GetModuleFileName( NULL, mirandaPath, SIZEOF(mirandaPath));
	mir_sntprintf( cmdLine, SIZEOF( cmdLine ), _T("/restart:%d"), GetCurrentProcessId());
	CreateProcess( mirandaPath, cmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi );
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

typedef BOOL (APIENTRY *PGENRANDOM)( PVOID, ULONG );

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
    if (pfnRtlGenRandom == NULL)
    {
        srand(GetTickCount());
        unsigned short* buf = (unsigned short*)lParam;
        for ( ; (long)(wParam-=2) >= 0; )
            *(buf++) = (unsigned short)rand();
        if (lParam < 0)
            *(char*)buf = (char)(rand() & 0xFF);
    }
    return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////

#if defined( _UNICODE )
char* __fastcall rtrim(char* str)
{
	if (str == NULL) return NULL;
	char* p = strchr(str, 0);
	while (--p >= str)
	{
		switch (*p)
		{
		case ' ': case '\t': case '\n': case '\r':
			*p = 0; break;
		default:
			return str;
		}
	}
	return str;
}
#endif

TCHAR* __fastcall rtrim(TCHAR *str)
{
	if (str == NULL) return NULL;
	TCHAR* p = _tcschr(str, 0);
	while (--p >= str)
	{
		switch (*p)
		{
		case ' ': case '\t': case '\n': case '\r':
			*p = 0; break;
		default:
			return str;
		}
	}
	return str;
}

char* __fastcall ltrim(char* str)
{
	if (str == NULL) return NULL;
	char* p = str;

	for (;;)
	{
		switch (*p)
		{
		case ' ': case '\t': case '\n': case '\r':
			++p; break;
		default:
			memmove(str, p, strlen(p) + 1);
			return str;
		}
	}
}

char* __fastcall ltrimp(char* str)
{
	if (str == NULL) return NULL;
	char* p = str;

	for (;;)
	{
		switch (*p)
		{
		case ' ': case '\t': case '\n': case '\r':
			++p; break;
		default:
			return p;
		}
	}
}

bool __fastcall wildcmp(char * name, char * mask)
{
	char * last='\0';
	for(;; mask++, name++)
	{
		if(*mask != '?' && *mask != *name) break;
		if(*name == '\0') return ((BOOL)!*mask);
	}
	if(*mask != '*') return FALSE;
	for(;; mask++, name++)
	{
		while(*mask == '*')
		{
			last = mask++;
			if(*mask == '\0') return ((BOOL)!*mask);   /* true */
		}
		if(*name == '\0') return ((BOOL)!*mask);      /* *mask == EOS */
		if(*mask != '?' && *mask != *name) name -= (size_t)(mask - last) - 1, mask = last;
	}	
}

/////////////////////////////////////////////////////////////////////////////////////////

int LoadUtilsModule(void)
{
	bModuleInitialized = TRUE;

	CreateServiceFunction(MS_UTILS_RESIZEDIALOG,ResizeDialog);
	CreateServiceFunction(MS_UTILS_SAVEWINDOWPOSITION,SaveWindowPosition);
	CreateServiceFunction(MS_UTILS_RESTOREWINDOWPOSITION,RestoreWindowPosition);
	CreateServiceFunction(MS_UTILS_ASSERTINSIDESCREEN,AssertInsideScreen);
	CreateServiceFunction(MS_UTILS_GETCOUNTRYBYNUMBER,GetCountryByNumber);
	CreateServiceFunction(MS_UTILS_GETCOUNTRYLIST,GetCountryList);
	CreateServiceFunction(MS_UTILS_GETRANDOM,GenerateRandom);
	CreateServiceFunction(MS_SYSTEM_RESTART,RestartMiranda);
	CreateServiceFunction(MS_SYSTEM_GET_MD5I,GetMD5Interface);
	CreateServiceFunction(MS_SYSTEM_GET_SHA1I,GetSHA1Interface);
	InitOpenUrl();
	InitWindowList();
	InitHyperlink();
	InitColourPicker();
	InitBitmapFilter();
	InitXmlApi();
	InitTimeZones();
	return 0;
}

void UnloadUtilsModule(void)
{
	if ( !bModuleInitialized ) return;

	FreeWindowList();
	UninitTimeZones();
}

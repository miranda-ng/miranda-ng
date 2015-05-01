// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Angeli-Ka, Joe Kucera
// Copyright © 2012-2014 Miranda NG Team
// 
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
// -----------------------------------------------------------------------------
//  DESCRIPTION:
//
//  Support for Custom Statuses
// -----------------------------------------------------------------------------

#include "stdafx.h"
#include "m_extraicons.h"
#include "..\icq_xstatus\src\resource.h"

static HANDLE hXStatusIcons[XSTATUS_COUNT];
static int    hXStatusCListIcons[XSTATUS_COUNT];
static BOOL   bXStatusCListIconsValid[XSTATUS_COUNT];

int OnReloadIcons(WPARAM, LPARAM)
{
	memset(bXStatusCListIconsValid, 0, sizeof(bXStatusCListIconsValid));
	return 0;
}

BYTE CIcqProto::getContactXStatus(MCONTACT hContact)
{
	if (!m_bXStatusEnabled && !m_bMoodsEnabled)
		return 0;

	BYTE bXStatus = getByte(hContact, DBSETTING_XSTATUS_ID, 0);
	return (bXStatus < 1 || bXStatus > XSTATUS_COUNT) ? 0 : bXStatus;
}

DWORD CIcqProto::sendXStatusDetailsRequest(MCONTACT hContact, int bForced)
{
	DWORD dwCookie = 0;

	// only request custom status detail when the contact has one
	if (m_bXStatusEnabled && getContactXStatus(hContact) != 0) {
		int nNotifyLen = 94 + UINMAXLEN;
		char *szNotify = (char*)_alloca(nNotifyLen);

		mir_snprintf(szNotify, nNotifyLen, "<srv><id>cAwaySrv</id><req><id>AwayStat</id><trans>1</trans><senderId>%d</senderId></req></srv>", m_dwLocalUIN);

		dwCookie = SendXtrazNotifyRequest(hContact, "<Q><PluginID>srvMng</PluginID></Q>", szNotify, bForced);
	}
	return dwCookie;
}

DWORD CIcqProto::requestXStatusDetails(MCONTACT hContact, BOOL bAllowDelay)
{
	if (!validateStatusMessageRequest(hContact, MTYPE_SCRIPT_NOTIFY))
		return 0; // apply privacy rules

	if (!CheckContactCapabilities(hContact, CAPF_XSTATUS))
		return 0; // contact does not have xstatus

	// delay is disabled only if fired from dialog
	if (!CheckContactCapabilities(hContact, CAPF_XTRAZ) && bAllowDelay)
		return 0; // Contact does not support xtraz, do not request details

	struct rates_xstatus_request: public rates_queue_item {
	protected:
		virtual rates_queue_item* copyItem(rates_queue_item *aDest = NULL) {
			rates_xstatus_request *pDest = (rates_xstatus_request*)aDest;
			if (!pDest)
				pDest = new rates_xstatus_request(ppro, wGroup);

			pDest->bForced = bForced;
			return rates_queue_item::copyItem(pDest);
		};
	public:
		rates_xstatus_request(CIcqProto *ppro, WORD wGroup): rates_queue_item(ppro, wGroup) { };
		virtual ~rates_xstatus_request() { };

		virtual void execute() {
			dwCookie = ppro->sendXStatusDetailsRequest(hContact, bForced);
		};

		BOOL bForced;
		DWORD dwCookie;
	};

	WORD wGroup;
	{
		mir_cslock l(m_ratesMutex);
		wGroup = m_rates->getGroupFromSNAC(ICQ_MSG_FAMILY, ICQ_MSG_SRV_SEND);
	}

	rates_xstatus_request rr(this, wGroup);
	rr.bForced = !bAllowDelay;
	rr.hContact = hContact;

	// delay at least one sec if allowed
	if (!handleRateItem(&rr, RQT_REQUEST, 1000, bAllowDelay))
		return rr.dwCookie;

	return -1; // delayed
}

static HANDLE LoadXStatusIconLibrary(TCHAR *path, const TCHAR *sub)
{
	TCHAR* p = _tcsrchr(path, '\\');
	HANDLE hLib;

	_tcscpy(p, sub);
	_tcscat(p, _T("\\xstatus_ICQ.dll"));
	if (hLib = LoadLibrary(path))
		return hLib;
	_tcscpy(p, sub);
	_tcscat(p, _T("\\xstatus_icons.dll"));
	if (hLib = LoadLibrary(path))
		return hLib;
	_tcscpy(p, _T("\\"));
	return hLib;
}

static TCHAR* InitXStatusIconLibrary(TCHAR *buf, size_t buf_size)
{
	TCHAR path[2*MAX_PATH];
	HMODULE hXStatusIconsDLL;

	// get miranda's exe path
	GetModuleFileName(NULL, path, MAX_PATH);

	hXStatusIconsDLL = (HMODULE)LoadXStatusIconLibrary(path, _T("\\Icons"));
	if (!hXStatusIconsDLL) // TODO: add "Custom Folders" support
		hXStatusIconsDLL = (HMODULE)LoadXStatusIconLibrary(path, _T("\\Plugins"));

	if (hXStatusIconsDLL) {
		null_strcpy(buf, path, buf_size - 1);

		char ident[MAX_PATH];
		if ( LoadStringA(hXStatusIconsDLL, IDS_IDENTIFY, ident, sizeof(ident)) == 0 || strcmpnull(ident, "# Custom Status Icons #"))
			*buf = 0;

		FreeLibrary(hXStatusIconsDLL);
	}
	else *buf = 0;

	return buf;
}

HICON CIcqProto::getXStatusIcon(int bStatus, UINT flags)
{
	HICON icon = NULL;

	if (bStatus > 0 && bStatus <= XSTATUS_COUNT)
		icon = Skin_GetIconByHandle(hXStatusIcons[bStatus-1], (flags & LR_BIGICON) != 0);

	return (flags & LR_SHARED || !icon) ? icon : CopyIcon(icon);
}

void setContactExtraIcon(MCONTACT hContact, int xstatus)
{
	ExtraIcon_SetIcon(hExtraXStatus, hContact, (xstatus > 0) ? hXStatusIcons[xstatus-1] : NULL);
}

#define NULLCAP {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}

capstr capXStatus[XSTATUS_COUNT] = {
	{0x01, 0xD8, 0xD7, 0xEE, 0xAC, 0x3B, 0x49, 0x2A, 0xA5, 0x8D, 0xD3, 0xD8, 0x77, 0xE6, 0x6B, 0x92},
	{0x5A, 0x58, 0x1E, 0xA1, 0xE5, 0x80, 0x43, 0x0C, 0xA0, 0x6F, 0x61, 0x22, 0x98, 0xB7, 0xE4, 0xC7},
	{0x83, 0xC9, 0xB7, 0x8E, 0x77, 0xE7, 0x43, 0x78, 0xB2, 0xC5, 0xFB, 0x6C, 0xFC, 0xC3, 0x5B, 0xEC},
	{0xE6, 0x01, 0xE4, 0x1C, 0x33, 0x73, 0x4B, 0xD1, 0xBC, 0x06, 0x81, 0x1D, 0x6C, 0x32, 0x3D, 0x81},
	{0x8C, 0x50, 0xDB, 0xAE, 0x81, 0xED, 0x47, 0x86, 0xAC, 0xCA, 0x16, 0xCC, 0x32, 0x13, 0xC7, 0xB7},
	{0x3F, 0xB0, 0xBD, 0x36, 0xAF, 0x3B, 0x4A, 0x60, 0x9E, 0xEF, 0xCF, 0x19, 0x0F, 0x6A, 0x5A, 0x7F},
	{0xF8, 0xE8, 0xD7, 0xB2, 0x82, 0xC4, 0x41, 0x42, 0x90, 0xF8, 0x10, 0xC6, 0xCE, 0x0A, 0x89, 0xA6},
	{0x80, 0x53, 0x7D, 0xE2, 0xA4, 0x67, 0x4A, 0x76, 0xB3, 0x54, 0x6D, 0xFD, 0x07, 0x5F, 0x5E, 0xC6},
	{0xF1, 0x8A, 0xB5, 0x2E, 0xDC, 0x57, 0x49, 0x1D, 0x99, 0xDC, 0x64, 0x44, 0x50, 0x24, 0x57, 0xAF},
	{0x1B, 0x78, 0xAE, 0x31, 0xFA, 0x0B, 0x4D, 0x38, 0x93, 0xD1, 0x99, 0x7E, 0xEE, 0xAF, 0xB2, 0x18},
	{0x61, 0xBE, 0xE0, 0xDD, 0x8B, 0xDD, 0x47, 0x5D, 0x8D, 0xEE, 0x5F, 0x4B, 0xAA, 0xCF, 0x19, 0xA7},
	{0x48, 0x8E, 0x14, 0x89, 0x8A, 0xCA, 0x4A, 0x08, 0x82, 0xAA, 0x77, 0xCE, 0x7A, 0x16, 0x52, 0x08},
	{0x10, 0x7A, 0x9A, 0x18, 0x12, 0x32, 0x4D, 0xA4, 0xB6, 0xCD, 0x08, 0x79, 0xDB, 0x78, 0x0F, 0x09},
	{0x6F, 0x49, 0x30, 0x98, 0x4F, 0x7C, 0x4A, 0xFF, 0xA2, 0x76, 0x34, 0xA0, 0x3B, 0xCE, 0xAE, 0xA7},
	{0x12, 0x92, 0xE5, 0x50, 0x1B, 0x64, 0x4F, 0x66, 0xB2, 0x06, 0xB2, 0x9A, 0xF3, 0x78, 0xE4, 0x8D},
	{0xD4, 0xA6, 0x11, 0xD0, 0x8F, 0x01, 0x4E, 0xC0, 0x92, 0x23, 0xC5, 0xB6, 0xBE, 0xC6, 0xCC, 0xF0},
	{0x60, 0x9D, 0x52, 0xF8, 0xA2, 0x9A, 0x49, 0xA6, 0xB2, 0xA0, 0x25, 0x24, 0xC5, 0xE9, 0xD2, 0x60},
	{0x63, 0x62, 0x73, 0x37, 0xA0, 0x3F, 0x49, 0xFF, 0x80, 0xE5, 0xF7, 0x09, 0xCD, 0xE0, 0xA4, 0xEE},
	{0x1F, 0x7A, 0x40, 0x71, 0xBF, 0x3B, 0x4E, 0x60, 0xBC, 0x32, 0x4C, 0x57, 0x87, 0xB0, 0x4C, 0xF1},
	{0x78, 0x5E, 0x8C, 0x48, 0x40, 0xD3, 0x4C, 0x65, 0x88, 0x6F, 0x04, 0xCF, 0x3F, 0x3F, 0x43, 0xDF},
	{0xA6, 0xED, 0x55, 0x7E, 0x6B, 0xF7, 0x44, 0xD4, 0xA5, 0xD4, 0xD2, 0xE7, 0xD9, 0x5C, 0xE8, 0x1F},
	{0x12, 0xD0, 0x7E, 0x3E, 0xF8, 0x85, 0x48, 0x9E, 0x8E, 0x97, 0xA7, 0x2A, 0x65, 0x51, 0xE5, 0x8D},
	{0xBA, 0x74, 0xDB, 0x3E, 0x9E, 0x24, 0x43, 0x4B, 0x87, 0xB6, 0x2F, 0x6B, 0x8D, 0xFE, 0xE5, 0x0F},
	{0x63, 0x4F, 0x6B, 0xD8, 0xAD, 0xD2, 0x4A, 0xA1, 0xAA, 0xB9, 0x11, 0x5B, 0xC2, 0x6D, 0x05, 0xA1},
	{0x2C, 0xE0, 0xE4, 0xE5, 0x7C, 0x64, 0x43, 0x70, 0x9C, 0x3A, 0x7A, 0x1C, 0xE8, 0x78, 0xA7, 0xDC},
	{0x10, 0x11, 0x17, 0xC9, 0xA3, 0xB0, 0x40, 0xF9, 0x81, 0xAC, 0x49, 0xE1, 0x59, 0xFB, 0xD5, 0xD4},
	{0x16, 0x0C, 0x60, 0xBB, 0xDD, 0x44, 0x43, 0xF3, 0x91, 0x40, 0x05, 0x0F, 0x00, 0xE6, 0xC0, 0x09},
	{0x64, 0x43, 0xC6, 0xAF, 0x22, 0x60, 0x45, 0x17, 0xB5, 0x8C, 0xD7, 0xDF, 0x8E, 0x29, 0x03, 0x52},
	{0x16, 0xF5, 0xB7, 0x6F, 0xA9, 0xD2, 0x40, 0x35, 0x8C, 0xC5, 0xC0, 0x84, 0x70, 0x3C, 0x98, 0xFA},
	{0x63, 0x14, 0x36, 0xff, 0x3f, 0x8a, 0x40, 0xd0, 0xa5, 0xcb, 0x7b, 0x66, 0xe0, 0x51, 0xb3, 0x64}, 
	{0xb7, 0x08, 0x67, 0xf5, 0x38, 0x25, 0x43, 0x27, 0xa1, 0xff, 0xcf, 0x4c, 0xc1, 0x93, 0x97, 0x97},
	{0xdd, 0xcf, 0x0e, 0xa9, 0x71, 0x95, 0x40, 0x48, 0xa9, 0xc6, 0x41, 0x32, 0x06, 0xd6, 0xf2, 0x80},
	NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP,
	NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP,
	NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP,
	NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP,
	NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP, NULLCAP,
	NULLCAP, NULLCAP, NULLCAP, NULLCAP};

const char *nameXStatus[XSTATUS_COUNT] = {
	LPGEN("Angry"),         // 23
	LPGEN("Taking a bath"), // 1
	LPGEN("Tired"),         // 2
	LPGEN("Birthday"),      // 3
	LPGEN("Drinking beer"), // 4
	LPGEN("Thinking"),      // 5
	LPGEN("Eating"),        // 80
	LPGEN("Watching TV"),   // 7
	LPGEN("Meeting"),       // 8
	LPGEN("Coffee"),        // 9
	LPGEN("Listening to music"),// 10
	LPGEN("Business"),      // 11
	LPGEN("Shooting"),      // 12
	LPGEN("Having fun"),    // 13
	LPGEN("On the phone"),  // 14
	LPGEN("Gaming"),        // 15
	LPGEN("Studying"),      // 16
	LPGEN("Shopping"),      // 0
	LPGEN("Feeling sick"),  // 17
	LPGEN("Sleeping"),      // 18
	LPGEN("Surfing"),       // 19
	LPGEN("Internet"),      // 20
	LPGEN("Working"),       // 21
	LPGEN("Typing"),        // 22
	LPGEN("Picnic"),        // 66
	LPGEN("Cooking"),
	LPGEN("Smoking"),
	LPGEN("I'm high"),
	LPGEN("On WC"),         // 68
	LPGEN("To be or not to be"),// 77
	LPGEN("Watching pro7 on TV"),
	LPGEN("Love"),          // 61
	LPGEN("Hot Dog"),       //6
	LPGEN("Rough"),         //24
	LPGEN("Rock On"),       //25
	LPGEN("Baby"),          //26
	LPGEN("Soccer"),        //27
	LPGEN("Pirate"),        //28
	LPGEN("Cyclop"),        //29
	LPGEN("Monkey"),        //30
	LPGEN("Birdie"),        //31
	LPGEN("Cool"),          //32
	LPGEN("Evil"),          //33
	LPGEN("Alien"),         //34
	LPGEN("Scooter"),       //35
	LPGEN("Mask"),          //36
	LPGEN("Money"),         //37
	LPGEN("Pilot"),         //38
	LPGEN("Afro"),          //39
	LPGEN("St. Patrick"),   //40
	LPGEN("Headmaster"),    //41
	LPGEN("Lips"),          //42
	LPGEN("Ice-Cream"),     //43
	LPGEN("Pink Lady"),     //44
	LPGEN("Up yours"),      //45
	LPGEN("Laughing"),      //46
	LPGEN("Dog"),           //47
	LPGEN("Candy"),         //48
	LPGEN("Crazy Professor"),//50
	LPGEN("Ninja"),         //51
	LPGEN("Cocktail"),      //52
	LPGEN("Punch"),         //53
	LPGEN("Donut"),         //54
	LPGEN("Feeling Good"),  //55
	LPGEN("Lollypop"),      //56
	LPGEN("Oink Oink"),     //57
	LPGEN("Kitty"),         //58
	LPGEN("Sumo"),          //59
	LPGEN("Broken hearted"),//60
	LPGEN("Free for Chat"), //62
	LPGEN("@home"),         //63
	LPGEN("@work"),         //64
	LPGEN("Strawberry"),    //65
	LPGEN("Angel"),         //67
	LPGEN("Pizza"),         //69
	LPGEN("Snoring"),       //70
	LPGEN("On my mobile"),  //71
	LPGEN("Depressed"),     //72
	LPGEN("Beetle"),        //73
	LPGEN("Double Rainbow"),//74
	LPGEN("Basketball"),    //75
	LPGEN("Cupid shot me"), //76
	LPGEN("Celebrating"),   //78
	LPGEN("Sushi"),         //79
	LPGEN("Playing"),       //81
	LPGEN("Writing")        //84
	};

const int moodXStatus[XSTATUS_COUNT] = {
	23,
	1,
	2,
	3,
	4,
	5,
	80,
	7,
	8,
	9,
	10,
	11,
	12,
	13,
	14,
	15,
	16,
	0,
	17,
	18,
	19,
	20,
	21,
	22,
	66,
	-1,
	-1,
	-1,
	68,
	77,
	-1,
	61,
	6,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	32,
	33,
	34,
	35,
	36,
	37,
	38,
	39,
	40,
	41,
	42,
	43,
	44,
	45,
	46,
	47,
	48,
	50,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	62,
	63,
	64,
	65,
	67,
	69,
	70,
	71,
	72,
	73,
	74,
	75,
	76,
	78,
	79,
	81,
	84};

void CIcqProto::handleXStatusCaps(DWORD dwUIN, char *szUID, MCONTACT hContact, BYTE *caps, int capsize, char *moods, int moodsize)
{
	int bChanged = FALSE;
	int nCustomStatusID = 0, nMoodID = 0;

	if (!m_bXStatusEnabled && !m_bMoodsEnabled) {
		ClearContactCapabilities(hContact, CAPF_STATUS_MOOD | CAPF_XSTATUS);
		return;
	}
	int nOldXStatusID = getContactXStatus(hContact);

	if (m_bMoodsEnabled) {
		 // process custom statuses (moods) from ICQ6
		if (moods) {
			if (moodsize > 0)
				if (moodsize >= 32)
					moods[32] = '\0';

				for (int i = 0; i < XSTATUS_COUNT; i++) {
					char szMoodId[32], szMoodData[32];

					null_strcpy(szMoodData, moods, moodsize);

					if (moodXStatus[i] == -1) continue;
					mir_snprintf(szMoodId, SIZEOF(szMoodId), "0icqmood%d", moodXStatus[i]);
					if ( !strcmpnull(szMoodId, szMoodData)) {
						BYTE bXStatusId = (BYTE)(i+1);
						char str[MAX_PATH];

						SetContactCapabilities(hContact, CAPF_STATUS_MOOD);

						 // only write default name when it is really needed, i.e. on Custom Status change
						if (nOldXStatusID != bXStatusId) {
							setByte(hContact, DBSETTING_XSTATUS_ID, bXStatusId);
							db_set_utf(hContact, m_szModuleName, DBSETTING_XSTATUS_NAME, ICQTranslateUtfStatic(nameXStatus[i], str, MAX_PATH));
							delSetting(hContact, DBSETTING_XSTATUS_MSG);

							debugLogA("%s changed mood to %s.", strUID(dwUIN, szUID), ICQTranslateUtfStatic(nameXStatus[i], str, MAX_PATH));
							bChanged = TRUE;
						}

						// cannot retrieve mood details here - need to be processed with new user details
						nMoodID = bXStatusId;

						break;
					}
			}

			if (nMoodID == 0 && moods)
				ClearContactCapabilities(hContact, CAPF_STATUS_MOOD);
		}
	}

	if (m_bXStatusEnabled) {
		 // detect custom status capabilities
		if (caps) {
			if (capsize > 0)
				for (int i = 0; i < XSTATUS_COUNT; i++) {
					if (MatchCapability(caps, capsize, (const capstr*)capXStatus[i], BINARY_CAP_SIZE)) {
						BYTE bXStatusId = (BYTE)(i+1);
						char str[MAX_PATH];

						SetContactCapabilities(hContact, CAPF_XSTATUS);

						 // only write default name when it is really needed, i.e. on Custom Status change
						if (nMoodID == 0 && nOldXStatusID != bXStatusId) {
							setByte(hContact, DBSETTING_XSTATUS_ID, bXStatusId);
							db_set_utf(hContact, m_szModuleName, DBSETTING_XSTATUS_NAME, ICQTranslateUtfStatic(nameXStatus[i], str, MAX_PATH));
							delSetting(hContact, DBSETTING_XSTATUS_MSG);

							debugLogA("%s changed custom status to %s.", strUID(dwUIN, szUID), ICQTranslateUtfStatic(nameXStatus[i], str, MAX_PATH));
							bChanged = TRUE;
						}

						if (getByte("XStatusAuto", DEFAULT_XSTATUS_AUTO))
							requestXStatusDetails(hContact, TRUE);

						nCustomStatusID = bXStatusId;
						break;
					}
				}

			if (nCustomStatusID == 0)
				ClearContactCapabilities(hContact, CAPF_XSTATUS);
		}
	}

	if (nCustomStatusID != 0 && nMoodID != 0 && nCustomStatusID != nMoodID)
		debugLogA("Warning: Diverse custom statuses detected, using mood status.");

	if ((nCustomStatusID == 0 && (caps || !m_bXStatusEnabled)) && (nMoodID == 0 && (moods || !m_bMoodsEnabled))) {
		if (getByte(hContact, DBSETTING_XSTATUS_ID, -1) != -1)
			bChanged = TRUE;
		delSetting(hContact, DBSETTING_XSTATUS_ID);
		delSetting(hContact, DBSETTING_XSTATUS_NAME);
		delSetting(hContact, DBSETTING_XSTATUS_MSG);
	}

	if (m_bXStatusEnabled != 10 && m_bMoodsEnabled != 10)
		setContactExtraIcon(hContact, nMoodID ? nMoodID : (moods ? 0 : (nCustomStatusID ? nCustomStatusID : nOldXStatusID)));
}


void CIcqProto::updateServerCustomStatus(int fullUpdate)
{
	BYTE bXStatus = getContactXStatus(NULL);

	// update client capabilities
	if (fullUpdate) {
		if (m_bXStatusEnabled)
			setUserInfo();

		char szMoodData[32];

		// prepare mood id
		if (m_bMoodsEnabled && bXStatus && moodXStatus[bXStatus-1] != -1)
			mir_snprintf(szMoodData, SIZEOF(szMoodData), "0icqmood%d", moodXStatus[bXStatus-1]);
		else
			szMoodData[0] = '\0';

		SetStatusMood(szMoodData, 1500);
	}

	char *szStatusNote = NULL;

	// use custom status message as status note
	if (bXStatus && (m_bXStatusEnabled || m_bMoodsEnabled))
		szStatusNote = getSettingStringUtf(NULL, DBSETTING_XSTATUS_MSG, "");
	// retrieve standard status message (e.g. custom status set to none)
	else { 
		char **pszMsg = MirandaStatusToAwayMsg(m_iStatus);
		{
			mir_cslock l(m_modeMsgsMutex);
			if (pszMsg)
				szStatusNote = null_strdup(*pszMsg);
		}
		
		// no default status message, set empty
		if (!szStatusNote)
			szStatusNote = null_strdup("");
	}

	if (szStatusNote)
		SetStatusNote(szStatusNote, 1500, FALSE);

	SAFE_FREE(&szStatusNote);
}

static LRESULT CALLBACK MessageEditSubclassProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_CHAR:
		if(wParam=='\n' && GetKeyState(VK_CONTROL) & 0x8000) {
			PostMessage(GetParent(hwnd),WM_COMMAND,IDOK,0);
			return 0;
		}
		if (wParam == 1 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-a
			SendMessage(hwnd, EM_SETSEL, 0, -1);
			return 0;
		}
		if (wParam == 23 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-w
			SendMessage(GetParent(hwnd), WM_CLOSE, 0, 0);
			return 0;
		}
		if (wParam == 127 && GetKeyState(VK_CONTROL) & 0x8000) { // ctrl-backspace
			DWORD start, end;
			SendMessage(hwnd, EM_GETSEL, (WPARAM)&end, (LPARAM) (PDWORD) NULL);
			SendMessage(hwnd, WM_KEYDOWN, VK_LEFT, 0);
			SendMessage(hwnd, EM_GETSEL, (WPARAM)&start, (LPARAM) (PDWORD) NULL);
			WCHAR *text = GetWindowTextUcs(hwnd);
			memmove(text + start, text + end, sizeof(WCHAR) * (mir_wstrlen(text) + 1 - end));
			SetWindowTextUcs(hwnd, text);
			SAFE_FREE(&text);
			SendMessage(hwnd, EM_SETSEL, start, start);
			SendMessage(GetParent(hwnd), WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(hwnd), EN_CHANGE), (LPARAM) hwnd);
			return 0;
		}
		break;
	}
	return mir_callNextSubclass(hwnd, MessageEditSubclassProc, msg, wParam, lParam);
}

struct SetXStatusData
{
	CIcqProto* ppro;
	BYTE   bAction;
	BYTE   bXStatus;
	MCONTACT hContact;
	HANDLE hEvent;
	DWORD  iEvent;
	int    countdown;
	char*  okButtonFormat;
};

struct InitXStatusData
{
	CIcqProto* ppro;
	BYTE   bAction;
	BYTE   bXStatus;
	char*  szXStatusName;
	char*  szXStatusMsg;
	MCONTACT hContact;
};

#define HM_PROTOACK (WM_USER+10)
static INT_PTR CALLBACK SetXStatusDlgProc(HWND hwndDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	SetXStatusData *dat = (SetXStatusData*)GetWindowLongPtr(hwndDlg, GWLP_USERDATA);
	char str[MAX_PATH];

	switch(message) {
	case HM_PROTOACK:
		{
			ACKDATA *ack = (ACKDATA*)lParam;
			if (ack->type != ICQACKTYPE_XSTATUS_RESPONSE) break;	
			if (ack->hContact != dat->hContact) break;
			if ((DWORD)ack->hProcess != dat->iEvent) break;

			ShowDlgItem(hwndDlg, IDC_RETRXSTATUS, SW_HIDE);
			ShowDlgItem(hwndDlg, IDC_XMSG, SW_SHOW);
			ShowDlgItem(hwndDlg, IDC_XTITLE, SW_SHOW);
			SetDlgItemText(hwndDlg,IDOK,TranslateT("Close"));
			UnhookEvent(dat->hEvent); dat->hEvent = NULL;
			char *szText = dat->ppro->getSettingStringUtf(dat->hContact, DBSETTING_XSTATUS_NAME, "");
			SetDlgItemTextUtf(hwndDlg, IDC_XTITLE, szText);
			SAFE_FREE(&szText);
			szText = dat->ppro->getSettingStringUtf(dat->hContact, DBSETTING_XSTATUS_MSG, "");
			SetDlgItemTextUtf(hwndDlg, IDC_XMSG, szText);
			SAFE_FREE(&szText);
		}
		break;

	case WM_INITDIALOG:
		{
			InitXStatusData *init = (InitXStatusData*)lParam;

			TranslateDialogDefault(hwndDlg);
			dat = (SetXStatusData*)SAFE_MALLOC(sizeof(SetXStatusData));
			dat->ppro = init->ppro;
			SetWindowLongPtr(hwndDlg,GWLP_USERDATA,(LONG_PTR)dat);
			dat->bAction = init->bAction;

			if (!init->bAction) { // set our xStatus
				dat->bXStatus = init->bXStatus;
				SendDlgItemMessage(hwndDlg, IDC_XMSG, EM_LIMITTEXT, 1024, 0);
				mir_subclassWindow( GetDlgItem(hwndDlg,IDC_XMSG), MessageEditSubclassProc);
				SetDlgItemTextUtf(hwndDlg, IDC_XMSG, init->szXStatusMsg);

				if (dat->ppro->m_bXStatusEnabled) { // custom status enabled, prepare title edit
					SendDlgItemMessage(hwndDlg, IDC_XTITLE, EM_LIMITTEXT, 256, 0);
					mir_subclassWindow( GetDlgItem(hwndDlg,IDC_XTITLE), MessageEditSubclassProc);
					SetDlgItemTextUtf(hwndDlg, IDC_XTITLE, init->szXStatusName);
				}
				else { // only moods enabled, hide title, resize message edit control
					ShowDlgItem(hwndDlg, IDC_XTITLE_STATIC, SW_HIDE);
					ShowDlgItem(hwndDlg, IDC_XTITLE, SW_HIDE);
					MoveDlgItem(hwndDlg, IDC_XMSG_STATIC, 5, 0, 179, 8);
					MoveDlgItem(hwndDlg, IDC_XMSG, 5, 9, 179, 65);
				}

				dat->okButtonFormat = GetDlgItemTextUtf(hwndDlg,IDOK);
				dat->countdown = 5;
				SendMessage(hwndDlg, WM_TIMER, 0, 0);
				SetTimer(hwndDlg,1,1000,0);
			}
			else { // retrieve contact's xStatus
				dat->hContact = init->hContact;
				dat->bXStatus = dat->ppro->getContactXStatus(dat->hContact);
				dat->okButtonFormat = NULL;
				SendDlgItemMessage(hwndDlg, IDC_XTITLE, EM_SETREADONLY, 1, 0);
				SendDlgItemMessage(hwndDlg, IDC_XMSG, EM_SETREADONLY, 1, 0);

				if (dat->ppro->CheckContactCapabilities(dat->hContact, CAPF_XSTATUS) && !dat->ppro->getByte("XStatusAuto", DEFAULT_XSTATUS_AUTO)) {
					SetDlgItemText(hwndDlg,IDOK,TranslateT("Cancel"));
					dat->hEvent = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_PROTOACK);
					ShowDlgItem(hwndDlg, IDC_RETRXSTATUS, SW_SHOW);
					ShowDlgItem(hwndDlg, IDC_XMSG, SW_HIDE);
					ShowDlgItem(hwndDlg, IDC_XTITLE, SW_HIDE);
					dat->iEvent = dat->ppro->requestXStatusDetails(dat->hContact, FALSE);
				}
				else {
					SetDlgItemText(hwndDlg,IDOK,TranslateT("Close"));
					dat->hEvent = NULL;
					char *szText = dat->ppro->getSettingStringUtf(dat->hContact, DBSETTING_XSTATUS_NAME, "");
					SetDlgItemTextUtf(hwndDlg, IDC_XTITLE, szText);
					SAFE_FREE(&szText);

					// only for clients supporting just moods and not custom status
					if (dat->ppro->CheckContactCapabilities(dat->hContact, CAPF_STATUS_MOOD) && !dat->ppro->CheckContactCapabilities(dat->hContact, CAPF_XSTATUS)) {
						szText = dat->ppro->getSettingStringUtf(dat->hContact, DBSETTING_STATUS_NOTE, "");
						// hide title, resize message edit control
						ShowDlgItem(hwndDlg, IDC_XTITLE_STATIC, SW_HIDE);
						ShowDlgItem(hwndDlg, IDC_XTITLE, SW_HIDE);
						MoveDlgItem(hwndDlg, IDC_XMSG_STATIC, 5, 0, 179, 8);
						MoveDlgItem(hwndDlg, IDC_XMSG, 5, 9, 179, 65);
					}
					else szText = dat->ppro->getSettingStringUtf(dat->hContact, DBSETTING_XSTATUS_MSG, "");

					SetDlgItemTextUtf(hwndDlg, IDC_XMSG, szText);
					SAFE_FREE(&szText);
				}
			}

			if (dat->bXStatus) {
				SendMessage(hwndDlg, WM_SETICON, ICON_BIG,   (LPARAM)dat->ppro->getXStatusIcon(dat->bXStatus, LR_SHARED | LR_BIGICON));
				SendMessage(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)dat->ppro->getXStatusIcon(dat->bXStatus, LR_SHARED));
			}

			char buf[MAX_PATH];
			char *format = GetWindowTextUtf(hwndDlg);

			mir_snprintf(str, SIZEOF(str), format, dat->bXStatus?ICQTranslateUtfStatic(nameXStatus[dat->bXStatus-1], buf, MAX_PATH):"");
			SetWindowTextUtf(hwndDlg, str);
			SAFE_FREE(&format);
		}
		return TRUE;

	case WM_TIMER:
		if(dat->countdown == -1) {
			DestroyWindow(hwndDlg); 
			break;
		}

		mir_snprintf(str,SIZEOF(str),dat->okButtonFormat,dat->countdown);
		SetDlgItemTextUtf(hwndDlg,IDOK,str);
		dat->countdown--;
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam)) {
		case IDOK:
			DestroyWindow(hwndDlg);
			break;
		
		case IDC_XTITLE:
		case IDC_XMSG:
			if (!dat->bAction) { // set our xStatus
				KillTimer(hwndDlg,1);
				SetDlgItemText(hwndDlg,IDOK,TranslateT("OK"));
			}
		}
		break;

	case WM_DESTROY:
		if (!dat->bAction) { // set our xStatus
			char szSetting[64];

			dat->ppro->setByte(DBSETTING_XSTATUS_ID, dat->bXStatus);
			char *szValue = GetDlgItemTextUtf(hwndDlg,IDC_XMSG);
			mir_snprintf(szSetting, SIZEOF(szSetting), "XStatus%dMsg", dat->bXStatus);
			db_set_utf(NULL, dat->ppro->m_szModuleName, szSetting, szValue);
			db_set_utf(NULL, dat->ppro->m_szModuleName, DBSETTING_XSTATUS_MSG, szValue);
			SAFE_FREE(&szValue);

			if (dat->ppro->m_bXStatusEnabled) {
				szValue = GetDlgItemTextUtf(hwndDlg,IDC_XTITLE);
				mir_snprintf(szSetting, SIZEOF(szSetting), "XStatus%dName", dat->bXStatus);
				db_set_utf(NULL, dat->ppro->m_szModuleName, szSetting, szValue);
				db_set_utf(NULL, dat->ppro->m_szModuleName, DBSETTING_XSTATUS_NAME, szValue);
				SAFE_FREE(&szValue);

				if (dat->bXStatus) {
					Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_BIG, 0));
					Skin_ReleaseIcon((HICON)SendMessage(hwndDlg, WM_GETICON, ICON_SMALL, 0));
				}
			}
			dat->ppro->updateServerCustomStatus(TRUE);
		}
		if (dat->hEvent) UnhookEvent(dat->hEvent);
		SAFE_FREE(&dat->okButtonFormat);
		SetWindowLongPtr(hwndDlg, GWLP_USERDATA, 0);
		SAFE_FREE((void**)&dat);
		break;

	case WM_CLOSE:
		DestroyWindow(hwndDlg);
		break;
	}
	return FALSE;
}

void CIcqProto::setXStatusEx(BYTE bXStatus, BYTE bQuiet)
{
	BYTE bOldXStatus = getByte(DBSETTING_XSTATUS_ID, 0);

	if (!m_bHideXStatusUI) {
		CLISTMENUITEM mi = { sizeof(mi) };
		if (bOldXStatus <= XSTATUS_COUNT) {
			mi.flags = CMIM_FLAGS;
			Menu_ModifyItem(hXStatusItems[bOldXStatus], &mi);
		}

		mi.flags = CMIM_FLAGS | CMIF_CHECKED;
		Menu_ModifyItem(hXStatusItems[bXStatus], &mi);
	}

	if (bXStatus) {
		char szSetting[64];
		char str[MAX_PATH];
		char *szName = NULL, *szMsg = NULL;

		if (m_bXStatusEnabled) {
			mir_snprintf(szSetting, SIZEOF(szSetting), "XStatus%dName", bXStatus);
			szName = getSettingStringUtf(NULL, szSetting, ICQTranslateUtfStatic(nameXStatus[bXStatus-1], str, MAX_PATH));
		}
		mir_snprintf(szSetting, SIZEOF(szSetting), "XStatus%dMsg", bXStatus);
		szMsg = getSettingStringUtf(NULL, szSetting, "");

		mir_snprintf(szSetting, SIZEOF(szSetting), "XStatus%dStat", bXStatus);
		if (!bQuiet && !getByte(szSetting, 0)) {
			InitXStatusData init;
			init.ppro = this;
			init.bAction = 0; // set
			init.bXStatus = bXStatus;
			init.szXStatusName = szName;
			init.szXStatusMsg = szMsg;
			CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SETXSTATUS), NULL, SetXStatusDlgProc, (LPARAM)&init);
		}
		else {
			setByte(DBSETTING_XSTATUS_ID, bXStatus);
			if (m_bXStatusEnabled)
				db_set_utf(NULL, m_szModuleName, DBSETTING_XSTATUS_NAME, szName);
			db_set_utf(NULL, m_szModuleName, DBSETTING_XSTATUS_MSG, szMsg);

			updateServerCustomStatus(TRUE);
		}
		SAFE_FREE(&szName);
		SAFE_FREE(&szMsg);
	}
	else {
		setByte(DBSETTING_XSTATUS_ID, bXStatus);
		delSetting(DBSETTING_XSTATUS_NAME);
		delSetting(DBSETTING_XSTATUS_MSG);

		updateServerCustomStatus(TRUE);
	}
}

INT_PTR CIcqProto::menuXStatus(WPARAM, LPARAM, LPARAM fParam)
{
	setXStatusEx((BYTE)fParam, 0);
	return 0;
}

void CIcqProto::InitXStatusItems(BOOL bAllowStatus)
{
	size_t len = mir_strlen(m_szModuleName);
	char srvFce[MAX_PATH + 64];
	char szItem[MAX_PATH + 64];
	int bXStatusMenuBuilt = 0;

	BYTE bXStatus = getContactXStatus(NULL);

	if (!m_bXStatusEnabled && !m_bMoodsEnabled)
		return;

	if (!bAllowStatus)
		return;

	// Custom Status UI is disabled, no need to continue items' init
	if (m_bHideXStatusUI || m_bHideXStatusMenu)
		return;

	mir_snprintf(szItem, SIZEOF(szItem), Translate("%s Custom Status"), m_szModuleName);

	CLISTMENUITEM mi = { sizeof(mi) };
	mi.pszPopupName = szItem;
	mi.popupPosition = 500084000;
	mi.position = 2000040000;

	for (int i = 0; i <= XSTATUS_COUNT; i++) {
		mir_snprintf(srvFce, SIZEOF(srvFce), "%s/menuXStatus%d", m_szModuleName, i);

		mi.position++;

		if (!i) 
			bXStatusMenuBuilt = ServiceExists(srvFce);

		if (!bXStatusMenuBuilt)
			CreateProtoServiceParam(srvFce+len, &CIcqProto::menuXStatus, i);

		mi.flags = (bXStatus == i ? CMIF_CHECKED : 0);
		mi.icolibItem = i ? hXStatusIcons[i-1] : NULL;
		mi.pszName = i ? (char*)nameXStatus[i-1] : (char *)LPGEN("None");
		mi.pszService = srvFce;
		mi.pszContactOwner = m_szModuleName;

		hXStatusItems[i] = Menu_AddStatusMenuItem(&mi);
		Menu_ShowItem(hXStatusItems[i], !(m_bHideXStatusUI || m_bHideXStatusMenu));
	}
}

void InitXStatusIcons()
{
	TCHAR lib[2*MAX_PATH] = {0};

	SKINICONDESC sid = { sizeof(sid) };
	sid.pszSection = "Protocols/" ICQ_PROTOCOL_NAME "/"LPGEN("Custom Status");
	sid.flags = SIDF_PATH_TCHAR;
	sid.ptszDefaultFile = InitXStatusIconLibrary(lib, SIZEOF(lib));

	for (int i = 0; i < XSTATUS_COUNT; i++) {
		char szTemp[100];
		mir_snprintf(szTemp, SIZEOF(szTemp), "icq_xstatus%d", i);
		sid.pszName = szTemp;
		sid.pszDescription = (LPSTR)nameXStatus[i];
		sid.iDefaultIndex = -(IDI_XSTATUS1+i);
		hXStatusIcons[i] = Skin_AddIcon(&sid);
	}

	// initialize arrays for CList custom status icons
	memset(bXStatusCListIconsValid, 0, sizeof(bXStatusCListIconsValid));
	memset(hXStatusCListIcons, -1, sizeof(hXStatusCListIcons));
}

INT_PTR CIcqProto::ShowXStatusDetails(WPARAM hContact, LPARAM)
{
	InitXStatusData init;
	init.ppro = this;
	init.bAction = 1; // retrieve
	init.hContact = hContact;
	CreateDialogParam(hInst, MAKEINTRESOURCE(IDD_SETXSTATUS), NULL, SetXStatusDlgProc, (LPARAM)&init);

	return 0;
}

INT_PTR CIcqProto::SetXStatusEx(WPARAM, LPARAM lParam)
{
	CUSTOM_STATUS *pData = (CUSTOM_STATUS*)lParam;

	if (!m_bXStatusEnabled && !m_bMoodsEnabled) return 1;

	if (pData->cbSize < sizeof(CUSTOM_STATUS)) return 1; // Failure

	if (pData->flags & CSSF_MASK_STATUS) { // set custom status
		int status = *pData->status;

		if (status >= 0 && status <= XSTATUS_COUNT)
			setXStatusEx((BYTE)status, 1);
		else
			return 1; // Failure
	}

	if (pData->flags & (CSSF_MASK_NAME | CSSF_MASK_MESSAGE)) {
		BYTE status = getContactXStatus(NULL);

		if (!status) return 1; // Failure

		if (m_bXStatusEnabled && (pData->flags & CSSF_MASK_NAME)) { // set custom status name
			if (pData->flags & CSSF_UNICODE)
				setTString(DBSETTING_XSTATUS_NAME, pData->pwszName);
			else
				setString(DBSETTING_XSTATUS_NAME, pData->pszName);
		}
		if (pData->flags & CSSF_MASK_MESSAGE) { // set custom status message
			if (pData->flags & CSSF_UNICODE)
				setTString(DBSETTING_XSTATUS_MSG, pData->pwszMessage);
			else
				setString(DBSETTING_XSTATUS_MSG, pData->pszMessage);

			// update status note if used for custom status message
			updateServerCustomStatus(FALSE);
		}
	}

	// hide menu items + contact menu item
	if (pData->flags & CSSF_DISABLE_UI)
		m_bHideXStatusUI = (*pData->wParam) ? 0 : 1;

	// hide menu items only
	if (pData->flags & CSSF_DISABLE_MENU)
		m_bHideXStatusMenu = (*pData->wParam) ? 0 : 1;

	return 0; // Success
}

INT_PTR CIcqProto::GetXStatusEx(WPARAM hContact, LPARAM lParam)
{
	if (!m_bXStatusEnabled && !m_bMoodsEnabled)
		return 1;

	CUSTOM_STATUS *pData = (CUSTOM_STATUS*)lParam;
	if (pData->cbSize < sizeof(CUSTOM_STATUS))
		return 1; // Failure

	// fill status member
	if (pData->flags & CSSF_MASK_STATUS)
		*pData->status = getContactXStatus(hContact);
	
	// fill status name member
	if (pData->flags & CSSF_MASK_NAME) {
		if (pData->flags & CSSF_DEFAULT_NAME) {
			int status = (pData->wParam == NULL) ? getContactXStatus(hContact) : *pData->wParam;
			if (status < 1 || status > XSTATUS_COUNT)
				return 1; // Failure

			if (pData->flags & CSSF_UNICODE) {
				char *text = (char*)nameXStatus[status-1];
				MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, text, -1, pData->pwszName, MAX_PATH);
			}
			else strcpy(pData->pszName, (char*)nameXStatus[status-1]);
		}
		else { // moods does not support status title
			if (!m_bXStatusEnabled) return 1;

			if (pData->flags & CSSF_UNICODE) {
				char *str = getSettingStringUtf(hContact, DBSETTING_XSTATUS_NAME, "");
				WCHAR *wstr = make_unicode_string(str);

				wcscpy(pData->pwszName, wstr);
				SAFE_FREE(&str);
				SAFE_FREE(&wstr);
			}
			else {
				DBVARIANT dbv;
				if ( !getString(hContact, DBSETTING_XSTATUS_NAME, &dbv) && dbv.pszVal) {
					strcpy(pData->pszName, dbv.pszVal);
					db_free(&dbv);
				}
				else strcpy(pData->pszName, "");
			}
		}
	}

	if (pData->flags & CSSF_MASK_MESSAGE) { // fill status message member
		if (pData->flags & CSSF_UNICODE) {
			char *str = getSettingStringUtf(hContact, CheckContactCapabilities(hContact, CAPF_STATUS_MOOD) ? DBSETTING_STATUS_NOTE : DBSETTING_XSTATUS_MSG, "");
			WCHAR *wstr = make_unicode_string(str);

			wcscpy(pData->pwszMessage, wstr);
			SAFE_FREE(&str);
			SAFE_FREE(&wstr);
		}
		else {
			DBVARIANT dbv = {0};

			if (!getString(hContact, CheckContactCapabilities(hContact, CAPF_STATUS_MOOD) ? DBSETTING_STATUS_NOTE : DBSETTING_XSTATUS_MSG, &dbv) && dbv.pszVal)
				strcpy(pData->pszMessage, dbv.pszVal);
			else
				strcpy(pData->pszMessage, "");

			db_free(&dbv);
		}
	}

	if (pData->flags & CSSF_DISABLE_UI)
		if (pData->wParam)
			*pData->wParam = !m_bHideXStatusUI;

	if (pData->flags & CSSF_DISABLE_MENU)
		if (pData->wParam)
			*pData->wParam = !m_bHideXStatusMenu;

	if (pData->flags & CSSF_STATUSES_COUNT)
		if (pData->wParam)
			*pData->wParam = XSTATUS_COUNT;

	if (pData->flags & CSSF_STR_SIZES) {
		DBVARIANT dbv = {DBVT_DELETED};

		if (pData->wParam) {
			if (m_bXStatusEnabled && !getString(hContact, DBSETTING_XSTATUS_NAME, &dbv))
				*pData->wParam = mir_strlen(dbv.pszVal);
			else
				*pData->wParam = 0;
			db_free(&dbv);
		}

		if (pData->lParam) {
			if (!getString(hContact, CheckContactCapabilities(hContact, CAPF_STATUS_MOOD) ? DBSETTING_STATUS_NOTE : DBSETTING_XSTATUS_MSG, &dbv))
				*pData->lParam = mir_strlen(dbv.pszVal);
			else
				*pData->lParam = 0;
			db_free(&dbv);
		}
	}

	return 0; // Success
}

INT_PTR CIcqProto::GetXStatusIcon(WPARAM wParam, LPARAM lParam)
{
	if (!m_bXStatusEnabled && !m_bMoodsEnabled)
		return 0;

	if (!wParam)
		wParam = getContactXStatus(NULL);

	if (wParam >= 1 && wParam <= XSTATUS_COUNT)
		return (INT_PTR)getXStatusIcon((BYTE)wParam, lParam);

	return 0;
}

INT_PTR CIcqProto::RequestXStatusDetails(WPARAM hContact, LPARAM)
{
	if (!m_bXStatusEnabled)
		return 0;

	// user has xstatus, no auto-retrieve details, valid contact, request details
	if (hContact && !getByte("XStatusAuto", DEFAULT_XSTATUS_AUTO) && getContactXStatus(hContact) && CheckContactCapabilities(hContact, CAPF_XSTATUS))
		return requestXStatusDetails(hContact, TRUE);

	return 0;
}

INT_PTR CIcqProto::RequestAdvStatusIconIdx(WPARAM wParam, LPARAM)
{
	if (!m_bXStatusEnabled && !m_bMoodsEnabled)
		return -1;

	BYTE bXStatus = getContactXStatus(wParam);
	if (bXStatus) {
		if (!bXStatusCListIconsValid[bXStatus-1]) { // adding icon
			int idx = hXStatusCListIcons[bXStatus-1]; 
			HIMAGELIST hCListImageList = (HIMAGELIST)CallService(MS_CLIST_GETICONSIMAGELIST,0,0);
			if (hCListImageList) {
				HICON hXStatusIcon = getXStatusIcon(bXStatus, LR_SHARED);

				if (idx > 0)
					ImageList_ReplaceIcon(hCListImageList, idx, hXStatusIcon);
				else
					hXStatusCListIcons[bXStatus-1] = ImageList_AddIcon(hCListImageList, hXStatusIcon);
				// mark icon index in the array as valid
				bXStatusCListIconsValid[bXStatus-1] = TRUE;

				Skin_ReleaseIcon(hXStatusIcon);
			}		
		}

		int idx = bXStatusCListIconsValid[bXStatus-1] ? hXStatusCListIcons[bXStatus-1] : -1;
		if (idx > 0)
			return (idx & 0xFFFF) << 16;
	}
	return -1;
}

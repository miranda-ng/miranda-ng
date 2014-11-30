// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
//
// Copyright © 2000-2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001-2002 Jon Keating, Richard Hughes
// Copyright © 2002-2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004-2010 Joe Kucera
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
//  Provides capability & signature based client detection
// -----------------------------------------------------------------------------

#include "icqoscar.h"

const capstr capShortCaps = {0x09, 0x46, 0x00, 0x00, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00}; // CAP_AIM_BUDDYICON

static const char* makeClientVersion(char *szBuf, const char *szClient, unsigned v1, unsigned v2, unsigned v3, unsigned v4)
{
	if (v4)
		mir_snprintf(szBuf, 64, "%s%u.%u.%u.%u", szClient, v1, v2, v3, v4);
	else if (v3)
		mir_snprintf(szBuf, 64, "%s%u.%u.%u", szClient, v1, v2, v3);
	else
		mir_snprintf(szBuf, 64, "%s%u.%u", szClient, v1, v2);

	return szBuf;
}

static void verToStr(char *szStr, int v)
{
	char szVer[64];

	makeClientVersion(szVer, "", (v>>24)&0x7F, (v>>16)&0xFF, (v>>8)&0xFF, v&0xFF);
	strcat(szStr, szVer);
	if (v&0x80000000) strcat(szStr, " alpha");
}

static char* MirandaVersionToStringEx(char* szStr, int bUnicode, const char* szPlug, int v, int m)
{
	if (!v) // this is not Miranda
		return NULL;

	strcpy(szStr, "Miranda IM ");

	if (!m && v == 1)
		verToStr(szStr, 0x80010200);
	else if (!m && (v&0x7FFFFFFF) <= 0x030301)
		verToStr(szStr, v);
	else {
		if (m) {
			verToStr(szStr, m);
			strcat(szStr, " ");
		}
		if (bUnicode)
			strcat(szStr, "Unicode ");

		strcat(szStr, "(");
		strcat(szStr, szPlug);
		strcat(szStr, " v");
		verToStr(szStr, v);
		strcat(szStr, ")");
	}

	return szStr;
}

char* MirandaModToString(char* szStr, capstr* capId, int bUnicode, const char* szModName)
{ // decode icqj mod version
	char* szClient;
	DWORD mver = (*capId)[0x4] << 0x18 | (*capId)[0x5] << 0x10 | (*capId)[0x6] << 8 | (*capId)[0x7];
	DWORD iver = (*capId)[0x8] << 0x18 | (*capId)[0x9] << 0x10 | (*capId)[0xA] << 8 | (*capId)[0xB];
	DWORD scode = (*capId)[0xC] << 0x18 | (*capId)[0xD] << 0x10 | (*capId)[0xE] << 8 | (*capId)[0xF];

	szClient = MirandaVersionToStringEx(szStr, bUnicode, szModName, iver, mver);
	if (scode == 0x5AFEC0DE)
	{
		strcat(szClient, " + SecureIM");
	}
	return szClient;
}

const capstr capMirandaIm = {'M', 'i', 'r', 'a', 'n', 'd', 'a', 'M', 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capMirandaNg = {'M', 'i', 'r', 'a', 'n', 'd', 'a', 'N', 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capIcqJs7    = {'i', 'c', 'q', 'j', ' ', 'S', 'e', 'c', 'u', 'r', 'e', ' ', 'I', 'M', 0, 0};
const capstr capIcqJSin   = {'s', 'i', 'n', 'j', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Miranda ICQJ S!N
const capstr capIcqJp     = {'i', 'c', 'q', 'p', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capAimOscar  = {'M', 'i', 'r', 'a', 'n', 'd', 'a', 'A', 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capMimMobile = {'M', 'i', 'r', 'a', 'n', 'd', 'a', 'M', 'o', 'b', 'i', 'l', 'e', 0, 0, 0};
const capstr capMimPack   = {'M', 'I', 'M', '/', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // Custom Miranda Pack
const capstr capTrillian  = {0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34, 0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x09};
const capstr capTrilCrypt = {0xf2, 0xe7, 0xc7, 0xf4, 0xfe, 0xad, 0x4d, 0xfb, 0xb2, 0x35, 0x36, 0x79, 0x8b, 0xdf, 0x00, 0x00};
const capstr capSim       = {'S', 'I', 'M', ' ', 'c', 'l', 'i', 'e', 'n', 't', ' ', ' ', 0, 0, 0, 0};
const capstr capSimOld    = {0x97, 0xb1, 0x27, 0x51, 0x24, 0x3c, 0x43, 0x34, 0xad, 0x22, 0xd6, 0xab, 0xf7, 0x3f, 0x14, 0x00};
const capstr capLicq      = {'L', 'i', 'c', 'q', ' ', 'c', 'l', 'i', 'e', 'n', 't', ' ', 0, 0, 0, 0};
const capstr capKopete    = {'K', 'o', 'p', 'e', 't', 'e', ' ', 'I', 'C', 'Q', ' ', ' ', 0, 0, 0, 0};
const capstr capmIcq      = {'m', 'I', 'C', 'Q', ' ', 0xA9, ' ', 'R', '.', 'K', '.', ' ', 0, 0, 0, 0};
const capstr capClimm     = {'c', 'l', 'i', 'm', 'm', 0xA9, ' ', 'R', '.', 'K', '.', ' ', 0, 0, 0, 0};
const capstr capAndRQ     = {'&', 'R', 'Q', 'i', 'n', 's', 'i', 'd', 'e', 0, 0, 0, 0, 0, 0, 0};
const capstr capRAndQ     = {'R', '&', 'Q', 'i', 'n', 's', 'i', 'd', 'e', 0, 0, 0, 0, 0, 0, 0};
const capstr capIMadering = {'I', 'M', 'a', 'd', 'e', 'r', 'i', 'n', 'g', ' ', 'C', 'l', 'i', 'e', 'n', 't'};
const capstr capmChat     = {'m', 'C', 'h', 'a', 't', ' ', 'i', 'c', 'q', ' ', 0, 0, 0, 0, 0, 0};
const capstr capJimm      = {'J', 'i', 'm', 'm', ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capCorePager = {'C', 'O', 'R', 'E', ' ', 'P', 'a', 'g', 'e', 'r', 0, 0, 0, 0, 0, 0};
const capstr capDiChat    = {'D', '[', 'i', ']', 'C', 'h', 'a', 't', ' ', 0, 0, 0, 0, 0, 0, 0};
const capstr capVmIcq     = {'V', 'm', 'I', 'C', 'Q', ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capSmapeR    = {'S', 'm', 'a', 'p', 'e', 'r', ' ', 0, 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capAnastasia = {0x44, 0xE5, 0xBF, 0xCE, 0xB0, 0x96, 0xE5, 0x47, 0xBD, 0x65, 0xEF, 0xD6, 0xA3, 0x7E, 0x36, 0x02};
const capstr capPalmJicq  = {'J', 'I', 'C', 'Q', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capInluxMsgr = {0xA7, 0xE4, 0x0A, 0x96, 0xB3, 0xA0, 0x47, 0x9A, 0xB8, 0x45, 0xC9, 0xE4, 0x67, 0xC5, 0x6B, 0x1F};
const capstr capYapp      = {'Y', 'a', 'p', 'p', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capMipClient = {0x4d, 0x49, 0x50, 0x20, 0x43, 0x6c, 0x69, 0x65, 0x6e, 0x74, 0x20, 0x76, 0x00, 0x00, 0x00, 0x00};
const capstr capPigeon    = {'P', 'I', 'G', 'E', 'O', 'N', '!', 0, 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capDigsbyBeta= {0x09, 0x46, 0x01, 0x05, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x45, 0x53, 0x54, 0x00};
const capstr capDigsby    = {'d', 'i', 'g', 's', 'b', 'y', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capJapp      = {0x6a, 0x61, 0x70, 0x70, 0xa9, 0x20, 0x62, 0x79, 0x20, 0x53, 0x65, 0x72, 0x67, 0x6f, 0x00, 0x00};
const capstr capNaim      = {0xFF, 0xFF, 0xFF, 0xFF, 'n', 'a', 'i', 'm', 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capCitron    = {0x09, 0x19, 0x19, 0x82, 0xde, 0xad, 0xbe, 0xef, 0xca, 0xfe, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
const capstr capQip       = {0x56, 0x3F, 0xC8, 0x09, 0x0B, 0x6F, 0x41, 'Q', 'I', 'P', ' ', '2', '0', '0', '5', 'a'};
const capstr capQipPDA    = {0x56, 0x3F, 0xC8, 0x09, 0x0B, 0x6F, 0x41, 'Q', 'I', 'P', ' ', ' ', ' ', ' ', ' ', '!'};
const capstr capQipSymbian= {0x51, 0xAD, 0xD1, 0x90, 0x72, 0x04, 0x47, 0x3D, 0xA1, 0xA1, 0x49, 0xF4, 0xA3, 0x97, 0xA4, 0x1F};
const capstr capQipIphone = {0x60, 0xDE, 0x5C, 0x8A, 0xDF, 0x8C, 0x4E, 0x1D, 0xA4, 0xC8, 0xBC, 0x3B, 0xD9, 0x79, 0x4D, 0xD8};
const capstr capQipMobile = {0xB0, 0x82, 0x62, 0xF6, 0x7F, 0x7C, 0x45, 0x61, 0xAD, 0xC1, 0x1C, 0x6D, 0x75, 0x70, 0x5E, 0xC5};
const capstr capQipInfium = {0x7C, 0x73, 0x75, 0x02, 0xC3, 0xBE, 0x4F, 0x3E, 0xA6, 0x9F, 0x01, 0x53, 0x13, 0x43, 0x1E, 0x1A};
const capstr capQip2010   = {0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, 0x0A, 0x03, 0x0B, 0x04, 0x01, 0x53, 0x00, 0x00, 0x00, 0x00};
const capstr capQip2012   = {0x7F, 0x7F, 0x7C, 0x7D, 0x7E, 0x7F, 0x0A, 0x03, 0x0B, 0x04, 0x01, 0x53, 0x13, 0x43, 0x1E, 0x1A};
const capstr capIm2       = {0x74, 0xED, 0xC3, 0x36, 0x44, 0xDF, 0x48, 0x5B, 0x8B, 0x1C, 0x67, 0x1A, 0x1F, 0x86, 0x09, 0x9F}; // IM2 Ext Msg
const capstr capQutIm     = {'q', 'u', 't', 'i', 'm', 0x30, 0x2e, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const capstr capBayan     = {'b', 'a', 'y', 'a', 'n', 'I', 'C', 'Q', 0, 0, 0, 0, 0, 0, 0, 0};
const capstr capJabberJIT = {'J', 'I', 'T', ' ', 0x76, 0x2E, 0x31, 0x2E, 0x78, 0x2E, 0x78, 0x00, 0x00, 0x00, 0x00, 0x00};
const capstr capIcqKid2   = {'I', 'c', 'q', 'K', 'i', 'd', '2', 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
const capstr capWebIcqPro = {'W', 'e', 'b', 'I', 'c', 'q', 'P', 'r', 'o', ' ', 0, 0, 0, 0, 0, 0};
const capstr capJasmine   = {0x4A, 0x61, 0x73, 0x6D, 0x69, 0x6E, 0x65, 0x20, 0x76, 0x65, 0x72, 0xFF, 0x00, 0x00, 0x00, 0x00};
const capstr capMraJava   = {0x4a, 0x32, 0x4d, 0x45, 0x20, 0x6d, 0x40, 0x61, 0x67, 0x65, 0x6e, 0x74, 0x00, 0x00, 0x00, 0x00};
const capstr capMacIcq    = {0xdd, 0x16, 0xf2, 0x02, 0x84, 0xe6, 0x11, 0xd4, 0x90, 0xdb, 0x00, 0x10, 0x4b, 0x9b, 0x4b, 0x7d};
const capstr capIs2001    = {0x2e, 0x7a, 0x64, 0x75, 0xfa, 0xdf, 0x4d, 0xc8, 0x88, 0x6f, 0xea, 0x35, 0x95, 0xfd, 0xb6, 0xdf};
const capstr capIs2002    = {0x10, 0xcf, 0x40, 0xd1, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
const capstr capComm20012 = {0xa0, 0xe9, 0x3f, 0x37, 0x4c, 0x7f, 0x11, 0xd1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
const capstr capStrIcq    = {0xa0, 0xe9, 0x3f, 0x37, 0x4f, 0xe9, 0xd3, 0x11, 0xbc, 0xd2, 0x00, 0x04, 0xac, 0x96, 0xdd, 0x96};
const shortcapstr capAimIcon   = {0x13, 0x46}; // CAP_AIM_BUDDYICON
const shortcapstr capAimDirect = {0x13, 0x45}; // CAP_AIM_DIRECTIM
const shortcapstr capAimFileShare = {0x13, 0x48}; // CAP_AIM_FILE_SHARE
const shortcapstr capIcqDevils    = {0x13, 0x4C}; // CAP_DEVILS
const shortcapstr capAimSmartCaps = {0x01, 0xFF};
const shortcapstr capAimLiveVideo = {0x01, 0x01}; // CAP_AIM_LIVE_VIDEO
const shortcapstr capAimLiveAudio = {0x01, 0x04}; // CAP_AIM_LIVE_AUDIO
const shortcapstr capStatusTextAware = {0x01, 0x0A}; // CAP_HOST_STATUS_TEXT_AWARE
const capstr capIcqLiteNew= {0xc8, 0x95, 0x3a, 0x9f, 0x21, 0xf1, 0x4f, 0xaa, 0xb0, 0xb2, 0x6d, 0xe6, 0x63, 0xab, 0xf5, 0xb7};
const capstr capXtrazVideo= {0x17, 0x8C, 0x2D, 0x9B, 0xDA, 0xA5, 0x45, 0xBB, 0x8D, 0xDB, 0xF3, 0xBD, 0xBD, 0x53, 0xA1, 0x0A};
const capstr capOscarChat = {0x74, 0x8F, 0x24, 0x20, 0x62, 0x87, 0x11, 0xD1, 0x82, 0x22, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00};
const capstr capUim       = {0xA7, 0xE4, 0x0A, 0x96, 0xB3, 0xA0, 0x47, 0x9A, 0xB8, 0x45, 0xC9, 0xE4, 0x67, 0xC5, 0x6B, 0x1F};
const capstr capRambler   = {0x7E, 0x11, 0xB7, 0x78, 0xA3, 0x53, 0x49, 0x26, 0xA8, 0x02, 0x44, 0x73, 0x52, 0x08, 0xC4, 0x2A};
const capstr capAbv       = {0x00, 0xE7, 0xE0, 0xDF, 0xA9, 0xD0, 0x4F, 0xe1, 0x91, 0x62, 0xC8, 0x90, 0x9A, 0x13, 0x2A, 0x1B};
const capstr capNetvigator= {0x4C, 0x6B, 0x90, 0xA3, 0x3D, 0x2D, 0x48, 0x0E, 0x89, 0xD6, 0x2E, 0x4B, 0x2C, 0x10, 0xD9, 0x9F};
const capstr captZers     = {0xb2, 0xec, 0x8f, 0x16, 0x7c, 0x6f, 0x45, 0x1b, 0xbd, 0x79, 0xdc, 0x58, 0x49, 0x78, 0x88, 0xb9}; // CAP_TZERS
const capstr capSimpLite  = {0x53, 0x49, 0x4D, 0x50, 0x53, 0x49, 0x4D, 0x50, 0x53, 0x49, 0x4D, 0x50, 0x53, 0x49, 0x4D, 0x50};
const capstr capSimpPro   = {0x53, 0x49, 0x4D, 0x50, 0x5F, 0x50, 0x52, 0x4F, 0x53, 0x49, 0x4D, 0x50, 0x5F, 0x50, 0x52, 0x4F};
const capstr capIMsecure  = {'I', 'M', 's', 'e', 'c', 'u', 'r', 'e', 'C', 'p', 'h', 'r', 0x00, 0x00, 0x06, 0x01}; // ZoneLabs
const capstr capIMSecKey1 = {1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // ZoneLabs
const capstr capIMSecKey2 = {2, 2, 2, 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // ZoneLabs
const capstr capFakeHtml  = {0x01, 0x38, 0xca, 0x7b, 0x76, 0x9a, 0x49, 0x15, 0x88, 0xf2, 0x13, 0xfc, 0x00, 0x97, 0x9e, 0xa8};

const char* cliLibicq2k  = "libicq2000";
const char* cliLicqVer   = "Licq ";
const char* cliCentericq = "Centericq";
const char* cliLibicqUTF = "libicq2000 (Unicode)";
const char* cliTrillian  = "Trillian";
const char* cliTrillian4 = "Trillian Astra";
const char* cliQip       = "QIP %s";
const char* cliIM2       = "IM2";
const char* cliSpamBot   = "Spam Bot";

const char* CIcqProto::detectUserClient(MCONTACT hContact, int nIsICQ, WORD wUserClass, DWORD dwOnlineSince, const char *szCurrentClient,
										WORD wVersion, DWORD dwFT1, DWORD dwFT2, DWORD dwFT3, BYTE bDirectFlag, DWORD dwDirectCookie, DWORD dwWebPort, /* ICQ specific */
										BYTE *caps, size_t wLen, /* Client capabilities */
										BYTE *bClientId, /* Output: detected client-type */
										char *szClientBuf)
{
	LPCSTR szClient = NULL;
	int bMirandaIM = FALSE;

	*bClientId = CLID_ALTERNATIVE; // Most clients does not tick as MsgIDs

	// Is this a Miranda IM client?
	if (dwFT1 == 0xffffffff)
	{
		if (dwFT2 == 0xffffffff)
		{ // This is Gaim not Miranda
			szClient = "Gaim";
		}
		else if (!dwFT2 && wVersion == 7)
		{ // This is WebICQ not Miranda
			szClient = "WebICQ";
		}
		else if (!dwFT2 && dwFT3 == 0x3B7248ED)
		{ // And this is most probably Spam Bot
			szClient = cliSpamBot;
		}
		else {
			// Yes this is most probably Miranda, get the version info
			szClient = MirandaVersionToStringEx(szClientBuf, 0, "ICQ", dwFT2, 0);
			*bClientId = CLID_MIRANDA;
			bMirandaIM = TRUE;
		}
	}
	else if (dwFT1 == 0x7fffffff) {
		// This is Miranda with unicode core
		szClient = MirandaVersionToStringEx(szClientBuf, 1, "ICQ", dwFT2, 0);
		*bClientId = CLID_MIRANDA;
		bMirandaIM = TRUE;
	}
	else if ((dwFT1 & 0xFF7F0000) == 0x7D000000) {
		// This is probably an Licq client
		DWORD ver = dwFT1 & 0xFFFF;

		szClient = makeClientVersion(szClientBuf, cliLicqVer, ver / 1000, (ver / 10) % 100, ver % 10, 0);
		if (dwFT1 & 0x00800000)
			strcat(szClientBuf, "/SSL");
	}
	else if (dwFT1 == 0xffffff8f)
		szClient = "StrICQ";
	else if (dwFT1 == 0xffffff42)
		szClient = "mICQ";
	else if (dwFT1 == 0xffffffbe) {
		unsigned ver1 = (dwFT2>>24)&0xFF;
		unsigned ver2 = (dwFT2>>16)&0xFF;
		unsigned ver3 = (dwFT2>>8)&0xFF;

		szClient = makeClientVersion(szClientBuf, "Alicq ", ver1, ver2, ver3, 0);
	}
	else if (dwFT1 == 0xFFFFFF7F)
		szClient = "&RQ";
	else if (dwFT1 == 0xFFFFF666) {
		// this is R&Q (Rapid Edition)
		mir_snprintf(szClientBuf, 64, "R&Q %u", (unsigned)dwFT2);
		szClient = szClientBuf;
	}
	else if (dwFT1 == 0xFFFFFFAB)
		szClient = "YSM";
	else if (dwFT1 == 0x04031980)
		szClient = "vICQ";
	else if ((dwFT1 == 0x3AA773EE) && (dwFT2 == 0x3AA66380))
		szClient = cliLibicq2k;
	else if (dwFT1 == 0x3B75AC09)
		szClient = cliTrillian;
	else if (dwFT1 == 0x3BA8DBAF) { // FT2: 0x3BEB5373; FT3: 0x3BEB5262;
		if (wVersion == 2)
			szClient = "stICQ";
	}
	else if (dwFT1 == 0xFFFFFFFE && dwFT3 == 0xFFFFFFFE)
		szClient = "Jimm";
	else if (dwFT1 == 0x3FF19BEB && dwFT3 == 0x3FF19BEB)
		szClient = cliIM2;
	else if (dwFT1 == 0xDDDDEEFF && !dwFT2 && !dwFT3)
		szClient = "SmartICQ";
	else if ((dwFT1 & 0xFFFFFFF0) == 0x494D2B00 && !dwFT2 && !dwFT3)
		// last byte of FT1: (5 = Win32, 3 = SmartPhone, Pocket PC)
		szClient = "IM+";
	else if (dwFT1 == 0x3B4C4C0C && !dwFT2 && dwFT3 == 0x3B7248ed)
		szClient = "KXicq2";
	else if (dwFT1 == 0x66666666 && dwFT3 == 0x66666666) {
		// http://darkjimm.ucoz.ru/
		if (dwFT2 == 0x10000) {
			strcpy(szClientBuf, "D[i]Chat v.");
			strcat(szClientBuf, "0.1a");
		}
		else {
			makeClientVersion(szClientBuf, "D[i]Chat v.", (dwFT2 >> 8) & 0x0F, (dwFT2 >> 4) & 0x0F, 0, 0);
			if ((dwFT2 & 0x0F) == 1)
				strcat(szClientBuf, " alpha");
			else if ((dwFT2 & 0x0F) == 2)
				strcat(szClientBuf, " beta");
			else if ((dwFT2 & 0x0F) == 3)
				strcat(szClientBuf, " final");
		}
		szClient = szClientBuf;
	}
	else if (dwFT1 == dwFT2 && dwFT2 == dwFT3 && wVersion == 8) {
		if ((dwFT1 < dwOnlineSince + 3600) && (dwFT1 > (dwOnlineSince - 3600)))
			szClient = cliSpamBot;
	}
	else if (!dwFT1 && !dwFT2 && !dwFT3 && !wVersion && !wLen && dwWebPort == 0x75BB)
		szClient = cliSpamBot;
	else if (dwFT1 == 0x44F523B0 && dwFT2 == 0x44F523A6 && dwFT3 == 0x44F523A6 && wVersion == 8)
		szClient = "Virus";

	// capabilities based detection
	capstr* capId;
	char ver[10];

	if (nIsICQ && caps) {
		// check capabilities for client identification
		if (capId = MatchCapability(caps, wLen, &capMirandaIm, 8)) {
			// new Miranda Signature
			DWORD iver = (*capId)[0xC] << 0x18 | (*capId)[0xD] << 0x10 | (*capId)[0xE] << 8 | (*capId)[0xF];
			DWORD mver = (*capId)[0x8] << 0x18 | (*capId)[0x9] << 0x10 | (*capId)[0xA] << 8 | (*capId)[0xB];

			szClient = MirandaVersionToStringEx(szClientBuf, dwFT1 == 0x7fffffff, "ICQ", iver, mver);

			if (MatchCapability(caps, wLen, &capIcqJs7, 0x4)) {
				// detect mod
				strcat(szClientBuf, " (s7 & sss)");
				if (MatchCapability(caps, wLen, &capIcqJs7, 0xE))
					strcat(szClientBuf, " + SecureIM");
			}
			else if ((dwFT1 & 0x7FFFFFFF) == 0x7FFFFFFF) {
				if (MatchCapability(caps, wLen, &capMimMobile))
					strcat(szClientBuf, " (Mobile)");

				if (dwFT3 == 0x5AFEC0DE)
					strcat(szClientBuf, " + SecureIM");
			}
			*bClientId = CLID_MIRANDA;
			bMirandaIM = TRUE;
		}
		else if (capId = MatchCapability(caps, wLen, &capMirandaNg, 8)) {
			WORD v[4];
			BYTE *buf = *capId + 8;
			unpackWord(&buf, &v[0]); unpackWord(&buf, &v[1]); unpackWord(&buf, &v[2]); unpackWord(&buf, &v[3]);
			mir_snprintf(szClientBuf, MAX_PATH, "Miranda NG ICQ %d.%d.%d.%d", v[0], v[1], v[2], v[3]);

			szClient = szClientBuf;
			if ((dwFT1 & 0x7FFFFFFF) == 0x7FFFFFFF && dwFT3 == 0x5AFEC0DE)
				strcat(szClientBuf, " + SecureIM");

			*bClientId = CLID_MIRANDA;
			bMirandaIM = TRUE;
		}
		else if (capId = MatchCapability(caps, wLen, &capIcqJs7, 4)) {
			// detect newer icqj mod
			szClient = MirandaModToString(szClientBuf, capId, dwFT3 == 0x80000000, "ICQ S7 & SSS");
			bMirandaIM = TRUE;
		}
		else if (capId = MatchCapability(caps, wLen, &capIcqJSin, 4)) {
			// detect newer icqj mod
			szClient = MirandaModToString(szClientBuf, capId, dwFT3 == 0x80000000, "ICQ S!N");
			bMirandaIM = TRUE;
		}
		else if (capId = MatchCapability(caps, wLen, &capIcqJp, 4)) {
			// detect icqj plus mod
			szClient = MirandaModToString(szClientBuf, capId, dwFT3 == 0x80000000, "ICQ Plus");
			bMirandaIM = TRUE;
		}
		else if (capId = MatchCapability(caps, wLen, &capJasmine, 12)) {
			BYTE *p = (*capId) + 12;
			szClient = makeClientVersion(szClientBuf, "Jasmine IM v", p[0], p[1], p[2], p[3]);
		}
		else if (capId = MatchCapability(caps, wLen, &capMraJava, 12)) {
			unsigned ver1 = (*capId)[13];
			unsigned ver2 = (*capId)[14];

			szClient = makeClientVersion(szClientBuf, "Mail.ru Agent (Java) v", ver1, ver2, 0, 0);
		}
		else if (MatchCapability(caps, wLen, &capTrillian) || MatchCapability(caps, wLen, &capTrilCrypt)) {
			// this is Trillian, check for new versions
			if (CheckContactCapabilities(hContact, CAPF_RTF)) {
				if (CheckContactCapabilities(hContact, CAPF_OSCAR_FILE))
					szClient = cliTrillian4;
				else {
					// workaroud for a bug in Trillian - make it receive msgs, other features will not work!
					ClearContactCapabilities(hContact, CAPF_SRV_RELAY);
					szClient = "Trillian v3";
				}
			}
			else if (MatchCapability(caps, wLen, &capFakeHtml) || CheckContactCapabilities(hContact, CAPF_OSCAR_FILE))
				szClient = cliTrillian4;
			else
				szClient = cliTrillian;
		}
		else if ((capId = MatchCapability(caps, wLen, &capSimOld, 0xF)) && ((*capId)[0xF] != 0x92 && (*capId)[0xF] >= 0x20 || (*capId)[0xF] == 0)) {
			int hiVer = (((*capId)[0xF]) >> 6) - 1;
			unsigned loVer = (*capId)[0xF] & 0x1F;

			if ((hiVer < 0) || ((hiVer == 0) && (loVer == 0)))
				szClient = "Kopete";
			else
				szClient = makeClientVersion(szClientBuf, "SIM ", (unsigned)hiVer, loVer, 0, 0);
		}
		else if (capId = MatchCapability(caps, wLen, &capSim, 0xC)) {
			unsigned ver1 = (*capId)[0xC];
			unsigned ver2 = (*capId)[0xD];
			unsigned ver3 = (*capId)[0xE];
			unsigned ver4 = (*capId)[0xF];

			szClient = makeClientVersion(szClientBuf, "SIM ", ver1, ver2, ver3, ver4 & 0x0F);
			if (ver4 & 0x80)
				strcat(szClientBuf,"/Win32");
			else if (ver4 & 0x40)
				strcat(szClientBuf,"/MacOS X");
		}
		else if (capId = MatchCapability(caps, wLen, &capLicq, 0xC)) {
			unsigned ver1 = (*capId)[0xC];
			unsigned ver2 = (*capId)[0xD] % 100;
			unsigned ver3 = (*capId)[0xE];

			szClient = makeClientVersion(szClientBuf, cliLicqVer, ver1, ver2, ver3, 0);
			if ((*capId)[0xF])
				strcat(szClientBuf,"/SSL");
		}
		else if (capId = MatchCapability(caps, wLen, &capKopete, 0xC)) {
			unsigned ver1 = (*capId)[0xC];
			unsigned ver2 = (*capId)[0xD];
			unsigned ver3 = (*capId)[0xE];
			unsigned ver4 = (*capId)[0xF];

			szClient = makeClientVersion(szClientBuf, "Kopete ", ver1, ver2, ver3, ver4);
		}
		else if (capId = MatchCapability(caps, wLen, &capClimm, 0xC)) {
			unsigned ver1 = (*capId)[0xC];
			unsigned ver2 = (*capId)[0xD];
			unsigned ver3 = (*capId)[0xE];
			unsigned ver4 = (*capId)[0xF];

			szClient = makeClientVersion(szClientBuf, "climm ", ver1, ver2, ver3, ver4);
			if ((ver1 & 0x80) == 0x80)
				strcat(szClientBuf, " alpha");
			if (dwFT3 == 0x02000020)
				strcat(szClientBuf, "/Win32");
			else if (dwFT3 == 0x03000800)
				strcat(szClientBuf, "/MacOS X");
		}
		else if (capId = MatchCapability(caps, wLen, &capmIcq, 0xC)) {
			unsigned ver1 = (*capId)[0xC];
			unsigned ver2 = (*capId)[0xD];
			unsigned ver3 = (*capId)[0xE];
			unsigned ver4 = (*capId)[0xF];

			szClient = makeClientVersion(szClientBuf, "mICQ ", ver1, ver2, ver3, ver4);
			if ((ver1 & 0x80) == 0x80)
				strcat(szClientBuf, " alpha");
		}
		// IM2 v2 provides also Aim Icon cap
		else if (MatchCapability(caps, wLen, &capIm2))
			szClient = cliIM2;
		else if (capId = MatchCapability(caps, wLen, &capAndRQ, 9)) {
			unsigned ver1 = (*capId)[0xC];
			unsigned ver2 = (*capId)[0xB];
			unsigned ver3 = (*capId)[0xA];
			unsigned ver4 = (*capId)[9];

			szClient = makeClientVersion(szClientBuf, "&RQ ", ver1, ver2, ver3, ver4);
		}
		else if (capId = MatchCapability(caps, wLen, &capRAndQ, 9)) {
			unsigned ver1 = (*capId)[0xC];
			unsigned ver2 = (*capId)[0xB];
			unsigned ver3 = (*capId)[0xA];
			unsigned ver4 = (*capId)[9];

			szClient = makeClientVersion(szClientBuf, "R&Q ", ver1, ver2, ver3, ver4);
		}
		// http://imadering.com
		else if (MatchCapability(caps, wLen, &capIMadering))
			szClient = "IMadering";
		else if (MatchCapability(caps, wLen, &capQipPDA))
			szClient = "QIP PDA (Windows)";
		else if (MatchCapability(caps, wLen, &capQipSymbian))
			szClient = "QIP PDA (Symbian)";
		else if (MatchCapability(caps, wLen, &capQipIphone))
			szClient = "QIP Mobile (IPhone)";
		else if (MatchCapability(caps, wLen, &capQipMobile))
			szClient = "QIP Mobile (Java)";
		else if (MatchCapability(caps, wLen, &capQipInfium)) {

			strcpy(szClientBuf, "QIP Infium");
			if (dwFT1) {
				mir_snprintf(ver, 10, " (%d)", dwFT1);
				strcat(szClientBuf, ver);
			}
			if (dwFT2 == 0x0B)
				strcat(szClientBuf, " Beta");

			szClient = szClientBuf;
		}
		else if (MatchCapability(caps, wLen, &capQip2010, 12)) {
			strcpy(szClientBuf, "QIP 2010");
			if (dwFT1) {
				mir_snprintf(ver, 10, " (%d)", dwFT1);
				strcat(szClientBuf, ver);
			}

			szClient = szClientBuf;
		}
		else if (MatchCapability(caps, wLen, &capQip2012, 12)) {
			strcpy(szClientBuf, "QIP 2012");
			if (dwFT1) {
				mir_snprintf(ver, 10, " (%d)", dwFT1);
				strcat(szClientBuf, ver);
			}

			szClient = szClientBuf;
		}
		else if (capId = MatchCapability(caps, wLen, &capQip, 0xE)) {
			if (dwFT3 == 0x0F)
				strcpy(ver, "2005");
			else
				null_strcpy(ver, (char*)(*capId) + 11, 5);

			mir_snprintf(szClientBuf, 64, cliQip, ver);
			if (dwFT1 && dwFT2 == 0x0E) {
				mir_snprintf(ver, 10, " (%d%d%d%d)", dwFT1 >> 0x18, (dwFT1 >> 0x10) & 0xFF, (dwFT1 >> 0x08) & 0xFF, dwFT1 & 0xFF);
				strcat(szClientBuf, ver);
			}
			szClient = szClientBuf;
		}
		else if (capId = MatchCapability(caps, wLen, &capmChat, 0xA)) {
			strcpy(szClientBuf, "mChat ");
			strncat(szClientBuf, (char*)(*capId) + 0xA, 6);
			szClient = szClientBuf;
		}
		else if (capId = MatchCapability(caps, wLen, &capJimm, 5)) {
			strcpy(szClientBuf, "Jimm ");
			strncat(szClientBuf, (char*)(*capId) + 5, 11);
			szClient = szClientBuf;
		}
		// http://corepager.net.ru/index/0-2
		else if (capId = MatchCapability(caps, wLen, &capCorePager, 0xA)) {
			strcpy(szClientBuf, "CORE Pager");
			if (dwFT2 == 0x0FFFF0011 && dwFT3 == 0x1100FFFF && (dwFT1 >> 0x18)) {
				char ver[16];

				mir_snprintf(ver, 10, " %d.%d", dwFT1 >> 0x18, (dwFT1 >> 0x10) & 0xFF);
				if ((dwFT1 & 0xFF) == 0x0B)
					strcat(ver, " Beta");
				strcat(szClientBuf, ver);
			}
			szClient = szClientBuf;
		}
		// http://darkjimm.ucoz.ru/
		else if (capId = MatchCapability(caps, wLen, &capDiChat, 9)) {
			strcpy(szClientBuf, "D[i]Chat");
			strncat(szClientBuf, (char*)(*capId) + 8, 8);
			szClient = szClientBuf;
		}
		else if (MatchCapability(caps, wLen, &capMacIcq))
			szClient = "ICQ for Mac";
		else if (MatchCapability(caps, wLen, &capUim))
			szClient = "uIM";
		// http://chis.nnov.ru/anastasia
		else if (MatchCapability(caps, wLen, &capAnastasia))
			szClient = "Anastasia";
		// http://www.jsoft.ru
		else if (capId = MatchCapability(caps, wLen, &capPalmJicq, 0xC)) {
			unsigned ver1 = (*capId)[0xC];
			unsigned ver2 = (*capId)[0xD];
			unsigned ver3 = (*capId)[0xE];
			unsigned ver4 = (*capId)[0xF];

			szClient = makeClientVersion(szClientBuf, "JICQ ", ver1, ver2, ver3, ver4);
		}
		// http://www.inlusoft.com
		else if (MatchCapability(caps, wLen, &capInluxMsgr))
			szClient = "Inlux Messenger";
		// http://mip.rufon.net
		else if (capId = MatchCapability(caps, wLen, &capMipClient, 0xC)) {	
			unsigned ver1 = (*capId)[0xC];
			unsigned ver2 = (*capId)[0xD];
			unsigned ver3 = (*capId)[0xE];
			unsigned ver4 = (*capId)[0xF];

			if (ver1 < 30)
				makeClientVersion(szClientBuf, "MIP ", ver1, ver2, ver3, ver4);
			else {
				strcpy(szClientBuf, "MIP ");
				strncat(szClientBuf, (char*)(*capId) + 11, 5);
			}
			szClient = szClientBuf;
		}
		//http://mip.rufon.net - new signature
		else if (capId = MatchCapability(caps, wLen, &capMipClient, 0x04)) {
			strcpy(szClientBuf, "MIP ");
			strncat(szClientBuf, (char*)(*capId) + 4, 12);
			szClient = szClientBuf;
		}
		else if (capId = MatchCapability(caps, wLen, &capVmIcq, 0x06)) {
			strcpy(szClientBuf, "VmICQ");
			strncat(szClientBuf, (char*)(*capId) + 5, 11);
			szClient = szClientBuf;
		}
		// http://www.smape.com/smaper
		else if (capId = MatchCapability(caps, wLen, &capSmapeR, 0x07)) {
			strcpy(szClientBuf, "SmapeR");
			strncat(szClientBuf, (char*)(*capId) + 6, 10);
			szClient = szClientBuf;
		}
		// http://yapp.ru
		else if (capId = MatchCapability(caps, wLen, &capYapp, 0x04)) {	
			strcpy(szClientBuf, "Yapp! v");
			strncat(szClientBuf, (char*)(*capId) + 8, 5);
			szClient = szClientBuf;
		}
		// http://www.dibsby.com (newer builds)
		else if (MatchCapability(caps, wLen, &capDigsby, 0x06))
			szClient = "Digsby";
		// http://www.digsby.com - probably by mistake (feature detection as well)
		else if (MatchCapability(caps, wLen, &capDigsbyBeta))
			szClient = "Digsby";
		// http://www.japp.org.ua
		else if (MatchCapability(caps, wLen, &capJapp))
			szClient = "japp";
		// http://pigeon.vpro.ru
		else if (MatchCapability(caps, wLen, &capPigeon, 0x07))
			szClient = "PIGEON!";
		// http://www.qutim.org
		else if (capId = MatchCapability(caps, wLen, &capQutIm, 0x05)) {	
			if ((*capId)[0x6] == 0x2E) { // old qutim id
				unsigned ver1 = (*capId)[0x5] - 0x30;
				unsigned ver2 = (*capId)[0x7] - 0x30;

				makeClientVersion(szClientBuf, "qutIM ", ver1, ver2, 0, 0);
			}
			else { // new qutim id
				unsigned ver1 = (*capId)[0x6];
				unsigned ver2 = (*capId)[0x7];
				unsigned ver3 = (*capId)[0x8];
				unsigned ver4 = ((*capId)[0x9] << 8) || (*capId)[0xA];

				makeClientVersion(szClientBuf, "qutIM ", ver1, ver2, ver3, ver4);

				switch ((*capId)[0x5]) {
				case 'l':
					strcat(szClientBuf, "/Linux");
					break;
				case 'w':
					strcat(szClientBuf, "/Win32");
					break;
				case 'm':
					strcat(szClientBuf, "/MacOS X");
					break;
				}
			}
			szClient = szClientBuf;
		}
		// http://www.barobin.com/bayanICQ.html
		else if (capId = MatchCapability(caps, wLen, &capBayan, 8)) {
			strcpy(szClientBuf, "bayanICQ ");
			strncat(szClientBuf, (char*)(*capId) + 8, 5);
			szClient = szClientBuf;
		}
		else if (capId = MatchCapability(caps, wLen, &capJabberJIT, 0x04))
			szClient = "Jabber ICQ Transport";
		// http://sourceforge.net/projects/icqkid2
		else if (capId = MatchCapability(caps, wLen, &capIcqKid2, 0x07)) {
			unsigned ver1 = (*capId)[0x7];
			unsigned ver2 = (*capId)[0x8];
			unsigned ver3 = (*capId)[0x9];
			unsigned ver4 = (*capId)[0xA];

			szClient = makeClientVersion(szClientBuf, "IcqKid2 v", ver1, ver2, ver3, ver4);
		}
		// http://intrigue.ru/workshop/webicqpro/webicqpro.html
		else if (capId = MatchCapability(caps, wLen, &capWebIcqPro, 0x0A))
			szClient = "WebIcqPro";
		// http://www.citron-im.com
		else if (capId = MatchCapability(caps, wLen, &capCitron))
			szClient = "Citron IM";
		// try to determine which client is behind libicq2000
		else if (szClient == cliLibicq2k) {	
			if (CheckContactCapabilities(hContact, CAPF_RTF))
				szClient = cliCentericq; // centericq added rtf capability to libicq2000
			else if (CheckContactCapabilities(hContact, CAPF_UTF))
				szClient = cliLibicqUTF; // IcyJuice added unicode capability to libicq2000
			// others - like jabber transport uses unmodified library, thus cannot be detected
		}
		// THE SIGNATURE DETECTION ENDS HERE, after this only feature default will be detected
		else if (szClient == NULL) {
			// ZA mangled the version, OMG!
			if (wVersion == 8 && CheckContactCapabilities(hContact, CAPF_XTRAZ) && (MatchCapability(caps, wLen, &capIMSecKey1, 6) || MatchCapability(caps, wLen, &capIMSecKey2, 6)))
				wVersion = ICQ_VERSION;

			// try to determine 2001-2003 versions
			if (wVersion == 8 && (MatchCapability(caps, wLen, &capComm20012) || CheckContactCapabilities(hContact, CAPF_SRV_RELAY))) {
				if (MatchCapability(caps, wLen, &capIs2001)) {
					if (!dwFT1 && !dwFT2 && !dwFT3) {
						if (CheckContactCapabilities(hContact, CAPF_RTF))
							szClient = "TICQClient"; // possibly also older GnomeICU
						else
							szClient = "ICQ for Pocket PC";
					}
					else {
						*bClientId = CLID_GENERIC;
						szClient = "ICQ 2001";
					}
				}
				else if (MatchCapability(caps, wLen, &capIs2002)) {
					*bClientId = CLID_GENERIC;
					szClient = "ICQ 2002";
				}
				else if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY | CAPF_UTF | CAPF_RTF)) {
					if (!dwFT1 && !dwFT2 && !dwFT3) {
						if (!dwWebPort)
							szClient = "GnomeICU 0.99.5+"; // no other way
						else
							szClient = "IC@";
					}
					else {
						*bClientId = CLID_GENERIC;
						szClient = "ICQ 2002/2003a";
					}
				}
				// libpurple (e.g. Pidgin 2.7.x)
				else if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY | CAPF_UTF | CAPF_TYPING | CAPF_XTRAZ) && 
					MatchCapability(caps, wLen, &capOscarChat) && MatchShortCapability(caps, wLen, &capAimIcon) &&
					MatchCapability(caps, wLen, &capFakeHtml))
				{	
					if (MatchShortCapability(caps, wLen, &capAimDirect))
						szClient = "libpurple";
					else
						szClient = "Meebo";
				}
				else if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY | CAPF_UTF | CAPF_TYPING)) {
					if (!dwFT1 && !dwFT2 && !dwFT3)
						szClient = "PreludeICQ";
				}
			}
			else if (wVersion == 8) {
				if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_TYPING) && MatchShortCapability(caps, wLen, &capAimIcon) && MatchShortCapability(caps, wLen, &capAimDirect))
					szClient = "imo.im"; //https://imo.im/ - Web IM
			}
			// try to determine lite versions
			else if (wVersion >= 9) {
				if (CheckContactCapabilities(hContact, CAPF_XTRAZ)) {
					*bClientId = CLID_GENERIC;
					if (CheckContactCapabilities(hContact, CAPF_OSCAR_FILE)) {
						if (MatchCapability(caps, wLen, &captZers)) {
							// capable of tZers ?
							if ( MatchCapability(caps, wLen, &capIcqLiteNew) && MatchShortCapability(caps, wLen, &capStatusTextAware) &&
								MatchShortCapability(caps, wLen, &capAimLiveVideo) && MatchShortCapability(caps, wLen, &capAimLiveAudio))
							{
								strcpy(szClientBuf, "ICQ 7");
							}
							else if (MatchCapability(caps, wLen, &capFakeHtml)) {
								if (MatchShortCapability(caps, wLen, &capAimLiveVideo) && MatchShortCapability(caps, wLen, &capAimLiveAudio)) {
									strcpy(szClientBuf, "ICQ 6");
									*bClientId = CLID_ICQ6;
								}
								else if (CheckContactCapabilities(hContact, CAPF_RTF) && !CheckContactCapabilities(hContact, CAPF_CONTACTS) && MatchShortCapability(caps, wLen, &capIcqDevils)) {
									strcpy(szClientBuf, "Qnext v4"); // finally handles SRV_RELAY correctly
									*bClientId = CLID_ALTERNATIVE;
								}
							}
							else strcpy(szClientBuf, "icq5.1");
						}
						else strcpy(szClientBuf, "icq5");

						if (MatchCapability(caps, wLen, &capRambler))
							strcat(szClientBuf, " (Rambler)");
						else if (MatchCapability(caps, wLen, &capAbv))
							strcat(szClientBuf, " (Abv)");
						else if (MatchCapability(caps, wLen, &capNetvigator))
							strcat(szClientBuf, " (Netvigator)");

						szClient = szClientBuf;
					}
					else if (!CheckContactCapabilities(hContact, CAPF_ICQDIRECT)) {
						*bClientId = CLID_ALTERNATIVE;
						if (CheckContactCapabilities(hContact, CAPF_RTF)) {
							// most probably Qnext - try to make that shit at least receiving our msgs
							ClearContactCapabilities(hContact, CAPF_SRV_RELAY);
							debugLogA("Forcing simple messages (QNext client).");
							szClient = "Qnext";
						}
						else if (CheckContactCapabilities(hContact, CAPF_TYPING) && MatchCapability(caps, wLen, &captZers) && MatchCapability(caps, wLen, &capFakeHtml)) {
							if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY | CAPF_UTF) && MatchShortCapability(caps, wLen, &capAimLiveAudio))
								szClient = "Mail.ru Agent (PC)";
							else
								szClient = "Fring";
						}
						else szClient = "pyICQ";
					}
					else szClient = "ICQ Lite v4";
				}
				else if (MatchCapability(caps, wLen, &capIcqLiteNew))
					szClient = "ICQ Lite"; // the new ICQ Lite based on ICQ6
				else if (!CheckContactCapabilities(hContact, CAPF_ICQDIRECT)) {
					if (MatchCapability(caps, wLen, &capFakeHtml) && MatchCapability(caps, wLen, &capOscarChat) && MatchShortCapability(caps, wLen, &capAimSmartCaps))
						szClient = cliTrillian4;
					else if (CheckContactCapabilities(hContact, CAPF_UTF) && !CheckContactCapabilities(hContact, CAPF_RTF))
						szClient = "pyICQ";
				}
			}
			else if (wVersion == 7) {
				if (CheckContactCapabilities(hContact, CAPF_RTF))
					szClient = "GnomeICU"; // this is an exception
				else if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY)) {
					if (!dwFT1 && !dwFT2 && !dwFT3)
						szClient = "&RQ";
					else {
						*bClientId = CLID_GENERIC;
						szClient = "ICQ 2000";
					}
				}
				else if (CheckContactCapabilities(hContact, CAPF_UTF)) {
					if (CheckContactCapabilities(hContact, CAPF_TYPING))
						szClient = "Icq2Go! (Java)";
					else if (wUserClass & CLASS_WIRELESS)
						szClient = "Pocket Web 1&1";
					else
						szClient = "Icq2Go!";
				}
			}
			else if (wVersion == 0xA) {
				if ( !CheckContactCapabilities(hContact, CAPF_RTF) && !CheckContactCapabilities(hContact, CAPF_UTF)) {
					// this is bad, but we must do it - try to detect QNext
					ClearContactCapabilities(hContact, CAPF_SRV_RELAY);
					debugLogA("Forcing simple messages (QNext client).");
					szClient = "Qnext";
				}
				else if (!CheckContactCapabilities(hContact, CAPF_RTF) && CheckContactCapabilities(hContact, CAPF_UTF) && !dwFT1 && !dwFT2 && !dwFT3)
					// not really good, but no other option
					szClient = "NanoICQ";
			}
			else if (wVersion == 0xB) {
				if (CheckContactCapabilities(hContact, CAPF_XTRAZ | CAPF_SRV_RELAY | CAPF_TYPING | CAPF_UTF) && MatchShortCapability(caps, wLen, &capIcqDevils))
					szClient = "Mail.ru Agent (Symbian)";
			}
			else if (wVersion == 0) {
				// capability footprint based detection - not really reliable
				if (!dwFT1 && !dwFT2 && !dwFT3 && !dwWebPort && !dwDirectCookie) {
					// DC info is empty
					if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_CONTACTS | CAPF_XTRAZ | CAPF_OSCAR_FILE))
						szClient = "ICQ 8";
					else if (CheckContactCapabilities(hContact, CAPF_TYPING) && MatchCapability(caps, wLen, &capIs2001) &&
						MatchCapability(caps, wLen, &capIs2002) && MatchCapability(caps, wLen, &capComm20012))
						szClient = cliSpamBot;
					else if (MatchShortCapability(caps, wLen, &capAimIcon) && MatchShortCapability(caps, wLen, &capAimDirect) &&
						CheckContactCapabilities(hContact, CAPF_OSCAR_FILE | CAPF_UTF))
					{	// detect libgaim/libpurple versions
						if (CheckContactCapabilities(hContact, CAPF_SRV_RELAY))
							szClient = "Adium X"; // yeah, AFAIK only Adium has this fixed
						else if (CheckContactCapabilities(hContact, CAPF_TYPING))
							szClient = "libpurple";
						else
							szClient = "libgaim";
					}
					else if (MatchShortCapability(caps, wLen, &capAimIcon) && MatchShortCapability(caps, wLen, &capAimDirect) &&
						MatchCapability(caps, wLen, &capOscarChat) && CheckContactCapabilities(hContact, CAPF_OSCAR_FILE) && wLen == 0x40)
						szClient = "libgaim"; // Gaim 1.5.1 most probably
					else if (CheckContactCapabilities(hContact, CAPF_OSCAR_FILE) && MatchCapability(caps, wLen, &capOscarChat) && wLen == 0x20)
						szClient = "Easy Message";
					else if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_TYPING) && MatchShortCapability(caps, wLen, &capAimIcon) && MatchCapability(caps, wLen, &capOscarChat) && wLen == 0x40)
						szClient = "Meebo";
					else if (CheckContactCapabilities(hContact, CAPF_UTF) && MatchShortCapability(caps, wLen, &capAimIcon) && wLen == 0x20)
						szClient = "PyICQ-t Jabber Transport";
					else if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_XTRAZ) && MatchShortCapability(caps, wLen, &capAimIcon) && MatchCapability(caps, wLen, &capXtrazVideo))
						szClient = "PyICQ-t Jabber Transport";
					else if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_SRV_RELAY | CAPF_ICQDIRECT | CAPF_TYPING) && wLen == 0x40)
						szClient = "Agile Messenger"; // Smartphone 2002
					else if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_SRV_RELAY | CAPF_ICQDIRECT | CAPF_OSCAR_FILE) && MatchShortCapability(caps, wLen, &capAimFileShare))
						szClient = "Slick"; // http://lonelycatgames.com/?app=slick
					else if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_SRV_RELAY | CAPF_OSCAR_FILE | CAPF_CONTACTS) && MatchShortCapability(caps, wLen, &capAimFileShare) && MatchShortCapability(caps, wLen, &capAimIcon))
						szClient = "Digsby"; // http://www.digsby.com
					else if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_SRV_RELAY | CAPF_CONTACTS) && MatchShortCapability(caps, wLen, &capAimIcon) && MatchCapability(caps, wLen, &capFakeHtml))
						szClient = "mundu IM"; // http://messenger.mundu.com
					else if (CheckContactCapabilities(hContact, CAPF_UTF | CAPF_OSCAR_FILE) && MatchCapability(caps, wLen, &capOscarChat)) {
						if (CheckContactCapabilities(hContact, CAPF_TYPING))
							szClient = "eBuddy"; // http://www.ebuddy.com
						else
							szClient = "eBuddy (Mobile)";
					}
					else if (CheckContactCapabilities(hContact, CAPF_CONTACTS | CAPF_OSCAR_FILE) && MatchShortCapability(caps, wLen, &capAimIcon) && MatchShortCapability(caps, wLen, &capAimDirect) && MatchCapability(caps, wLen, &capOscarChat))
						szClient = "IloveIM"; //http://www.iloveim.com/

				}
			}
		}
	}
	else if (!nIsICQ) {
		// detect AIM clients
		if (caps) {
			if (capId = MatchCapability(caps, wLen, &capAimOscar, 8)) {
				// AimOscar Signature
				DWORD aver = (*capId)[0xC] << 0x18 | (*capId)[0xD] << 0x10 | (*capId)[0xE] << 8 | (*capId)[0xF];
				DWORD mver = (*capId)[0x8] << 0x18 | (*capId)[0x9] << 0x10 | (*capId)[0xA] << 8 | (*capId)[0xB];

				szClient = MirandaVersionToStringEx(szClientBuf, 0, "AimOscar", aver, mver);
				bMirandaIM = TRUE;
			}
			else if (capId = MatchCapability(caps, wLen, &capSim, 0xC)) {
				// Sim is universal
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xD];
				unsigned ver3 = (*capId)[0xE];

				szClient = makeClientVersion(szClientBuf, "SIM ", ver1, ver2, ver3, 0);
				if ((*capId)[0xF] & 0x80)
					strcat(szClientBuf,"/Win32");
				else if ((*capId)[0xF] & 0x40)
					strcat(szClientBuf,"/MacOS X");
			}
			else if (capId = MatchCapability(caps, wLen, &capKopete, 0xC)) {
				unsigned ver1 = (*capId)[0xC];
				unsigned ver2 = (*capId)[0xD];
				unsigned ver3 = (*capId)[0xE];
				unsigned ver4 = (*capId)[0xF];

				szClient = makeClientVersion(szClientBuf, "Kopete ", ver1, ver2, ver3, ver4);
			}
			else if (MatchCapability(caps, wLen, &capIm2))
				// IM2 extensions
				szClient = cliIM2;
			else if (MatchCapability(caps, wLen, &capNaim, 0x8))
				szClient = "naim";
			// http://www.dibsby.com
			else if (MatchCapability(caps, wLen, &capDigsby, 0x06) || MatchCapability(caps, wLen, &capDigsbyBeta))
				szClient = "Digsby";
			else if (MatchShortCapability(caps, wLen, &capAimIcon) && MatchCapability(caps, wLen, &capOscarChat) && 
				CheckContactCapabilities(hContact, CAPF_UTF | CAPF_TYPING) && wLen == 0x40)
				szClient = "Meebo";
			// libpurple (e.g. Pidgin 2.7.x)
			else if (wLen == 0x90 && CheckContactCapabilities(hContact, CAPF_SRV_RELAY | CAPF_UTF | CAPF_TYPING | CAPF_XTRAZ) && 
				      MatchCapability(caps, wLen, &capOscarChat) && MatchShortCapability(caps, wLen, &capAimIcon) && 
				      MatchShortCapability(caps, wLen, &capAimDirect) && MatchCapability(caps, wLen, &capFakeHtml))
				szClient = "libpurple";
			// libpurple - Meebo (without DirectIM and OFT)
			else if (wLen == 0x70 && CheckContactCapabilities(hContact, CAPF_SRV_RELAY | CAPF_UTF | CAPF_TYPING | CAPF_XTRAZ) && 
				      MatchCapability(caps, wLen, &capOscarChat) && MatchShortCapability(caps, wLen, &capAimIcon) && 
				      MatchCapability(caps, wLen, &capFakeHtml))
				szClient = "Meebo";
			else
				szClient = "AIM";
		}
		else if(wUserClass & CLASS_WIRELESS)
			szClient = "AIM (Mobile)";
		else
			szClient = "AIM";
	}

	// custom miranda packs
	if (caps && bMirandaIM) {
		capstr* capId;
		if (capId = MatchCapability(caps, wLen, &capMimPack, 4)) {
			char szPack[16];
			mir_snprintf(szPack, 16, " [%.12s]", (*capId)+4);

			// make sure client string is not constant
			if (szClient != szClientBuf) {
				strcpy(szClientBuf, szClient);
				szClient = szClientBuf;
			}

			strcat(szClientBuf, szPack);
		}
	}

	BOOL bClientDetected = (szClient != NULL);

	// client detection failed, provide default clients
	if (!szClient) {
		*bClientId = CLID_GENERIC;
		switch (wVersion) {
		case 6:
			szClient = "ICQ99";
			break;
		case 7:
			szClient = "ICQ 2000/Icq2Go";
			break;
		case 8:
			szClient = "ICQ 2001-2003a";
			break;
		case 9:
			szClient = "ICQ Lite";
			break;
		case 0xA:
			szClient = "ICQ 2003b";
		}
	}

	if (szClient) {
		char *szExtra = NULL;

		if (MatchCapability(caps, wLen, &capSimpLite))
			szExtra = " + SimpLite";
		else if (MatchCapability(caps, wLen, &capSimpPro))
			szExtra = " + SimpPro";
		else if (MatchCapability(caps, wLen, &capIMsecure) || MatchCapability(caps, wLen, &capIMSecKey1, 6) || MatchCapability(caps, wLen, &capIMSecKey2, 6))
			szExtra = " + IMsecure";

		if (szExtra) {
			if (szClient != szClientBuf) {
				strcpy(szClientBuf, szClient);
				szClient = szClientBuf;
			}
			strcat(szClientBuf, szExtra);
		}
	}

	// Log the detection result if it has changed or contact just logged on...
	if (!szCurrentClient || strcmpnull(szCurrentClient, szClient)) {
		if (bClientDetected)
			debugLogA("Client identified as %s", szClient);
		else
			debugLogA("No client identification, put default ICQ client for protocol.");
	}

	return szClient;
}

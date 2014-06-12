// ---------------------------------------------------------------------------80
//                ICQ plugin for Miranda Instant Messenger
//                ________________________________________
// 
// Copyright © 2000,2001 Richard Hughes, Roland Rabien, Tristan Van de Vreede
// Copyright © 2001,2002 Jon Keating, Richard Hughes
// Copyright © 2002,2003,2004 Martin Öberg, Sam Kothari, Robert Rainwater
// Copyright © 2004,2005,2006,2007 Joe Kucera
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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.


#ifndef M_ICQPLUS_H__
#define M_ICQPLUS_H__ 1

/*#define PS_ICQP_SERVER_IGNORE			"/ServerIgnore"

#define MS_INCOGNITO_REQUEST	"/IncognitoRequest"


#define MS_TZER_DIALOG "/TzerDialog"
//Send tzer 
//wParam=(WPARAM)hContact;
//lParam=(int)tZer
#define MS_SEND_TZER "/SendtZer"



*/

#define MAX_CAPNAME 64
typedef struct
{
	int cbSize;
	char caps[0x10];
	HANDLE hIcon;
	char name[MAX_CAPNAME];
} ICQ_CUSTOMCAP;

// Add a custom icq capability.
// wParam = 0;
// lParam = (LPARAM)(ICQ_CUSTOMCAP *)&icqCustomCap;
#define PS_ICQ_ADDCAPABILITY "/IcqAddCapability"

// Check if capability is supportes. Only icqCustomCap.caps does matter.
// wParam = (WPARAM)(HANDLE)hContact;
// lParam = (LPARAM)(ICQ_CUSTOMCAP *)&icqCustomCap;
// returns non-zero if capability is supported
#define PS_ICQ_CHECKCAPABILITY "/IcqCheckCapability"
/*

#ifndef ICQ_NOHELPERS
static __inline void IcqBuildMirandaCap(ICQ_CUSTOMCAP *icqCustomCap, const char *name, HICON hIcon, const char *id)
{
	if (!icqCustomCap) return;
	icqCustomCap->cbSize = sizeof(*icqCustomCap);
	icqCustomCap->hIcon = hIcon;
	strncpy(icqCustomCap->name, name, sizeof(icqCustomCap->name))[sizeof(icqCustomCap->name)-1]=0;
	memset(icqCustomCap->caps, 0, 0x10);
	strncpy(icqCustomCap->caps+0, "Miranda/", 8);
	strncpy(icqCustomCap->caps+8, id, 8);
}
#endif*/

/* usage:
char icqCustomCapStr[0x10] = {...};
ICQ_CUSTOMCAP icqCustomCap;
icqCustomCap.cbSize = sizeof(icqCustomCap);
memcpy(icqCustomCap.caps, icqCustomCapStr, 0x10);
lstrcpy(icqCustomCap.name, "Custom Name");
CallProtoService("ICQ", PS_ICQ_ADDCAPABILITY, 0, (LPARAM)&icqCustomCap);
*/

//check user status via ASD 
//wParam=(WPARAM)hContact;

//#define MS_ICQ_GET_USER_STATUS "/IcqGetUserStatus"


#endif // M_ICQPLUS_H__
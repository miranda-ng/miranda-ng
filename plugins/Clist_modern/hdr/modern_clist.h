/*

Miranda IM: the free IM client for Microsoft* Windows*

Copyright 2000-2008 Miranda ICQ/IM project, 
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
#pragma once

#ifndef _CLIST_H_
#define _CLIST_H_

void LoadContactTree(void);
int ExtIconFromStatusMode(HANDLE hContact, const char *szProto,int status);
HTREEITEM GetTreeItemByHContact(HANDLE hContact);
void cli_ChangeContactIcon(HANDLE hContact,int iIcon,int add);
int GetContactInfosForSort(HANDLE hContact,char **Proto,TCHAR **Name,int *Status);

typedef HMONITOR ( WINAPI *pfnMyMonitorFromPoint )(POINT,DWORD);
extern pfnMyMonitorFromPoint MyMonitorFromPoint;

typedef HMONITOR( WINAPI *pfnMyMonitorFromWindow) (HWND, DWORD);
extern pfnMyMonitorFromWindow MyMonitorFromWindow;

typedef BOOL(WINAPI *pfnMyGetMonitorInfo) (HMONITOR, LPMONITORINFO);
extern pfnMyGetMonitorInfo MyGetMonitorInfo;

class CSmileyString
{	 
public:
	SortedList*	plText;
	int			iMaxSmileyHeight;

	CSmileyString()   : plText( NULL ), iMaxSmileyHeight( 0 ) {};
	CSmileyString( const CSmileyString& ssIn )
	{	
		_CopySmileyList( ssIn.plText );
		iMaxSmileyHeight = ssIn.iMaxSmileyHeight;
	}

	CSmileyString& operator= ( const CSmileyString& ssIn )
	{
		DestroySmileyList();
		_CopySmileyList( ssIn.plText );
		iMaxSmileyHeight = ssIn.iMaxSmileyHeight;
		return *this;
	}

	~CSmileyString()
	{
		DestroySmileyList();
	}

	void ReplaceSmileys(struct SHORTDATA *dat, struct tag_DNCE * pdnce, TCHAR *szText, BOOL replace_smileys);

	/**	Destroy smiley list */ 
	void DestroySmileyList();
	/**  Copy Smiley List */
	void _CopySmileyList( SortedList *plInput );
	void AddListeningToIcon(struct SHORTDATA *dat, struct tag_DNCE * pdnce, TCHAR *szText, BOOL replace_smileys);

};

struct tag_DNCE{
	HANDLE	m_cache_hContact;
	TCHAR*	m_cache_tcsName;
#if defined( _UNICODE )
	char*	m_cache_szName;
#endif
	TCHAR*	m_cache_tcsGroup;
	int		m_cache_nHidden;
	int		m_cache_nNoHiddenOffline;

	char*	m_cache_cszProto;
	boolean m_cache_bProtoNotExists;
	int		m_cache_nStatus;
	int		m_cache_nHiddenSubcontact;

	int		i;
	int		ApparentMode;
	int		NotOnList;
	int		IdleTS;
	void*	ClcContact;
	BYTE	IsExpanded;
	boolean isUnknown;

	TCHAR	*	szSecondLineText;
	CSmileyString ssSecondLine;

	TCHAR	*	szThirdLineText;
	CSmileyString ssThirdLine;

	HANDLE	hTimeZone;
	DWORD	dwLastMsgTime;
};
typedef tag_DNCE displayNameCacheEntry,*pdisplayNameCacheEntry, *PDNCE;



typedef struct tagEXTRASLOTINFO
{
    union
    {
        TCHAR * ptszSlotName;   // one of this string should be given
        char  * pszSlotName;
    };
    char * pszSlotID;
    BOOL fUnicode;
    BYTE iSlot;               // the slot 10-16 are available, do not use
} EXTRASLOTINFO;

#define CLVM_FILTER_PROTOS			1
#define CLVM_FILTER_GROUPS			2
#define CLVM_FILTER_STATUS			4
#define CLVM_FILTER_VARIABLES		8
#define CLVM_STICKY_CONTACTS		16
#define CLVM_FILTER_STICKYSTATUS	32
#define CLVM_FILTER_LASTMSG			64
#define CLVM_FILTER_LASTMSG_OLDERTHAN 128
#define CLVM_FILTER_LASTMSG_NEWERTHAN 256

#define CLVM_PROTOGROUP_OP		1
#define CLVM_GROUPSTATUS_OP		2
#define CLVM_AUTOCLEAR			4
#define CLVM_INCLUDED_UNGROUPED 8
#define CLVM_USELASTMSG			16

#define CLVM_USEGROUPS			32
#define CLVM_DONOTUSEGROUPS		64

//changes the 'use groups' flag and call CLUI    v0.8.0.16+
//wParam=newValue
//lParam=0
//returns 0 on success, nonzero on failure
//newValue is 0 to not use gruops, 1 to use groups
//or -1 to toggle the value
#define MS_CLIST_SETUSEGROUPS  "CList/SetUseGroups"


#if defined(_UNICODE)
#define CLVM_MODULE "CLVM_W"
#else
#define CLVM_MODULE "CLVM"
#endif

#define GROUPF_SHOWOFFLINE 0x40   

#endif
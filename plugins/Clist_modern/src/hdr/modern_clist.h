/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (c) 2012-14 Miranda NG project (http://miranda-ng.org),
Copyright (c) 2000-08 Miranda ICQ/IM project,
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
HTREEITEM GetTreeItemByHContact(MCONTACT hContact);
void cli_ChangeContactIcon(MCONTACT hContact,int iIcon,int add);
int GetContactInfosForSort(MCONTACT hContact,char **Proto,TCHAR **Name,int *Status);

///////////////////////////////////////////////////////////////////////////////

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

	void ReplaceSmileys(struct SHORTDATA *dat, struct ClcCacheEntry *pdnce, TCHAR *szText, BOOL replace_smileys);

	/**	Destroy smiley list */
	void DestroySmileyList();
	/**  Copy Smiley List */
	void _CopySmileyList( SortedList *plInput );
	void AddListeningToIcon(struct SHORTDATA *dat, struct ClcCacheEntry *pdnce, TCHAR *szText, BOOL replace_smileys);

};

///////////////////////////////////////////////////////////////////////////////

struct ClcCacheEntry : public ClcCacheEntryBase
{
	int      m_cache_nNoHiddenOffline;

	int      m_cache_nStatus;
	char*    m_cache_cszProto;
	bool     m_bProtoNotExists, m_bIsSub;
	bool     isUnknown;

	int      ApparentMode;
	int      NotOnList;
	int      IdleTS;
	void*    ClcContact;
	BYTE     IsExpanded;

	TCHAR*   szSecondLineText;
	CSmileyString ssSecondLine;

	TCHAR*   szThirdLineText;
	CSmileyString ssThirdLine;

	HANDLE   hTimeZone;
	DWORD    dwLastMsgTime;

	void     getName(void);
	void     freeName(void);
};

///////////////////////////////////////////////////////////////////////////////

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

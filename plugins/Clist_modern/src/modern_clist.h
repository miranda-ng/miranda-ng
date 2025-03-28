/*

Miranda NG: the free IM client for Microsoft* Windows*

Copyright (C) 2012-25 Miranda NG team (https://miranda-ng.org),
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
int GetContactInfosForSort(MCONTACT hContact, char **Proto, wchar_t **Name, int *Status);

///////////////////////////////////////////////////////////////////////////////

class CSmileyString
{
public:
	SortedList*	plText;
	int			iMaxSmileyHeight;

	CSmileyString() : plText(nullptr), iMaxSmileyHeight(0) {};
	CSmileyString(const CSmileyString& ssIn)
	{
		_CopySmileyList(ssIn.plText);
		iMaxSmileyHeight = ssIn.iMaxSmileyHeight;
	}

	CSmileyString& operator= (const CSmileyString& ssIn)
	{
		DestroySmileyList();
		_CopySmileyList(ssIn.plText);
		iMaxSmileyHeight = ssIn.iMaxSmileyHeight;
		return *this;
	}

	~CSmileyString()
	{
		DestroySmileyList();
	}

	void ReplaceSmileys(ClcData *dat, ClcCacheEntry *pdnce, wchar_t *szText, BOOL replace_smileys);

	/**	Destroy smiley list */
	void DestroySmileyList();
	/**  Copy Smiley List */
	void _CopySmileyList(SortedList *plInput);
	void AddListeningToIcon(ClcData *dat, wchar_t *szText);

};

///////////////////////////////////////////////////////////////////////////////

struct ClcCacheEntry : public ClcCacheEntryBase
{
	int      m_bNoHiddenOffline;

	int      m_iStatus;
	bool     m_bIsSub;
	bool     m_bIsUnknown;

	int      NotOnList;
	int      IdleTS;
	void*    ClcContact;
	int      IsExpanded;

	wchar_t*   szSecondLineText;
	CSmileyString ssSecondLine;

	wchar_t*   szThirdLineText;
	CSmileyString ssThirdLine;

	HANDLE   hTimeZone;
	uint32_t    dwLastMsgTime, dwLastOnlineTime;

	int __forceinline getStatus() const
	{	return (this == nullptr) ? ID_STATUS_OFFLINE : m_iStatus;
	}
};

///////////////////////////////////////////////////////////////////////////////

#define CLVM_FILTER_PROTOS             0x0001
#define CLVM_FILTER_GROUPS             0x0002
#define CLVM_FILTER_STATUS             0x0004
#define CLVM_FILTER_VARIABLES          0x0008
#define CLVM_STICKY_CONTACTS           0x0010
#define CLVM_FILTER_STICKYSTATUS       0x0020
#define CLVM_FILTER_LASTMSG            0x0040
#define CLVM_FILTER_LASTMSG_OLDERTHAN  0x0080
#define CLVM_FILTER_LASTMSG_NEWERTHAN  0x0100

#define CLVM_PROTOGROUP_OP             0x0001
#define CLVM_GROUPSTATUS_OP       		0x0002
#define CLVM_AUTOCLEAR            		0x0004
#define CLVM_INCLUDED_UNGROUPED 			0x0008
#define CLVM_USELASTMSG         			0x0010

#define CLVM_USEGROUPS                 0x0020
#define CLVM_DONOTUSEGROUPS            0x0040

#define CLVM_FOLDGROUPS                0x0080
#define CLVM_UNFOLDGROUPS              0x0100

#define CLVM_HIDEEMPTYGROUPS           0x0200
#define CLVM_SHOWEMPTYGROUPS           0x0400

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

#endif

/*
Scriver

Copyright (c) 2000-09 Miranda ICQ/IM project,

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

#ifndef SRMM_SENDQUEUE_H
#define SRMM_SENDQUEUE_H

namespace SendQueue
{
	struct Item : public MZeroedObject
	{
		~Item()
		{
			mir_free(proto);
			mir_free(sendBuffer);
		}

		CMsgDialog *pDlg;
		MCONTACT hContact;
		char  *proto;
		int    hSendId;
		int    timeout;
		char  *sendBuffer;
		int    sendBufferSize;
		int    codepage;
		int    flags;
		HWND   hwndErrorDlg;
	};

	Item* CreateItem(CMsgDialog *pDlg);
	Item* FindItem(MCONTACT hContact, HANDLE hSendId);
	Item* FindOldestPendingItem(CMsgDialog *pDlg, MCONTACT hContact);

	bool RemoveItem(Item *item);
	void ReportTimeouts(CMsgDialog *pDlg);
	void ReleaseItems(CMsgDialog *pDlg);
	int  ReattachItems(CMsgDialog *pDlg, MCONTACT hContact);
	void RemoveAllItems();
	void SendItem(Item *item);
};

#endif

// ---------------------------------------------------------------------------
//                Contacts+ for Miranda Instant Messenger
//                _______________________________________
// 
// Copyright © 2002 Dominus Procellarum 
// Copyright © 2004-2008 Joe Kucera
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
//
// ---------------------------------------------------------------------------

#ifndef __RECEIVE_H
#define __RECEIVE_H


#define HM_EVENTSENT    (WM_USER+10)

#define DM_UPDATETITLE    (WM_USER+11)

#define IDI_ADDCONTACT                  210
#define IDI_USERDETAILS                 160
#define IDI_HISTORY                     174
#define IDI_DOWNARROW                   264

struct TReceivedItem {
  char* mcaUIN;
  TCHAR* mcaNick;
  ~TReceivedItem() { SAFE_FREE((void**)&mcaUIN); SAFE_FREE((void**)&mcaNick); }
  TReceivedItem() { mcaUIN = NULL; mcaNick = NULL; }
};

struct TRecvContactsData { 
  HANDLE  mhDbEvent;    // handle to recv DB event
  HANDLE  mhContact;    // from whom we received this
  HIMAGELIST mhListIcon;// icons for listview
  HMENU   mhPopup;      // popup menu for listview
  HANDLE  hHook;        // hook to event
  HANDLE  rhSearch;     // handle to uin-search
  char*   haUin;
  int     iPopupItem;
  TReceivedItem** maReceived;// received contacts
  int     cbReceived;
  TReceivedItem* AddReceivedItem();
  HICON   hIcons[4];    // icons for dialog
  TRecvContactsData(HANDLE contact) { mhContact = contact; hHook = NULL; cbReceived = 0; maReceived = NULL; haUin = NULL; };
  ~TRecvContactsData() { if (cbReceived) { for(int i=0;i<cbReceived;i++) delete maReceived[i]; SAFE_FREE((void**)&maReceived); SAFE_FREE((void**)&haUin); }; };
};

extern HANDLE ghRecvWindowList;

INT_PTR CALLBACK RecvDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

#endif /* __RECEIVE_H */
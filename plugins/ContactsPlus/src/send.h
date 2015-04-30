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

#ifndef __SEND_H
#define __SEND_H


#define TIMERID_MSGSEND   1024
#define TIMEOUT_MSGSEND   9000    //ms
#define HM_EVENTSENT      (WM_USER+10)
#define DM_ERRORDECIDED   (WM_USER+18)
#define DM_UPDATETITLE    (WM_USER+11)
#define MSGERROR_CANCEL   0
#define MSGERROR_RETRY    1
#define MSGERROR_DONE     2

#define IDI_ADDCONTACT                  210
#define IDI_USERDETAILS                 160
#define IDI_HISTORY                     174
#define IDI_DOWNARROW                   264

struct TSendProcessList {
  int Count;
  HANDLE* Items;
  mir_cs lock;
  void Add(HANDLE hProcc);
  void Remove(HANDLE hProcc);
  TSendProcessList();
  ~TSendProcessList();
};

struct TSendContactsData {  // hope uack is released automaticly, static property
  HANDLE hHook;             // hook to event
  void HookProtoAck(HWND hwndDlg);
  void UnhookProtoAck();
  MCONTACT *aContacts;        // contacts to be sent
  int nContacts;            // now many UIDs shall we send?
  void ClearContacts();
  void AddContact(MCONTACT hContact);
  MCONTACT hContact;          // to whom shall we send?
  TSendProcessList uacklist;// ackdata - necessary for errorbox
  HWND hError;              // handle of error box, if any
  void ShowErrorDlg(HWND hwndDlg, char* szMsg, bool bAllowRetry);
  int SendContactsPacket(HWND hwndDlg, MCONTACT *phContacts, int nContacts);
  int SendContacts(HWND hwndDlg);
  HICON hIcons[4];          // icons for dialog
  TSendContactsData(MCONTACT contact);
  ~TSendContactsData();
};

struct TAckData
{
  MCONTACT hContact;    // to whom was it sent
  MCONTACT* aContacts;  // obj
  int nContacts;      // how many
  TAckData(MCONTACT contact) { hContact = contact; aContacts = NULL; nContacts = 0;};
  ~TAckData() { mir_free(aContacts); }
};

typedef TAckData* PAckData; 

struct TCTSend {
  char* mcaUIN;
  char* mcaNick;
};

struct gAckItem {  // some shit here
  HANDLE hProcc;
  PAckData ackData;
  gAckItem(HANDLE procC, PAckData aData) { ackData=aData; hProcc=procC; };
  ~gAckItem() { /*delete ackData;*/ };
};

struct gAckList {
  gAckItem** Items;
  int Count;
  TAckData* Get(HANDLE hProcc) { for (int i=0; i<Count; i++) if (Items[i]->hProcc==hProcc) { return Items[i]->ackData; }; return NULL; };
  TAckData* Add(HANDLE hProcc, TAckData* ackData) { Items=(gAckItem**)mir_realloc(Items, (Count+1)*sizeof(gAckItem*)); Items[Count]=new gAckItem(hProcc, ackData); Count++; return ackData; };
  TAckData* Remove(HANDLE hProcc) { for (int i=0; i<Count; i++) if (Items[i]->hProcc==hProcc) { TAckData* data=Items[i]->ackData; delete Items[i]; memmove(Items+i, Items+i+1, (Count-i-1)*sizeof(gAckItem*)); Count--; return data; }; return NULL; };
  gAckList() { Count = 0; Items = NULL; }
  ~gAckList() { if (Count) { for (int i=0; i<Count; i++) delete Items[i]; mir_free(Items); }; }
};

extern HANDLE ghSendWindowList;
extern gAckList gaAckData;

INT_PTR CALLBACK SendDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam);


#endif /* __SEND_H */
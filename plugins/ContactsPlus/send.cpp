// --------------------------------------------------------------------------
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
// -----------------------------------------------------------------------------

#include "contacts.h"


/* TSendProcessList */

void TSendProcessList::Add(HANDLE hProcc) {
  EnterCriticalSection(&lock); 
  Items=(HANDLE*)realloc(Items, (Count+1)*sizeof(HANDLE)); 
  Items[Count]=hProcc; 
  Count++; 
  LeaveCriticalSection(&lock); 
}


void TSendProcessList::Remove(HANDLE hProcc) {
  EnterCriticalSection(&lock); 
  for (int i=0; i<Count; i++) 
    if (Items[i]==hProcc) { 
      memmove(Items+i, Items+i+1, (Count-i-1)*sizeof(HANDLE)); 
      Count--; 
      break;
    }
  LeaveCriticalSection(&lock); 
}


TSendProcessList::TSendProcessList() { 
  InitializeCriticalSection(&lock); 
  Count = 0; 
  Items = NULL; 
}


TSendProcessList::~TSendProcessList() { 
  if (Count) 
    SAFE_FREE((void**)&Items); 
  DeleteCriticalSection(&lock); 
}


/* TSendContactsData */

TSendContactsData::TSendContactsData(HANDLE contact): uacklist() {
  hContact = contact; 
  hHook = NULL; 
  hError = NULL; 
  aContacts = NULL; 
  nContacts = 0;
}


TSendContactsData::~TSendContactsData() { 
  ClearContacts();
  UnhookProtoAck(); 
}
 

void TSendContactsData::HookProtoAck(HWND hwndDlg) {
  if (!hHook) 
    hHook = HookEventMessage(ME_PROTO_ACK, hwndDlg, HM_EVENTSENT); 
};


void TSendContactsData::UnhookProtoAck() { 
  if (hHook) 
  { 
    UnhookEvent(hHook); 
    hHook=NULL; 
  }; 
}


void TSendContactsData::ShowErrorDlg(HWND hwndDlg, char* szMsg, bool bAllowRetry) {
  ShowWindow(hwndDlg, SW_SHOWNORMAL);
  EnableWindow(hwndDlg, FALSE);
  if (!hError) 
  {
    hError = CreateDialogParamT(hInst, MAKEINTRESOURCEA(IDD_MSGSENDERROR), hwndDlg, ErrorDlgProc, (LPARAM)szMsg);
    if (!bAllowRetry)
      EnableDlgItem(hError, IDOK, FALSE); // do not allow again - fatal, could not be better
  }
}


void TSendContactsData::ClearContacts() {
  if (nContacts)
    SAFE_FREE((void**)&aContacts);
  nContacts=0;
}


void TSendContactsData::AddContact(HANDLE hContact) {
  aContacts = (HANDLE*)realloc(aContacts, (nContacts+1)*sizeof(HANDLE));
  aContacts[nContacts] = hContact;
  nContacts++;
}


int TSendContactsData::SendContactsPacket(HWND hwndDlg, HANDLE *phContacts, int nContacts) {
  HANDLE hProcc = (HANDLE)SRCCallContactService(hContact, PSS_CONTACTS, MAKEWPARAM(0, nContacts), (LPARAM)phContacts);
  if (!hProcc)
  { // on trivial error - do not close dialog
    ShowErrorDlg(hwndDlg, "Contacts transfer failed!", FALSE);
    return FALSE; // Failure
  }
  TAckData* ackData = gaAckData.Add(hProcc, new TAckData(hContact));
  uacklist.Add(hProcc);
  ackData->nContacts = nContacts;
  ackData->aContacts = (HANDLE*)malloc(nContacts*sizeof(HANDLE));
  memmove(ackData->aContacts, phContacts, nContacts*sizeof(HANDLE)); // copy the array of hContact for ack array
  EnableDlgItem(hwndDlg, IDOK, FALSE);
  EnableDlgItem(hwndDlg, IDC_LIST, FALSE);

  return TRUE; // Success
}


int TSendContactsData::SendContacts(HWND hwndDlg) {
  char* szProto = GetContactProto(hContact);
  int nMaxContacts = SRCCallProtoService(szProto, PS_GETCAPS, PFLAG_MAXCONTACTSPERPACKET, (LPARAM)hContact);

  if (!nMaxContacts) {
    ShowErrorDlg(hwndDlg, "The selected contact does not support receiving contacts.", FALSE);
    return FALSE;
  }
  // hook event - we want to receive protocol acknowledgements
  HookProtoAck(hwndDlg);
  
  for (int j = 0; j < nContacts / nMaxContacts; j++ )
  { // send in packets, each of nMaxContacts contacts
    if (!SendContactsPacket(hwndDlg, aContacts + j*nMaxContacts, nMaxContacts))
      return FALSE;
  }
  if (nContacts%nMaxContacts!=0) 
  {
    if (!SendContactsPacket(hwndDlg, aContacts + nContacts/nMaxContacts*nMaxContacts, nContacts%nMaxContacts))
      return FALSE;
  }
  return TRUE;
}


/* Send Dialog Implementation */

static void ResetListOptions(HWND hwndList)
{
  COLORREF bgColour,fgColour;

  SendMessageT(hwndList,CLM_SETBKBITMAP,0,(LPARAM)(HBITMAP)NULL);
  bgColour=GetSysColor(COLOR_WINDOW);
  SendMessageT(hwndList,CLM_SETBKCOLOR,bgColour,0);
  SendMessageT(hwndList,CLM_SETGREYOUTFLAGS,0,0);
  SendMessageT(hwndList,CLM_SETLEFTMARGIN,4,0);
  SendMessageT(hwndList,CLM_SETINDENT,10,0);
  for(int i=0; i<=FONTID_MAX; i++) 
  {
    fgColour=(COLORREF)SendMessageT(hwndList,CLM_GETTEXTCOLOR,i,0);
    if(abs(GetRValue(fgColour)-GetRValue(bgColour))<10 &&
      abs(GetGValue(fgColour)-GetGValue(bgColour))<10 &&
      abs(GetBValue(fgColour)-GetBValue(bgColour))<10)
      SendMessageT(hwndList,CLM_SETTEXTCOLOR,i,GetSysColor(COLOR_WINDOWTEXT));
  }
}


static HANDLE FindNextClistContact(HWND hList, HANDLE hContact, HANDLE *phItem)
{
  HANDLE hNextContact = SRCFindNextContact(hContact);
  HANDLE hNextItem = NULL;
  
  while (hNextContact && !(hNextItem = (HANDLE)SendMessageT(hList, CLM_FINDCONTACT, (WPARAM)hNextContact,0)))
    hNextContact = SRCFindNextContact(hNextContact);

  if (phItem)
    *phItem = hNextItem;

  return hNextContact;
}


static HANDLE FindFirstClistContact(HWND hList, HANDLE *phItem)
{
  HANDLE hContact = SRCFindFirstContact();
  HANDLE hItem = (HANDLE)SendMessageT(hList, CLM_FINDCONTACT, (WPARAM)hContact, 0);

  if (hContact && !hItem)
    return FindNextClistContact(hList, hContact, phItem);
  
  if (phItem)
    *phItem = hItem;

  return hContact;
}


bool binListEvent = FALSE;

static void SetAllContactChecks(HWND hwndList, HANDLE hReceiver) // doubtful name
{
  HANDLE hContact, hItem;

  if (binListEvent) return;
  binListEvent = TRUE;
  char* szProto = GetContactProto(hReceiver); 
  if (szProto == NULL) return;

  if (CallService(MS_CLUI_GETCAPS, 0, 0) & CLUIF_HIDEEMPTYGROUPS && DBGetContactSettingByte(NULL, "CList", "HideEmptyGroups", SETTING_USEGROUPS_DEFAULT))
    SendMessageT(hwndList, CLM_SETHIDEEMPTYGROUPS, (WPARAM) TRUE, 0);
  else
    SendMessageT(hwndList, CLM_SETHIDEEMPTYGROUPS, (WPARAM) FALSE, 0);

  hContact = FindFirstClistContact(hwndList, &hItem);
  while (hContact)
  {
    char* szProto2 = GetContactProto(hContact);

    if (strcmpnull(szProto, szProto2))
    { // different protocols or protocol undefined, remove contact, useless anyway
      SendMessageT(hwndList, CLM_DELETEITEM, (WPARAM)hItem, 0);
    }
    else // otherwise uncheck
      SendMessageT(hwndList, CLM_SETCHECKMARK,(WPARAM)hItem, 0);

    hContact = FindNextClistContact(hwndList, hContact, &hItem);
  }

  binListEvent = FALSE;
}


INT_PTR CALLBACK SendDlgProc( HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  TSendContactsData* wndData = (TSendContactsData*)GetWindowLong(hwndDlg, DWLP_USER);

  switch (msg)
  {
    case WM_INITDIALOG:
    {
      TranslateDialogDefault(hwndDlg);
      SendMessageT(hwndDlg, WM_SETICON, ICON_SMALL, (LPARAM)LoadIcon(hInst, MAKEINTRESOURCE(IDI_CONTACTS)));
      ResetListOptions(GetDlgItem(hwndDlg,IDC_LIST));
      SetAllContactChecks(GetDlgItem(hwndDlg,IDC_LIST), (HANDLE)lParam);
      WindowList_Add(ghSendWindowList, hwndDlg, (HANDLE)lParam);
      wndData = new TSendContactsData((HANDLE)lParam);
      SetWindowLong(hwndDlg, DWLP_USER, (LONG)wndData);
      // new dlg init 
      wndData->hIcons[0] = InitMButton(hwndDlg, IDC_ADD, MAKEINTRESOURCEA(IDI_ADDCONTACT), "Add Contact Permanently to List");
      wndData->hIcons[1] = InitMButton(hwndDlg, IDC_DETAILS, MAKEINTRESOURCEA(IDI_USERDETAILS), "View User's Details");
      wndData->hIcons[2] = InitMButton(hwndDlg, IDC_HISTORY, MAKEINTRESOURCEA(IDI_HISTORY), "View User's History");
      wndData->hIcons[3] = InitMButton(hwndDlg, IDC_USERMENU, MAKEINTRESOURCEA(IDI_DOWNARROW), "User Menu");

      SendMessageT(hwndDlg,DM_UPDATETITLE,0,0);
      // new dialog init done
      return TRUE;
    }
        
    case WM_SETFOCUS:
      SetFocus(GetDlgItem(hwndDlg,IDC_LIST));
      break;
    
    case WM_NOTIFY:
      if (((LPNMHDR)lParam)->idFrom == IDC_LIST) 
      {
        switch (((LPNMHDR)lParam)->code)
        {
          case CLN_NEWCONTACT:
          case CLN_LISTREBUILT: // rebuild list
            if (wndData) SetAllContactChecks(GetDlgItem(hwndDlg,IDC_LIST), wndData->hContact);
          case CLN_OPTIONSCHANGED:
            ResetListOptions(GetDlgItem(hwndDlg,IDC_LIST));
            break;
        }
      }
      break;

    case WM_TIMER:
      if (wParam == TIMERID_MSGSEND) 
      {
        if (!g_SendAckSupported)
        { // old Miranda has this ack unimplemented, we need to send it by ourselves
          ACKDATA ack = {0};

          ack.cbSize = sizeof(ACKDATA);
          ack.type = ACKTYPE_CONTACTS;
          ack.result = ACKRESULT_SUCCESS;
          ack.hContact = wndData->hContact;
          while (wndData->uacklist.Count)
          { // the uack gets empty after processing all messages :)
            ack.hProcess = wndData->uacklist.Items[0];
            SendMessageT(hwndDlg, HM_EVENTSENT, NULL, (WPARAM)&ack); // this removes the ack from our array
          }
          break;
        }
        KillTimer(hwndDlg,wParam);
        wndData->ShowErrorDlg(hwndDlg, "The contacts send timed out.", TRUE);
      }
      break;

    case DM_ERRORDECIDED:
    {
      EnableWindow(hwndDlg,TRUE);
      wndData->hError = NULL;
      switch(wParam) 
      {
        case MSGERROR_CANCEL:
        {
          wndData->UnhookProtoAck();
          if (wndData->uacklist.Count)
          {
            for (int i=0; i<wndData->uacklist.Count; i++)
            {
              delete gaAckData.Remove(wndData->uacklist.Items[i]); // remove our ackdata & release structure
            }  
            SAFE_FREE((void**)&wndData->uacklist.Items);
            wndData->uacklist.Count = 0;
          }
          EnableDlgItem(hwndDlg,IDOK,TRUE);
          EnableDlgItem(hwndDlg,IDC_LIST,TRUE);
          ShowWindow(hwndDlg,SW_SHOWNORMAL);
          SetFocus(GetDlgItem(hwndDlg,IDC_LIST));
          break;
        }
        case MSGERROR_DONE:
          // contacts were delivered succesfully after timeout
          SetFocus(GetDlgItem(hwndDlg,IDC_LIST));
          wndData->UnhookProtoAck();
          break;

        case MSGERROR_RETRY:// resend timeouted packets

          for (int i=0; i<wndData->uacklist.Count; i++)
          {
            TAckData* lla = gaAckData.Remove(wndData->uacklist.Items[i]);
            HANDLE hProcc = (HANDLE)SRCCallContactService(wndData->hContact, PSS_CONTACTS, MAKEWPARAM(0, lla->nContacts), (LPARAM)lla->aContacts);

            if (!hProcc) // if fatal do not include
            {
              wndData->uacklist.Remove(wndData->uacklist.Items[i]);
              delete lla; // release the structure
              continue;
            }
            else
            { // update process code
              wndData->uacklist.Items[i] = hProcc;
              gaAckData.Add(hProcc, lla);
            }
          }// collect TAckData for our window, resend
          break;
        }
      break;
    }

    case WM_COMMAND:
    {
      if(!lParam && CallService(MS_CLIST_MENUPROCESSCOMMAND,MAKEWPARAM(LOWORD(wParam), MPCF_CONTACTMENU), (LPARAM)wndData->hContact))
        break;

      switch(LOWORD(wParam))
      {
        case IDOK:
        {
          if (!IsWindowEnabled(GetDlgItem(hwndDlg,IDOK))) break;
          HANDLE hContact, hItem;
          wndData->ClearContacts(); // do not include contacts twice

          HWND hList = GetDlgItem(hwndDlg, IDC_LIST);
          hContact = FindFirstClistContact(hList, &hItem);
          while (hContact)
          {
            if (SendMessageT(hList, CLM_GETCHECKMARK, (WPARAM)hItem, 0))
            { // build list of contacts to send
              wndData->AddContact(hContact);
            }
            hContact = FindNextClistContact(hList, hContact, &hItem);
          }
          /* send contacts */
          if (!wndData->SendContacts(hwndDlg))
            break;

          if (g_SendAckSupported)
            SetTimer(hwndDlg,TIMERID_MSGSEND,DBGetContactSettingDword(NULL,"SRMsg","MessageTimeout",TIMEOUT_MSGSEND),NULL);
          else
            SetTimer(hwndDlg,TIMERID_MSGSEND,1000,NULL); // wait one second - if no error occures

          break;
        }
        case IDCANCEL: 
        {
          DestroyWindow(hwndDlg); 
          break;
        }
        case ID_SELECTALL:
        {  // select all contacts
          HANDLE hContact, hItem;
          HWND hwndList = GetDlgItem(hwndDlg, IDC_LIST);

          hContact = FindFirstClistContact(hwndList, &hItem);
          while (hContact) {
            SendMessageT(hwndList,CLM_SETCHECKMARK,(WPARAM)hItem, 1);
            hContact = FindNextClistContact(hwndList, hContact, &hItem);
          };
          break;
        }
        case IDC_USERMENU:
        {  
          RECT rc;
          HMENU hMenu = (HMENU)CallService(MS_CLIST_MENUBUILDCONTACT,(WPARAM)wndData->hContact,0);

          GetWindowRect(GetDlgItem(hwndDlg,IDC_USERMENU),&rc);
          TrackPopupMenu(hMenu,0,rc.left,rc.bottom,0,hwndDlg,NULL);
          DestroyMenu(hMenu);
          break;
        }
        case IDC_HISTORY:
          CallService(MS_HISTORY_SHOWCONTACTHISTORY,(WPARAM)wndData->hContact,0);
          break;

        case IDC_DETAILS:
          CallService(MS_USERINFO_SHOWDIALOG,(WPARAM)wndData->hContact,0);
          break;

        case IDC_ADD:
          DialogAddContactExecute(hwndDlg, wndData->hContact);
          break;
      }
      break;
    }
    case HM_EVENTSENT:
    {
      ACKDATA *ack=(ACKDATA*)lParam;
      DBEVENTINFO dbei={0};

      if (ack->type != ACKTYPE_CONTACTS) break;

      TAckData* ackData = gaAckData.Get(ack->hProcess);

      if (ackData == NULL) break;    // on unknown hprocc go away
 
      if (ackData->hContact != ack->hContact) break; // this is not ours, strange

      if (ack->result == ACKRESULT_FAILED)
      { // some process failed, show error dialog
        KillTimer(hwndDlg, TIMERID_MSGSEND);
        wndData->ShowErrorDlg(hwndDlg, (char *)ack->lParam, TRUE);
        // ackData get used in error handling, released there
        break;
      }

      dbei.cbSize = sizeof(dbei);
      dbei.szModule = GetContactProto(ackData->hContact);
      dbei.eventType = EVENTTYPE_CONTACTS;
      dbei.flags = DBEF_SENT;
      if (g_UnicodeCore && g_Utf8EventsSupported)
        dbei.flags |= DBEF_UTF;
      dbei.timestamp = time(NULL);
      //make blob
      TCTSend* maSend = (TCTSend*)_alloca(ackData->nContacts*sizeof(TCTSend));
      ZeroMemory(maSend, ackData->nContacts*sizeof(TCTSend));
      dbei.cbBlob=0;
      char* pBlob;
      int i;
      for (i=0; i<ackData->nContacts; i++)
      { // prepare data & count size
        if (g_UnicodeCore && g_Utf8EventsSupported)
          maSend[i].mcaNick = make_utf8_string((WCHAR*)GetContactDisplayNameT(ackData->aContacts[i]));
        else
          maSend[i].mcaNick = (unsigned char*)null_strdup((char*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)ackData->aContacts[i], 0));
        maSend[i].mcaUIN = GetContactUID(ackData->aContacts[i], FALSE);
        dbei.cbBlob += (DWORD)strlennull(maSend[i].mcaUIN) + (DWORD)strlennull((char*)maSend[i].mcaNick) + 2;
      }
      dbei.pBlob = (PBYTE)_alloca(dbei.cbBlob);
      for (i=0, pBlob=(char*)dbei.pBlob; i < ackData->nContacts; i++) 
      {
        strcpy(pBlob, (char*)maSend[i].mcaNick);
        pBlob += strlennull(pBlob) + 1;
        strcpy(pBlob, maSend[i].mcaUIN);
        pBlob += strlennull(pBlob) + 1;
      }
      CallService(MS_DB_EVENT_ADD, (WPARAM)ackData->hContact,(LPARAM)&dbei);
      gaAckData.Remove(ack->hProcess); // do not release here, still needed
      wndData->uacklist.Remove(ack->hProcess); // packet confirmed
      for (i=0; i<ackData->nContacts; i++) 
      {
        SAFE_FREE((void**)&maSend[i].mcaUIN);
        SAFE_FREE((void**)&maSend[i].mcaNick);
      }
      delete ackData; // all done, release structure
      if (!wndData->uacklist.Count) 
      {
        SkinPlaySound("SentContacts");
        KillTimer(hwndDlg, TIMERID_MSGSEND);

        if (wndData->hError)
          SendMessageT(wndData->hError, DM_ERRORDECIDED, MSGERROR_DONE, 0);

        SendMessageT(hwndDlg, WM_CLOSE, 0, 0); // all packets confirmed, close the dialog
      }
      break;
    }
    
    case WM_CLOSE:
    {
      wndData->UnhookProtoAck();
      DestroyWindow(hwndDlg);
      break;
    }
    case WM_DESTROY:
    {
      int i;
      for (i = 0; i < SIZEOF(wndData->hIcons); i++)
        DestroyIcon(wndData->hIcons[i]);
      WindowList_Remove(ghSendWindowList, hwndDlg);
      delete wndData;
      break;
    }
    case WM_MEASUREITEM:
      return CallService(MS_CLIST_MENUMEASUREITEM,wParam,lParam);

    case WM_DRAWITEM:
    {  
      DrawProtocolIcon(hwndDlg, lParam, wndData->hContact);  
      return CallService(MS_CLIST_MENUDRAWITEM,wParam,lParam);
    }
    case DM_UPDATETITLE:
    {
      UpdateDialogTitle(hwndDlg, wndData?wndData->hContact:NULL, "Send Contacts to");
      if (wndData)
        UpdateDialogAddButton(hwndDlg, wndData->hContact);
      break;        
    }
  }

  return FALSE;
}


// Error Dialog

INT_PTR CALLBACK ErrorDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch(msg)
  {
  case WM_INITDIALOG:
    {
      RECT rc, rcParent;
      
      TranslateDialogDefault(hwndDlg);

      if (lParam)
      {
        WCHAR tmp[MAX_PATH];

        SetDlgItemTextT(hwndDlg, IDC_ERRORTEXT, SRCTranslateT((char*)lParam, tmp));
      }
      GetWindowRect(hwndDlg, &rc);
      GetWindowRect(GetParent(hwndDlg), &rcParent);
      SetWindowPos(hwndDlg, 0,
        (rcParent.left+rcParent.right-(rc.right-rc.left))/2,
        (rcParent.top+rcParent.bottom-(rc.bottom-rc.top))/2,
        0, 0, SWP_NOZORDER|SWP_NOSIZE);
    }
    return TRUE;
    
  case WM_COMMAND:
    switch(LOWORD(wParam)) 
    {
    case IDOK:
      SendMessageT(GetParent(hwndDlg), DM_ERRORDECIDED, MSGERROR_RETRY, 0);
      DestroyWindow(hwndDlg);
      break;

    case IDCANCEL:
      SendMessageT(GetParent(hwndDlg), DM_ERRORDECIDED, MSGERROR_CANCEL, 0);
      DestroyWindow(hwndDlg);
      break;
    }
    break;
  case DM_ERRORDECIDED:
    if (wParam!=MSGERROR_DONE) break;
    SendMessageT(GetParent(hwndDlg), DM_ERRORDECIDED, MSGERROR_DONE, 0);
    DestroyWindow(hwndDlg);
    break;
  }
  
  return FALSE;
}

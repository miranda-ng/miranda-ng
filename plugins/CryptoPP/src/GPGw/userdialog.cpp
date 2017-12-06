#include "../commonheaders.h"
#include "gpgw.h"

void RefreshListView(HWND hLV);

INT_PTR CALLBACK UserIdDialogProcedure(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  static char *keyid;
  HWND hLV = GetDlgItem(hdlg,IDC_KEYLIST);

  switch(msg)
  {
    case WM_INITDIALOG:
    {
	  int i;
   	  LVCOLUMN lvc;
	  static char *szColHdr[] = { "Key ID", "User ID" };
   	  static int iColWidth[] = { 110, 255 };
   	  keyid = (char *)lparam;

   	  memset(&lvc, 0, sizeof(LVCOLUMN));
   	  lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
   	  lvc.fmt = LVCFMT_LEFT;
   	  for (i = 0; i < 2; i++) {
   		  lvc.iSubItem = i;
   		  lvc.pszText = szColHdr[i];
   		  lvc.cx = iColWidth[i];
   		  ListView_InsertColumn(hLV, i, &lvc);
   	  }

   	  RefreshListView(hLV);

	  ListView_SetExtendedListViewStyle(hLV, ListView_GetExtendedListViewStyle(hLV) | LVS_EX_FULLROWSELECT);
      ListView_SetItemState(hLV, 0, LVIS_SELECTED|LVIS_FOCUSED, LVIS_SELECTED|LVIS_FOCUSED);
	  EnableWindow(hLV, TRUE);
	}
    break;
    case WM_COMMAND:
      switch(LOWORD(wparam))
      {
	  case IDOK:
		  ListView_GetItemText(hLV, ListView_GetNextItem(hLV, -1, LVNI_SELECTED), 0, keyid, keyidsize);
	  case IDCANCEL:
          EndDialog(hdlg, wparam);
          return TRUE;
          break;
      case IDC_REFRESH:
		  updateKeyUserIDs(publickeyuserid);
		  updateKeyUserIDs(secretkeyuserid);
		  RefreshListView(hLV);
		  break;
      }
      break;
  }

  return FALSE;
}


void RefreshListView(HWND hLV) {

      LVITEM lvi;
	  int i;

	  ListView_DeleteAllItems(hLV);
   	  memset(&lvi, 0, sizeof(LVITEM));
      lvi.mask = LVIF_TEXT;
      for (i = 0; i < getKeyUserIDCount(publickeyuserid); i++) {

			char keyid[keyidsize];
			char *userid;
    		char *keyuserid = getKeyUserID(publickeyuserid, i);

		    userid = getNextPart(keyid, keyuserid, txtidseparator);

    		lvi.iItem++;
      		lvi.pszText = keyid;
      		ListView_SetItemText(hLV, ListView_InsertItem(hLV, &lvi), 1, userid);
      }
}

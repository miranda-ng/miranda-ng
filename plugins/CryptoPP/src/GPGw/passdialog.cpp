#include "../commonheaders.h"
#include "gpgw.h"

char dlgpassphrase[passphrasesize];

INT_PTR CALLBACK PassphraseDialogProcedure(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
{
  switch(msg)
  {
    case WM_INITDIALOG:
      SetDlgItemText(hdlg, IDC_USERID, (char *)lparam);
    break;
    case WM_COMMAND:
      switch(LOWORD(wparam))
      {
        case IDOK:
          memset(dlgpassphrase, 0, sizeof(dlgpassphrase));
          GetDlgItemText(hdlg, IDC_PASSPHRASE, dlgpassphrase, SIZEOF(dlgpassphrase));
        case IDCANCEL:
          EndDialog(hdlg, wparam);
          return TRUE;
        break;
      }
    break;
  }

  return FALSE;
}


#include "commonheaders.h"

char dlgpassphrase[passphrasesize];


BOOL CALLBACK PassphraseDialogProcedure(HWND hdlg, UINT msg, WPARAM wparam, LPARAM lparam)
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
          ZeroMemory(dlgpassphrase, sizeof(dlgpassphrase));
          GetDlgItemText(hdlg, IDC_PASSPHRASE, dlgpassphrase, sizeof(dlgpassphrase));
        case IDCANCEL:
          EndDialog(hdlg, wparam);
          return TRUE;
        break;
      }
    break;
  }

  return FALSE;
}


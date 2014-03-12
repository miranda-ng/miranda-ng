{$IFNDEF M_DROPBOX}
{$DEFINE M_DROPBOX}

const
//upload file on Dropbox
//wParam = (MCONTACT)hContact
//lParam = (LPARAM)(const wchar_t*)path - full path to file
// returns file htansfer handle or NULL on failure
// returns immediately, without waiting for the send
  MS_DROPBOX_SEND_FILE:PAnsiChar = 'Dropbox/Send/File';

// notifies a caller about file send end
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(const wchar_t*)url - "\r\n" separated download link to file
  ME_DROPBOX_SEND_SUCCEEDED:PAnsiChar = 'Dropbox/Send/Succeeded';

// notifies a caller about file send failure
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(HANDLE)hProcess
  ME_DROPBOX_SEND_FAILED:PAnsiChar = 'Dropbox/Send/Failed';

{$ENDIF}

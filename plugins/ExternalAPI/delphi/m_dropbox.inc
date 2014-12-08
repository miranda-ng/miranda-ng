{$IFNDEF M_DROPBOX}
{$DEFINE M_DROPBOX}

const
// upload file on Dropbox
// wParam = (MCONTACT)hContact - NULL to send to the Dropbox contact
// lParam = (LPARAM)(const wchar_t*)path - full path to file
// returns file htansfer handle or NULL on failure
// returns immediately, without waiting for the send
// note, that you can track progress by using ME_PROTO_ACK
  MS_DROPBOX_SEND_FILE:PAnsiChar = 'Dropbox/Send/File';

// if you want to get download links of sent files
// use ME_DROPBOX_SENT hook. you'll get:
type
  TTRANSFERINFO = record
    hProcess:THANDLE; // hProcess
    status  :int;     // status of transfer. 0 on success otherwise fail
    data    :pointer; // NULL ended array of download links
  end;

const
// notifies a caller that file has been sent
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(TRANSFERINFO*)info - transfer info
  ME_DROPBOX_SENT:PAnsiChar = 'Dropbox/Sent/Event';

{$ENDIF}

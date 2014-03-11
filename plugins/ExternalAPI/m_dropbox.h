#ifndef M_DROPBOX_H_
#define M_DROPBOX_H_

//upload file on Dropbox
//wParam = (MCONTACT)hContact
//lParam = (LPARAM)(const wchar_t*)path - full path to file
// returns file htansfer handle or NULL on failure
// returns immediately, without waiting for the send
#define MS_DROPBOX_SEND_FILE     "Dropbox/Send/File"

// notifies a caller about file send end
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(const wchar_t*)url - "\r\n" separated download link to file
#define ME_DROPBOX_SEND_SUCCEEDED "Dropbox/Send/Succeeded"

// notifies a caller about file send failure
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(HANDLE)hProcess
#define ME_DROPBOX_SEND_FAILED "Dropbox/Send/Failed"

#endif //M_DROPBOX_H_
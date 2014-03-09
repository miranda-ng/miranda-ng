#ifndef M_DROPBOX_H_
#define M_DROPBOX_H_

//upload file on Dropbox
//wParam = (MCONTACT)hContact
//lParam = (LPARAM)(const char*)szPath
//returns 0 on success or nonzero on failure
//returns immediately, before the file or folder is uploaded
#define MS_DROPBOX_SEND_FILE     "Dropbox/Send/File"

//upload file on Dropbox
//wParam = (MCONTACT)hContact
//lParam = (LPARAM)(const wchar_t*)wszPath
//returns 0 on success or nonzero on failure
//returns immediately, before the file or folder is uploaded
#define MS_DROPBOX_SEND_FILEW     "Dropbox/Send/FileW"

#endif //M_DROPBOX_H_
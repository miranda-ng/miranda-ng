#ifndef M_DROPBOX_H_
#define M_DROPBOX_H_

// upload file on Dropbox
// wParam = (MCONTACT)hContact - NULL to send to the Dropbox contact
// lParam = (LPARAM)(const wchar_t*)path - full path to file
// returns file htansfer handle or NULL on failure
// returns immediately, without waiting for the send
// note, that you can track progress by using ME_PROTO_ACK
#define MS_DROPBOX_SEND_FILE     "Dropbox/Send/File"

// if you want to get download links of sent files
// use ME_DROPBOX_SENT hook. you'll get:
struct TRANSFERINFO
{
	HANDLE hProcess;	// hProcess
	int status;			// status of transfer. 0 on success otherwise fail
	char** data;		// NULL ended array of download links
};

// notifies a caller that file has been sent
// wParam = (MCONTACT)hContact
// lParam = (LPARAM)(TRANSFERINFO*)info - transfer info
#define ME_DROPBOX_SENT "Dropbox/Sent/Event"

#endif //M_DROPBOX_H_
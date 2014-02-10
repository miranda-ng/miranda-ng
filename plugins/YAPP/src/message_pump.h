#ifndef _MESSAGE_PUMP_INC
#define _MESSAGE_PUMP_INC

extern unsigned message_pump_thread_id;
void PostMPMessage(UINT msg, WPARAM, LPARAM);

#define MUM_CREATEPOPUP					(WM_USER + 0x011)
#define MUM_DELETEPOPUP					(WM_USER + 0x012)

#define MUM_NMUPDATE					(WM_USER + 0x013)
#define MUM_NMREMOVE					(WM_USER + 0x014)
#define MUM_NMAVATAR					(WM_USER + 0x015)

struct PopupData
{
	int cbSize;
	int flags; // OR of PDF_* flags below
	MCONTACT hContact;
	HICON hIcon;
	union {
		char *pszTitle;
		TCHAR *ptzTitle;
		wchar_t *pwzTitle;
	};
	union {
		char *pszText;
		TCHAR *ptzText;
		wchar_t *pwzText;
	};
	COLORREF colorBack;		// if colorBack and colorText are equal, defaults will be used
	COLORREF colorText;
	WNDPROC windowProc;		// optional custom window procedure
	int timeout;			// -1 == infinite, 0 == default, otherwise timeout in seconds
	void *opaque;

	void SetIcon(HICON);
};

// given a popup data pointer, and a handle to an event, this function
// will post a message to the message queue which will set the hwnd value
// and then set the event...so create an event, call this function and then wait on the event
// when the event is signalled, the hwnd will be valid

void InitMessagePump();
void DeinitMessagePump();

#endif

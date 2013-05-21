
struct HISTORIA
{
	DWORD noteID;
	BOOL clicked;
	BOOL again;
	struct HISTORIA *next;
	struct POPUPSQUEUE *pq;
};

struct POPUPSQUEUE
{
	HWND hWnd;
	struct POPUPSQUEUE *next;
};

#if !defined(MIID_LOTUSNOTIFY)
	#define MIID_LOTUSNOTIFY  {0x127faa08, 0x54d0, 0x4f99, { 0x8a, 0x50, 0x6b, 0x5f, 0xd8, 0x92, 0x42, 0xec } }
#endif
// {127FAA08-54D0-4f99-8A50-6B5FD89242EC}

//structure contines only LN msg id to send to popup 
typedef struct {
	DWORD id;
	char strNote[4*16]; 
} POPUPATT;

__declspec(dllexport) STATUS LNPUBLIC MainEntryPoint (void);

void ErMsgT(TCHAR* msg);

void checkthread(void*);

#pragma once
#define MS_MIROTR_ADDMIROTRMENUITEM		MODULENAME"/AddMirOTRMenuItem"
#define MS_MIROTR_MENUBUILDMIROTR		MODULENAME"/MenuBuildMirOTR"
#define MS_MIROTR_REMOVEMIROTRMENUITEM	MODULENAME"/RemoveMirOTRMenuItem"

#define CMIF_NOTNOTPRIVATE		CMIF_NOTOFFLINE
#define CMIF_NOTUNVERIFIED		CMIF_NOTONLINE
#define CMIF_NOTPRIVATE			CMIF_NOTONLIST 
#define CMIF_NOTFINISHED		CMIF_NOTOFFLIST
#define CMIF_DISABLED			0x8000

//add a new item to the MirOTR Menu
//wParam=0
//lParam=(LPARAM)(CLISTMENUITEM*)&mi
//returns a handle to the new item, or NULL on failure
//the service that is called when the item is clicked is called with
//wParam=0, lParam=hContact
//dividers are inserted every 100000 positions
//pszContactOwner is ignored for this service.
// 
// WARNING: do not use Translate(TS) for p(t)szName or p(t)szPopupName as they
// are translated by the core, which may lead to double translation.
// Use LPGEN instead which are just dummy wrappers/markers for "lpgen.pl".
typedef struct {
	int cbSize;	            //size in bytes of this structure
	union {
	  char*  pszName;      //[TRANSLATED-BY-CORE] text of the menu item
		TCHAR* ptszName;     //Unicode text of the menu item
	};
	int position;           //approx position on the menu. lower numbers go nearer the top
	HGENMENU root;			//submenu where the item will be added, NULL for root
	DWORD flags;            //set of MOMIF_* flags
	union {
		HICON hIcon;         //icon to put by the item. If this was not loaded from
						   //a resource, you can delete it straight after the call
		HANDLE icolibItem;   //set CMIF_ICONFROMICOLIB to pass this value
	};
	DWORD hotKey;           //keyboard accelerator, same as lParam of WM_HOTKEY,0 for none
	char* pszService;       //name of service to call when the item gets selected
} MIROTRMENUITEM;

void InitMirOTRMenu(void);
void UninitMirOTRMenu(void);

void ShowOTRMenu(MCONTACT hContact, POINT pt);

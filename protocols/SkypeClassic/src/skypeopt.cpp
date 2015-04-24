#include "skype.h"
#include "skypeopt.h"
#include "pthread.h"
#include "gchat.h"
#include "skypeprofile.h"

#include "uxtheme.h"
#define HAVE_UXTHEMES

#ifdef SKYPE_AUTO_DETECTION
#include "ezxml/ezxml.c"
#endif

#ifdef UNICODE
#include "utf8.h"
#endif

#pragma warning (disable: 4706) // assignment within conditional expression

// VC6 SDK defines
#ifndef BIF_SHAREABLE
#define BIF_SHAREABLE          0x8000  // sharable resources displayed (remote shares, requires BIF_USENEWUI)
#endif
#ifndef BIF_NEWDIALOGSTYLE
#define BIF_NEWDIALOGSTYLE     0x0040   // Use the new dialog layout with the ability to resize
#endif									// Caller needs to call OleInitialize() before using this API
#ifndef BIF_NONEWFOLDERBUTTON
#define BIF_NONEWFOLDERBUTTON  0x0200   // Do not add the "New Folder" button to the dialog.  Only applicable with BIF_NEWDIALOGSTYLE.
#endif


extern HINSTANCE hInst;
extern char protocol, g_szProtoName[];
extern BOOL SkypeInitialized, bProtocolSet, bIsImoproxy;

BOOL showPopup, showPopupErr, popupWindowColor, popupWindowColorErr;
unsigned int popupBackColor, popupBackColorErr;
unsigned int popupTextColor, popupTextColorErr;
int popupTimeSec, popupTimeSecErr;
POPUPDATAT InCallPopup;
POPUPDATAT ErrorPopup;

static SkypeProfile myProfile;
static HBITMAP hAvatar = NULL;

extern BOOL PopupServiceExists;
extern BOOL (WINAPI *MyEnableThemeDialogTexture)(HANDLE, DWORD);

int RegisterOptions(WPARAM wParam, LPARAM)
{
	OPTIONSDIALOGPAGE odp = { 0 };
   odp.hInstance = hInst;
   odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPTIONS);
   odp.pszGroup = "Network";
   odp.pszTitle = SKYPE_PROTONAME;
   odp.pfnDlgProc = OptionsDlgProc;
   odp.flags = ODPF_BOLDGROUPS;
   Options_AddPage(wParam, &odp);

   if(PopupServiceExists)
   {
	   odp.pszTemplate = MAKEINTRESOURCEA(IDD_OPT_POPUP);
	   odp.pszGroup = "Popups";
	   odp.pfnDlgProc = OptPopupDlgProc;
	   Options_AddPage(wParam, &odp);
   }

   return 0;
}

INT_PTR CALLBACK OptPopupDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static RECT r;

	switch ( msg ) 
	{
		case WM_INITDIALOG:
			TranslateDialogDefault( hwnd );
			// Message Popup
			popupTimeSec = db_get_dw(NULL, SKYPE_PROTONAME, "popupTimeSec", 4);
			popupTextColor = db_get_dw(NULL, SKYPE_PROTONAME, "popupTextColor", GetSysColor(COLOR_WINDOWTEXT));
			popupBackColor = db_get_dw(NULL, SKYPE_PROTONAME, "popupBackColor", GetSysColor(COLOR_BTNFACE));
			popupWindowColor = db_get_b(NULL, SKYPE_PROTONAME, "popupWindowColor", FALSE);
			showPopup = db_get_b(NULL, SKYPE_PROTONAME, "showPopup", TRUE);
			// ERROR Message Popup
			popupTimeSecErr = db_get_dw(NULL, SKYPE_PROTONAME, "popupTimeSecErr", 4);
			popupTextColorErr = db_get_dw(NULL, SKYPE_PROTONAME, "popupTextColorErr", GetSysColor(COLOR_WINDOWTEXT));
			popupBackColorErr = db_get_dw(NULL, SKYPE_PROTONAME, "popupBackColorErr", GetSysColor(COLOR_BTNFACE));
			popupWindowColorErr = db_get_b(NULL, SKYPE_PROTONAME, "popupWindowColorErr", FALSE);
			showPopupErr = db_get_b(NULL, SKYPE_PROTONAME, "showPopupErr", TRUE);

			EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORS),showPopup);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR),showPopup && ! popupWindowColor);
			EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPBACKCOLOR),showPopup && ! popupWindowColor);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR),showPopup && ! popupWindowColor);
			EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPTEXTCOLOR),showPopup && ! popupWindowColor);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIME),showPopup);
			EnableWindow(GetDlgItem(hwnd,IDC_PREVIEW),showPopup);
			EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORSERR),showPopupErr);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLORERR),showPopupErr && ! popupWindowColorErr);
			EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPBACKCOLORERR),showPopupErr && ! popupWindowColorErr);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLORERR),showPopupErr && ! popupWindowColorErr);
			EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPTEXTCOLORERR),showPopupErr && ! popupWindowColorErr);
			EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIMEERR),showPopupErr);
			EnableWindow(GetDlgItem(hwnd,IDC_PREVIEWERR),showPopupErr);
			CheckDlgButton(hwnd, IDC_POPUPINCOMING, showPopup ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd, IDC_USEWINCOLORS, popupWindowColor ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd, IDC_POPUPERROR, showPopupErr ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwnd, IDC_USEWINCOLORSERR, popupWindowColorErr ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwnd, IDC_POPUPTIME, EM_SETLIMITTEXT, 3, 0L);
			SetDlgItemInt(hwnd, IDC_POPUPTIME, popupTimeSec,FALSE);
			SendDlgItemMessage(hwnd, IDC_POPUPTIMEERR, EM_SETLIMITTEXT, 3, 0L);
			SetDlgItemInt(hwnd, IDC_POPUPTIMEERR, popupTimeSecErr,FALSE);
			SendDlgItemMessage(hwnd, IDC_POPUPBACKCOLOR, CPM_SETCOLOUR,0, popupBackColor);
			SendDlgItemMessage(hwnd, IDC_POPUPBACKCOLOR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_BTNFACE));
			SendDlgItemMessage(hwnd, IDC_POPUPTEXTCOLOR, CPM_SETCOLOUR,0, popupTextColor);
			SendDlgItemMessage(hwnd, IDC_POPUPTEXTCOLOR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_WINDOWTEXT));
			SendDlgItemMessage(hwnd, IDC_POPUPBACKCOLORERR, CPM_SETCOLOUR,0, popupBackColorErr);
			SendDlgItemMessage(hwnd, IDC_POPUPBACKCOLORERR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_BTNFACE));
			SendDlgItemMessage(hwnd, IDC_POPUPTEXTCOLORERR, CPM_SETCOLOUR,0, popupTextColorErr);
			SendDlgItemMessage(hwnd, IDC_POPUPTEXTCOLORERR, CPM_SETDEFAULTCOLOUR, 0, GetSysColor(COLOR_WINDOWTEXT));


			return TRUE;
			break;

		case WM_NOTIFY:
			switch(((LPNMHDR)lParam)->idFrom)
			{
				case 0:
					switch (((LPNMHDR)lParam)->code)
					{
						case PSN_APPLY:
							db_set_dw(NULL, SKYPE_PROTONAME, "popupBackColor", popupBackColor);
							db_set_dw(NULL, SKYPE_PROTONAME, "popupTextColor", popupTextColor);
							db_set_dw(NULL, SKYPE_PROTONAME, "popupTimeSec", popupTimeSec);
							db_set_b(NULL, SKYPE_PROTONAME, "popupWindowColor", (BYTE)popupWindowColor);
							db_set_b(NULL, SKYPE_PROTONAME, "showPopup", (BYTE)showPopup);
							db_set_dw(NULL, SKYPE_PROTONAME, "popupBackColorErr", popupBackColorErr);
							db_set_dw(NULL, SKYPE_PROTONAME, "popupTextColorErr", popupTextColorErr);
							db_set_dw(NULL, SKYPE_PROTONAME, "popupTimeSecErr", popupTimeSecErr);
							db_set_b(NULL, SKYPE_PROTONAME, "popupWindowColorErr", (BYTE)popupWindowColorErr);
							db_set_b(NULL, SKYPE_PROTONAME, "showPopupErr", (BYTE)showPopupErr);
							break;
					}
			}
			break;

		

		case WM_COMMAND:
			switch( LOWORD( wParam )) 
			{
				case IDC_PREVIEW:
				{
					MCONTACT hContact;
					TCHAR * lpzContactName;

					hContact = db_find_first();
					lpzContactName = (TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME,hContact,GCDNF_TCHAR);
					InCallPopup.lchContact = hContact;
					InCallPopup.lchIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALL));
					InCallPopup.colorBack = ! popupWindowColor ? popupBackColor : GetSysColor(COLOR_BTNFACE);
					InCallPopup.colorText = ! popupWindowColor ? popupTextColor : GetSysColor(COLOR_WINDOWTEXT);
					InCallPopup.iSeconds = popupTimeSec;
					InCallPopup.PluginData = (void *)1;
					
					mir_tstrcpy(InCallPopup.lptzText, TranslateT("Incoming Skype call"));

					mir_tstrcpy(InCallPopup.lptzContactName, lpzContactName);

					CallService(MS_POPUP_ADDPOPUPT,(WPARAM)&InCallPopup,0);


					break;
				}
				case IDC_PREVIEWERR:					
					ErrorPopup.lchContact = NULL;
					ErrorPopup.lchIcon = LoadIcon(hInst,MAKEINTRESOURCE(IDI_CALL));
					ErrorPopup.colorBack = ! popupWindowColorErr ? popupBackColorErr : GetSysColor(COLOR_BTNFACE);
					ErrorPopup.colorText = ! popupWindowColorErr ? popupTextColorErr : GetSysColor(COLOR_WINDOWTEXT);
					ErrorPopup.iSeconds = popupTimeSecErr;
					ErrorPopup.PluginData = (void *)1;
					
					mir_tstrcpy(ErrorPopup.lptzText, TranslateT("Preview error message"));

					mir_tstrcpy(ErrorPopup.lptzContactName, _T("Error Message"));


					CallService(MS_POPUP_ADDPOPUPT,(WPARAM)&ErrorPopup,0);

					break;

				case IDC_POPUPTIME:
				case IDC_POPUPTIMEERR:
				{
					BOOL Translated;
					popupTimeSec = GetDlgItemInt(hwnd,IDC_POPUPTIME,&Translated,FALSE);
					popupTimeSecErr = GetDlgItemInt(hwnd,IDC_POPUPTIMEERR,&Translated,FALSE);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				}
				case IDC_POPUPTEXTCOLOR:
				case IDC_POPUPBACKCOLOR:
				case IDC_POPUPTEXTCOLORERR:
				case IDC_POPUPBACKCOLORERR:
					popupBackColor = (UINT)SendDlgItemMessage(hwnd,IDC_POPUPBACKCOLOR,CPM_GETCOLOUR,0,0);
					popupTextColor = (UINT)SendDlgItemMessage(hwnd,IDC_POPUPTEXTCOLOR,CPM_GETCOLOUR,0,0);
					popupBackColorErr = (UINT)SendDlgItemMessage(hwnd,IDC_POPUPBACKCOLORERR,CPM_GETCOLOUR,0,0);
					popupTextColorErr = (UINT)SendDlgItemMessage(hwnd,IDC_POPUPTEXTCOLORERR,CPM_GETCOLOUR,0,0);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_USEWINCOLORS:
					popupWindowColor = (IsDlgButtonChecked(hwnd,IDC_USEWINCOLORS)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR), showPopup && ! popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPBACKCOLOR), showPopup && ! popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR), showPopup && ! popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPTEXTCOLOR), showPopup && ! popupWindowColor);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_POPUPINCOMING:
					showPopup = (IsDlgButtonChecked(hwnd,IDC_POPUPINCOMING)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORS),showPopup);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLOR),showPopup && ! popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPBACKCOLOR),showPopup && ! popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLOR),showPopup && ! popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPTEXTCOLOR),showPopup && ! popupWindowColor);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIME),showPopup);
					EnableWindow(GetDlgItem(hwnd,IDC_PREVIEW),showPopup);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_USEWINCOLORSERR:
					popupWindowColorErr = (IsDlgButtonChecked(hwnd,IDC_USEWINCOLORSERR)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLORERR), showPopupErr && ! popupWindowColorErr);
					EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPBACKCOLORERR), showPopupErr && ! popupWindowColorErr);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLORERR), showPopupErr && ! popupWindowColorErr);
					EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPTEXTCOLORERR), showPopupErr && ! popupWindowColorErr);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
				case IDC_POPUPERROR:
					showPopupErr = (IsDlgButtonChecked(hwnd,IDC_POPUPERROR)==BST_CHECKED);
					EnableWindow(GetDlgItem(hwnd,IDC_USEWINCOLORSERR),showPopupErr);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPBACKCOLORERR),showPopupErr && ! popupWindowColorErr);
					EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPBACKCOLORERR),showPopupErr && ! popupWindowColorErr);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTEXTCOLORERR),showPopupErr && ! popupWindowColorErr);
					EnableWindow(GetDlgItem(hwnd,IDC_STATIC_POPUPTEXTCOLORERR),showPopupErr && ! popupWindowColorErr);
					EnableWindow(GetDlgItem(hwnd,IDC_POPUPTIMEERR),showPopupErr);
					EnableWindow(GetDlgItem(hwnd,IDC_PREVIEWERR),showPopupErr);
					SendMessage(GetParent(hwnd),PSM_CHANGED,0,0);
					break;
			}	
			
			break;

		case WM_DESTROY:
			break;
	}

	return 0;
}

INT_PTR CALLBACK OptionsDlgProc(HWND hwnd, UINT msg, WPARAM, LPARAM lParam)
{
	static int iInit = TRUE;
   
   switch(msg)
   {
      case WM_INITDIALOG:
      {
         TCITEM tci;
         RECT rcClient;
         GetClientRect(hwnd, &rcClient);

		 iInit = TRUE;
         tci.mask = TCIF_PARAM|TCIF_TEXT;
         tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_DEFAULT), hwnd, OptionsDefaultDlgProc);
         tci.pszText = TranslateT("Skype default");
		 TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 0, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-5,rcClient.bottom-31,1);
#ifdef HAVE_UXTHEMES
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);
#endif

         tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_ADVANCED),hwnd,OptionsAdvancedDlgProc);
         tci.pszText = TranslateT("Skype advanced");
         TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 1, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-5,rcClient.bottom-31,1);
         ShowWindow((HWND)tci.lParam, SW_HIDE);
#ifdef HAVE_UXTHEMES
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);
#endif

		 tci.lParam = (LPARAM)CreateDialog(hInst,MAKEINTRESOURCE(IDD_OPT_PROXY),hwnd,OptionsProxyDlgProc);
         tci.pszText = TranslateT("Skype proxy");
         TabCtrl_InsertItem(GetDlgItem(hwnd, IDC_OPTIONSTAB), 2, &tci);
         MoveWindow((HWND)tci.lParam,1,28,rcClient.right-5,rcClient.bottom-31,1);
         ShowWindow((HWND)tci.lParam, SW_HIDE);
#ifdef HAVE_UXTHEMES
		 if(MyEnableThemeDialogTexture)
             MyEnableThemeDialogTexture((HWND)tci.lParam, ETDT_ENABLETAB);
#endif

         iInit = FALSE;
         return FALSE;
      }

      case PSM_CHANGED: // used so tabs dont have to call SendMessage(GetParent(GetParent(hwnd)), PSM_CHANGED, 0, 0);
         if(!iInit)
             SendMessage(GetParent(hwnd), PSM_CHANGED, 0, 0);
         break;
      case WM_NOTIFY:
         switch(((LPNMHDR)lParam)->idFrom) {
            case 0:
               switch (((LPNMHDR)lParam)->code)
               {
                  case PSN_APPLY:
                     {
                        TCITEM tci;
                        int i,count;
                        tci.mask = TCIF_PARAM;
                        count = TabCtrl_GetItemCount(GetDlgItem(hwnd,IDC_OPTIONSTAB));
                        for (i=0;i<count;i++)
                        {
                           TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),i,&tci);
                           SendMessage((HWND)tci.lParam,WM_NOTIFY,0,lParam);
                        }						
                     }
                  break;
               }
            break;
            case IDC_OPTIONSTAB:
               switch (((LPNMHDR)lParam)->code)
               {
                  case TCN_SELCHANGING:
                     {
                        TCITEM tci;
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwnd,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_HIDE);                     
                     }
                  break;
                  case TCN_SELCHANGE:
                     {
                        TCITEM tci;
                        tci.mask = TCIF_PARAM;
                        TabCtrl_GetItem(GetDlgItem(hwnd,IDC_OPTIONSTAB),TabCtrl_GetCurSel(GetDlgItem(hwnd,IDC_OPTIONSTAB)),&tci);
                        ShowWindow((HWND)tci.lParam,SW_SHOW);                     
                     }
                  break;
               }
            break;

         }
      break;
   }
   return FALSE;
}

INT_PTR CALLBACK OptionsProxyDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	const int Skype2SocketControls[]={ IDC_STATIC_HOST, IDC_HOST, IDC_STATIC_PORT, IDC_PORT, IDC_REQPASS, IDC_PASSWORD, IDC_STATIC_RESTART };
	static BOOL initDlg=FALSE;
	DBVARIANT dbv;
	
	switch (uMsg){
		case WM_INITDIALOG:	
			initDlg=TRUE;
			TranslateDialogDefault(hwndDlg);
			if (!db_get_s(NULL, SKYPE_PROTONAME, "Host", &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_HOST, dbv.pszVal);
				db_free(&dbv);
			} else SetDlgItemText(hwndDlg, IDC_HOST, _T("localhost"));
			SendDlgItemMessage(hwndDlg, IDC_PORT, EM_SETLIMITTEXT, 5, 0L);
			SetDlgItemInt(hwndDlg, IDC_PORT, db_get_w(NULL, SKYPE_PROTONAME, "Port", 1401), FALSE);
			CheckDlgButton(hwndDlg, IDC_REQPASS, db_get_b(NULL, SKYPE_PROTONAME, "RequiresPassword", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_USES2S, db_get_b(NULL, SKYPE_PROTONAME, "UseSkype2Socket", 0) ? BST_CHECKED : BST_UNCHECKED);
			if (!db_get_s(NULL, SKYPE_PROTONAME, "Password", &dbv)) {
				SetDlgItemTextA(hwndDlg, IDC_PASSWORD, dbv.pszVal);
				db_free(&dbv);
			}
			SendMessage(hwndDlg, WM_COMMAND, IDC_USES2S, 0);
			SendMessage(hwndDlg, WM_COMMAND, IDC_REQPASS, 0);
			initDlg=FALSE;
			return TRUE;
		case WM_NOTIFY: {
			NMHDR* nmhdr = (NMHDR*)lParam;

			switch (nmhdr->code){
				case PSN_APPLY:
				case PSN_KILLACTIVE:
				{
					char buf[1024];
					GetDlgItemTextA(hwndDlg, IDC_HOST, buf, SIZEOF(buf));
					db_set_s(NULL, SKYPE_PROTONAME, "Host", buf);
					db_set_w(NULL, SKYPE_PROTONAME, "Port", (unsigned short)GetDlgItemInt(hwndDlg, IDC_PORT, NULL, FALSE));
					db_set_b(NULL, SKYPE_PROTONAME, "RequiresPassword", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_REQPASS)));
					db_set_b (NULL, SKYPE_PROTONAME, "UseSkype2Socket", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_USES2S)));
					memset(buf, 0, sizeof(buf));
					GetDlgItemTextA(hwndDlg, IDC_PASSWORD, buf, SIZEOF(buf));
					db_set_s(NULL, SKYPE_PROTONAME, "Password", buf);
					return TRUE;
				}
			}			
			break; 
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_USES2S:
					for (int i=0; i < SIZEOF(Skype2SocketControls); i++)
						EnableWindow(GetDlgItem(hwndDlg, Skype2SocketControls[i]), IsDlgButtonChecked(hwndDlg, LOWORD(wParam)));
					if (IsDlgButtonChecked(hwndDlg, LOWORD(wParam)))
						SendMessage(hwndDlg, WM_COMMAND, IDC_REQPASS, 0);
					break;
				case IDC_REQPASS:
					EnableWindow(GetDlgItem(hwndDlg, IDC_PASSWORD), IsDlgButtonChecked(hwndDlg, LOWORD(wParam)));
					break;

			}
			if (!initDlg) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
	}
	return 0;
}

INT_PTR CALLBACK OptionsAdvancedDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static BOOL initDlg=FALSE;
	static int statusModes[]={ID_STATUS_OFFLINE,ID_STATUS_ONLINE,ID_STATUS_AWAY,ID_STATUS_NA,ID_STATUS_OCCUPIED,ID_STATUS_DND,ID_STATUS_FREECHAT,ID_STATUS_INVISIBLE,ID_STATUS_OUTTOLUNCH,ID_STATUS_ONTHEPHONE};
	
	switch (uMsg){
		case WM_INITDIALOG: {	
			initDlg=TRUE;

			TranslateDialogDefault(hwndDlg);
			CheckDlgButton(hwndDlg, IDC_ENABLEMENU, db_get_b(NULL, SKYPE_PROTONAME, "EnableMenu", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOERRORS, db_get_b(NULL, SKYPE_PROTONAME, "SuppressErrors", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_KEEPSTATE, db_get_b(NULL, SKYPE_PROTONAME, "KeepState", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_TIMEZONE, db_get_b(NULL, SKYPE_PROTONAME, "UseTimeZonePatch", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_IGNTZ, db_get_b(NULL, SKYPE_PROTONAME, "IgnoreTimeZones", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWDEFAULTAVATAR, db_get_b(NULL, SKYPE_PROTONAME, "ShowDefaultSkypeAvatar", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SUPPRESSCALLSUMMARYMESSAGE, db_get_b(NULL, SKYPE_PROTONAME, "SuppressCallSummaryMessage", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOSKYPE3STATS, db_get_b(NULL, SKYPE_PROTONAME, "NoSkype3Stats", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHOWFULLNAME, db_get_b(NULL, SKYPE_PROTONAME, "ShowFullname", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOACK, db_get_b(NULL, SKYPE_PROTONAME, "NoAck", 1) ? BST_CHECKED : BST_UNCHECKED);

			if (ServiceExists(MS_GC_NEWSESSION) && (!bProtocolSet || protocol>=5)) {
				CheckDlgButton(hwndDlg, IDC_GROUPCHAT, db_get_b(NULL, SKYPE_PROTONAME, "UseGroupchat", 0) ? BST_CHECKED : BST_UNCHECKED);
				CheckDlgButton(hwndDlg, IDC_GROUPCHATREAD, db_get_b(NULL, SKYPE_PROTONAME, "MarkGroupchatRead", 0) ? BST_CHECKED : BST_UNCHECKED);
			} else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHAT), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_GROUPCHATREAD), FALSE);
			}

#ifdef USEPOPUP
			if (ServiceExists(MS_POPUP_ADDPOPUPT))
				CheckDlgButton(hwndDlg, IDC_USEPOPUP, db_get_b(NULL, SKYPE_PROTONAME, "UsePopup", 0) ? BST_CHECKED : BST_UNCHECKED);
			else
#endif
				EnableWindow(GetDlgItem(hwndDlg, IDC_USEPOPUP), FALSE);

			int j=db_get_dw(NULL, SKYPE_PROTONAME, "SkypeOutStatusMode", ID_STATUS_ONTHEPHONE);
			for(int i=0;i<sizeof(statusModes)/sizeof(statusModes[0]);i++) {
				int k=SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_ADDSTRING,0,(LPARAM)CallService(MS_CLIST_GETSTATUSMODEDESCRIPTION,statusModes[i],GSMDF_TCHAR));
				SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_SETITEMDATA,k,statusModes[i]);
				if (statusModes[i]==j) SendDlgItemMessage(hwndDlg,IDC_SKYPEOUTSTAT,CB_SETCURSEL,i,0);
			}
			initDlg=FALSE;
			return TRUE;
	}
		case WM_NOTIFY: {
			NMHDR* nmhdr = (NMHDR*)lParam;

			switch (nmhdr->code){
				case PSN_APPLY:
				case PSN_KILLACTIVE:
					db_set_b (NULL, SKYPE_PROTONAME, "EnableMenu", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_ENABLEMENU)));
					db_set_b (NULL, SKYPE_PROTONAME, "UsePopup", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_USEPOPUP)));
					db_set_b (NULL, SKYPE_PROTONAME, "UseGroupchat", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_GROUPCHAT)));
					db_set_b (NULL, SKYPE_PROTONAME, "MarkGroupchatRead", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_GROUPCHATREAD)));
					db_set_b (NULL, SKYPE_PROTONAME, "SuppressErrors", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NOERRORS)));
					db_set_b (NULL, SKYPE_PROTONAME, "KeepState", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_KEEPSTATE)));
					db_set_dw(NULL, SKYPE_PROTONAME, "SkypeOutStatusMode", SendDlgItemMessage(hwndDlg, IDC_SKYPEOUTSTAT, CB_GETITEMDATA, SendDlgItemMessage(hwndDlg, IDC_SKYPEOUTSTAT, CB_GETCURSEL, 0, 0), 0));
					db_set_b (NULL, SKYPE_PROTONAME, "UseTimeZonePatch", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_TIMEZONE)));
					db_set_b (NULL, SKYPE_PROTONAME, "IgnoreTimeZones", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_IGNTZ)));
					db_set_b (NULL, SKYPE_PROTONAME, "ShowDefaultSkypeAvatar", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SHOWDEFAULTAVATAR)));
					db_set_b (NULL, SKYPE_PROTONAME, "SuppressCallSummaryMessage", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SUPPRESSCALLSUMMARYMESSAGE)));
					db_set_b (NULL, SKYPE_PROTONAME, "NoSkype3Stats", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NOSKYPE3STATS)));
					db_set_b (NULL, SKYPE_PROTONAME, "ShowFullname", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SHOWFULLNAME)));
					db_set_b (NULL, SKYPE_PROTONAME, "NoAck", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NOACK)));
					return TRUE;
			}			
			break; 
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_CLEANUP:
					pthread_create(( pThreadFunc )CleanupNicknames, NULL);
					break;
			}
			if (!initDlg) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
	}
	return 0;
}

static int CALLBACK BrowseCallbackProc(HWND hWnd, UINT uMsg, LPARAM, LPARAM lpData)
{
	switch (uMsg)
	{
		case BFFM_INITIALIZED:
		{
			// Set initial directory.
			wchar_t* wszInitFolder = make_unicode_string((const unsigned char*)lpData);
			SendMessage(hWnd, BFFM_SETSELECTION, TRUE, (LPARAM)wszInitFolder);
			free(wszInitFolder);
			break;
		}
	}
	return 0;
}

INT_PTR CALLBACK OptionsDefaultDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static BOOL initDlg=FALSE;
	static int skypeLaunchControls[]={IDC_NOSPLASH,IDC_MINIMIZED,IDC_NOTRAY,IDC_REMOVEABLE,IDC_SECONDARY,IDC_DATAPATHO,IDC_CUSTOMCOMMAND,IDC_STATIC_PATHINFO};
	
	switch (uMsg){
		case WM_INITDIALOG:	
		{
			DBVARIANT dbv;

			initDlg = TRUE;
			TranslateDialogDefault(hwndDlg);

			BOOL startSkype = db_get_b(NULL, SKYPE_PROTONAME, "StartSkype", 1);

			CheckDlgButton(hwndDlg, IDC_STARTSKYPE, startSkype ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOSPLASH, db_get_b(NULL, SKYPE_PROTONAME, "nosplash", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_MINIMIZED, db_get_b(NULL, SKYPE_PROTONAME, "minimized", 1) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_NOTRAY, db_get_b(NULL, SKYPE_PROTONAME, "notray", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_REMOVEABLE, db_get_b(NULL, SKYPE_PROTONAME, "removable", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SECONDARY, db_get_b(NULL, SKYPE_PROTONAME, "secondary", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_DATAPATHO, db_get_b(NULL, SKYPE_PROTONAME, "datapath:", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_SHUTDOWN, db_get_b(NULL, SKYPE_PROTONAME, "Shutdown", 0) ? BST_CHECKED : BST_UNCHECKED);
			CheckDlgButton(hwndDlg, IDC_UNLOADOFFLINE, db_get_b(NULL, SKYPE_PROTONAME, "UnloadOnOffline", 0) ? BST_CHECKED : BST_UNCHECKED);
			
			CheckDlgButton(hwndDlg, IDC_CUSTOMCOMMAND, db_get_b(NULL, SKYPE_PROTONAME, "UseCustomCommand", 0) ? BST_CHECKED : BST_UNCHECKED);
			SendDlgItemMessage(hwndDlg, IDC_COMMANDLINE, EM_SETLIMITTEXT, MAX_PATH-1, 0L);
			if(!db_get_s(NULL,SKYPE_PROTONAME,"CommandLine",&dbv)) 
			{
				SetDlgItemTextA(hwndDlg, IDC_COMMANDLINE, dbv.pszVal);
				db_free(&dbv);
			}

			SendDlgItemMessage(hwndDlg, IDC_DATAPATH, EM_SETLIMITTEXT, MAX_PATH-1, 0L);
			if(!db_get_s(NULL,SKYPE_PROTONAME,"datapath",&dbv)) 
			{
				SetDlgItemTextA(hwndDlg, IDC_DATAPATH, dbv.pszVal);
				db_free(&dbv);
			}

			for(int i=0; i < SIZEOF(skypeLaunchControls); i++)
				EnableWindow(GetDlgItem(hwndDlg, skypeLaunchControls[i]), startSkype);

			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSECMDL), startSkype && IsDlgButtonChecked(hwndDlg, IDC_CUSTOMCOMMAND));
			EnableWindow(GetDlgItem(hwndDlg, IDC_COMMANDLINE), startSkype && IsDlgButtonChecked(hwndDlg, IDC_CUSTOMCOMMAND));

			EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSEDP), startSkype && IsDlgButtonChecked(hwndDlg, IDC_DATAPATHO));
			EnableWindow(GetDlgItem(hwndDlg, IDC_DATAPATH), startSkype && IsDlgButtonChecked(hwndDlg, IDC_DATAPATHO));

            // LoginUserName
            if(!db_get_ts(NULL,SKYPE_PROTONAME,"LoginUserName",&dbv)) 
			{
				SetDlgItemText(hwndDlg, IDC_USERNAME, dbv.ptszVal);
				db_free(&dbv);
			}

            // LoginPassword
            if(!db_get_ts(NULL,SKYPE_PROTONAME,"LoginPassword",&dbv)) 
			{
				SetDlgItemText(hwndDlg, IDC_PASSWORD, dbv.ptszVal);
				db_free(&dbv);
			}

			SendDlgItemMessage(hwndDlg, IDC_CONNATTEMPTS, EM_SETLIMITTEXT, 3, 0L);
			SetDlgItemInt (hwndDlg, IDC_CONNATTEMPTS, db_get_w(NULL, SKYPE_PROTONAME, "ConnectionAttempts", 10), FALSE);
			SendMessage(hwndDlg, WM_COMMAND, IDC_STARTSKYPE, 0);
			initDlg=FALSE;
			return TRUE;
		}
		case WM_NOTIFY: {
			NMHDR* nmhdr = (NMHDR*)lParam;

			switch (nmhdr->code){
				case PSN_APPLY:
				case PSN_KILLACTIVE:
				{
					char text[500];
					TCHAR wtext[500];
					char szRelativePath[MAX_PATH];

					db_set_b (NULL, SKYPE_PROTONAME, "StartSkype", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_STARTSKYPE)));
					db_set_b (NULL, SKYPE_PROTONAME, "nosplash", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NOSPLASH)));
					db_set_b (NULL, SKYPE_PROTONAME, "minimized", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_MINIMIZED)));
					db_set_b (NULL, SKYPE_PROTONAME, "notray", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_NOTRAY)));
					db_set_b (NULL, SKYPE_PROTONAME, "Shutdown", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SHUTDOWN)));
					db_set_b (NULL, SKYPE_PROTONAME, "UnloadOnOffline", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_UNLOADOFFLINE)));
					db_set_w (NULL, SKYPE_PROTONAME, "ConnectionAttempts", (unsigned short)GetDlgItemInt(hwndDlg, IDC_CONNATTEMPTS, NULL, FALSE));
					db_set_b (NULL, SKYPE_PROTONAME, "UseCustomCommand", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_CUSTOMCOMMAND)));
					db_set_b (NULL, SKYPE_PROTONAME, "datapath:", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_DATAPATHO)));
					db_set_b (NULL, SKYPE_PROTONAME, "removable", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_REMOVEABLE)));
					db_set_b (NULL, SKYPE_PROTONAME, "secondary", (BYTE)(IsDlgButtonChecked(hwndDlg, IDC_SECONDARY)));

					GetDlgItemTextA(hwndDlg,IDC_COMMANDLINE,text,SIZEOF(text));
					strncpy(szRelativePath, text, sizeof(szRelativePath)-1);
					CallService (MS_UTILS_PATHTORELATIVE, (WPARAM)text, (LPARAM)szRelativePath);
					db_set_s(NULL, SKYPE_PROTONAME, "CommandLine", szRelativePath);

					GetDlgItemTextA(hwndDlg,IDC_DATAPATH,text,SIZEOF(text));
					strncpy(szRelativePath, text, sizeof(szRelativePath)-1);
					CallService (MS_UTILS_PATHTORELATIVE, (WPARAM)text, (LPARAM)szRelativePath);
					db_set_s(NULL, SKYPE_PROTONAME, "datapath", szRelativePath);
                   
                    // LoginUserName
                    GetDlgItemText(hwndDlg,IDC_USERNAME,wtext,SIZEOF(wtext));
					db_set_ts(NULL, SKYPE_PROTONAME, "LoginUserName", wtext);

                    // LoginPassword
                    GetDlgItemText(hwndDlg,IDC_PASSWORD,wtext,SIZEOF(wtext));
					db_set_ts(NULL, SKYPE_PROTONAME, "LoginPassword", wtext);

					return TRUE;
				}
			}			
			break; 
		}
		case WM_COMMAND: {
			switch (LOWORD(wParam)) {
				case IDC_STARTSKYPE: {
					BOOL startSkype = IsDlgButtonChecked(hwndDlg, IDC_STARTSKYPE);

					for (int i = 0; i < SIZEOF(skypeLaunchControls); i ++)
						EnableWindow(GetDlgItem(hwndDlg, skypeLaunchControls[i]), startSkype);

					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSECMDL), startSkype && IsDlgButtonChecked(hwndDlg, IDC_CUSTOMCOMMAND));
					EnableWindow(GetDlgItem(hwndDlg, IDC_COMMANDLINE), startSkype && IsDlgButtonChecked(hwndDlg, IDC_CUSTOMCOMMAND));

					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSEDP), startSkype && IsDlgButtonChecked(hwndDlg, IDC_DATAPATHO));
					EnableWindow(GetDlgItem(hwndDlg, IDC_DATAPATH), startSkype && IsDlgButtonChecked(hwndDlg, IDC_DATAPATHO));
				}
					break;
				case IDC_CLEANUP:
					pthread_create(( pThreadFunc )CleanupNicknames, NULL);
					break;
				case IDC_DATAPATHO:
					EnableWindow(GetDlgItem(hwndDlg, IDC_DATAPATH), IsDlgButtonChecked(hwndDlg, IDC_DATAPATHO));
					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSEDP), IsDlgButtonChecked(hwndDlg, IDC_DATAPATHO));
					break;
				case IDC_CUSTOMCOMMAND:
					EnableWindow(GetDlgItem(hwndDlg, IDC_COMMANDLINE), IsDlgButtonChecked(hwndDlg, IDC_CUSTOMCOMMAND));
					EnableWindow(GetDlgItem(hwndDlg, IDC_BROWSECMDL), IsDlgButtonChecked(hwndDlg, IDC_CUSTOMCOMMAND));
					break;
				case IDC_BROWSECMDL:
				{
					OPENFILENAMEA ofn={0};
					char szFileName[MAX_PATH];
					char szAbsolutePath[MAX_PATH];
						
					ofn.lStructSize=sizeof(ofn);
					ofn.hwndOwner=hwndDlg;
					ofn.lpstrFilter="Executable files (*.exe)\0*.exe\0All files (*.*)\0*.*\0";
					ofn.nMaxFile=SIZEOF(szFileName);
					ofn.lpstrDefExt="exe";
					ofn.lpstrFile=szFileName;
					ofn.Flags=OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_ENABLESIZING;

					GetDlgItemTextA(hwndDlg,IDC_COMMANDLINE,szFileName,SIZEOF(szFileName));
					TranslateMirandaRelativePathToAbsolute(szFileName, szAbsolutePath, FALSE);
					strcpy (szFileName, szAbsolutePath);

					BOOL gofnResult = GetOpenFileNameA(&ofn);
					if (!gofnResult && CommDlgExtendedError() == FNERR_INVALIDFILENAME){
						strcpy(szFileName, ".\\Skype.exe");
						TranslateMirandaRelativePathToAbsolute(szFileName, szAbsolutePath, FALSE);
						strcpy (szFileName, szAbsolutePath);
						gofnResult = GetOpenFileNameA(&ofn);
					}

					if(gofnResult)
						SetDlgItemTextA(hwndDlg, IDC_COMMANDLINE, szFileName);

					break;
				}
				case IDC_BROWSEDP:
				{
					char szFileName[MAX_PATH], szAbsolutePath[MAX_PATH];

					GetDlgItemTextA (hwndDlg, IDC_DATAPATH, szFileName, SIZEOF(szFileName));

					TranslateMirandaRelativePathToAbsolute(szFileName, szAbsolutePath, FALSE);
					BROWSEINFOA bi={0};
					bi.hwndOwner = hwndDlg;
					bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_SHAREABLE | BIF_NEWDIALOGSTYLE | BIF_NONEWFOLDERBUTTON;
					bi.lpfn = BrowseCallbackProc;
					bi.lParam = (LPARAM)szAbsolutePath;

					LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
					if (pidl) {
						if (SHGetPathFromIDListA(pidl, szFileName))
							SetDlgItemTextA(hwndDlg, IDC_DATAPATH, szFileName);
						CoTaskMemFree (pidl);
					}
					break;
				}

#ifdef SKYPE_AUTO_DETECTION
				case IDC_AUTODETECTION:
					DoAutoDetect(hwndDlg);
					break;
#endif
			}
			if (!initDlg) SendMessage(GetParent(hwndDlg), PSM_CHANGED, 0, 0);
		}
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////
// OnDetailsInit - initializes user info dialog pages.

int OnDetailsInit( WPARAM wParam, LPARAM lParam )
{
	MCONTACT hContact = (MCONTACT)lParam;

	OPTIONSDIALOGPAGE odp = { 0 };
	odp.hIcon = NULL;
	odp.hInstance = hInst;

	if ( hContact == NULL ) {
		char szTitle[256];
		mir_snprintf(szTitle, SIZEOF( szTitle ), "%s %s", SKYPE_PROTONAME, Translate( "Details" ));
	
		odp.pfnDlgProc = DetailsDlgProc;
		odp.position = 1900000000;
		odp.pszTemplate = MAKEINTRESOURCEA(IDD_SETDETAILS);
		odp.pszTitle = szTitle;
		UserInfo_AddPage(wParam, &odp);
	}

	return 0;
}

/*AvatarDlgProc
*
* For setting the skype avatar
*
*/
INT_PTR CALLBACK AvatarDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	static RECT r;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		hAvatar = NULL;
		if(ServiceExists(MS_AV_GETMYAVATAR)){
			struct avatarCacheEntry *ace = (struct avatarCacheEntry *)CallService(MS_AV_GETMYAVATAR, 0,(LPARAM) SKYPE_PROTONAME);
			if (ace!=NULL) {
				hAvatar = ( HBITMAP )CallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )ace->szFilename);
				if ( hAvatar != NULL )
					SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)hAvatar );
			}
		}


		
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_SETAVATAR:
			{
				char szFileName[ MAX_PATH ];
				if ( EnterBitmapFileName( szFileName ) != ERROR_SUCCESS )
					return FALSE;

				hAvatar = ( HBITMAP )CallService( MS_UTILS_LOADBITMAP, 0, ( LPARAM )szFileName);
				if ( hAvatar != NULL ){
					SendDlgItemMessage(hwndDlg, IDC_AVATAR, STM_SETIMAGE, IMAGE_BITMAP, (WPARAM)hAvatar );
					CallService(SKYPE_SETAVATAR, 0, ( LPARAM )szFileName);
				}
				break;
			}
			case IDC_DELETEAVATAR:
				if ( hAvatar != NULL ) {
					DeleteObject( hAvatar );
					hAvatar = NULL;
					CallService(SKYPE_SETAVATAR, 0, 0);
				}
				db_unset( NULL, SKYPE_PROTONAME, "AvatarFile" );
				InvalidateRect( hwndDlg, NULL, TRUE );
				break;
		}	}
		break;

	case WM_DESTROY:
		if ( hAvatar != NULL )
			DeleteObject( hAvatar );
		break;
	}

	return 0;
}

/*DetailsDlgProc
*
* For setting the skype infos
*
*/
INT_PTR CALLBACK DetailsDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM)
{
	static int sexM = 0,sexF = 0, sex;

	switch ( msg ) {
	case WM_INITDIALOG:
		TranslateDialogDefault( hwndDlg );

		memset(&myProfile, 0, sizeof(myProfile));
		SkypeProfile_Load(&myProfile);
		if(SkypeInitialized)
			SkypeProfile_LoadFromSkype(&myProfile);

		SendDlgItemMessage(hwndDlg,IDC_SEX,CB_ADDSTRING,0,(LPARAM)_T(""));
		sexM = SendDlgItemMessage(hwndDlg,IDC_SEX,CB_ADDSTRING,0,(LPARAM)TranslateT("MALE"));
		sexF = SendDlgItemMessage(hwndDlg,IDC_SEX,CB_ADDSTRING,0,(LPARAM)TranslateT("FEMALE"));
		
		switch(myProfile.Sex) {
		case 0x4D: SendDlgItemMessage(hwndDlg,IDC_SEX,CB_SETCURSEL, sexM, 0); break;
		case 0x46: SendDlgItemMessage(hwndDlg,IDC_SEX,CB_SETCURSEL, sexF, 0); break;
		}

		SetDlgItemText(hwndDlg, IDC_FULLNAME, myProfile.FullName);
		SetDlgItemTextA(hwndDlg, IDC_HOMEPAGE, myProfile.HomePage);
		SetDlgItemTextA(hwndDlg, IDC_HOMEPHONE, myProfile.HomePhone);
		SetDlgItemTextA(hwndDlg, IDC_OFFICEPHONE, myProfile.OfficePhone);
		SetDlgItemText(hwndDlg, IDC_CITY, myProfile.City);
		SetDlgItemText(hwndDlg, IDC_PROVINCE, myProfile.Province);
		DateTime_SetSystemtime (GetDlgItem (hwndDlg, IDC_BIRTHDAY), GDT_VALID, &myProfile.Birthday);
		return TRUE;

	case WM_COMMAND:
		if ( HIWORD( wParam ) == BN_CLICKED ) {
			switch( LOWORD( wParam )) {
			case IDC_SAVEDETAILS:
				GetDlgItemText(hwndDlg,IDC_FULLNAME,myProfile.FullName,SIZEOF(myProfile.FullName));
				GetDlgItemTextA(hwndDlg,IDC_HOMEPAGE,myProfile.HomePage,SIZEOF(myProfile.HomePage));
				GetDlgItemTextA(hwndDlg,IDC_HOMEPHONE,myProfile.HomePhone,SIZEOF(myProfile.HomePhone));
				GetDlgItemTextA(hwndDlg,IDC_OFFICEPHONE,myProfile.OfficePhone,SIZEOF(myProfile.OfficePhone));
				GetDlgItemText(hwndDlg,IDC_CITY,myProfile.City,SIZEOF(myProfile.City));
				GetDlgItemText(hwndDlg,IDC_PROVINCE,myProfile.Province,SIZEOF(myProfile.Province));
				sex = SendDlgItemMessage(hwndDlg, IDC_SEX, CB_GETCURSEL, 0, 0);
				
				myProfile.Sex = 0;
				if(sex == sexF) myProfile.Sex = 0x46; else
				if(sex == sexM) myProfile.Sex = 0x4D;
				DateTime_GetSystemtime (GetDlgItem (hwndDlg, IDC_BIRTHDAY), &myProfile.Birthday);

				SkypeProfile_Save(&myProfile);
				if(SkypeInitialized)
					SkypeProfile_SaveToSkype(&myProfile);
				break;
			}	
		}
		break;

	case WM_DESTROY:
		if ( hAvatar != NULL )
			DeleteObject( hAvatar );
		break;
	}

	return 0;
}

#ifdef SKYPE_AUTO_DETECTION
/**
 * DoAutoDetect
 * @param dlg The default option dialog handle
 */
void DoAutoDetect(HWND dlg)
{
	char basePath[MAX_PATH];
	char fileName[MAX_PATH];
	char tmpUser[255];
	ezxml_t f1, acc;
	
	if (FAILED(SHGetFolderPath(dlg,CSIDL_APPDATA,NULL,0,basePath)))
	{
		OUTPUT("Error in retrieving appdata path!");
		return;
	}

	strcat(basePath,"\\Skype\\");
	sprintf (fileName, "%s\\shared.xml", basePath);

	if (f1 = ezxml_parse_file(fileName))
	{
 		if (acc = ezxml_get(f1, "Lib", 0, "Account", 0, "Default", -1))
		{
			if (GetDlgItemTextA(dlg, IDC_USERNAME, tmpUser, SIZEOF(tmpUser)))
				SetDlgItemTextA(dlg, IDC_USERNAME, acc->txt);
			/* Can't find this stuff in current Skype verions??
			sprintf (fileName, "%s\\%s\\config.xml", basePath, acc->txt);
			if ((acc = ezxml_get(f1, "UI", 0, "Messages", 0, "OpenWindowInCompactMode", -1)) && *acc->txt!='0')
			{
				ezxml_set_txt (acc, "0");
				// ezXML doesn't supprot saving yet
			}
			*/
		}
		ezxml_free(f1);
	}
	else
	{
		OUTPUT("Failed to open skypes configuration files!");
		return;
	}
}
#endif

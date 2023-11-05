#ifndef M_POPUP_INT_H
#define M_POPUP_INT_H

#include <m_popup.h>

// Creates new popup
//    wParam = (WPARAM)(POPUPDATA2*)&ppd2
//    lParam = (LPARAM)(combination of APF_* flags)
//    returns: window handle (if requested) of NULL on success, -1 on failure.
#define MS_POPUP_ADDPOPUP2 "Popup/AddPopup2"

// Update an popup
//    wParam = (WPARAM)(HWND)hwndPopup
//    lParam = (LPARAM)(POPUPDATA2*)&ppd2
//    returns: zero on success, -1 on failure.
#define MS_POPUP_CHANGEPOPUP2 "Popup/ChangePopup2"

/////////////////////////////////////////////////////////////////////////////////////////
// Compatibility services

#define MS_POPUP_ADDPOPUP "Popup/AddPopup"
#define MS_POPUP_ADDPOPUPW "Popup/AddPopupW"

#define MS_POPUP_CHANGEW "Popup/ChangeW"
#define MS_POPUP_CHANGETEXTW "Popup/ChangetextW"

#define MS_POPUP_DESTROYPOPUP "Popup/Delete"

#define MS_POPUP_GETPLUGINDATA "Popup/GetPluginData"

#define MS_POPUP_GETCONTACT "Popup/GetContact"

#define MS_POPUP_REGISTERACTIONS "Popup/RegisterActions"

#define MS_POPUP_SHOWMESSAGE "Popup/ShowMessage"
#define MS_POPUP_SHOWMESSAGEW "Popup/ShowMessageW"

#define MS_POPUP_REGISTERCLASS   "Popup/RegisterClass"
#define MS_POPUP_UNREGISTERCLASS "Popup/UnregisterClass"
#define MS_POPUP_ADDPOPUPCLASS	"Popup/AddPopupClass"

#endif // M_POPUP_INT_H

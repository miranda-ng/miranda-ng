#ifndef __m_popup2_h__
#define __m_popup2_h__

#define NFOPT_POPUP2_BACKCOLOR    "Popup2/BackColor"
#define NFOPT_POPUP2_TEXTCOLOR    "Popup2/TextColor"
#define NFOPT_POPUP2_TIMEOUT      "Popup2/Timeout"
#define NFOPT_POPUP2_LCLICKSVC    "Popup2/LClickSvc"
#define NFOPT_POPUP2_LCLICKCOOKIE "Popup2/LClickCookie"
#define NFOPT_POPUP2_RCLICKSVC    "Popup2/RClickSvc"
#define NFOPT_POPUP2_RCLICKCOOKIE "Popup2/RClickCookie"
#define NFOPT_POPUP2_STATUSMODE   "Popup2/StatusMode"
#define NFOPT_POPUP2_PLUGINDATA   "Popup2/PluginData"
#define NFOPT_POPUP2_WNDPROC      "Popup2/WndProc"

#define NFOPT_POPUP2_BACKCOLOR_S  "Popup2/BackColor/Save"
#define NFOPT_POPUP2_TEXTCOLOR_S  "Popup2/TextColor/Save"
#define NFOPT_POPUP2_TIMEOUT_S    "Popup2/Timeout/Save"

#define MS_POPUP2_SHOW   "Popup2/Show"
#define MS_POPUP2_UPDATE "Popup2/Update"
#define MS_POPUP2_REMOVE "Popup2/Remove"

#ifndef POPUP2_NOHELPERS
	#define MPopup2Show(a)   (CallService(MS_POPUP2_SHOW, 0, (LPARAM)(a)))
	#define MPopup2Update(a) (CallService(MS_POPUP2_UPDATE, 0, (LPARAM)(a)))
	#define MPopup2Remove(a) (CallService(MS_POPUP2_REMOVE, 0, (LPARAM)(a)))
#endif

#endif // __m_popup2_h__

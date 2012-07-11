// lParam = (char *)status message
// wParam = new status, from statusmodes.h
#define MS_SA_SETSTATUSMODE "SimpleAway/SetStatusMode"
#define MS_AWAYSYS_SETSTATUSMODE MS_SA_SETSTATUSMODE //for compatibility with some plugins

//Internal use only
#define MS_SA_TTCHANGESTATUSMSG "SimpleAway/TTChangeStatusMessage"

//wParam=new status, from statusmodes.h
//lParam=protocol name, NULL if for all protocols (added in v0.3.1alpha)
#define MS_SA_CHANGESTATUSMSG "SimpleAway/ChangeStatusMessage"

// wParam = 0
// lParam = 0
// allways returns 1
#define MS_SA_ISSARUNNING  "SimpleAway/IsSARunning"

// wParam = 0
// lParam = 0
#define MS_SA_COPYAWAYMSG  "SimpleAway/CopyAwayMsg"
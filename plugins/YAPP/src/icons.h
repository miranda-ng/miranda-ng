#ifndef __icons_h__
#define __icons_h__

//ICONS
#define SECT_TOLBAR         "Toolbar"
#define SECT_POPUP          MODULE

#define ICO_TB_POPUP_ON	    "TBButton_popup_ToogleUp"
#define ICO_TB_POPUP_OFF    "TBButton_popup_ToogleDOWN"
#define ICO_POPUP_ON        MODULE"_enabled"
#define ICO_POPUP_OFF       MODULE"_disabled"
#define ICO_HISTORY         MODULE"_history"

void	InitIcons();
HICON	IcoLib_GetIcon(LPCSTR pszIcon, bool big=false);

#endif // __icons_h__

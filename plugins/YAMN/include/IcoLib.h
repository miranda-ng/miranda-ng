typedef struct {
	int cbSize;
    char *pszSection;        //section name used to group icons
	char *pszDescription;	   //description for options dialog
	char *pszName;		   //name to refer to icon when playing and in db
    char *pszDefaultFile;    //default icon file to use
	int  iDefaultIndex;
} SKINICONDESC;

//
//  Add a icon into options UI
//  NB! pszName should be unique, e.g.: clistmw_apply, tabsrmm_history
//
//  wParam = (WPARAM)0
//  lParam = (LPARAM)(SKINICONDESC*)sid;
//
#define MS_SKIN2_ADDICON "Skin2/Icons/AddIcon"
//
//  Retrieve HICON with name specified in lParam
//  Returned HICON SHOULDN'T be destroyed, it managed by IcoLib
//
#define MS_SKIN2_GETICON "Skin2/Icons/GetIcon"
//
//  Icons change notification
//
#define ME_SKIN2_ICONSCHANGED "Skin2/IconsChanged"

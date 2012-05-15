typedef struct {
	int cbSize;
    char *pszSection;     //section name used to group icons
	char *pszDescription; //description for options dialog
	char *pszName;        //name to refer to icon when playing and in db
	                      //this name is miranda-wide. so use prefixes of your plugin
	                      //e.g: "isee_connect", "clist_delete", etc
    char *pszDefaultFile; //default icon file to use
	int  iDefaultIndex;
} SKINICONDESC;

typedef struct {    
	int cbSize;
    char *pszSection;
	char *pszDescription;
	char *pszName;
    char *pszDefaultFile;
	int  iDefaultIndex;
	HICON hDefaultIcon;
} SKINICONDESC2;

//
//  Add a icon into options UI
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

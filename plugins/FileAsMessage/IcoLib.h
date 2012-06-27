typedef struct {
	int cbSize;
    char *pszSection;        //section name used to group icons
	char *pszDescription;	   //description for options dialog
	char *pszName;		   //name to refer to icon when playing and in db
    char *pszDefaultFile;    //default icon file to use
	int  iDefaultIndex;
} SKINICONDESC;

#define MS_SKIN2_ADDICON "Skin2/Icons/AddIcon"
#define MS_SKIN2_GETICON "Skin2/Icons/GetIcon"
#define ME_SKIN2_ICONSCHANGED "Skin2/IconsChanged"

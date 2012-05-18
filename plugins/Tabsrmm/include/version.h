#define VER_PLAIN	"3,1,99,8"
#define _VER_MAJOR	3
#define _VER_MINOR  1
#define _VER_REVISION 99
#define _VER_BUILD	8

#if defined(_WIN64)
	#define RES_FILE_DESC "TabSRMM (Unicode) x86_amd64"
	#define ADDONS_UPDATE_URL "http://addons.miranda-im.org/details.php?action=viewfile&id=4318"
	#define ADDONS_DL_URL "http://addons.miranda-im.org/feed.php?dlfile=4318"
#else
	#define RES_FILE_DESC "TabSRMM (Unicode) x86"
	#define ADDONS_UPDATE_URL "http://addons.miranda-im.org/details.php?action=viewfile&id=4317"
	#define ADDONS_DL_URL "http://addons.miranda-im.org/feed.php?dlfile=4317"
#endif

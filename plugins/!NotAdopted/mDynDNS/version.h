#define __MAJOR_VERSION            0
#define __MINOR_VERSION            8
#define __RELEASE_NUM            1
#define __BUILD_NUM               2

#include <stdver.h>

#define __USER_AGENT_STRING         "MirandaIM - mDynDns plugin - "__MAJOR_VERSION.__MINOR_VERSION.__RELEASE_NUM.__BUILD_NUM

#define __FILENAME               "mDynDNS.dll"
#define __DESC                  "Alows user to update his IP on DynDNS server."
#define __AUTHOR               "Kirsen, Merlin"
#define __AUTHOREMAIL            "ing.u.horn@googlemail.com"
#define __COPYRIGHT               "© 2006 Kirsen, © 2009 Merlin"
#define __AUTHORWEB               "http://code.google.com/p/merlins-miranda"

#define PLUGNAME               "mDynDNS"
#ifdef _UNICODE
#define __PLUGIN_NAME            PLUGNAME" (Unicode)"
#define __FLVersionURL            "http://addons.miranda-im.org/details.php?action=viewfile&id=4075"
#define __FLVersionPrefix         "<span class=\"fileNameHeader\">mDynDNS (Unicode) "
#define __FLUpdateURL            "http://addons.miranda-im.org/feed.php?dlfile=4075"
#define __BetaUpdateURL            "http://merlins-miranda.googlecode.com/files/mDynDNSW.zip"
#else
#define __PLUGIN_NAME            PLUGNAME" (2in1)"
#define __FLVersionURL            "http://addons.miranda-im.org/details.php?action=viewfile&id=4075"
#define __FLVersionPrefix         "<span class=\"fileNameHeader\">mDynDNS (2in1) "
#define __FLUpdateURL            "http://addons.miranda-im.org/feed.php?dlfile=4075"
#define __BetaUpdateURL            "http://merlins-miranda.googlecode.com/files/mDynDNS.zip"
#endif

#define __BetaVersionURL         "http://merlins-miranda.googlecode.com/svn/trunk/miranda/plugins/mDynDNS/changelog.txt"
#define __BetaVersionPrefix         "mDynDNS: "
#define __BetaChangelogURL         "http://merlins-miranda.googlecode.com/svn/trunk/miranda/plugins/mDynDNS/changelog.txt"

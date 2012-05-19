#ifndef _XMLDATA_INC
#define _XMLDATA_INC

#include "options.h"
#include "bzip2-1.0.3/bzlib.h"
#include "utils.h"
#include "popups.h"
#include "services.h"

#define MIM_BACKEND_URL_PREFIX		"http://addons.miranda-im.org/backend/"
#define MIM_DOWNLOAD_URL_PREFIX		"http://addons.miranda-im.org/feed.php?dlfile="
#define MIM_VIEW_URL_PREFIX			"http://addons.miranda-im.org/details.php?action=viewfile&id="
#define MIM_CHANGELOG_URL_PREFIX	"http://addons.miranda-im.org/details.php?action=viewlog&id="
// this is the minimum age (in hours) of the local copy before a new download is allowed
#define	MIN_XMLDATA_AGE		8

void FreeXMLData(const Category cat);
bool OldXMLDataExists(const Category cat);
bool LoadOldXMLData(const Category cat, bool update_age);
bool UpdateXMLData(const Category cat, const char *redirect_url = 0, int recurse_count = 0);
bool XMLDataAvailable(const Category cat);

const char *FindVersion(int fileID, BYTE *pbVersionBytes, int cpbVersionBytes, const Category cat);
int FindFileID(const char *name, const Category cat, UpdateList *update_list);

// update file listing id's for non-plugin AUTOREGISTER components
void UpdateFLIDs(UpdateList &update_list);

#endif

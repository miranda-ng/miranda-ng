#ifndef _SOCKET_INC
#define _SOCKET_INC

#include "services.h"
#include "options.h"
#include "xmldata.h"
#include "popups.h"


// gets a file from a url, puts in in the temp_folder and names it after the plugin_name (same extension as url)
// it will automatically unzip a zip file, putting the contents in the temp_folder and using the names from the archive
// if it unzips the file, the zip archive is removed afterward
bool GetFile(char *url, TCHAR *temp_folder, char *plugin_name, char *version, bool dlls_only, int recurse_count = 0);

// check the xml backend data or version URL (depending on whether the plugin is on the file listing) to see if we need an update
// returns new version string (which you must free) and sets beta to indicate where update should be retreived from (i.e. if
// beta url is enabled, will return true in beta unless the non-beta version is higher)
char *UpdateRequired(UpdateInternal &update_internal, bool *beta);

#endif

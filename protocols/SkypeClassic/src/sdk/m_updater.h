#ifndef _M_UPDATER_H
#define _M_UPDATER_H

// NOTES:
// - For langpack updates, include a string of the following format in the langpack text file:
//    ";FLID: <file listing name> <version>"
//    version must be four numbers seperated by '.', in the range 0-255 inclusive
// - Updater will disable plugins that are downloaded but were not active prior to the update (this is so that, if an archive contains e.g. ansi and 
//    unicode versions, the correct plugin will be the only one active after the new version is installed)...so if you add a support plugin, you may need 
//    to install an ini file to make the plugin activate when miranda restarts after the update
// - Updater will replace all dlls that have the same internal shortName as a downloaded update dll (this is so that msn1.dll and msn2.dll, for example,
//    will both be updated) - so if you have a unicode and a non-unicode version of a plugin in your archive, you should make the internal names different (which will break automatic 
//    updates from the file listing if there is only one file listing entry for both versions, unless you use the 'MS_UPDATE_REGISTER' service below)
// - Updater will install all files in the root of the archive into the plugins folder, except for langpack files that contain the FLID string which go into the root folder (same
//    folder as miranda32.exe)...all folders in the archive will also be copied to miranda's root folder, and their contents transferred into the new folders. The only exception is a 
//    special folder called 'root_files' - if there is a folder by that name in the archive, it's contents will also be copied into miranda's root folder - this is intended to be used 
//    to install additional dlls etc that a plugin may require)

// if you set Update.szUpdateURL to the following value when registering, as well as setting your beta site and version data,
// Updater will ignore szVersionURL and pbVersionPrefix, and attempt to find the file listing URL's from the backend XML data.
// for this to work, the plugin name in pluginInfo.shortName must match the file listing exactly (except for case)
#define UPDATER_AUTOREGISTER		"UpdaterAUTOREGISTER"
// Updater will also use the backend xml data if you provide URL's that reference the miranda file listing for updates (so you can use that method 
// if e.g. your plugin shortName does not match the file listing) - it will grab the file listing id from the end of these URLs

typedef struct Update_tag {
  int cbSize;     
  char *szComponentName;		// component name as it will appear in the UI (will be translated before displaying)

  char *szVersionURL;			// URL where the current version can be found (NULL to disable)
  BYTE *pbVersionPrefix;		// bytes occuring in VersionURL before the version, used to locate the version information within the URL data
								// (note that this URL could point at a binary file - dunno why, but it could :)
  int cpbVersionPrefix;			// number of bytes pointed to by pbVersionPrefix
  char *szUpdateURL;			// URL where dll/zip is located
								// set to UPDATER_AUTOREGISTER if you want Updater to find the file listing URLs (ensure plugin shortName matches file listing!)

  char *szBetaVersionURL;		// URL where the beta version can be found (NULL to disable betas)
  BYTE *pbBetaVersionPrefix;	// bytes occuring in VersionURL before the version, used to locate the version information within the URL data
  int cpbBetaVersionPrefix;		// number of bytes pointed to by pbVersionPrefix
  char *szBetaUpdateURL;		// URL where dll/zip is located

  BYTE *pbVersion;				// bytes of current version, used for comparison with those in VersionURL
  int cpbVersion;				// number of bytes pointed to by pbVersion

  char *szBetaChangelogURL;		// url for displaying changelog for beta versions
} Update;

// register a comonent with Updater
//
// wparam = 0
// lparam = (LPARAM)&Update
#define MS_UPDATE_REGISTER      "Update/Register"

// utility functions to create a version string from a DWORD or from pluginInfo
// point buf at a buffer at least 16 chars wide - but note the version string returned may be shorter
//
__inline static char *CreateVersionString(DWORD version, char *buf) {
	mir_snprintf(buf, 16, "%d.%d.%d.%d", (version >> 24) & 0xFF, (version >> 16) & 0xFF, (version >> 8) & 0xFF, version & 0xFF);
	return buf;
}

__inline static char *CreateVersionStringPlugin(PLUGININFO *pluginInfo, char *buf) {
	return CreateVersionString(pluginInfo->version, buf);
}


// register the 'easy' way - use this method if you have no beta URL and the plugin is on the miranda file listing
// NOTE: the plugin version string on the file listing must be the string version of the version in pluginInfo (i.e. 0.0.0.1,
// four numbers between 0 and 255 inclusivem, so no letters, brackets, etc.)
//
// wParam = (int)fileID				- this is the file ID from the file listing (i.e. the number at the end of the download link)
// lParam = (PLUGININFO*)&pluginInfo
#define MS_UPDATE_REGISTERFL	"Update/RegisterFL"

// this function can be used to 'unregister' components - useful for plugins that register non-plugin/langpack components and
// may need to change those components on the fly
// lParam = (char *)szComponentName
#define MS_UPDATE_UNREGISTER	"Update/Unregister"

// this event is fired when the startup process is complete, but NOT if a restart is imminent
// it is designed for status managment plugins to use as a trigger for beggining their own startup process
// wParam = lParam = 0 (unused)
// (added in version 0.1.6.0)
#define ME_UPDATE_STARTUPDONE	"Update/StartupDone"

// this service can be used to enable/disable Updater's global status control
// it can be called from the StartupDone event handler
// wParam = (BOOL)enable
// lParam = 0
// (added in version 0.1.6.0)
#define MS_UPDATE_ENABLESTATUSCONTROL	"Update/EnableStatusControl"

// An description of usage of the above service and event:
// Say you are a status control plugin that normally sets protocol or global statuses in your ModulesLoaded event handler.
// In order to make yourself 'Updater compatible', you would move the status control code from ModulesLoaded to another function, 
// say DoStartup. Then, in ModulesLoaded you would check for the existence of the MS_UPDATE_ENABLESTATUSCONTROL service.
// If it does not exist, call DoStartup. If it does exist, hook the ME_UPDATE_STARTUPDONE event and call DoStartup from there. You may
// also wish to call MS_UPDATE_ENABLESTATUSCONTROL with wParam == FALSE at this time, to disable Updater's own status control feature.

// this service can be used to determine whether updates are possible for a component with the given name
// wParam = 0
// lParam = (char *)szComponentName
// returns TRUE if updates are supported, FALSE otherwise
#define MS_UPDATE_ISUPDATESUPPORTED		"Update/IsUpdateSupported"

#endif


/////////////// Usage Example ///////////////

#ifdef EXAMPLE_CODE

// you need to #include "m_updater.h" and HookEvent(ME_SYSTEM_MODULESLOADED, OnModulesLoaded) in your Load function...

int OnModulesLoaded(WPARAM wParam, LPARAM lParam) {

	Update update = {0}; // for c you'd use memset or ZeroMemory...
	char szVersion[16];

	update.cbSize = sizeof(Update);

	update.szComponentName = pluginInfo.shortName;
	update.pbVersion = (BYTE *)CreateVersionString(&pluginInfo, szVersion);
	update.cpbVersion = strlen((char *)update.pbVersion);

	// these are the three lines that matter - the archive, the page containing the version string, and the text (or data) 
	// before the version that we use to locate it on the page
	// (note that if the update URL and the version URL point to standard file listing entries, the backend xml
	// data will be used to check for updates rather than the actual web page - this is not true for beta urls)
	update.szUpdateURL = "http://scottellis.com.au:81/test/updater.zip";
	update.szVersionURL = "http://scottellis.com.au:81/test/updater_test.html";
	update.pbVersionPrefix = (BYTE *)"Updater version ";
	
	update.cpbVersionPrefix = strlen((char *)update.pbVersionPrefix);

	// do the same for the beta versions of the above struct members if you wish to allow beta updates from another URL

	CallService(MS_UPDATE_REGISTER, 0, (WPARAM)&update);

	// Alternatively, to register a plugin with e.g. file ID 2254 on the file listing...
	// CallService(MS_UPDATE_REGISTERFL, (WPARAM)2254, (LPARAM)&pluginInfo);

	return 0;
}

#endif

/* 

   PluginUninstaller 1.1.2.1 for Miranda IM 0.3.3a and + 
   ------------------------------------------------------------------------ 
                  Developers - C/C++ Header File 
             
   Plugin Info: ---------------------------- 
   | Version:     1.1.2.1 
   | Filename:    uninstaller.dll 
   | Author:      H. Herkenrath (hrathh@users.sourceforge.net) 
   | Description: Extends the plugin options and offers the possibility 
   |              to directly remove plugins and delete all associated 
   |              settings and files. 

     Contents: -------------------------------    
   | > General Info: 
   |   - Uninstall Example/Template 
   |   - Changing displayed icon
   |   - Changing displayed docs
   |   - Message boxes on uninstall 
   |   - Service Accesibility 
   |   - Including this file 
   |
   | > Structs:
   |   - Uninstall Params                           (PLUGINUNINSTALLPARAMS)
   | 
   | > Helpers: 
   |   - Macro:    Run service while uninstalling   (PUICallService)
   |   - Function: Remove some files in directory   (PUIRemoveFilesInDirectory)
   |
   | > Events: 
   |   - Allow to uninstall a plugin                (ME_PLUGINUNINSTALLER_OKTOUNINSTALL) 
   |   - Plugin gets uninstalled                    (ME_PLUGINUNINSTALLER_UNINSTALL) 
   | 
   | > Services: 
   |   - Remove database module                     (MS_PLUGINUNINSTALLER_REMOVEDBMODULE)
   |   - Remove a setting globally                  (MS_PLUGINUNINSTALLER_REMOVEDBSETTINGGLOBALLY)
   |   - Remove skinned sound                       (MS_PLUGINUNINSTALLER_REMOVESKINSOUND)
   |   - Uninstall a plugin                         (MS_PLUGINUNISTALLER_UNISTALLPLUGIN)
   |   - Getting handles                            (MS_PLUGINUNINSTALLER_GETHANDLE)
   |


   This file is only thought for plugin developers. 
   If you only want to use "PluginUninstaller" and don't want to develop a plugin 
   or something with it you don't need this file. 

   If there are any problems or bugs with or in this file or something else 
   please mail me. My e-mail address is: hrathh@users.sourceforge.net 
   For more documentation you can use this address, too. :-) 

   If you have any whishes on some plugin uninstalling for your 
   plugin you can mail me, too. :-) 

*/ 
#ifndef M_UNINSTALLER_H 
#define M_UNINSTALLER_H 

#ifndef CallService
 #pragma message("Mistake Alert!: "m_uninstaller.h" needs to be included after "newpluginapi.h"!\n         The following errors are resulting of this mistake.\n")
#endif


// | General Info 
//  ----------------------------- 

//    Uninstall Example/Template 
//    --------------------------- 
//    Making your plugin uninstallable is very easy. 
//    Just add the following "Uninstall" function near the "Unload" function 
//    in your plugin. 
//    A template plugin is available in the source code package. 

//    Old:
//    int __declspec(dllexport) Uninstall(BOOL bIsMirandaRunning, BOOL bDoDeleteSettings, char* pszPluginPath); 

//    New:
//int __declspec(dllexport) UninstallEx(PLUGINUNINSTALLPARAMS* ppup) 
//{ 
   // Available Variables: 
   // ----------------------------- 
   // ppup->bIsMirandaRunning: 
   //    Contains if Miranda is running 
   //    (Currently this is always TRUE). 

   // ppup->bDoDeleteSettings: 
   //    Contains if the users selected 
   //    that he wants all settings be deleted. 
    
   // ppup->pszPluginsPath: 
   //    Contains the plugins directory name. 
    

   // Notes: 
   // ----------------------------- 

   // Run before "Unload" function: 
   //   -> IMPORTANT: Be careful not to write to the database or to files in "Unload" again!!!
   //   -> Perhaps create a global BOOL variable which is set to TRUE when your plugin gets uninstalled 
   //      or check of a database setting "IsInstalled" in Unload() or sth. like that 
    
   // All Miranda is still loaded 

   // Here you can do: 
   // - Delete settings group in database 
   // - Delete registry items 
   // - Delete ini-files and other settings files 
   // - Delete other files 

   // Your plugin dll gets automatically deleted 

   // Services to remove are offered: 
   // MS_PLUGINUNINSTALLER_REMOVEDBMODULE 
   // MS_PLUGINUNINSTALLER_REMOVEDBSETTINGGLOBALLY 
   // MS_PLUGINUNINSTALLER_REMOVESKINSOUND 
 

   // Getting other useful paths: 
   // ----------------------------- 
    
   // System directory: 

      //char szSysPath[MAX_PATH]; 
      //GetSystemDirectory(szSysPath, MAX_PATH); 
    

   // Windows directory: 
    
      //char szWinPath[MAX_PATH]; 
      //GetWindowsDirectory(szWinPath, MAX_PATH); 
    

   // Other directories: 
    
   //   char szPath[MAX_PATH]; 
   //   SHGetSpecialFolderPath(NULL, szPath, CSIDL_* , FALSE); 
    
   //   Some available dirs: 
   //   CSIDL_APPDATA   CSIDL_SENDTO    CSIDL_FAVORITES 
   //   CSIDL_STARTUP   CSIDL_PROFILE   CSIDL_DESKTOPDIRECTORY    

 
   // Delete Files
   //const char* apszFiles[] = {"MyPlugin_Readme.txt", "MyPlugin_License.txt", "MyPlugin_Developer.txt", "MyPlugin_Translation.txt"};
   //PUIRemoveFilesInPath(ppup->pszPluginsPath, apszFiles);

   // Delete Settings
   //if(ppup->bDoDeleteSettings == TRUE) 
   //{
      //if (ppup->bIsMirandaRunning == TRUE) // Check if it is possible to access services
      //{
        // Remove plugin's module 
        //PUIRemoveDbModule("MyPlugin"); 
       
        // Remove plugin's sounds 
        //PUIRemoveSkinSound("MySoundSetting1"); 
        //PUIRemoveSkinSound("MySoundSetting2"); 
      //} 
   //} 
     
   // Remember: 
   // Do not forget to remove your (eventually) created registry items here, too. 


   // The plugin's dll file gets deleted after returning. 
    
   // Remember: 
   // If your DLL file is additionally in use by another application (eg. Windows) 
   // you need to free the DLL *here* completely. Otherwise it can't be deleted. 
    
//   return 0; 
//} 



//    Changing displayed icon 
//    --------------------------- 
//    The icon that gets displayed on the options page is always the "first"
//    icon in your DLL file. 
//    An icon in your DLL file is the first icon when it has the lowest recource ID.
//    If you would like to have an other icon shown in the options please change your
//    icon resource IDs so that the icon you would like to have has the lowest one.
//    For example if you use MS Visual C++, open "resource.h" and change the resource define
//    of your prefered icon to the lowest icon number.


//    Changing displayed docs
//    --------------------------- 
//    The items "License" and "More Information" on the plugin details page
//    are created when the a license and/or a readme file for the plugin exists.
//    The files get detected automatically and need a special name
//    so that they get detected.
//    The text files need to be either placed in the "Plugins" directory or
//    in the "Docs" directory. Whereof the last one is the better one :-)
//
//    For the license file the following file name formatings are possible:
//    PluginName-License.txt (I personally think that this is the best naming solution... :-) )
//    PluginName_License.txt,
//	
//    For the readme file the following ones are possible:
//    PluginName-Readme.txt (Again...I like this one :-D ),
//    PluginName_Readme.txt,

//    Message boxes on uninstall
//    --------------------------- 
//    If you would like to ask the user for something to remove/uninstall
//    please hook the event ME_PLUGINUNINSTALLER_UNINSTALL and show your
//    message box there. Save the action the user chose in a
//    global BOOL variable and do the chosen action in "UninstallEx".
//    You can get the plugins options window handle with MS_PLUGINUNINSTALLER_GETHANDLE.


//    Service Accessibility 
//    --------------------------- 
//    Remember that you only can use these functions after the event ME_SYSTEM_MODULESLOADED 
//    or later because "PluginUninstaller" needs to be loaded first. 
//    Normally you only use them in your "UninstallEx" function.
//
//    IMPORTANT!:
//		Please make sure that you always use the macro PUICallService
//	    in the "UninstallEx" function instead of the CallService function.


//    Including this file
//    --------------------------- 
//    To use some of the uninstalling functionality you have to include this file 
//    into your project. 
//
//    IMPORTANT!:
//		Please make sure that you include the file "newpluginapi.h" before this one. 
//		If this isn't the case there may some compile errors come up.

      // -> Example: 
      // If your plugin is in the directory "Plugins/MyPlugin/" and 
      // this include file is in the directory "Plugins/PluginUninstaller" 
      // you can use the following: 

      //#include "../PluginUninstaller/m_uninstaller.h" 

      // If your plugin is in an directory that is different to that one just 
      // change the include path to the one you want. 





// | Structs 
//  ----------------------------- 

// --------------------------------------------- 
// -- Struct: Uninstall Params -----------------
// --------------------------------------------- 

// Struct: PLUGINUNINSTALLPARAMS
// (Gets passed to "UninstallEx" function)

typedef int (*HELPERPROC)(const char*, WPARAM, LPARAM); // Used internally (for pHelperProcAddress)
							   
typedef struct {
	BOOL bIsMirandaRunning;         // Is TRUE when Miranda is loaded and services are available (Please use PUICallService instead of CallService) 
	BOOL bDoDeleteSettings;         // Is TRUE when user wants to delete settings (If this is FALSE, please only delete your files)
	char* pszPluginsPath;           // Contains the plugin directory path
	char* pszDocsPath;              // Contains the document directory for plugins documentation (Added in version 1.1.1.0)
	char* pszIconsPath;             // Contains the icon directory for icon dlls (Added in version 1.1.2.0)
	HELPERPROC pHelperProcAddress;  // Used internally (Contains proc address for PUICallService)
} PLUGINUNINSTALLPARAMS;





// | Helper 
//  ----------------------------- 


// --------------------------------------------- 
// -- Macro: Run service while uninstalling ---- 
// --------------------------------------------- 

// Macro: PUICallService

#define PUICallService(service, wParam, lParam) (ppup->pHelperProcAddress) (service, wParam, lParam);

// Description: 
// ------------- 
// This service provides the possibility to call a Miranda
// service in the "UninstallEx" function.
// Important!: Use this macro always instead of "CallService",
// because else a crash occurs when the plugin was decativated
// and gets uninstalled

// Parameters: 
// ------------- 
// Same parameters as CallService of Miranda Core.

// Return Values: 
// -------------- 
// Return values are the same as the CallService function of Miranda Core.
// Additionaly returns CALLSERVICE_NOTFOUND if Miranda is not loaded
// which means the services are not accessable.


   // Example: 
   // ---------------------------------- 

   //if ( (bIsMirandaRunning == TRUE) && (bDoDeleteSettings == TRUE) ) 
   //{ 
      // Remove plugin's module 
      //PUICallService(MS_PLUGINUNINSTALLER_REMOVEDBMODULE, (WPARAM)"MyPlugin", 0); 
   //} 




// --------------------------------------------- 
// -- Function: Remove some files in directory - 
// --------------------------------------------- 

// Function: PUIRemoveFilesInDirectory

static BOOL __inline PUIRemoveFilesInDirectory(char* pszPath, const char* apszFiles[]);

// Description: 
// ------------- 
// This helper provides the possibility to easily
// remove specified files in a specified directory.

// Note: The last version of this helper (PUIRemoveFilesInPath)
// did not work correctly.
// Please do now always append a NULL slot to the end of your array.

// Parameters: 
// -------------
// char* pszPath = Path to the files in array
// const LPCSTR apszFiles[] = NULL-terminated array of files to be deleted.

// Return Values: 
// -------------- 
// Returns TRUE if the files could be deleted.
// FALSE if the files could not be deleted or did not exist.


static BOOL __inline PUIRemoveFilesInDirectory(char* pszPath, const char* apszFiles[]) 
{
   char szFile[MAX_PATH];
   BOOL bReturn = FALSE;
   int iFile = 0;

   while (apszFiles[iFile] != NULL)
   {
      strncpy(szFile, pszPath, SIZEOF(szFile));
      strncat(szFile, apszFiles[iFile], SIZEOF(szFile)-strlen(szFile));

      if ((BOOL)DeleteFile(szFile) == TRUE) bReturn = TRUE;
	  iFile++;
   }

   return bReturn;
}

   // Example: 
   // ---------------------------------- 

   //const char* apszFiles[] = {"File1.txt", "File2.txt", "File3.txt", NULL};
   //PUIRemoveFilesInDirectory(ppup->pszPluginsPath, apszFiles);




// | Events 
//  ----------------------------- 


// --------------------------------------------- 
// -- Event: Allow to uninstall a plugin ------- 
// --------------------------------------------- 

// Event: ME_PLUGINUNINSTALLER_OKTOUNINSTALL 

#define ME_PLUGINUNINSTALLER_OKTOUNINSTALL "PluginUninstaller/OkToUninstall" 

// Submitted Values: 
// ----------------- 
// wParam = pszPluginName (String containing the translated plugin name) 
// lParam = pszPluginFile (String containing the plugin dll file name in lower case) 

// Return Values: 
// ----------------- 
// Returning 1 on this event causes the "Remove Plugin" button to be disabled. 



// --------------------------------------------- 
// -- Event: Plugin gets uninstalled ----------- 
// --------------------------------------------- 

// Event: ME_PLUGINUNINSTALLER_UNINSTALL 

#define ME_PLUGINUNINSTALLER_UNINSTALL "PluginUninstaller/Uninstall" 

// Submitted Values: 
// ----------------- 
// wParam = pszPluginName (String containing the translated plugin name) 
// lParam = pszPluginFile (String containing the plugin dll file name in lower case) 

// Return Values: 
// ----------------- 
// Returning 1 on this event causes the uninstall process to be canceled. 

// Notice:
//    Hook this event if you would like to ask the user for something to remove/uninstall
//    and show your message box on this event. Save the action the user chose in a
//    global BOOL variable and do the chosen action in "UninstallEx".
//    You can get the plugins options window handle with MS_PLUGINUNINSTALLER_GETHANDLE.

// Other plugins can use this event to be noticed that another plugin isn't installed anylonger. 




// | Services 
//  ----------------------------- 


// --------------------------------------------- 
// -- Service: Remove database module ---------- 
// --------------------------------------------- 

// Service: MS_PLUGINUNINSTALLER_REMOVEDBMODULE 

#define MS_PLUGINUNINSTALLER_REMOVEDBMODULE "PluginUninstaller/RemoveDbModule" 

// Description: 
// ------------- 
// This service provides the possibility to delete all database modules 
// associated to your plugin. 
// The specified database module will be removed in all contacts 
// including the NULL contact.
// Remember to call it always with PUICallService in "UninstallEx" function.

// Parameters: 
// ------------- 
// wParam = (char*)pszModule				// Pointer to a string containd module name. Can't be NULL
// lParam = (const char*)apszIgnoreSettings	// NULL terminated array of strings. Can be 0 if no settings should be ignored.
											// See example 3 for more details

// Return Values: 
// -------------- 
// Returns 0 on success. 
// Nonzero if the module was not present in database. 


#ifndef UNINSTALLER_NOHELPERS

// Can only be used in "UninstallEx" function
#define PUIRemoveDbModule(pszModule)	PUICallService(MS_PLUGINUNINSTALLER_REMOVEDBMODULE, (WPARAM)pszModule, 0);

#endif 


   // Example 1: 
   // ---------------------------------- 

   //PUIRemoveDbModule("MyPlugin"); 


   // Example 2: 
   // ---------------------------------- 

   //char szModule[] = "MyModule"; 
   //PUICallService(MS_PLUGINUNINSTALLER_REMOVEDBMODULE, (WPARAM)szModule, 0); 

	
   // Example 3: 
   // ---------------------------------- 

   // This deletes all settings in the specified module exept 
   // the specified settings: "Setting1",..."Setting4"

   // char szModule[] = "MyModule"; 
   // const char* apszIgnoreSettings[] = {"Setting1", "Setting2", "Setting3", "Setting4", NULL}; 
   // PUICallService(MS_PLUGINUNINSTALLER_REMOVEDBMODULE, (WPARAM)szModule, (LPARAM)&apszIgnoreSettings); 



// --------------------------------------------- 
// -- Service: Remove a setting globally ------- 
// --------------------------------------------- 

// Service: MS_PLUGINUNINSTALLER_REMOVEDBSETTINGGLOBALLY

#define MS_PLUGINUNINSTALLER_REMOVEDBSETTINGGLOBALLY "PluginUninstaller/RemoveDbSettingGlobally" 

// Description: 
// ------------- 
// This service provides the possibility to delete a specific
// setting in database in all contacts including the NULL contact. 
// Remember to call it always with PUICallService in "UninstallEx" function.

// Parameters: 
// ------------- 
// wParam = (char*)pszModule 
// lParam = (char*)pszSetting 

// Return Values: 
// -------------- 
// Returns 0 on success. 
// Nonzero if the setting was not present in database. 


#ifndef UNINSTALLER_NOHELPERS

// Can only be used in "UninstallEx" function
#define PUIRemoveDbSettingGlobally(pszModule, pszSetting) PUICallService(MS_PLUGINUNINSTALLER_REMOVEDBSETTINGGLOBALLY, (WPARAM)pszModule, (LPARAM)pszSetting);


#endif 


   // Example 1: 
   // ---------------------------------- 

   //PUIRemoveDbSettingGlobally("MyPlugin", "MySetting"); 


   // Example 2: 
   // ---------------------------------- 

   //szModule[] = "MyPlugin"; 
   //szSetting[] = "MySetting"; 
   //PUICallService(MS_PLUGINUNINSTALLER_REMOVEDBSETTINGGLOBALLY, (WPARAM)szModule, (LPARAM)szSetting); 






// --------------------------------------------- 
// -- Service: Remove skinned sound ------------ 
// --------------------------------------------- 

// Service: MS_PLUGINUNINSTALLER_REMOVESKINSOUND 

#define MS_PLUGINUNINSTALLER_REMOVESKINSOUND "PluginUninstaller/RemoveSkinSound" 

// Description: 
// ------------- 
// This service provides the possibility to delete all your sound settings 
// associated to your plugin. 
// The specified sound will be be removed. 
// Remember to call it always with PUICallService in "UninstallEx" function.

// Parameters: 
// ------------- 
// wParam = (char*)pszSoundSetting 
// lParam = 0 

// Return Values: 
// -------------- 
// Returns 0 on success. 
// Nonzero if the sound was not present in database. 


#ifndef UNINSTALLER_NOHELPERS 

// Can only be used in "UninstallEx" function
#define PUIRemoveSkinSound(pszSoundSetting) PUICallService(MS_PLUGINUNINSTALLER_REMOVESKINSOUND, (WPARAM)pszSoundSetting, 0);

#endif


   // Example 1: 
   // ---------------------------------- 

   //PUIRemoveSkinSound("MySoundSetting"); 


   // Example 2: 
   // ---------------------------------- 

   //szSoundModule[] = "MySoundSetting"; 
   //PUICallService(MS_PLUGINUNINSTALLER_REMOVEDBMODULE, (WPARAM)szSoundSetting, 0); 





// --------------------------------------------- 
// -- Service: Uninstall a plugin -------------- 
// --------------------------------------------- 

// Service: MS_PLUGINUNINSTALLER_UNINSTALLPLUGIN 

#define MS_PLUGINUNINSTALLER_UNINSTALLPLUGIN "PluginUninstaller/UninstallPlugin" 

// Description: 
// ------------- 
// This service marks a plugin to be uninstalled at next restart of Miranda IM. 
// It uses the default value for "Delete all settings". 
// You can use this service for example when you want that your sub-plugin gets 
// also removed when your main-plugin is uninstalled. 
// Note: This service is not needed for the normal uninstalling functionality. 

// Parameters: 
// ------------- 
// wParam = (char*)pszPluginName // do not translate this! 
// lParam = (char*)pszPluginFile // without path, only file name! 

// Return Values: 
// -------------- 
// Returns always 0. 


#ifndef UNINSTALLER_NOHELPERS 

int __inline PUIUninstallPlugin(char* pszPluginName, char* pszPluginFile)
{
	return CallService(MS_PLUGINUNINSTALLER_UNINSTALLPLUGIN, (WPARAM)pszPluginName, (LPARAM)pszPluginFile);
}

#endif


   // Example 1: 
   // ---------------------------------- 

   //PUIUninstallPlugin("PluginName", "plugin.dll"); 


   // Example 2: 
   // ---------------------------------- 
    
   // hInst => Handle of a specific (your?) plugin 
   // char szPluginName[] = "YourPluginName"; 

   //char* pFileName; 
   //char szPath[MAX_PATH]; 

   //GetModuleFileName(hInst, szPath, sizeof(szPath)); 
   //pFileName = strrchr(szPath, '\\'); 
   //pFileName = pFileName+1; // Pointer arithmetic 

   //CallService(MS_PLUGINUNINSTALLER_UNINSTALLPLUGIN, (WPARAM)szPluginName, (LPARAM)pFileName); 




// ---------------------------------------------
// -- Service: Getting handles -----------------
// ---------------------------------------------

// Service: MS_PLUGINUNINSTALLER_GETHANDLE

#define MS_PLUGINUNINSTALLER_GETHANDLE "PluginUninstaller/GetHandle"

// Description:
// -------------
// This service gets a specified window/instance handle.

// Note: This service must not be used in "UninstallEx" function.
//       It is mainly thought for being used in ME_PLUGINUNINSTALLER_UNINSTALL event
//       to give out a MessageBox or something like that.   

// Parameters:
// -------------
// wParam = UINT uHandleType;
// lParam = 0

// Possible values for wParam:
#define PUIHT_HINST_PLUGIN_INSTANCE		0	// HINSTANCE of the PluginUninstaller plugin 
#define PUIHT_HWND_PLUGIN_OPTIONS		1	// HWND of the plugin options dialog (if it is loaded; else NULL)

// Return Values:
// --------------
// Returns the specified handle value.
// If no handle type is specified it returns NULL.
// The handle doesn't need to be destroyed.


#ifndef UNINSTALLER_NOHELPERS 

HANDLE __inline PUIGetHandle(UINT uHandleType)
{
	return (HANDLE)CallService(MS_PLUGINUNINSTALLER_GETHANDLE, uHandleType, 0);
}

#endif


	// Example
	// ----------------------------------

	//HWND hwndDlg;
	//hwndDlg = (HWND)PUIGetHandle(PUIHT_HWND_PLUGIN_OPTIONS);





#endif // M_UNINSTALLER_H 

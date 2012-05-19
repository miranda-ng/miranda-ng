#include "common.h"
#include "scan.h"

typedef PLUGININFO * (__cdecl * Miranda_Plugin_Info) ( DWORD mirandaVersion );
typedef PLUGININFOEX * (__cdecl * Miranda_Plugin_Info_Ex) ( DWORD mirandaVersion );

struct AlternateShortName
{
    const char* from;
    const char* to;
};

static const AlternateShortName alternate_shortname_map[] =
{
	{ "Version Informations", "Version Information" },
	{ "Jabber Protocol", "JabberG Protocol" },
	{ "Jabber Protocol (Unicode)", "JabberG Protocol (Unicode)" },
#ifdef _UNICODE
	{ "PopUp Interoperability", "PopUp Plus (Unicode)" },
#else
	{ "PopUp Interoperability", "PopUp Plus" },
#endif
	//{ "Messaging Style Conversation", "nConvers++" }, // will this conflict with other nConvers'?
	{ "MimQQ-libeva", "MirandaQQ (libeva Version)" },

	// grr
	{ "Updater", __PLUGIN_NAME },
	{ "Updater (Unicode)", __PLUGIN_NAME },
};

char* findAlternateShortName(const char* name)
{
    for (int i = 0; i < SIZEOF(alternate_shortname_map); ++i)
    {
        if (strcmp(name, alternate_shortname_map[i].from) == 0)
            return mir_strdup(alternate_shortname_map[i].to);
    }
#ifdef _UNICODE
	if (!strstr(name, "Unicode"))
	{
		char *buf = (char*)mir_alloc(256);
		mir_snprintf(buf, 256, "%s (Unicode)", name);
		return buf;
	}
#endif
    return NULL;
}

bool valDllName(TCHAR* name)
{
	TCHAR *p = _tcsrchr(name, '.');
	return p && _tcsicmp(p, _T(".dll")) == 0;
}

void ScanPlugins(FilenameMap *fn_map, UpdateList *update_list) 
{
	if (!XMLDataAvailable(MC_PLUGINS)) return;

	TCHAR plugins_folder[MAX_PATH], dll_path[MAX_PATH];
	TCHAR *dll_name;
	Miranda_Plugin_Info dll_info_func;
	Miranda_Plugin_Info_Ex dll_info_func_ex;
	DWORD mirandaVersion = (DWORD)CallService(MS_SYSTEM_GETVERSION, 0, 0);
	PLUGININFO *pluginInfo;

	GetRootDir(plugins_folder);
	_tcscat(plugins_folder, _T("\\Plugins"));

	_tcscpy(dll_path, plugins_folder);
	_tcscat(dll_path, _T("\\"));

	// set dll_name to point into the dll_path string, at the point where we can write the plugin name
	// to end up with the full dll path
	dll_name = dll_path + _tcslen(dll_path);

	// add filemask
	_tcscat(plugins_folder, _T("\\*.dll"));

	WIN32_FIND_DATA findData;
	HANDLE hFileSearch = FindFirstFile(plugins_folder, &findData);
	if (hFileSearch != INVALID_HANDLE_VALUE) 
	{
		do {
			HMODULE hModule = NULL;
			bool notLoaded = false;
			if (valDllName(findData.cFileName))
			{
				_tcscpy(dll_name, findData.cFileName);
				if (hModule = GetModuleHandle(dll_path)) 
				{
					hModule = LoadLibrary(dll_path);
					notLoaded = true;
				}
			}
			if (hModule) 
            {
				dll_info_func = (Miranda_Plugin_Info)GetProcAddress(hModule, "MirandaPluginInfo");
				dll_info_func_ex = (Miranda_Plugin_Info_Ex)GetProcAddress(hModule, "MirandaPluginInfoEx");
				if((dll_info_func_ex && (pluginInfo = (PLUGININFO *)dll_info_func_ex(mirandaVersion))) || (dll_info_func && (pluginInfo = dll_info_func(mirandaVersion)))) 
                {
					// *** This is a dodgy and unfair hack...
					// In order to disable new plugins that may be unintentionally installed with an update,
					// updater will check for the 'plugindisabled' setting for each dll. The problem is that
					// this setting may not be there for running plugins - and isn't there for new ones. So,
					// we'll disable anything new when the setting isn't found anyway - but we write the
					// value below for all plugins so that we can expect to find it
					char *lowname = _strlwr(mir_t2a(findData.cFileName));
					if(DBGetContactSettingByte(0, "PluginDisable", lowname, 255) == 255) // setting not present
						DBWriteContactSettingByte(0, "PluginDisable", lowname, 0);
					mir_free(lowname);

					bool found = false;
					char* alternateName = findAlternateShortName(pluginInfo->shortName);
					if (alternateName) 
                    {
                        int file_id = FindFileID(alternateName, MC_PLUGINS, update_list);
						if (file_id != -1) 
                        {
							found = true;
							if (FindFileInList(pluginInfo->shortName) >= 0)
								RegisterForFileListing(file_id, pluginInfo->shortName, pluginInfo->version, true, MC_PLUGINS);
							else
								RegisterForFileListing(file_id, alternateName, pluginInfo->version, true, MC_PLUGINS);

                            if (fn_map)
                            {
                                FileNameStruct* fns = fn_map->find((FileNameStruct*)&file_id);
                                if (fns == NULL)
                                {
                                    fns = new FileNameStruct(file_id);
                                    fn_map->insert(fns);
                                }
                                fns->list.insert(mir_tstrdup(findData.cFileName));
                            }
						}
						mir_free(alternateName);
					} 

                    if (!found) 
                    {
						int file_id = FindFileID(pluginInfo->shortName, MC_PLUGINS, update_list);
						if (file_id != -1) 
                        {
							RegisterForFileListing(file_id, pluginInfo, true);
                            if (fn_map)
                            {
                                FileNameStruct* fns = fn_map->find((FileNameStruct*)&file_id);
                                if (fns == NULL)
                                {
                                    fns = new FileNameStruct(file_id);
                                    fn_map->insert(fns);
                                }
                                fns->list.insert(mir_tstrdup(findData.cFileName));
                            }
						}
					}
				}
				if (notLoaded) FreeLibrary(hModule);
			}
		} while(FindNextFile(hFileSearch, &findData));
		FindClose(hFileSearch);
	}
}

typedef struct LangpackData_tag {
	DWORD version;
	char *fl_name;
} LangpackData;


bool GetLangpackData(const TCHAR *filename, LangpackData *ld) 
{
    char line[1024], *ver, *conv;

    FILE *fp = _tfopen(filename, _T("r"));
    if (fp == NULL) return false;

    while (fgets(line, sizeof(line), fp))
    {
        conv = strstr(line, "; FLID:");
		if (conv) 
        {
            conv += 7;

            while (*conv == ' ') conv++;
            
            ver = strchr(conv, 0) + 1;
            while (ver >= conv) if (*ver != ' ') break; else *ver-- = 0;

            ver = strrchr(conv, ' ');
            if (ver) 
            { 
                *ver = 0; 
                VersionFromString(++ver, &ld->version);
            }
            ld->fl_name = mir_strdup(conv);
            fclose(fp);
            return true;
        }
    }
    fclose(fp);
    return false;
}

void ScanLangpacks(FilenameMap *fn_map, UpdateList *update_list) 
{
	if(!XMLDataAvailable(MC_LOCALIZATION)) return;

	TCHAR mir_folder[MAX_PATH], langpack_path[MAX_PATH], *langpack_name;

	GetRootDir(mir_folder); _tcscat(mir_folder, _T("\\"));
	_tcscpy(langpack_path, mir_folder);

	// set langpack_name to point into the langpack_name string, at the point where we can write the file name
	// to end up with the full path
	langpack_name = langpack_path + _tcslen(langpack_path);

	// add filemask
	_tcscat(mir_folder, _T("langpack_*.txt"));

	WIN32_FIND_DATA findData;
	HANDLE hFileSearch = FindFirstFile(mir_folder, &findData);
	if (hFileSearch != INVALID_HANDLE_VALUE) 
    {
		do {
			_tcscpy(langpack_name, findData.cFileName);
			LangpackData ld = {0};
			if (GetLangpackData(langpack_path, &ld)) 
            {
                int file_id = FindFileID(ld.fl_name, MC_LOCALIZATION, update_list);
				if (file_id != -1) 
                {
					RegisterForFileListing(file_id, ld.fl_name, ld.version, true, MC_LOCALIZATION);
					if (fn_map) 
                    {
                        FileNameStruct* fns = fn_map->find((FileNameStruct*)&file_id);
                        if (fns == NULL)
                        {
                            fns = new FileNameStruct(file_id);
                            fn_map->insert(fns);
                        }
                        fns->list.insert(mir_tstrdup(findData.cFileName));
                    }
				}
				mir_free(ld.fl_name);
			}
		} while(FindNextFile(hFileSearch, &findData));
		FindClose(hFileSearch);
	}
}

bool RearrangeDllsWorker(char *shortName, StrList &filenames, TCHAR *basedir) 
{
	bool dll_enabled = false;
	BYTE disabled_val;

	TCHAR file_path[MAX_PATH];

	Miranda_Plugin_Info dll_info_func;
	Miranda_Plugin_Info_Ex dll_info_func_ex;
	DWORD mirandaVersion = (DWORD)CallService(MS_SYSTEM_GETVERSION, 0, 0);
	PLUGININFO *pluginInfo;
	HMODULE hModule;

	// add filemask
	mir_sntprintf(file_path, SIZEOF(file_path), _T("%s\\*.dll"), basedir);

	WIN32_FIND_DATA findData;
	HANDLE hFileSearch = FindFirstFile(file_path, &findData);
	if (hFileSearch != INVALID_HANDLE_VALUE) 
	{
		do 
		{
			mir_sntprintf(file_path, SIZEOF(file_path), _T("%s\\%s"), basedir, findData.cFileName);
			if (valDllName(findData.cFileName) && (hModule = LoadLibrary(file_path))) 
			{
				dll_info_func = (Miranda_Plugin_Info)GetProcAddress(hModule, "MirandaPluginInfo");
				dll_info_func_ex = (Miranda_Plugin_Info_Ex)GetProcAddress(hModule, "MirandaPluginInfoEx");
				if((dll_info_func_ex && (pluginInfo = (PLUGININFO *)dll_info_func_ex(mirandaVersion))) || (dll_info_func && (pluginInfo = dll_info_func(mirandaVersion)))) 
                {
					bool found = !_stricmp(pluginInfo->shortName, shortName);
					if (!found)
					{
						char* alternateName = findAlternateShortName(pluginInfo->shortName);
						found = alternateName && !_stricmp(alternateName, shortName);
						mir_free(alternateName);
					}

					if (found) 
                    {
						bool moved = false;
						TCHAR* newname = NULL;
						for (int j = 0; j < filenames.getCount(); j++) 
                        {
							TCHAR new_filename[MAX_PATH];
                            TCHAR* fileName = filenames[j];
							mir_sntprintf(new_filename, SIZEOF(new_filename), _T("%s\\%s"), basedir, fileName);

							// disable any new plugins (i.e. not installed before) that somehome got into the
							// dowloaded archives (e.g. loadavatars comes with loadavatarsw - installing both is not good!)
                            char *temp_str = _strlwr(mir_t2a(fileName));
							disabled_val = DBGetContactSettingByte(0, "PluginDisable", temp_str, 255);
							if (disabled_val == 255) { // assume this means setting not in db (should be 1 or 0)
								DBWriteContactSettingByte(0, "PluginDisable", temp_str, 1);
								disabled_val = 1;
							}
                            mir_free(temp_str);

							dll_enabled |= (disabled_val == 0);						

							if (!moved) 
							{
								if (_tcsicmp(findData.cFileName, fileName))
									MoveFile(file_path, new_filename);

                                mir_free(newname); newname = mir_tstrdup(new_filename);
								moved = true;
							} else
								CopyFile(newname, new_filename, FALSE);
						}
                        mir_free(newname);
						FreeLibrary(hModule);
						break;
					}
				}
				FreeLibrary(hModule);
			} 
		} 
		while(FindNextFile(hFileSearch, &findData));
		FindClose(hFileSearch);
	}
	return dll_enabled;
}


bool RearrangeDlls(char *shortName, StrList &filenames) 
{
	bool dll_enabled = false;

	TCHAR dir[MAX_PATH];
	mir_sntprintf(dir, SIZEOF(dir), _T("%s\\Plugins"), options.temp_folder);

	return 
		RearrangeDllsWorker(shortName, filenames, options.temp_folder) ||
		RearrangeDllsWorker(shortName, filenames, dir);
}

bool RearrangeLangpacks(char *shortName, StrList &filenames) 
{
	
	TCHAR file_path[MAX_PATH], updates_folder[MAX_PATH], new_filename[MAX_PATH], *langpack_name;
	// do exactly the same thing again, for the updates/plugins folder... :(

	mir_sntprintf(file_path, SIZEOF(file_path), _T("%s\\"), options.temp_folder);
	langpack_name = file_path + _tcslen(file_path);

	// add filemask
	mir_sntprintf(updates_folder, SIZEOF(updates_folder), _T("%s\\langpack_*.txt"), options.temp_folder);

	WIN32_FIND_DATA findData;
	HANDLE hFileSearch = FindFirstFile(updates_folder, &findData);
	if(hFileSearch != INVALID_HANDLE_VALUE) {
		do {
			_tcscpy(langpack_name, findData.cFileName);

			LangpackData ld = {0};
			if (GetLangpackData(file_path, &ld)) 
            {
				if (_stricmp(ld.fl_name, shortName) == 0) 
                {
					bool moved = false;
					TCHAR *newname = NULL;
					for (int j = 0; j < filenames.getCount(); j++) 
                    {
                        TCHAR *fileName = filenames[j];
						mir_sntprintf(new_filename, SIZEOF(new_filename), _T("%s\\%s"), options.temp_folder, fileName);
						
                        if (!moved) 
                        {
							if (_tcscmp(findData.cFileName, fileName) != 0)
								MoveFile(file_path, new_filename);
                            mir_free(newname); newname = mir_tstrdup(new_filename);
							moved = true;
						} else
							CopyFile(newname, new_filename, FALSE);
					}
                    mir_free(newname);
					break;
				}
			}
		} while(FindNextFile(hFileSearch, &findData));
		FindClose(hFileSearch);
	}

	return true;
}

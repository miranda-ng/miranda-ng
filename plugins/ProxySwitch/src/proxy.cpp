/*
proxySwitch

The plugin watches IP address changes, reports them via popups and adjusts
the proxy settings of Miranda and Internet Explorer accordingly.
*/

#include "stdafx.h"

/* ################################################################################ */

int Enum_Settings(const char *szSetting, LPARAM lParam)
{
	PPROXY_SETTINGS ps = (PPROXY_SETTINGS)lParam;

	if (mir_strcmp(szSetting, "NLUseProxy") != 0 && mir_strcmpi(szSetting, "useproxy") != 0)
		return 0;

	if (ps->count >= ps->_alloc) {
		ps->_alloc += 10;
		ps->item = (PPROXY_SETTING)mir_realloc(ps->item, ps->_alloc * sizeof(PROXY_SETTING));
		ZeroMemory(&(ps->item[ps->count]), 10 * sizeof(PROXY_SETTING));
	}
	mir_strncpy(ps->item[ps->count].ModuleName, ps->_current_module, MAXLABELLENGTH - 1);
	mir_strncpy(ps->item[ps->count].SettingName, szSetting, MAXLABELLENGTH - 1);
	ps->count++;

	return 0;
}

int Enum_Modules(const char *szModuleName, uint32_t, LPARAM lParam) 
{
	//DBCONTACTENUMSETTINGS e;
	MCONTACT hContact = NULL;

	((PPROXY_SETTINGS)lParam)->_current_module = szModuleName;

	//e.pfnEnumProc = Enum_Settings;
	//e.lParam = lParam;
	//e.szModule = szModuleName;
	db_enum_settings(hContact, (DBSETTINGENUMPROC)Enum_Settings, szModuleName, (void*)lParam);
	//CallService(MS_DB_CONTACT_ENUMSETTINGS, (WPARAM)hContact,(LPARAM)&e);

	return 0;
}

/* ################################################################################ */

void Create_Proxy_Settings_List(PPROXY_SETTINGS ps) 
{
	ZeroMemory(ps, sizeof(PROXY_SETTINGS));
	ps->_alloc = 10;
	ps->item = (PPROXY_SETTING)mir_alloc(ps->_alloc * sizeof(PROXY_SETTING));
	ZeroMemory(ps->item, ps->_alloc * sizeof(PROXY_SETTING));

	db_enum_modules((DBMODULEENUMPROC)Enum_Modules);
	//CallService(MS_DB_MODULES_ENUM, (WPARAM)ps, (LPARAM)Enum_Modules );

	ps->_alloc = ps->count + 1;
	ps->item = (PPROXY_SETTING)mir_realloc(ps->item, ps->_alloc * sizeof(PROXY_SETTING));
	ZeroMemory(&(ps->item[ps->count]), sizeof(PROXY_SETTING));
	ps->_current_module = NULL;
}

/* ################################################################################ */

void Free_Proxy_Settings_List(PPROXY_SETTINGS ps) 
{
	if (ps->item)
		mir_free(ps->item);
	ZeroMemory(ps, sizeof(PROXY_SETTINGS));
}

/* ################################################################################ */

char Get_Miranda_Proxy_Status(void) 
{
	PROXY_SETTINGS ps;
	int i, p;
	char proxy;
	proxy = PROXY_NO_CONFIG;

	Create_Proxy_Settings_List(&ps);
	for (i = 0; i < ps.count; i++) {
		p = db_get_b(0, ps.item[i].ModuleName, ps.item[i].SettingName, FALSE);
		if (proxy == PROXY_NO_CONFIG) {
			proxy = p;
			continue;
		}
		if (proxy != p) {
			proxy = PROXY_MIXED;
			break;
		}
	}
	Free_Proxy_Settings_List(&ps);
	return proxy;
}

/* ################################################################################ */

void Set_Miranda_Proxy_Status(char proxy) 
{
	PROXY_SETTINGS ps;
	NETLIBUSERSETTINGS nlus;
	int i;

	if (proxy < 0)
		return;
	Create_Proxy_Settings_List(&ps);
	for (i = 0; i < ps.count; i++) {
		if (ps.item[i].SettingName[0] != 0)
			db_set_b(0, ps.item[i].ModuleName, ps.item[i].SettingName, proxy);
		ZeroMemory(&nlus, sizeof(nlus));
		nlus.cbSize = sizeof(nlus);
		if (Netlib_GetUserSettingsByName(ps.item[i].ModuleName, &nlus)) {
			nlus.useProxy = proxy;
			nlus.szProxyAuthPassword = NEWSTR_ALLOCA(nlus.szProxyAuthPassword);
			nlus.szProxyAuthUser = NEWSTR_ALLOCA(nlus.szProxyAuthUser);
			nlus.szProxyServer = NEWSTR_ALLOCA(nlus.szProxyServer);
			nlus.szIncomingPorts = NEWSTR_ALLOCA(nlus.szIncomingPorts);
			nlus.szOutgoingPorts = NEWSTR_ALLOCA(nlus.szOutgoingPorts);
			Netlib_SetUserSettingsByName(ps.item[i].ModuleName, &nlus);
		}
	}
	Free_Proxy_Settings_List(&ps);
}

/* ################################################################################ */

char Get_IE_Proxy_Status(void)
{
	INTERNET_PER_CONN_OPTION_LIST    list;
	INTERNET_PER_CONN_OPTION         option[1];
	unsigned long                    nSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);

	option[0].dwOption = INTERNET_PER_CONN_FLAGS;

	list.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);
	list.pszConnection = NULL;
	list.dwOptionCount = 1;
	list.dwOptionError = 0;
	list.pOptions = option;

	if (!InternetQueryOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, &nSize))
		return -1;

	return option[0].Value.dwValue & PROXY_TYPE_PROXY ? 1 : 0;
}

/* ################################################################################ */

void Set_IE_Proxy_Status(char proxy)
{
	INTERNET_PER_CONN_OPTION_LIST    list;
	INTERNET_PER_CONN_OPTION         option[1];
	unsigned long                    nSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);

	if (proxy < 0)
		return;
	option[0].dwOption = INTERNET_PER_CONN_FLAGS;
	option[0].Value.dwValue = proxy ? PROXY_TYPE_PROXY | PROXY_TYPE_DIRECT : PROXY_TYPE_DIRECT;

	list.dwSize = sizeof(INTERNET_PER_CONN_OPTION_LIST);
	list.pszConnection = NULL;
	list.dwOptionCount = 1;
	list.dwOptionError = 0;
	list.pOptions = option;

	if (!InternetSetOption(NULL, INTERNET_OPTION_PER_CONNECTION_OPTION, &list, nSize))
		return;

	InternetQueryOption(NULL, INTERNET_OPTION_SETTINGS_CHANGED, NULL, 0);
	InternetQueryOption(NULL, INTERNET_OPTION_REFRESH, NULL, 0);
}

/* ################################################################################ */

char Get_Firefox_Proxy_Status(void)
{
	wchar_t path[MAX_PATH];
	wchar_t prefs[MAX_PATH];
	char line[500];
	FILE *fP;
	struct _stat info;
	struct _wfinddata_t dir;
	long hFile;
	char *setting;
	int p, proxy;

	ZeroMemory(&info, sizeof(info));
	proxy = PROXY_NO_CONFIG;
	if (!SHGetSpecialFolderPath(NULL, path, CSIDL_APPDATA, 0))
		return proxy;
	mir_wstrcat(path, L"\\Mozilla\\Firefox\\Profiles\\*");
	if ((hFile = _wfindfirst(path, &dir)) != -1L) {
		do {
			if (!(dir.attrib & _A_SUBDIR) || dir.name[0] == '.')
				continue;
			mir_wstrcpy(prefs, path);
			prefs[mir_wstrlen(prefs) - 1] = 0;
			mir_wstrcat(prefs, dir.name);
			mir_wstrcat(prefs, L"\\prefs.js");
			if ((fP = _wfopen(prefs, L"r")) != NULL) {
				p = 0;
				while (fgets(line, 500, fP)) {
					if ((setting = strstr(line, "user_pref(\"network.proxy.type\",")) != NULL) {
						setting += 31;
						p = atoi(setting);
						p = p == 3 ? 0 : p > 0;
						break;
					}
				}
				fclose(fP);
				proxy = proxy == -2 ? p : (proxy == p ? p : -1);
			}
		} while (_wfindnext(hFile, &dir) == 0);
		_findclose(hFile);
	}
	return proxy;
}

/* ################################################################################ */

void Set_Firefox_Proxy_Status(char proxy) 
{
	wchar_t path[MAX_PATH];
	wchar_t prefsR[MAX_PATH];
	wchar_t prefsW[MAX_PATH];
	char line[500];
	FILE *fR, *fW;
	struct _stat info;
	struct _wfinddata_t dir;
	long hFile;
	char done;

	ZeroMemory(&info, sizeof(info));
	if (!SHGetSpecialFolderPath(NULL, path, CSIDL_APPDATA, 0))
		return;
	mir_wstrcat(path, L"\\Mozilla\\Firefox\\Profiles\\*");
	if ((hFile = _wfindfirst(path, &dir)) != -1L) {
		do {
			if (!(dir.attrib & _A_SUBDIR) || dir.name[0] == '.')
				continue;
			mir_wstrcpy(prefsR, path);
			prefsR[mir_wstrlen(prefsR) - 1] = 0;
			mir_wstrcat(prefsR, dir.name);
			mir_wstrcat(prefsR, L"\\prefs.js");
			done = 0;
			if ((fR = _wfopen(prefsR, L"r")) != NULL) {
				mir_wstrcpy(prefsW, prefsR);
				mir_wstrcat(prefsW, L"~");
				if ((fW = _wfopen(prefsW, L"w")) != NULL) {
					while (fgets(line, 500, fR)) {
						if (strstr(line, "\"network.proxy.type\""))
							continue;
						if (strstr(line, "\"network.proxy") && !done) {
							fprintf(fW, "user_pref(\"network.proxy.type\", %d);\n", proxy);
							done = 1;
						}
						fprintf(fW, "%s", line);
					}
					if (!done) {
						fprintf(fW, "user_pref(\"network.proxy.type\", %d);\n", proxy);
						done = 1;
					}
					fclose(fW);
				}
				fclose(fR);
			}
			if (done) {
				_wremove(prefsR);
				_wrename(prefsW, prefsR);
			}
		} while (_wfindnext(hFile, &dir) == 0);
		_findclose(hFile);
	}
}

/* ################################################################################ */

char Firefox_Installed(void) 
{
	wchar_t path[MAX_PATH];
	struct _stat info;
	ZeroMemory(&info, sizeof(info));

	if (SHGetSpecialFolderPath(NULL, path, CSIDL_APPDATA, 0)) {
		mir_wstrcat(path, L"\\Mozilla\\Firefox\\Profiles");
		if (_wstat(path, &info) == 0 && (info.st_mode & _S_IFDIR) == _S_IFDIR)
			return 1;
	}
	return 0;
}

/* ################################################################################ */

void Disconnect_All_Protocols(PPROTO_SETTINGS settings, int disconnect) 
{
	int count = 0, c, i, status;
	PROTOCOLDESCRIPTOR **plist;

	Proto_EnumProtocols(&c, &plist);

	ZeroMemory(settings, sizeof(PROTO_SETTINGS));
	settings->item = (PPROTO_SETTING)mir_alloc(c * sizeof(PROTO_SETTING));
	ZeroMemory(settings->item, c * sizeof(PROTO_SETTING));

	for (i = 0; i < c; i++) {
		if (plist[i]->type != PROTOTYPE_PROTOCOL)
			continue;
		if (CallProtoService(plist[i]->szName, PS_GETCAPS, PFLAGNUM_2, 0) == 0)
			continue;
		status = CallProtoService(plist[i]->szName, PS_GETSTATUS, 0, 0);
		mir_strncpy(settings->item[count].ProtoName, plist[i]->szName, MAXLABELLENGTH - 1);
		if (status != ID_STATUS_OFFLINE && disconnect) {
			CallProtoService(plist[i]->szName, PS_SETSTATUS, ID_STATUS_OFFLINE, 0);
		}
		if (status < MAX_CONNECT_RETRIES) 
			status = ID_STATUS_ONLINE;
		if (status == ID_STATUS_OFFLINE) 
			status = ID_STATUS_ONLINE;
		settings->item[count].Status = status;
		count++;
	}
	settings->count = count;
}

/* ################################################################################ */

void Connect_All_Protocols(PPROTO_SETTINGS settings) 
{
	int i;
	for (i = 0; i < settings->count; i++) {
		CallProtoService(settings->item[i].ProtoName, PS_SETSTATUS, settings->item[i].Status, 0);
	}
	mir_free(settings->item);
	ZeroMemory(settings, sizeof(PROTO_SETTINGS));
}

#include "common.h"
#include "socket.h"

void unzip_mem(char* buf, int len, TCHAR* dest);

bool GetFile(char *url, TCHAR *temp_folder, char *plugin_name, char *version, bool dlls_only, int recurse_count /*=0*/) {
	if(recurse_count > MAX_REDIRECT_RECURSE) {
		NLog("GetFile: error, too many redirects");
		return false;
	}

	TCHAR save_file[MAX_PATH];

	if(url == 0 || temp_folder == 0 || plugin_name == 0)
		return false;

	// ensure temp_folder exists
	if(!CreatePath(options.temp_folder)) {
		NLogF("GetFile: error creating temp folder, code %u", GetLastError());
		return false;
	}

	// ensure zip_folder exists, if necessary
	if(options.save_zips && !CreatePath(options.zip_folder)) {
		NLogF("GetFile: error creating zip folder, code %u", GetLastError());
		return false;
	}

	TCHAR *temp_str = GetTString(plugin_name);
	mir_sntprintf(save_file, SIZEOF(save_file), _T("%s\\%s"), temp_folder, temp_str);
	mir_free(temp_str);
	if (version) 
	{
		temp_str = GetTString(version);
		_tcscat(save_file, _T("_"));
		_tcscat(save_file, temp_str);
		mir_free(temp_str);
	}
	// copt extension from url
	char *ext = strrchr(url, '.');
	if(ext && *ext && strcmp(ext, ".dll") == 0) {
		_tcscat(save_file, _T(".dll"));
	} else { // default to zip extension (e.g. miranda fl)
		_tcscat(save_file, _T(".zip"));
		ext = ".zip";
	}

	// replace version text in URL
	char tmp_url[1024];
	if (version != NULL) {
		char *p;
		size_t pos = 0;
		size_t version_len = strlen(version);
		while ((p = strstr(url, "%VERSION%")) != NULL && (p - url + version_len < sizeof(tmp_url) - 1)) {
			strncpy(&tmp_url[pos], url, p - url);
			pos += p - url;
			strcpy(&tmp_url[pos], version);
			pos += version_len;
			url += p - url + 9; // 9 == strlen("%VERSION%")
		}
		if (strlen(url) < sizeof(tmp_url) - 1) {
			strcpy(&tmp_url[pos], url);
			pos += strlen(url);
		}
		tmp_url[pos] = 0;
		url = tmp_url;
	}


	NETLIBHTTPREQUEST req = {0};

	req.cbSize = sizeof(req);
	req.requestType = REQUEST_GET;
	req.szUrl = url;
	req.flags = NLHRF_NODUMP | NLHRF_HTTP11;
	req.nlc = hNetlibHttp;

	if (CallService(MS_SYSTEM_GETVERSION, 0, 0) >= PLUGIN_MAKE_VERSION(0,9,0,5))
		req.flags |= NLHRF_PERSISTENT | NLHRF_REDIRECT;

	NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&req);

	if (resp) 
	{
		hNetlibHttp = resp->nlc;
		if (resp->resultCode == 200) 
		{
			if (ext && *ext && _stricmp(ext, ".zip") == 0) 
			{
				if (!options.no_unzip) 
					unzip_mem(resp->pData, resp->dataLength, temp_folder); 
										
				if (options.save_zips) 
				{
					TCHAR save_archive[MAX_PATH];
					mir_sntprintf(save_archive, SIZEOF(save_archive), _T("%s%s"), options.zip_folder, _tcsrchr(save_file, '\\'));

					HANDLE hSaveFile = CreateFile(save_archive, GENERIC_WRITE, FILE_SHARE_WRITE, 0, 
						CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
					if (hSaveFile != INVALID_HANDLE_VALUE) 
					{
						unsigned long bytes_written;
						WriteFile(hSaveFile, resp->pData, resp->dataLength, &bytes_written, NULL); 
						CloseHandle(hSaveFile);
					}
					else
						NLogF("GetFile: error creating file, code %u", GetLastError());
				} 
				
				if(dlls_only) 
				{
					NLog("Deleting non-dlls");
					DeleteNonDlls(temp_folder);
				}

			}
			else
			{
				HANDLE hSaveFile = CreateFile(save_file, GENERIC_WRITE, FILE_SHARE_WRITE, 0, 
					CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
				if (hSaveFile != INVALID_HANDLE_VALUE) 
				{
					unsigned long bytes_written;
					WriteFile(hSaveFile, resp->pData, resp->dataLength, &bytes_written, NULL); 
					CloseHandle(hSaveFile);
				}
				else
					NLogF("GetFile: error creating file, code %u", GetLastError());
			}

			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
			return true;
		} 
		else if (resp->resultCode >= 300 && resp->resultCode < 400) 
		{
			// get new location
			bool ret = false;
			for (int i = 0; i < resp->headersCount; i++) 
			{
				if (_stricmp(resp->headers[i].szName, "Location") == 0) 
				{
					ret = GetFile(resp->headers[i].szValue, temp_folder, plugin_name, version, dlls_only, recurse_count + 1);
					break;
				}
			}
			CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
			return ret;
		} 
		else 
		{
			TCHAR buff[1024], *tmp;
			mir_sntprintf(buff, SIZEOF(buff), TranslateT("Failed to download \"%s\" - Invalid response, code %d"), (tmp = mir_a2t(plugin_name)), resp->resultCode);
			ShowError(buff);
			mir_free(tmp);
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	} 
	else 
	{
		hNetlibHttp = NULL;
		int err = GetLastError();
		if (err && !Miranda_Terminated()) 
		{
			TCHAR buff[1024], *tmp;
			int len = mir_sntprintf(buff, SIZEOF(buff), TranslateT("Failed to download \"%s\": "), (tmp = mir_a2t(plugin_name)));
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, err, 0, buff + len, 512 - len, 0);
			ShowError(buff);
			mir_free(tmp);
		}
	}

	return false;
}

char *CheckVersionURL(char *url, BYTE *pbPrefixBytes, int cpbPrefixBytes, BYTE *pbVersionBytes, int cpbVersionBytes)
{
	if (url == 0 || pbPrefixBytes == 0 || cpbPrefixBytes == 0 || pbVersionBytes == 0 || cpbVersionBytes == 0)
		return 0;

	char *ret = NULL;
	NETLIBHTTPREQUEST req = {0};

	req.cbSize = sizeof(req);
	req.requestType = REQUEST_GET;
	req.szUrl = url;
	req.flags = NLHRF_DUMPASTEXT | NLHRF_HTTP11;
	req.nlc = hNetlibHttp;

	if (CallService(MS_SYSTEM_GETVERSION, 0, 0) >= PLUGIN_MAKE_VERSION(0,9,0,5))
		req.flags |= NLHRF_PERSISTENT | NLHRF_REDIRECT;

	NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION, 
		(WPARAM)hNetlibUser, (LPARAM)&req);

	if (resp) 
	{
		hNetlibHttp = resp->nlc;
		if (resp->resultCode == 200) 
		{
			// find the location of the prefix
			char* ver = (char*)memmem(resp->pData, resp->dataLength, pbPrefixBytes, cpbPrefixBytes);
			if (ver)
			{
				int len = resp->dataLength - ((ver += cpbPrefixBytes) - resp->pData);
				if (cpbVersionBytes <= len && memcmp(ver, pbVersionBytes, cpbVersionBytes) == 0)
				{
					// same version as current
					CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
					return 0;
				} 
				else 
				{
					DWORD ver_current, ver_potential;
					char *buff = (char *)alloca(cpbVersionBytes + 1);
					memcpy(buff, (char *)pbVersionBytes, cpbVersionBytes);
					buff[cpbVersionBytes] = 0;

					// this is safe because pData finishes with a zero always (according to m_netlib.h docs)
					if (VersionFromString(buff, &ver_current) && VersionFromString(ver, &ver_potential)) 
					{
						switch(options.ver_req) 
						{
						case VR_MAJOR:
							ver_current &= 0xFF000000;
							ver_potential &= 0xFF000000;
							break;
						case VR_MINOR:
							ver_current &= 0xFFFF0000;
							ver_potential &= 0xFFFF0000;
							break;
						case VR_RELEASE:
							ver_current &= 0xFFFFFF00;
							ver_potential &= 0xFFFFFF00;
							break;
						case VR_BUILD:
							break;
						}

						// we can covert the versions to DWORDS, so compare...
						if (ver_current < ver_potential) 
						{
							char buff2[16];
							CreateVersionString(ver_potential, buff2);
							ret = _strdup(buff2);
						} 
					} 
					else  // invalid version(s), but different from current - assume it's an update
						ret = _strdup(Translate("Yes"));
				}
			}
		} 
		else if (resp->resultCode == 302) // redirect
		{
			// get new location
			for (int i = 0; i < resp->headersCount; i++) 
			{
				if(_stricmp(resp->headers[i].szName, "Location") == 0) 
				{
					ret = CheckVersionURL(resp->headers[i].szValue, pbPrefixBytes, cpbPrefixBytes, pbVersionBytes, cpbVersionBytes);
					break;
				}
			}
		} 
		else 
			NLogF("CheckVersionURL: error, http result code %d", resp->resultCode);

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	} 
	else 
	{
		hNetlibHttp = NULL;
		int err = GetLastError();
		if (err) 
			NLogF("CheckVersionURL: error code %d", err);
	}

	return ret;
}

char *UpdateRequired(UpdateInternal &update_internal, bool *beta) 
{
	// determine whether update is required

    char *ret = NULL, *ret_beta = NULL;

	if (options.use_xml_backend && update_internal.file_id != -1) 
	{
		const char * res;
		if (update_internal.cat == MC_UNKNOWN) 
		{
			if (XMLDataAvailable(MC_PLUGINS) && 
				(res = FindVersion(update_internal.file_id, update_internal.update.pbVersion, update_internal.update.cpbVersion, MC_PLUGINS)))
			{
				update_internal.cat = MC_PLUGINS;
				if (strcmp(res, "same")) ret = _strdup(res);
			} 
			else if (XMLDataAvailable(MC_LOCALIZATION) && 
				(res = FindVersion(update_internal.file_id, update_internal.update.pbVersion, update_internal.update.cpbVersion, MC_LOCALIZATION)))
			{
				update_internal.cat = MC_LOCALIZATION;
				if (strcmp(res, "same")) ret = _strdup(res);
			}
		} 
		else 
		{
			res = FindVersion(update_internal.file_id, update_internal.update.pbVersion, update_internal.update.cpbVersion, update_internal.cat);
			if (res && strcmp(res, "same")) ret = _strdup(res);
		}
	} 
	else 
	{
		ret = CheckVersionURL(update_internal.update.szVersionURL, update_internal.update.pbVersionPrefix,
			update_internal.update.cpbVersionPrefix, update_internal.update.pbVersion, update_internal.update.cpbVersion);
	}

	if (update_internal.update_options.use_beta) 
	{
		ret_beta = CheckVersionURL(update_internal.update.szBetaVersionURL, update_internal.update.pbBetaVersionPrefix,
			update_internal.update.cpbBetaVersionPrefix, update_internal.update.pbVersion, update_internal.update.cpbVersion);
	}

	if (ret && !ret_beta) 
	{
		if(beta) *beta = false;
		return ret;
	} 
	else if (!ret && ret_beta) 
	{
		if (beta) *beta = true;
		return ret_beta;
	} 
	else if (ret && ret_beta) 
	{
		// find highest version of ret and ret_beta

		DWORD vRet, vRetBeta;
		VersionFromString(ret, &vRet);
		VersionFromString(ret_beta, &vRetBeta);

		if (vRetBeta > vRet)
		{
			free(ret);
			if (beta) *beta = true;
			return ret_beta;
		} 
		else 
		{
			free(ret_beta);
			if (beta) *beta = false;
			return ret;
		}
	}

	return 0;
}

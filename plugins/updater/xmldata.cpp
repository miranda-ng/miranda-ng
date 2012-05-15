#include "common.h"
#include "xmldata.h"

const char *category_files[] = { "category_plugins", "category_localisation" };

BYTE *pData[NUM_CATEGORIES];
int dataLength[NUM_CATEGORIES];
ezxml_t doc[NUM_CATEGORIES];

bool XMLDataAvailable(const Category cat) {
	return (pData[cat] && dataLength[cat]);
}

void FreeXMLData(const Category cat) {
	free(pData[cat]);
    pData[cat] = NULL;

    ezxml_free(doc[cat]);
	doc[cat] = NULL;

    dataLength[cat] = 0;
}

bool OldXMLDataExists(const Category cat) {
	TCHAR xml_data_filename[MAX_PATH];
	TCHAR *ts;
	mir_sntprintf(xml_data_filename, SIZEOF(xml_data_filename), _T("%s\\%s.xml"),
		options.data_folder, (ts = GetTString(category_files[cat])));
	mir_free(ts);

	HANDLE hDataFile = CreateFile(xml_data_filename, 0, 0, 0, OPEN_EXISTING, 0, 0);
	if(hDataFile != INVALID_HANDLE_VALUE) {
		CloseHandle(hDataFile);
		return true;
	}

	return false;
}

// return age of file in hours
const ULARGE_INTEGER mult = { 600000000, 0}; // number of 100 microsecond blocks in a minute
long OldXMLDataAge(const Category cat) {
	TCHAR xml_data_filename[MAX_PATH];
	TCHAR *ts;
	mir_sntprintf(xml_data_filename, SIZEOF(xml_data_filename), _T("%s\\%s.xml"),
		options.data_folder, (ts = GetTString(category_files[cat])));
	mir_free(ts);

	HANDLE hDataFile = CreateFile(xml_data_filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(hDataFile != INVALID_HANDLE_VALUE) {
		FILETIME ft_then, ft_now;
		GetFileTime(hDataFile, 0, 0, &ft_then);
		CloseHandle(hDataFile);

		SYSTEMTIME now;
		GetSystemTime(&now);
		SystemTimeToFileTime(&now, &ft_now);

		ULARGE_INTEGER uli_now, uli_then, diff;
		uli_now.HighPart = ft_now.dwHighDateTime;
		uli_now.LowPart = ft_now.dwLowDateTime;
		uli_then.HighPart = ft_then.dwHighDateTime;
		uli_then.LowPart = ft_then.dwLowDateTime;

		diff.QuadPart = uli_now.QuadPart - uli_then.QuadPart;

		long minutes = (long)(diff.QuadPart / mult.QuadPart); // rounded down 

		// convert to hours (add 30 so we round up properly)
		return (minutes + 30) / 60;
	}

	return -1;
}

bool LoadOldXMLData(const Category cat, bool update_age) {
	TCHAR xml_data_filename[MAX_PATH];
	TCHAR *ts;
	mir_sntprintf(xml_data_filename, SIZEOF(xml_data_filename), _T("%s\\%s.xml"),
		options.data_folder, (ts = GetTString(category_files[cat])));
	mir_free(ts);

	if(pData[cat]) free(pData[cat]);
	pData[cat] = 0;
	dataLength[cat] = 0;

	// load 
	HANDLE hDataFile = CreateFile(xml_data_filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
	if(hDataFile != INVALID_HANDLE_VALUE) {
		dataLength[cat] = GetFileSize(hDataFile, 0);
		if(dataLength[cat]) {
			unsigned long bytes_read;
			pData[cat] = (BYTE *)malloc(dataLength[cat]);
			if(ReadFile(hDataFile, pData[cat], dataLength[cat], &bytes_read, 0))
				dataLength[cat] = bytes_read;
			else {
				free(pData[cat]);
				pData[cat] = 0;
				dataLength[cat] = 0;
			}

		}

		if(update_age) {
			FILETIME ft_now;
			SYSTEMTIME now;
			GetSystemTime(&now);
			SystemTimeToFileTime(&now, &ft_now);
			SetFileTime(hDataFile, 0, 0, &ft_now);
		}

		CloseHandle(hDataFile);
	}
	
	if(pData[cat] && dataLength[cat]) {
		doc[cat] = ezxml_parse_str((char*)pData[cat], dataLength[cat]);
		return doc[cat] != NULL;
	}
	return false;
}

bool SaveXMLData(const Category cat) {
	TCHAR xml_data_filename[MAX_PATH];
	TCHAR *ts;
	mir_sntprintf(xml_data_filename, SIZEOF(xml_data_filename), _T("%s\\%s.xml"),
		options.data_folder, (ts = GetTString(category_files[cat])));
	mir_free(ts);

	if(!CreatePath(options.data_folder)) {
		return false;
	}
	
	// save data
	if(pData[cat] && dataLength[cat]) {
		HANDLE hDataFile = CreateFile(xml_data_filename, GENERIC_READ | GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
		if(hDataFile != INVALID_HANDLE_VALUE) {
			unsigned long bytes_written;
			WriteFile(hDataFile, pData[cat], dataLength[cat], &bytes_written, 0);
			CloseHandle(hDataFile);
			return true;
		}
	}

	return false;
}

extern "C" void bz_internal_error ( int errcode ){}

bool bz2_decompress_xml(char *in_data, int in_data_length, BYTE **pDat, int *data_length) {
	const int BLOCKSIZE = 1024 * 100;

	bz_stream bzs = {0};

	switch(BZ2_bzDecompressInit(&bzs, 0, 0)) {
	case BZ_CONFIG_ERROR: 
		//MessageBox(0, "Configuration Error", "BZ2 Decompres Init", MB_OK | MB_ICONERROR); 
		ShowError(TranslateT("BZ2 Decompression, configuration error"));
		return false;
	case BZ_PARAM_ERROR: 
		//MessageBox(0, "Parameters Error", "BZ2 Decompres Init", MB_OK | MB_ICONERROR); 
		ShowError(TranslateT("BZ2 Decompression, parameter error"));
		return false;
	case BZ_MEM_ERROR: 
		//MessageBox(0, "Memory Error", "BZ2 Decompres Init", MB_OK | MB_ICONERROR); 
		ShowError(TranslateT("DB2 Decompression, memory error"));
		return false;
	}

	bzs.avail_in = in_data_length;
	bzs.next_in = in_data;

	bzs.avail_out = BLOCKSIZE;
	*pDat = (BYTE *)malloc(bzs.avail_out + 1); // allocate 100k (at present, xml data is about 87k)  (1 byte extra for a terminating 0 for safety)
	bzs.next_out = (char *)*pDat;

	int blocknum = 0;
	int ret;
	while((ret = BZ2_bzDecompress(&bzs)) == BZ_OK && bzs.avail_in > 0) {
		if(bzs.avail_out == 0) {
			blocknum++;
			*pDat = (BYTE *)realloc(*pDat, (blocknum + 1) * BLOCKSIZE + 1);
			bzs.next_out = (char *)(*pDat + (blocknum * BLOCKSIZE));
			bzs.avail_out = BLOCKSIZE;
		}
	}

	BZ2_bzDecompressEnd(&bzs);

	if(ret != BZ_STREAM_END) {
//		char msg[512];
//		sprintf(msg, "Error decompressing, code: %d", ret);
//		MessageBox(0, msg, "Error Decompressing BZ2 XML data", MB_OK);
		free(*pDat);
		*pDat = 0;
		*data_length = 0;
		return false;
	}

	*data_length = bzs.total_out_lo32;		// assume it's not too massive!
	(*pDat)[*data_length] = 0;				// for safety - last char shouldn't matter to us

	//char msg[256];
	//sprintf(msg, "Bytes decompressed: %d", data_length);
	//MessageBox(0, msg, "msg", MB_OK);

	return true;
}

bool UpdateXMLData(const Category cat, const char *redirect_url /*= 0*/, int recurse_count /*=0*/) {
	
	if(recurse_count > MAX_REDIRECT_RECURSE) {
		PUShowMessageT(TranslateT("Updater: Error getting data - too many redirects"), SM_WARNING);
		return false;
	}

	NETLIBHTTPREQUEST req = {0};
	NETLIBHTTPHEADER etag_hdr = {0};

	if(OldXMLDataExists(cat)) {
		// ensure backend not checked more than once every MIN_XMLDATA_AGE hours
		long age = OldXMLDataAge(cat);
		if(age >= 0 && age < MIN_XMLDATA_AGE)	{	// get it only if our file is at least 8 hours old
#ifdef DEBUG_HTTP_POPUPS
			char buff[512];
			sprintf(buff, "XML Data is recent (%d hours old) - not downloading, using local copy", age);
			PUShowMessage(buff, SM_NOTIFY);
#endif

			return LoadOldXMLData(cat, false);
		}

		// add ETag header for conditional get
		DBCONTACTGETSETTING cgs;
		DBVARIANT dbv;
		cgs.szModule = MODULE;
		char buff[256];
		strcpy(buff, "DataETag_");
		strcat(buff, category_files[cat]);
		cgs.szSetting = buff;
		cgs.pValue = &dbv;
		if(!CallService(MS_DB_CONTACT_GETSETTING, 0, (LPARAM)&cgs)) {
			req.headersCount = 1;
			req.headers = &etag_hdr;
			etag_hdr.szName = "If-None-Match";
			etag_hdr.szValue = _strdup(dbv.pszVal);
			DBFreeVariant(&dbv);
		}
	}

	req.cbSize = sizeof(req);
	req.requestType = REQUEST_GET;
	char URL[MAX_PATH];
	if(!redirect_url) {
		strcpy(URL, MIM_BACKEND_URL_PREFIX);
		strcat(URL, category_files[cat]);
		strcat(URL, ".bz2");
	} else {
		strcpy(URL, redirect_url);
	}
	req.szUrl = URL;
	req.flags = NLHRF_HTTP11;
	req.nlc = hNetlibHttp;

	if (CallService(MS_SYSTEM_GETVERSION, 0, 0) >= PLUGIN_MAKE_VERSION(0,9,0,5))
		req.flags |= NLHRF_PERSISTENT | NLHRF_REDIRECT;

	NETLIBHTTPREQUEST *resp = (NETLIBHTTPREQUEST *)CallService(MS_NETLIB_HTTPTRANSACTION, (WPARAM)hNetlibUser, (LPARAM)&req);
	free(etag_hdr.szValue);

	if (!resp) 
	{
		hNetlibHttp = NULL;
		if (!Miranda_Terminated())
		{
			int err = GetLastError();
			if (err) 
			{
				TCHAR buff[512];
				int len = mir_sntprintf(buff, SIZEOF(buff), TranslateT("Failed to download XML data: "));
				FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, err, 0, buff + len, 512 - len, 0);
				ShowError(buff);
				//MessageBox(0, buff + len, TranslateT("Updater: Error Downloading XML Data"), MB_OK | MB_ICONWARNING);
			} 
			else 
			{
				ShowError(TranslateT("Failed to download XML data - Response is NULL"));
				//MessageBox(0, TranslateT("Error downloading XML data...\nResponse is NULL"), TranslateT("Updater Error"), MB_OK | MB_ICONWARNING);
				NLog("Failed to download XML data - Response is NULL");
			}
		}
		return LoadOldXMLData(cat, false);
	} 
	else if (resp->resultCode == 304) { // 'Not Modified' response
		hNetlibHttp = resp->nlc;
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		resp = 0;
#ifdef DEBUG_HTTP_POPUPS
		PUShowMessage("XML Data unchanged - using local copy", SM_NOTIFY);
#endif

		// mark data as current
		return LoadOldXMLData(cat, true);
	} else if(resp->resultCode >= 300 && resp->resultCode < 400) { // redirect response
		hNetlibHttp = resp->nlc;
		// get new location
		bool ret = false;
		for(int i = 0; i < resp->headersCount; i++) {
			//MessageBox(0, resp->headers[i].szValue, resp->headers[i].szName, MB_OK);
			if(strcmp(resp->headers[i].szName, "Location") == 0) {
				ret = UpdateXMLData(cat, resp->headers[i].szValue, recurse_count + 1);
				break;
			}
		}
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		resp = 0;
		if(!ret) return LoadOldXMLData(cat, false);

		return ret;

	} else if(resp->resultCode != 200) {
		hNetlibHttp = resp->nlc;
		TCHAR buff[512];
		mir_sntprintf(buff, SIZEOF(buff), TranslateT("Failed to download XML data - Invalid response, code %d"), resp->resultCode);
		ShowError(buff);
		NLog(buff);
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		resp = 0;
		return LoadOldXMLData(cat, false);
	}
		
	// resp->resultCode == 200
	hNetlibHttp = resp->nlc;

	if(!bz2_decompress_xml(resp->pData, resp->dataLength, &pData[cat], &dataLength[cat])) {
		ShowError(TranslateT("Failed to decompress XML data"));
		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
		resp = 0;
		return LoadOldXMLData(cat, false);
	}

	// store date header and data for response 'Not Modified' (304) above
	for(int i = 0; i < resp->headersCount; i++) {
		//MessageBox(0, resp->headers[i].szValue, resp->headers[i].szName, MB_OK);
		if(strcmp(resp->headers[i].szName, "ETag") == 0) {
			//MessageBox(0, resp->headers[i].szValue, "Storing ETag", MB_OK);
			char buff[256];
			strcpy(buff, "DataETag_");
			strcat(buff, category_files[cat]);
			DBWriteContactSettingString(0, MODULE, buff, resp->headers[i].szValue);
		}
	}

	CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT, 0, (LPARAM)resp);
	resp = 0;

	SaveXMLData(cat);

	doc[cat] = ezxml_parse_str((char*)pData[cat], strlen((char*)pData[cat]));
	if (!doc[cat]) return false;

	return true;
}

bool GetXMLData(BYTE **pDat, int &dataLen, const Category cat) {
	if(pData[cat] && dataLength[cat]) {
		*pDat = pData[cat];
		dataLen = dataLength[cat];
		return true;
	} else
		return false;
}

bool VersionLess(const char *current, const char *potential) {
	DWORD dwCur, dwPot;
	if(VersionFromString(current, &dwCur) && VersionFromString(potential, &dwPot)) {
		switch(options.ver_req) {
		case VR_MAJOR:
			dwCur &= 0xFF000000;
			dwPot &= 0xFF000000;
			break;
		case VR_MINOR:
			dwCur &= 0xFFFF0000;
			dwPot &= 0xFFFF0000;
			break;
		case VR_RELEASE:
			dwCur &= 0xFFFFFF00;
			dwPot &= 0xFFFFFF00;
			break;
		case VR_BUILD:
			break;
		}
		return dwCur < dwPot;
	}
	return false;
}

const char *FindVersion(int file_id, BYTE *pbVersionBytes, int cpbVersionBytes, const Category cat) {

	if(!doc[cat]) return 0;

	char version_string[128];
	strncpy(version_string, (char *)pbVersionBytes, cpbVersionBytes);
	version_string[sizeof(version_string)-1] = 0;

	ezxml_t root = ezxml_get(doc[cat], "channel", 0, "item", -1);
    while (root) {
        int id = atoi(ezxml_txt(ezxml_child(root, "id")));
        const char* version = ezxml_txt(ezxml_child(root, "version"));
        
		if (id == file_id && version[0]) {
			if (strncmp(version, (char*)pbVersionBytes, cpbVersionBytes) && VersionLess(version_string, version)) {
				return version;
			} else {
				return "same";
            }
		} 

        root = ezxml_next(root);
    }

	return 0;
}

int FindFileID(const char *name, const Category cat, UpdateList *update_list) 
{
	if (!doc[cat]) return -1;

	if (update_list) 
    {
		// couldn't find it in xml file - check if a plugin gave us a file id for a different shortName
		for (int i = 0; i < update_list->getCount(); ++i) 
        {
			UpdateInternal &ui = (*update_list)[i];
			if (ui.file_id != -1 && strcmp(ui.update.szComponentName, name) == 0) 
				return ui.file_id;
		}
	}

	// ignore case in name
	int id = -1;
	char *version = NULL;

	ezxml_t root = ezxml_get(doc[cat], "channel", 0, "item", -1);
    while (root) 
	{
        const char* title = ezxml_txt(ezxml_child(root, "title"));
        if (_stricmp(title, name) == 0) 
		{
            const char* subcategory = ezxml_txt(ezxml_child(root, "subcategory"));
            if (strcmp(subcategory, "Archived")) 
			{
                int id1 = atoi(ezxml_txt(ezxml_child(root, "id")));
                if (id1)
				{
					char *version1 = ezxml_txt(ezxml_child(root, "version"));
					if (!version || (version1 && VersionLess(version, version1)))
					{
						version = version1;
						id = id1;
					}
				}
            }
        }
        root = ezxml_next(root);
    }

	return id;
}

void UpdateFLIDs(UpdateList &update_list) 
{
    for (int i = 0; i < update_list.getCount(); ++i) 
    {
		if(update_list[i].file_id == -1 && update_list[i].update.szUpdateURL && strcmp(update_list[i].update.szUpdateURL, UPDATER_AUTOREGISTER) == 0) 
		{
			int file_id = FindFileID(update_list[i].update.szComponentName, MC_PLUGINS, 0);
			if (file_id == -1)
				file_id = FindFileID(update_list[i].update.szComponentName, MC_LOCALIZATION, 0);
			if (file_id != -1) 
			{
				update_list[i].file_id = file_id;
				char *buff = (char *)safe_alloc((int)strlen(MIM_DOWNLOAD_URL_PREFIX) + 9);
				sprintf(buff, MIM_DOWNLOAD_URL_PREFIX "%d", file_id);
				update_list[i].update.szUpdateURL = buff;
				update_list[i].shortName = safe_strdup(update_list[i].update.szComponentName);

				if(update_list[i].update.szBetaVersionURL) 
				{
					update_list[i].update_options.fixed = false;
					LoadUpdateOptions(update_list[i].update.szComponentName, &update_list[i].update_options);
				}
			}
		}
	}
}

/*
Copyright © 2009 Jim Porter

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common.h"
#include "utility.h"

#include <io.h>

std::string b64encode(const std::string &s)
{
	NETLIBBASE64 encode;
	encode.cbDecoded = s.length();
	encode.pbDecoded = (BYTE*)s.c_str();
	encode.cchEncoded = Netlib_GetBase64EncodedBufferSize(encode.cbDecoded);
	encode.pszEncoded = new char[encode.cchEncoded+1];
	CallService(MS_NETLIB_BASE64ENCODE,0,(LPARAM)&encode);
	std::string ret = encode.pszEncoded;
	delete[] encode.pszEncoded;

	return ret;
}

http::response mir_twitter::slurp(const std::string &url, http::method meth, const std::string &post_data)
{
	NETLIBHTTPREQUEST req = {sizeof(req)};
	NETLIBHTTPREQUEST *resp;
	req.requestType = (meth == http::get) ? REQUEST_GET:REQUEST_POST;
	req.szUrl = ( char* )url.c_str();

	// probably not super-efficient to do this every time, but I don't really care
	std::string auth = "Basic " + b64encode(username_ + ":" + password_);

	NETLIBHTTPHEADER hdr[2];
	hdr[0].szName = "Authorization";
	hdr[0].szValue = (char*)( auth.c_str());

	req.headers = hdr;
	req.headersCount = 1;

	if (meth == http::post)
	{
		hdr[1].szName = "Content-Type";
		hdr[1].szValue = "application/x-www-form-urlencoded";

		req.headersCount = 2;
		req.dataLength = post_data.size();
		req.pData = ( char* )post_data.c_str();
	}

	http::response resp_data;

	resp = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService( MS_NETLIB_HTTPTRANSACTION,
		reinterpret_cast<WPARAM>(handle_), reinterpret_cast<LPARAM>(&req)));

	if (resp)
	{
		resp_data.code = resp->resultCode;
		resp_data.data = resp->pData ? resp->pData:"";

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)resp);
	}

	return resp_data;
}



bool save_url(HANDLE hNetlib,const std::string &url,const std::string &filename)
{
	NETLIBHTTPREQUEST req = {sizeof(req)};
	NETLIBHTTPREQUEST *resp;
	req.requestType = REQUEST_GET;
	req.szUrl = const_cast<char*>(url.c_str());

	resp = reinterpret_cast<NETLIBHTTPREQUEST*>(CallService( MS_NETLIB_HTTPTRANSACTION,
		reinterpret_cast<WPARAM>(hNetlib), reinterpret_cast<LPARAM>(&req)));

	if (resp)
	{
		// Create folder if necessary
		std::string dir = filename.substr(0,filename.rfind('\\'));
		if (_access(dir.c_str(),0))
			CallService(MS_UTILS_CREATEDIRTREE, 0, (LPARAM)dir.c_str());

		// Write to file
		FILE *f = fopen(filename.c_str(),"wb");
		fwrite(resp->pData,1,resp->dataLength,f);
		fclose(f);

		CallService(MS_NETLIB_FREEHTTPREQUESTSTRUCT,0,(LPARAM)resp);
		return true;
	}
	else
		return false;
}

static const struct
{
	char *ext;
	int fmt;
} formats[] = {
	{ ".png",  PA_FORMAT_PNG  },
	{ ".jpg",  PA_FORMAT_JPEG },
	{ ".jpeg", PA_FORMAT_JPEG },
	{ ".ico",  PA_FORMAT_ICON },
	{ ".bmp",  PA_FORMAT_BMP  },
	{ ".gif",  PA_FORMAT_GIF  },
};

int ext_to_format(const std::string &ext)
{
	for(size_t i=0; i<SIZEOF(formats); i++)
	{
		if (ext == formats[i].ext)
			return formats[i].fmt;
	}

	return PA_FORMAT_UNKNOWN;
}
#include "../common.h"
#include "MediaUploader.h"

// TODO get rid of unneeded headers added by NETLIBHTTPREQUEST. it sould look like this:
//POST https://mmiXYZ.whatsapp.net/u/gOzeKj6U64LABC
//Content-Type: multipart/form-data; boundary=zzXXzzYYzzXXzzQQ
//Host: mmiXYZ.whatsapp.net
//User-Agent: WhatsApp/2.12.96 S40Version/14.26 Device/Nokia302
//Content-Length: 9999999999
//
//So remove these somehow:
//Accept-Encoding: deflate, gzip
//Connection: Keep-Alive
//Proxy-Connection: Keep-Alive

static NETLIBHTTPHEADER s_imageHeaders[] =
{
	{ "User-Agent", ACCOUNT_USER_AGENT },
	{ "Content-Type", "multipart/form-data; boundary=zzXXzzYYzzXXzzQQ" }
};

static std::vector<unsigned char>* sttFileToMem(const TCHAR *ptszFileName)
{
	HANDLE hFile = CreateFile(ptszFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
		return NULL;

	DWORD upperSize, lowerSize = GetFileSize(hFile, &upperSize);
	std::vector<unsigned char> *result = new std::vector<unsigned char>(lowerSize);
	ReadFile(hFile, (void*)result->data(), lowerSize, &upperSize, NULL);
	CloseHandle(hFile);
	return result;
}

namespace MediaUploader
{
	std::string sendData(std::string host, std::string head, std::string filePath, std::string tail)
	{
		// TODO string crap: can this be done more nicely?
		std::wstring stemp = std::wstring(filePath.begin(), filePath.end());
		LPCWSTR sw = stemp.c_str();

		vector<unsigned char> *dataVector = sttFileToMem(sw);

		vector<unsigned char> allVector(head.begin(), head.end());
		allVector.insert(allVector.end(), dataVector->begin(), dataVector->end());
		allVector.insert(allVector.end(), tail.begin(), tail.end());

		NETLIBHTTPREQUEST nlhr = { sizeof(NETLIBHTTPREQUEST) };
		nlhr.requestType = REQUEST_POST;
		nlhr.szUrl = (char*)host.c_str();
		nlhr.headers = s_imageHeaders;
		nlhr.headersCount = _countof(s_imageHeaders);
		nlhr.flags = NLHRF_HTTP11 | NLHRF_GENERATEHOST | NLHRF_REMOVEHOST | NLHRF_SSL;
		nlhr.pData = (char*)allVector.data();
		nlhr.dataLength = (int)allVector.size();

		NETLIBHTTPREQUEST* pnlhr = (NETLIBHTTPREQUEST*)CallService(MS_NETLIB_HTTPTRANSACTION,
			(WPARAM)WASocketConnection::hNetlibUser, (LPARAM)&nlhr);

		string data = pnlhr->pData;

		if (!data.empty())
			return data;
		else return 0;
	}

	std::string pushfile(std::string url, FMessage * message, std::string from)
	{
		return getPostString(url, message, from);
	}

	std::string getPostString(std::string url, FMessage * message, std::string from)
	{
		string filePath = message->media_url;
		string to = message->key.remote_jid;
		long fileSize = message->media_size;
		string extension = split(filePath, '.')[1];

		const BYTE *path = (const BYTE*)filePath.c_str();

		uint8_t digest[16];
		md5_string(filePath, digest);
		char dest[33];
		bin2hex(digest, sizeof(digest), dest);

		string cryptoname = dest;
		cryptoname += "." + extension;
		string boundary = "zzXXzzYYzzXXzzQQ";

		string hBAOS = "--" + boundary + "\r\n";
		hBAOS += "Content-Disposition: form-data; name=\"to\"\r\n\r\n";
		hBAOS += to + "\r\n";
		hBAOS += "--" + boundary + "\r\n";
		hBAOS += "Content-Disposition: form-data; name=\"from\"\r\n\r\n";
		hBAOS += from + "\r\n";
		hBAOS += "--" + boundary + "\r\n";
		hBAOS += "Content-Disposition: form-data; name=\"file\"; filename=\"" + cryptoname + "\"\r\n";
		hBAOS += "Content-Type: " + getMimeFromExtension(extension) + "\r\n\r\n";

		string fBAOS = "\r\n--" + boundary + "--\r\n";
		long contentlength = sizeof(hBAOS) + sizeof(fBAOS) + fileSize;

		return sendData(url, hBAOS, filePath, fBAOS);
	}

	static map<string, string> extensions;

	std::string getMimeFromExtension(const string &extension)
	{
		if (extensions.empty()) {
			extensions["audio/3gpp"] = "3gp";
			extensions["audio/x-caf"] = "caf";
			extensions["audio/wav"] = "wav";
			extensions["audio/mpeg"] = "mp3";
			extensions["audio/mpeg3"] = "mp3";
			extensions["audio/x-mpeg-32"] = "mp3";
			extensions["audio/x-ms-wma"] = "wma";
			extensions["audio/ogg"] = "ogg";
			extensions["audio/aiff"] = "aif";
			extensions["audio/x-aiff"] = "aif";
			extensions["audio/mp4"] = "m4a";
			extensions["image/jpeg"] = "jpg";
			extensions["image/gif"] = "gif";
			extensions["image/png"] = "png";
			extensions["video/3gpp"] = "3gp";
			extensions["video/mp4"] = "mp4";
			extensions["video/quicktime"] = "mov";
			extensions["video/avi"] = "avi";
			extensions["video/msvideo"] = "avi";
			extensions["video/x-msvideo"] = "avi";
		}

		for (auto it = extensions.begin(); it != extensions.end(); ++it)
			if ((*it).second == extension)
				return (*it).first;

		return "";
	}
}

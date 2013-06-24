#include "common.h"

void UnixTimeToFileTime(time_t t, LPFILETIME pft)
{
	// Note that LONGLONG is a 64-bit value
	LONGLONG ll;

	ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
}

DWORD utils::conversion::to_timestamp(std::string data)
{
	DWORD timestamp = NULL;
	/*
	if (!utils::conversion::from_string<DWORD>(timestamp, data, std::dec)) {
		timestamp = static_cast<DWORD>(::time(NULL));
	}
	*/
	return timestamp;
}

std::string utils::text::source_get_value(std::string* data, unsigned int argument_count, ...)
{
	va_list arg;
	std::string ret;
	std::string::size_type start = 0, end = 0;
	
	va_start(arg, argument_count);
	
	for (unsigned int i = argument_count; i > 0; i--)
	{
		if (i == 1)
		{
			end = data->find(va_arg(arg, char*), start);
			if (start == std::string::npos || end == std::string::npos)
				break;
			ret = data->substr(start, end - start);
		} else {
			std::string term = va_arg(arg, char*);
			start = data->find(term, start);
			if (start == std::string::npos)
				break;
			start += term.length();
		}
	}
	
	va_end(arg);	
	return ret;
}

std::string getLastErrorMsg()
{ 
	 // Retrieve the system error message for the last-error code

	 LPVOID lpMsgBuf;
	 DWORD dw = WSAGetLastError(); 

	 FormatMessageA(
		  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		  FORMAT_MESSAGE_FROM_SYSTEM |
		  FORMAT_MESSAGE_IGNORE_INSERTS,
		  NULL,
		  dw,
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		  (LPSTR) &lpMsgBuf,
		  0, NULL );

	 // Display the error message and exit the process
	 /*
	 lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, 
		  (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR)); 
	 StringCchPrintf((LPTSTR)lpDisplayBuf, 
		  LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		  TEXT("%s"), 
		  lpMsgBuf); 
		  */

	 std::string ret((LPSTR) lpMsgBuf);
	 LocalFree(lpMsgBuf);
	 //LocalFree(lpDisplayBuf);

	 //return std::string((LPCTSTR)lpDisplayBuf);
	 return ret;
}

int utils::debug::log(std::string file_name, std::string text)
{
	char szFile[MAX_PATH];
	GetModuleFileNameA(g_hInstance, szFile, SIZEOF(szFile));
	std::string path = szFile;
	path = path.substr(0, path.rfind("\\"));
	path = path.substr(0, path.rfind("\\") + 1);
	path = path + file_name.c_str() + ".txt";

	SYSTEMTIME time;
	GetLocalTime(&time);

	std::ofstream out(path.c_str(), std::ios_base::out | std::ios_base::app | std::ios_base::ate);
	out << "[" << (time.wHour < 10 ? "0" : "") << time.wHour << ":" << (time.wMinute < 10 ? "0" : "") << time.wMinute << ":" << (time.wSecond < 10 ? "0" : "") << time.wSecond << "] " << text << std::endl;
	out.close();

	return EXIT_SUCCESS;
}

BYTE* utils::md5string(const BYTE *data, int size, BYTE *digest)
{
	mir_md5_state_t md5_state;
	mir_md5_init(&md5_state);
	mir_md5_append(&md5_state, data, size);
	mir_md5_finish(&md5_state, digest);
	return digest;
}

/////////////////////////////////////////////////////////////////////////////////////////
// external stubs for WhatsAPI++

std::string base64_encode(void* pData, size_t len)
{
	return (char*)ptrA( mir_base64_encode((BYTE*)pData, (unsigned)len));
}

void md5_string(const std::string& data, BYTE digest[16])
{
	utils::md5string(data, digest);
}

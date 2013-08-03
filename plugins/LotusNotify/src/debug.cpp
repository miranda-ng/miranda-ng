#include "stdafx.h"
#include "debug.h"

HANDLE netlibHandle;


void logRegister(){

	// Register netlib user for logging function
	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_TCHAR | NUF_NOOPTIONS;
	nlu.szSettingsModule = PLUGINNAME;
	nlu.ptszDescriptiveName = mir_a2u(PLUGINNAME);
	netlibHandle = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);

}

void logUnregister(){

	Netlib_CloseHandle(netlibHandle);
	netlibHandle = NULL;

}

void log(const wchar_t* szText){

	if (netlibHandle) {
		CallService(MS_NETLIB_LOGW, (WPARAM)netlibHandle, (LPARAM)szText);
	}

	#ifdef _DEBUG
	OutputDebugString(szText);
	#endif //_DEBUG

}

void log_p(const wchar_t* szText, ...){

	va_list args;
	va_start(args, szText);
	int len = _vscwprintf(szText, args ) + 1; // _vscprintf doesn't count terminating '\0' //!!!!!!!!!!!!!!!!
	wchar_t* buffer = new wchar_t[len * sizeof(wchar_t)];
	mir_vsntprintf(buffer, len, szText, args);
	va_end(args);
	log(buffer);
	delete buffer;

}









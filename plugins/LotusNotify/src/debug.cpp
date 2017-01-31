#include "stdafx.h"
#include "debug.h"

HNETLIBUSER netlibHandle;

void logRegister(){
	// Register netlib user for logging function
	NETLIBUSER nlu = {};
	nlu.flags = NUF_UNICODE | NUF_NOOPTIONS;
	nlu.szSettingsModule = PLUGINNAME;
	nlu.szDescriptiveName.w = mir_a2u(PLUGINNAME);
	netlibHandle = Netlib_RegisterUser(&nlu);
}

void logUnregister(){
	Netlib_CloseHandle(netlibHandle);
	netlibHandle = NULL;
}

void log(const wchar_t* szText){

	if (netlibHandle) {
		Netlib_LogW(netlibHandle, szText);
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
	mir_vsnwprintf(buffer, len, szText, args);
	va_end(args);
	log(buffer);
	delete[] buffer;
}


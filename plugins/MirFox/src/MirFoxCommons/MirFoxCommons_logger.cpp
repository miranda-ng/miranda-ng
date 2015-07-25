#include "MirFoxCommons_pch.h"
#include "MirFoxCommons_logger.h"


/*static*/ MFLogger * MFLogger::m_pOnlyOneInstance;


MFLogger::MFLogger()
{
	logFunction = NULL;
	memcpy(m_prefix, L"      ", PREFIX_SIZE * sizeof(m_prefix[0]));
}


void
MFLogger::initLogger(LogFunction logFunction_p){

	logFunction = logFunction_p;
	InitializeCriticalSection(&logCs);

}

void
MFLogger::set6CharsPrefix(const wchar_t* prefix){
	size_t len = wcslen(prefix);
	memcpy(m_prefix, L"      ", PREFIX_SIZE * sizeof(m_prefix[0]));
	memcpy(m_prefix, prefix, len * sizeof(wchar_t));
}


void
MFLogger::releaseLogger(){

	logFunction = NULL;
	DeleteCriticalSection(&logCs);

}


void
MFLogger::log(const wchar_t* szText){

	size_t len = wcslen(szText) + 1;
	wchar_t* buffer = new wchar_t[(PREFIX_SIZE + len) * sizeof(wchar_t)];
	memcpy(buffer, m_prefix, PREFIX_SIZE * sizeof(m_prefix[0]));
	memcpy(buffer + PREFIX_SIZE , szText, len * sizeof(wchar_t));
	log_int(buffer);
	delete buffer;

}


void
MFLogger::log_p(const wchar_t* szText, ...){

	va_list args;
	va_start(args, szText);
	int len = (PREFIX_SIZE + _vscwprintf(szText, args ) + 1); // _vscprintf doesn't count terminating '\0'
	wchar_t* buffer = new wchar_t[len * sizeof(wchar_t)];
	vswprintf_s(buffer + PREFIX_SIZE, len, szText, args);
	va_end(args);
	memcpy(buffer, m_prefix, PREFIX_SIZE * sizeof(m_prefix[0]));
	log_int(buffer);
	delete buffer;

}


void
MFLogger::log_d(const wchar_t* szText){

	#ifdef _DEBUG
	size_t len = wcslen(szText) + 1;
	wchar_t* buffer = new wchar_t[(PREFIX_SIZE + len) * sizeof(wchar_t)];
	memcpy(buffer, m_prefix, PREFIX_SIZE * sizeof(m_prefix[0]));
	memcpy(buffer + PREFIX_SIZE , szText, len * sizeof(wchar_t));
	log_int(buffer);
	delete buffer;
	#endif //_DEBUG

}


void
MFLogger::log_dp(const wchar_t* szText, ...){

	#ifdef _DEBUG
	va_list args;
	va_start(args, szText);
	int len = (PREFIX_SIZE + _vscwprintf(szText, args ) + 1); // _vscprintf doesn't count terminating '\0'
	wchar_t* buffer = new wchar_t[len * sizeof(wchar_t)];
	vswprintf_s(buffer + PREFIX_SIZE, len, szText, args);
	va_end(args);
	memcpy(buffer, m_prefix, PREFIX_SIZE * sizeof(m_prefix[0]));
	log_int(buffer);
	delete buffer;
	#endif //_DEBUG

}


void
MFLogger::log_int(const wchar_t* szText){

	if (!logFunction){
		return;
	}

	EnterCriticalSection(&logCs);
	(*logFunction)(szText);
	LeaveCriticalSection(&logCs);

}



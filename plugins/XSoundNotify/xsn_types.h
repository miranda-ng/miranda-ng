#ifndef __XSN_TYPES
#define __XSN_TYPES

#include <string>
#include <windows.h>
#include <newpluginapi.h>
#include <m_database.h>

#ifndef UNICODE  
	typedef std::string xsn_string;
#else
	typedef std::wstring xsn_string;
#endif

typedef std::string ModuleString;
typedef std::string ProtocolString;

struct XSN_Variant : DBVARIANT
{
	XSN_Variant();
	~XSN_Variant();

	xsn_string toString() const;
	bool empty() const;
};

struct XSN_ModuleInfo
{
	static const char * name();
	static const char * soundSetting();
};

#endif

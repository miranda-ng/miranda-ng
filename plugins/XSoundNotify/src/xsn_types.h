#ifndef __XSN_TYPES
#define __XSN_TYPES

typedef std::string ModuleString;
typedef std::string ProtocolString;

struct XSN_Variant : DBVARIANT
{
	XSN_Variant();
	~XSN_Variant();

	std::tstring toString() const;
	bool empty() const;
};

struct XSN_ModuleInfo
{
	static const char *name();
	static const char *soundSetting();
};

#endif

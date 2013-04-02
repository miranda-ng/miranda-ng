#include "Common.h"

XSN_Variant::XSN_Variant()
{
	ptszVal = nullptr;	
}

XSN_Variant::~XSN_Variant()
{
	if (ptszVal)
		DBFreeVariant(this);
}

xsn_string XSN_Variant::toString() const
{	
	return ptszVal == nullptr ? xsn_string() : xsn_string(ptszVal);
}

bool XSN_Variant::empty() const
{
	return ptszVal == nullptr;
}

const char * XSN_ModuleInfo::name()
{
	return "XSoundNotify";
}

const char * XSN_ModuleInfo::soundSetting()
{
	return "XSNPlugin_sound";
}

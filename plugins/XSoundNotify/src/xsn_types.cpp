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

std::tstring XSN_Variant::toString() const
{	
	return ptszVal == nullptr ? std::tstring() : std::tstring(ptszVal);
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

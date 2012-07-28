#ifndef __XSN_DATA_H
#define __XSN_DATA_H

#include <unordered_map>
#include "xsn_types.h"
#include <windows.h>

class SoundNotifyData
{
public:
	SoundNotifyData();
	SoundNotifyData(HANDLE contact, const ModuleString & module, const xsn_string & sound);

	void setSound(const xsn_string & sound);

	HANDLE contact() const;	
	const xsn_string & soundPath() const;
	const ModuleString & module() const;	
	bool isSoundChanged() const;

private:
	HANDLE			_contact;	
	ModuleString	_module;
	xsn_string		_soundPath;	
	bool			_soundChanged;
};
typedef std::shared_ptr<SoundNotifyData> SoundNotifyDataPtr;

typedef std::unordered_map<xsn_string, SoundNotifyDataPtr> UserDataTable;
typedef std::unordered_map<ProtocolString, UserDataTable> ProtocolTable;
typedef std::unordered_map<ModuleString, ProtocolString> ModuleConvertTable;

#endif

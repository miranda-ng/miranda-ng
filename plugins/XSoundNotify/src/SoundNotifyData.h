#ifndef __XSN_DATA_H
#define __XSN_DATA_H

class SoundNotifyData
{
public:
	SoundNotifyData();
	SoundNotifyData(HANDLE contact, const ModuleString &module, const std::tstring &sound);

	void setSound(const std::tstring &sound);

	HANDLE contact() const;	
	const std::tstring &soundPath() const;
	const ModuleString &module() const;	
	bool isSoundChanged() const;

private:
	HANDLE			_contact;	
	ModuleString	_module;
	std::tstring	_soundPath;	
	bool			_soundChanged;
};
typedef std::shared_ptr<SoundNotifyData> SoundNotifyDataPtr;

typedef std::unordered_map<std::tstring, SoundNotifyDataPtr> UserDataTable;
typedef std::unordered_map<ProtocolString, UserDataTable> ProtocolTable;
typedef std::unordered_map<ModuleString, ProtocolString> ModuleConvertTable;

#endif

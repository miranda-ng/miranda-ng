#include "Common.h"

SoundNotifyDataStorage::SoundNotifyDataStorage()
{

}

void SoundNotifyDataStorage::init()
{
	initModuleConvertTable(_moduleTable);
	registerProtocols();

	HANDLE contact = (HANDLE)CallService(MS_DB_CONTACT_FINDFIRST,0,0);
	for(; contact; contact = (HANDLE)CallService(MS_DB_CONTACT_FINDNEXT,(WPARAM)contact,0))
	{
		try
		{
			addContact(contact);
		}
		catch (...)
		{
			// log
		}		
	}
}

void SoundNotifyDataStorage::commit()
{
	for (auto it = _protocolTable.begin(), end = _protocolTable.end(); it != end; ++it)
		for (auto user = it->second.begin(), userEnd = it->second.end(); user != userEnd; ++user)
			if (user->second->isSoundChanged())
			{
				if (user->second->soundPath().empty())
					DBDeleteContactSetting(user->second->contact(), XSN_ModuleInfo::name(), XSN_ModuleInfo::soundSetting());
				else				
					DBWriteContactSettingTString(user->second->contact(), XSN_ModuleInfo::name(), XSN_ModuleInfo::soundSetting(), user->second->soundPath().c_str());
			}	
}

void SoundNotifyDataStorage::addContact(HANDLE contact)
{			
	ModuleString module = getContactModule(contact);
	ProtocolString proto = _moduleTable[module];
	xsn_string user = getContactId(contact, module, proto);
	if (user.empty())
		return ;
	
	XSN_Variant sound;
	DBGetContactSettingTString(contact, XSN_ModuleInfo::name(), XSN_ModuleInfo::soundSetting(), &sound);
	_protocolTable[proto][user] = SoundNotifyDataPtr(new SoundNotifyData(contact, module, sound.toString()));		
}

xsn_string SoundNotifyDataStorage::getContactId(HANDLE contact, const ModuleString & module, const ProtocolString & proto)
{
	auto it = _registeredProtocols.find(proto);
	if (it == _registeredProtocols.end())
		return xsn_string();
	return it->second(contact, module);
}

ProtocolTable & SoundNotifyDataStorage::getData()
{
	return _protocolTable;
}

void SoundNotifyDataStorage::registerProtocols()
{
	_registeredProtocols["ICQ"] = &getIcqContactId;
	_registeredProtocols["JABBER"] = &getJabberContactId;
}

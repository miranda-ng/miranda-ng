#include "xsn_utils.h"
#include <m_protocols.h>
#include <boost/lexical_cast.hpp>

void initModuleConvertTable(ModuleConvertTable & table)
{
	int number = 0;
	// may be need to free PROTOACCOUNT, but did't found information about it
	PROTOACCOUNT ** accounts; 
	ProtoEnumAccounts(&number, &accounts);
	for (int i = 0; i < number; ++i)
		table[ModuleString(accounts[i]->szModuleName)] = ProtocolString(accounts[i]->szProtoName);
}

bool isReceiveMessage(LPARAM event)
{
	DBEVENTINFO info = { sizeof(info) };	
	CallService(MS_DB_EVENT_GET, event, (LPARAM)&info);
	// i don't understand why it works and how it works, but it works correctly - practice way (методом тыка)
	// so, i think correct condition would be : eventType == EVENTTYPE_MESSAGE && info.flags & DBEF_READ, but it really isn't
	return !(((info.eventType != EVENTTYPE_MESSAGE) && !(info.flags & DBEF_READ)) || (info.flags & DBEF_SENT));
}

xsn_string getContactSound(HANDLE contact)
{
	XSN_Variant sound;
	DBGetContactSettingTString(contact, XSN_ModuleInfo::name(), XSN_ModuleInfo::soundSetting(), &sound);
	return sound.toString();	
}

ModuleString getContactModule(HANDLE contact)
{
	char * proto = (char *)CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM)contact, 0);
	if (proto == nullptr)
		throw std::runtime_error("MS_PROTO_GETCONTACTBASEPROTO failed");
	return ModuleString(proto);
}

xsn_string getIcqContactId(HANDLE contact, const ModuleString & module)
{
	XSN_Variant nick;
	DBGetContactSettingTString(contact, module.c_str(), "CustomNick", &nick);
	if (nick.empty())
		DBGetContactSettingTString(contact, module.c_str(), "Nick", &nick);
	DWORD uin = DBGetContactSettingDword(contact, module.c_str(), "UIN", 0);
	xsn_string uinStr = boost::lexical_cast<xsn_string>(uin);
	if (nick.empty())
		return uinStr;
	return nick.toString() + TEXT(" (") + uinStr + TEXT(")");
}

xsn_string getJabberContactId(HANDLE contact, const ModuleString & module)
{
	XSN_Variant jid, nick;
	DBGetContactSettingTString(contact, module.c_str(), "jid", &jid);
	DBGetContactSettingTString(contact, "CList", "MyHandle", &nick);
	if (nick.empty())
		return jid.toString();
	return nick.toString() + TEXT(" (") + jid.toString() + TEXT(")");
}

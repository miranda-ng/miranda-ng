#ifndef _XSN_DATA_STORAGE
#define _XSN_DATA_STORAGE

class SoundNotifyDataStorage
{
public:
	SoundNotifyDataStorage();

	void init();
	void commit();

	ProtocolTable &getData();

protected:
	void addContact(HANDLE contact);
	std::tstring getContactId(HANDLE contact, const ModuleString &module, const ProtocolString &proto);
	void registerProtocols();

private:
	ProtocolTable		_protocolTable;
	ModuleConvertTable	_moduleTable;

	typedef std::tstring (*getContactIdFunc)(HANDLE contact, const ModuleString &module);
	typedef std::unordered_map<ProtocolString, getContactIdFunc> RegisteredProtocols;
	RegisteredProtocols _registeredProtocols;
};

#endif

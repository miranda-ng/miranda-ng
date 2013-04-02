#ifndef _XSN_DATA_STORAGE
#define _XSN_DATA_STORAGE

class SoundNotifyDataStorage
{
public:
	SoundNotifyDataStorage();

	void init();
	void commit();

	ProtocolTable & getData();

protected:
	void addContact(HANDLE contact);
	xsn_string getContactId(HANDLE contact, const ModuleString & module, const ProtocolString & proto);
	void registerProtocols();

private:
	ProtocolTable		_protocolTable;
	ModuleConvertTable	_moduleTable;

	typedef boost::function<xsn_string (HANDLE contact, const ModuleString & module)> getContactIdFunc;
	typedef std::unordered_map<ProtocolString, getContactIdFunc> RegisteredProtocols;
	RegisteredProtocols	_registeredProtocols;
};

#endif

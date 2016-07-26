#ifndef _MIRFOXDATA_INC
#define _MIRFOXDATA_INC



enum MFENUM_PLUGIN_STATE {
	MFENUM_PLUGIN_STATE_NEW,			//just started - unstable
	MFENUM_PLUGIN_STATE_INIT,			//inicializing inner data
	MFENUM_PLUGIN_STATE_WORK,			//inited and ready to work
	MFENUM_PLUGIN_STATE_ERROR			//error occured, plugin doesn't work
};

enum MFENUM_OPTIONS_STATE {
	MFENUM_OPTIONS_NEW,					//just started - unstable
	MFENUM_OPTIONS_INIT,				//inicializing inner data
	MFENUM_OPTIONS_WORK					//inited and ready to work
};

enum MFENUM_MIRANDAACCOUNT_STATE {
	MFENUM_MIRANDAACCOUNT_STATE_UNKNOWN,	//just loaded - unstable
	MFENUM_MIRANDAACCOUNT_STATE_ON,			//miranda account checked in options
	MFENUM_MIRANDAACCOUNT_STATE_OFF			//miranda account not checked in options
};

enum MFENUM_MIRANDACONTACT_STATE {
	MFENUM_MIRANDACONTACT_STATE_UNKNOWN,	//just loaded - unstable
	MFENUM_MIRANDACONTACT_STATE_ON,			//miranda contact checked in options
	MFENUM_MIRANDACONTACT_STATE_OFF			//miranda contact not checked in options
};

enum MFENUM_SEND_MESSAGE_MODE {
	MFENUM_SMM_ONLY_SEND			= 1,	// 1 - only send message, show popup
	MFENUM_SMM_SEND_AND_SHOW_MW		= 2,	// 2 - send message and open message window, no show popup
	MFENUM_SMM_ONLY_SHOW_MW			= 3		// 3 - only show message window with pasted message, no show popup
};




class MirandaAccount;
class MirandaContact;


class MirfoxData
{

public:

	MirfoxData(void);
	~MirfoxData(void);


	//Miranda handles to open message window hook procedure
	HANDLE hhook_EventOpenMW;
	HANDLE hhook_OpenMW;

	bool volatile Plugin_Terminated;
	int volatile workerThreadsCount;

	MFENUM_SEND_MESSAGE_MODE leftClickSendMode;
	MFENUM_SEND_MESSAGE_MODE rightClickSendMode;
	MFENUM_SEND_MESSAGE_MODE middleClickSendMode;


	/*
	 * getters, setters, adding
	 */

	//inline
	//get current plugin state MFENUM_PLUGINSTATE
	MFENUM_PLUGIN_STATE getPluginState() const {
		return pluginState;
	}

	//inline
	//set current plugin state MFENUM_PLUGINSTATE
	void setPluginState(MFENUM_PLUGIN_STATE pluginStateL){
		pluginState = pluginStateL;
	}








  //Contacts

	//get list of MirandaAContacts
	boost::ptr_list<MirandaContact>* getMirandaContacts();

	//update MirandaContact's state by id
	//return 0 - ok,
	int updateMirandaContactState(SharedMemoryUtils& sharedMemoryUtils, MCONTACT contactHandle, MFENUM_MIRANDACONTACT_STATE & contactState);

	//update All MirandaContact's names
	//return 0 - ok,
	int updateAllMirandaContactsNames(SharedMemoryUtils& sharedMemoryUtils);

	//return MirandaContact* by HANDLE
	MirandaContact* getMirandaContactPtrByHandle(MCONTACT contactHandle);


  //Accounts

	//get list of MirandaAccounts
	boost::ptr_list<MirandaAccount>* getMirandaAccounts();

	//update MirandaAccount's state by id
	//return 0 - ok,
	int updateMirandaAccountState(SharedMemoryUtils& sharedMemoryUtils, char* szModuleName, MFENUM_MIRANDAACCOUNT_STATE& accountState);

	//you MUST delete returned char* (if it is not NULL)
	char* getAccountSzModuleNameById(uint64_t id);

	//return MirandaAccount* by szModuleName
	MirandaAccount* getMirandaAccountPtrBySzModuleName(char* szModuleName);

	//refresh data support
	void refreshAccount_Add(SharedMemoryUtils& sharedMemoryUtils, char* szModuleName, wchar_t* tszAccountName, char* szProtoName);
	void refreshAccount_Edit(SharedMemoryUtils& sharedMemoryUtils, char* szModuleName, wchar_t* tszAccountName);
	void refreshAccount_Delete(SharedMemoryUtils& sharedMemoryUtils, char* szModuleName);
	void refreshContact_Add(SharedMemoryUtils& sharedMemoryUtils, MCONTACT hContact);
	void refreshContact_Edit(SharedMemoryUtils& sharedMemoryUtils, MCONTACT hContact);
	void refreshContact_Delete(SharedMemoryUtils& sharedMemoryUtils, MCONTACT hContact);


  //options

	MFENUM_OPTIONS_STATE tab1OptionsState;
	MFENUM_OPTIONS_STATE tab2OptionsState;
	MFENUM_OPTIONS_STATE tab3OptionsState;

	//inline
	//get clientsProfilesFilterCheckbox bool
	bool getClientsProfilesFilterCheckbox() const {
		return clientsProfilesFilterCheckbox;
	}

	//inline
	//set clientsProfilesFilterCheckbox bool
	void setClientsProfilesFilterCheckbox(bool clientsProfilesFilterCheckboxL){
		clientsProfilesFilterCheckbox = clientsProfilesFilterCheckboxL;
	}

	//inline
	//get ptr to clientsProfilesFilterWString std::string
	std::wstring * getClientsProfilesFilterStringPtr();
	//normalize clientsProfilesFilterString
	void normalizeClientsProfilesFilterString(std::size_t maxCSize);

	//inline
	//get addAccountToContactNameCheckbox bool
	bool getAddAccountToContactNameCheckbox() const {
		return addAccountToContactNameCheckbox;
	}

	//inline
	//set addAccountToContactNameCheckbox bool
	void setAddAccountToContactNameCheckbox(bool addAccountToContactNameCheckboxL){
		addAccountToContactNameCheckbox = addAccountToContactNameCheckboxL;
	}



	//id of process record in csm
	uint16_t processCsmId;


	/*
	 * functions
	 */

	void initializeMirfoxData();

	void releaseMirfoxData();


private:


	// current plugin state
	// @see MFENUM_PLUGINSTATE
	MFENUM_PLUGIN_STATE pluginState;


	// list of pointers to MirandaAccount class instances
	boost::ptr_list<MirandaAccount> mirandaAccounts;

	//initialize accounts list
	void initializeMirandaAccounts();

	uint64_t mirfoxAccountIdPool;
	int maxAccountIOrder;

	//add MirandaAccount item to list of MirandaAccounts
	void addMirandaAccount(MirandaAccount* mirandaAccountL);

	//clears list of MirandaAccounts
	void clearMirandaAccounts();


	// list of pointers to MirandaContact class instances
	boost::ptr_list<MirandaContact> mirandaContacts;

	//initialize contacts list
	void initializeMirandaContacts();

	//add MirandaContact item to list of MirandaContacts
	void addMirandaContact(MirandaContact* mirandaContactL);

	//clears list of MirandaContacts
	void clearMirandaContacts();

	//set contactNameW at MirandaContact to contact display name
	void setContactDisplayName(MirandaContact* mirandaContact);


	MFENUM_MIRANDAACCOUNT_STATE createOrGetAccountStateFromDB(MirandaAccount* mirandaAccount);

	MFENUM_MIRANDACONTACT_STATE createOrGetContactStateFromDB(MirandaContact* mirandaContact);


	static bool shouldProtoBeActiveByName(std::string protoName);

	int getAccountDefaultState(MirandaAccount* account);

	int getContactDefaultState(MirandaContact* contact);


	//options
	bool clientsProfilesFilterCheckbox;
	std::wstring clientsProfilesFilterString;

	bool addAccountToContactNameCheckbox;

	void initializeOptions();


};




class MirandaAccount
{
public:

	MirandaAccount(uint64_t idL, char* szModuleNameL, wchar_t* tszAccountNameL, char* szProtoNameL, int displayOrderL);
	~MirandaAccount(void);
	MirandaAccount*	getObjectPtr();

	uint64_t					id;					//selfgenerated id, used in sm
	char*						szModuleName;		//unique string id of account
	wchar_t*						tszAccountName;		//account name
	char*						szProtoName;		//physical protocol name
	int							displayOrder;		//presentation order - not used now
	MFENUM_MIRANDAACCOUNT_STATE accountState;		//state in options

};


class MirandaContact
{
public:

	MirandaContact(MCONTACT contactHandleL);
	~MirandaContact(void);
	MirandaContact*	getObjectPtr();

	MCONTACT					contactHandle;		//HANDLE to contact in miranda (unique)
	std::wstring				contactNameW;		//presented name
	MFENUM_MIRANDACONTACT_STATE contactState;		//state in options
	MirandaAccount*				mirandaAccountPtr;	//contact's account

};







#endif //#ifndef _MIRFOXDATA_INC

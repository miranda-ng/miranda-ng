#include "common.h"
#include "MirfoxData.h"





/*
 * MirfoxData
 * class functions implementation
 */


MirfoxData::MirfoxData(void)
{
	pluginState = MFENUM_PLUGIN_STATE_NEW;
	tab1OptionsState = MFENUM_OPTIONS_NEW;
	tab2OptionsState = MFENUM_OPTIONS_NEW;

	Plugin_Terminated = false;
	workerThreadsCount = 0;

	clientsProfilesFilterCheckbox = false;

	leftClickSendMode = MFENUM_SMM_ONLY_SEND;
	rightClickSendMode = MFENUM_SMM_ONLY_SHOW_MW;
	middleClickSendMode = MFENUM_SMM_ONLY_SEND;

	processCsmId = 0;
}

MirfoxData::~MirfoxData(void)
{
}


//Contacts

void MirfoxData::addMirandaContact(MirandaContact* mirandaContactL){
	mirandaContacts.push_back(mirandaContactL);
}

boost::ptr_list<MirandaContact>* MirfoxData::getMirandaContacts(){
	return &mirandaContacts;
}

void MirfoxData::clearMirandaContacts(){
	mirandaContacts.clear(); //all pointers are deleted by boost
}

int
MirfoxData::updateMirandaContactState(MCONTACT contactHandle, MFENUM_MIRANDACONTACT_STATE & contactState)
{

	boost::ptr_list<MirandaContact>* mirandaContactsPtr = getMirandaContacts();
	boost::ptr_list<MirandaContact>::iterator mirandaContactsIter;
	for (mirandaContactsIter = mirandaContactsPtr->begin(); mirandaContactsIter != mirandaContactsPtr->end(); mirandaContactsIter++){
		if (mirandaContactsIter->contactHandle == contactHandle ){
			mirandaContactsIter->contactState = contactState;
			return 0;
		}
	}
	return 1; //mirandaContact not found

}

MirandaContact*
MirfoxData::getMirandaContactPtrByHandle(MCONTACT contactHandle){

	MFLogger* logger = MFLogger::getInstance();

	if (contactHandle == NULL){
		logger->log(L"getMirandaContactPtrByHandle: return NULL for HANDLE: [NULL]");
		return NULL;
	}

	boost::ptr_list<MirandaContact>* mirandaContactsPtr = getMirandaContacts();
	boost::ptr_list<MirandaContact>::iterator mirandaContactsIter;
	for (mirandaContactsIter = mirandaContactsPtr->begin(); mirandaContactsIter != mirandaContactsPtr->end(); mirandaContactsIter++){
		if (mirandaContactsIter->contactHandle == contactHandle ){
			logger->log_p(L"getMirandaContactPtrByHandle: found MirandaContact for HANDLE: [" SCNuPTR L"]", contactHandle);
			return mirandaContactsIter->getObjectPtr();
		}
	}

	logger->log_p(L"getMirandaContactPtrByHandle: return NULL for HANDLE: [" SCNuPTR L"]", contactHandle);
	return NULL; //mirandaContact not found

}



//Accounts

void MirfoxData::addMirandaAccount(MirandaAccount* mirandaAccountL){
	mirandaAccounts.push_back(mirandaAccountL);
}

boost::ptr_list<MirandaAccount>* MirfoxData::getMirandaAccounts(){
	return &mirandaAccounts;
}

void MirfoxData::clearMirandaAccounts(){
	mirandaAccounts.clear(); //all pointers are deleted by boost
}

int
MirfoxData::updateMirandaAccountState(char* szModuleName, MFENUM_MIRANDAACCOUNT_STATE& accountState)
{

	boost::ptr_list<MirandaAccount>* mirandaAccountsPtr = getMirandaAccounts();
	boost::ptr_list<MirandaAccount>::iterator mirandaAccountsIter;
	for (mirandaAccountsIter = mirandaAccountsPtr->begin(); mirandaAccountsIter != mirandaAccountsPtr->end(); mirandaAccountsIter++){
		if (strcmp(mirandaAccountsIter->szModuleName, szModuleName) == 0 ){
			mirandaAccountsIter->accountState = accountState;
			return 0;
		}
	}

	return 1; //mirandaAccount not found
}

//you MUST delete returned char* (if it is not NULL)
char*
MirfoxData::getAccountSzModuleNameById(uint64_t id)
{

	MFLogger* logger = MFLogger::getInstance();

	boost::ptr_list<MirandaAccount>* mirandaAccountsPtr = getMirandaAccounts();
	boost::ptr_list<MirandaAccount>::iterator mirandaAccountsIter;
	for (mirandaAccountsIter = mirandaAccountsPtr->begin(); mirandaAccountsIter != mirandaAccountsPtr->end(); mirandaAccountsIter++){
		if (mirandaAccountsIter->id == id){

			size_t len = strlen(mirandaAccountsIter->szModuleName) + 1;
			char* returnPtr = new char[len];
			strcpy_s(returnPtr, len, mirandaAccountsIter->szModuleName);
			logger->log_p(L"getAccountSzModuleNameById: return: [%S]   for id = [%I64u]", returnPtr, id);
			return returnPtr;

		}
	}

	logger->log_p(L"getAccountSzModuleNameById: return NULL for id = [%I64u]", id);
	return NULL; //mirandaAccount not found
}



MirandaAccount*
MirfoxData::getMirandaAccountPtrBySzModuleName(char* szModuleName)
{

	MFLogger* logger = MFLogger::getInstance();

	if (szModuleName == NULL){
		logger->log(L"getMirandaAccountPtrBySzModuleName: return NULL for szModuleName: [NULL]");
		return NULL;
	}

	boost::ptr_list<MirandaAccount>* mirandaAccountsPtr = getMirandaAccounts();
	boost::ptr_list<MirandaAccount>::iterator mirandaAccountsIter;
	for (mirandaAccountsIter = mirandaAccountsPtr->begin(); mirandaAccountsIter != mirandaAccountsPtr->end(); mirandaAccountsIter++){
		if (mirandaAccountsIter->szModuleName != NULL && strcmp(mirandaAccountsIter->szModuleName, szModuleName) == 0){
			logger->log_p(L"getMirandaAccountPtrBySzModuleName: found MirandaAccount for szModuleName: [%S]", szModuleName);
			return mirandaAccountsIter->getObjectPtr();
		}
	}

	logger->log_p(L"getMirandaAccountPtrBySzModuleName: return NULL for  szModuleName: [%S]", szModuleName);
	return NULL; //mirandaAccount not found

}



//options

//get ptr to clientsProfilesFilterWString std::string
std::wstring * MirfoxData::getClientsProfilesFilterStringPtr() {
	return & clientsProfilesFilterString;
}

void MirfoxData::normalizeClientsProfilesFilterString(std::size_t maxCSize){

	boost::replace_all(clientsProfilesFilterString, L" ", L",");
	boost::replace_all(clientsProfilesFilterString, L";", L",");
	boost::replace_all(clientsProfilesFilterString, L"|", L",");
	while (clientsProfilesFilterString.find(L",,") != std::wstring::npos) {
		boost::replace_all(clientsProfilesFilterString, L",,", L",");
	}

	if (clientsProfilesFilterString.size() + 1 > maxCSize){
		clientsProfilesFilterString.resize(maxCSize);
	}

}



void
MirfoxData::initializeMirfoxData()
{

	initializeMirandaAccounts();	//must be before initializeMirandaContacts
	initializeMirandaContacts();
	initializeOptions();

}





/*static*/ bool
MirfoxData::shouldProtoBeActiveByName(std::string protoName)
{
	if (
			boost::iequals("ExchangeRates", protoName)
		||	boost::iequals("mTV", protoName)
		||	boost::iequals("Quotes", protoName)
		||	boost::iequals("Weather", protoName)
		||	boost::iequals("GmailMNotifier", protoName)
		||	boost::iequals("RSSNews", protoName)
		||	boost::iequals("PING", protoName)
		||	boost::iequals("WorldTime", protoName)
		||	boost::iequals("NIM_Contact", protoName)
		||	boost::iequals("POP3", protoName)
		||	boost::iequals("webview", protoName)
		||	boost::iequals("YAMN", protoName)
		||	boost::iequals("lotusnotify", protoName)
		||	boost::iequals("webinfo", protoName)
		||	boost::iequals("infofromweb", protoName)
	){
		return false;
	}

	return true;
}

//return 1 if on, 2 if off
int
MirfoxData::getAccountDefaultState(MirandaAccount* account)
{
	if (account == NULL){
		return 2;
	}

	if (shouldProtoBeActiveByName(account->szProtoName)){
		return 1;
	} else {
		return 2;
	}
}

//return 1 if on, 2 if off
int
MirfoxData::getContactDefaultState(MirandaContact* contact)
{
	MFLogger* logger = MFLogger::getInstance();

	if (contact == NULL)
		return 2;

	if (contact->mirandaAccountPtr == NULL)
		return 2;

	if (contact->mirandaAccountPtr->szProtoName == NULL)
		return 2;

	if (!shouldProtoBeActiveByName(contact->mirandaAccountPtr->szProtoName))
		return 2;

	if (db_get_b(contact->contactHandle, "CList", "Hidden", 0) == 1 || db_get_b(contact->contactHandle, "CList", "NotOnList", 0) == 1 )
		return 2;

	return 1;
}

void
MirfoxData::initializeMirandaAccounts()
{

	clearMirandaAccounts();

	int accountsCount = 0;
	PROTOACCOUNT **accounts;

	//get accounts from Miranda by CallService MS_PROTO_ENUMACCOUNTS
	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&accountsCount, (LPARAM)&accounts);

	uint64_t protocolId = 1;

	for(int i=0; i<accountsCount; i++) {

		//checking account
		if(accounts[i]->bIsEnabled == 0){
			continue;
		}
		if(accounts[i]->bDynDisabled != 0){
			continue;
		}

		//add to list
		MirandaAccount* mirandaAccountItemPtr = new MirandaAccount(
			protocolId,
			accounts[i]->szModuleName,
			accounts[i]->tszAccountName,
			accounts[i]->szProtoName,
			accounts[i]->iOrder
		  );

		MFLogger* logger = MFLogger::getInstance();
		logger->log_p(L"initializeMirandaAccounts: tszAccountName: [%s]   protocol: [%S]", accounts[i]->tszAccountName, accounts[i]->szProtoName );

		protocolId++;

		std::string mirandaAccountDBKey("ACCOUNTSTATE_");
		mirandaAccountDBKey += accounts[i]->szModuleName;

		int keyValue = db_get_b(0, PLUGIN_DB_ID, mirandaAccountDBKey.c_str(), 0);
		if (keyValue == 1 || keyValue == 2){
			//setting exist
			if (keyValue == 1){
				mirandaAccountItemPtr->accountState = MFENUM_MIRANDAACCOUNT_STATE_ON;	//1
			} else {
				mirandaAccountItemPtr->accountState = MFENUM_MIRANDAACCOUNT_STATE_OFF;	//2
			}
		} else {
			//setting does not exist, or is invalid -> save default setting (1 - ON)
			if (MirfoxData::getAccountDefaultState(mirandaAccountItemPtr) == 1){ //on = 1
				mirandaAccountItemPtr->accountState = MFENUM_MIRANDAACCOUNT_STATE_ON;	//1
				db_set_b(0, PLUGIN_DB_ID, mirandaAccountDBKey.c_str(), 1);
			} else { //off = 2
				mirandaAccountItemPtr->accountState = MFENUM_MIRANDAACCOUNT_STATE_OFF;	//2
				db_set_b(0, PLUGIN_DB_ID, mirandaAccountDBKey.c_str(), 2);
			}
		}

		addMirandaAccount(mirandaAccountItemPtr);

	}

	//TODO - sort by mirandaAccount.displayOrder

}

void MirfoxData::initializeMirandaContacts()
{

	MFLogger* logger = MFLogger::getInstance();

	//clean data
	clearMirandaContacts();


	//get contects from miranda
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)){
		//add to list
		MirandaContact* mirandaContactItemPtr = new MirandaContact(
			hContact	  //handle to contact in miranda
		  );
		addMirandaContact(mirandaContactItemPtr);
	}


	boost::ptr_list<MirandaContact>* mirandaContactsPtr = getMirandaContacts();
	boost::ptr_list<MirandaContact>::iterator mirandaContactsIter;


	//determine contact's account
	for (mirandaContactsIter = mirandaContactsPtr->begin(); mirandaContactsIter != mirandaContactsPtr->end(); mirandaContactsIter++){

		logger->log_p(L"initializeMirandaContacts: try to get account for hContact = [" SCNuPTR L"]", mirandaContactsIter->contactHandle);
		char* szModuleName = (char*)CallService(MS_PROTO_GETCONTACTBASEACCOUNT, (WPARAM)(mirandaContactsIter->contactHandle), 0);
		if (szModuleName == NULL){
			continue;  //mirandaContactsIter->mirandaAccountPtr will be NULL
		}
		mirandaContactsIter->mirandaAccountPtr = getMirandaAccountPtrBySzModuleName(szModuleName);

	}


	//determine contact's name
	for (mirandaContactsIter = mirandaContactsPtr->begin(); mirandaContactsIter != mirandaContactsPtr->end(); mirandaContactsIter++){

		logger->log_p(L"initializeMirandaContacts: try to get name for hContact = [" SCNuPTR L"]", mirandaContactsIter->contactHandle);

		if (mirandaContactsIter->mirandaAccountPtr != NULL){

			if ( strcmp(mirandaContactsIter->mirandaAccountPtr->szProtoName, "Twitter") == 0){
				//hack for Twitter protocol

				DBVARIANT dbv;
				if (!db_get_s(mirandaContactsIter->contactHandle, mirandaContactsIter->mirandaAccountPtr->szModuleName, "Username", &dbv, DBVT_WCHAR)) {
					mirandaContactsIter->contactNameW = std::wstring(dbv.pwszVal);
					db_free(&dbv);
				}

			} else {
				//standard miranda way for another protocols

				mirandaContactsIter->contactNameW =
						(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)mirandaContactsIter->contactHandle, GCDNF_TCHAR);
						//get contact's display name from clist

			}

		}

		if (mirandaContactsIter->contactNameW.size() == 0){
			//last chance (if some hack didn't work or mirandaContactsIter->mirandaAccountPtr is NULL)
			mirandaContactsIter->contactNameW =
					(TCHAR*)CallService(MS_CLIST_GETCONTACTDISPLAYNAME, (WPARAM)mirandaContactsIter->contactHandle, GCDNF_TCHAR);
					//get contact's display name from clist
		}

		logger->log_p(L"initializeMirandaContacts: got name for hContact = [" SCNuPTR L"]  is: [%s]", mirandaContactsIter->contactHandle,
				&(mirandaContactsIter->contactNameW)==NULL ? L"<null>" : mirandaContactsIter->contactNameW.c_str());

	}


	//determine contact's state
	for (mirandaContactsIter = mirandaContactsPtr->begin(); mirandaContactsIter != mirandaContactsPtr->end(); mirandaContactsIter++){

		logger->log_p(L"initializeMirandaContacts: try to get state for hContact = [" SCNuPTR L"]", mirandaContactsIter->contactHandle);

		int keyValue = db_get_b(mirandaContactsIter->contactHandle, PLUGIN_DB_ID, "state", 0);
		if (keyValue == 1 || keyValue == 2){
			//setting exist
			if (keyValue == 1){
				mirandaContactsIter->contactState = MFENUM_MIRANDACONTACT_STATE_ON;		//1
			} else {
				mirandaContactsIter->contactState = MFENUM_MIRANDACONTACT_STATE_OFF;	//2
			}
		} else {
			//setting does not exist, or is invalid -> save default setting (1 - ON)
			if (MirfoxData::getContactDefaultState(mirandaContactsIter->getObjectPtr()) == 1){ //on = 1
				mirandaContactsIter->contactState = MFENUM_MIRANDACONTACT_STATE_ON;		//1
				db_set_b(mirandaContactsIter->contactHandle, PLUGIN_DB_ID, "state", 1);
			} else { //off = 2
				mirandaContactsIter->contactState = MFENUM_MIRANDACONTACT_STATE_OFF;	//2
				db_set_b(mirandaContactsIter->contactHandle, PLUGIN_DB_ID, "state", 2);
			}
		}

	}



}





void MirfoxData::initializeOptions()
{

	//clientsProfilesFilterCheckbox
	int opt1KeyValue = db_get_b(0, PLUGIN_DB_ID, "clientsProfilesFilterCheckbox", 0);
	if (opt1KeyValue == 1 || opt1KeyValue == 2){
		//setting exist
		if (opt1KeyValue == 1){
			setClientsProfilesFilterCheckbox(true);  //1
		} else {
			setClientsProfilesFilterCheckbox(false);  //2
		}
	} else {
		//setting does not exist, or is invalid -> save default setting (2 - false)
		setClientsProfilesFilterCheckbox(false);	 //2
		db_set_b(0, PLUGIN_DB_ID, "clientsProfilesFilterCheckbox", 2);
	}


	//clientsProfilesFilterString
	DBVARIANT opt2Dbv = {0};
	INT_PTR opt2Result = db_get_s(0, PLUGIN_DB_ID, "clientsProfilesFilterString", &opt2Dbv, DBVT_TCHAR);
	if (opt2Result == 0){	//success
		//option exists in DB, get value
		(* getClientsProfilesFilterStringPtr()) = opt2Dbv.pwszVal;
	} else {
		//option not exists in DB, set default value
		(* getClientsProfilesFilterStringPtr()) = L"";
		db_set_ts(0, PLUGIN_DB_ID, "clientsProfilesFilterString", getClientsProfilesFilterStringPtr()->c_str());
	}
	db_free(&opt2Dbv);



	int opt3KeyValue = db_get_b(0, PLUGIN_DB_ID, "leftClickSendMode", 0);
	if (opt3KeyValue == MFENUM_SMM_ONLY_SEND || opt3KeyValue == MFENUM_SMM_SEND_AND_SHOW_MW || opt3KeyValue == MFENUM_SMM_ONLY_SHOW_MW){
		//setting exist
		leftClickSendMode = (MFENUM_SEND_MESSAGE_MODE)opt3KeyValue;
	} else {
		//setting does not exist, or is invalid -> save default setting (MFENUM_SMM_ONLY_SEND)
		leftClickSendMode = MFENUM_SMM_ONLY_SEND;
		db_set_b(0, PLUGIN_DB_ID, "leftClickSendMode", MFENUM_SMM_ONLY_SEND);
	}

	int opt4KeyValue = db_get_b(0, PLUGIN_DB_ID, "rightClickSendMode", 0);
	if (opt4KeyValue == MFENUM_SMM_ONLY_SEND || opt4KeyValue == MFENUM_SMM_SEND_AND_SHOW_MW || opt4KeyValue == MFENUM_SMM_ONLY_SHOW_MW){
		//setting exist
		rightClickSendMode = (MFENUM_SEND_MESSAGE_MODE)opt4KeyValue;
	} else {
		//setting does not exist, or is invalid -> save default setting (MFENUM_SMM_SEND_AND_SHOW_MW)
		rightClickSendMode = MFENUM_SMM_SEND_AND_SHOW_MW;
		db_set_b(0, PLUGIN_DB_ID, "rightClickSendMode", MFENUM_SMM_SEND_AND_SHOW_MW);
	}

	int opt5KeyValue = db_get_b(0, PLUGIN_DB_ID, "middleClickSendMode", 0);
	if (opt5KeyValue == MFENUM_SMM_ONLY_SEND || opt5KeyValue == MFENUM_SMM_SEND_AND_SHOW_MW || opt5KeyValue == MFENUM_SMM_ONLY_SHOW_MW){
		//setting exist
		middleClickSendMode = (MFENUM_SEND_MESSAGE_MODE)opt5KeyValue;
	} else {
		//setting does not exist, or is invalid -> save default setting (must be MFENUM_SMM_ONLY_SEND due to Firefox bug and crash)
		middleClickSendMode = MFENUM_SMM_ONLY_SEND;
		db_set_b(0, PLUGIN_DB_ID, "middleClickSendMode", MFENUM_SMM_ONLY_SEND);
	}



}



void MirfoxData::releaseMirfoxData()
{

	clearMirandaContacts();
	clearMirandaAccounts();

}









/*
 * MirandaAccount
 * class functions implementation
 */

MirandaAccount::MirandaAccount(uint64_t idL, char* szModuleNameL, TCHAR* tszAccountNameL, char* szProtoNameL, int displayOrderL)
{
	accountState = MFENUM_MIRANDAACCOUNT_STATE_UNKNOWN;
	id = idL;
	szModuleName = szModuleNameL;
	tszAccountName = tszAccountNameL;
	szProtoName = szProtoNameL;
	displayOrder = displayOrderL;
}

MirandaAccount::~MirandaAccount(void)
{
}

MirandaAccount*
MirandaAccount::getObjectPtr()
{
	return this;
}




/*
 * MirandaContact
 * class functions implementation
 */

MirandaContact::MirandaContact(MCONTACT contactHandleL)
{
	contactState = MFENUM_MIRANDACONTACT_STATE_UNKNOWN;
	contactHandle = contactHandleL;
	mirandaAccountPtr = NULL;
}

MirandaContact::~MirandaContact(void)
{
}

MirandaContact*
MirandaContact::getObjectPtr()
{
	return this;
}







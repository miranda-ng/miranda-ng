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
	tab3OptionsState = MFENUM_OPTIONS_NEW;

	Plugin_Terminated = false;
	workerThreadsCount = 0;

	clientsProfilesFilterCheckbox = false;

	leftClickSendMode = MFENUM_SMM_ONLY_SEND;
	rightClickSendMode = MFENUM_SMM_ONLY_SHOW_MW;
	middleClickSendMode = MFENUM_SMM_ONLY_SEND;

	processCsmId = 0;
	hhook_EventOpenMW = nullptr;
	hhook_OpenMW = nullptr;

	mirfoxAccountIdPool = 1;
	maxAccountIOrder = 0;
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
MirfoxData::updateMirandaContactState(SharedMemoryUtils& sharedMemoryUtils, MCONTACT contactHandle, MFENUM_MIRANDACONTACT_STATE & contactState)
{

	boost::ptr_list<MirandaContact>* mirandaContactsPtr = getMirandaContacts();
	boost::ptr_list<MirandaContact>::iterator mirandaContactsIter;
	for (mirandaContactsIter = mirandaContactsPtr->begin(); mirandaContactsIter != mirandaContactsPtr->end(); mirandaContactsIter++){
		if (mirandaContactsIter->contactHandle == contactHandle ){
			MFENUM_MIRANDACONTACT_STATE oldState = mirandaContactsIter->contactState;
			mirandaContactsIter->contactState = contactState;
			if (contactState != oldState ){
				if (contactState == MFENUM_MIRANDACONTACT_STATE_ON){
					sharedMemoryUtils.refreshMsm_Add('C', (uint64_t)mirandaContactsIter->contactHandle, mirandaContactsIter->contactNameW);
				} else {
					sharedMemoryUtils.refreshMsm_Delete('C', (uint64_t)mirandaContactsIter->contactHandle);
				}
			}
			return 0;
		}
	}
	return 1; //mirandaContact not found

}

int
MirfoxData::updateAllMirandaContactsNames(SharedMemoryUtils& sharedMemoryUtils)
{

	boost::ptr_list<MirandaContact>* mirandaContactsPtr = getMirandaContacts();
	boost::ptr_list<MirandaContact>::iterator mirandaContactsIter;
	for (mirandaContactsIter = mirandaContactsPtr->begin(); mirandaContactsIter != mirandaContactsPtr->end(); mirandaContactsIter++){

		setContactDisplayName(mirandaContactsIter->getObjectPtr());

		if (mirandaContactsIter->contactState == MFENUM_MIRANDACONTACT_STATE_ON){
			sharedMemoryUtils.refreshMsm_Edit('C', (uint64_t)mirandaContactsIter->contactHandle, mirandaContactsIter->contactNameW);
		}

	}
	return 0;

}


MirandaContact*
MirfoxData::getMirandaContactPtrByHandle(MCONTACT contactHandle){

	MFLogger* logger = MFLogger::getInstance();

	if (contactHandle == NULL){
		logger->log(L"getMirandaContactPtrByHandle: return NULL for HANDLE: [NULL]");
		return nullptr;
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
	return nullptr; //mirandaContact not found

}


void
MirfoxData::setContactDisplayName(MirandaContact* mirandaContact){

	if (mirandaContact->mirandaAccountPtr != nullptr && strcmp(mirandaContact->mirandaAccountPtr->szProtoName, "Twitter") == 0){
		// hack for Twitter protocol
		DBVARIANT dbv;
		if (!db_get_s(mirandaContact->contactHandle, mirandaContact->mirandaAccountPtr->szModuleName, "Username", &dbv, DBVT_WCHAR)) {
			mirandaContact->contactNameW = std::wstring(dbv.pwszVal);
			db_free(&dbv);
		}
	} else {
		// standard miranda way for another protocols
		mirandaContact->contactNameW = Clist_GetContactDisplayName(mirandaContact->contactHandle);
	}

	if (getAddAccountToContactNameCheckbox()){
		mirandaContact->contactNameW = mirandaContact->contactNameW.append(L" (").append(mirandaContact->mirandaAccountPtr->tszAccountName).append(L")");

	}

	MFLogger::getInstance()->log_p(L"initializeMirandaContacts: got name for hContact = [" SCNuPTR L"]  is: [%s]", mirandaContact->contactHandle,
			&(mirandaContact->contactNameW)==nullptr ? L"<null>" : mirandaContact->contactNameW.c_str());

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
MirfoxData::updateMirandaAccountState(SharedMemoryUtils& sharedMemoryUtils, char* szModuleName, MFENUM_MIRANDAACCOUNT_STATE& accountState)
{

	boost::ptr_list<MirandaAccount>* mirandaAccountsPtr = getMirandaAccounts();
	boost::ptr_list<MirandaAccount>::iterator mirandaAccountsIter;
	for (mirandaAccountsIter = mirandaAccountsPtr->begin(); mirandaAccountsIter != mirandaAccountsPtr->end(); mirandaAccountsIter++){
		if (strcmp(mirandaAccountsIter->szModuleName, szModuleName) == 0 ){
			MFENUM_MIRANDAACCOUNT_STATE oldState = mirandaAccountsIter->accountState;
			mirandaAccountsIter->accountState = accountState;
			if (accountState != oldState ){
				if (accountState == MFENUM_MIRANDAACCOUNT_STATE_ON){
					std::wstring tszAccountNameW = mirandaAccountsIter->tszAccountName;
					sharedMemoryUtils.refreshMsm_Add('A', (uint64_t)mirandaAccountsIter->id, tszAccountNameW);
				} else {
					sharedMemoryUtils.refreshMsm_Delete('A', (uint64_t)mirandaAccountsIter->id);
				}
			}
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
	return nullptr; //mirandaAccount not found
}



MirandaAccount*
MirfoxData::getMirandaAccountPtrBySzModuleName(char* szModuleName)
{

	MFLogger* logger = MFLogger::getInstance();

	if (szModuleName == nullptr){
		logger->log(L"getMirandaAccountPtrBySzModuleName: return NULL for szModuleName: [NULL]");
		return nullptr;
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
	return nullptr; //mirandaAccount not found

}




// refresh support

void MirfoxData::refreshAccount_Add(SharedMemoryUtils& sharedMemoryUtils, char* szModuleName, wchar_t* tszAccountName, char* szProtoName)
{
	MFLogger* logger = MFLogger::getInstance();
	logger->log_p(L"MirfoxData::refreshAccount_Add  proto [%S]", szModuleName);

	//add proto to mirandaAccounts
	mirfoxAccountIdPool++;
	maxAccountIOrder++;

	MirandaAccount* mirandaAccountItemPtr = new MirandaAccount(
		mirfoxAccountIdPool,
		szModuleName,
		tszAccountName,
		szProtoName,
		maxAccountIOrder
	  );

	mirandaAccountItemPtr->accountState = createOrGetAccountStateFromDB(mirandaAccountItemPtr);

	/*
	MFENUM_MIRANDAACCOUNT_STATE getOrCreateAccountStateInDB(char* szModuleName);
	std::string mirandaAccountDBKey("ACCOUNTSTATE_");
	mirandaAccountDBKey += szModuleName;
	int keyValue = g_plugin.getByte(mirandaAccountDBKey.c_str(), 0);
	if (keyValue == 1 || keyValue == 2){
		//setting exist
		if (keyValue == 1){
			mirandaAccountItemPtr->accountState = MFENUM_MIRANDAACCOUNT_STATE_ON;	//1
		} else {
			mirandaAccountItemPtr->accountState = MFENUM_MIRANDAACCOUNT_STATE_OFF;	//2
		}
	} else {
		//setting does not exist, or is invalid -> save default setting (1 - ON)
		if (getAccountDefaultState(mirandaAccountItemPtr) == 1){ //on = 1
			mirandaAccountItemPtr->accountState = MFENUM_MIRANDAACCOUNT_STATE_ON;	//1
			g_plugin.setByte(mirandaAccountDBKey.c_str(), 1);
		} else { //off = 2
			mirandaAccountItemPtr->accountState = MFENUM_MIRANDAACCOUNT_STATE_OFF;	//2
			g_plugin.setByte(mirandaAccountDBKey.c_str(), 2);
		}
	}
	 */
	addMirandaAccount(mirandaAccountItemPtr);

	//add proto to SM
	std::wstring tszAccountNameW = mirandaAccountItemPtr->tszAccountName;
	sharedMemoryUtils.refreshMsm_Add('A', mirandaAccountItemPtr->id, tszAccountNameW);

	return;
}

void MirfoxData::refreshAccount_Edit(SharedMemoryUtils& sharedMemoryUtils, char* szModuleName, wchar_t* tszAccountName)
{
	MFLogger* logger = MFLogger::getInstance();
	logger->log_p(L"MirfoxData::refreshAccount_Edit  proto [%S]", szModuleName);

	//edit proto in mirandaAccounts
	MirandaAccount* mirandaAccount = getMirandaAccountPtrBySzModuleName(szModuleName);
	if (!mirandaAccount){
		logger->log(L"MirfoxData::refreshAccount_Edit  edit proto not found in mirandaAccounts");
		return;
	}

	mirandaAccount->tszAccountName = tszAccountName;

	//edit proto in SM
	std::wstring tszAccountNameW = mirandaAccount->tszAccountName;
	sharedMemoryUtils.refreshMsm_Edit('A', mirandaAccount->id, tszAccountNameW);

	return;
}

void MirfoxData::refreshAccount_Delete(SharedMemoryUtils& sharedMemoryUtils, char* szModuleName)
{
	MFLogger* logger = MFLogger::getInstance();
	logger->log_p(L"MirfoxData::refreshAccount_Delete  proto [%S]", szModuleName);

	uint64_t deletedId;

	//del proto in mirandaAccounts
	boost::ptr_list<MirandaAccount>* mirandaAccountsPtr = getMirandaAccounts();
	boost::ptr_list<MirandaAccount>::iterator mirandaAccountsIter;
	for (mirandaAccountsIter = mirandaAccountsPtr->begin(); mirandaAccountsIter != mirandaAccountsPtr->end(); mirandaAccountsIter++){
		if (mirandaAccountsIter->szModuleName != NULL && strcmp(mirandaAccountsIter->szModuleName, szModuleName) == 0){
			deletedId = mirandaAccountsIter->id;
			mirandaAccountsPtr->erase(mirandaAccountsIter);
			break;
		}
	}

	std::string mirandaAccountDBKey("ACCOUNTSTATE_");
	mirandaAccountDBKey += szModuleName;
	g_plugin.delSetting(mirandaAccountDBKey.c_str());

	//del proto from SM
	sharedMemoryUtils.refreshMsm_Delete('A', deletedId);

	return;
}

void MirfoxData::refreshContact_Add(SharedMemoryUtils& sharedMemoryUtils, MCONTACT hContact)
{
	MFLogger* logger = MFLogger::getInstance();
	logger->log_p(L"MirfoxData::refreshContact_Add  hContact [" SCNuPTR L"]", hContact);

	//add contact to mirandaContacts
	MirandaContact* mirandaContactItemPtr = new MirandaContact(
		hContact	  //handle to contact in miranda
	  );

	char *szModuleName = Proto_GetBaseAccountName(mirandaContactItemPtr->contactHandle);
	if (szModuleName != nullptr)
		mirandaContactItemPtr->mirandaAccountPtr = getMirandaAccountPtrBySzModuleName(szModuleName);

	// Always getting '(Unknown Contact)' here if called from HookEvent ME_DB_CONTACT_ADDED, (updated to proper via ME_DB_CONTACT_SETTINGCHANGED)
	setContactDisplayName(mirandaContactItemPtr);

	mirandaContactItemPtr->contactState = createOrGetContactStateFromDB(mirandaContactItemPtr);

	addMirandaContact(mirandaContactItemPtr);


	//add contact to SM
	sharedMemoryUtils.refreshMsm_Add('C', (uint64_t)mirandaContactItemPtr->contactHandle, mirandaContactItemPtr->contactNameW);


	return;
}

void MirfoxData::refreshContact_Edit(SharedMemoryUtils& sharedMemoryUtils, MCONTACT hContact)
{
	MFLogger* logger = MFLogger::getInstance();
	logger->log_p(L"MirfoxData::refreshContact_Edit  hContact [" SCNuPTR L"]", hContact);


	MirandaContact* mirandaContact = getMirandaContactPtrByHandle(hContact);

	if (!mirandaContact){
		logger->log(L"refreshContact_Edit  edited contact not found in mirandaContactss");
		return;
	}

	setContactDisplayName(mirandaContact);


	// edit contact in SM
	sharedMemoryUtils.refreshMsm_Edit('C', (uint64_t)mirandaContact->contactHandle, mirandaContact->contactNameW);


	return;
}

void MirfoxData::refreshContact_Delete(SharedMemoryUtils& sharedMemoryUtils, MCONTACT hContact)
{
	MFLogger* logger = MFLogger::getInstance();
	logger->log_p(L"MirfoxData::refreshContact_Delete  hContact [" SCNuPTR L"]", hContact);

	//del contact from mirandaContacts
	boost::ptr_list<MirandaContact>* mirandaContactsPtr = getMirandaContacts();
	boost::ptr_list<MirandaContact>::iterator mirandaContactsIter;
	for (mirandaContactsIter = mirandaContactsPtr->begin(); mirandaContactsIter != mirandaContactsPtr->end(); mirandaContactsIter++){
		if (mirandaContactsIter->contactHandle == hContact ){
			mirandaContactsPtr->erase(mirandaContactsIter);
			break;
		}
	}


	//del contact from SM
	sharedMemoryUtils.refreshMsm_Delete('C', (uint64_t)hContact);


	return;
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

	initializeOptions();
	initializeMirandaAccounts();	//must be before initializeMirandaContacts
	initializeMirandaContacts();

}





/*static*/ bool
MirfoxData::shouldProtoBeActiveByName(std::string protoName)
{
	if (  boost::iequals("MetaContacts", protoName)
		||	boost::iequals("CurrencyRates", protoName)
		||	boost::iequals("Weather", protoName)
		||	boost::iequals("GmailMNotifier", protoName)
		||	boost::iequals("NewsAggregator", protoName)
		||	boost::iequals("PING", protoName)
		||	boost::iequals("NIM_Contact", protoName)
		||	boost::iequals("POP3", protoName)
		||	boost::iequals("webview", protoName)
		||	boost::iequals("YAMN", protoName)
		||	boost::iequals("lotusnotify", protoName))
	{
		return false;
	}

	return true;
}

//return 1 if on, 2 if off
int
MirfoxData::getAccountDefaultState(MirandaAccount* account)
{
	if (account == nullptr){
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

	if (contact == nullptr)
		return 2;

	if (contact->mirandaAccountPtr == nullptr)
		return 2;

	if (contact->mirandaAccountPtr->szProtoName == nullptr)
		return 2;

	if (!shouldProtoBeActiveByName(contact->mirandaAccountPtr->szProtoName))
		return 2;

	return 1;
}


MFENUM_MIRANDAACCOUNT_STATE
MirfoxData::createOrGetAccountStateFromDB(MirandaAccount* mirandaAccount){

	std::string mirandaAccountDBKey("ACCOUNTSTATE_");
	mirandaAccountDBKey += mirandaAccount->szModuleName;
	int keyValue = g_plugin.getByte(mirandaAccountDBKey.c_str(), 0);
	if (keyValue == 1 || keyValue == 2){
		//setting exist
		if (keyValue == 1){
			return MFENUM_MIRANDAACCOUNT_STATE_ON;	//1
		} else {
			return MFENUM_MIRANDAACCOUNT_STATE_OFF;	//2
		}
	} else {
		//setting does not exist, or is invalid -> save default setting (1 - ON)
		if (getAccountDefaultState(mirandaAccount) == 1){ //on = 1
			g_plugin.setByte(mirandaAccountDBKey.c_str(), 1);
			return MFENUM_MIRANDAACCOUNT_STATE_ON;	//1
		} else { //off = 2
			g_plugin.setByte(mirandaAccountDBKey.c_str(), 2);
			return MFENUM_MIRANDAACCOUNT_STATE_OFF;	//2
		}
	}

}

MFENUM_MIRANDACONTACT_STATE
MirfoxData::createOrGetContactStateFromDB(MirandaContact* mirandaContact){

	int keyValue = g_plugin.getByte(mirandaContact->contactHandle, "state");
	if (keyValue == 1 || keyValue == 2){
		//setting exist
		if (keyValue == 1){
			return MFENUM_MIRANDACONTACT_STATE_ON;	//1
		} else {
			return MFENUM_MIRANDACONTACT_STATE_OFF;	//2
		}
	} else {
		//setting does not exist, or is invalid -> save default setting (1 - ON)
		if (MirfoxData::getContactDefaultState(mirandaContact->getObjectPtr()) == 1){ //on = 1
			g_plugin.setByte(mirandaContact->contactHandle, "state", 1);
			return MFENUM_MIRANDACONTACT_STATE_ON;		//1
		} else { //off = 2
			g_plugin.setByte(mirandaContact->contactHandle, "state", 2);
			return MFENUM_MIRANDACONTACT_STATE_OFF;	//2
		}
	}

}


void
MirfoxData::initializeMirandaAccounts()
{
	clearMirandaAccounts();

	//get accounts from Miranda by CallService MS_PROTO_ENUMACCOUNTS
	for (auto &pa : Accounts()) {
		// checking account
		if (pa->bIsEnabled == 0)
			continue;

		if (pa->bDynDisabled != 0)
			continue;

		//add to list
		MirandaAccount* mirandaAccountItemPtr = new MirandaAccount(mirfoxAccountIdPool, pa->szModuleName, pa->tszAccountName, pa->szProtoName, pa->iOrder);

		MFLogger* logger = MFLogger::getInstance();
		logger->log_p(L"initializeMirandaAccounts: tszAccountName: [%s]   protocol: [%S]", pa->tszAccountName, pa->szProtoName);

		mirfoxAccountIdPool++;
		if (pa->iOrder > maxAccountIOrder)
			maxAccountIOrder = pa->iOrder;

		mirandaAccountItemPtr->accountState = createOrGetAccountStateFromDB(mirandaAccountItemPtr);

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
	for (auto &hContact : Contacts()){

		//"Hidden" contacts not allowed in MirfoxData and SM, "NotOnList" contacts allowed and enabled
		if (Contact::IsHidden(hContact))
			continue;

		//add to MirfoxData list
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
		char *szModuleName = Proto_GetBaseAccountName(mirandaContactsIter->contactHandle);
		if (szModuleName == NULL)
			continue;  //mirandaContactsIter->mirandaAccountPtr will be NULL

		mirandaContactsIter->mirandaAccountPtr = getMirandaAccountPtrBySzModuleName(szModuleName);

		//determine contact's name
		setContactDisplayName(mirandaContactsIter->getObjectPtr());

		//determine contact's state
		mirandaContactsIter->contactState = createOrGetContactStateFromDB(mirandaContactsIter->getObjectPtr());

	}

}




void MirfoxData::initializeOptions()
{

	//addAccountToContactNameCheckbox
	int opt2KeyValue = g_plugin.getByte("addAccountToContactNameCheckbox", 0);
	if (opt2KeyValue == 1 || opt2KeyValue == 2){
		//setting exist
		if (opt2KeyValue == 1){
			setAddAccountToContactNameCheckbox(true);  //1
		} else {
			setAddAccountToContactNameCheckbox(false);  //2
		}
	} else {
		//setting does not exist, or is invalid -> save default setting (2 - false)
		setAddAccountToContactNameCheckbox(false);	 //2
		g_plugin.setByte("addAccountToContactNameCheckbox", 2);
	}


	//clientsProfilesFilterCheckbox
	int opt1KeyValue = g_plugin.getByte("clientsProfilesFilterCheckbox", 0);
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
		g_plugin.setByte("clientsProfilesFilterCheckbox", 2);
	}

	// clientsProfilesFilterString
	ptrW opt2Result(g_plugin.getWStringA("clientsProfilesFilterString"));
	if (opt2Result != nullptr) { // success
		//option exists in DB, get value
		(* getClientsProfilesFilterStringPtr()) = opt2Result;
	} else {
		//option not exists in DB, set default value
		(* getClientsProfilesFilterStringPtr()) = L"";
		g_plugin.setWString("clientsProfilesFilterString", getClientsProfilesFilterStringPtr()->c_str());
	}

	int opt3KeyValue = g_plugin.getByte("leftClickSendMode", 0);
	if (opt3KeyValue == MFENUM_SMM_ONLY_SEND || opt3KeyValue == MFENUM_SMM_SEND_AND_SHOW_MW || opt3KeyValue == MFENUM_SMM_ONLY_SHOW_MW){
		//setting exist
		leftClickSendMode = (MFENUM_SEND_MESSAGE_MODE)opt3KeyValue;
	} else {
		//setting does not exist, or is invalid -> save default setting (MFENUM_SMM_ONLY_SEND)
		leftClickSendMode = MFENUM_SMM_ONLY_SEND;
		g_plugin.setByte("leftClickSendMode", MFENUM_SMM_ONLY_SEND);
	}

	int opt4KeyValue = g_plugin.getByte("rightClickSendMode", 0);
	if (opt4KeyValue == MFENUM_SMM_ONLY_SEND || opt4KeyValue == MFENUM_SMM_SEND_AND_SHOW_MW || opt4KeyValue == MFENUM_SMM_ONLY_SHOW_MW){
		//setting exist
		rightClickSendMode = (MFENUM_SEND_MESSAGE_MODE)opt4KeyValue;
	} else {
		//setting does not exist, or is invalid -> save default setting (MFENUM_SMM_SEND_AND_SHOW_MW)
		rightClickSendMode = MFENUM_SMM_SEND_AND_SHOW_MW;
		g_plugin.setByte("rightClickSendMode", MFENUM_SMM_SEND_AND_SHOW_MW);
	}

	int opt5KeyValue = g_plugin.getByte("middleClickSendMode", 0);
	if (opt5KeyValue == MFENUM_SMM_ONLY_SEND || opt5KeyValue == MFENUM_SMM_SEND_AND_SHOW_MW || opt5KeyValue == MFENUM_SMM_ONLY_SHOW_MW){
		//setting exist
		middleClickSendMode = (MFENUM_SEND_MESSAGE_MODE)opt5KeyValue;
	} else {
		//setting does not exist, or is invalid -> save default setting (must be MFENUM_SMM_ONLY_SEND due to Firefox bug and crash)
		middleClickSendMode = MFENUM_SMM_ONLY_SEND;
		g_plugin.setByte("middleClickSendMode", MFENUM_SMM_ONLY_SEND);
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

MirandaAccount::MirandaAccount(uint64_t idL, char* szModuleNameL, wchar_t* tszAccountNameL, char* szProtoNameL, int displayOrderL)
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
	mirandaAccountPtr = nullptr;
}

MirandaContact::~MirandaContact(void)
{
}

MirandaContact*
MirandaContact::getObjectPtr()
{
	return this;
}







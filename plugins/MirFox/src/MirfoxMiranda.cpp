#include "common.h"
#include "MirfoxMiranda.h"

CMirfoxMiranda::CMirfoxMiranda()
				: sharedMemoryUtils(SharedMemoryUtils::getInstance())
				, mirandaUtils(MirandaUtils::getInstance())
				, logger(MFLogger::getInstance())
{
}



CMirfoxMiranda::~CMirfoxMiranda()
{
}

MirfoxData& CMirfoxMiranda::getMirfoxData()
{
	return mirfoxData;
}

SharedMemoryUtils& CMirfoxMiranda::getSharedMemoryUtils()
{
	return *sharedMemoryUtils;
}

int CMirfoxMiranda::onMirandaInterfaceLoad()
{

	mirandaUtils->netlibRegister(); //for Miranda logger init
	logger->initLogger(&MirandaUtils::netlibLog);
	logger->set6CharsPrefix(L"MNG   ");

	mirandaUtils->translateOldDBNames();
	
	mirfoxData.initializeMirfoxData();
	logger->log(L"dll init, MirfoxData initialized");

	initializeSharedMemory(mirfoxData);
	if (mirfoxData.getPluginState() == MFENUM_PLUGIN_STATE_ERROR){
		return 0;
	}

	initializeMessageQueue(mirfoxData);
	if (mirfoxData.getPluginState() == MFENUM_PLUGIN_STATE_ERROR){
		return 0;
	}

	mirfoxData.hhook_EventOpenMW = CreateHookableEvent("MirFox/OpenMW");
	mirfoxData.hhook_OpenMW = HookEvent("MirFox/OpenMW", MirandaUtils::on_hook_OpenMW);

	commitSharedMemory();


	return 0;
}


void CMirfoxMiranda::onAccListChanged(WPARAM wParam, LPARAM lParam)
{
	PROTOACCOUNT *acc = (PROTOACCOUNT*)lParam;

	switch (wParam) {
	case PRAC_ADDED:
		if (acc != nullptr) {
			//checking account
			if(acc->bIsEnabled == 0 || acc->bDynDisabled != 0){
				return;
			}
			mirfoxData.refreshAccount_Add(getSharedMemoryUtils(), acc->szModuleName, acc->tszAccountName, acc->szProtoName);
		}
		break;

	case PRAC_UPGRADED:
	case PRAC_CHANGED:
		if (acc != nullptr) {
			//checking account
			if(acc->bIsEnabled == 0 || acc->bDynDisabled != 0){
				return;
			}
			mirfoxData.refreshAccount_Edit(getSharedMemoryUtils(), acc->szModuleName, acc->tszAccountName);
		}
		break;

	case PRAC_REMOVED:
		mirfoxData.refreshAccount_Delete(getSharedMemoryUtils(), acc->szModuleName);
		break;

	case PRAC_CHECKED:
		if (acc != nullptr) {
			if (acc->bIsEnabled) {
				//checking account
				if(acc->bDynDisabled != 0){
					return;
				}
				mirfoxData.refreshAccount_Add(getSharedMemoryUtils(), acc->szModuleName, acc->tszAccountName, acc->szProtoName);
			} else {
				mirfoxData.refreshAccount_Delete(getSharedMemoryUtils(), acc->szModuleName);
			}
		}
		break;
	}

	return;
}


void CMirfoxMiranda::onContactAdded_async(void* threadArg)
{
	Thread_Push(nullptr);
	OnContactAsyncThreadArgStruct* onContactAsyncThreadArgStruct = (OnContactAsyncThreadArgStruct*)threadArg;
	CMirfoxMiranda* mirfoxMiranda = onContactAsyncThreadArgStruct->mirfoxMiranda;
	MFLogger* logger = MFLogger::getInstance();

	// ME_DB_CONTACT_ADDED is fired when only hContact is known, wait some time for finish creating contact
	// wait some time for finish contact creation
	SleepEx(300, TRUE);
	logger->log_p(L"CMirfoxMiranda::onContactAdded_async  AFTER wait [" SCNuPTR L"]", onContactAsyncThreadArgStruct->hContact);
	bool canAdd = true;

	// ceck miranda
	if (Miranda_IsTerminated() || mirfoxMiranda->getMirfoxData().Plugin_Terminated)
		canAdd = false;

	// check if hContact still exist
	if (canAdd && !db_is_contact(onContactAsyncThreadArgStruct->hContact))
		canAdd = false;

	// execute
	if (canAdd && Contact::IsHidden(onContactAsyncThreadArgStruct->hContact))
		canAdd = false;

	// add
	if (canAdd)
		mirfoxMiranda->getMirfoxData().refreshContact_Add(mirfoxMiranda->getSharedMemoryUtils(), onContactAsyncThreadArgStruct->hContact);

	// clean
	delete onContactAsyncThreadArgStruct;
	Thread_Pop();
	return;
}


void CMirfoxMiranda::onContactDeleted(MCONTACT hContact)
{

	mirfoxData.refreshContact_Delete(getSharedMemoryUtils(), hContact);

	return;
}


void CMirfoxMiranda::onContactSettingChanged(MCONTACT hContact, LPARAM lParam)
{

	DBCONTACTWRITESETTING* cws = (DBCONTACTWRITESETTING*)lParam;
	logger->log_p(L"onContactSettingChanged CList Hidden  hContact [" SCNuPTR L"]", hContact);

	if (cws->value.bVal > 0){
		mirfoxData.refreshContact_Delete(getSharedMemoryUtils(), hContact);
	} else { //unset
		mirfoxData.refreshContact_Add(getSharedMemoryUtils(), hContact);
	}

	return;
}

void CMirfoxMiranda::onContactSettingChanged_async(void* threadArg){

	Thread_Push(nullptr);
	OnContactAsyncThreadArgStruct* onContactAsyncThreadArgStruct = (OnContactAsyncThreadArgStruct*)threadArg;
	CMirfoxMiranda* mirfoxMiranda = onContactAsyncThreadArgStruct->mirfoxMiranda;
	MFLogger* logger = MFLogger::getInstance();

	// this ME_DB_CONTACT_SETTINGCHANGED hook can be added after ME_DB_CONTACT_ADDED
	// if we delay ME_DB_CONTACT_ADDED routines, we must also delay this
	// wait some time for finish contact creation
	SleepEx(300+100, TRUE);
	logger->log_p(L"CMirfoxMiranda::onContactSettingChanged_async  AFTER wait [" SCNuPTR L"]", onContactAsyncThreadArgStruct->hContact);
	bool canAdd = true;

	// ceck miranda
	if (Miranda_IsTerminated() || mirfoxMiranda->getMirfoxData().Plugin_Terminated)
		canAdd = false;

	// check if hContact still exist
	if (canAdd && !db_is_contact(onContactAsyncThreadArgStruct->hContact))
		canAdd = false;

	// edit
	if (canAdd && Contact::IsHidden(onContactAsyncThreadArgStruct->hContact))
		canAdd = false;

	if (canAdd)
		mirfoxMiranda->getMirfoxData().refreshContact_Edit(mirfoxMiranda->getSharedMemoryUtils(), onContactAsyncThreadArgStruct->hContact);

	// clean
	delete onContactAsyncThreadArgStruct;
	Thread_Pop();
	return;
}





int CMirfoxMiranda::onMirandaInterfaceUnload()
{

	UnhookEvent(mirfoxData.hhook_OpenMW);
	DestroyHookableEvent(mirfoxData.hhook_EventOpenMW);

	unloadMessageQueue(mirfoxData.processCsmId);
	unloadSharedMemory();

	mirfoxData.releaseMirfoxData();

	logger->releaseLogger();
	mirandaUtils->netlibUnregister();

	return 0;
}

void CMirfoxMiranda::initializeSharedMemory(MirfoxData& mirfoxData)
{

	//initialize CSM record and MSMs with data from mirfoxData
	initializeSharedMemoryData(mirfoxData, sharedMemoryUtils);
	
	if (mirfoxData.getPluginState() == MFENUM_PLUGIN_STATE_ERROR){
		return;
	}

	//Create new thread to maintain shared memory data
	mir_forkthread(CMirfoxMiranda::csmThread, this);

}

void CMirfoxMiranda::commitSharedMemory()
{
	//commitSM();
	sharedMemoryUtils->commitSM();
}

void CMirfoxMiranda::initializeSharedMemoryData(MirfoxData& mirfoxData, SharedMemoryUtils* sharedMemoryUtils)
{

	std::wstring visableTo;
	if (mirfoxData.getClientsProfilesFilterCheckbox()){
		visableTo = *mirfoxData.getClientsProfilesFilterStringPtr();
	}else{
		visableTo = TEXT("");
	}
	OpenOrCreateSMReturnStruct result = sharedMemoryUtils->openOrCreateSM('M', mirandaUtils->getDisplayName(), visableTo, false);
	logger->log_p(L"openOrCreateCSM('M')   result = [%u]", result);
	
	if(result.errorCode != 0){
		//error
		if (result.errorCode == -3){  //existing csm version is too high -> i'm too old
			MessageBox(nullptr, TranslateT("This MirFox (Miranda) plugin is too old. Please update it."), TranslateT("MirFox (Miranda) - Error"), MB_OK | MB_ICONWARNING );
		}
		if (result.errorCode == -4){  //existing csm version is too low -> sb is too old
			MessageBox(nullptr, TranslateT("This MirFox (Miranda) plugin cannot start because some other MirFox component is too old. Please, check and update your MirFox components."), TranslateT("MirFox (Miranda) - Error"), MB_OK | MB_ICONWARNING );
		}
		mirfoxData.setPluginState(MFENUM_PLUGIN_STATE_ERROR);
		return;
	} else {
		mirfoxData.processCsmId = result.processCsmId;
		wchar_t prefix[7];
		mir_snwprintf(prefix, L"MNG%d", result.processCsmId);
		logger->set6CharsPrefix(prefix);
	}
	

	sharedMemoryUtils->resetMsmParameters();

	//addOptionToSM();
	//	no options to add yet

	//addTranslationToSM();
	//	no translations to add yet

	//addAccountToSM();
	boost::ptr_list<MirandaAccount>* mirandaAccountsPtr = mirfoxData.getMirandaAccounts();
	boost::ptr_list<MirandaAccount>::iterator mirandaAccountsIter;
	for (mirandaAccountsIter = mirandaAccountsPtr->begin(); mirandaAccountsIter != mirandaAccountsPtr->end(); mirandaAccountsIter++){
		if (mirandaAccountsIter->accountState == MFENUM_MIRANDAACCOUNT_STATE_ON){
			std::wstring tszAccountNameW = mirandaAccountsIter->tszAccountName;
			sharedMemoryUtils->addAccountToSM(mirandaAccountsIter->id, tszAccountNameW);
		}
	}
	
	//addGroupToSM();
	//TODO groups support
	std::wstring groupName = L"ROOT GROUP";
	sharedMemoryUtils->addGroupToSM((uint64_t)1, (uint64_t)NULL, 'R', groupName);

	//addContactToSM();
	boost::ptr_list<MirandaContact>* mirandaContactsPtr = mirfoxData.getMirandaContacts();
	boost::ptr_list<MirandaContact>::iterator mirandaContactsIter;
	for (mirandaContactsIter = mirandaContactsPtr->begin(); mirandaContactsIter != mirandaContactsPtr->end(); mirandaContactsIter++){
		if (mirandaContactsIter->contactState == MFENUM_MIRANDACONTACT_STATE_ON){
			//TODO mirandaAccountHandle support
			sharedMemoryUtils->addContactToSM((uint64_t)mirandaContactsIter->contactHandle, (uint64_t)NULL, (uint64_t)1, mirandaContactsIter->contactNameW);
		}
	}
}



void CMirfoxMiranda::unloadSharedMemory()
{
	sharedMemoryUtils->unloadSharedMemory(mirfoxData.processCsmId);
	//all msm's will be deleted  when miranda process returns
}

void CMirfoxMiranda::csmThread(void* threadArg)
{
	Thread_Push(nullptr);

	CMirfoxMiranda* mirfoxMirandaPtr = (CMirfoxMiranda*)threadArg;
	MirfoxData* mirfoxDataPtr = &(mirfoxMirandaPtr->getMirfoxData());
	SharedMemoryUtils* sharedMemoryUtils = SharedMemoryUtils::getInstance();
	MirandaUtils* mirandaUtils = MirandaUtils::getInstance();
	MFLogger* logger = MFLogger::getInstance();

	mirfoxDataPtr->workerThreadsCount++;

	logger->log_p(L"MirfoxMiranda::csmThread (processCsmId = [%u]) - started", mirfoxDataPtr->processCsmId);

	int i = CSMTHREAD_FIRSTRUN_TIME;		//first run after 10s	(100 * 100ms)
	for (;;)
	{
		if (i <= 0){
			i = CSMTHREAD_NEXTRUN_TIME;		//next run after 10s

			logger->log_p(L"MirfoxMiranda::csmThread (processCsmId = [%u]) - checking", mirfoxDataPtr->processCsmId);

			boost::interprocess::windows_shared_memory* checkedCsm =
					sharedMemoryUtils->getSmById(sharedMemoryUtils->getCsmName(), sharedMemoryUtils->getCsmTotalSize());

			// - check if our csm record is correct
			int result = sharedMemoryUtils->checkCSM(checkedCsm, mirfoxDataPtr->processCsmId, mirandaUtils->getDisplayName());

			if (result == 1){
				//wrong record in CSM - try to recreate own record
				logger->log_p(L"MirfoxMiranda::csmThread (old processCsmId = [%u]) - checkCSM returned 1, try to recreate record in CSM", mirfoxDataPtr->processCsmId);

				mirfoxDataPtr->setPluginState(MFENUM_PLUGIN_STATE_INIT);
				
				uint16_t unloadedMQProcessId = mirfoxDataPtr->processCsmId;
				//refresh msm
				mirfoxMirandaPtr->initializeSharedMemoryData(*mirfoxDataPtr, sharedMemoryUtils);
				if (unloadedMQProcessId != mirfoxDataPtr->processCsmId){
					//refresh miranda message queue if id changed
					mirfoxMirandaPtr->unloadMessageQueue(unloadedMQProcessId);
					mirfoxMirandaPtr->initializeMessageQueue(*mirfoxDataPtr);
				}
				
				if (mirfoxDataPtr->getPluginState() != MFENUM_PLUGIN_STATE_ERROR){
					
					sharedMemoryUtils->commitSM();
					
					mirfoxDataPtr->setPluginState(MFENUM_PLUGIN_STATE_WORK);
					result = 0; //ok
					logger->log_p(L"MirfoxMiranda::csmThread - Success of recreating own record in CSM and own MSMs.  new processCsmId = [%u]", mirfoxDataPtr->processCsmId);
					
				}

			}

			if (result != 0){ //can't recreate own shared memory record
				logger->log_p(L"ERROR. CSM record data is still corrupted. goto plugin error state   result: %i", result);
				mirfoxDataPtr->setPluginState(MFENUM_PLUGIN_STATE_ERROR);
				break; //exit thread
			}

			// - update our timestamp
			sharedMemoryUtils->updateCsmTimestamp(*checkedCsm, mirfoxDataPtr->processCsmId);

			// - delete another records with too old timestamps
			sharedMemoryUtils->checkAnotherCsm(*checkedCsm, mirfoxDataPtr->processCsmId);

			//delete checkedCsm object, clear handle
			delete checkedCsm;

		}

		i--;
		SleepEx(CSMTHREAD_TICK_TIME, TRUE);	//check exit every 0,1s

		//if miranda is exiting - exit this thread
		if (Miranda_IsTerminated() || mirfoxDataPtr->Plugin_Terminated){
			logger->log_p(L"MirfoxMiranda::csmThread break by Plugin_Terminated (=%d) or Miranda_IsTerminated()", mirfoxDataPtr->Plugin_Terminated);
			break;
		}

	}

	mirfoxDataPtr->workerThreadsCount--;
	Thread_Pop();
	return;

}

void CMirfoxMiranda::initializeMessageQueue(MirfoxData& mirfoxData)
{

	MessageQueueUtils* messageQueueUtils = MessageQueueUtils::getInstance();

	//get name of message queue for this client
	std::string mqName = messageQueueUtils->getMqName(mirfoxData.processCsmId);
	//create own mq
	int result = messageQueueUtils->createMessageQueue(mqName);
	if (result > 0){
		mirfoxData.setPluginState(MFENUM_PLUGIN_STATE_ERROR);
		//sm will not be commited
		return;
	}

	//Create new thread to maintain actions from message queue
	mir_forkthread(CMirfoxMiranda::msgQueueThread, this);

}

void CMirfoxMiranda::unloadMessageQueue(uint16_t unloadedMQProcessId)
{

	MessageQueueUtils* messageQueueUtils = MessageQueueUtils::getInstance();
	messageQueueUtils->unloadMessageQueue(unloadedMQProcessId);

}

void CMirfoxMiranda::msgQueueThread(void* threadArg)
{
	Thread_Push(nullptr);

	CMirfoxMiranda* mirfoxMirandaPtr = (CMirfoxMiranda*)threadArg;
	MirfoxData* mirfoxDataPtr = &(mirfoxMirandaPtr->getMirfoxData());
	uint16_t myProcessCsmId = mirfoxDataPtr->processCsmId;
	MFLogger* logger = MFLogger::getInstance();

	mirfoxDataPtr->workerThreadsCount++;

	logger->log_p(L"MirfoxMiranda::msgQueueThread - started for processCsmId = [%u]", myProcessCsmId);

	MessageQueueUtils* messageQueueUtils = MessageQueueUtils::getInstance();
	MirandaUtils* mirandaUtils = MirandaUtils::getInstance();

	char menuItemType;
	char userActionType;
	char userButton;
	uint64_t targetHandle;
	wchar_t* userActionSelection = new wchar_t[MQCONST_MQSM_TEXT_SIZEC + 1];


	//infinite loop for listening to messages from queue
	int i = MQTHREAD_FIRSTRUN_TIME;		//first run after 0s	(0 * 100ms)
	for (;;)
	{
		if (i<=0){
			i = MQTHREAD_NEXTRUN_TIME;		//next run after 0,10s

			if (messageQueueUtils->tryReceiveMessage(menuItemType, userActionType, userButton, targetHandle, userActionSelection, MQCONST_MQSM_TEXT_SIZEC + 1) == true){
				//message received, variables are available

				logger->log_p(L"mqThread: message received   menuItemType = [%c]  userActionType = [%c]  userButton = [%c]  targetHandle = [%I64u]",
						menuItemType, userActionType, userButton, targetHandle);

				if (menuItemType == 'B'){

					//for B - one action thread per one account needed

					//for all enabled accounts;
					boost::ptr_list<MirandaAccount>* mirandaAccountsPtr = mirfoxDataPtr->getMirandaAccounts();
					boost::ptr_list<MirandaAccount>::iterator mirandaAccountsIter;
					for (mirandaAccountsIter = mirandaAccountsPtr->begin(); mirandaAccountsIter != mirandaAccountsPtr->end(); mirandaAccountsIter++){
						if (mirandaAccountsIter->accountState == MFENUM_MIRANDAACCOUNT_STATE_ON){

							ActionThreadArgStruct* actionThreadArgPtr = new(ActionThreadArgStruct);

							actionThreadArgPtr->userActionType = userActionType;
							actionThreadArgPtr->menuItemType = 'A';
							actionThreadArgPtr->userButton = userButton;

							actionThreadArgPtr->targetHandle = NULL;

							std::size_t mnSize = strlen(mirandaAccountsIter->szModuleName) + 1;
							char* accountSzModuleName = new char[mnSize];
							memset(accountSzModuleName, 0, mnSize * sizeof(char));
							strcpy_s(accountSzModuleName, mnSize, mirandaAccountsIter->szModuleName);
							actionThreadArgPtr->accountSzModuleName = accountSzModuleName;

							std::size_t uasSize = mir_wstrlen(userActionSelection) + 1;
							actionThreadArgPtr->userActionSelection = new wchar_t[uasSize];
							memset(actionThreadArgPtr->userActionSelection, 0, uasSize * sizeof(wchar_t));
							wcsncpy_s(actionThreadArgPtr->userActionSelection, uasSize, userActionSelection, uasSize - 1);

							actionThreadArgPtr->instancePtr = mirandaUtils;
							actionThreadArgPtr->mirfoxDataPtr = mirfoxDataPtr;

							mir_forkthread(MirandaUtils::userActionThread, actionThreadArgPtr);

						}
					}

				} else {

					//for A and C - one action thread needed

					ActionThreadArgStruct* actionThreadArgPtr = new(ActionThreadArgStruct);

					actionThreadArgPtr->userActionType = userActionType;
					actionThreadArgPtr->menuItemType = menuItemType;
					actionThreadArgPtr->userButton = userButton;

					actionThreadArgPtr->targetHandle = (HANDLE)targetHandle;
					if (menuItemType == 'A'){ //action on account
						actionThreadArgPtr->accountSzModuleName = mirfoxDataPtr->getAccountSzModuleNameById(targetHandle);
					}

					std::size_t uasSize = mir_wstrlen(userActionSelection) + 1;
					actionThreadArgPtr->userActionSelection = new wchar_t[uasSize];
					memset(actionThreadArgPtr->userActionSelection, 0, uasSize * sizeof(wchar_t));
					wcsncpy_s(actionThreadArgPtr->userActionSelection, uasSize, userActionSelection, uasSize - 1);

					actionThreadArgPtr->instancePtr = mirandaUtils;
					actionThreadArgPtr->mirfoxDataPtr = mirfoxDataPtr;

					mir_forkthread(MirandaUtils::userActionThread, actionThreadArgPtr);

				}
			}
		}

		i--;
		SleepEx(MQTHREAD_TICK_TIME, TRUE);	//check exit every 0,1s
	
		//if miranda is exiting - exit this thread
		if (Miranda_IsTerminated() || mirfoxDataPtr->Plugin_Terminated){
			logger->log_p(L"mqThread break by Plugin_Terminated (=%d) or Miranda_IsTerminated()", mirfoxDataPtr->Plugin_Terminated);
			delete [] userActionSelection;
			break;
		}
		if(messageQueueUtils->unloadedMQProcessId == myProcessCsmId){
			messageQueueUtils->unloadedMQProcessId = -1;
			logger->log_p(L"mqThread [%u]: returning. unloadedMQProcessId == myProcessCsmId", myProcessCsmId);
			delete [] userActionSelection;
			break;
		}

	}

	mirfoxDataPtr->workerThreadsCount--;
	Thread_Pop();
	return;
}

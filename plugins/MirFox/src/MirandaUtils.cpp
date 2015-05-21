#include "common.h"
#include "MirandaUtils.h"


/*static*/ MirandaUtils * MirandaUtils::m_pOnlyOneInstance;

// private constructor
MirandaUtils::MirandaUtils()
			: logger(MFLogger::getInstance())
{

	netlibHandle = NULL;
	InitializeCriticalSection(&ackMapCs);

}

void MirandaUtils::netlibLog(const wchar_t* szText){
	MirandaUtils::getInstance()->netlibLog_int(szText);
}

void MirandaUtils::netlibLog_int(const wchar_t* szText){
	if (netlibHandle) {
		CallService(MS_NETLIB_LOGW, (WPARAM)netlibHandle, (LPARAM)szText);
	}
#ifdef _DEBUG
	OutputDebugString(szText);
#endif //_DEBUG
}

void MirandaUtils::netlibRegister(){
	// Register netlib user for logging function
	NETLIBUSER nlu = { 0 };
	nlu.cbSize = sizeof(nlu);
	nlu.flags = NUF_TCHAR | NUF_NOOPTIONS;
	nlu.szSettingsModule = PLUGIN_DB_ID;
	nlu.ptszDescriptiveName = TranslateT("MirFox log");

	netlibHandle = (HANDLE)CallService(MS_NETLIB_REGISTERUSER, 0, (LPARAM)&nlu);
}

void MirandaUtils::netlibUnregister(){
	Netlib_CloseHandle(netlibHandle);
	netlibHandle = NULL;
}


std::wstring& MirandaUtils::getProfileName()
{
	if (profileName.size()>0){
		//profileName is now inited
		return profileName;
	}
	
	wchar_t mirandaProfileNameW[128] = {0};
	CallService(MS_DB_GETPROFILENAMEW, SIZEOF(mirandaProfileNameW), (WPARAM)mirandaProfileNameW);
	profileName.append(mirandaProfileNameW);

	return profileName;
}


std::wstring& MirandaUtils::getDisplayName()
{
	if (displayName.size()>0){
		//displayName is now inited
		return displayName;
	}

	displayName.append(L"Miranda NG v.");
	char mirandaVersion[128];
	CallService(MS_SYSTEM_GETVERSIONTEXT, (WPARAM)SIZEOF(mirandaVersion), (LPARAM)mirandaVersion);
	displayName.append(_A2T(mirandaVersion));
	displayName.append(L" (");
	displayName.append(getProfileName());
	displayName.append(L")");

	return displayName;
}


void MirandaUtils::userActionThread(void* threadArg)
{
	Thread_Push(0);
	ActionThreadArgStruct* actionThreadArgPtr = (ActionThreadArgStruct*)threadArg;

	if (actionThreadArgPtr->mirfoxDataPtr == NULL){
		MFLogger::getInstance()->log(L"MirandaUtils::userActionThread: ERROR mirfoxDataPtr == NULL");
		return;
	}

	if (actionThreadArgPtr->mirfoxDataPtr->Plugin_Terminated){
		MFLogger::getInstance()->log(L"MirandaUtils::userActionThread: Plugin_Terminated return");
		return;
	}

	actionThreadArgPtr->mirfoxDataPtr->workerThreadsCount++;

	if (actionThreadArgPtr->menuItemType == 'C'){
		actionThreadArgPtr->instancePtr->sendMessageToContact(actionThreadArgPtr);
	} else if (actionThreadArgPtr->menuItemType == 'A'){
		actionThreadArgPtr->instancePtr->setStatusOnAccount(actionThreadArgPtr);
		delete actionThreadArgPtr->accountSzModuleName;
	} else {
		MFLogger::getInstance()->log(TEXT("MirandaUtils::userActionThread: ERROR: unknown actionThreadArgPtr->menuItemType"));
	}

	delete actionThreadArgPtr->userActionSelection;
	actionThreadArgPtr->mirfoxDataPtr->workerThreadsCount--;
	delete actionThreadArgPtr;

	Thread_Pop();
}

void MirandaUtils::sendMessageToContact(ActionThreadArgStruct* args)
{
	logger->log(L"MirandaUtils::sendMessageToContact: start");

	if (args->targetHandle == NULL){
		logger->log(L"MirandaUtils::sendMessageToContact: ERROR targetHandle == NULL");
		return;
	}

	if (args->userButton == 'R'){					//'R'ight mouse button
		this->sendMessage(args, args->mirfoxDataPtr->rightClickSendMode);
	} else if (args->userButton == 'M'){			//'M'iddle mouse button
		this->sendMessage(args, args->mirfoxDataPtr->middleClickSendMode);
	} else {										//'L'eft mouse button
		this->sendMessage(args, args->mirfoxDataPtr->leftClickSendMode);
	}
}

void MirandaUtils::sendMessage(ActionThreadArgStruct* args, MFENUM_SEND_MESSAGE_MODE mode)
{
	logger->log_p(L"MirandaUtils::sendMessage: mode = [%d]  to = [" SCNuPTR L"]  msg = [%s]", mode, args->targetHandle, args->userActionSelection );

	if (mode == MFENUM_SMM_ONLY_SEND || mode == MFENUM_SMM_SEND_AND_SHOW_MW){

		//TODO - metacontacts support - C:\MIRANDA\SOURCES\PLUGINS\popup_trunk\src\popup_wnd2.cpp : 1083
		//	//check for MetaContact and get szProto from subcontact
		//	if(strcmp(targetHandleSzProto, gszMetaProto)==0) {
		//		HANDLE hSubContact = db_mc_getDefault(hContact);
		//		if(!hSubContact) return FALSE;
		//		targetHandleSzProto = (char *) CallService(MS_PROTO_GETCONTACTBASEPROTO, (WPARAM) hSubContact, 0);
		//	}

		char *targetHandleSzProto = GetContactProto((MCONTACT)args->targetHandle); //targetHandleSzProto doesnt need mir_free or delete
		if (targetHandleSzProto == NULL){
			logger->log(L"MirandaUtils::sendMessageToContact: ERROR targetHandleSzProto == NULL");
			return;
		}

		ptrA msgBuffer(mir_utf8encodeW(args->userActionSelection));
		std::size_t bufSize = strlen(msgBuffer) + 1;

		logger->log_p(L"SMTC: bufSize = [%d]", bufSize);
		HANDLE hProcess = sendMessageMiranda((MCONTACT)args->targetHandle, msgBuffer);
		logger->log_p(L"SMTC: hProcess = [" SCNuPTR L"]", hProcess);

		MIRFOXACKDATA* myMfAck = NULL;

		if (hProcess != NULL){
			//if hProcess of sending process is null there will not be any ack

			EnterCriticalSection(&ackMapCs);
			ackMap[hProcess] = (MIRFOXACKDATA*)NULL;
			LeaveCriticalSection(&ackMapCs);

			int counter = 0;
			const int ACK_WAIT_TIME = 250;			//[ms]
			const int MAX_ACK_WAIT_COUNTER = 40;	//40 * 250ms = 10s

			do {
				SleepEx(ACK_WAIT_TIME, TRUE);
				counter++;
				EnterCriticalSection(&ackMapCs);
				myMfAck = ackMap[hProcess];
				LeaveCriticalSection(&ackMapCs);
				if(Miranda_Terminated() || args->mirfoxDataPtr->Plugin_Terminated){
					logger->log_p(L"SMTC: ACK break by Plugin_Terminated (=%d) or Miranda_Terminated()", args->mirfoxDataPtr->Plugin_Terminated);
					break;
				}
			} while (myMfAck == NULL && counter <= MAX_ACK_WAIT_COUNTER); //TODO or Plugin_Terminated or Miranda_Terminated()

			logger->log_p(L"SMTC: ACK found  counter = [%d]   myMfAck = [" SCNuPTR L"]", counter, myMfAck);
		}

		MirandaContact* mirandaContact = args->mirfoxDataPtr->getMirandaContactPtrByHandle((MCONTACT)args->targetHandle);
		const wchar_t* contactNameW = NULL;
		TCHAR* tszAccountName = NULL;
		if (mirandaContact){
			contactNameW = mirandaContact->contactNameW.c_str();
			MirandaAccount* mirandaAccount = mirandaContact->mirandaAccountPtr;
			if (mirandaAccount)
				tszAccountName = mirandaAccount->tszAccountName;
		}
		if(myMfAck != NULL && myMfAck->result == ACKRESULT_SUCCESS){
			addMessageToDB((MCONTACT)args->targetHandle, msgBuffer, bufSize, targetHandleSzProto);
			if (mode == MFENUM_SMM_ONLY_SEND){
				//show notyfication popup (only in SMM_ONLY_SEND mode)
				wchar_t* buffer = new wchar_t[1024 * sizeof(wchar_t)];
				if (contactNameW != NULL && tszAccountName != NULL)
					mir_sntprintf(buffer, 1024, TranslateT("Message sent to %s (%s)"), contactNameW, tszAccountName);
				else
					mir_sntprintf(buffer, 1024, TranslateT("Message sent"));

				if(ServiceExists(MS_POPUP_ADDPOPUPCLASS))
					ShowClassPopupT("MirFox_Notify", _T("MirFox"), buffer);
				else
					PUShowMessageT(buffer, SM_NOTIFY);

				delete[] buffer;
			}
			else if (mode == MFENUM_SMM_SEND_AND_SHOW_MW){
				//notify hook to open window
				if (args->mirfoxDataPtr != NULL && args->mirfoxDataPtr->hhook_EventOpenMW != NULL){
					OnHookOpenMvStruct* onHookOpenMv = new(OnHookOpenMvStruct);
					onHookOpenMv->targetHandle = args->targetHandle;
					onHookOpenMv->msgBuffer = NULL;
					NotifyEventHooks(args->mirfoxDataPtr->hhook_EventOpenMW, (WPARAM)onHookOpenMv, 0);
				}
				else logger->log(L"SMTC: ERROR1 args->mirfoxDataPtr == NULL || args->mirfoxDataPtr->hhook_EventOpenMW == NULL");
			}
		}
		else {
			//error - show error popup
			wchar_t* buffer = new wchar_t[1024 * sizeof(wchar_t)];
			if (myMfAck != NULL){
				logger->log_p(L"SMTC: ERROR - Cannot send message - result = [%d] ", myMfAck->result);
				if (myMfAck->errorDesc != NULL){
					if (contactNameW != NULL && tszAccountName != NULL){
						mir_sntprintf(buffer, 1024, TranslateT("Cannot send message to %s (%s) - %S"), contactNameW, tszAccountName, myMfAck->errorDesc);
					} else {
						mir_sntprintf(buffer, 1024, TranslateT("Cannot send message - %S"), myMfAck->errorDesc);
					}
				} else {
					if (contactNameW != NULL && tszAccountName != NULL){
						mir_sntprintf(buffer, 1024, TranslateT("Cannot send message to %s (%s)"), contactNameW, tszAccountName);
					} else {
						mir_sntprintf(buffer, 1024, TranslateT("Cannot send message"));
					}
				}
			} else {
				logger->log(L"SMTC: ERROR - Cannot send message 2");
				if (contactNameW != NULL && tszAccountName != NULL){
					mir_sntprintf(buffer, 1024, TranslateT("Cannot send message to %s (%s)"), contactNameW, tszAccountName);
				} else {
					mir_sntprintf(buffer, 1024, TranslateT("Cannot send message"));
				}
			}

			if(ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
				ShowClassPopupT("MirFox_Error", TranslateT("MirFox error"), buffer);
			} else {
				PUShowMessageT(buffer, SM_WARNING);
			}

			delete[] buffer;
		}

		if (myMfAck != NULL){ //when we found ack, not when we exceed MAX_ACK_WAIT_COUNTER
			if (myMfAck->errorDesc != NULL) delete myMfAck->errorDesc;
			delete myMfAck->szModule;
			delete myMfAck;
		}
		EnterCriticalSection(&ackMapCs);
		ackMap.erase(hProcess);
		LeaveCriticalSection(&ackMapCs);
	}
	else if (mode == MFENUM_SMM_ONLY_SHOW_MW) {
		//notify hook to open window
		if (args->mirfoxDataPtr != NULL && args->mirfoxDataPtr->hhook_EventOpenMW != NULL){

			OnHookOpenMvStruct* onHookOpenMv = new(OnHookOpenMvStruct);
			onHookOpenMv->targetHandle = args->targetHandle;
			//adding newline to message in Message Window, only in this mode
			std::wstring* msgBuffer = new std::wstring(); //deleted at on_hook_OpenMW
			msgBuffer->append(args->userActionSelection);
			msgBuffer->append(L"\r\n");
			onHookOpenMv->msgBuffer = msgBuffer;
			NotifyEventHooks(args->mirfoxDataPtr->hhook_EventOpenMW, (WPARAM)onHookOpenMv, 0);
		}
		else logger->log(L"SMTC: ERROR1 args->mirfoxDataPtr == NULL || args->mirfoxDataPtr->hhook_EventOpenMW == NULL");
	}
}

HANDLE MirandaUtils::sendMessageMiranda(MCONTACT hContact, char *msgBuffer)
{
	return (HANDLE)CallContactService(hContact, PSS_MESSAGE, 0, (LPARAM)msgBuffer);
}

void MirandaUtils::addMessageToDB(MCONTACT hContact, char* msgBuffer, std::size_t bufSize, char* targetHandleSzProto)
{
	DBEVENTINFO dbei = {0};
	dbei.cbSize = sizeof(dbei);
	dbei.eventType = EVENTTYPE_MESSAGE;
	dbei.flags = DBEF_SENT | DBEF_UTF;
	dbei.szModule = targetHandleSzProto;
	dbei.timestamp = (DWORD)time(NULL);
	dbei.cbBlob = (DWORD)bufSize;
	dbei.pBlob = (PBYTE)msgBuffer;
	db_event_add(hContact, &dbei);
}


//http://www.shloemi.com/2012/09/solved-setforegroundwindow-win32-api-not-always-works/
void MirandaUtils::ForceForegroundWindow(HWND hWnd)
{
	DWORD foreThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
	DWORD appThread = GetCurrentThreadId();

	if (foreThread != appThread) {
		AttachThreadInput(foreThread, appThread, true);
		BringWindowToTop(hWnd);
		ShowWindow(hWnd, SW_SHOW);
		AttachThreadInput(foreThread, appThread, false);
	} else {
		BringWindowToTop(hWnd);
		ShowWindow(hWnd, SW_SHOW);
	}
}

int MirandaUtils::on_hook_OpenMW(WPARAM wParam, LPARAM lParam)
{
	OnHookOpenMvStruct* param = (OnHookOpenMvStruct*)wParam;

	if (param->msgBuffer != NULL){
		TCHAR *msgBuffer = mir_tstrdup(param->msgBuffer->c_str());
		CallServiceSync(MS_MSG_SENDMESSAGET, (WPARAM)param->targetHandle, (LPARAM)msgBuffer);
		mir_free(msgBuffer);

		delete param->msgBuffer;
	} else {
		//only open window
		CallServiceSync(MS_MSG_SENDMESSAGET, (WPARAM)param->targetHandle, 0);
	}

	// show and focus window
	if (db_get_b(0, PLUGIN_DB_ID, "doNotFocusWhenOpenMW", 0) == 1){
		delete param;
		return 0;
	}

	MessageWindowData mwd;
	mwd.cbSize = sizeof(MessageWindowData);
	mwd.hContact = (MCONTACT)param->targetHandle;
	mwd.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	MessageWindowInputData mwid;
	mwid.cbSize = sizeof(MessageWindowInputData);
	mwid.hContact = (MCONTACT)param->targetHandle;
	mwid.uFlags = MSG_WINDOW_UFLAG_MSG_BOTH;

	delete param;

	if (!CallService(MS_MSG_GETWINDOWDATA, (WPARAM)&mwid, (LPARAM)&mwd) && mwd.hwndWindow){
		HWND parent;
		HWND hWnd = mwd.hwndWindow;
		while((parent = GetParent(hWnd)) != 0) hWnd = parent; // ensure we have the top level window (need parent window for scriver & tabsrmm)
		ForceForegroundWindow(hWnd);
	}

	return 0;
}

void MirandaUtils::setStatusOnAccount(ActionThreadArgStruct* args)
{
	logger->log(L"MirandaUtils::setStatusOnAccount: start");
	int status = CallProtoService(args->accountSzModuleName, PS_GETSTATUS, 0, 0);
	logger->log_p(L"SSOA: on account: [%S]  targetHandle = [" SCNuPTR L"]   at status = [%d]", args->accountSzModuleName, args->targetHandle, status);

	INT_PTR result = -1;

	if (!(CallProtoService(args->accountSzModuleName, PS_GETCAPS, PFLAGNUM_1, 0) & PF1_INDIVMODEMSG)){
		result = CallProtoService(args->accountSzModuleName, PS_SETAWAYMSGW, (WPARAM)status, (LPARAM)args->userActionSelection);
		if (result == CALLSERVICE_NOTFOUND){
			char *szMsg = mir_u2a(args->userActionSelection);
			result = CallProtoService(args->accountSzModuleName, PS_SETAWAYMSG, (WPARAM)status, (LPARAM)szMsg);
			mir_free(szMsg);
		}
	}

	MirandaAccount* mirandaAccount = args->mirfoxDataPtr->getMirandaAccountPtrBySzModuleName(args->accountSzModuleName);
	TCHAR* tszAccountName = NULL;
	if (mirandaAccount){
		tszAccountName = mirandaAccount->tszAccountName;
	}

	wchar_t* buffer = new wchar_t[1024 * sizeof(wchar_t)];
	if(result == 0){
		if (tszAccountName != NULL){
			logger->log_p(L"SSOA: Status message set on [%s]", tszAccountName);
			mir_sntprintf(buffer, 1024, TranslateT("Status message set on %s"), tszAccountName);
		} else {
			logger->log(L"SSOA: Status message set");
			mir_sntprintf(buffer, 1024, TranslateT("Status message set"));
		}

		if(ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
			ShowClassPopupT("MirFox_Notify", _T("MirFox"), buffer);
		} else {
			PUShowMessageT(buffer, SM_NOTIFY);
		}
	} else {
		if (tszAccountName != NULL){
			logger->log_p(L"SSOA: ERROR - Cannot set status message 2 on [%s] - result = [%d] ", tszAccountName, result);
			mir_sntprintf(buffer, 1024, TranslateT("Cannot set status message on %s"), tszAccountName);
		} else {
			logger->log_p(L"SSOA: ERROR - Cannot set status message 2 - result = [%d] ", result);
			mir_sntprintf(buffer, 1024, TranslateT("Cannot set status message"));
		}

		if(ServiceExists(MS_POPUP_ADDPOPUPCLASS)) {
			ShowClassPopupT("MirFox_Error", TranslateT("MirFox error"), buffer);
		} else {
			PUShowMessageT(buffer, SM_WARNING);
		}
	}
	delete[] buffer;
}

int MirandaUtils::onProtoAck(WPARAM, LPARAM lParam)
{
	MirandaUtils* mirandaUtils = MirandaUtils::getInstance();
	mirandaUtils->onProtoAckOnInstance((ACKDATA*)lParam);
	return 0;
}

void MirandaUtils::onProtoAckOnInstance(ACKDATA* ack)
{
	if (ack == NULL || ack->type != ACKTYPE_MESSAGE){
		//we are waiting for ACKTYPE_MESSAGE ack's
		return;
	}

	EnterCriticalSection(&ackMapCs);
	ackMapIt = ackMap.find(ack->hProcess);
	if (ackMapIt != ackMap.end()){
		//we waited for this ack, save copy (only needed data) to our map. Oryginal ack object is unstable, it is probably controled not in our thread
		logger->log_p(L"!!! ACK received  acl: hContact = [" SCNuPTR L"] result = [%d] szModule = [%S] lParam = [%S]", ack->hProcess, ack->result, ack->szModule, (ack->lParam != NULL && *((char*)ack->lParam) != '\0') ? (char*)ack->lParam : "null");
		MIRFOXACKDATA* myMfAck = new(MIRFOXACKDATA);
		myMfAck->result = ack->result;
		myMfAck->hContact = ack->hContact;
		size_t len1 = strlen(ack->szModule) + 1;
		char* myMfSzModulePtr = new char[len1];
		strcpy_s(myMfSzModulePtr, len1, ack->szModule);
		myMfAck->szModule = myMfSzModulePtr;
		if (ack->lParam != NULL && *((char*)ack->lParam) != '\0'){
			size_t len2 = strlen((char*)ack->lParam) + 1;
			char* myMfSzLparamPtr = new char[len2];
			strcpy_s(myMfSzLparamPtr, len2, (char*)ack->lParam);
			myMfAck->errorDesc = myMfSzLparamPtr;
		} else {
			myMfAck->errorDesc = NULL;
		}

		ackMap[ack->hProcess] = myMfAck;
	}
	LeaveCriticalSection(&ackMapCs);
}

#define OLD_PLUGIN_DB_ID				"MirfoxMiranda"

/**
 * function changes db module name from "MirfoxMiranda" (used before 0.3.0.0) to "Mirfox"
 */
void MirandaUtils::translateOldDBNames() {
	//settings			"clientsProfilesFilterCheckbox", "clientsProfilesFilterString"
	int opt1KeyValue = db_get_b(0, OLD_PLUGIN_DB_ID, "clientsProfilesFilterCheckbox", 0);
	if (opt1KeyValue != 0){
		db_set_b(0, PLUGIN_DB_ID, "clientsProfilesFilterCheckbox", opt1KeyValue);
		db_unset(0, OLD_PLUGIN_DB_ID, "clientsProfilesFilterCheckbox");
		logger->log(L"TranslateOldDBNames:  'clientsProfilesFilterCheckbox' db entry found and moved");
	} else {
		logger->log(L"TranslateOldDBNames:  no old settings found. returning.");
		return;
	}

	DBVARIANT opt2Dbv = {0};
	INT_PTR opt2Result = db_get_s(0, OLD_PLUGIN_DB_ID, "clientsProfilesFilterString", &opt2Dbv, DBVT_TCHAR);
	if (opt2Result == 0){	//success
		std::wstring clientsProfilesFilterString = opt2Dbv.pwszVal;
		db_set_ts(0, PLUGIN_DB_ID, "clientsProfilesFilterString", clientsProfilesFilterString.c_str());
		db_unset(0, OLD_PLUGIN_DB_ID, "clientsProfilesFilterString");
		logger->log(L"TranslateOldDBNames:  'clientsProfilesFilterString' db entry found and moved");
		db_free(&opt2Dbv);
	}

	//account's settings		"ACCOUNTSTATE_"
	int accountsTmpCount = 0;
	PROTOACCOUNT **accountsTmp;
	CallService(MS_PROTO_ENUMACCOUNTS, (WPARAM)&accountsTmpCount, (LPARAM)&accountsTmp);
	for(int i=0; i<accountsTmpCount; i++) {

		logger->log_p(L"TranslateOldDBNames: found ACCOUNT: [%s]  protocol: [%S]", accountsTmp[i]->tszAccountName, accountsTmp[i]->szProtoName);

		std::string mirandaAccountDBKey("ACCOUNTSTATE_");
		mirandaAccountDBKey += accountsTmp[i]->szModuleName;
		int keyValue = db_get_b(0, OLD_PLUGIN_DB_ID, mirandaAccountDBKey.c_str(), 0);
		if (keyValue != 0){
			db_set_b(0, PLUGIN_DB_ID, mirandaAccountDBKey.c_str(), keyValue);
			db_unset(0, OLD_PLUGIN_DB_ID, mirandaAccountDBKey.c_str());
			logger->log(L"TranslateOldDBNames:  ACCOUNT db entry found and moved");
		}
	}

	//contacts		"state"
	for (MCONTACT hContact = db_find_first(); hContact; hContact = db_find_next(hContact)){
		logger->log_p(L"TranslateOldDBNames: found CONTACT: [" SCNuPTR L"]", hContact);

		int keyValue = db_get_b(hContact, OLD_PLUGIN_DB_ID, "state", 0);
		if (keyValue != 0){
			db_set_b(hContact, PLUGIN_DB_ID, "state", keyValue);
			db_unset(hContact, OLD_PLUGIN_DB_ID, "state");
			logger->log(L"TranslateOldDBNames:  CONTACT db entry found and moved");
		}
	}

	//delete db module
	CallService(MS_DB_MODULE_DELETE, 0, (LPARAM)OLD_PLUGIN_DB_ID);
}


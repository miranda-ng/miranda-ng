
#include "MirFoxCommons_pch.h"
#include "MirFoxCommons_messageQueueBySM.h"


/////////////////  MQ CONSTS   /////////////////

const char * 							MQCONST_MQ_NAME_PREFIX					= "mirfox_mq_";   //mirfox_mq_<##id>

const boost::interprocess::offset_t		MQCONST_MQMIRSM_STATUS_OFFSET			= 0;
const std::size_t 						MQCONST_MQMIRSM_STATUS_SIZE				= sizeof(char); //1B		//'E'-Empty 'M'-Message exists
const boost::interprocess::offset_t		MQCONST_MQMIRSM_MENUITEMTYPE_OFFSET		= MQCONST_MQMIRSM_STATUS_OFFSET + MQCONST_MQMIRSM_STATUS_SIZE;
const std::size_t 						MQCONST_MQMIRSM_MENUITEMTYPE_SIZE		= sizeof(char); //1B
const boost::interprocess::offset_t		MQCONST_MQMIRSM_USERACTIONTYPE_OFFSET	= MQCONST_MQMIRSM_MENUITEMTYPE_OFFSET + MQCONST_MQMIRSM_MENUITEMTYPE_SIZE;
const std::size_t 						MQCONST_MQMIRSM_USERACTIONTYPE_SIZE		= sizeof(char); //1B
const boost::interprocess::offset_t		MQCONST_MQMIRSM_USERBUTTON_OFFSET		= MQCONST_MQMIRSM_USERACTIONTYPE_OFFSET + MQCONST_MQMIRSM_USERACTIONTYPE_SIZE;
const std::size_t 						MQCONST_MQMIRSM_USERBUTTON_SIZE			= sizeof(char); //1B
const boost::interprocess::offset_t		MQCONST_MQMIRSM_TARGETHANDLE_OFFSET		= MQCONST_MQMIRSM_USERBUTTON_OFFSET + MQCONST_MQMIRSM_USERBUTTON_SIZE;
const std::size_t 						MQCONST_MQMIRSM_TARGETHANDLE_SIZE		= sizeof(uint64_t); //8B
const boost::interprocess::offset_t		MQCONST_MQMIRSM_TEXTARRAY_OFFSET		= MQCONST_MQMIRSM_TARGETHANDLE_OFFSET + MQCONST_MQMIRSM_TARGETHANDLE_SIZE;
const std::size_t 						MQCONST_MQMIRSM_TEXTARRAY_SIZEC			= MQCONST_MQSM_TEXT_SIZEC; //2042
const std::size_t 						MQCONST_MQMIRSM_TEXTARRAY_SIZE			= sizeof(wchar_t) * MQCONST_MQMIRSM_TEXTARRAY_SIZEC; //4084B
const std::size_t 						MQCONST_MQMIRSM_TOTAL_SIZE 				= MQCONST_MQMIRSM_STATUS_SIZE + MQCONST_MQMIRSM_MENUITEMTYPE_SIZE + MQCONST_MQMIRSM_USERACTIONTYPE_SIZE + MQCONST_MQMIRSM_USERBUTTON_SIZE + MQCONST_MQMIRSM_TARGETHANDLE_SIZE + MQCONST_MQMIRSM_TEXTARRAY_SIZE;




/////////////////////PUBLIC///////////////////////////


//static fields definitions
/*static*/ MessageQueueUtils * MessageQueueUtils::m_pOnlyOneInstance;


//constructor
MessageQueueUtils::MessageQueueUtils()
				: logger(MFLogger::getInstance())
{
	unloadedMQProcessId = -1;
	//one mutex per all messagequeues schould be enough
	smMutex = CreateMutex(NULL, FALSE, L"Local\\mirfoxMqMirSm");
}


MessageQueueUtils::~MessageQueueUtils()
{
	CloseHandle(smMutex);
}


std::string
MessageQueueUtils::getMqName(uint16_t processId)
{

	std::stringstream mqName;
	mqName << MQCONST_MQ_NAME_PREFIX;

	if (processId <= 9){
		mqName << "0";
	}
	mqName << processId;

	logger->log_p(L"messageQueueBySM::getMqName = [%S]", mqName.str().c_str());

	return mqName.str();
}


int
MessageQueueUtils::createMessageQueue(std::string mqName)
{

	try{

		//TODO access to mqMirSm is not synchronied, i think it's not neccessery
		mqMirSm = new boost::interprocess::windows_shared_memory(
				boost::interprocess::open_or_create,	//TODO what if sm exists
				mqName.c_str(),
				boost::interprocess::read_write,
				getMqMirSmTotalSize()
			);

		logger->log(L"MessageQueueUtils::createMessageQueue  created mq by sm");

	} catch (const boost::interprocess::interprocess_exception& ex) {
		logger->log_p(L"MessageQueueUtils::createMessageQueue interprocess_exception: [%S] native_error=[%d] error_code=[%d]", ex.what(), ex.get_native_error(), ex.get_error_code() );
		return 1; //error
	} catch (...){
		logger->log(L"MessageQueueUtils::createMessageQueue EXCEPTION: unknown");
		return 1; //error
	}

	//initialize
	boost::interprocess::mapped_region region1(
			*mqMirSm,
			boost::interprocess::read_write,
			MQCONST_MQMIRSM_STATUS_OFFSET,
			MQCONST_MQMIRSM_STATUS_SIZE
		);
	char* statusPtr = static_cast<char*>(region1.get_address());
	*statusPtr = 'E';


	return 0; //ok

}


void
MessageQueueUtils::unloadMessageQueue(uint16_t unloadedMQProcessId)
{

	logger->log_p(L"MessageQueueUtils::unloadMessageQueue unloadedMQProcessId = [%u]", unloadedMQProcessId);
	this->unloadedMQProcessId = unloadedMQProcessId;
	if (mqMirSm){
		delete mqMirSm;
		mqMirSm = NULL;
	} else {
		logger->log(L"MessageQueueUtils::unloadMessageQueue - mqMirSm was NULL");
	}

}


/**
 * return true if message was received and variables are available
 */
bool
MessageQueueUtils::tryReceiveMessage(char& menuItemType, char& userActionType, char& userButton, uint64_t& targetHandle, wchar_t*& userActionSelection, size_t uasBuffCSize)
{

	try{

		WaitForSingleObject(smMutex, 100000);


		boost::interprocess::mapped_region region1(
				*mqMirSm,
				boost::interprocess::read_write,
				MQCONST_MQMIRSM_STATUS_OFFSET,
				MQCONST_MQMIRSM_STATUS_SIZE
			);
		char* mqmirsmStatusPtr = static_cast<char*>(region1.get_address());

		if (*mqmirsmStatusPtr != 'M'){
			ReleaseMutex(smMutex);
			return false;
		}


		//there is a new message in the queue (sm mode) *mqmirsmStatusPtr=='M'
		//transfer data from shared memory to variables

		boost::interprocess::mapped_region region2(
				*mqMirSm,
				boost::interprocess::read_only,
				MQCONST_MQMIRSM_MENUITEMTYPE_OFFSET,
				MQCONST_MQMIRSM_MENUITEMTYPE_SIZE
			);
		menuItemType = *(static_cast<char*>(region2.get_address()));

		boost::interprocess::mapped_region region3(
				*mqMirSm,
				boost::interprocess::read_only,
				MQCONST_MQMIRSM_USERACTIONTYPE_OFFSET,
				MQCONST_MQMIRSM_USERACTIONTYPE_SIZE
			);
		userActionType = *(static_cast<char*>(region3.get_address()));

		boost::interprocess::mapped_region region4(
				*mqMirSm,
				boost::interprocess::read_only,
				MQCONST_MQMIRSM_USERBUTTON_OFFSET,
				MQCONST_MQMIRSM_USERBUTTON_SIZE
			);
		userButton = *(static_cast<char*>(region4.get_address()));

		boost::interprocess::mapped_region region5(
				*mqMirSm,
				boost::interprocess::read_only,
				MQCONST_MQMIRSM_TARGETHANDLE_OFFSET,
				MQCONST_MQMIRSM_TARGETHANDLE_SIZE
			);
		targetHandle = *(static_cast<uint64_t*>(region5.get_address()));

		boost::interprocess::mapped_region region6(
				*mqMirSm,
				boost::interprocess::read_only,
				MQCONST_MQMIRSM_TEXTARRAY_OFFSET,
				MQCONST_MQMIRSM_TEXTARRAY_SIZE
			);
		wchar_t* userActionSelectionPtr = static_cast<wchar_t*>(region6.get_address());
		wcsncpy_s(userActionSelection, uasBuffCSize, userActionSelectionPtr, MQCONST_MQMIRSM_TEXTARRAY_SIZEC);


		//update shared memory status to Empty
		*mqmirsmStatusPtr = 'E';
		ReleaseMutex(smMutex);

	} catch (const boost::interprocess::interprocess_exception& ex) {
		logger->log_p(L"MessageQueueUtils::tryReceiveMessage interprocess_exception: [%S] native_error=[%d] error_code=[%d]", ex.what(), ex.get_native_error(), ex.get_error_code() );
		return false;
	} catch (...){
		logger->log(L"MessageQueueUtils::tryReceiveMessage EXCEPTION: unknown");
		return false;
	}

	logger->log_p(L"MessageQueueUtils::tryReceiveMessage (mq mode)  RECEIVED DATA:  menuItemType = [%c] userActionType = [%c] userButton = [%c] targetHandle = [%I64u]",
			menuItemType, userActionType, userButton, targetHandle);

	return true;
}


void
MessageQueueUtils::sendMessage(int clientRecordId, char menuItemType, char userActionType, char userButton, uint64_t targetHandle, std::wstring userActionSelection)
{

	logger->log(L"MessageQueueUtils::sendMessage");

	//if text to send is too long
	if (userActionSelection.size() > MQCONST_MQSM_TEXT_SIZEC){
		userActionSelection.resize(MQCONST_MQSM_TEXT_SIZEC - 3);
		userActionSelection.append(TEXT("..."));
	}

	try{

		boost::interprocess::windows_shared_memory* sm = new boost::interprocess::windows_shared_memory(
				boost::interprocess::open_only,
				getMqName(clientRecordId).c_str(),
				boost::interprocess::read_write);

		if (sm == NULL){
			logger->log(L"MessageQueueUtils::sendMessage (sm mode) sm == NULL");
			return; //error
		}


		int counter = 0;
		const int MQ_WAIT_TIME = 200;			//[ms]
		const int MAX_MQ_WAIT_COUNTER = 10;		//10 * 200ms = 2s
		bool sentSuccess = false;

		do {

			counter++;

			WaitForSingleObject(smMutex, INFINITE);


			boost::interprocess::mapped_region region1(
					*sm,
					boost::interprocess::read_write,
					MQCONST_MQMIRSM_STATUS_OFFSET,
					MQCONST_MQMIRSM_STATUS_SIZE
				);
			char* mqmirsmStatusPtr = static_cast<char*>(region1.get_address());

			if (*mqmirsmStatusPtr == 'E'){

				boost::interprocess::mapped_region region2(
						*sm,
						boost::interprocess::read_write,
						MQCONST_MQMIRSM_MENUITEMTYPE_OFFSET,
						MQCONST_MQMIRSM_MENUITEMTYPE_SIZE
					);
				char* menuItemTypePtr = static_cast<char*>(region2.get_address());
				*menuItemTypePtr = menuItemType;


				boost::interprocess::mapped_region region3(
						*sm,
						boost::interprocess::read_write,
						MQCONST_MQMIRSM_USERACTIONTYPE_OFFSET,
						MQCONST_MQMIRSM_USERACTIONTYPE_SIZE
					);
				char* userActionTypePtr = static_cast<char*>(region3.get_address());
				*userActionTypePtr = userActionType;


				boost::interprocess::mapped_region region4(
						*sm,
						boost::interprocess::read_write,
						MQCONST_MQMIRSM_USERBUTTON_OFFSET,
						MQCONST_MQMIRSM_USERBUTTON_SIZE
					);
				char* userButtonPtr = static_cast<char*>(region4.get_address());
				*userButtonPtr = userButton;


				boost::interprocess::mapped_region region5(
						*sm,
						boost::interprocess::read_write,
						MQCONST_MQMIRSM_TARGETHANDLE_OFFSET,
						MQCONST_MQMIRSM_TARGETHANDLE_SIZE
					);
				uint64_t* targetHandlePtr = static_cast<uint64_t*>(region5.get_address());
				*targetHandlePtr = targetHandle;


				boost::interprocess::mapped_region region6(
						*sm,
						boost::interprocess::read_write,
						MQCONST_MQMIRSM_TEXTARRAY_OFFSET,
						MQCONST_MQMIRSM_TEXTARRAY_SIZE
					);
				wchar_t* userActionSelectionPtr = static_cast<wchar_t*>(region6.get_address());
				memset(userActionSelectionPtr, 0, MQCONST_MQMIRSM_TEXTARRAY_SIZE);
				memcpy(userActionSelectionPtr, userActionSelection.c_str(), userActionSelection.length() * sizeof(wchar_t));

				//update shared memory status to Message
				*mqmirsmStatusPtr = 'M';

				sentSuccess = true;

			}
			// else wait and repeat try

			ReleaseMutex(smMutex);

			if (!sentSuccess){
				SleepEx(MQ_WAIT_TIME, TRUE);
			}

		} while (sentSuccess == false && counter <= MAX_MQ_WAIT_COUNTER);

		if (!sentSuccess){
			logger->log(L"MessageQueueUtils::sendMessage (sm mode) - MAX_MQ_WAIT_COUNTER exceeded but still sentSuccess == false");
		}

		delete sm;

	} catch (const boost::interprocess::interprocess_exception& ex) {
		logger->log_p(L"MessageQueueUtils::sendMessage (sm mode) interprocess_exception: [%S] native_error=[%d] error_code=[%d]", ex.what(), ex.get_native_error(), ex.get_error_code() );
		return; //error
	} catch (...){
		logger->log(L"MessageQueueUtils::sendMessage (sm mode) EXCEPTION: unknown");
		return; //error
	}

}



//return total size of mqMirSm in Bytes
std::size_t
MessageQueueUtils::getMqMirSmTotalSize()
{

	logger->log_p(L"MQCONST_MQMIRSM_TOTAL_SIZE (schould be 4096B) = [" SCNuPTR L"]", MQCONST_MQMIRSM_TOTAL_SIZE);
	return MQCONST_MQMIRSM_TOTAL_SIZE;

}




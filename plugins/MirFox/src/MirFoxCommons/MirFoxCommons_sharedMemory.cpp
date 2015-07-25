
#include "MirFoxCommons_pch.h"
#include "MirFoxCommons_sharedMemory.h"



/////////////////  CSM CONSTS   /////////////////
const char * 							SMUCONST_CSM_NAME 						= "mirfox_csm";

const uint32_t							SMUCONST_CSM_CURRENT_VERSION			= 4; //1:100929 2:110530 3:110704 4:v0.5(130523)
const uint16_t							SMUCONST_CSM_CURRENT_RECORDVERSION		= 1; //100929

const boost::interprocess::offset_t		SMUCONST_CSM_HEADER_OFFSET				= 0;
const boost::interprocess::offset_t		SMUCONST_CSM_HEADER_VERSION_OFFSET		= SMUCONST_CSM_HEADER_OFFSET;
const std::size_t 						SMUCONST_CSM_HEADER_VERSION_SIZE 		= sizeof(uint32_t); //4B
const boost::interprocess::offset_t		SMUCONST_CSM_HEADER_RESERVED_OFFSET		= SMUCONST_CSM_HEADER_VERSION_OFFSET + SMUCONST_CSM_HEADER_VERSION_SIZE;
const std::size_t 						SMUCONST_CSM_HEADER_RESERVED_SIZE 		= 204; //B
const std::size_t 						SMUCONST_CSM_HEADER_SIZE 				= SMUCONST_CSM_HEADER_VERSION_SIZE + SMUCONST_CSM_HEADER_RESERVED_SIZE;

const int								SMUCONST_CSM_RECORDS_COUNT				= 8;
const boost::interprocess::offset_t		SMUCONST_CSM_RECORDS_OFFSET				= SMUCONST_CSM_HEADER_SIZE;

const boost::interprocess::offset_t		SMUCONST_CSM_RECORD_RECID_RECOFFSET		= 0;
const std::size_t 						SMUCONST_CSM_RECORD_RECID_SIZE			= sizeof(uint16_t); //2B
const boost::interprocess::offset_t		SMUCONST_CSM_RECORD_TYPE_RECOFFSET		= SMUCONST_CSM_RECORD_RECID_RECOFFSET + SMUCONST_CSM_RECORD_RECID_SIZE;
const std::size_t 						SMUCONST_CSM_RECORD_TYPE_SIZE			= sizeof(char); //1B
const boost::interprocess::offset_t		SMUCONST_CSM_RECORD_VERSION_RECOFFSET	= SMUCONST_CSM_RECORD_TYPE_RECOFFSET + SMUCONST_CSM_RECORD_TYPE_SIZE;
const std::size_t 						SMUCONST_CSM_RECORD_VERSION_SIZE		= sizeof(uint16_t); //2B
const boost::interprocess::offset_t		SMUCONST_CSM_RECORD_STATUS_RECOFFSET	= SMUCONST_CSM_RECORD_VERSION_RECOFFSET + SMUCONST_CSM_RECORD_VERSION_SIZE;
const std::size_t 						SMUCONST_CSM_RECORD_STATUS_SIZE			= sizeof(char); //1B
const boost::interprocess::offset_t		SMUCONST_CSM_RECORD_KAT_RECOFFSET		= SMUCONST_CSM_RECORD_STATUS_RECOFFSET + SMUCONST_CSM_RECORD_STATUS_SIZE;	//Keep Alive Timestamp
const std::size_t 						SMUCONST_CSM_RECORD_KAT_SIZE			= sizeof(time_t); //8B (time_t is 8B at MSVS)
const boost::interprocess::offset_t		SMUCONST_CSM_RECORD_MSMT_RECOFFSET		= SMUCONST_CSM_RECORD_KAT_RECOFFSET + SMUCONST_CSM_RECORD_KAT_SIZE;
const std::size_t 						SMUCONST_CSM_RECORD_MSMT_SIZE			= sizeof(time_t); //8B (time_t is 8B at MSVS)
const boost::interprocess::offset_t		SMUCONST_CSM_RECORD_DNAME_RECOFFSET		= SMUCONST_CSM_RECORD_MSMT_RECOFFSET + SMUCONST_CSM_RECORD_MSMT_SIZE;		//Display Name
const std::size_t 						SMUCONST_CSM_RECORD_DNAME_SIZEC			= SMUCONST_CSM_RECORD_DNAME_SIZEC_DEF;
const std::size_t 						SMUCONST_CSM_RECORD_DNAME_SIZE			= sizeof(wchar_t) * SMUCONST_CSM_RECORD_DNAME_SIZEC; //156B
const boost::interprocess::offset_t		SMUCONST_CSM_RECORD_VISABLETO_RECOFFSET	= SMUCONST_CSM_RECORD_DNAME_RECOFFSET + SMUCONST_CSM_RECORD_DNAME_SIZE;
const std::size_t 						SMUCONST_CSM_RECORD_VISABLETO_SIZEC		= SMUCONST_CSM_RECORD_VISABLETO_SIZEC_DEF;
const std::size_t 						SMUCONST_CSM_RECORD_VISABLETO_SIZE		= sizeof(wchar_t) * SMUCONST_CSM_RECORD_VISABLETO_SIZEC; //256B
const boost::interprocess::offset_t		SMUCONST_CSM_RECORD_RESERVED_RECOFFSET	= SMUCONST_CSM_RECORD_KAT_RECOFFSET + SMUCONST_CSM_RECORD_KAT_SIZE;
const std::size_t 						SMUCONST_CSM_RECORD_RESERVED_SIZE		= 52;

const std::size_t 						SMUCONST_CSM_RECORD_SIZE 				= SMUCONST_CSM_RECORD_RECID_SIZE + SMUCONST_CSM_RECORD_TYPE_SIZE + SMUCONST_CSM_RECORD_VERSION_SIZE + SMUCONST_CSM_RECORD_STATUS_SIZE + SMUCONST_CSM_RECORD_KAT_SIZE + SMUCONST_CSM_RECORD_MSMT_SIZE + SMUCONST_CSM_RECORD_DNAME_SIZE + SMUCONST_CSM_RECORD_VISABLETO_SIZE + SMUCONST_CSM_RECORD_RESERVED_SIZE;
const std::size_t 						SMUCONST_CSM_ALLRECORDS_SIZE 			= SMUCONST_CSM_RECORD_SIZE * SMUCONST_CSM_RECORDS_COUNT;

const std::size_t 						SMUCONST_CSM_TOTAL_SIZE 				= SMUCONST_CSM_HEADER_SIZE + SMUCONST_CSM_ALLRECORDS_SIZE;
										//schould be equal 4096B

const double							CSMTHREAD_RECORD_MAXAGE  				= 125.;    //[s]

/////////////////  MSM CONSTS   /////////////////
const char * 							SMUCONST_MSM_NAME_PREFIX				= "mirfox_msm_";   //mirfox_msm_<##id><##no>

const boost::interprocess::offset_t		SMUCONST_MSM_HEADER_OFFSET				= 0;
const boost::interprocess::offset_t		SMUCONST_MSM_HEADER_ISLAST_OFFSET		= SMUCONST_MSM_HEADER_OFFSET;
const std::size_t 						SMUCONST_MSM_HEADER_ISLAST_SIZE 		= sizeof(char); //1B
const boost::interprocess::offset_t		SMUCONST_MSM_HEADER_RESERVED_OFFSET		= SMUCONST_MSM_HEADER_ISLAST_OFFSET + SMUCONST_MSM_HEADER_ISLAST_SIZE;
const std::size_t 						SMUCONST_MSM_HEADER_RESERVED_SIZE 		= 7;  //B
const std::size_t 						SMUCONST_MSM_HEADER_SIZE 				= SMUCONST_MSM_HEADER_ISLAST_SIZE + SMUCONST_MSM_HEADER_RESERVED_SIZE;

const int								SMUCONST_MSM_RECORDS_COUNT				= 180;
const boost::interprocess::offset_t		SMUCONST_MSM_RECORDS_OFFSET				= SMUCONST_MSM_HEADER_SIZE;

const boost::interprocess::offset_t		SMUCONST_MSM_RECORD_TYPE_RECOFFSET		= 0;
const std::size_t 						SMUCONST_MSM_RECORD_TYPE_SIZE			= sizeof(char); //1B
										//	[O]Option, [T]Translation, [A]Account, [G]Group, [C]Contact
const boost::interprocess::offset_t		SMUCONST_MSM_RECORD_HANDLE_RECOFFSET	= SMUCONST_MSM_RECORD_TYPE_RECOFFSET + SMUCONST_MSM_RECORD_TYPE_SIZE;
const std::size_t 						SMUCONST_MSM_RECORD_HANDLE_SIZE			= sizeof(uint64_t); //8B
										//	{AGC} miranda HANDLE to account/group/contact - HANDLE 32/64b, {O} option id, {T} translation id
const boost::interprocess::offset_t		SMUCONST_MSM_RECORD_ACCOUNTH_RECOFFSET	= SMUCONST_MSM_RECORD_HANDLE_RECOFFSET + SMUCONST_MSM_RECORD_HANDLE_SIZE;
const std::size_t 						SMUCONST_MSM_RECORD_ACCOUNTH_SIZE		= sizeof(uint64_t); //8B
										//	{C} account id for contact
const boost::interprocess::offset_t		SMUCONST_MSM_RECORD_GROUPH_RECOFFSET	= SMUCONST_MSM_RECORD_ACCOUNTH_RECOFFSET + SMUCONST_MSM_RECORD_ACCOUNTH_SIZE;
const std::size_t 						SMUCONST_MSM_RECORD_GROUPH_SIZE			= sizeof(uint64_t); //8B
										//	{C} group id for contact, {G} parent group id for group
const boost::interprocess::offset_t		SMUCONST_MSM_RECORD_STATUS_RECOFFSET	= SMUCONST_MSM_RECORD_GROUPH_RECOFFSET + SMUCONST_MSM_RECORD_GROUPH_SIZE;
const std::size_t 						SMUCONST_MSM_RECORD_STATUS_SIZE			= sizeof(char); //1B
										//	{CA} status mode [1-7], {G} group type (root/normal), {T} position if more then one translation for 1 translationId
const boost::interprocess::offset_t		SMUCONST_MSM_RECORD_DNAME_RECOFFSET		= SMUCONST_MSM_RECORD_STATUS_RECOFFSET + SMUCONST_MSM_RECORD_STATUS_SIZE;
const std::size_t 						SMUCONST_MSM_RECORD_DNAME_SIZEC			= SMUCONST_MSM_RECORD_DNAME_SIZEC_DEF;
const std::size_t 						SMUCONST_MSM_RECORD_DNAME_SIZE			= sizeof(wchar_t) * SMUCONST_MSM_RECORD_DNAME_SIZEC; //156B
										//	{AGC} display name {T} translated text {O} option value

const std::size_t 						SMUCONST_MSM_RECORD_SIZE 				= SMUCONST_MSM_RECORD_TYPE_SIZE + SMUCONST_MSM_RECORD_HANDLE_SIZE + SMUCONST_MSM_RECORD_ACCOUNTH_SIZE + SMUCONST_MSM_RECORD_GROUPH_SIZE + SMUCONST_MSM_RECORD_STATUS_SIZE + SMUCONST_MSM_RECORD_DNAME_SIZE;
const std::size_t 						SMUCONST_MSM_ALLRECORDS_SIZE 			= SMUCONST_MSM_RECORD_SIZE * SMUCONST_MSM_RECORDS_COUNT;

const std::size_t 						SMUCONST_MSM_TOTAL_SIZE 				= SMUCONST_MSM_HEADER_SIZE + SMUCONST_MSM_ALLRECORDS_SIZE;
										//schould be equal 32768B



/////////////////////PUBLIC///////////////////////////


/*static*/ SharedMemoryUtils * SharedMemoryUtils::m_pOnlyOneInstance;

SharedMemoryUtils::SharedMemoryUtils()
				: freeMsmRecordsCount(0)			//free records in current Miranda SM
				, nextMsmNumber(1)					//number of next Miranda SM instance
				, processCsmId(-1)					//current process record id in csm, not initialized here
				, logger(MFLogger::getInstance())
{
}

SharedMemoryUtils::~SharedMemoryUtils()
{
}



//public
//@see MirFoxCommons_sharedMemory.h
OpenOrCreateSMReturnStruct
SharedMemoryUtils::openOrCreateSM(char type, std::wstring& displayName, std::wstring& visableTo, bool doCommitSM)
{

	//TODO access to CSM is not synchronied, i think it's not neccessery
	csm = new boost::interprocess::windows_shared_memory(
			boost::interprocess::open_or_create,
			getCsmName(),
			boost::interprocess::read_write,
			getCsmTotalSize()
		); //TODO exception handling

	OpenOrCreateSMReturnStruct returnValue;
	returnValue.errorCode = 0;
	returnValue.processCsmId = 0;
	
	if (!isCsmInited(*csm)){
		initCsm(*csm);
	} else {
		int result = checkCsmVersion(*csm);
		if (result != 0){
			returnValue.errorCode = result;
			return returnValue;
		}
	}
	
	processCsmId = allocateRecordInCsm(*csm, type, displayName, visableTo, doCommitSM);

	if (isCsmInited(*csm)){
		logger->log_dp(L"csm inited ok. processCsmId = [%u]", processCsmId);
	}else{
		logger->log_dp(L"csm inited error. processCsmId = [%u]", processCsmId);
	}

	returnValue.processCsmId = processCsmId;
	return returnValue;

}


void
SharedMemoryUtils::resetMsmParameters()
{
	freeMsmRecordsCount = 0;
	nextMsmNumber = 1;
}

int
SharedMemoryUtils::addOptionToSM(int optionID, std::wstring& optionValue)
{
	return addRecordToMsm('O', (uint64_t)optionID, (uint64_t)NULL, (uint64_t)NULL, (char)NULL, optionValue);
}

int
SharedMemoryUtils::addTranslationToSM(int translationId, std::wstring& translationValue )
{
	return addRecordToMsm('T', (uint64_t)translationId, (uint64_t)NULL, (uint64_t)NULL, (char)NULL, translationValue);
}

int
SharedMemoryUtils::addAccountToSM(uint64_t mirandaAccountId, std::wstring& displayName)
{
	return addRecordToMsm('A', mirandaAccountId, (uint64_t)NULL, (uint64_t)NULL, (char)NULL, displayName);
}

int
SharedMemoryUtils::addGroupToSM(uint64_t mirandaGroupHandle, uint64_t parentGroupHandle, char groupType, std::wstring& displayName)
{
	return addRecordToMsm('G', mirandaGroupHandle, (uint64_t)NULL, parentGroupHandle, groupType, displayName);
}

int
SharedMemoryUtils::addContactToSM(uint64_t mirandaContactHandle, uint64_t mirandaAccountHandle, uint64_t mirandaGroupHandle, std::wstring& displayName)
{
	return addRecordToMsm('C', mirandaContactHandle, mirandaAccountHandle, mirandaGroupHandle, (char)NULL, displayName);
}


int
SharedMemoryUtils::commitSM()
{

	//mark last msm: isLastNode=1
	boost::interprocess::windows_shared_memory * lastMsm = &(msmList.back());
	boost::interprocess::mapped_region region1(
			*lastMsm,
			boost::interprocess::read_write,
			SMUCONST_MSM_HEADER_ISLAST_OFFSET,
			SMUCONST_MSM_HEADER_ISLAST_SIZE
		);
	char* isLastNodePtr = static_cast<char*>(region1.get_address());
	*isLastNodePtr = '1';


	uint16_t recordNo = processCsmId - 1;

	//set status to (W)orking
	boost::interprocess::mapped_region region3(
		*csm,
		boost::interprocess::read_write,
		SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_STATUS_RECOFFSET,
		SMUCONST_CSM_RECORD_STATUS_SIZE
	);
	char* recordStatusPtr = static_cast<char*>(region3.get_address());
	*recordStatusPtr = 'W';

	return 0;
}



boost::interprocess::windows_shared_memory*
SharedMemoryUtils::getSmById(const char* smName, std::size_t smSize){

	boost::interprocess::windows_shared_memory* sm
	 = new boost::interprocess::windows_shared_memory(
			boost::interprocess::open_or_create,
			smName,
			boost::interprocess::read_write,
			smSize
		); //TODO exception handling

	return sm;

}



//@see MirFoxCommons_sharedMemory.h
int
SharedMemoryUtils::checkCSM(boost::interprocess::windows_shared_memory* checkedCsm, uint16_t processCsmId, std::wstring& displayNameProfile)
{

	if (!isCsmInited(*checkedCsm)){
		initCsm(*checkedCsm);
	} else {
		int result = checkCsmVersion(*checkedCsm);
		if (result != 0){
			return result;
		}
	}

	int result2 = checkCsmRecord(*checkedCsm, processCsmId, displayNameProfile);
	return result2;

}


void
SharedMemoryUtils::updateCsmTimestamp(boost::interprocess::windows_shared_memory& updateCsm, uint16_t processCsmId)
{
	uint16_t recordNo = processCsmId - 1;

	boost::interprocess::mapped_region region1(
			updateCsm,
			boost::interprocess::read_write,
			SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_KAT_RECOFFSET,
			SMUCONST_CSM_RECORD_KAT_SIZE
		);
	time_t* recordKatPtr = static_cast<time_t*>(region1.get_address());
	*recordKatPtr = mfGetCurrentTimestamp();

}



void
SharedMemoryUtils::checkAnotherCsm(boost::interprocess::windows_shared_memory& checkedCsm, uint16_t processCsmId){

	time_t currentTimestamp = mfGetCurrentTimestamp();

	for (uint16_t recordNo = 0; recordNo < SMUCONST_CSM_RECORDS_COUNT; recordNo++ ){

		if (recordNo == processCsmId - 1){ //do not check own record
			continue;
		}

		boost::interprocess::mapped_region region1(
				checkedCsm,
				boost::interprocess::read_write,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_STATUS_RECOFFSET,
				SMUCONST_CSM_RECORD_STATUS_SIZE
			);
		char* recordStatusPtr = static_cast<char*>(region1.get_address());

		if (*recordStatusPtr == 'W'){
			//for every record in status (W) Working

			boost::interprocess::mapped_region region2(
					checkedCsm,
					boost::interprocess::read_write,
					SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_KAT_RECOFFSET,
					SMUCONST_CSM_RECORD_KAT_SIZE
				);
			time_t* recordKatPtr = static_cast<time_t*>(region2.get_address());

			double timeDiff = /*std::*/difftime(currentTimestamp, *recordKatPtr);
			logger->log_p(L"checkAnotherCsm: recordNo=%u (processCsmId=%u) status==W timeDiff=%f", recordNo, recordNo+1, timeDiff);


			if (timeDiff > CSMTHREAD_RECORD_MAXAGE){
				logger->log(L"checkAnotherCsm: free record in CSM");
				freeCsmRecord(checkedCsm, recordNo);
			}


		}//end if

	}//end for

}




void
SharedMemoryUtils::unloadSharedMemory(uint16_t processCsmId) //
{
	unloadFromCSM(processCsmId);
	unloadMSMs();
}




//TODO - add windows user name
const char*
SharedMemoryUtils::getCsmName()
{
	return SMUCONST_CSM_NAME;
}

//TODO - add windows user name
/**
 * returns expecter name for miranda shared memory with id from csm record (from 1 to 99) and current number (from 1 to 99)
 * mirfox_msm_<##id><##no>
 */
std::string
SharedMemoryUtils::getMsmName(uint16_t id, int currentNumber)
{

	std::stringstream msmName;
	msmName << SMUCONST_MSM_NAME_PREFIX;

	if (id <= 9){
		msmName << "0";
	}
	msmName << id;

	if (currentNumber <= 9){
		msmName << "0";
	}
	msmName << currentNumber;

	logger->log_p(L"SharedMemoryUtils::getMsmName = [%S]", msmName.str().c_str());

	return msmName.str();
}


//return total size of CSM in Bytes
std::size_t
SharedMemoryUtils::getCsmTotalSize()
{

	//logger->log_dp(L"SMUCONST_CSM_TOTAL_SIZE = %u", SMUCONST_CSM_TOTAL_SIZE);
	return SMUCONST_CSM_TOTAL_SIZE;

}


//return total size of MSM in Bytes
std::size_t
SharedMemoryUtils::getMsmTotalSize()
{

	//logger->log_dp(L"SMUCONST_MSM_TOTAL_SIZE = %u", SMUCONST_MSM_TOTAL_SIZE);
	return SMUCONST_MSM_TOTAL_SIZE;

}





bool
SharedMemoryUtils::isAnyMirandaCsmRecordAvailable(std::wstring& forThisProfileName)
{

	for (int recordNo = 0; recordNo < SMUCONST_CSM_RECORDS_COUNT; recordNo++ ){

		boost::interprocess::mapped_region region1(
			*csm,
			boost::interprocess::read_only,
			SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_STATUS_RECOFFSET,
			SMUCONST_CSM_RECORD_STATUS_SIZE
		);
		char* recordStatusPtr = static_cast<char*>(region1.get_address());

		if (*recordStatusPtr == 'W'){

			boost::interprocess::mapped_region region2(
				*csm,
				boost::interprocess::read_only,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_TYPE_RECOFFSET,
				SMUCONST_CSM_RECORD_TYPE_SIZE
			);
			char* recordTypePtr = static_cast<char*>(region2.get_address());

			if (*recordTypePtr == 'M'){

				boost::interprocess::mapped_region region3(
					*csm,
					boost::interprocess::read_only,
					SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_VISABLETO_RECOFFSET,
					SMUCONST_CSM_RECORD_VISABLETO_SIZE
				);
				wchar_t* recordVisabletoPtr = static_cast<wchar_t*>(region3.get_address());
				std::wstring visableTo = recordVisabletoPtr;

				if (isTokenOnList(forThisProfileName, visableTo)){
					return true;
				}

			}//end if M

		}//end if W

	}//end for

	return false;
}






int
SharedMemoryUtils::getClientInstances(boost::ptr_list<ClientInstanceClass> * clientInstancesListPtr, std::wstring& forThisProfileName)
{

	if (clientInstancesListPtr==NULL){
		logger->log(L"SharedMemoryUtils::getClientInstances clientInstancesListPtr==NULL");
		return -1;
	}

	int count = 0;

	for (int recordNo = 0; recordNo < SMUCONST_CSM_RECORDS_COUNT; recordNo++ ){

		boost::interprocess::mapped_region region1(
			*csm,
			boost::interprocess::read_only,
			SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_STATUS_RECOFFSET,
			SMUCONST_CSM_RECORD_STATUS_SIZE
		);
		char* recordStatusPtr = static_cast<char*>(region1.get_address());

		if (*recordStatusPtr == 'W'){

			boost::interprocess::mapped_region region2(
				*csm,
				boost::interprocess::read_only,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_TYPE_RECOFFSET,
				SMUCONST_CSM_RECORD_TYPE_SIZE
			);
			char* recordTypePtr = static_cast<char*>(region2.get_address());

			if (*recordTypePtr == 'M'){

				boost::interprocess::mapped_region region3(
					*csm,
					boost::interprocess::read_only,
					SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_VISABLETO_RECOFFSET,
					SMUCONST_CSM_RECORD_VISABLETO_SIZE
				);
				wchar_t* recordVisabletoPtr = static_cast<wchar_t*>(region3.get_address());
				std::wstring visableTo = recordVisabletoPtr;

				if (isTokenOnList(forThisProfileName, visableTo)){

					//this instance match all conditions
					//get its name and id (recordNo)
					ClientInstanceClass* clientInstance = new ClientInstanceClass();

					boost::interprocess::mapped_region region4(
						*csm,
						boost::interprocess::read_write,
						SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_RECID_RECOFFSET,
						SMUCONST_CSM_RECORD_RECID_SIZE
					);
					uint16_t* recordIdPtr = static_cast<uint16_t*>(region4.get_address());
					clientInstance->recordId = *recordIdPtr;

					boost::interprocess::mapped_region region5(
						*csm,
						boost::interprocess::read_only,
						SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_DNAME_RECOFFSET,
						SMUCONST_CSM_RECORD_DNAME_SIZE
					);
					wchar_t* displayNamePtr = static_cast<wchar_t*>(region5.get_address());
					clientInstance->displayName = displayNamePtr;


					clientInstancesListPtr->push_back(clientInstance);
					count++;

				}

			}//end if M

		}//end if W

	}//end for

	logger->log_p(L"SharedMemoryUtils::getClientInstances return count = %d", count);
	return count;
}




int
SharedMemoryUtils::getContacts(boost::ptr_list<ContactClass> * contactsListPtr, unsigned short clientRecordNo)
{

	if (contactsListPtr==NULL){
		logger->log(L"SharedMemoryUtils::getContacts contactsListPtr==NULL");
		return -1;
	}

	bool isMsmAvailable = true;
	int msmNumber = 1;
	std::string msmName;
	boost::interprocess::windows_shared_memory* msm;

	int foundContactsCount = 0;


	do {

		//get MSM to seek contacts
		msmName = getMsmName(clientRecordNo, msmNumber);
		msmNumber++;

		try {

			msm = new boost::interprocess::windows_shared_memory(
					boost::interprocess::open_only,
					msmName.c_str(),
					boost::interprocess::read_only);

		} catch (const boost::interprocess::interprocess_exception& ex) {
			isMsmAvailable = false;
			logger->log_p(L"SharedMemoryUtils::getContacts (isMsmAvailable = false) interprocess_exception: [%S] native_error=[%d] error_code=[%d]", ex.what(), ex.get_native_error(), ex.get_error_code() );
		} catch (...){
			isMsmAvailable = false;
			logger->log(L"SharedMemoryUtils::getContacts (isMsmAvailable = false) EXCEPTION: unknown");
		}
		//TODO problems with this catch


		if(isMsmAvailable){

			for (int recordNo = 0; recordNo < SMUCONST_MSM_RECORDS_COUNT; recordNo++ ){

				boost::interprocess::mapped_region region1(
					*msm,
					boost::interprocess::read_only,
					SMUCONST_MSM_HEADER_SIZE + (recordNo * SMUCONST_MSM_RECORD_SIZE) + SMUCONST_MSM_RECORD_TYPE_RECOFFSET,
					SMUCONST_MSM_RECORD_TYPE_SIZE
				);
				char* recordTypePtr = static_cast<char*>(region1.get_address());

				if (*recordTypePtr == 'C'){  //Contact

					ContactClass* contactInstance = new ContactClass();

					boost::interprocess::mapped_region region2(
						*msm,
						boost::interprocess::read_only,
						SMUCONST_MSM_HEADER_SIZE + (recordNo * SMUCONST_MSM_RECORD_SIZE) + SMUCONST_MSM_RECORD_DNAME_RECOFFSET,
						SMUCONST_MSM_RECORD_DNAME_SIZE
					);
					wchar_t* displayNamePtr = static_cast<wchar_t*>(region2.get_address());
					contactInstance->displayName = displayNamePtr;

					boost::interprocess::mapped_region region3(
						*msm,
						boost::interprocess::read_only,
						SMUCONST_MSM_HEADER_SIZE + (recordNo * SMUCONST_MSM_RECORD_SIZE) + SMUCONST_MSM_RECORD_HANDLE_RECOFFSET,
						SMUCONST_MSM_RECORD_HANDLE_SIZE
					);
					uint64_t* agcHandlePtr = static_cast<uint64_t*>(region3.get_address());
					contactInstance->handle = *agcHandlePtr;

					contactsListPtr->push_back(contactInstance);
					foundContactsCount++;

				}


				//TODO - set all records to type 'E'Empty - if E break for



			} //end for


			//check SMUCONST_MSM_HEADER_ISLAST_OFFSET
			boost::interprocess::mapped_region region4(
				*msm,
				boost::interprocess::read_only,
				SMUCONST_MSM_HEADER_ISLAST_OFFSET,
				SMUCONST_MSM_HEADER_ISLAST_SIZE
			);
			char* isLastNodePtr = static_cast<char*>(region4.get_address());
			if (*isLastNodePtr == '1'){
				isMsmAvailable = false;
				logger->log(L"SharedMemoryUtils::getContacts - isLastNodePtr - isMsmAvailable = false");
			}


		}// end if

	} while (isMsmAvailable);

	logger->log_p(L"SharedMemoryUtils::getContacts return foundContactsCount = %d", foundContactsCount);

	return foundContactsCount;
}


int
SharedMemoryUtils::getAccounts(boost::ptr_list<AccountClass> * accountsListPtr, unsigned short clientRecordNo)
{

	if (accountsListPtr==NULL){
		logger->log(L"SharedMemoryUtils::getAccounts contactsListPtr==NULL");
		return -1;
	}

	bool isMsmAvailable = true;
	int msmNumber = 1;
	std::string msmName;
	boost::interprocess::windows_shared_memory* msm;

	int foundAccountsCount = 0;


	do {

		//get MSM to seek accounts
		msmName = getMsmName(clientRecordNo, msmNumber);
		msmNumber++;

		try {

			msm = new boost::interprocess::windows_shared_memory(
					boost::interprocess::open_only,
					msmName.c_str(),
					boost::interprocess::read_only);

		} catch (const boost::interprocess::interprocess_exception& ex) {
			isMsmAvailable = false;
			logger->log_p(L"SharedMemoryUtils::getAccounts (isMsmAvailable = false) interprocess_exception: [%S] native_error=[%d] error_code=[%d]", ex.what(), ex.get_native_error(), ex.get_error_code() );
		} catch (...){
			isMsmAvailable = false;
			logger->log(L"SharedMemoryUtils::getAccounts (isMsmAvailable = false) EXCEPTION: unknown");
		}
		//TODO problems with this catch


		if(isMsmAvailable){

			for (int recordNo = 0; recordNo < SMUCONST_MSM_RECORDS_COUNT; recordNo++ ){

				boost::interprocess::mapped_region region1(
					*msm,
					boost::interprocess::read_only,
					SMUCONST_MSM_HEADER_SIZE + (recordNo * SMUCONST_MSM_RECORD_SIZE) + SMUCONST_MSM_RECORD_TYPE_RECOFFSET,
					SMUCONST_MSM_RECORD_TYPE_SIZE
				);
				char* recordTypePtr = static_cast<char*>(region1.get_address());

				if (*recordTypePtr == 'A'){  //Account

					AccountClass* accountInstance = new AccountClass();

					boost::interprocess::mapped_region region2(
						*msm,
						boost::interprocess::read_only,
						SMUCONST_MSM_HEADER_SIZE + (recordNo * SMUCONST_MSM_RECORD_SIZE) + SMUCONST_MSM_RECORD_DNAME_RECOFFSET,
						SMUCONST_MSM_RECORD_DNAME_SIZE
					);
					wchar_t* displayNamePtr = static_cast<wchar_t*>(region2.get_address());
					accountInstance->displayName = displayNamePtr;

					boost::interprocess::mapped_region region3(
						*msm,
						boost::interprocess::read_only,
						SMUCONST_MSM_HEADER_SIZE + (recordNo * SMUCONST_MSM_RECORD_SIZE) + SMUCONST_MSM_RECORD_HANDLE_RECOFFSET,
						SMUCONST_MSM_RECORD_HANDLE_SIZE
					);
					uint64_t* agcHandlePtr = static_cast<uint64_t*>(region3.get_address());
					accountInstance->handle = *agcHandlePtr;

					accountsListPtr->push_back(accountInstance);
					foundAccountsCount++;

				}


				//TODO - set all records to type 'E'Empty - if E break for



			} //end for


			//check SMUCONST_MSM_HEADER_ISLAST_OFFSET
			boost::interprocess::mapped_region region4(
				*msm,
				boost::interprocess::read_only,
				SMUCONST_MSM_HEADER_ISLAST_OFFSET,
				SMUCONST_MSM_HEADER_ISLAST_SIZE
			);
			char* isLastNodePtr = static_cast<char*>(region4.get_address());
			if (*isLastNodePtr == '1'){
				isMsmAvailable = false;
				logger->log(L"SharedMemoryUtils::getAccounts - isLastNodePtr - isMsmAvailable = false");
			}


		}// end if

	} while (isMsmAvailable);

	logger->log_p(L"SharedMemoryUtils::getAccounts return foundAccountsCount = %d", foundAccountsCount);

	return foundAccountsCount;
}















//////////////PRIVATE///////////////






//return true if version field in header of csm is > 0
bool
SharedMemoryUtils::isCsmInited(boost::interprocess::windows_shared_memory & csm)
{

	boost::interprocess::mapped_region region(
			csm,
			boost::interprocess::read_write,
			SMUCONST_CSM_HEADER_VERSION_OFFSET,
			SMUCONST_CSM_HEADER_VERSION_SIZE
		);

	uint32_t* versionPtr = static_cast<uint32_t*>(region.get_address());

	if (*versionPtr != 0){
		logger->log_d(L"isCsmInited: csm was initialized before");
		return true;
	}else{
		logger->log(L"isCsmInited: csm was NOT initialized before");
		return false;
	}


}


void
SharedMemoryUtils::initCsm(boost::interprocess::windows_shared_memory& csm)
{

	//save csm header version
	boost::interprocess::mapped_region region1(
			csm,
			boost::interprocess::read_write,
			SMUCONST_CSM_HEADER_VERSION_OFFSET,
			SMUCONST_CSM_HEADER_VERSION_SIZE
		);
	uint32_t* versionPtr = static_cast<uint32_t*>(region1.get_address());
	*versionPtr = SMUCONST_CSM_CURRENT_VERSION;


	//prepare records
	for (uint16_t recordNo = 0; recordNo < SMUCONST_CSM_RECORDS_COUNT; recordNo++ ){

		//save scm record id (=recordNo)
		boost::interprocess::mapped_region region2(
				csm,
				boost::interprocess::read_write,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_RECID_RECOFFSET,
				SMUCONST_CSM_RECORD_RECID_SIZE
			);
		uint16_t* recordIdPtr = static_cast<uint16_t*>(region2.get_address());
		*recordIdPtr = recordNo + 1;

		//save scm record status (=E (Empty))
		boost::interprocess::mapped_region region3(
				csm,
				boost::interprocess::read_write,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_STATUS_RECOFFSET,
				SMUCONST_CSM_RECORD_STATUS_SIZE
			);
		char* recordStatusPtr = static_cast<char*>(region3.get_address());
		*recordStatusPtr = 'E';

	}


}


//@see MirFoxCommons_sharedMemory.h
int
SharedMemoryUtils::checkCsmVersion(boost::interprocess::windows_shared_memory& csm){

	boost::interprocess::mapped_region region(
			csm,
			boost::interprocess::read_write,
			SMUCONST_CSM_HEADER_VERSION_OFFSET,
			SMUCONST_CSM_HEADER_VERSION_SIZE
		);

	uint32_t* versionPtr = static_cast<uint32_t*>(region.get_address());

	//check existing csm version
	if (*versionPtr > SMUCONST_CSM_CURRENT_VERSION){
		return -3;  //csm is too new
	} else if (*versionPtr < SMUCONST_CSM_CURRENT_VERSION){
		return -4;  //csm is too old
	} else { //if (*versionPtr == SMUCONST_CSM_CURRENT_VERSION)
		return 0;   //csm is ok
	}

}





/**
 *
 * return
 * >0 - id of assigned record
 * -1 - no empty record found
 */
uint16_t
SharedMemoryUtils::allocateRecordInCsm(boost::interprocess::windows_shared_memory& csm, char type, std::wstring& displayName, std::wstring& visableTo, bool doCommitSM)
{

	//try to find first record with status E (Empty) in CSM
	for (unsigned short recordNo = 0; recordNo < SMUCONST_CSM_RECORDS_COUNT; recordNo++ ){
	
		boost::interprocess::mapped_region region1(
			csm,
			boost::interprocess::read_write,
			SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_STATUS_RECOFFSET,
			SMUCONST_CSM_RECORD_STATUS_SIZE
		);
		char* recordStatusPtr = static_cast<char*>(region1.get_address());
		
		if (*recordStatusPtr == 'E'){

			//save new status 'N' (New)
			*recordStatusPtr = 'N';
		
			//save type
			boost::interprocess::mapped_region region2(
				csm,
				boost::interprocess::read_write,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_TYPE_RECOFFSET,
				SMUCONST_CSM_RECORD_TYPE_SIZE
			);
			char* recordTypePtr = static_cast<char*>(region2.get_address());
			*recordTypePtr = type;
			
			//save first keep alive timestamp
			boost::interprocess::mapped_region region3(
				csm,
				boost::interprocess::read_write,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_KAT_RECOFFSET,
				SMUCONST_CSM_RECORD_KAT_SIZE
			);
			time_t* recordKatPtr = static_cast<time_t*>(region3.get_address());
			*recordKatPtr = mfGetCurrentTimestamp();

			//save version
			boost::interprocess::mapped_region region4(
				csm,
				boost::interprocess::read_write,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_VERSION_RECOFFSET,
				SMUCONST_CSM_RECORD_VERSION_SIZE
			);
			uint16_t* recordVersionPtr = static_cast<uint16_t*>(region4.get_address());
			*recordVersionPtr = SMUCONST_CSM_CURRENT_RECORDVERSION;

			//save display name
			boost::interprocess::mapped_region region5(
				csm,
				boost::interprocess::read_write,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_DNAME_RECOFFSET,
				SMUCONST_CSM_RECORD_DNAME_SIZE
			);
			wchar_t* recordDnamePtr = static_cast<wchar_t*>(region5.get_address());
			wcsncpy_s(recordDnamePtr, SMUCONST_CSM_RECORD_DNAME_SIZEC, displayName.c_str(), _TRUNCATE);

			//save visableTo
			boost::interprocess::mapped_region region6(
				csm,
				boost::interprocess::read_write,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_VISABLETO_RECOFFSET,
				SMUCONST_CSM_RECORD_VISABLETO_SIZE
			);
			wchar_t* recordVisabletoPtr = static_cast<wchar_t*>(region6.get_address());
			wcsncpy_s(recordVisabletoPtr, SMUCONST_CSM_RECORD_VISABLETO_SIZEC, visableTo.c_str(), _TRUNCATE);
		
			//get id
			boost::interprocess::mapped_region region7(
				csm,
				boost::interprocess::read_write,
				SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_RECID_RECOFFSET,
				SMUCONST_CSM_RECORD_RECID_SIZE
			);
			uint16_t* recordIdPtr = static_cast<uint16_t*>(region7.get_address());
			

			if (doCommitSM){
				*recordStatusPtr = 'W';
			}

			logger->log_p(L"allocateRecordInCsm: allocated record with id (processCsmId): %u", *recordIdPtr);
			
			return *recordIdPtr;
		
		}//end if
	
	}//end for
	
	logger->log(L"allocateRecordInCsm: no empty records in CSM. return -1");
	return -2;
	
}



int
SharedMemoryUtils::addRecordToMsm(char type, uint64_t agcHandle, uint64_t accountHandle, uint64_t groupHandle, char status, std::wstring& value)
{

	logger->log_p(L"addRecordToMsm: type=[%c], agcHandle=[%I64u], accountHandle=[%I64u], groupHandle=[%I64u], status=[%c], value=[%s], agcHandle=[%I64u]",
			type, agcHandle, accountHandle, groupHandle, status==NULL?'-':status , value.c_str(), agcHandle);


	//if there is not enought space in current msm - create new one
	if (freeMsmRecordsCount == 0){

		std::size_t msmTotalSize = getMsmTotalSize();
		std::string msmName = getMsmName(processCsmId, nextMsmNumber);

		logger->log_p(L"create new MSM msmName=[%S] " SCNuPTR L" Bytes (schould be 32768 B)",	msmName.c_str(), msmTotalSize);

		boost::interprocess::windows_shared_memory* msm;
		msm = new boost::interprocess::windows_shared_memory(
			boost::interprocess::open_or_create,
			msmName.c_str(),
			boost::interprocess::read_write,
			msmTotalSize
		); //TODO exception handling //TODO delete msm

		//zero shared memory (neccessery if sm was opened(it is old wrong msm from deleted miranda msm record so it contains old bad data))
		boost::interprocess::mapped_region regionToDel(
				*msm,
				boost::interprocess::read_write,
				0,
				msmTotalSize
			);
		memset(regionToDel.get_address(), 0, msmTotalSize);

		//set isLastNodePtr initial value to 0 , will be update at commitCSM()
		boost::interprocess::mapped_region region1(
				*msm,
				boost::interprocess::read_write,
				SMUCONST_MSM_HEADER_ISLAST_OFFSET,
				SMUCONST_MSM_HEADER_ISLAST_SIZE
			);
		char* isLastNodePtr = static_cast<char*>(region1.get_address());
		*isLastNodePtr = '0';

		//TODO - set all records to type 'E'Empty

		msmList.push_back(msm);

		nextMsmNumber++;
		freeMsmRecordsCount = SMUCONST_MSM_RECORDS_COUNT;

	}

	boost::interprocess::windows_shared_memory * currentMsm = &(msmList.back());

	//save data to record
	int actualRecordOffset = SMUCONST_MSM_RECORDS_OFFSET + ((SMUCONST_MSM_RECORDS_COUNT - freeMsmRecordsCount) * SMUCONST_MSM_RECORD_SIZE);

	//char type
	boost::interprocess::mapped_region region1(
		*currentMsm,
		boost::interprocess::read_write,
		actualRecordOffset + SMUCONST_MSM_RECORD_TYPE_RECOFFSET,
		SMUCONST_MSM_RECORD_TYPE_SIZE
	);
	char* recordTypePtr = static_cast<char*>(region1.get_address());
	*recordTypePtr = type;

	//HANDLE agcHandle
	boost::interprocess::mapped_region region2(
		*currentMsm,
		boost::interprocess::read_write,
		actualRecordOffset + SMUCONST_MSM_RECORD_HANDLE_RECOFFSET,
		SMUCONST_MSM_RECORD_HANDLE_SIZE
	);
	uint64_t* agcHandlePtr  = static_cast<uint64_t*>(region2.get_address());
	*agcHandlePtr = agcHandle;

	//HANDLE accountHandle
	boost::interprocess::mapped_region region3(
		*currentMsm,
		boost::interprocess::read_write,
		actualRecordOffset + SMUCONST_MSM_RECORD_ACCOUNTH_RECOFFSET,
		SMUCONST_MSM_RECORD_ACCOUNTH_SIZE
	);
	uint64_t* accountHandlePtr = static_cast<uint64_t*>(region3.get_address());
	*accountHandlePtr = accountHandle;

	//HANDLE groupHandle
	boost::interprocess::mapped_region region4(
		*currentMsm,
		boost::interprocess::read_write,
		actualRecordOffset + SMUCONST_MSM_RECORD_GROUPH_RECOFFSET,
		SMUCONST_MSM_RECORD_GROUPH_SIZE
	);
	uint64_t* groupHandlePtr = static_cast<uint64_t*>(region4.get_address());
	*groupHandlePtr = groupHandle;

	//char status
	boost::interprocess::mapped_region region5(
		*currentMsm,
		boost::interprocess::read_write,
		actualRecordOffset + SMUCONST_MSM_RECORD_STATUS_RECOFFSET,
		SMUCONST_MSM_RECORD_STATUS_SIZE
	);
	char* recordStatusPtr = static_cast<char*>(region5.get_address());
	*recordStatusPtr = status;

	//std::wstring& value
	boost::interprocess::mapped_region region6(
		*currentMsm,
		boost::interprocess::read_write,
		actualRecordOffset + SMUCONST_MSM_RECORD_DNAME_RECOFFSET,
		SMUCONST_MSM_RECORD_DNAME_SIZE
	);
	wchar_t* recordValuePtr = static_cast<wchar_t*>(region6.get_address());
	wcsncpy_s(recordValuePtr, SMUCONST_MSM_RECORD_DNAME_SIZEC, value.c_str(), _TRUNCATE);

	freeMsmRecordsCount --;

	return 0;
}




time_t
SharedMemoryUtils::mfGetCurrentTimestamp()
{

	time_t currentTimestamp = time(NULL);
	//logger->log_dp(L"mfGetCurrentTimestamp() [" SCNuPTR L"]", currentTimestamp);
	return currentTimestamp;

}


int
SharedMemoryUtils::checkCsmRecord(boost::interprocess::windows_shared_memory& checkedCsm, uint16_t processCsmId, std::wstring& displayNameProfile)
{
	uint16_t recordNo = processCsmId - 1;

	boost::interprocess::mapped_region region1(
		checkedCsm,
		boost::interprocess::read_write,
		SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_STATUS_RECOFFSET,
		SMUCONST_CSM_RECORD_STATUS_SIZE
	);
	char* recordStatusPtr = static_cast<char*>(region1.get_address());

	if (*recordStatusPtr == 'W'){

		boost::interprocess::mapped_region region2(
			checkedCsm,
			boost::interprocess::read_only,
			SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_DNAME_RECOFFSET,
			SMUCONST_CSM_RECORD_DNAME_SIZE
		);
		wchar_t* displayNamePtr = static_cast<wchar_t*>(region2.get_address());
		std::wstring displayNameSM = displayNamePtr;
		
		if (displayNameSM.compare(displayNameProfile) == 0){
			return 0; //OK
		} else {
			logger->log_p(L"checkCsmRecord not ok for processCsmId = %u: displayNameProfile [%s] doesn't match displayNameSM [%s].", processCsmId, displayNameProfile.c_str(), displayNameSM.c_str());
			return 1; //csm record data corrupted - displayName not match 
		}
		
	} else {

		logger->log_p(L"checkCsmRecord not ok for processCsmId = %u: recordStatus is [%c] expected [W].", processCsmId, *recordStatusPtr);
		return 1; //csm record data corrupted - not in state W

	}

}


//recordNo === processCsmId -1
void
SharedMemoryUtils::freeCsmRecord(boost::interprocess::windows_shared_memory& csm, uint16_t recordNo)
{

	logger->log_p(L"freeCsmRecord: recordNo = %u processCsmId = %u", recordNo, recordNo+1);

	//zero type
	boost::interprocess::mapped_region region1(
		csm,
		boost::interprocess::read_write,
		SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_TYPE_RECOFFSET,
		SMUCONST_CSM_RECORD_TYPE_SIZE
	);
	char* recordTypePtr = static_cast<char*>(region1.get_address());
	*recordTypePtr = (char)0;

	//zero keepalive timestamp
	boost::interprocess::mapped_region region2(
		csm,
		boost::interprocess::read_write,
		SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_KAT_RECOFFSET,
		SMUCONST_CSM_RECORD_KAT_SIZE
	);
	time_t* recordKatPtr = static_cast<time_t*>(region2.get_address());
	*recordKatPtr = (time_t)0;

	//zero msm timestamp
	boost::interprocess::mapped_region region3(
		csm,
		boost::interprocess::read_write,
		SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_MSMT_RECOFFSET,
		SMUCONST_CSM_RECORD_MSMT_SIZE
	);
	time_t* recordMsmtPtr = static_cast<time_t*>(region3.get_address());
	*recordMsmtPtr = (time_t)0;

	//zero displayname
	boost::interprocess::mapped_region region4(
		csm,
		boost::interprocess::read_write,
		SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_DNAME_RECOFFSET,
		SMUCONST_CSM_RECORD_DNAME_SIZE
	);
	memset(region4.get_address(), 0, SMUCONST_CSM_RECORD_DNAME_SIZE);

	//zero visableto
	boost::interprocess::mapped_region region5(
		csm,
		boost::interprocess::read_write,
		SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_VISABLETO_RECOFFSET,
		SMUCONST_CSM_RECORD_VISABLETO_SIZE
	);
	memset(region5.get_address(), 0, SMUCONST_CSM_RECORD_VISABLETO_SIZE);

	//status = E
	boost::interprocess::mapped_region region6(
		csm,
		boost::interprocess::read_write,
		SMUCONST_CSM_HEADER_SIZE + (recordNo * SMUCONST_CSM_RECORD_SIZE) + SMUCONST_CSM_RECORD_STATUS_RECOFFSET,
		SMUCONST_CSM_RECORD_STATUS_SIZE
	);
	char* recordStatusPtr = static_cast<char*>(region6.get_address());
	*recordStatusPtr = 'E';
}


bool
SharedMemoryUtils::isTokenOnList(std::wstring& token, std::wstring& tokensList)
{
	if (tokensList.length() == 0){
		//TODO @future - empty tokensList (visableTo )string but checkbox at miranda options checked - so it meens not to show anybody
		return true;
	}

	typedef boost::tokenizer<boost::char_separator<wchar_t,std::wstring::traits_type>,std::wstring::const_iterator,std::wstring > TOKENIZER;
	boost::char_separator<wchar_t,std::wstring::traits_type> sep(L";,. ");
	TOKENIZER tokens(tokensList, sep );
	BOOST_FOREACH(std::wstring tokenFromTokens, tokens)
	{
		if (tokenFromTokens.compare(token) == 0){
			return true;
		}
	}
	return false;
}


void
SharedMemoryUtils::unloadFromCSM(uint16_t processCsmId)
{
	if (csm == NULL){
		logger->log(L"SharedMemoryUtils::unloadFromCSM: csm == NULL , return");
		return;
	}
	if (processCsmId <= 0){
		logger->log(L"SharedMemoryUtils::unloadFromCSM: Csm Record not initialized , return");
		return;
	}
	freeCsmRecord(*csm, processCsmId - 1);
	delete csm;
}


void
SharedMemoryUtils::unloadMSMs()
{
	logger->log_p(L"SharedMemoryUtils::unloadMSMs: size=%d msmList.clear", msmList.size());
	msmList.clear(); //all pointers are deleted by boost
}



#ifndef _WSX22_IPC_SHAREDMEMORY
#define _WSX22_IPC_SHAREDMEMORY


#include "MirFoxCommons_logger.h"


//shared memory related definitions
#define CSMTHREAD_TICK_TIME		100		// 100ms - check exit every this time
#define CSMTHREAD_FIRSTRUN_TIME	100		// CSMTHREAD_FIRSTRUN_TIME * CSMTHREAD_TICK_TIME = 10s  - time to first run of checking csm state in csm thread (after thread start)
#define CSMTHREAD_NEXTRUN_TIME	100		// CSMTHREAD_NEXTRUN_TIME * CSMTHREAD_TICK_TIME = 10s  - time to next run of checking csm state in csm thread

#define SMUCONST_CSM_RECORD_VISABLETO_SIZEC_DEF 128
#define SMUCONST_CSM_RECORD_DNAME_SIZEC_DEF 78
#define SMUCONST_MSM_RECORD_DNAME_SIZEC_DEF 78



class ClientInstanceClass
{
public:
	std::wstring displayName;	//display name
	uint16_t recordId;			//id
};

class ContactClass
{
public:
	std::wstring displayName;	//display name
	uint64_t handle;			//miranda HANDLE to contact (casted pointer)
};

class AccountClass
{
public:
	std::wstring displayName;	//display name
	uint64_t handle;			//miranda HANDLE to account (casted pointer)
};



struct OpenOrCreateSMReturnStruct {
	int errorCode;
	uint16_t processCsmId;
};



/**
 * SharedMemory Utils
 *
 * Singleton pattern based on
 * http://www.codeproject.com/KB/threads/SingletonThreadSafety.aspx
 */
class SharedMemoryUtils
{

public:

	
	//constructor
	SharedMemoryUtils();
	//destructor
	~SharedMemoryUtils();


	/**
	 * open existing or create new central shared memory
	 *
	 * type - char - 'M' - Miranda, 'F' - Firefox
	 *
	 * return OpenOrCreateSMReturnStruct
	 * .errorCode
	 *  0 - ok, and .processCsmId = id of process record in csm
	 * -2 - no more available (Empty) records in central shared memory
	 * -3 - existing csm version is too high (from checkCsmVersion)
	 * -4 - existing csm version is too low (from checkCsmVersion)
	 */
	OpenOrCreateSMReturnStruct openOrCreateSM(char type, std::wstring& displayName, std::wstring& visableTo, bool doCommitSM);


	int addOptionToSM(int optionID, std::wstring& optionValue);

	int addTranslationToSM(int translationId, std::wstring& translationValue );

	int addAccountToSM(uint64_t mirandaAccountId, std::wstring& displayName);

	// group type:  R - root, N - normal
	int addGroupToSM(uint64_t mirandaGroupHandle, uint64_t parentGroupHandle, char groupType, std::wstring& displayName);

	int addContactToSM(uint64_t mirandaContactHandle, uint64_t mirandaAccountHandle, uint64_t mirandaGroupHandle, std::wstring& displayName);

	//call after openOrCreateSM and after creating sm thread
	int commitSM();

	//delete returned sm object after use
	boost::interprocess::windows_shared_memory* getSmById(const char* smName, std::size_t smSize);

	//returns:
	// 0 - ok
	// 1 - record error (state != 'W' (working) or wrong displayName)
	// -3 - existing csm version is too high (from checkCsmVersion)
	// -4 - existing csm version is too low (from checkCsmVersion)
	int checkCSM(boost::interprocess::windows_shared_memory* checkedCsm, uint16_t processCsmId, std::wstring& displayNameProfile);

	void updateCsmTimestamp(boost::interprocess::windows_shared_memory& updateCsm, uint16_t processCsmId);

	//dla wszystkich innych rekord�w w csm w statusie W, sprawdza czy rekord w csm nie jest przeterminowany jeli tak to go usuwa
	void checkAnotherCsm(boost::interprocess::windows_shared_memory& checkedCsm, uint16_t processCsmId);

	//unload and free shared memory records and structures
	void unloadSharedMemory(uint16_t processCsmId);

	//execute before start creating MSMs
	void resetMsmParameters();


	////FUNCTIONS FOR FIREFOX

	//return true if any client (miranda instance) is available in csm for current firefox profile
	bool isAnyMirandaCsmRecordAvailable(std::wstring& forThisProfileName);

	//return number of records in clientInstanceNamesList, empty list as parameter
	int getClientInstances(boost::ptr_list<ClientInstanceClass> * clientInstancesListPtr, std::wstring& forThisProfileName);

	//return number of records in contactsListPtr, empty list as parameter
	int getContacts(boost::ptr_list<ContactClass> * contactsListPtr, unsigned short clientRecordNo);

	//return number of records in accountsListPtr, empty list as parameter
	int getAccounts(boost::ptr_list<AccountClass> * accountsListPtr, unsigned short clientRecordNo);

	////FUNCTIONS FOR FIREFOX - end




	const char* getCsmName();
	std::string getMsmName(uint16_t id, int currentNumber);
	std::size_t getCsmTotalSize();
	std::size_t getMsmTotalSize();



	//static method that returns only instance of SharedMemoryUtils
	static SharedMemoryUtils * getInstance()
	{
		//initialized always from one thread at a time
		if (m_pOnlyOneInstance == NULL) {
			if (m_pOnlyOneInstance == NULL) {
				m_pOnlyOneInstance = new SharedMemoryUtils();
			}
		}
		return m_pOnlyOneInstance;
	}


private:

	//holds one and only object of MySingleton
	static SharedMemoryUtils* m_pOnlyOneInstance;
	
	MFLogger* logger;


	//global variables
	boost::interprocess::windows_shared_memory* csm;
	boost::ptr_list<boost::interprocess::windows_shared_memory> msmList;
	


	bool isCsmInited(boost::interprocess::windows_shared_memory& csm);
	void initCsm(boost::interprocess::windows_shared_memory& csm);

	//return:
	//	0 - versions match,
	// -3 - existing csm version is too high
	// -4 - existing csm version is too low
	int checkCsmVersion(boost::interprocess::windows_shared_memory& csm);

	uint16_t allocateRecordInCsm(boost::interprocess::windows_shared_memory& csm, char type, std::wstring& displayName, std::wstring& visableTo, bool doCommitSM);

	time_t mfGetCurrentTimestamp();

	int addRecordToMsm(char type, uint64_t agcHandle, uint64_t accountHandle, uint64_t groupHandle, char status, std::wstring& value);

	int checkCsmRecord(boost::interprocess::windows_shared_memory& checkedCsm, uint16_t processCsmId, std::wstring& displayNameProfile);
	
	void freeCsmRecord(boost::interprocess::windows_shared_memory& csm, uint16_t recordNo);

	bool isTokenOnList(std::wstring& token, std::wstring& tokensList);

	//free own record in csm
	void unloadFromCSM(uint16_t processCsmId);

	//free msm instances
	void unloadMSMs();

	uint16_t processCsmId;
	int freeMsmRecordsCount;
	int nextMsmNumber;  //from 1
	
	
};


#endif //#ifndef _WSX22_IPC_SHAREDMEMORY


#ifndef _WSX22_IPC_MESSAGEQUEUEBYSM
#define _WSX22_IPC_MESSAGEQUEUEBYSM



//message queue by shared memory related definitions
#define MQCONST_MQSM_TEXT_SIZEC	2042	//number of wchar_t chars in message - now used sharedmemory mode

#define MQTHREAD_TICK_TIME		100		// 100ms - check exit every this time
#define MQTHREAD_FIRSTRUN_TIME	0		// MQTHREAD_FIRSTRUN_TIME * MQTHREAD_TICK_TIME = 0s  - time to first run of checking message queue in mq thread (after thread start)
#define MQTHREAD_NEXTRUN_TIME	1		// MQTHREAD_NEXTRUN_TIME * MQTHREAD_TICK_TIME = 0s  - time to next run of checking message queue in mq thread



#include "MirFoxCommons_logger.h"






/**
 * MessageQueue Utils
 *
 * based on Boost Shared Memory, beacouse boost message queue doesn't work between 32bit and 64bit processes (at boost 1.46)
 *
 * Singleton pattern based on
 * http://www.codeproject.com/KB/threads/SingletonThreadSafety.aspx
 */
class MessageQueueUtils
{

public:

	//constructor
	MessageQueueUtils();
	//destructor
	~MessageQueueUtils();



	std::string getMqName(uint16_t processId);

	//return 0 if success, >0 if error
	int createMessageQueue(std::string mqName);

	void unloadMessageQueue(uint16_t unloadedMQProcessId);

	//wchar_t*& - pointer by reference
	bool tryReceiveMessage (char& menuItemType, char& userActionType, char& userButton, uint64_t& targetHandle, wchar_t*& userActionSelection, size_t uasBuffCSize);

	void sendMessage(int clientRecordId, char menuItemType, char userActionType, char userButton, uint64_t targetHandle, std::wstring userActionSelection);

	uint16_t volatile unloadedMQProcessId;


	//static method that returns only instance of SharedMemoryUtils
	static MessageQueueUtils * getInstance()
	{
		//initialized always from one thread at a time
		if (m_pOnlyOneInstance == NULL) {
			if (m_pOnlyOneInstance == NULL) {
				m_pOnlyOneInstance = new MessageQueueUtils();
			}
		}
		return m_pOnlyOneInstance;
	}


private:

	boost::interprocess::windows_shared_memory* volatile mqMirSm;

	std::size_t getMqMirSmTotalSize();

	//CRITICAL_SECTION smCs;
	HANDLE smMutex;

	//holds one and only object of MySingleton
	static MessageQueueUtils * m_pOnlyOneInstance;

	MFLogger* logger;

};


#endif //#ifndef _WSX22_IPC_MESSAGEQUEUEBYSM

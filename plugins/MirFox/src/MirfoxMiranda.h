#ifndef _MIRFOXMIRANDA_INC
#define _MIRFOXMIRANDA_INC

#include "MirfoxData.h"
#include "MirandaUtils.h"


class CMirfoxMiranda;


struct OnContactAsyncThreadArgStruct {
	MCONTACT hContact;
	CMirfoxMiranda* mirfoxMiranda;
};


class CMirfoxMiranda
{

public:
	CMirfoxMiranda();
	~CMirfoxMiranda();


	//functions needed by MirandaInterface and Miranda Options

	MirfoxData& getMirfoxData();

	SharedMemoryUtils& getSharedMemoryUtils();

	int onMirandaInterfaceLoad();

	int onMirandaInterfaceUnload();

	//hooks support - to refresh data
	void onAccListChanged(WPARAM wParam, LPARAM lParam);
	static void onContactAdded_async(void* threadArg); //at async new thread
	void onContactDeleted(MCONTACT hContact);
	void onContactSettingChanged(MCONTACT hContact, LPARAM lParam);
	static void onContactSettingChanged_async(void* threadArg); //at async new thread


	//csm maintanance thread function (threadArg - pointer to this CMirfoxMiranda class instance)
	static void csmThread(void* threadArg);

	//message queue thread function (threadArg - pointer to this CMirfoxMiranda class instance)
	static void msgQueueThread(void* threadArg);



private:

	void initializeSharedMemory(MirfoxData& mirfoxData);
	void initializeSharedMemoryData(MirfoxData& mirfoxData, SharedMemoryUtils* sharedMemoryUtils);
	void commitSharedMemory();
	void unloadSharedMemory();

	void initializeMessageQueue(MirfoxData& mirfoxData);
	void unloadMessageQueue(uint16_t unloadedMQProcessId);



	MirfoxData mirfoxData;

	SharedMemoryUtils* sharedMemoryUtils;
	MirandaUtils* mirandaUtils;

	MFLogger* logger;

};



#endif //#IFNDEF _MIRFOXMIRANDA_INC

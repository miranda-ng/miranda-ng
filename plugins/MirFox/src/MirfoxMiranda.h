#ifndef _MIRFOXMIRANDA_INC
#define _MIRFOXMIRANDA_INC

#include "MirfoxData.h"
#include "MirandaUtils.h"



class CMirfoxMiranda
{

public:
	CMirfoxMiranda();
	~CMirfoxMiranda();


	//functions needed by MirandaInterface and Miranda Options

	MirfoxData& getMirfoxData();

	int onMirandaInterfaceLoad();

	int onMirandaInterfaceUnload();

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

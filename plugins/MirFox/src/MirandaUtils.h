#ifndef _MIRANDAUTILS_INC
#define _MIRANDAUTILS_INC

#include "MirfoxData.h"



class MirandaUtils;


struct ActionThreadArgStruct {
	wchar_t* userActionSelection;
	HANDLE targetHandle;
	char* accountSzModuleName;
	char userActionType;
	char menuItemType;
	char userButton;
	MirandaUtils* instancePtr;
	MirfoxData* mirfoxDataPtr;
};




class MirandaUtils
{

public:

	static MirandaUtils *
	getInstance()
	{
		if (m_pOnlyOneInstance == NULL) {
			if (m_pOnlyOneInstance == NULL) {
				m_pOnlyOneInstance = new MirandaUtils();
			}
		}
	return m_pOnlyOneInstance;
	}


	void netlibRegister(void);
	void netlibUnregister(void);
	void netlibLog_int(const wchar_t* szText);
	static void netlibLog(const wchar_t* szText);


	std::wstring& getProfileName();

	std::wstring& getDisplayName();

	static void userActionThread(void* threadArg);

	static int onProtoAck(WPARAM wParam, LPARAM lParam);

	void translateOldDBNames();

	struct OnHookOpenMvStruct {
		HANDLE targetHandle;
		std::wstring* msgBuffer;
	};

	static int on_hook_OpenMW(WPARAM wParam, LPARAM lParam);


private:


	static MirandaUtils * m_pOnlyOneInstance;

	// private constructor
	MirandaUtils();




	void sendMessageToContact(ActionThreadArgStruct* args);

	void sendMessage(ActionThreadArgStruct* args, MFENUM_SEND_MESSAGE_MODE mode);

	HANDLE sendMessageMiranda(MCONTACT hContact, char* msgBuffer);

	void addMessageToDB(MCONTACT hContact, char* msgBuffer, std::size_t bufSize, char* targetHandleSzProto);

	void setStatusOnAccount(ActionThreadArgStruct* args);

	void onProtoAckOnInstance(ACKDATA* ack);

	static void ForceForegroundWindow(HWND hWnd);


	HANDLE netlibHandle;

	std::wstring profileName;
	std::wstring displayName;



	typedef struct {
		const char* szModule;
		MCONTACT hContact;
		int result;
		const char* errorDesc;
	} MIRFOXACKDATA;

	std::map<HANDLE, MIRFOXACKDATA*> ackMap;
	std::map<HANDLE, MIRFOXACKDATA*>::iterator ackMapIt;
	CRITICAL_SECTION ackMapCs;


	MFLogger* logger;


};



#endif //#IFNDEF _MIRANDAUTILS_INC

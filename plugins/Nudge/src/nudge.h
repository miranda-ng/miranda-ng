#ifndef NUDGE_H
#define NUDGE_H

#define MODULENAME "Nudge"

//	NUDGE account status flags
#define NUDGE_ACC_ST0	0x00000001		//Check (countdown) when Offline
#define NUDGE_ACC_ST1	0x00000002		//Check (countdown) when Online
#define NUDGE_ACC_ST2	0x00000004		//Check (countdown) when Away
#define NUDGE_ACC_ST3	0x00000008		//Check (countdown) when Not available
#define NUDGE_ACC_ST4	0x00000010		//Check (countdown) when Occupied
#define NUDGE_ACC_ST5	0x00000020		//Check (countdown) when DND
#define NUDGE_ACC_ST6	0x00000040		//Check (countdown) when Free for chat
#define NUDGE_ACC_ST7	0x00000080		//Check (countdown) when Invisible

#define TEXT_LEN 1024

struct CNudge
{
	bool useByProtocol;
	int sendTimeSec;
	int recvTimeSec;
	int resendDelaySec;

	void Load(void);
	void Save(void);
};

struct CNudgeElement : public MZeroedObject
{
	char ProtocolName[64];
	wchar_t AccountName[128];
	char NudgeSoundname[100];
	wchar_t recText[TEXT_LEN];
	wchar_t senText[TEXT_LEN];
	bool showPopup;
	bool showStatus;
	bool openMessageWindow;
	bool openContactList;
	bool useIgnoreSettings;
	bool shakeClist;
	bool shakeChat;
	bool enabled;
	bool autoResend;
	uint32_t statusFlags;
	int iProtoNumber;
	HANDLE hEvent;

	void Load(void);
	void Save(void);
};

#endif // NUDGE_H

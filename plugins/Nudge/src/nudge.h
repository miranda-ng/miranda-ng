#ifndef NUDGE_H
#define NUDGE_H

#define MODULENAME "Nudge"

//	NUDGE account status flags
#define NUDGE_ACC_ST0	0x00000001		//Check (countdown) when Offline
#define NUDGE_ACC_ST1	0x00000002		//Check (countdown) when Online
#define NUDGE_ACC_ST2	0x00000004		//Check (countdown) when Away
#define NUDGE_ACC_ST3	0x00000008		//Check (countdown) when N/A
#define NUDGE_ACC_ST4	0x00000010		//Check (countdown) when Occupied
#define NUDGE_ACC_ST5	0x00000020		//Check (countdown) when DND
#define NUDGE_ACC_ST6	0x00000040		//Check (countdown) when Free for chat
#define NUDGE_ACC_ST7   0x00000080		//Check (countdown) when Invisible
#define NUDGE_ACC_ST8   0x00000100		//Check (countdown) when On the phone
#define NUDGE_ACC_ST9   0x00000200		//Check (countdown) when Out to lunch

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
	TCHAR AccountName[128];
	char NudgeSoundname[100];
	TCHAR recText[TEXT_LEN];
	TCHAR senText[TEXT_LEN];
	bool showPopup;
	bool showStatus;
	bool openMessageWindow;
	bool openContactList;
	bool useIgnoreSettings;
	bool shakeClist;
	bool shakeChat;
	bool enabled;
	bool autoResend;
	DWORD statusFlags;
	int iProtoNumber;
	HANDLE hEvent;

	void Load(void);
	void Save(void);
};

#endif // NUDGE_H

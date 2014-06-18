extern HINSTANCE hInst;

int getMsgId(char *msg);

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

#ifdef _DEBUG
	#define CHECKPOINT
#else
	#define CHECKPOINT error
#endif

#define TIMER_SEND	100

typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned long ulong;
enum
{
	CMD_REQ,
	CMD_ACCEPT,
	CMD_CANCEL,
	
	CMD_NEXT,
	
	CMD_DATA,
	CMD_DACK,
	CMD_END,
	
	CMD_COUNT
};

extern char cFECmd[CMD_COUNT];

#define STATE_IDLE		0x100 // idle, no operation
#define STATE_REQSENT	0x02 // request sent, awaiting of  response
#define STATE_PRERECV	0x04 // incoming request, awaiting of user
#define STATE_OPERATE	0x08 // operating mode
#define STATE_ACKREQ	0x10 // ACK-request scheduled
#define STATE_CANCELLED 0x20 // operation aborted
#define STATE_FINISHED	0x40 // ... finished successfully
#define STATE_PAUSED	0x80 // ... paused

#define CHUNK_UNSENT	0x00
#define CHUNK_SENT		0x01
#define CHUNK_ACK		0x02

#define ICON_PLAY		0
#define ICON_PAUSE		1
#define ICON_REFRESH	2
#define ICON_STOP		3
#define ICON_MAIN		4

struct FILEECHO
{
public:
	MCONTACT hContact;
	HWND hDlg;

	bool inSend;
	int iState;
	int contactStatus;

	HANDLE hFile, hMapping;
	uchar *lpData;

	char *filename;
	uint chunkIndx;			// next chunk to send
	uint chunkCount;		// count of chunks
	uint chunkSent;
	uint chunkCountx;
	uint *chunkPos;			// offsets of chunks in file
	uchar *chunkAck;		// acknowledge of incoming chunks

	uchar codeSymb;			// symb for replace NUL-symb.

	uint fileSize;
	uint chunkMaxLen;		// limit for outgoing chunk

	DWORD lastTimestamp;
	DWORD lastDelay;
	bool asBinary;

	// settings
	uint dwSendInterval;
	//uint dwChunkSize;

	COLORREF rgbSent, rgbRecv, rgbUnSent, rgbToSend;
#ifdef DEBUG
	uint overhead;
#endif

	FILEECHO(MCONTACT Contact);

	void setState(DWORD state);
	void updateProgress();
	void updateTitle();
	
	void perform(char *str);
	void cmdACCEPT();
	void cmdDACK(char *data);
	void sendReq();
	int  sendCmd(int id, int cmd, char *szParam, char *szPrefix = NULL);
	
	void cmdDATA(char *data);
	void cmdEND();

	int  createTransfer();
	void destroyTransfer();

	void onSendTimer();
	void onRecvTimer();
	void incomeRequest(char *data);
	
};

void InitCRC32();

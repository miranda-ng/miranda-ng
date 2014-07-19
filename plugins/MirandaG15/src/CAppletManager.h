#ifndef _COUTPUTMANAGER_H_
#define _COUTPUTMANAGER_H_

#include "CLCDOutputManager.h"

#include "CProtocolData.h"
#include "CIRCHistory.h"
#include "CIRCConnection.h"

#include "CNotificationScreen.h"
#include "CEventScreen.h"
#include "CContactlistScreen.h"
#include "CChatScreen.h"
#include "CCreditsScreen.h"
#include "CScreensaverScreen.h"

#include "CLCDConnectionLogitech.h"

struct SMessageJob
{
	HANDLE hEvent;
	MCONTACT hContact;
	DWORD dwFlags;
	char *pcBuffer;
	int iBufferSize;
	DWORD dwTimestamp;
};

class CAppletManager : public CLCDOutputManager
{
public:
	// returns the AppletManager's instance
	static CAppletManager *GetInstance();

	// Constructor
	CAppletManager();
	// Destructor
	~CAppletManager();

	// Initializes the AppletManager
	bool Initialize(tstring strAppletName);
	// Deinitializes the AppletManager
	bool Shutdown();
	// Updates the AppletManager
	bool Update();

	// the update timer's callback function
	static VOID CALLBACK UpdateTimer(HWND hwnd,UINT uMsg,UINT_PTR idEvent,DWORD dwTime);

	// called when the plugin's configuration has changed
	void OnConfigChanged();
	
	// called to process the specified event
	void HandleEvent(CEvent *pEvent);

	// screen activation functions
	void ActivatePreviousScreen();
	void ActivateScreensaverScreen();
	void ActivateCreditsScreen();
	void ActivateEventScreen();
	void ActivateCListScreen();
	bool ActivateChatScreen(MCONTACT hContact);

	// hook functions
	static int HookMessageWindowEvent(WPARAM wParam, LPARAM lParam);
	static int HookContactIsTyping(WPARAM wParam, LPARAM lParam);
	static int HookEventAdded(WPARAM wParam, LPARAM lParam); 
	static int HookStatusChanged(WPARAM wParam, LPARAM lParam);
	static int HookProtoAck(WPARAM wParam, LPARAM lParam);
	static int HookContactAdded(WPARAM wParam, LPARAM lParam);
	static int HookContactDeleted(WPARAM wParam, LPARAM lParam);
	static int HookSettingChanged(WPARAM wParam,LPARAM lParam);
	static int HookChatInbound(WPARAM wParam,LPARAM lParam);

	// check if a contacts message window is opened
	static bool IsMessageWindowOpen(MCONTACT hContact);
	// marks the given event as read
	static void MarkMessageAsRead(MCONTACT hContact,HANDLE hEvent);
	// translates the given database event
	static bool TranslateDBEvent(CEvent *pEvent,WPARAM wParam, LPARAM lParam);
	// sends a message to the specified contact
	static HANDLE SendMessageToContact(MCONTACT hContact,tstring strMessage);
	// sends typing notifications to the specified contact
	static void SendTypingNotification(MCONTACT hContact,bool bEnable);

	// returns the contacts message service name
	static char *GetMessageServiceName(MCONTACT hContact,bool bIsUnicode);
	static bool  IsUtfSendAvailable(MCONTACT hContact);
	// returns a formatted timestamp string
	static tstring GetFormattedTimestamp(tm *time);

	// returns the contacts group
	static tstring GetContactGroup(MCONTACT hContact);
	// returns the contacts displayname
	static tstring GetContactDisplayname(MCONTACT hContact,bool bShortened=false);

	// returns the history class for the specified IRC channel
	CIRCHistory *GetIRCHistory(MCONTACT hContact);
	CIRCHistory *GetIRCHistoryByName(tstring strProtocol,tstring strChannel);

	// returns the IRC connection class for the specified protocol
	CIRCConnection *GetIRCConnection(tstring strProtocol);
	// creates the IRC connection class for the specified protocol
	CIRCConnection *CreateIRCConnection(tstring strProtocol);

	// returns the bitmap for the specified status
	HBITMAP GetStatusBitmap(int iStatus);
	// returns the bitmap for the specified event
	HBITMAP GetEventBitmap(EventType eType, bool bLarge = false);

	// checks if the patched IRC protocol is in place
	bool IsIRCHookEnabled();

	static tstring TranslateString(TCHAR *szString,...);

private:
	list<CIRCHistory*> m_LIRCHistorys;
	// deletes the history class for the specified IRC channel
	void DeleteIRCHistory(MCONTACT hContact);
	// creates a history class for the specified IRC channel
	CIRCHistory *CreateIRCHistory(MCONTACT hContact,tstring strChannel);
	CIRCHistory *CreateIRCHistoryByName(tstring strProtocol,tstring strChannel);

	// activate a screen
	void ActivateScreen(CScreen *pScreen);

	// applies the volumewheel setting
	void SetVolumeWheelHook();

	// Called when the connection state has changed
	void OnConnectionChanged(int iConnectionState);
	// Called when the active screen has expired
	void OnScreenExpired(CLCDScreen *pScreen);
	
	// updates all pending message jobs
	void UpdateMessageJobs();
	// adds a message job to the list
	void AddMessageJob(SMessageJob *pJob);
	// finishes a message job
	void FinishMessageJob(SMessageJob *pJob);
	// cancels a message job
	void CancelMessageJob(SMessageJob *pJob);

	// removes a message job from the list

	// strip IRC formatting
	static tstring StripIRCFormatting(tstring strText);

	// Light status
	SG15LightStatus m_G15LightStatus;

	list<SMessageJob*> m_MessageJobs;

	// update timer handle
	UINT					m_uiTimer;

	// screens
	CNotificationScreen		m_NotificationScreen;
	CEventScreen			m_EventScreen;
	CContactlistScreen		m_ContactlistScreen;
	CChatScreen				m_ChatScreen;
	CCreditsScreen			m_CreditsScreen;
	CScreensaverScreen		m_ScreensaverScreen;

	// protocol data
	vector<CProtocolData*> m_vProtocolData;
	CProtocolData* GetProtocolData(tstring strProtocol);

	// hook handles
	HANDLE m_hMIHookMessageWindowEvent;
	HANDLE m_hMIHookContactIsTyping;
	HANDLE m_hMIHookEventAdded;	
	HANDLE m_hMIHookStatusChanged;	
	HANDLE m_hMIHookProtoAck;
	HANDLE m_hMIHookSettingChanged;
	HANDLE m_hMIHookContactDeleted;
	HANDLE m_hMIHookContactAdded;
	HANDLE m_hMIHookChatEvent;

	vector<CIRCConnection*> m_vIRCConnections;

	// last active screen
	CScreen *m_pLastScreen;
	
	bool m_bScreensaver;

	HBITMAP m_ahStatusBitmaps[8];
	HBITMAP m_ahEventBitmaps[4];
	HBITMAP m_ahLargeEventBitmaps[4];
};

#endif
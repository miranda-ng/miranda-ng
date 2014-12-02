/*
 * This code implements window handling (new mail)
 *
 * (c) majvan 2002-2004
 */
/* There can be problems when compiling this file, because in this file
 * we are using both unicode and no-unicode functions and compiler does not
 * like it in one file
 * When you got errors, try to comment the #define <stdio.h> and compile, then
 * put it back to uncommented and compile again :)
 */

#include "../yamn.h" 

#define	TIMER_FLASHING 0x09061979
#define MAILBROWSER_MINXSIZE	200		//min size of mail browser window
#define MAILBROWSER_MINYSIZE	130

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
char* s_MonthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
bool bDate = false,bSub=false,bSize=false,bFrom=false;
int PosX=0,PosY=0,SizeX=460,SizeY=100;
int HeadSizeX = 0x2b2, HeadSizeY = 0x0b5, HeadPosX = 100, HeadPosY = 100;
int HeadSplitPos=250; // per-mils of the size 
static int FromWidth=250,SubjectWidth=280,SizeWidth=50,SizeDate=205;
unsigned char optDateTime =  (SHOWDATELONG | SHOWDATENOTODAY);

struct CMailNumbersSub
{
	int Total;		//any mail
	int New;			//uses YAMN_MSG_NEW flag
	int UnSeen;			//uses YAMN_MSG_UNSEEN flag
//	int Browser;		//uses YAMN_MSG_BROWSER flag
	int BrowserUC;		//uses YAMN_MSG_BROWSER flag and YAMN_MSG_UNSEEN flag
	int Display;		//uses YAMN_MSG_DISPLAY flag
	int DisplayTC;		//uses YAMN_MSG_DISPLAY flag and YAMN_MSG_DISPLAYC flag
	int DisplayUC;		//uses YAMN_MSG_DISPLAY flag and YAMN_MSG_DISPLAYC flag and YAMN_MSG_UNSEEN flag
	int Popup;			//uses YAMN_MSG_POPUP flag
	int PopupTC;		//uses YAMN_MSG_POPUPC flag
	int PopupNC;		//uses YAMN_MSG_POPUPC flag and YAMN_MSG_NEW flag
	int PopupRun;		//uses YAMN_MSG_POPUP flag and YAMN_MSG_NEW flag
	int PopupSL2NC;		//uses YAMN_MSG_SPAML2 flag and YAMN_MSG_NEW flag
	int PopupSL3NC;		//uses YAMN_MSG_SPAML3 flag and YAMN_MSG_NEW flag
//	int SysTray;		//uses YAMN_MSG_SYSTRAY flag
	int SysTrayUC;		//uses YAMN_MSG_SYSTRAY flag and YAMN_MSG_UNSEEN flag
//	int Sound;		//uses YAMN_MSG_SOUND flag
	int SoundNC;		//uses YAMN_MSG_SOUND flag and YAMN_MSG_NEW flag
//	int App;		//uses YAMN_MSG_APP flag
	int AppNC;		//uses YAMN_MSG_APP flag and YAMN_MSG_NEW flag
	int EventNC;		//uses YAMN_MSG_NEVENT flag and YAMN_MSG_NEW flag
};

struct CMailNumbers
{
	struct CMailNumbersSub Real;
	struct CMailNumbersSub Virtual;
};

struct CMailWinUserInfo
{
	HACCOUNT Account;
	int TrayIconState;
	BOOL UpdateMailsMessagesAccess;
	BOOL Seen;
	BOOL RunFirstTime;
};

struct CChangeContent
{
	DWORD nflags;
	DWORD nnflags;
};

struct CUpdateMails
{
	struct CChangeContent *Flags;
	BOOL Waiting;
	HANDLE Copied;
};
struct CSortList
{
	HWND hDlg;
	int	iSubItem;
};

//Retrieves HACCOUNT, whose mails are displayed in ListMails
// hLM- handle of dialog window
// returns handle of account
inline HACCOUNT GetWindowAccount(HWND hDialog);

//Looks to mail flags and increment mail counter (e.g. if mail is new, increments the new mail counter
// msgq- mail, which increments the counters
// MN- counnters structure
void IncrementMailCounters(HYAMNMAIL msgq,struct CMailNumbers *MN);

enum
{
	UPDATE_FAIL=0,		//function failed
	UPDATE_NONE,		//none update has been performed
	UPDATE_OK,		//some changes occured, update performed
};
//Just looks for mail changes in account and update the mail browser window
// hDlg- dialog handle
// ActualAccount- account handle
// nflags- flags what to do when new mail arrives
// nnflags- flags what to do when no new mail arrives
// returns one of UPDATE_XXX value(not implemented yet)
int UpdateMails(HWND hDlg,HACCOUNT ActualAccount,DWORD nflags,DWORD nnflags);

//When new mail occurs, shows window, plays sound, runs application...
// hDlg- dialog handle. Dialog of mailbrowser is already created and actions are performed over this window
// ActualAccount- handle of account, whose mails are to be notified
// MN- statistics of mails in account
// nflags- what to do or not to do (e.g. to show mailbrowser window or prohibit to show)
// nflags- flags what to do when new mail arrives
// nnflags- flags what to do when no new mail arrives
void DoMailActions(HWND hDlg,HACCOUNT ActualAccount,struct CMailNumbers *MN,DWORD nflags,DWORD nnflags);

//Looks for items in mailbrowser and if they were deleted, delete them from browser window
// hListView- handle of listview window
// ActualAccount- handle of account, whose mails are show
// MailNumbers- pointer to structure, in which function stores numbers of mails with some property
// returns one of UPDATE_XXX value (not implemented yet)
int ChangeExistingMailStatus(HWND hListView,HACCOUNT ActualAccount,struct CMailNumbers *MN);

//Adds new mails to ListView and if any new, shows multi popup (every new message is new popup window created by popup plugin)
// hListView- handle of listview window
// ActualAccount- handle of account, whose mails are show
// NewMailPopup- pointer to prepared structure for popup plugin, can be NULL if no popup show
// MailNumbers- pointer to structure, in which function stores numbers of mails with some property
// nflags- flags what to do when new mail arrives
// returns one of UPDATE_XXX value (not implemented yet)
int AddNewMailsToListView(HWND hListView,HACCOUNT ActualAccount,struct CMailNumbers *MailNumbers,DWORD nflags);

//Window callback procedure for popup window (created by popup plugin)
LRESULT CALLBACK NewMailPopupProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

//Window callback procedure for popup window (created by popup plugin)
LRESULT CALLBACK NoNewMailPopupProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);

//Dialog callback procedure for mail browser
INT_PTR CALLBACK DlgProcYAMNMailBrowser(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam);

//MailBrowser thread function creates window if needed, tray icon and plays sound
DWORD WINAPI MailBrowser(LPVOID Param);

LRESULT CALLBACK ListViewSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

//Runs mail browser in new thread
INT_PTR RunMailBrowserSvc(WPARAM,LPARAM);

#define	YAMN_BROWSER_SHOWPOPUP	0x01

	//	list view items' order criteria
	#define LVORDER_NOORDER		-1
	#define LVORDER_STRING		 0
	#define LVORDER_NUMERIC		 1
	#define LVORDER_DATETIME	 2

	//	list view order direction
	#define LVORDER_ASCENDING	 1
	#define LVORDER_NONE		 0
	#define LVORDER_DESCENDING	-1

	//	list view sort type
	#define LVSORTPRIORITY_NONE -1

	//	List view column info.
	typedef struct _SAMPLELISTVIEWCOLUMN
	{
		UINT	 uCXCol;		//	index
		int		 nSortType;		//	sorting type (STRING = 0, NUMERIC, DATE, DATETIME)
		int		 nSortOrder;	//	sorting order (ASCENDING = -1, NONE, DESCENDING)
		int		 nPriority;		//	sort priority (-1 for none, 0, 1, ..., nColumns - 1 maximum)
		TCHAR lpszName[128];	//	column name
	} SAMPLELISTVIEWCOLUMN;

	//	Compare priority
	typedef struct _LVCOMPAREINFO
	{
		int	iIdx;				//	Index
		int iPriority;			//	Priority
	} LVCOMPAREINFO, *LPLVCOMPAREINFO;

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

LPARAM readItemLParam(HWND hwnd,DWORD iItem)
{
	LVITEM item;

	item.mask = LVIF_PARAM;
	item.iItem = iItem;
	item.iSubItem = 0;
	SendMessage(hwnd,LVM_GETITEM,0,(LPARAM)&item);
	return item.lParam;
}

inline HACCOUNT GetWindowAccount(HWND hDlg)
{
	struct CMailWinUserInfo *mwui;

	if (NULL==(mwui=(struct CMailWinUserInfo *)GetWindowLongPtr(hDlg,DWLP_USER)))
		return NULL;
	return mwui->Account;
}

void IncrementMailCounters(HYAMNMAIL msgq,struct CMailNumbers *MN)
{
	if (msgq->Flags & YAMN_MSG_VIRTUAL)
		MN->Virtual.Total++;
	else
		MN->Real.Total++;

	if (msgq->Flags & YAMN_MSG_NEW)
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.New++;
		else
			MN->Real.New++;
	if (msgq->Flags & YAMN_MSG_UNSEEN)
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.UnSeen++;
		else
			MN->Real.UnSeen++;
	if ((msgq->Flags & (YAMN_MSG_UNSEEN | YAMN_MSG_BROWSER)) == (YAMN_MSG_UNSEEN | YAMN_MSG_BROWSER))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.BrowserUC++;
		else
			MN->Real.BrowserUC++;
	if (msgq->Flags & YAMN_MSG_DISPLAY)
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.Display++;
		else
			MN->Real.Display++;
	if ((msgq->Flags & (YAMN_MSG_DISPLAYC | YAMN_MSG_DISPLAY)) == (YAMN_MSG_DISPLAYC | YAMN_MSG_DISPLAY))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.DisplayTC++;
		else
			MN->Real.DisplayTC++;
	if ((msgq->Flags & (YAMN_MSG_UNSEEN | YAMN_MSG_DISPLAYC | YAMN_MSG_DISPLAY)) == (YAMN_MSG_UNSEEN | YAMN_MSG_DISPLAYC | YAMN_MSG_DISPLAY))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.DisplayUC++;
		else
			MN->Real.DisplayUC++;
	if (msgq->Flags & YAMN_MSG_POPUP)
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.Popup++;
		else
			MN->Real.Popup++;
	if ((msgq->Flags & YAMN_MSG_POPUPC) == YAMN_MSG_POPUPC)
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.PopupTC++;
		else
			MN->Real.PopupTC++;
	if ((msgq->Flags & (YAMN_MSG_NEW | YAMN_MSG_POPUPC)) == (YAMN_MSG_NEW | YAMN_MSG_POPUPC))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.PopupNC++;
		else
			MN->Real.PopupNC++;
	if ((msgq->Flags & (YAMN_MSG_NEW | YAMN_MSG_POPUP)) == (YAMN_MSG_NEW | YAMN_MSG_POPUP))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.PopupRun++;
		else
			MN->Real.PopupRun++;
	if ((msgq->Flags & YAMN_MSG_NEW) && YAMN_MSG_SPAML(msgq->Flags,YAMN_MSG_SPAML2))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.PopupSL2NC++;
		else
			MN->Real.PopupSL2NC++;
	if ((msgq->Flags & YAMN_MSG_NEW) && YAMN_MSG_SPAML(msgq->Flags,YAMN_MSG_SPAML3))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.PopupSL3NC++;
		else
			MN->Real.PopupSL3NC++;
/*	if (msgq->MailData->Flags & YAMN_MSG_SYSTRAY)
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.SysTray++;
		else
			MN->Real.SysTray++;
*/	if ((msgq->Flags & (YAMN_MSG_UNSEEN | YAMN_MSG_SYSTRAY)) == (YAMN_MSG_UNSEEN|YAMN_MSG_SYSTRAY))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.SysTrayUC++;
		else
			MN->Real.SysTrayUC++;
/*	if (msgq->MailData->Flags & YAMN_MSG_SOUND)
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.Sound++;
		else
			MN->Real.Sound++;
*/	if ((msgq->Flags & (YAMN_MSG_NEW|YAMN_MSG_SOUND)) == (YAMN_MSG_NEW|YAMN_MSG_SOUND))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.SoundNC++;
		else
			MN->Real.SoundNC++;
/*	if (msgq->MailData->Flags & YAMN_MSG_APP)
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.App++;
		else
			MN->Real.App++;
*/	if ((msgq->Flags & (YAMN_MSG_NEW|YAMN_MSG_APP)) == (YAMN_MSG_NEW|YAMN_MSG_APP))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.AppNC++;
		else
			MN->Real.AppNC++;
	if ((msgq->Flags & (YAMN_MSG_NEW|YAMN_MSG_NEVENT)) == (YAMN_MSG_NEW|YAMN_MSG_NEVENT))
		if (msgq->Flags & YAMN_MSG_VIRTUAL)
			MN->Virtual.EventNC++;
		else
			MN->Real.EventNC++;
}

int UpdateMails(HWND hDlg,HACCOUNT ActualAccount,DWORD nflags,DWORD nnflags)
{
#define MAILBROWSERTITLE LPGEN("%s - %d new mail messages, %d total")

	struct CMailWinUserInfo *mwui;
	struct CMailNumbers MN;

	HYAMNMAIL msgq;
	BOOL Loaded;
	BOOL RunMailBrowser,RunPopups;

	mwui=(struct CMailWinUserInfo *)GetWindowLongPtr(hDlg,DWLP_USER);
	//now we ensure read access for account and write access for its mails
	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"UpdateMails:ActualAccountSO-read wait\n");
	#endif
	if (WAIT_OBJECT_0 != WaitToReadFcn(ActualAccount->AccountAccessSO))
	{
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"UpdateMails:ActualAccountSO-read wait failed\n");
		#endif
		PostMessage(hDlg,WM_DESTROY,0,0);

		return UPDATE_FAIL;
	}
	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"UpdateMails:ActualAccountSO-read enter\n");
	#endif

	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"UpdateMails:ActualAccountMsgsSO-write wait\n");
	#endif
	if (WAIT_OBJECT_0 != WaitToWriteFcn(ActualAccount->MessagesAccessSO))
	{
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"UpdateMails:ActualAccountMsgsSO-write wait failed\n");
		DebugLog(SynchroFile,"UpdateMails:ActualAccountSO-read done\n");
		#endif
		ReadDoneFcn(ActualAccount->AccountAccessSO);

		PostMessage(hDlg,WM_DESTROY,0,0);
		return UPDATE_FAIL;
	}
	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"UpdateMails:ActualAccountMsgsSO-write enter\n");
	#endif

	memset(&MN, 0, sizeof(MN));

	for (msgq=(HYAMNMAIL)ActualAccount->Mails;msgq != NULL;msgq=msgq->Next)
	{
		if (!LoadedMailData(msgq))				//check if mail is already in memory
		{
			Loaded=false;
			if (NULL==LoadMailData(msgq))			//if we could not load mail to memory, consider this mail deleted and do not display it
				continue;
		}
		else
			Loaded=true;

		IncrementMailCounters(msgq,&MN);

		if (!Loaded)
			UnloadMailData(msgq);			//do not keep data for mail in memory
	}

	if (mwui != NULL)
		mwui->UpdateMailsMessagesAccess=TRUE;

	//Now we are going to check if extracting data from mail headers are needed.
	//If popups will be displayed or mailbrowser window
	if	((((mwui != NULL) && !(mwui->RunFirstTime)) &&
		  (
		   ((nnflags & YAMN_ACC_MSGP) && !(MN.Real.BrowserUC+MN.Virtual.BrowserUC)) ||
		   ((nflags & YAMN_ACC_MSGP) && (MN.Real.BrowserUC+MN.Virtual.BrowserUC))
		  )
		 ) ||		//if mail window was displayed before and flag YAMN_ACC_MSGP is set
		 ((nnflags & YAMN_ACC_MSG) && !(MN.Real.BrowserUC+MN.Virtual.BrowserUC)) ||		//if needed to run mailbrowser when no unseen and no unseen mail found
		 ((nflags & YAMN_ACC_MSG) && (MN.Real.BrowserUC+MN.Virtual.BrowserUC)) ||		//if unseen mails found, we sure run mailbrowser
		 ((nflags & YAMN_ACC_ICO) && (MN.Real.SysTrayUC+MN.Virtual.SysTrayUC))
		)			//if needed to run systray
			RunMailBrowser=TRUE;
	else	RunMailBrowser=FALSE;

	if ( (nflags & YAMN_ACC_POP) && 
		(ActualAccount->Flags & YAMN_ACC_POPN) &&
		(MN.Real.PopupNC+MN.Virtual.PopupNC))		//if some popups with mails are needed to show
			RunPopups=TRUE;
	else	RunPopups=FALSE;

	if (RunMailBrowser)
		ChangeExistingMailStatus(GetDlgItem(hDlg,IDC_LISTMAILS),ActualAccount,&MN);
	if (RunMailBrowser || RunPopups)
		AddNewMailsToListView(hDlg==NULL ? NULL : GetDlgItem(hDlg,IDC_LISTMAILS),ActualAccount,&MN,nflags);

	if (RunMailBrowser)
	{
		size_t len = strlen(ActualAccount->Name)+strlen(Translate(MAILBROWSERTITLE))+10;	//+10 chars for numbers
		char *TitleStrA=new char[len];
		WCHAR *TitleStrW=new WCHAR[len];

		mir_snprintf(TitleStrA, len, Translate(MAILBROWSERTITLE), ActualAccount->Name, MN.Real.DisplayUC + MN.Virtual.DisplayUC, MN.Real.Display + MN.Virtual.Display);
		MultiByteToWideChar(CP_ACP,MB_USEGLYPHCHARS,TitleStrA,-1,TitleStrW,(int)strlen(TitleStrA)+1);
		SendMessageW(hDlg,WM_SETTEXT,0,(LPARAM)TitleStrW);
		delete[] TitleStrA;
		delete[] TitleStrW;
	}

	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"UpdateMails:Do mail actions\n");
	#endif

	DoMailActions(hDlg,ActualAccount,&MN,nflags,nnflags);

	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"UpdateMails:Do mail actions done\n");
	#endif
	
	SetRemoveFlagsInQueueFcn((HYAMNMAIL)ActualAccount->Mails,YAMN_MSG_NEW,0,YAMN_MSG_NEW,YAMN_FLAG_REMOVE);				//rempve the new flag
	if (!RunMailBrowser)
		SetRemoveFlagsInQueueFcn((HYAMNMAIL)ActualAccount->Mails,YAMN_MSG_UNSEEN,YAMN_MSG_STAYUNSEEN,YAMN_MSG_UNSEEN,YAMN_FLAG_REMOVE);	//remove the unseen flag when it was not displayed and it has not "stay unseen" flag set

	if (mwui != NULL)
	{
		mwui->UpdateMailsMessagesAccess=FALSE;
		mwui->RunFirstTime=FALSE;
	}
	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"UpdateMails:ActualAccountMsgsSO-write done\n");
	DebugLog(SynchroFile,"UpdateMails:ActualAccountSO-read done\n");
	#endif
	WriteDoneFcn(ActualAccount->MessagesAccessSO);
	ReadDoneFcn(ActualAccount->AccountAccessSO);

	if (RunMailBrowser)
		UpdateWindow(GetDlgItem(hDlg,IDC_LISTMAILS));
	else if (hDlg != NULL)
		DestroyWindow(hDlg);

	return 1;
}

int ChangeExistingMailStatus(HWND hListView,HACCOUNT ActualAccount,struct CMailNumbers *MN)
{
	int i,in;
	LVITEMW item;
	HYAMNMAIL mail,msgq;

	in=ListView_GetItemCount(hListView);
	item.mask=LVIF_PARAM;

	for (i=0;i<in;i++)
	{
		item.iItem=i;
		item.iSubItem=0;
		if (TRUE==ListView_GetItem(hListView,&item))
			mail=(HYAMNMAIL)item.lParam;
		else
			continue;
		for (msgq=(HYAMNMAIL)ActualAccount->Mails;(msgq != NULL)&&(msgq != mail);msgq=msgq->Next);	//found the same mail in account queue
		if (msgq==NULL)		//if mail was not found
			if (TRUE==ListView_DeleteItem(hListView,i))
			{
				in--;i--;
				continue;
			}
	}

	return TRUE;
}

void MimeDateToLocalizedDateTime(char *datein, WCHAR *dateout, int lendateout);
int AddNewMailsToListView(HWND hListView,HACCOUNT ActualAccount,struct CMailNumbers *MN,DWORD nflags)
{
	HYAMNMAIL msgq;
	POPUPDATAT NewMailPopup = {0};

	WCHAR *FromStr;
	WCHAR SizeStr[20];
	WCHAR LocalDateStr[128];

	LVITEMW item;
	LVFINDINFO fi; 

	int foundi,lfoundi;
	struct CHeader UnicodeHeader;
	BOOL Loaded,Extracted,FromStrNew=FALSE;

	memset(&item, 0, sizeof(item));
	memset(&UnicodeHeader, 0, sizeof(UnicodeHeader));

	if (hListView != NULL)
	{
		item.mask=LVIF_TEXT | LVIF_PARAM;
		item.iItem=0;
		memset(&fi, 0, sizeof(fi));
		fi.flags=LVFI_PARAM;						//let's go search item by lParam number
		lfoundi=0;
	}

	NewMailPopup.lchContact=(ActualAccount->hContact != NULL) ? ActualAccount->hContact : (MCONTACT)ActualAccount;
	NewMailPopup.lchIcon=g_LoadIconEx(2);
	NewMailPopup.colorBack=nflags & YAMN_ACC_POPC ? ActualAccount->NewMailN.PopupB : GetSysColor(COLOR_BTNFACE);
	NewMailPopup.colorText=nflags & YAMN_ACC_POPC ? ActualAccount->NewMailN.PopupT : GetSysColor(COLOR_WINDOWTEXT);
	NewMailPopup.iSeconds=ActualAccount->NewMailN.PopupTime;

	NewMailPopup.PluginWindowProc=NewMailPopupProc;
	NewMailPopup.PluginData=(void *)0;					//it's new mail popup

	for (msgq=(HYAMNMAIL)ActualAccount->Mails;msgq != NULL;msgq=msgq->Next,lfoundi++)
	{
//		now we hide mail pointer to item's lParam member. We can later use it to retrieve mail datas

		Extracted=FALSE;FromStr=NULL;FromStrNew=FALSE;

		if (hListView != NULL)
		{
			fi.lParam=(LPARAM)msgq;
			if (-1 != (foundi=ListView_FindItem(hListView,-1,&fi)))	//if mail is already in window
			{
				lfoundi=foundi;
				continue;					//do not insert any item
			}

			item.iItem=lfoundi;			//insert after last found item
			item.lParam=(LPARAM)msgq;
		}

		if (!LoadedMailData(msgq))				//check if mail is already in memory
		{
			Loaded=false;
			if (NULL==LoadMailData(msgq))			//if we could not load mail to memory, consider this mail deleted and do not display it
				continue;
		}
		else
			Loaded=true;

		if (((hListView != NULL) && (msgq->Flags & YAMN_MSG_DISPLAY)) ||
			((nflags & YAMN_ACC_POP) && (ActualAccount->Flags & YAMN_ACC_POPN) && (msgq->Flags & YAMN_MSG_POPUP) && (msgq->Flags & YAMN_MSG_NEW)))
		{

			if (!Extracted) ExtractHeader(msgq->MailData->TranslatedHeader,msgq->MailData->CP,&UnicodeHeader);
			Extracted=TRUE;

			if ((UnicodeHeader.From != NULL) && (UnicodeHeader.FromNick != NULL)) {
				size_t size = wcslen(UnicodeHeader.From) + wcslen(UnicodeHeader.FromNick) + 4;
				FromStr = new WCHAR[size];
				mir_sntprintf(FromStr, size, L"%s <%s>", UnicodeHeader.FromNick, UnicodeHeader.From);
				FromStrNew = TRUE;
			}
			else if (UnicodeHeader.From != NULL)
				FromStr=UnicodeHeader.From;
			else if (UnicodeHeader.FromNick != NULL)
				FromStr=UnicodeHeader.FromNick;
			else if (UnicodeHeader.ReturnPath != NULL)
				FromStr=UnicodeHeader.ReturnPath;

			if (NULL==FromStr) {
				FromStr = L"";
				FromStrNew = FALSE;
			}
		}

		if ((hListView != NULL) && (msgq->Flags & YAMN_MSG_DISPLAY)) {
			item.iSubItem = 0;
			item.pszText = FromStr;
			item.iItem = SendMessageW(hListView,LVM_INSERTITEMW,0,(LPARAM)&item);

			item.iSubItem = 1;
			item.pszText = (NULL != UnicodeHeader.Subject ? UnicodeHeader.Subject : (WCHAR*)L"");
			SendMessageW(hListView,LVM_SETITEMTEXTW,(WPARAM)item.iItem,(LPARAM)&item);

			item.iSubItem = 2;
			mir_sntprintf(SizeStr, SIZEOF(SizeStr), L"%d kB", msgq->MailData->Size / 1024);
			item.pszText = SizeStr;
			SendMessageW(hListView,LVM_SETITEMTEXTW,(WPARAM)item.iItem,(LPARAM)&item);

			item.iSubItem = 3;
			item.pszText = L"";
			
			for (CMimeItem *heads = msgq->MailData->TranslatedHeader; heads != NULL; heads = heads->Next)	{
				if ( !_stricmp(heads->name, "Date")) { 
					MimeDateToLocalizedDateTime(heads->value, LocalDateStr, 128);
					item.pszText = LocalDateStr;
					break;
				}
			}
			SendMessageW(hListView,LVM_SETITEMTEXTW,(WPARAM)item.iItem,(LPARAM)&item);
		}

		if ((nflags & YAMN_ACC_POP) && (ActualAccount->Flags & YAMN_ACC_POPN) && (msgq->Flags & YAMN_MSG_POPUP) && (msgq->Flags & YAMN_MSG_NEW)) {
			mir_tstrncpy(NewMailPopup.lptzContactName, FromStr, SIZEOF(NewMailPopup.lptzContactName));
			mir_tstrncpy(NewMailPopup.lptzText, UnicodeHeader.Subject, SIZEOF(NewMailPopup.lptzText));

			PYAMN_MAILSHOWPARAM MailParam = (PYAMN_MAILSHOWPARAM)malloc(sizeof(YAMN_MAILSHOWPARAM));
			if (MailParam) {
				MailParam->account = ActualAccount;
				MailParam->mail = msgq;
				MailParam->ThreadRunningEV = 0;
				NewMailPopup.PluginData = MailParam;
				PUAddPopupT(&NewMailPopup);
			}
		}

		if ((msgq->Flags & YAMN_MSG_UNSEEN) && (ActualAccount->NewMailN.Flags & YAMN_ACC_KBN))
			CallService(MS_KBDNOTIFY_EVENTSOPENED,1,NULL);
		
		if (FromStrNew)
			delete[] FromStr;

		if (Extracted)
		{
			DeleteHeaderContent(&UnicodeHeader);
			memset(&UnicodeHeader, 0, sizeof(UnicodeHeader));
		}

		if (!Loaded)
		{
			SaveMailData(msgq);
			UnloadMailData(msgq);			//do not keep data for mail in memory
		}
	}

	return TRUE;
}

void DoMailActions(HWND hDlg,HACCOUNT ActualAccount,struct CMailNumbers *MN,DWORD nflags,DWORD nnflags)
{
	char *NotIconText = Translate("- new mail message(s)");
	NOTIFYICONDATA nid;

	memset(&nid, 0, sizeof(nid));

	if (MN->Real.EventNC+MN->Virtual.EventNC)
		NotifyEventHooks(hNewMailHook,0,0);

	if ((nflags & YAMN_ACC_KBN) && (MN->Real.PopupRun+MN->Virtual.PopupRun))
	{
		CallService(MS_KBDNOTIFY_STARTBLINK,(WPARAM)MN->Real.PopupNC+MN->Virtual.PopupNC,NULL);
	}

	if ((nflags & YAMN_ACC_CONT) && (MN->Real.PopupRun+MN->Virtual.PopupRun))
	{
		char sMsg[250];
		mir_snprintf(sMsg, 249, Translate("%s : %d new mail message(s), %d total"), ActualAccount->Name, MN->Real.PopupNC + MN->Virtual.PopupNC, MN->Real.PopupTC + MN->Virtual.PopupTC);
		if (!(nflags & YAMN_ACC_CONTNOEVENT)) {
			CLISTEVENT cEvent;
			cEvent.cbSize = sizeof(CLISTEVENT);
			cEvent.hContact = ActualAccount->hContact;
			cEvent.hIcon = g_LoadIconEx(2);
			cEvent.hDbEvent = (HANDLE)ActualAccount->hContact;
			cEvent.lParam = (LPARAM) ActualAccount->hContact;
			cEvent.pszService = MS_YAMN_CLISTDBLCLICK;
			cEvent.pszTooltip = sMsg;
			cEvent.flags = 0;
			CallServiceSync(MS_CLIST_ADDEVENT, 0,(LPARAM)&cEvent);
		}
		db_set_s(ActualAccount->hContact, "CList", "StatusMsg", sMsg);
		
		if (nflags & YAMN_ACC_CONTNICK)
		{
			db_set_s(ActualAccount->hContact, YAMN_DBMODULE, "Nick",sMsg);
		}
	}

	if ((nflags & YAMN_ACC_POP) && 
		!(ActualAccount->Flags & YAMN_ACC_POPN) && 
		(MN->Real.PopupRun+MN->Virtual.PopupRun))
	{
		POPUPDATAT NewMailPopup ={0};

		NewMailPopup.lchContact=(ActualAccount->hContact != NULL) ? ActualAccount->hContact : (MCONTACT)ActualAccount;
		NewMailPopup.lchIcon=g_LoadIconEx(2);
		NewMailPopup.colorBack=nflags & YAMN_ACC_POPC ? ActualAccount->NewMailN.PopupB : GetSysColor(COLOR_BTNFACE);
		NewMailPopup.colorText=nflags & YAMN_ACC_POPC ? ActualAccount->NewMailN.PopupT : GetSysColor(COLOR_WINDOWTEXT);
		NewMailPopup.iSeconds=ActualAccount->NewMailN.PopupTime;

		NewMailPopup.PluginWindowProc = NewMailPopupProc;
		NewMailPopup.PluginData = (void *)0;	//multiple popups

		mir_tstrncpy(NewMailPopup.lptzContactName, _A2T(ActualAccount->Name), SIZEOF(NewMailPopup.lptzContactName));
		mir_sntprintf(NewMailPopup.lptzText, SIZEOF(NewMailPopup.lptzText), TranslateT("%d new mail message(s), %d total"), MN->Real.PopupNC + MN->Virtual.PopupNC, MN->Real.PopupTC + MN->Virtual.PopupTC);
		PUAddPopupT(&NewMailPopup);
	}

	//destroy tray icon if no new mail
	if ((MN->Real.SysTrayUC+MN->Virtual.SysTrayUC==0) && (hDlg != NULL))
	{
		nid.hWnd=hDlg;
		nid.uID=0;
		Shell_NotifyIcon(NIM_DELETE,&nid);
	}

	//and remove the event
	if ((nflags & YAMN_ACC_CONT) && (!(nflags & YAMN_ACC_CONTNOEVENT)) && (MN->Real.UnSeen + MN->Virtual.UnSeen==0)) {
		CallService(MS_CLIST_REMOVEEVENT,(WPARAM)ActualAccount->hContact,(LPARAM)ActualAccount->hContact);
	}

	if ((MN->Real.BrowserUC+MN->Virtual.BrowserUC==0) && (hDlg != NULL))
	{
		if (!IsWindowVisible(hDlg) && !(nflags & YAMN_ACC_MSG))
			PostMessage(hDlg,WM_DESTROY,0,0);				//destroy window if no new mail and window is not visible
		if (nnflags & YAMN_ACC_MSG)											//if no new mail and msg should be executed
		{
			SetForegroundWindow(hDlg);
			ShowWindow(hDlg,SW_SHOWNORMAL);
		}
	}
	else 
		if (hDlg != NULL)								//else insert icon and set window if new mails
		{
			SendMessageW(GetDlgItem(hDlg,IDC_LISTMAILS),LVM_SCROLL,0,(LPARAM)0x7ffffff);

			if ((nflags & YAMN_ACC_ICO) && (MN->Real.SysTrayUC+MN->Virtual.SysTrayUC))
			{
				char* src;
				TCHAR *dest;
				int i;

				for (src=ActualAccount->Name,dest=nid.szTip,i=0;(*src != (TCHAR)0) && (i+1<sizeof(nid.szTip));*dest++=*src++);
				for (src=NotIconText;(*src != (TCHAR)0) && (i+1<sizeof(nid.szTip));*dest++=*src++);
				*dest=(TCHAR)0;
				nid.cbSize=sizeof(NOTIFYICONDATA);
				nid.hWnd=hDlg;
				nid.hIcon=g_LoadIconEx(2);
				nid.uID=0;
				nid.uFlags=NIF_ICON | NIF_MESSAGE | NIF_TIP;
				nid.uCallbackMessage=WM_YAMN_NOTIFYICON;
				Shell_NotifyIcon(NIM_ADD,&nid);
				SetTimer(hDlg,TIMER_FLASHING,500,NULL);
			}
			if (nflags & YAMN_ACC_MSG)											//if no new mail and msg should be executed
				ShowWindow(hDlg,SW_SHOWNORMAL);
		}

	if (MN->Real.AppNC+MN->Virtual.AppNC != 0)
	{
		if (nflags & YAMN_ACC_APP)
		{
			PROCESS_INFORMATION pi;
			STARTUPINFOW si;
			memset(&si, 0, sizeof(si));
			si.cb=sizeof(si);

			if (ActualAccount->NewMailN.App != NULL)
			{
				WCHAR *Command;
				if (ActualAccount->NewMailN.AppParam != NULL)
					Command=new WCHAR[wcslen(ActualAccount->NewMailN.App)+wcslen(ActualAccount->NewMailN.AppParam)+6];
				else
					Command=new WCHAR[wcslen(ActualAccount->NewMailN.App)+6];
		
				if (Command != NULL)
				{
					mir_wstrcpy(Command,L"\"");
					mir_wstrcat(Command,ActualAccount->NewMailN.App);
					mir_wstrcat(Command,L"\" ");
					if (ActualAccount->NewMailN.AppParam != NULL)
						mir_wstrcat(Command,ActualAccount->NewMailN.AppParam);
					CreateProcessW(NULL,Command,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
					delete[] Command;
				}
			}
		}
	}

	if (MN->Real.SoundNC+MN->Virtual.SoundNC != 0)
		if (nflags & YAMN_ACC_SND)
			CallService(MS_SKIN_PLAYSOUND,0,(LPARAM)YAMN_NEWMAILSOUND);

	if ((nnflags & YAMN_ACC_POP) && (MN->Real.PopupRun+MN->Virtual.PopupRun==0))
	{
		POPUPDATAT NoNewMailPopup;

		NoNewMailPopup.lchContact=(ActualAccount->hContact != NULL) ? ActualAccount->hContact : (MCONTACT)ActualAccount;
		NoNewMailPopup.lchIcon=g_LoadIconEx(1);
		NoNewMailPopup.colorBack=ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC ? ActualAccount->NoNewMailN.PopupB : GetSysColor(COLOR_BTNFACE);
		NoNewMailPopup.colorText=ActualAccount->NoNewMailN.Flags & YAMN_ACC_POPC ? ActualAccount->NoNewMailN.PopupT : GetSysColor(COLOR_WINDOWTEXT);
		NoNewMailPopup.iSeconds=ActualAccount->NoNewMailN.PopupTime;

		NoNewMailPopup.PluginWindowProc=NoNewMailPopupProc;
		NoNewMailPopup.PluginData=(void *)0;					//it's not new mail popup

		mir_tstrncpy(NoNewMailPopup.lptzContactName,_A2T(ActualAccount->Name),SIZEOF(NoNewMailPopup.lptzContactName));
		if (MN->Real.PopupSL2NC+MN->Virtual.PopupSL2NC)
			mir_sntprintf(NoNewMailPopup.lptzText, SIZEOF(NoNewMailPopup.lptzText), TranslateT("No new mail message, %d spam(s)"), MN->Real.PopupSL2NC + MN->Virtual.PopupSL2NC);
		else
			mir_tstrncpy(NoNewMailPopup.lptzText,TranslateT("No new mail message"),SIZEOF(NoNewMailPopup.lptzText));
		PUAddPopupT(&NoNewMailPopup);
	}

	if ((nflags & YAMN_ACC_CONT) && (MN->Real.PopupRun+MN->Virtual.PopupRun==0))
	{
		if (ActualAccount->hContact != NULL)
		{
			if (MN->Real.PopupTC+MN->Virtual.PopupTC)
			{
				char tmp[255];
				mir_snprintf(tmp, SIZEOF(tmp), Translate("%d new mail message(s), %d total"), MN->Real.PopupNC + MN->Virtual.PopupNC, MN->Real.PopupTC + MN->Virtual.PopupTC);
				db_set_s(ActualAccount->hContact, "CList", "StatusMsg", tmp);
			}
			else db_set_s(ActualAccount->hContact, "CList", "StatusMsg", Translate("No new mail message"));

			if (nflags & YAMN_ACC_CONTNICK)
				db_set_s(ActualAccount->hContact, YAMN_DBMODULE, "Nick", ActualAccount->Name);
		}
	}
	return;
}

DWORD WINAPI ShowEmailThread(LPVOID Param);
LRESULT CALLBACK NewMailPopupProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	INT_PTR PluginParam=0;
	switch(msg)
	{
		case WM_COMMAND:
			//if clicked and it's new mail popup window
			if ((HIWORD(wParam)==STN_CLICKED) && (-1 != (PluginParam=CallService(MS_POPUP_GETPLUGINDATA,(WPARAM)hWnd,(LPARAM)&PluginParam))))
			{
				MCONTACT hContact = 0;
				HACCOUNT Account;
				if (PluginParam) {
					PYAMN_MAILSHOWPARAM MailParam = new YAMN_MAILSHOWPARAM;
					memcpy(MailParam,(PINT_PTR)PluginParam,sizeof(YAMN_MAILSHOWPARAM));
					hContact = MailParam->account->hContact;
					Account = MailParam->account;
					if (NULL != (MailParam->ThreadRunningEV=CreateEvent(NULL,FALSE,FALSE,NULL))) {
						HANDLE NewThread;
						if (NULL != (NewThread=CreateThread(NULL,0,ShowEmailThread,(LPVOID)MailParam,0,NULL)))
						{
							CloseHandle(NewThread);
						}
						CloseHandle(MailParam->ThreadRunningEV);
					}
					//delete MailParam;
				} else {
					DBVARIANT dbv;

					hContact=(MCONTACT)CallService(MS_POPUP_GETCONTACT,(WPARAM)hWnd,0);

					if (!db_get((MCONTACT)hContact,YAMN_DBMODULE,"Id",&dbv)) 
					{
						Account=(HACCOUNT) CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)dbv.pszVal);
						db_free(&dbv);
					}
					else
						Account = (HACCOUNT)hContact; //????


					#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"PopupProc:LEFTCLICK:ActualAccountSO-read wait\n");
					#endif
					if (WAIT_OBJECT_0==WaitToReadFcn(Account->AccountAccessSO))
					{
						#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"PopupProc:LEFTCLICK:ActualAccountSO-read enter\n");
						#endif
						switch(msg)
						{
							case WM_COMMAND:
							{
								YAMN_MAILBROWSERPARAM Param={(HANDLE)0,Account,
									(Account->NewMailN.Flags & ~YAMN_ACC_POP) | YAMN_ACC_MSGP | YAMN_ACC_MSG,
									(Account->NoNewMailN.Flags & ~YAMN_ACC_POP) | YAMN_ACC_MSGP | YAMN_ACC_MSG};

								RunMailBrowserSvc((WPARAM)&Param,(LPARAM)YAMN_MAILBROWSERVERSION);
							}
							break;
						}
						#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"PopupProc:LEFTCLICK:ActualAccountSO-read done\n");
						#endif
						ReadDoneFcn(Account->AccountAccessSO);
					}
					#ifdef DEBUG_SYNCHRO
					else
						DebugLog(SynchroFile,"PopupProc:LEFTCLICK:ActualAccountSO-read enter failed\n");
					#endif
				}
				if ((Account->NewMailN.Flags & YAMN_ACC_CONT) && !(Account->NewMailN.Flags & YAMN_ACC_CONTNOEVENT)) {
					CallService(MS_CLIST_REMOVEEVENT, hContact, hContact);
				}
			}
			// fall through
		case WM_CONTEXTMENU:
			SendMessageW(hWnd,UM_DESTROYPOPUP,0,0);
			break;			
		case UM_FREEPLUGINDATA:{
				PYAMN_MAILSHOWPARAM mpd = (PYAMN_MAILSHOWPARAM)PUGetPluginData(hWnd);
				MCONTACT hContact = 0;
				if ((mpd) && (INT_PTR)mpd != -1)free(mpd);
				return FALSE;
			}
		case UM_INITPOPUP:
			//This is the equivalent to WM_INITDIALOG you'd get if you were the maker of dialog popups.
			WindowList_Add(YAMNVar.MessageWnds,hWnd,NULL);
			break;
		case UM_DESTROYPOPUP:
			WindowList_Remove(YAMNVar.MessageWnds,hWnd);
			break;
		case WM_YAMN_STOPACCOUNT:
		{
			HACCOUNT ActualAccount;
			MCONTACT hContact;
			DBVARIANT dbv;

			hContact=(MCONTACT)CallService(MS_POPUP_GETCONTACT,(WPARAM)hWnd,0);

			if (!db_get((MCONTACT) hContact,YAMN_DBMODULE,"Id",&dbv)) 
			{
				ActualAccount=(HACCOUNT) CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)dbv.pszVal);
				db_free(&dbv);
			}
			else
				ActualAccount = (HACCOUNT) hContact;

			if ((HACCOUNT)wParam != ActualAccount)
				break;
			DestroyWindow(hWnd);
			return 0;
		}
		case WM_NOTIFY:
		default:
			break;
	}
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

LRESULT CALLBACK NoNewMailPopupProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam) 
{
	switch(msg)
	{
		case WM_COMMAND:
			if ((HIWORD(wParam)==STN_CLICKED) && (msg==WM_COMMAND))
			{
				HACCOUNT ActualAccount;
				MCONTACT hContact;
				DBVARIANT dbv;

				hContact=(MCONTACT)CallService(MS_POPUP_GETCONTACT,(WPARAM)hWnd,0);

				if (!db_get((MCONTACT)hContact,YAMN_DBMODULE,"Id",&dbv)) 
				{
					ActualAccount=(HACCOUNT) CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)dbv.pszVal);
					db_free(&dbv);
				}
				else
					ActualAccount = (HACCOUNT) hContact;

				#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"PopupProc:LEFTCLICK:ActualAccountSO-read wait\n");
				#endif
				if (WAIT_OBJECT_0==WaitToReadFcn(ActualAccount->AccountAccessSO))
				{
					#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"PopupProc:LEFTCLICK:ActualAccountSO-read enter\n");
					#endif
					switch(msg)
					{
						case WM_COMMAND:
						{
							YAMN_MAILBROWSERPARAM Param={(HANDLE)0,ActualAccount,ActualAccount->NewMailN.Flags,ActualAccount->NoNewMailN.Flags,0};

							Param.nnflags=Param.nnflags | YAMN_ACC_MSG;			//show mails in account even no new mail in account
							Param.nnflags=Param.nnflags & ~YAMN_ACC_POP;
							
							Param.nflags=Param.nflags | YAMN_ACC_MSG;			//show mails in account even no new mail in account
							Param.nflags=Param.nflags & ~YAMN_ACC_POP;

							RunMailBrowserSvc((WPARAM)&Param,(LPARAM)YAMN_MAILBROWSERVERSION);
						}
						break;
					}
					#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"PopupProc:LEFTCLICK:ActualAccountSO-read done\n");
					#endif
					ReadDoneFcn(ActualAccount->AccountAccessSO);
				}
				#ifdef DEBUG_SYNCHRO
				else
					DebugLog(SynchroFile,"PopupProc:LEFTCLICK:ActualAccountSO-read enter failed\n");
				#endif
				SendMessageW(hWnd,UM_DESTROYPOPUP,0,0);
			}
			break;
	
		case WM_CONTEXTMENU:
			SendMessageW(hWnd,UM_DESTROYPOPUP,0,0);
			break;

		case UM_FREEPLUGINDATA:
			//Here we'd free our own data, if we had it.
			return FALSE;
		case UM_INITPOPUP:
			//This is the equivalent to WM_INITDIALOG you'd get if you were the maker of dialog popups.
			WindowList_Add(YAMNVar.MessageWnds,hWnd,NULL);
			break;
		case UM_DESTROYPOPUP:
			WindowList_Remove(YAMNVar.MessageWnds,hWnd);
			break;
		case WM_YAMN_STOPACCOUNT:
		{
			HACCOUNT ActualAccount;
			MCONTACT hContact;
			DBVARIANT dbv;

			hContact=(MCONTACT)CallService(MS_POPUP_GETCONTACT,(WPARAM)hWnd,0);

			if (!db_get((MCONTACT) hContact,YAMN_DBMODULE,"Id",&dbv)) 
			{
				ActualAccount=(HACCOUNT) CallService(MS_YAMN_FINDACCOUNTBYNAME,(WPARAM)POP3Plugin,(LPARAM)dbv.pszVal);
				db_free(&dbv);
			}
			else
				ActualAccount = (HACCOUNT) hContact;

			if ((HACCOUNT)wParam != ActualAccount)
				break;

			DestroyWindow(hWnd);
			return 0;
		}
		case WM_NOTIFY:
/*			switch(((LPNMHDR)lParam)->code)
			{
				case NM_CLICK:
				{
				}
			}
			break;
*/		default:
			break;
	}
	return DefWindowProc(hWnd,msg,wParam,lParam);
}

#ifdef __GNUC__
//number of 100 ns periods between FILETIME 0 (1601/01/01 00:00:00.0000000) and TIMESTAMP 0 (1970/01/01 00:00:00)
#define NUM100NANOSEC  116444736000000000ULL
//The biggest time Get[Date|Time]Format can handle (Fri, 31 Dec 30827 23:59:59.9999999)
#define MAXFILETIME 0x7FFF35F4F06C7FFFULL
#else
#define NUM100NANOSEC  116444736000000000
#define MAXFILETIME 0x7FFF35F4F06C7FFF
#endif

ULONGLONG MimeDateToFileTime(char *datein)
{
	char *day=0, *month=0, *year=0, *time=0, *shift=0;
	SYSTEMTIME st;
	ULONGLONG res=0;
	int wShiftSeconds = CallService(MS_DB_TIME_TIMESTAMPTOLOCAL,0,0);
	GetLocalTime(&st);
	//datein = "Xxx, 1 Jan 2060 5:29:1 +0530 XXX";
	//datein = "Xxx,  1 Jan 2060 05:29:10 ";
	//datein = "      ManySpaces  1.5   Jan 2060 05::";
	//datein = "Xxx,  35 February 20 :29:10 ";
	//datein = "01.12.2007 (22:38:17)"; //
	if (datein) {
		char tmp [64];
		while (  datein[0]==' ')  datein++; // eat leading spaces
		strncpy(tmp,datein,63); tmp [63]=0;
		if (atoi(tmp)) { // Parseable integer on DayOfWeek field? Buggy mime date.
			day = tmp;
		} else {
			int i = 0;
			while (tmp[i]==' ')i++; if (day = strchr(&tmp[i],' ')) {day[0]=0; day++;}
		}
		if (day) {while (  day[0]==' ')  day++;if (month= strchr(day,  ' ')) {month[0]=0; month++;}}
		if (month) {while (month[0]==' ')month++;if (year = strchr(month,' ')) { year[0]=0; year++;}}
		if (year) {while ( year[0]==' ') year++;if (time = strchr(year, ' ')) { time[0]=0; time++;}}
		if (time) {while ( time[0]==' ') time++;if (shift= strchr(time, ' ')) {shift[0]=0; shift++;shift[5]=0;}}

		if (year) {
			st.wYear = atoi(year);
			if (strlen(year)<4)	if (st.wYear<70)st.wYear += 2000; else st.wYear += 1900;
		};
		if (month) for (int i=0;i<12;i++) if (strncmp(month,s_MonthNames[i],3)==0) {st.wMonth = i + 1; break;}
		if (day) st.wDay = atoi(day);
		if (time) {
			char *h, *m, *s;
			h = time;
			if (m = strchr(h,':')) {
				m[0]=0; m++;
				if (s = strchr(m,':')) {s[0] = 0; s++;}
			} else s=0;
			st.wHour = atoi(h);
			st.wMinute = m?atoi(m):0;
			st.wSecond = s?atoi(s):0;
		} else {st.wHour=st.wMinute=st.wSecond=0;}

		if (shift) {
			if (strlen(shift)<4) {
				//has only hour
				wShiftSeconds = (atoi(shift))*3600;
			} else {
				char *smin = shift + strlen(shift)-2;
				int ismin = atoi(smin);
				smin[0] = 0;
				int ishour = atoi(shift);
				wShiftSeconds = (ishour*60+(ishour<0?-1:1)*ismin)*60;
			}
		}
	} // if (datein)
	FILETIME ft;
	if (SystemTimeToFileTime(&st,&ft)) {
		res = ((ULONGLONG)ft.dwHighDateTime<<32)|((ULONGLONG)ft.dwLowDateTime);
		LONGLONG w100nano = Int32x32To64((DWORD)wShiftSeconds,10000000);
		res -=  w100nano;
	}else{
		res=0;
	}
	return res;
}

void FileTimeToLocalizedDateTime(LONGLONG filetime, WCHAR *dateout, int lendateout)
{
	int localeID = CallService(MS_LANGPACK_GETLOCALE,0,0);
	//int localeID = MAKELCID(LANG_URDU, SORT_DEFAULT);
	if (localeID==CALLSERVICE_NOTFOUND) localeID=LOCALE_USER_DEFAULT;
	if (filetime>MAXFILETIME) filetime = MAXFILETIME;
	else if (filetime<=0) {
		wcsncpy(dateout,TranslateW(L"Invalid"),lendateout);
		return;
	}
	SYSTEMTIME st;
	WORD wTodayYear, wTodayMonth, wTodayDay;
	FILETIME ft;
	BOOL willShowDate = !(optDateTime&SHOWDATENOTODAY);
	if (!willShowDate) {
		GetLocalTime(&st);
		wTodayYear = st.wYear;
		wTodayMonth = st.wMonth;
		wTodayDay = st.wDay;
	}
	ft.dwLowDateTime = (DWORD)filetime;
	ft.dwHighDateTime = (DWORD)(filetime >> 32);
	FILETIME localft;
	if (!FileTimeToLocalFileTime(&ft,&localft)) {
			// this should never happen
			wcsncpy(dateout,L"Incorrect FileTime",lendateout);
	} else {
		if (!FileTimeToSystemTime(&localft,&st)) {
			// this should never happen
			wcsncpy(dateout,L"Incorrect LocalFileTime",lendateout);
		} else {
			dateout[lendateout-1]=0;
			int templen = 0;
			if (!willShowDate) willShowDate = (wTodayYear != st.wYear)||(wTodayMonth != st.wMonth)||(wTodayDay != st.wDay);
			if (willShowDate) {
				templen = GetDateFormatW(localeID,(optDateTime&SHOWDATELONG)?DATE_LONGDATE:DATE_SHORTDATE,&st,NULL,dateout,lendateout-2);
				dateout[templen-1] = ' ';
			}
			if (templen<(lendateout-1)) {
				GetTimeFormatW(localeID,(optDateTime&SHOWDATENOSECONDS)?TIME_NOSECONDS:0,&st,NULL,&dateout[templen],lendateout-templen-1);
			}
		}
	}
}

void MimeDateToLocalizedDateTime(char *datein, WCHAR *dateout, int lendateout)
{
	ULONGLONG ft = MimeDateToFileTime(datein);
	FileTimeToLocalizedDateTime(ft,dateout,lendateout);
}

int CALLBACK ListViewCompareProc(LPARAM lParam1, LPARAM lParam2,LPARAM lParamSort ) {
	if (lParam1 == NULL || lParam2 == NULL)
		return 0;

	int					nResult	= 0;
	char				*str1;
	char				*str2;
	HYAMNMAIL			email1	= (HYAMNMAIL)lParam1;
	HYAMNMAIL			email2	= (HYAMNMAIL)lParam2;
	struct CShortHeader	Header1;
	struct CShortHeader	Header2;
	memset(&Header1, 0, sizeof(Header1));
	memset(&Header2, 0, sizeof(Header2));

	try {
		ExtractShortHeader(email1->MailData->TranslatedHeader,&Header1);
		ExtractShortHeader(email2->MailData->TranslatedHeader,&Header2);

		switch((int)lParamSort)
		{
			case 0:	//From
				if (Header1.FromNick == NULL)
					 str1 = Header1.From;
				else str1 = Header1.FromNick;

				if (Header2.FromNick == NULL)
					 str2 = Header2.From;
				else str2 = Header2.FromNick;

				nResult = strcmp(str1, str2);

				if (bFrom) nResult = -nResult;
				break;
			case 1:	//Subject
				if (Header1.Subject == NULL)
					 str1 = " ";
				else str1 = Header1.Subject;

				if (Header2.Subject == NULL)
					 str2 = " ";
				else str2 = Header2.Subject;

				nResult = strcmp(str1, str2);

				if (bSub) nResult = -nResult;
				break;
			case 2:	//Size
				if (email1->MailData->Size == email2->MailData->Size)	nResult =  0;
				if (email1->MailData->Size >  email2->MailData->Size)	nResult =  1;
				if (email1->MailData->Size <  email2->MailData->Size)	nResult = -1;

				if (bSize) nResult = -nResult;
				break;

			case 3:	//Date
				{
				ULONGLONG ts1 = 0, ts2 = 0;
				ts1 = MimeDateToFileTime(Header1.Date);
				ts2 = MimeDateToFileTime(Header2.Date);
				if (ts1 > ts2) nResult = 1;
				else if (ts1 < ts2) nResult = -1;
				else nResult = 0;
				}
				if (bDate) nResult = -nResult;
				break;

			default:
				if (Header1.Subject == NULL) str1 = " ";
				else str1 = Header1.Subject;

				if (Header2.Subject == NULL) str2 = " ";
				else str2 = Header2.Subject;

				nResult = strcmp(str1, str2);
				break;
		}
		//MessageBox(NULL,str1,str2,0);
	}
	catch( ... )
	{
	}

	//free mem
	DeleteShortHeaderContent(&Header1);
	DeleteShortHeaderContent(&Header2);
	return nResult;

} 

HCURSOR hCurSplitNS, hCurSplitWE;
#define DM_SPLITTERMOVED     (WM_USER+15)

static LRESULT CALLBACK SplitterSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_NCHITTEST:
		return HTCLIENT;

	case WM_SETCURSOR:
		SetCursor(hCurSplitNS);
		return TRUE;

	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		return 0;

	case WM_MOUSEMOVE:
		if (GetCapture() == hwnd) {
			RECT rc;
			GetClientRect(hwnd, &rc);
			SendMessage(GetParent(hwnd), DM_SPLITTERMOVED, (short) HIWORD(GetMessagePos()) + rc.bottom / 2, (LPARAM) hwnd);
		}
		return 0;

	case WM_LBUTTONUP:
		ReleaseCapture();
		return 0;
	}
	return mir_callNextSubclass(hwnd, SplitterSubclassProc, msg, wParam, lParam);
}

void ConvertCodedStringToUnicode(char *stream,WCHAR **storeto,DWORD cp,int mode);
int ConvertStringToUnicode(char *stream,unsigned int cp,WCHAR **out);

INT_PTR CALLBACK DlgProcYAMNShowMessage(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			PYAMN_MAILSHOWPARAM MailParam = (PYAMN_MAILSHOWPARAM)lParam;
			WCHAR *iHeaderW=NULL;
			WCHAR *iValueW=NULL;
			int StrLen;
			HWND hListView = GetDlgItem(hDlg,IDC_LISTHEADERS);
			mir_subclassWindow(GetDlgItem(hDlg, IDC_SPLITTER), SplitterSubclassProc);
			SetWindowLongPtr(hDlg,DWLP_USER,(LONG_PTR)MailParam);
			SendMessageW(hDlg,WM_SETICON,(WPARAM)ICON_BIG,(LPARAM)g_LoadIconEx(2, true));
			SendMessageW(hDlg,WM_SETICON,(WPARAM)ICON_SMALL,(LPARAM)g_LoadIconEx(2));

			ListView_SetUnicodeFormat(hListView,TRUE);
			ListView_SetExtendedListViewStyle(hListView,LVS_EX_FULLROWSELECT);

			StrLen=MultiByteToWideChar(CP_ACP,MB_USEGLYPHCHARS,Translate("Header"),-1,NULL,0);
			iHeaderW=new WCHAR[StrLen+1];
			MultiByteToWideChar(CP_ACP,MB_USEGLYPHCHARS,Translate("Header"),-1,iHeaderW,StrLen);

			StrLen=MultiByteToWideChar(CP_ACP,MB_USEGLYPHCHARS,Translate("Value"),-1,NULL,0);
			iValueW=new WCHAR[StrLen+1];
			MultiByteToWideChar(CP_ACP,MB_USEGLYPHCHARS,Translate("Value"),-1,iValueW,StrLen);

			LVCOLUMNW lvc0={LVCF_FMT | LVCF_TEXT | LVCF_WIDTH,LVCFMT_LEFT,130,iHeaderW,0,0};
			LVCOLUMNW lvc1={LVCF_FMT | LVCF_TEXT | LVCF_WIDTH,LVCFMT_LEFT,400,iValueW,0,0};
			SendMessageW(hListView,LVM_INSERTCOLUMNW,0,(LPARAM)&lvc0);
			SendMessageW(hListView,LVM_INSERTCOLUMNW,1,(LPARAM)&lvc1);
			if (NULL != iHeaderW)
				delete[] iHeaderW;
			if (NULL != iValueW)
				delete[] iValueW;

			//WindowList_Add(YAMNVar.MessageWnds,hDlg,NULL);
			//WindowList_Add(YAMNVar.NewMailAccountWnd,hDlg,ActualAccount);
			SendMessage(hDlg,WM_YAMN_CHANGECONTENT,0,(LPARAM)MailParam);
			MoveWindow(hDlg,HeadPosX,HeadPosY,HeadSizeX,HeadSizeY,0);
			ShowWindow(hDlg,SW_SHOWNORMAL);
		}
		break;

	case WM_YAMN_CHANGECONTENT:
		{
			PYAMN_MAILSHOWPARAM MailParam  = (PYAMN_MAILSHOWPARAM)
				(lParam?lParam:GetWindowLongPtr(hDlg,DWLP_USER));
			HWND hListView = GetDlgItem(hDlg,IDC_LISTHEADERS);
			HWND hEdit = GetDlgItem(hDlg,IDC_EDITBODY);
			//do not redraw
			SendMessage(hListView, WM_SETREDRAW, 0, 0);
			ListView_DeleteAllItems(hListView);
			struct CMimeItem *Header;
			LVITEMW item;
			item.mask=LVIF_TEXT | LVIF_PARAM;
			WCHAR *From=0,*Subj=0;
			char *contentType=0, *transEncoding=0, *body=0; //should not be delete[]-ed
			for (Header=MailParam->mail->MailData->TranslatedHeader;Header != NULL;Header=Header->Next)
			{
				WCHAR *str1 = 0;
				WCHAR *str2 = 0;
				WCHAR str_nul[2] = {0};
				if (!body) if (!_stricmp(Header->name,"Body")) {body = Header->value; continue;}
				if (!contentType) if (!_stricmp(Header->name,"Content-Type")) contentType = Header->value;
				if (!transEncoding) if (!_stricmp(Header->name,"Content-Transfer-Encoding")) transEncoding = Header->value;
				//ConvertCodedStringToUnicode(Header->name,&str1,MailParam->mail->MailData->CP,1); 
				{
					int streamsize = MultiByteToWideChar(20127,0,Header->name,-1,NULL,0);
					str1 = (WCHAR *)malloc(sizeof(WCHAR) * (streamsize + 1));
					MultiByteToWideChar(20127,0,Header->name,-1,str1,streamsize);//US-ASCII
				}
				ConvertCodedStringToUnicode(Header->value,&str2,MailParam->mail->MailData->CP,1);
				if (!str2) { str2 = (WCHAR *)str_nul; }// the header value may be NULL
				if (!From) if (!_stricmp(Header->name,"From")) {
					From =new WCHAR[wcslen(str2)+1];
					wcscpy(From,str2);
				}
				if (!Subj) if (!_stricmp(Header->name,"Subject")) {
					Subj =new WCHAR[wcslen(str2)+1];
					wcscpy(Subj,str2);
				}
				//if (!hasBody) if (!strcmp(Header->name,"Body")) hasBody = true;
				int count = 0; WCHAR **split=0;
				int ofs = 0;
				while (str2[ofs]) {
					if ((str2[ofs]==0x266A)||(str2[ofs]==0x25D9)||(str2[ofs]==0x25CB)||
						(str2[ofs]==0x09)||(str2[ofs]==0x0A)||(str2[ofs]==0x0D))count++;
					ofs++;
				}
				split=new WCHAR*[count+1];
				count=0; ofs=0;
				split[0]=str2;
				while (str2[ofs]) {
					if ((str2[ofs]==0x266A)||(str2[ofs]==0x25D9)||(str2[ofs]==0x25CB)||
						(str2[ofs]==0x09)||(str2[ofs]==0x0A)||(str2[ofs]==0x0D)) {
							if (str2[ofs-1]) {
								count++;
							}
							split[count]=(WCHAR *)(str2+ofs+1);
							str2[ofs]=0;
					}
					ofs++;
				};

				if (!_stricmp(Header->name,"From")||!_stricmp(Header->name,"To")||!_stricmp(Header->name,"Date")||!_stricmp(Header->name,"Subject")) 
					item.iItem = 0;
				else 
					item.iItem = 999;
				for (int i=0;i<=count;i++) {
					item.iSubItem=0;
					if (i==0)
						item.pszText=str1;
					else {
						item.iItem++;
						item.pszText=0;
					}
					item.iItem=SendMessageW(hListView,LVM_INSERTITEMW,0,(LPARAM)&item);
					item.iSubItem=1;
					item.pszText=str2?split[i]:0;
					SendMessageW(hListView,LVM_SETITEMTEXTW,(WPARAM)item.iItem,(LPARAM)&item);
				} 
				delete[] split;

				if (str1)
					free(str1);
				if (str2 != (WCHAR *)str_nul)
					free(str2);
			}
			if (body) {
				WCHAR *bodyDecoded = 0;
				char *localBody=0;
				if (contentType) {
					if (!_strnicmp(contentType,"text",4)) {
						if (transEncoding) {
							if (!_stricmp(transEncoding,"base64")) {
								int size = (int)strlen(body)*3/4+5;
								localBody = new char[size+1];
								DecodeBase64(body,localBody,size); 
							} else if (!_stricmp(transEncoding,"quoted-printable")) {
								int size = (int)strlen(body)+2;
								localBody = new char[size+1];
								DecodeQuotedPrintable(body,localBody,size,FALSE); 
							}
						}
					} else if (!_strnicmp(contentType,"multipart/",10)) {
						char *bondary=NULL;
						if (NULL != (bondary=ExtractFromContentType(contentType,"boundary=")))
						{
							bodyDecoded = ParseMultipartBody(body,bondary);
							delete[] bondary;
						}
					}
				}
				if (!bodyDecoded)ConvertStringToUnicode(localBody?localBody:body,MailParam->mail->MailData->CP,&bodyDecoded);
				SendMessageW(hEdit,WM_SETTEXT,0,(LPARAM)bodyDecoded);
				delete[] bodyDecoded;
				if (localBody) delete[] localBody;
				SetFocus(hEdit);
			}
			if (!(MailParam->mail->Flags & YAMN_MSG_BODYRECEIVED)) {
				MailParam->mail->Flags |= YAMN_MSG_BODYREQUESTED;
				CallService(MS_YAMN_ACCOUNTCHECK,(WPARAM)MailParam->account,0);
			} else {
				if (MailParam->mail->Flags & YAMN_MSG_UNSEEN) {
					MailParam->mail->Flags&=~YAMN_MSG_UNSEEN; //mark the message as seen
					HWND hMailBrowser;
					if (hMailBrowser=WindowList_Find(YAMNVar.NewMailAccountWnd, (MCONTACT)MailParam->account)) {
						struct CChangeContent Params={MailParam->account->NewMailN.Flags|YAMN_ACC_MSGP,MailParam->account->NoNewMailN.Flags|YAMN_ACC_MSGP};	
						SendMessageW(hMailBrowser,WM_YAMN_CHANGECONTENT,(WPARAM)MailParam->account,(LPARAM)&Params);
					}
					else UpdateMails(NULL,MailParam->account,MailParam->account->NewMailN.Flags,MailParam->account->NoNewMailN.Flags);
				}
			}
			ShowWindow(GetDlgItem(hDlg, IDC_SPLITTER),(MailParam->mail->Flags & YAMN_MSG_BODYRECEIVED)?SW_SHOW:SW_HIDE);
			ShowWindow(hEdit,(MailParam->mail->Flags & YAMN_MSG_BODYRECEIVED)?SW_SHOW:SW_HIDE);
			WCHAR *title=0;
			size_t size = (From ? wcslen(From) : 0) + (Subj ? wcslen(Subj) : 0) + 4;
			title = new WCHAR[size];
			if (From && Subj)
				mir_sntprintf(title, size, L"%s (%s)", Subj, From);
			else if (From)
				_tcsncpy_s(title, size, From, _TRUNCATE);
			else if (Subj)
				_tcsncpy_s(title, size, Subj, _TRUNCATE);
			else
				_tcsncpy_s(title, size, L"none", _TRUNCATE);
			if (Subj) delete[] Subj;
			if (From) delete[] From;
			SendMessageW(hDlg,WM_SETTEXT,0,(LPARAM)title);
			delete[] title;
			// turn on redrawing
			SendMessage(hListView, WM_SETREDRAW, 1, 0);
			SendMessage(hDlg, WM_SIZE, 0, HeadSizeY<<16|HeadSizeX);
		} break;

	case WM_YAMN_STOPACCOUNT:
		{
			PYAMN_MAILSHOWPARAM MailParam  = (PYAMN_MAILSHOWPARAM)
				(lParam?lParam:GetWindowLongPtr(hDlg,DWLP_USER));

			if (NULL==MailParam)
				break;
			if ((HACCOUNT)wParam != MailParam->account)
				break;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"ShowMessage:STOPACCOUNT:sending destroy msg\n");
#endif
			DestroyWindow(hDlg);
		}
		return 1;

	case WM_CTLCOLORSTATIC: 
		//here should be check if this is our edittext control. 
		//but we have only one static control (for now);
		SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
		SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
		return (INT_PTR)GetSysColorBrush(COLOR_WINDOW);

	case WM_DESTROY:
		{
			RECT coord;
			if (GetWindowRect(hDlg,&coord)) {
				HeadPosX=coord.left;
				HeadSizeX=coord.right-coord.left;
				HeadPosY=coord.top;
				HeadSizeY=coord.bottom-coord.top;
			}

			PostQuitMessage(1);
		}
		break;

	case WM_SYSCOMMAND:
		switch(wParam) {
		case SC_CLOSE:
			DestroyWindow(hDlg);
			break;
		}
		break;

	case WM_MOVE:
		HeadPosX=LOWORD(lParam);	//((LPRECT)lParam)->right-((LPRECT)lParam)->left;
		HeadPosY=HIWORD(lParam);	//((LPRECT)lParam)->bottom-((LPRECT)lParam)->top;
		return 0;

	case DM_SPLITTERMOVED:
		if ((HWND) lParam == GetDlgItem(hDlg, IDC_SPLITTER)) {
			POINT pt;
			pt.x = 0;
			pt.y = wParam;
			ScreenToClient(hDlg, &pt);
			HeadSplitPos = (pt.y*1000)/HeadSizeY;//+rc.bottom-rc.top;
			if (HeadSplitPos>=1000) HeadSplitPos = 999;
			else if (HeadSplitPos<=0) HeadSplitPos = 1;
			else SendMessage(hDlg, WM_SIZE, 0, HeadSizeY<<16|HeadSizeX);
		}
		return 0;

	case WM_SIZE:
		if (wParam == SIZE_RESTORED) {
			HWND hList = GetDlgItem(hDlg,IDC_LISTHEADERS);
			HWND hEdit = GetDlgItem(hDlg,IDC_EDITBODY);
			BOOL changeX = LOWORD(lParam) != HeadSizeX;
			BOOL isBodyShown = ((PYAMN_MAILSHOWPARAM)(GetWindowLongPtr(hDlg,DWLP_USER)))->mail->Flags & YAMN_MSG_BODYRECEIVED;
			HeadSizeX=LOWORD(lParam);	//((LPRECT)lParam)->right-((LPRECT)lParam)->left;
			HeadSizeY=HIWORD(lParam);	//((LPRECT)lParam)->bottom-((LPRECT)lParam)->top;
			int localSplitPos = (HeadSplitPos*HeadSizeY)/1000;
			int localSizeX;
			RECT coord;
			MoveWindow(GetDlgItem(hDlg,IDC_SPLITTER),5,localSplitPos,HeadSizeX-10,2,TRUE);
			MoveWindow(hEdit,5,localSplitPos+6,HeadSizeX-10,HeadSizeY-localSplitPos-11,TRUE);	//where to put text window while resizing
			MoveWindow(hList,  5         ,5     ,HeadSizeX-10    ,(isBodyShown?localSplitPos:HeadSizeY)-10,TRUE);	//where to put headers list window while resizing
			//if (changeX) {
			if (GetClientRect(hList,&coord)) {
				localSizeX=coord.right-coord.left;
			} else localSizeX=HeadSizeX;
			LONG iNameWidth =  ListView_GetColumnWidth(hList,0);
			ListView_SetColumnWidth(hList,1,(localSizeX<=iNameWidth)?0:(localSizeX-iNameWidth));
			//}
		}
		return 0;

	case WM_CONTEXTMENU:
		if ( GetWindowLongPtr(( HWND )wParam, GWLP_ID ) == IDC_LISTHEADERS)	{
			//MessageBox(0,"LISTHEADERS","Debug",0);
			HWND hList = GetDlgItem( hDlg, IDC_LISTHEADERS );
			POINT pt = { (signed short)LOWORD( lParam ), (signed short)HIWORD( lParam ) };
			HTREEITEM hItem = 0;
			if (pt.x==-1) pt.x = 0;
			if (pt.y==-1) pt.y = 0;
			if (int numRows = ListView_GetItemCount(hList)) {
				HMENU hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)1, TranslateT("Copy Selected"));
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)2, TranslateT("Copy All"));
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)0, TranslateT("Cancel"));
				int nReturnCmd = TrackPopupMenu( hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hDlg, NULL );
				DestroyMenu( hMenu );
				if (nReturnCmd>0) {
					int courRow=0;
					size_t sizeNeeded = 0;
					TCHAR headname[64]={0}, headvalue[256]={0}; 
					for (courRow=0; courRow < numRows; courRow++) {
						if ((nReturnCmd==1) && (ListView_GetItemState(hList, courRow, LVIS_SELECTED)==0)) continue;
						ListView_GetItemText(hList, courRow, 0, headname, SIZEOF(headname));
						ListView_GetItemText(hList, courRow, 1, headvalue, SIZEOF(headvalue));
						size_t headnamelen = _tcslen(headname);
						if (headnamelen) sizeNeeded += 1 + headnamelen;
						sizeNeeded += 3 + _tcslen(headvalue);
					}
					if (sizeNeeded && OpenClipboard(hDlg)) {
						EmptyClipboard();
						HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE,(sizeNeeded+1)*sizeof(TCHAR));
						TCHAR *buff = ( TCHAR* )GlobalLock(hData);
						int courPos = 0;
						for (courRow=0;courRow<numRows;courRow++) {
							if ((nReturnCmd==1) && (ListView_GetItemState(hList, courRow, LVIS_SELECTED)==0)) continue;
							ListView_GetItemText(hList, courRow, 0, headname, SIZEOF(headname));
							ListView_GetItemText(hList, courRow, 1, headvalue, SIZEOF(headvalue));
							if ( _tcslen(headname)) courPos += mir_sntprintf(&buff[courPos], sizeNeeded + 1, _T("%s:\t%s\r\n"), headname, headvalue);
							else courPos += mir_sntprintf(&buff[courPos], sizeNeeded + 1, _T("\t%s\r\n"), headvalue);
						}
						GlobalUnlock(hData);

						SetClipboardData(CF_UNICODETEXT,hData);

						CloseClipboard();
					}
				}
			}
		}
		break; // just in case
	}
	return 0;
}

DWORD WINAPI ShowEmailThread(LPVOID Param) {
	struct MailShowMsgWinParam MyParam;
	MyParam=*(struct MailShowMsgWinParam *)Param;

	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"ShowMessage:Incrementing \"using threads\" %x (account %x)\n",MyParam.account->UsingThreads,MyParam.account);
	#endif
	SCIncFcn(MyParam.account->UsingThreads);
	SetEvent(MyParam.ThreadRunningEV);
	if (MyParam.mail->MsgWindow) {
		//if (!BringWindowToTop(MyParam.mail->MsgWindow)) {
		if (!SetForegroundWindow(MyParam.mail->MsgWindow)) {
			SendMessage(MyParam.mail->MsgWindow,WM_DESTROY,0,0);
			MyParam.mail->MsgWindow = 0;
			goto CREADTEVIEWMESSAGEWINDOW;
		}else{
			if (IsIconic(MyParam.mail->MsgWindow)) {
				OpenIcon(MyParam.mail->MsgWindow);
			}
		}
	} else {
CREADTEVIEWMESSAGEWINDOW:
		MyParam.mail->MsgWindow = CreateDialogParamW(YAMNVar.hInst,MAKEINTRESOURCEW(IDD_DLGSHOWMESSAGE),NULL,DlgProcYAMNShowMessage,(LPARAM)&MyParam);
		WindowList_Add(YAMNVar.MessageWnds,MyParam.mail->MsgWindow,NULL);
		MSG msg;
		while(GetMessage(&msg,NULL,0,0)) {
			if (!IsDialogMessage(MyParam.mail->MsgWindow, &msg)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
		}	}
		WindowList_Remove(YAMNVar.MessageWnds,MyParam.mail->MsgWindow);
		MyParam.mail->MsgWindow = NULL;
	}
	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"ShowMessage:Decrementing \"using threads\" %x (account %x)\n",MyParam.account->UsingThreads,MyParam.account);
	#endif
	SCDecFcn(MyParam.account->UsingThreads);
	delete Param;
	return 1;
}

INT_PTR CALLBACK DlgProcYAMNMailBrowser(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg) {
	case WM_INITDIALOG:
		{
			HACCOUNT ActualAccount;
			struct MailBrowserWinParam *MyParam=(struct MailBrowserWinParam *)lParam;
			struct CMailWinUserInfo *mwui;

			ListView_SetUnicodeFormat(GetDlgItem(hDlg,IDC_LISTMAILS),TRUE);
			ListView_SetExtendedListViewStyle(GetDlgItem(hDlg,IDC_LISTMAILS),LVS_EX_FULLROWSELECT);

			ActualAccount=MyParam->account;
			mwui=new struct CMailWinUserInfo;
			mwui->Account=ActualAccount;
			mwui->TrayIconState=0;
			mwui->UpdateMailsMessagesAccess=FALSE;
			mwui->Seen=FALSE;
			mwui->RunFirstTime=TRUE;

			SetWindowLongPtr(hDlg,DWLP_USER,(LONG_PTR)mwui);
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:INIT:ActualAccountSO-read wait\n");
#endif
			if (WAIT_OBJECT_0 != WaitToReadFcn(ActualAccount->AccountAccessSO))
			{
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"MailBrowser:INIT:ActualAccountSO-read enter failed\n");
#endif
				DestroyWindow(hDlg);
				return FALSE;
			}
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:INIT:ActualAccountSO-read enter\n");
#endif

			SendMessageW(GetDlgItem(hDlg,IDC_BTNAPP),WM_SETTEXT,0,(LPARAM)TranslateW(L"Run application"));
			SendMessageW(GetDlgItem(hDlg,IDC_BTNDEL),WM_SETTEXT,0,(LPARAM)TranslateW(L"Delete selected"));
			SendMessageW(GetDlgItem(hDlg,IDC_BTNCHECKALL),WM_SETTEXT,0,(LPARAM)TranslateW(L"Select All"));
			SendMessageW(GetDlgItem(hDlg,IDC_BTNOK),WM_SETTEXT,0,(LPARAM)TranslateW(L"OK"));

			LVCOLUMNW lvc0={LVCF_FMT | LVCF_TEXT | LVCF_WIDTH,LVCFMT_LEFT,FromWidth,TranslateW(L"From"),0,0};
			LVCOLUMNW lvc1={LVCF_FMT | LVCF_TEXT | LVCF_WIDTH,LVCFMT_LEFT,SubjectWidth,TranslateW(L"Subject"),0,0};
			LVCOLUMNW lvc2={LVCF_FMT | LVCF_TEXT | LVCF_WIDTH,LVCFMT_LEFT,SizeWidth,TranslateW(L"Size"),0,0};
			LVCOLUMNW lvc3={LVCF_FMT | LVCF_TEXT | LVCF_WIDTH,LVCFMT_LEFT,SizeDate,TranslateW(L"Date"),0,0};
			SendMessageW(GetDlgItem(hDlg,IDC_LISTMAILS),LVM_INSERTCOLUMNW,0,(LPARAM)&lvc0);
			SendMessageW(GetDlgItem(hDlg,IDC_LISTMAILS),LVM_INSERTCOLUMNW,1,(LPARAM)&lvc1);
			SendMessageW(GetDlgItem(hDlg,IDC_LISTMAILS),LVM_INSERTCOLUMNW,(WPARAM)2,(LPARAM)&lvc2);
			SendMessageW(GetDlgItem(hDlg,IDC_LISTMAILS),LVM_INSERTCOLUMNW,(WPARAM)3,(LPARAM)&lvc3);

			if ((ActualAccount->NewMailN.App != NULL) && (wcslen(ActualAccount->NewMailN.App)))
				EnableWindow(GetDlgItem(hDlg,IDC_BTNAPP),TRUE);
			else
				EnableWindow(GetDlgItem(hDlg,IDC_BTNAPP),FALSE);
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:INIT:ActualAccountSO-read done\n");
#endif
			ReadDoneFcn(ActualAccount->AccountAccessSO);

			WindowList_Add(YAMNVar.MessageWnds,hDlg,NULL);
			WindowList_Add(YAMNVar.NewMailAccountWnd,hDlg, (MCONTACT)ActualAccount);

			{
				TCHAR accstatus[512];
				GetStatusFcn(ActualAccount,accstatus);
				SetDlgItemText(hDlg,IDC_STSTATUS,accstatus);
			}
			SetTimer(hDlg,TIMER_FLASHING,500,NULL);

			if (ActualAccount->hContact != NULL)
				CallService(MS_CLIST_REMOVEEVENT,(WPARAM)ActualAccount->hContact,(LPARAM)"yamn new mail message");

			mir_subclassWindow( GetDlgItem(hDlg, IDC_LISTMAILS), ListViewSubclassProc);
		}
		break;

	case WM_DESTROY:
		{
			HACCOUNT ActualAccount;
			RECT coord;
			LVCOLUMNW ColInfo;
			NOTIFYICONDATA nid;
			HYAMNMAIL Parser;
			struct CMailWinUserInfo *mwui;

			mwui=(struct CMailWinUserInfo *)GetWindowLongPtr(hDlg,DWLP_USER);
			if (NULL==(ActualAccount=GetWindowAccount(hDlg)))
				break;
			ColInfo.mask=LVCF_WIDTH;
			if (ListView_GetColumn(GetDlgItem(hDlg,IDC_LISTMAILS),0,&ColInfo))
				FromWidth=ColInfo.cx;
			if (ListView_GetColumn(GetDlgItem(hDlg,IDC_LISTMAILS),1,&ColInfo))
				SubjectWidth=ColInfo.cx;
			if (ListView_GetColumn(GetDlgItem(hDlg,IDC_LISTMAILS),2,&ColInfo))
				SizeWidth=ColInfo.cx;
			if (ListView_GetColumn(GetDlgItem(hDlg,IDC_LISTMAILS),3,&ColInfo))
				SizeDate=ColInfo.cx;

#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:DESTROY:save window position\n");
#endif
			if (!YAMNVar.Shutdown && GetWindowRect(hDlg,&coord))	//the YAMNVar.Shutdown testing is because M<iranda strange functionality at shutdown phase, when call to DBWriteContactSetting freezes calling thread
			{
				PosX=coord.left;
				SizeX=coord.right-coord.left;
				PosY=coord.top;
				SizeY=coord.bottom-coord.top;
				db_set_dw(NULL,YAMN_DBMODULE,YAMN_DBPOSX,PosX);
				db_set_dw(NULL,YAMN_DBMODULE,YAMN_DBPOSY,PosY);
				db_set_dw(NULL,YAMN_DBMODULE,YAMN_DBSIZEX,SizeX);
				db_set_dw(NULL,YAMN_DBMODULE,YAMN_DBSIZEY,SizeY);
			}
			KillTimer(hDlg,TIMER_FLASHING);

#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:DESTROY:remove window from list\n");
#endif
			WindowList_Remove(YAMNVar.NewMailAccountWnd,hDlg);
			WindowList_Remove(YAMNVar.MessageWnds,hDlg);

#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:DESTROY:ActualAccountMsgsSO-write wait\n");
#endif
			if (WAIT_OBJECT_0 != WaitToWriteFcn(ActualAccount->MessagesAccessSO))
			{
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"MailBrowser:DESTROY:ActualAccountMsgsSO-write wait failed\n");
#endif
				break;
			}
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:DESTROY:ActualAccountMsgsSO-write enter\n");
#endif
			//delete mails from queue, which are deleted from server (spam level 3 mails e.g.)
			for (Parser=(HYAMNMAIL)ActualAccount->Mails;Parser != NULL;Parser=Parser->Next)
			{
				if ((Parser->Flags & YAMN_MSG_DELETED) && YAMN_MSG_SPAML(Parser->Flags,YAMN_MSG_SPAML3) && mwui->Seen)		//if spaml3 was already deleted and user knows about it
				{
					DeleteMessageFromQueueFcn((HYAMNMAIL *)&ActualAccount->Mails,Parser,1);
					CallService(MS_YAMN_DELETEACCOUNTMAIL,(WPARAM)ActualAccount->Plugin,(LPARAM)Parser);
				}
			}

			//mark mails as read (remove "new" and "unseen" flags)
			if (mwui->Seen)
				SetRemoveFlagsInQueueFcn((HYAMNMAIL)ActualAccount->Mails,YAMN_MSG_DISPLAY,0,YAMN_MSG_NEW | YAMN_MSG_UNSEEN,0);
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:DESTROY:ActualAccountMsgsSO-write done\n");
#endif
			WriteDoneFcn(ActualAccount->MessagesAccessSO);

			memset(&nid, 0, sizeof(NOTIFYICONDATA));

			delete mwui;
			SetWindowLongPtr(hDlg,DWLP_USER,(LONG_PTR)NULL);

			nid.cbSize=sizeof(NOTIFYICONDATA);
			nid.hWnd=hDlg;
			nid.uID=0;
			Shell_NotifyIcon(NIM_DELETE,&nid);
			PostQuitMessage(0);
		}
		break;
	case WM_SHOWWINDOW:
		{
			struct CMailWinUserInfo *mwui;

			if (NULL==(mwui=(struct CMailWinUserInfo *)GetWindowLongPtr(hDlg,DWLP_USER)))
				return 0;
			mwui->Seen=TRUE;
		}
	case WM_YAMN_CHANGESTATUS:
		{
			HACCOUNT ActualAccount;
			if (NULL==(ActualAccount=GetWindowAccount(hDlg)))
				break;

			if ((HACCOUNT)wParam != ActualAccount)
				break;

			TCHAR accstatus[512];
			GetStatusFcn(ActualAccount,accstatus);
			SetDlgItemText(hDlg,IDC_STSTATUS,accstatus);
		}
		return 1;
	case WM_YAMN_CHANGECONTENT:
		{
			struct CUpdateMails UpdateParams;
			BOOL ThisThreadWindow=(GetCurrentThreadId()==GetWindowThreadProcessId(hDlg,NULL));

			if (NULL==(UpdateParams.Copied=CreateEvent(NULL,FALSE,FALSE,NULL)))
			{
				DestroyWindow(hDlg);
				return 0;
			}
			UpdateParams.Flags=(struct CChangeContent *)lParam;
			UpdateParams.Waiting=!ThisThreadWindow;

#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:CHANGECONTENT:posting UPDATEMAILS\n");
#endif
			if (ThisThreadWindow)
			{
				if (!UpdateMails(hDlg,(HACCOUNT)wParam,UpdateParams.Flags->nflags,UpdateParams.Flags->nnflags))
					DestroyWindow(hDlg);
			}
			else if (PostMessage(hDlg,WM_YAMN_UPDATEMAILS,wParam,(LPARAM)&UpdateParams))	//this ensures UpdateMails will execute the thread who created the browser window
			{
				if (!ThisThreadWindow)
				{
#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"MailBrowser:CHANGECONTENT:waiting for event\n");
#endif
					WaitForSingleObject(UpdateParams.Copied,INFINITE);
#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"MailBrowser:CHANGECONTENT:event signaled\n");
#endif
				}
			}

			CloseHandle(UpdateParams.Copied);
		}
		return 1;
	case WM_YAMN_UPDATEMAILS:
		{
			HACCOUNT ActualAccount;

			struct CUpdateMails *um=(struct CUpdateMails *)lParam;
			DWORD nflags,nnflags;

#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:UPDATEMAILS\n");
#endif

			if (NULL==(ActualAccount=GetWindowAccount(hDlg)))
				return 0;
			if ((HACCOUNT)wParam != ActualAccount)
				return 0;

			nflags=um->Flags->nflags;
			nnflags=um->Flags->nnflags;

			if (um->Waiting)
				SetEvent(um->Copied);

			if (!UpdateMails(hDlg,ActualAccount,nflags,nnflags))
				DestroyWindow(hDlg);
		}
		return 1;
	case WM_YAMN_STOPACCOUNT:
		{
			HACCOUNT ActualAccount;

			if (NULL==(ActualAccount=GetWindowAccount(hDlg)))
				break;
			if ((HACCOUNT)wParam != ActualAccount)
				break;
#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:STOPACCOUNT:sending destroy msg\n");
#endif
			PostQuitMessage(0);
		}
		return 1;
	case WM_YAMN_NOTIFYICON:
		{
			HACCOUNT ActualAccount;
			if (NULL==(ActualAccount=GetWindowAccount(hDlg)))
				break;

			switch(lParam)
			{
			case WM_LBUTTONDBLCLK:
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"MailBrowser:DBLCLICKICON:ActualAccountSO-read wait\n");
#endif
				if (WAIT_OBJECT_0 != WaitToReadFcn(ActualAccount->AccountAccessSO))
				{
#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"MailBrowser:DBLCLICKICON:ActualAccountSO-read wait failed\n");
#endif
					return 0;
				}
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"MailBrowser:DBLCLICKICON:ActualAccountSO-read enter\n");
#endif
				if (ActualAccount->AbilityFlags & YAMN_ACC_BROWSE)
				{
					ShowWindow(hDlg,SW_SHOWNORMAL);
					SetForegroundWindow(hDlg);
				}
				else
					DestroyWindow(hDlg);
#ifdef DEBUG_SYNCHRO
				DebugLog(SynchroFile,"MailBrowser:DBLCLICKICON:ActualAccountSO-read done\n");
#endif
				ReadDoneFcn(ActualAccount->AccountAccessSO);
				break;
			}
			break;
		}
	case WM_YAMN_SHOWSELECTED:
		{
			int iSelect;
			iSelect=SendMessage(GetDlgItem(hDlg,IDC_LISTMAILS),LVM_GETNEXTITEM,-1,MAKELPARAM((UINT)LVNI_FOCUSED,0)); // return item selected

			if (iSelect != -1) 
			{
				LV_ITEMW item;
				HYAMNMAIL ActualMail;

				item.iItem=iSelect;
				item.iSubItem=0;
				item.mask=LVIF_PARAM | LVIF_STATE;
				item.stateMask=0xFFFFFFFF;
				ListView_GetItem(GetDlgItem(hDlg,IDC_LISTMAILS),&item);
				ActualMail=(HYAMNMAIL)item.lParam;
				if (NULL != ActualMail)
				{
					//ShowEmailThread
					PYAMN_MAILSHOWPARAM MailParam = new YAMN_MAILSHOWPARAM;
					MailParam->account = GetWindowAccount(hDlg);
					MailParam->mail = ActualMail;
					if (NULL != (MailParam->ThreadRunningEV=CreateEvent(NULL,FALSE,FALSE,NULL))) {
						HANDLE NewThread;
						if (NULL != (NewThread=CreateThread(NULL,0,ShowEmailThread,MailParam,0,NULL)))
						{
							//WaitForSingleObject(MailParam->ThreadRunningEV,INFINITE);
							CloseHandle(NewThread);
						}
						CloseHandle(MailParam->ThreadRunningEV);
					}
					//delete MailParam;
				}
			}
		} break;
	case WM_SYSCOMMAND:
		{
			HACCOUNT ActualAccount;

			if (NULL==(ActualAccount=GetWindowAccount(hDlg)))
				break;
			switch(wParam)
			{
			case SC_CLOSE:
				DestroyWindow(hDlg);
				break;
			}
		}
		break;

	case WM_COMMAND:
		{
			HACCOUNT ActualAccount;
			int Items;

			if (NULL==(ActualAccount=GetWindowAccount(hDlg)))
				break;

			switch(LOWORD(wParam))
			{
			case IDC_BTNCHECKALL:
				ListView_SetItemState(GetDlgItem(hDlg,IDC_LISTMAILS), -1, 0, LVIS_SELECTED); // deselect all items
				ListView_SetItemState(GetDlgItem(hDlg,IDC_LISTMAILS),-1, LVIS_SELECTED ,LVIS_SELECTED);
				Items = ListView_GetItemCount(GetDlgItem(hDlg,IDC_LISTMAILS));
				ListView_RedrawItems(GetDlgItem(hDlg,IDC_LISTMAILS), 0, Items);
				UpdateWindow(GetDlgItem(hDlg,IDC_LISTMAILS));
				SetFocus(GetDlgItem(hDlg,IDC_LISTMAILS));
				break;

			case IDC_BTNOK:
				DestroyWindow(hDlg);
				break;

			case IDC_BTNAPP:
				{
					PROCESS_INFORMATION pi;
					STARTUPINFOW si;

					memset(&si, 0, sizeof(si));
					si.cb=sizeof(si);

#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"MailBrowser:BTNAPP:ActualAccountSO-read wait\n");
#endif
					if (WAIT_OBJECT_0==WaitToReadFcn(ActualAccount->AccountAccessSO))
					{
#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"MailBrowser:BTNAPP:ActualAccountSO-read enter\n");
#endif
						if (ActualAccount->NewMailN.App != NULL)
						{
							WCHAR *Command;
							if (ActualAccount->NewMailN.AppParam != NULL)
								Command=new WCHAR[wcslen(ActualAccount->NewMailN.App)+wcslen(ActualAccount->NewMailN.AppParam)+6];
							else
								Command=new WCHAR[wcslen(ActualAccount->NewMailN.App)+6];

							if (Command != NULL)
							{
								mir_wstrcpy(Command,L"\"");
								mir_wstrcat(Command,ActualAccount->NewMailN.App);
								mir_wstrcat(Command,L"\" ");
								if (ActualAccount->NewMailN.AppParam != NULL)
									mir_wstrcat(Command,ActualAccount->NewMailN.AppParam);
								CreateProcessW(NULL,Command,NULL,NULL,FALSE,NORMAL_PRIORITY_CLASS,NULL,NULL,&si,&pi);
								delete[] Command;
							}
						}

#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"MailBrowser:BTNAPP:ActualAccountSO-read done\n");
#endif
						ReadDoneFcn(ActualAccount->AccountAccessSO);
					}
#ifdef DEBUG_SYNCHRO
					else
						DebugLog(SynchroFile,"MailBrowser:BTNAPP:ActualAccountSO-read enter failed\n");
#endif
					if (!(GetKeyState(VK_SHIFT) & 0x8000) && !(GetKeyState(VK_CONTROL) & 0x8000))
						DestroyWindow(hDlg);

				}
				break;
			case IDC_BTNDEL:
				{
					LVITEMW item;
					HYAMNMAIL FirstMail=NULL,ActualMail;
					HANDLE ThreadRunningEV;
					DWORD tid,Total=0;

					//	we use event to signal, that running thread has all needed stack parameters copied
					if (NULL==(ThreadRunningEV=CreateEvent(NULL,FALSE,FALSE,NULL)))
						break;
					int Items=ListView_GetItemCount(GetDlgItem(hDlg,IDC_LISTMAILS));

					item.stateMask=0xFFFFFFFF;
#ifdef DEBUG_SYNCHRO
					DebugLog(SynchroFile,"MailBrowser:BTNDEL:ActualAccountMsgsSO-write wait\n");
#endif
					if (WAIT_OBJECT_0==WaitToWriteFcn(ActualAccount->MessagesAccessSO))
					{
#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"MailBrowser:BTNDEL:ActualAccountMsgsSO-write enter\n");
#endif
						for (int i=0;i<Items;i++)
						{
							item.iItem=i;
							item.iSubItem=0;
							item.mask=LVIF_PARAM | LVIF_STATE;
							item.stateMask=0xFFFFFFFF;
							ListView_GetItem(GetDlgItem(hDlg,IDC_LISTMAILS),&item);
							ActualMail=(HYAMNMAIL)item.lParam;
							if (NULL==ActualMail)
								break;
							if (item.state & LVIS_SELECTED)
							{
								ActualMail->Flags|=YAMN_MSG_USERDELETE;	//set to mail we are going to delete it
								Total++;
							}
						}

						// Enable write-access to mails
#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"MailBrowser:BTNDEL:ActualAccountMsgsSO-write done\n");
#endif
						WriteDoneFcn(ActualAccount->MessagesAccessSO);

						if (Total)
						{
							TCHAR DeleteMsg[1024];

							mir_sntprintf(DeleteMsg, SIZEOF(DeleteMsg), TranslateT("Do you really want to delete %d selected mails?"),Total);
							if (IDOK==MessageBox(hDlg,DeleteMsg,TranslateT("Delete confirmation"),MB_OKCANCEL | MB_ICONWARNING))
							{
								struct DeleteParam ParamToDeleteMails={YAMN_DELETEVERSION,ThreadRunningEV,ActualAccount,NULL};

								// Find if there's mail marked to delete, which was deleted before
#ifdef DEBUG_SYNCHRO
								DebugLog(SynchroFile,"MailBrowser:BTNDEL:ActualAccountMsgsSO-write wait\n");
#endif
								if (WAIT_OBJECT_0==WaitToWriteFcn(ActualAccount->MessagesAccessSO))
								{
#ifdef DEBUG_SYNCHRO
									DebugLog(SynchroFile,"MailBrowser:BTNDEL:ActualAccountMsgsSO-write enter\n");
#endif
									for (ActualMail=(HYAMNMAIL)ActualAccount->Mails;ActualMail != NULL;ActualMail=ActualMail->Next)
									{
										if ((ActualMail->Flags & YAMN_MSG_DELETED) && ((ActualMail->Flags & YAMN_MSG_USERDELETE)))	//if selected mail was already deleted
										{
											DeleteMessageFromQueueFcn((HYAMNMAIL *)&ActualAccount->Mails,ActualMail,1);
											CallService(MS_YAMN_DELETEACCOUNTMAIL,(WPARAM)ActualAccount->Plugin,(LPARAM)ActualMail);	//delete it from memory
											continue;
										}
									}
									// Set flag to marked mails that they can be deleted
									SetRemoveFlagsInQueueFcn((HYAMNMAIL)ActualAccount->Mails,YAMN_MSG_DISPLAY | YAMN_MSG_USERDELETE,0,YAMN_MSG_DELETEOK,1);
									// Create new thread which deletes marked mails.
									HANDLE NewThread;

									if (NULL != (NewThread=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ActualAccount->Plugin->Fcn->DeleteMailsFcnPtr,(LPVOID)&ParamToDeleteMails,0,&tid)))
									{
										WaitForSingleObject(ThreadRunningEV,INFINITE);
										CloseHandle(NewThread);
									}
									// Enable write-access to mails
#ifdef DEBUG_SYNCHRO
									DebugLog(SynchroFile,"MailBrowser:BTNDEL:ActualAccountMsgsSO-write done\n");
#endif
									WriteDoneFcn(ActualAccount->MessagesAccessSO);
								}
							}
							else
								//else mark messages that they are not to be deleted
								SetRemoveFlagsInQueueFcn((HYAMNMAIL)ActualAccount->Mails,YAMN_MSG_DISPLAY | YAMN_MSG_USERDELETE,0,YAMN_MSG_USERDELETE,0);
						}
					}
					CloseHandle(ThreadRunningEV);
					if (db_get_b(NULL, YAMN_DBMODULE, YAMN_CLOSEDELETE, 0))
						DestroyWindow(hDlg);

				}
				break;
			}
		}
		break;
	case WM_SIZE:
		if (wParam==SIZE_RESTORED)
		{
			LONG x=LOWORD(lParam);	//((LPRECT)lParam)->right-((LPRECT)lParam)->left;
			LONG y=HIWORD(lParam);	//((LPRECT)lParam)->bottom-((LPRECT)lParam)->top;
			MoveWindow(GetDlgItem(hDlg,IDC_BTNDEL),     5            ,y-5-25,(x-20)/3,25,TRUE);	//where to put DELETE button while resizing
			MoveWindow(GetDlgItem(hDlg,IDC_BTNCHECKALL),10+  (x-20)/3,y-5-25,(x-20)/6,25,TRUE);	//where to put CHECK ALL button while resizing				
			MoveWindow(GetDlgItem(hDlg,IDC_BTNAPP),     15+  (x-20)/3 + (x-20)/6,y-5-25,(x-20)/3,25,TRUE);	//where to put RUN APP button while resizing
			MoveWindow(GetDlgItem(hDlg,IDC_BTNOK),      20+2*(x-20)/3 + (x-20)/6 ,y-5-25,(x-20)/6,25,TRUE);	//where to put OK button while resizing
			MoveWindow(GetDlgItem(hDlg,IDC_LISTMAILS),  5         ,5     ,x-10    ,y-55,TRUE);	//where to put list mail window while resizing
			MoveWindow(GetDlgItem(hDlg,IDC_STSTATUS),   5         ,y-5-45     ,x-10    ,15,TRUE);	//where to put account status text while resizing
		}
		//			break;
		return 0;
	case WM_GETMINMAXINFO:
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x=MAILBROWSER_MINXSIZE;
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y=MAILBROWSER_MINYSIZE;
		return 0;
	case WM_TIMER:
		{
			NOTIFYICONDATA nid;
			struct CMailWinUserInfo *mwui=(struct CMailWinUserInfo *)GetWindowLongPtr(hDlg,DWLP_USER);

			memset(&nid, 0, sizeof(nid));
			nid.cbSize=sizeof(NOTIFYICONDATA);
			nid.hWnd=hDlg;
			nid.uID=0;
			nid.uFlags=NIF_ICON;
			if (mwui->TrayIconState==0)
				nid.hIcon=g_LoadIconEx(0);
			else
				nid.hIcon=g_LoadIconEx(2);
			Shell_NotifyIcon(NIM_MODIFY,&nid);
			mwui->TrayIconState=!mwui->TrayIconState;
			//			UpdateWindow(hDlg);
		}
		break;

	case WM_NOTIFY:
		switch(((LPNMHDR)lParam)->idFrom) {
		case IDC_LISTMAILS:
			switch(((LPNMHDR)lParam)->code) {
			case NM_DBLCLK:
				SendMessage(hDlg,WM_YAMN_SHOWSELECTED,0,0);
				break;
			case LVN_COLUMNCLICK:
				HACCOUNT ActualAccount;
				if (NULL != (ActualAccount=GetWindowAccount(hDlg))) {
					NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)lParam;
					if (WAIT_OBJECT_0==WaitToReadFcn(ActualAccount->AccountAccessSO))
					{
#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"MailBrowser:COLUMNCLICK:ActualAccountSO-read enter\n");
#endif
						switch((int)pNMListView->iSubItem)
						{
						case 0:
							bFrom = !bFrom;
							break;
						case 1:
							bSub = !bSub;
							break;
						case 2:
							bSize = !bSize;
							break;
						case 3:
							bDate = !bDate;
							break;
						default:
							break;
						}
						ListView_SortItems(pNMListView->hdr.hwndFrom,ListViewCompareProc,pNMListView->iSubItem);
#ifdef DEBUG_SYNCHRO
						DebugLog(SynchroFile,"MailBrowser:BTNAPP:ActualAccountSO-read done\n");
#endif
						ReadDoneFcn(ActualAccount->AccountAccessSO);
					}
				}
				break;

			case NM_CUSTOMDRAW:
				{
					HACCOUNT ActualAccount;
					LPNMLVCUSTOMDRAW cd=(LPNMLVCUSTOMDRAW)lParam;
					LONG_PTR PaintCode;

					if (NULL==(ActualAccount=GetWindowAccount(hDlg)))
						break;

					switch(cd->nmcd.dwDrawStage) {
					case CDDS_PREPAINT:
						PaintCode=CDRF_NOTIFYITEMDRAW;
						break;
					case CDDS_ITEMPREPAINT:
						PaintCode=CDRF_NOTIFYSUBITEMDRAW;
						break;
					case CDDS_ITEMPREPAINT | CDDS_SUBITEM:
						{
							//									COLORREF crText, crBkgnd;
							//									crText= RGB(128,128,255);
							HYAMNMAIL ActualMail;
							BOOL umma;

							{
								struct CMailWinUserInfo *mwui;
								mwui=(struct CMailWinUserInfo *)GetWindowLongPtr(hDlg,DWLP_USER);
								umma= mwui->UpdateMailsMessagesAccess;
							}
							ActualMail=(HYAMNMAIL)cd->nmcd.lItemlParam;
							if (!ActualMail) 
								ActualMail=(HYAMNMAIL)readItemLParam(cd->nmcd.hdr.hwndFrom,cd->nmcd.dwItemSpec);
#ifdef DEBUG_SYNCHRO
							DebugLog(SynchroFile,"MailBrowser:DRAWITEM:ActualAccountMsgsSO-read wait\n");
#endif
							if (!umma)
								if (WAIT_OBJECT_0 != WaitToReadFcn(ActualAccount->MessagesAccessSO))
								{
#ifdef DEBUG_SYNCHRO
									DebugLog(SynchroFile,"MailBrowser:DRAWITEM:ActualAccountMsgsSO-read wait failed\n");
#endif
									return 0;
								}
#ifdef DEBUG_SYNCHRO
								DebugLog(SynchroFile,"MailBrowser:DRAWITEM:ActualAccountMsgsSO-read enter\n");
#endif
								switch(ActualMail->Flags & YAMN_MSG_SPAMMASK)
								{
								case YAMN_MSG_SPAML1:
								case YAMN_MSG_SPAML2:
									cd->clrText=RGB(150,150,150);
									break;
								case YAMN_MSG_SPAML3:
									cd->clrText=RGB(200,200,200);
									cd->clrTextBk=RGB(160,160,160);
									break;
								case 0:
									if (cd->nmcd.dwItemSpec & 1)
										cd->clrTextBk=RGB(230,230,230);
									break;
								default:
									break;
								}
								if (ActualMail->Flags & YAMN_MSG_UNSEEN)
									cd->clrTextBk=RGB(220,235,250);
								PaintCode=CDRF_DODEFAULT;

								if (!umma)
								{
#ifdef DEBUG_SYNCHRO
									DebugLog(SynchroFile,"MailBrowser:DRAWITEM:ActualAccountMsgsSO-read done\n");
#endif
									ReadDoneFcn(ActualAccount->MessagesAccessSO);
								}

								break;
						}
					}
					SetWindowLongPtr(hDlg,DWLP_MSGRESULT,PaintCode);
					return 1;
				}
			}
		}
		break;

	case WM_CONTEXTMENU:
		if ( GetWindowLongPtr(( HWND )wParam, GWLP_ID ) == IDC_LISTMAILS)	{
			//MessageBox(0,"LISTHEADERS","Debug",0);
			HWND hList = GetDlgItem( hDlg, IDC_LISTMAILS );
			POINT pt = { (signed short)LOWORD( lParam ), (signed short)HIWORD( lParam ) };
			HTREEITEM hItem = 0;
			if (pt.x==-1) pt.x = 0;
			if (pt.y==-1) pt.y = 0;
			if (int numRows = ListView_GetItemCount(hList)) {
				HMENU hMenu = CreatePopupMenu();
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)1, TranslateT("Copy Selected"));
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)2, TranslateT("Copy All"));
				AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
				AppendMenu(hMenu, MF_STRING, (UINT_PTR)0, TranslateT("Cancel"));
				int nReturnCmd = TrackPopupMenu( hMenu, TPM_RETURNCMD, pt.x, pt.y, 0, hDlg, NULL );
				DestroyMenu( hMenu );
				if (nReturnCmd>0) {
					int courRow=0;
					size_t sizeNeeded = 0;
					TCHAR from[128]={0}, subject[256]={0}, size[16]={0}, date[64]={0};
					for (courRow=0;courRow<numRows;courRow++) {
						if ((nReturnCmd==1) && (ListView_GetItemState(hList, courRow, LVIS_SELECTED)==0)) continue;
						ListView_GetItemText(hList, courRow, 0, from, SIZEOF(from));
						ListView_GetItemText(hList, courRow, 1, subject, SIZEOF(subject));
						ListView_GetItemText(hList, courRow, 2, size, SIZEOF(size));
						ListView_GetItemText(hList, courRow, 3, date, SIZEOF(date));
						sizeNeeded += 5+_tcslen(from)+_tcslen(subject)+_tcslen(size)+_tcslen(date);
					}
					if (sizeNeeded && OpenClipboard(hDlg)) {
						EmptyClipboard();
						HGLOBAL hData = GlobalAlloc(GMEM_MOVEABLE,(sizeNeeded+1)*sizeof(TCHAR));
						TCHAR *buff = (TCHAR *)GlobalLock(hData);
						int courPos = 0;
						for (courRow=0; courRow < numRows; courRow++) {
							if ((nReturnCmd==1) && (ListView_GetItemState(hList, courRow, LVIS_SELECTED)==0)) continue;
							ListView_GetItemText(hList, courRow, 0, from, SIZEOF(from));
							ListView_GetItemText(hList, courRow, 1, subject, SIZEOF(subject));
							ListView_GetItemText(hList, courRow, 2, size, SIZEOF(size));
							ListView_GetItemText(hList, courRow, 3, date, SIZEOF(date));
							courPos += mir_sntprintf(&buff[courPos], sizeNeeded + 1, _T("%s\t%s\t%s\t%s\r\n"), from, subject, size, date);
						}
						GlobalUnlock(hData);

						SetClipboardData(CF_UNICODETEXT,hData);

						CloseClipboard();
					}
				}
			}
		}
		break; // just in case
	}
	return 0;
}

LRESULT CALLBACK ListViewSubclassProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	HWND hwndParent = GetParent(hDlg);
	
	switch(msg) {
      case WM_GETDLGCODE :
       {
        LPMSG lpmsg;
          if ( ( lpmsg = (LPMSG)lParam ) != NULL ) {
             if ( lpmsg->message == WM_KEYDOWN
             && lpmsg->wParam == VK_RETURN)
           return DLGC_WANTALLKEYS;
           }
         break;
       }
		case WM_KEYDOWN:
        {
			
            BOOL isCtrl = GetKeyState(VK_CONTROL) & 0x8000;
            BOOL isShift = GetKeyState(VK_SHIFT) & 0x8000;
            BOOL isAlt = GetKeyState(VK_MENU) & 0x8000;

			switch (wParam) 
			{
				case 'A':  // ctrl-a
					if (!isAlt && !isShift && isCtrl) SendMessage(hwndParent,WM_COMMAND,IDC_BTNCHECKALL,0);
					break;
				case VK_RETURN:
				case VK_SPACE:
					if (!isAlt && !isShift && !isCtrl) SendMessage(hwndParent,WM_YAMN_SHOWSELECTED,0,0);
					break;
				case VK_DELETE:
					SendMessage(hwndParent,WM_COMMAND,IDC_BTNDEL,0);
					break;
			}
			
			break;

		}
	}
	return mir_callNextSubclass(hDlg, ListViewSubclassProc, msg, wParam, lParam);
}

DWORD WINAPI MailBrowser(LPVOID Param)
{
	MSG msg;

	HWND hMailBrowser;
	BOOL WndFound=FALSE;
	HACCOUNT ActualAccount;
	struct MailBrowserWinParam MyParam;

	MyParam=*(struct MailBrowserWinParam *)Param;
	ActualAccount=MyParam.account;	
	#ifdef DEBUG_SYNCHRO
	DebugLog(SynchroFile,"MailBrowser:Incrementing \"using threads\" %x (account %x)\n",ActualAccount->UsingThreads,ActualAccount);
	#endif
	SCIncFcn(ActualAccount->UsingThreads);

//	we will not use params in stack anymore
	SetEvent(MyParam.ThreadRunningEV);

	__try
	{
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"MailBrowser:ActualAccountSO-read wait\n");
		#endif
		if (WAIT_OBJECT_0 != WaitToReadFcn(ActualAccount->AccountAccessSO))
		{
			#ifdef DEBUG_SYNCHRO
			DebugLog(SynchroFile,"MailBrowser:ActualAccountSO-read wait failed\n");
			#endif
			return 0;
		}
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"MailBrowser:ActualAccountSO-read enter\n");
		#endif
		if (!(ActualAccount->AbilityFlags & YAMN_ACC_BROWSE))
		{
			MyParam.nflags=MyParam.nflags & ~YAMN_ACC_MSG;
			MyParam.nnflags=MyParam.nnflags & ~YAMN_ACC_MSG;
		}
		if (!(ActualAccount->AbilityFlags & YAMN_ACC_POPUP))
			MyParam.nflags=MyParam.nflags & ~YAMN_ACC_POP;
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"MailBrowser:ActualAccountSO-read done\n");
		#endif
		ReadDoneFcn(ActualAccount->AccountAccessSO);

		if (NULL != (hMailBrowser=WindowList_Find(YAMNVar.NewMailAccountWnd, (MCONTACT)ActualAccount)))
			WndFound=TRUE;
		if ((hMailBrowser==NULL) && ((MyParam.nflags & YAMN_ACC_MSG) || (MyParam.nflags & YAMN_ACC_ICO) || (MyParam.nnflags & YAMN_ACC_MSG)))
		{
			hMailBrowser=CreateDialogParamW(YAMNVar.hInst,MAKEINTRESOURCEW(IDD_DLGVIEWMESSAGES),NULL,DlgProcYAMNMailBrowser,(LPARAM)&MyParam);
			SendMessageW(hMailBrowser,WM_SETICON,(WPARAM)ICON_BIG,(LPARAM)g_LoadIconEx(2,true));
			SendMessageW(hMailBrowser,WM_SETICON,(WPARAM)ICON_SMALL,(LPARAM)g_LoadIconEx(2));
			MoveWindow(hMailBrowser,PosX,PosY,SizeX,SizeY,TRUE);
		}

		if (hMailBrowser != NULL)
		{
			struct CChangeContent Params={MyParam.nflags,MyParam.nnflags};	//if this thread created window, just post message to update mails

			SendMessageW(hMailBrowser,WM_YAMN_CHANGECONTENT,(WPARAM)ActualAccount,(LPARAM)&Params);	//we ensure this will do the thread who created the browser window
		}
		else
			UpdateMails(NULL,ActualAccount,MyParam.nflags,MyParam.nnflags);	//update mails without displaying or refreshing any window

		if ((hMailBrowser != NULL) && !WndFound)		//we process message loop only for thread that created window
		{
			while(GetMessage(&msg,NULL,0,0))
			{
				if (!IsDialogMessage(hMailBrowser, &msg))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}

		if ((!WndFound) && (ActualAccount->Plugin->Fcn != NULL) && (ActualAccount->Plugin->Fcn->WriteAccountsFcnPtr != NULL) && ActualAccount->AbleToWork)
			ActualAccount->Plugin->Fcn->WriteAccountsFcnPtr();
	}
	__finally
	{
		#ifdef DEBUG_SYNCHRO
		DebugLog(SynchroFile,"MailBrowser:Decrementing \"using threads\" %x (account %x)\n",ActualAccount->UsingThreads,ActualAccount);
		#endif
		SCDecFcn(ActualAccount->UsingThreads);
	}
	return 1;
}

INT_PTR RunMailBrowserSvc(WPARAM wParam,LPARAM lParam)
{
	DWORD tid;
	//an event for successfull copy parameters to which point a pointer in stack for new thread
	HANDLE ThreadRunningEV;
	PYAMN_MAILBROWSERPARAM Param=(PYAMN_MAILBROWSERPARAM)wParam;

	if ((DWORD)lParam != YAMN_MAILBROWSERVERSION)
		return 0;

	if (NULL != (ThreadRunningEV=CreateEvent(NULL,FALSE,FALSE,NULL)))
	{
		HANDLE NewThread;

		Param->ThreadRunningEV=ThreadRunningEV;
		if (NULL != (NewThread=CreateThread(NULL,0,MailBrowser,Param,0,&tid)))
		{
			WaitForSingleObject(ThreadRunningEV,INFINITE);
			CloseHandle(NewThread);
		}
		CloseHandle(ThreadRunningEV);
		return 1;
	}
	return 0;
}

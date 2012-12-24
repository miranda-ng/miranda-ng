#if !defined(AFX_SMS_CONSTANS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)
#define AFX_SMS_CONSTANS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_




#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000



// {CF97FD5D-B911-47a8-AF03-D21968B5B894}
#define  SMS_GUID  { 0xcf97fd5d, 0xb911, 0x47a8, { 0xaf, 0x3, 0xd2, 0x19, 0x68, 0xb5, 0xb8, 0x94 } }


#define MIN_MIR_VER_VERSION_DWORD			PLUGIN_MAKE_VERSION(0,8,0,0)


#define PLUGIN_VERSION_DWORD				PLUGIN_MAKE_VERSION(0,2,5,3)
#define PROTOCOL_NAMEA						"SMSPlugin"
#define PROTOCOL_NAMEW						TEXT(PROTOCOL_NAMEA)
#define PROTOCOL_NAME_LEN					(sizeof(PROTOCOL_NAMEA)-1)
#define PROTOCOL_NAME_SIZE					sizeof(PROTOCOL_NAMEA)
#define PROTOCOL_DISPLAY_NAME_ORIG			"SMS"
#define PROTOCOL_DISPLAY_NAME_ORIGW			TEXT(PROTOCOL_DISPLAY_NAME_ORIG)




#define TIMEOUT_MSGSEND						60000			// send sms timeout
#define MIN_SMS_DBEVENT_LEN					4				// для фильтрации событий авторизации от джабер плагина
#define MAX_PHONE_LEN						MAX_PATH
#define PHONES_MIN_COUNT					4 //internal	// колличество элементов обязательно проверяемых при извлечении email адреса из инфы юзера



#define SMS_DEFAULT_
#define SMS_DEFAULT_SIGNATUREPOS	FALSE
#define SMS_DEFAULT_SHOWACK			TRUE
#define SMS_DEFAULT_USESIGNATURE	TRUE
#define SMS_DEFAULT_AUTOPOP			FALSE
#define SMS_DEFAULT_SAVEWINPOS		FALSE



//
#define ICQEVENTTYPE_SMSCONFIRMATION 3001


//Fonts defenitions
//#define SRMMMOD 	"SRMM"
#define SRMMMOD 	"TabSRMM_Fonts"

#define FONTF_BOLD   1
#define FONTF_ITALIC 2
struct FontOptionsList
{
	COLORREF defColour;
	TCHAR*   szDefFace;
	BYTE     defStyle;
	char     defSize;
}

static fontOptionsList[] = {
	{ RGB(106, 106, 106), _T("Arial"),    0, -12},
};


#define MSGFONTID_MYMSG		  0
#define MSGFONTID_YOURMSG	  2
#define MSGFONTID_MESSAGEAREA 16


#define SRMSGSET_BKGCOLOUR			"BkgColour"
#define SRMSGSET_INPBKGCOLOUR		"inputbg"
#define SRMSGDEFSET_BKGCOLOUR		GetSysColor(COLOR_WINDOW)
#define FONTF_BOLD   1
#define FONTF_ITALIC 2



//Decleration of functions that used when user hit miranda for new message/confirmation
int ReadAckSMS(WPARAM wParam,LPARAM lParam);
int ReadMsgSMS(WPARAM wParam,LPARAM lParam);
int SendSMSMenuCommand(WPARAM wParam,LPARAM lParam);



#define SMS_READ					"/ReadSms"
#define SMS_READ_ACK				"/ReadSmsAck"
#define SMS_SEND					"/SendSMS"
#define SMS_SEND_STR				L"Send &SMS..."
#define SMS_SEND_CM_STR				L"&SMS Message"


static const SERVICE_ITEM siPluginServices[] =
{
	{ SMS_READ,				ReadMsgSMS },
	{ SMS_READ_ACK,			ReadAckSMS },
	{ SMS_SEND,				SendSMSMenuCommand },
};




#endif // !defined(AFX_SMS_CONSTANS_H__F58D13FF_F6F2_476C_B8F0_7B9E9357CF48__INCLUDED_)

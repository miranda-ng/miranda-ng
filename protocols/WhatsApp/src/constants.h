#if !defined(CONSTANTS_H)
#define CONSTANTS_H

// Version management
#define PRODUCT_NAME _T("WhatsApp Protocol")

// Limits
#define WHATSAPP_GROUP_NAME_LIMIT   420

// Defaults
#define DEFAULT_MAP_STATUSES			0
#define DEFAULT_SYSTRAY_NOTIFY      0

#define DEFAULT_EVENT_NOTIFICATIONS_ENABLE   1
#define DEFAULT_EVENT_FEEDS_ENABLE           1
#define DEFAULT_EVENT_OTHER_ENABLE           1
#define DEFAULT_EVENT_CLIENT_ENABLE          1
#define DEFAULT_EVENT_COLBACK                0x00ffffff
#define DEFAULT_EVENT_COLTEXT                0x00000000
#define DEFAULT_EVENT_TIMEOUT_TYPE           0
#define DEFAULT_EVENT_TIMEOUT                -1

// #TODO Move constants below to WhatsAPI++

// WhatsApp
#define WHATSAPP_LOGIN_SERVER "c.whatsapp.net"
#define ACCOUNT_USER_AGENT "WhatsApp/2.12.89 S40Version/14.26 Device/Nokia302"
#define ACCOUNT_URL_CODEREQUESTV2 "https://v.whatsapp.net/v2/code"
#define ACCOUNT_URL_REGISTERREQUESTV2 "https://v.whatsapp.net/v2/register"
#define ACCOUNT_URL_EXISTSV2 "https://v.whatsapp.net/v2/exist"

// WhatsApp Nokia 302 S40
#define ACCOUNT_RESOURCE  "S40-2.12.89"

#define WHATSAPP_RECV_MESSAGE 1
#define WHATSAPP_SEND_MESSAGE 2

#define MAX_SILENT_INTERVAL 55

// Event flags
#define WHATSAPP_EVENT_CLIENT          0x10000000 // WhatsApp error or info message
#define WHATSAPP_EVENT_NOTIFICATION    0x40000000 // WhatsApp notification
#define WHATSAPP_EVENT_OTHER           0x80000000 // WhatsApp other event - friend requests/new messages

#define IS_CHAT 1

#define REG_STATE_REQ_CODE 1
#define REG_STATE_REG_CODE 2

#endif
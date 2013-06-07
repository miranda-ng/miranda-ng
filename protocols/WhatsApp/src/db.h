// DB macros
#define getByte( setting, error )           db_get_b( NULL, m_szModuleName, setting, error )
#define setByte( setting, value )           db_set_b( NULL, m_szModuleName, setting, value )
#define getWord( setting, error )           db_get_w( NULL, m_szModuleName, setting, error )
#define setWord( setting, value )           db_set_w( NULL, m_szModuleName, setting, value )
#define getDword( setting, error )          db_get_dw( NULL, m_szModuleName, setting, error )
#define setDword( setting, value )          db_set_dw( NULL, m_szModuleName, setting, value )
#define getString( setting, dest )          db_get_s( NULL, m_szModuleName, setting, dest, DBVT_ASCIIZ )
#define setString( setting, value )         db_set_s( NULL, m_szModuleName, setting, value )
#define getTString( setting, dest )         DBGetContactSettingTString( NULL, m_szModuleName, setting, dest )
#define setTString( setting, value )        DBWriteContactSettingTString( NULL, m_szModuleName, setting, value )
#define getU8String( setting, dest )        DBGetContactSettingUTF8String( NULL, m_szModuleName, setting, dest )
#define setU8String( setting, value )       DBWriteContactSettingUTF8String( NULL, m_szModuleName, setting, value )
#define deleteSetting( setting )            DBDeleteContactSetting( NULL, m_szModuleName, setting )

// DB keys
#define WHATSAPP_KEY_ID                      "ID"
#define WHATSAPP_KEY_LOGIN                   "Login"
#define WHATSAPP_KEY_CC                      "CountryCode"
#define WHATSAPP_KEY_NICK                    "Nickname"
#define WHATSAPP_KEY_PASS                    "Password"
#define WHATSAPP_KEY_IDX                     "DeviceID"
#define WHATSAPP_KEY_MAP_STATUSES			   "MapStatuses"
#define WHATSAPP_KEY_LOGGING_ENABLE			   "LoggingEnable"
#define WHATSAPP_KEY_NAME                    "RealName"
#define WHATSAPP_KEY_PUSH_NAME               "Nick"
#define WHATSAPP_KEY_LAST_SEEN               "LastSeen"
#define WHATSAPP_KEY_LAST_MSG_ID_HEADER      "LastMsgIdHeader"
#define WHATSAPP_KEY_LAST_MSG_ID             "LastMsgId"
#define WHATSAPP_KEY_LAST_MSG_STATE          "LastMsgState"
#define WHATSAPP_KEY_AVATAR_ID               "AvatarId"
#define WHATSAPP_KEY_SYSTRAY_NOTIFY			   "UseSystrayNotify"
#define WHATSAPP_KEY_DEF_GROUP               "DefaultGroup"
#define WHATSAPP_KEY_REG_CODE                "RegistrationCode"

#define WHATSAPP_KEY_EVENT_CLIENT_ENABLE            "EventClientEnable"
#define WHATSAPP_KEY_EVENT_OTHER_ENABLE             "EventOtherEnable"

#define WHATSAPP_KEY_EVENT_CLIENT_COLBACK           "PopupClientColorBack"
#define WHATSAPP_KEY_EVENT_CLIENT_COLTEXT           "PopupClientColorText"
#define WHATSAPP_KEY_EVENT_CLIENT_TIMEOUT           "PopupClientTimeout"
#define WHATSAPP_KEY_EVENT_CLIENT_DEFAULT           "PopupClientColorDefault"

#define WHATSAPP_KEY_EVENT_OTHER_COLBACK            "PopupOtherColorBack"
#define WHATSAPP_KEY_EVENT_OTHER_COLTEXT            "PopupOtherColorText"
#define WHATSAPP_KEY_EVENT_OTHER_TIMEOUT            "PopupOtherTimeout"
#define WHATSAPP_KEY_EVENT_OTHER_DEFAULT            "PopupOtherColorDefault"


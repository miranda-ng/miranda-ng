#define pluginName "StopSpam" //const char* ?
/*
TCHAR const * defAnswer = _T("nospam");
TCHAR const * defCongratulation = 
_T("Congratulations! You just passed human/robot test. Now you can write me a message.");
char const * defProtoList = "ICQ\r\n";
TCHAR const * infTalkProtPrefix = _T("StopSpam automatic message:\r\n");
char const * answeredSetting = "Answered";
char const * questCountSetting = "QuestionCount";
TCHAR const * defAufrepl = _T("StopSpam: send a message and reply to a anti-spam bot question.");*/


#ifdef _UNICODE
typedef std::wstring tstring;
#define PREF_TCHAR2 PREF_UTF
#else
typedef std::string tstring;
#define PREF_TCHAR2 0
#endif //_UNICODE


#ifndef _WSX22_UTILS_LOGGER
#define _WSX22_UTILS_LOGGER

#define PREFIX_SIZE 6
typedef void (__cdecl *LogFunction)(const wchar_t* szText);


/**
 * Universal Logger
 *
 * Singleton pattern based on
 * http://www.codeproject.com/KB/threads/SingletonThreadSafety.aspx
 */
class MFLogger
{

public:

	//static method that returns only instance of MFLogger
	//////no thread safe so use it first time from only one thread (guaranted in mirfox)
	static MFLogger *
	getInstance() {
		//initialized always from one thread
		if (m_pOnlyOneInstance == NULL) {
			if (m_pOnlyOneInstance == NULL) {
				m_pOnlyOneInstance = new MFLogger();
			}
		}
		return m_pOnlyOneInstance;
	}

	MFLogger();

	void initLogger(LogFunction logFunction_p);
	void set6CharsPrefix(const wchar_t* prefix);
	void releaseLogger();

	void log(const wchar_t* szText);
	void log_p(const wchar_t* szText, ...);
	void log_d(const wchar_t* szText);
	void log_dp(const wchar_t* szText, ...);

private:

	void log_int(const wchar_t* szText);

	//holds one and only object of MySingleton
	static MFLogger* m_pOnlyOneInstance;

	wchar_t m_prefix[PREFIX_SIZE];
	LogFunction logFunction;

	CRITICAL_SECTION logCs;

};


#endif //#ifndef _WSX22_UTILS_LOGGER

//#define DEBUG_RELEASE 1

#ifdef DEBUG_RELEASE
  #define _DEBUG 1
#endif

#ifdef _DEBUG
	void init_debug(void);
    void end_debug (void);
	void do_log(const char *pszFormat, ...);
	#define DEBUG_OUT(a) OUTPUT(a)
	#define TRACE(a) OutputDebugString(a)
	#define TRACEA(a) OutputDebugStringA(a)
	#define TRACEW(a) OutputDebugStringW(a)
	#define LOG(a) do_log a
#else
	#define DEBUG_OUT(a) 
	#define LOG(a)
	#define TRACE(a)
	#define TRACEA(a)
	#define TRACEW(a)
#endif


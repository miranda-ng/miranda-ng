// adapted 23/9/2004 from public domain code at http://tangentsoft.net/wskfaq/examples/dllping.html

#ifndef _ICMP_H
#define _ICMP_H

typedef HANDLE (WINAPI* pfnHV)(VOID);
typedef BOOL (WINAPI* pfnBH)(HANDLE);
typedef DWORD (WINAPI* pfnDHDPWPipPDD)(HANDLE, HANDLE, FARPROC, PVOID, IPAddr, LPVOID, WORD, PIP_OPTION_INFORMATION, LPVOID, DWORD, DWORD);

class ICMP {
protected:
	pfnHV pIcmpCreateFile;
	pfnBH pIcmpCloseHandle;
	pfnDHDPWPipPDD pIcmpSendEcho2;

	HMODULE hDLL;

	HANDLE hIP;

	unsigned int timeout;
	bool functions_loaded;

	// protected constructor - singleton class
	ICMP();
	static ICMP *instance;

	char *buff;

public:
	~ICMP();
	static ICMP *get_instance();
	static void cleanup();

	bool ping(char *host, ICMP_ECHO_REPLY &reply);
	
	void set_timeout(unsigned int t) {
		timeout = t;
	}

	void stop();

	unsigned int get_timeout() {return timeout;}

};
#endif //_ICMP_H

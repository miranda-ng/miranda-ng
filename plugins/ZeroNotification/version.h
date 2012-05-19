#define VER_MAJOR 0
#define VER_MINOR 0
#define VER_RELEASE 0
#define VER_BUILD 5

#define PLUGINNAME_SHORT "ZeroNotify"
#if defined(_WIN64)
	#define PLUGINNAME "Zero Notifications (x64)"
#elif defined(_UNICODE)
	#define PLUGINNAME "Zero Notifications (Unicode)"
#else
	#define PLUGINNAME "Zero Notifications"
#endif
#define DESCRIPTION "Adds options to automatically disable notifications when a protocol is in a certain status."
#define AUTHOR "bidyut (<0.0.0.5), Maat"
#define COPYRIGHT "© 2005 bidyut, © 2009 Maat"

#if defined(_UNICODE)
	// {47D489D3-310D-4ef6-BD05-699FFFD5A4AA}
	#define UID { 0x47d489d3, 0x310d, 0x4ef6, { 0xbd, 0x5, 0x69, 0x9f, 0xff, 0xd5, 0xa4, 0xaa } }
#else
	// {C92FA511-EACE-4add-99B1-FDB7AA150E14}
	#define UID { 0xc92fa511, 0xeace, 0x4add, { 0x99, 0xb1, 0xfd, 0xb7, 0xaa, 0x15, 0xe, 0x14 } }
#endif

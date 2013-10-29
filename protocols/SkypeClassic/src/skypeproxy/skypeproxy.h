/*** Skype API ***/
//Messages
#define SKYPECONTROLAPI_ATTACH_SUCCESS					0
#define SKYPECONTROLAPI_ATTACH_PENDING_AUTHORIZATION	1
#define SKYPECONTROLAPI_ATTACH_REFUSED					2
#define SKYPECONTROLAPI_ATTACH_NOT_AVAILABLE			3
#define SKYPECONTROLAPI_ATTACH_API_AVAILABLE			0x8001

// Errors
#define MISC_ERROR				1
#define USER_NOT_FOUND			2
#define USER_NOT_ONLINE			3
#define USER_BLOCKED			4
#define TYPE_UNSUPPORTED		5
#define SENDER_NOT_FRIEND		6
#define SENDER_NOT_AUTHORIZED	7


/*** Debugging macros ***/
#define OUTPUT(a) printf("* %s\n", a);
#define LOG(a, b) printf("- %s: %s\n", a, b);
#define LOGL(a, b) printf("- %s: %d\n", a, b);

/*** Program settings ***/
#define PING_INTERVAL	10000	// Ping every 10000 msec to see if Skype is still available
#define USE_AUTHENTICATION 0x01	// Program supports authentication -> Comment to disable!

/*** Commands ***/
#define AUTHENTICATE	0x01
#define CAPABILITIES	0x02

/*** Sockets ***/
#pragma comment(lib, "ws2_32")

/*** Prototypes ***/
void WatchDogTimer(char *);
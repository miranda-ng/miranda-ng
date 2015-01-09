/*** Program settings ***/
#define USE_AUTHENTICATION 0x01	// Program supports authentication -> Comment to disable!
#define USE_DATASLOTS      0x02	// Supports Commands >=3

/*** Commands ***/
#define AUTHENTICATE	0x01
#define CAPABILITIES	0x02
#define MY_CAPABILITIES	0x03
#define OPEN_SLOT		0x04
#define DATA_SLOT		0x05
#define CLOSE_SLOT		0x06

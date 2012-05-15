#ifndef __STSHELPERS__
#define __STSHELPERS__
#define PF2_FROMTRIGGER					0x00002000
#define PF2_CURRENT						0x00001000
#define PF2_OFFLINE						0x00000800
#define ID_STATUS_CURRENT				41000
#define ID_STATUS_FROMTRIGGER			41001

#define MIN_STATUS					ID_STATUS_OFFLINE
#define MAX_STATUS					ID_STATUS_OUTTOLUNCH

#define SSIF_DONTSETSTATUS		0x01
#define SSIF_DONTSETMESSAGE		0x02
#define SSIF_USEAWAYSYS			0x04

typedef struct {
	char *szProto;
	int flags;
	int status;
	char *szMsg;
} SETSTATUSINFO;

// find out whether a proto just changed status
#define CONNECTIONTIMEOUT			6000 // same value as NewStatusNotify
// call this...
int InitProtoStatusChangedHandling();
// ...if you want to use this
int ProtoChangedStatus(char *szProto);

int Flag2Status(unsigned long flag); // only if flag is only one status
unsigned long Status2Flag(int status);
int Hlp_SetStatus(SETSTATUSINFO *ssi, int count);
void Hlp_FreeStatusInfo(SETSTATUSINFO *ssi, int count);

#endif

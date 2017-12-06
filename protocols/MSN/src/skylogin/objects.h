#include "common.h"

typedef struct
{
	int		Id;
	uint	Rank;
} ObjListInfos_S;

typedef struct
{
	uint	Family;
	uint	Id;
	union
	{
		Memory_U	Memory;
		uchar	Table[8];		/* Actually DOUBLE int */
		uint	Nbr;
		Host	Addr;
	} Value;
	ObjListInfos_S	ObjListInfos;
}	ObjectDesc;

typedef struct
{
	ushort		Cmd;
	ushort		PacketID;
	ushort		Reply2ID;
	uint		NbObj;
	ObjectDesc	*Objs;
}	SResponse;



#define	 OBJ_FAMILY_NBR		0x00
#define	 OBJ_FAMILY_TABLE	0x01
#define	 OBJ_FAMILY_NETADDR	0x02
#define	 OBJ_FAMILY_STRING	0x03
#define	 OBJ_FAMILY_BLOB	0x04
#define	 OBJ_FAMILY_OBJLIST	0x05
#define  OBJ_FAMILY_INTLIST	0x06

#define	 OBJ_ID_NODEID		0x0D
#define	 OBJ_ID_LPORT		0x10
#define	 OBJ_ID_UPTIME		0x01
#define	 OBJ_ID_STVL		0x23
#define	 OBJ_ID_2000		0x09
#define  OBJ_ID_SK			0x08
#define  OBJ_ID_ZBOOL1		0x0C
#define  OBJ_ID_REQCODE		0x00
#define  OBJ_ID_ZBOOL2		0x02
#define  OBJ_ID_USERNAME	0x04
#define  OBJ_ID_USERPASS	0x05
#define  OBJ_ID_MODULUS		0x21
#define  OBJ_ID_PLATFORM	0x31
#define  OBJ_ID_LANG		0x36
#define  OBJ_ID_VERSION		0x0D
#define  OBJ_ID_PUBADDR		0x0E
#define  OBJ_ID_MISCD		0x33
#define  OBJ_ID_OAUTH		0x3D
#define  OBJ_ID_PARTNERID   0x3E
#define	 OBJ_ID_STACKVER	0x0B
#define	 OBJ_ID_STACKTS		0x0C
#define	 OBJ_ID_PEERLPORT	0x10
#define	 OBJ_ID_PUBNETADDR	0x11
#define	 OBJ_ID_NBCONNECTED	0x09
#define	 OBJ_ID_LOGINANSWER	0x01
#define	 OBJ_ID_CIPHERDLOGD	0x24
#define	 OBJ_ID_LDUSER		0x00
#define	 OBJ_ID_LDEXPIRY	0x04
#define	 OBJ_ID_LDMODULUS	0x01
#define  OBJ_ID_ESAUTHANSWR	0x0A
#define  OBJ_ID_ESHASHLIST	0x35
#define  OBJ_ID_HASH		0x32
#define  OBJ_ID_DISPLAYNAME	0x34
#define  OBJ_ID_UBLOB		0x33
#define  OBJ_ID_INTERNALNAM	0x10
#define  OBJ_ID_BUDDYSTATUS	0x79	// BUDDYSTATUS
#define  OBJ_ID_AUTHCERT	0x03
#define  OBJ_ID_USER2SEARCH	0x00
#define  OBJ_ID_BCMID		0x00
#define  OBJ_ID_BCMVER		0x01
#define  OBJ_ID_SLOTID		0x00
#define	 OBJ_ID_SLOTNBSN	0x07
#define	 OBJ_ID_SLOTSNADDR	0x03
#define	 OBJ_ID_DIRBLOB		0x0B

#define  OBJ_ID_CIRNAME		0x14	// FULLNAME
#define  OBJ_ID_CILANG		0x24	// LANGUAGES
#define  OBJ_ID_CICOUNTRY	0x28	// COUNTRY
#define  OBJ_ID_CIREGION	0x2C	// PROVINCE
#define  OBJ_ID_CIVILLE		0x30	// CITY
#define  OBJ_ID_CILOCATION	0x03
#define  OBJ_ID_CIPSTN_BAL	0x12D	// PSTN_BALANCE
#define  OBJ_ID_CIPSTN_BALC	0x128	// PSTN_BALANCE_CURRENCY
#define  OBJ_ID_CIBIRTHDAY	0x1D	// BIRTHDAY
#define  OBJ_ID_CIPHONE_H	0x34	// PHONE_HOME
#define  OBJ_ID_CIPHONE_O	0x38	// PHONE_OFFICE
#define  OBJ_ID_CIPHONE_M	0x3C	// PHONE_MOBILE
#define  OBJ_ID_CIHOMEPAGE	0x44	// HOMEPAGE
#define  OBJ_ID_CIABOUT		0x48	// ABOUT
#define  OBJ_ID_CIALIASES	0x60	// ALIASES
#define  OBJ_ID_CIMOOD_TEXT	0x68	// MOOD_TEXT
#define  OBJ_ID_CIRICH_MOOD	0x334	// RICH_MOOD_TEXT
#define  OBJ_ID_CITIMEZONE	0x6D	// TIMEZONE

#define  OBJ_ID_AUTHEDBUDD  0x71	// NROF_AUTHED_BUDDIES
#define  OBJ_ID_RCVDAUTHREQ	0x50	// RECEIVEDAUTHREQUEST
#define  OBJ_ID_ISAUTHORIZD 0x5D	// ISAUTHORIZED / ISBLOCKED
//#define  OBJ_ID_DISPLAYNAME 0x84	// DISPLAYNAME
#define  OBJ_ID_ONLINESTAT  0x89	// ONLINESTATUS
#define  OBJ_ID_LASTONLINET 0x8D	// LASTONLINETIMESTAMP
#define  OBJ_ID_CISEX       0x21	// SEX (MALE=1, FEMALE=2, UNKNOWN=0)


#define  OBJ_ID_FWTESTID	0x19
#define  OBJ_ID_FWTESTER	0x11
#define  OBJ_ID_PINGER		0x02
#define  OBJ_ID_TESTED		0x02
#define  OBJ_ID_TESTER		0x00
#define  OBJ_ID_RELAY		0x08
#define  OBJ_ID_SESPROPOSER	0x01
#define  OBJ_ID_SESCHALLENG	0x09
#define  OBJ_ID_SID2DEC		0x03
#define  OBJ_ID_SOLVEDCHALL	0x0A
#define  OBJ_ID_USRDBLOB	0x05
#define  OBJ_ID_AESPART1	0x06
#define	 OBJ_ID_PEERLOGIN	0x00
#define  OBJ_ID_PEERSESSID	0x03

#define  OBJ_ID_GROUPID		0x269
#define  OBJ_ID_GROUPNAME	0x25C

void	WriteValue(uchar **BufferAddr, uint Value);
void	WriteObject(uchar **Buffer, ObjectDesc Object);
int		ManageObjects(uchar **Buffer, uint Size, SResponse *Response);
void	FreeResponse(SResponse *Response);

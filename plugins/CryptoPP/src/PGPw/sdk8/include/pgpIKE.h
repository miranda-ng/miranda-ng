/*____________________________________________________________________________
	Copyright (C) 2002 PGP Corporation
	All rights reserved.
	
	$Id: pgpIKE.h,v 1.39 2003/09/24 03:09:32 ajivsov Exp $
____________________________________________________________________________*/
#ifndef Included_PGPike_h	/* [ */
#define Included_PGPike_h

#include "pgpPubTypes.h"

PGP_BEGIN_C_DECLARATIONS

#if PGP_WIN32
# pragma pack(push, 8)
#endif

#ifdef _MSC_VER
#pragma warning (disable:4200)
#endif

typedef struct PGPikeContext *			PGPikeContextRef;

#define	kInvalidPGPikeContextRef		((PGPikeContextRef) NULL)
#define PGPikeContextRefIsValid( ref )	( (ref) != kInvalidPGPikeContextRef )

#define kPGPike_CommonPort				500
#define kPGPike_AuthMaximumKeySize		(512/8)
#define kPGPike_ESPMaximumKeySize		32		/* 256 bits */
#define kPGPike_MaxExplicitIVSize		16
#define kPGPike_MaxTransforms			3
#define kPGPike_DefaultKBLife			1048576	/* 1GB */
#define kPGPike_DefaultSecLife			86400	/* 1 Day */
#define kPGPike_UserDataSize			96
#define kPGPike_XAuthStringLen			128
#define kPGPike_XAuthMessageLen			256

typedef PGPByte	PGPipsecSPI[4];

enum PGPikeDOI_
{
	kPGPike_DOI_IKE				= 0,
	kPGPike_DOI_IPSEC			= 1,

	PGP_ENUM_FORCE( PGPikeDOI_ )
};
PGPENUM_TYPEDEF( PGPikeDOI_, PGPikeDOI );

enum PGPikeAlert_
{
	kPGPike_AL_None						= 0,
	
	/* Error Types */
	kPGPike_AL_InvalidPayload			= 1,
	kPGPike_AL_DOIUnsupported			= 2,
	kPGPike_AL_SituationUnsupported		= 3,
	kPGPike_AL_InvalidCookie			= 4,
	kPGPike_AL_InvalidMajorVersion		= 5,
	kPGPike_AL_InvalidMinorVersion		= 6,
	kPGPike_AL_InvalidExchange			= 7,
	kPGPike_AL_InvalidFlags				= 8,
	kPGPike_AL_InvalidMessageID			= 9,
	kPGPike_AL_InvalidProtocolID		= 10,
	kPGPike_AL_InvalidSPI				= 11,
	kPGPike_AL_InvalidTransform			= 12,
	kPGPike_AL_InvalidAttribute			= 13,
	kPGPike_AL_NoProposalChoice			= 14,
	kPGPike_AL_BadProposal				= 15,
	kPGPike_AL_PayloadMalformed			= 16,
	kPGPike_AL_InvalidKey				= 17,
	kPGPike_AL_InvalidID				= 18,
	kPGPike_AL_InvalidCertEncoding		= 19,
	kPGPike_AL_InvalidCert				= 20,
	kPGPike_AL_UnsupportedCert			= 21,
	kPGPike_AL_InvalidCertAuthority		= 22,
	kPGPike_AL_InvalidHash				= 23,
	kPGPike_AL_AuthenticationFailed		= 24,
	kPGPike_AL_InvalidSignature			= 25,
	kPGPike_AL_AddressNotification		= 26,
	kPGPike_AL_NotifySALifetime			= 27,
	kPGPike_AL_CertUnavailable			= 28,
	kPGPike_AL_UnsupportedExchange		= 29,
	kPGPike_AL_UnequalPayloadLengths	= 30,
	
	kPGPike_AL_NATTranslationFailure	= 16300,
	kPGPike_AL_LastErrorType			= 16301,
	
	/* Status Types */
	kPGPike_AL_Connected				= 16384,
	kPGPike_AL_ResponderLifetime		= 24576,	/* IPSEC DOI */
	kPGPike_AL_ReplayStatus				= 24577,	/* IPSEC DOI */
	kPGPike_AL_InitialContact			= 24578,	/* IPSEC DOI */

	PGP_ENUM_FORCE( PGPikeAlert_ )
};
PGPENUM_TYPEDEF( PGPikeAlert_, PGPikeAlert );

enum PGPikeInternalAlert_
{
	kPGPike_IA_None						= 0,
	
	kPGPike_IA_ResponseTimeout			= 1,
	kPGPike_IA_NoProposals				= 2,
	kPGPike_IA_NewPhase1SA				= 3,
	kPGPike_IA_NewPhase2SA				= 4,
	kPGPike_IA_DeadPhase1SA				= 5,
	kPGPike_IA_DeadPhase2SA				= 6,
	kPGPike_IA_TooManyExchanges			= 7,
	kPGPike_IA_XAuthSuccess				= 8,
	kPGPike_IA_XAuthFailed				= 9,

	PGP_ENUM_FORCE( PGPikeInternalAlert_ )
};
PGPENUM_TYPEDEF( PGPikeInternalAlert_, PGPikeInternalAlert );

enum PGPikePref_
{
	kPGPike_PF_None						= 0,
	
	kPGPike_PF_Expiration				= 1,
	kPGPike_PF_AllowedAlgorithms		= 2,
	kPGPike_PF_IKEProposals				= 3,
	kPGPike_PF_IPSECProposals			= 4,

	PGP_ENUM_FORCE( PGPikePref_ )
};
PGPENUM_TYPEDEF( PGPikePref_, PGPikePref );

enum PGPipsecEncapsulation_
{
	kPGPike_PM_None							= 0,
	
	kPGPike_PM_Tunnel						= 1,
	kPGPike_PM_Transport					= 2,
	kPGPike_PM_UDPencapsulatedTunnel		= 61443,
	kPGPike_PM_UDPencapsulatedTransport		= 61444,

	PGP_ENUM_FORCE( PGPipsecEncapsulation_ )
};
PGPENUM_TYPEDEF( PGPipsecEncapsulation_, PGPipsecEncapsulation );

enum PGPikeLifeType_
{
	kPGPike_LT_None				= 0,
	
	kPGPike_LT_Seconds			= 1,
	kPGPike_LT_Kilobytes		= 2,

	PGP_ENUM_FORCE( PGPikeLifeType_ )
};
PGPENUM_TYPEDEF( PGPikeLifeType_, PGPikeLifeType );

enum PGPipsecIdentity_
{
	kPGPike_ID_None				= 0,
	
	kPGPike_ID_IPV4_Addr,
	kPGPike_ID_FQDN,
	kPGPike_ID_UserFQDN,
	kPGPike_ID_IPV4_Addr_Subnet,
	kPGPike_ID_IPV6_Addr,
	kPGPike_ID_IPV6_Addr_Subnet,
	kPGPike_ID_IPV4_Addr_Range,
	kPGPike_ID_IPV6_Addr_Range,
	kPGPike_ID_DER_ASN1_DN,
	kPGPike_ID_DER_ASN1_GN,
	kPGPike_ID_Key_ID,				/* used for PGP fingerprint */

	PGP_ENUM_FORCE( PGPipsecIdentity_ )
};
PGPENUM_TYPEDEF( PGPipsecIdentity_, PGPipsecIdentity );

/* If it doesn't say supported, it isn't. */
enum PGPipsecAHTransformID_
{
	kPGPike_AH_None				= 0,
	
	kPGPike_AH_MD5				= 2,	/* supported */
	kPGPike_AH_SHA				= 3,	/* supported */
	kPGPike_AH_DES				= 4,
	kPGPike_AH_SHA2_256			= 5,	/* supported */
	kPGPike_AH_SHA2_384			= 6,	/* supported */
	kPGPike_AH_SHA2_512			= 7,	/* supported */

	PGP_ENUM_FORCE( PGPipsecAHTransformID_ )
};
PGPENUM_TYPEDEF( PGPipsecAHTransformID_, PGPipsecAHTransformID );

enum PGPipsecAuthAttribute_
{
	kPGPike_AA_None				= 0,
	
	kPGPike_AA_HMAC_MD5			= 1,	/* supported */
	kPGPike_AA_HMAC_SHA			= 2,	/* supported */
	kPGPike_AA_DES_MAC			= 3,
	kPGPike_AA_KPDK				= 4,
	kPGPike_AA_HMAC_SHA2_256	= 5,	/* supported */
	kPGPike_AA_HMAC_SHA2_384	= 6,	/* supported */
	kPGPike_AA_HMAC_SHA2_512	= 7,	/* supported */

	PGP_ENUM_FORCE( PGPipsecAuthAttribute_ )
};
PGPENUM_TYPEDEF( PGPipsecAuthAttribute_, PGPipsecAuthAttribute );

enum PGPipsecESPTransformID_
{
	kPGPike_ET_DES_IV64			= 1,	/* supported */
	kPGPike_ET_DES				= 2,	/* supported */
	kPGPike_ET_3DES				= 3,	/* supported */
	kPGPike_ET_RC5				= 4,
	kPGPike_ET_IDEA				= 5,
	kPGPike_ET_CAST				= 6,	/* supported */
	kPGPike_ET_Blowfish			= 7,
	kPGPike_ET_3IDEA			= 8,
	kPGPike_ET_DES_IV32			= 9,
	kPGPike_ET_RC4				= 10,
	kPGPike_ET_NULL				= 11,	/* supported */
	kPGPike_ET_AES				= 12,
	
	PGP_ENUM_FORCE( PGPipsecESPTransformID_ )
};
PGPENUM_TYPEDEF( PGPipsecESPTransformID_, PGPipsecESPTransformID );

/* IPCOMP is not supported by this version of PGPike */
enum PGPipsecIPCOMPTransformID_
{
	kPGPike_IC_None			= 0,
	
	kPGPike_IC_OUI			= 1,
	kPGPike_IC_Deflate		= 2,		/* supported */
	kPGPike_IC_LZS			= 3,		/* supported */
	kPGPike_IC_V42bis		= 4,

	PGP_ENUM_FORCE( PGPipsecIPCOMPTransformID_ )
};
PGPENUM_TYPEDEF( PGPipsecIPCOMPTransformID_, PGPipsecIPCOMPTransformID );

enum PGPipsecProtocol_
{
	kPGPike_PR_None			= 0,
	kPGPike_PR_IKE			= 1,
	kPGPike_PR_AH			= 2,
	kPGPike_PR_ESP			= 3,
	kPGPike_PR_IPCOMP		= 4,

	PGP_ENUM_FORCE( PGPipsecProtocol_ )
};
PGPENUM_TYPEDEF( PGPipsecProtocol_, PGPipsecProtocol );

enum PGPikeGroupID_
{
	kPGPike_GR_None					= 0,	/* supported */
	
	kPGPike_GR_MODPOne				= 1,	/* supported */
	kPGPike_GR_MODPTwo				= 2,	/* supported */
	kPGPike_GR_MODPFive				= 5,	/* supported */

	kPGPike_GR_ECSix				= 6,	/* supported */
	kPGPike_GR_ECSeven				= 7,	/* supported */

	kPGPike_GR_ECEight				= 8,	/* supported */
	kPGPike_GR_ECNine				= 9,	/* supported */

	kPGPike_GR_MODP2048				= 42048,/* unassigned */
	kPGPike_GR_MODP3072				= 43072,/* unassigned */
	kPGPike_GR_MODP4096				= 44096,/* unassigned */
	kPGPike_GR_MODP6144				= 46144,/* unassigned */
	kPGPike_GR_MODP8192				= 48192,/* unassigned */
	
	PGP_ENUM_FORCE( PGPikeGroupID_ )
};
PGPENUM_TYPEDEF( PGPikeGroupID_, PGPikeGroupID );

enum PGPikeCipher_
{
	kPGPike_SC_None				= 0,
	
	kPGPike_SC_DES_CBC			= 1,	/* supported */
	kPGPike_SC_IDEA_CBC			= 2,
	kPGPike_SC_Blowfish_CBC		= 3,
	kPGPike_SC_RC5_R16_B64_CBC	= 4,
	kPGPike_SC_3DES_CBC			= 5,	/* supported */
	kPGPike_SC_CAST_CBC			= 6,	/* supported */
	kPGPike_SC_AES_CBC			= 7,
	
	PGP_ENUM_FORCE( PGPikeCipher_ )
};
PGPENUM_TYPEDEF( PGPikeCipher_, PGPikeCipher );

enum PGPikeHash_
{
	kPGPike_HA_None				= 0,
	
	kPGPike_HA_MD5				= 1,	/* supported */
	kPGPike_HA_SHA1				= 2,	/* supported */
	kPGPike_HA_Tiger			= 3,
	kPGPike_HA_SHA2_256			= 4,
	kPGPike_HA_SHA2_384			= 5,
	kPGPike_HA_SHA2_512			= 6,
	
	PGP_ENUM_FORCE( PGPikeHash_ )
};
PGPENUM_TYPEDEF( PGPikeHash_, PGPikeHash );

enum PGPikeAuthMethod_
{
	kPGPike_AM_None							= 0,
	
	kPGPike_AM_PreSharedKey					= 1,	/* supported */
	kPGPike_AM_DSS_Sig						= 2,	/* supported */
	kPGPike_AM_RSA_Sig						= 3,	/* supported */
	kPGPike_AM_RSA_Encrypt					= 4,
	kPGPike_AM_RSA_Encrypt_R				= 5,
	
	kPGPike_AM_HAuth_InitRSA				= 64221,
	kPGPike_AM_HAuth_RespRSA				= 64222,
	kPGPike_AM_HAuth_InitDSS				= 64223,
	kPGPike_AM_HAuth_RespDSS				= 64224,
	
	kPGPike_AM_XAuth_InitPreShared			= 65001,
	kPGPike_AM_XAuth_RespPreShared			= 65002,
	kPGPike_AM_XAuth_InitDSS				= 65003,
	kPGPike_AM_XAuth_RespDSS				= 65004,
	kPGPike_AM_XAuth_InitRSA				= 65005,
	kPGPike_AM_XAuth_RespRSA				= 65006,
	kPGPike_AM_XAuth_InitRSAEncryption		= 65007,
	kPGPike_AM_XAuth_RespRSAEncryption		= 65008,
	kPGPike_AM_XAuth_InitRSAREncryption		= 65009,
	kPGPike_AM_XAuth_RespRSAREncryption		= 65010,
	
	PGP_ENUM_FORCE( PGPikeAuthMethod_ )
};
PGPENUM_TYPEDEF( PGPikeAuthMethod_, PGPikeAuthMethod );

enum PGPikeAuthStyle_
{
	kPGPike_AS_Normal						= 0,
	
	kPGPike_AS_XAuth						= 1,
	kPGPike_AS_HybridAuth					= 2,
	
	PGP_ENUM_FORCE( PGPikeAuthStyle_ )
};
PGPENUM_TYPEDEF( PGPikeAuthStyle_, PGPikeAuthStyle );

enum PGPikeXAuthType_
{
	kPGPike_XT_Generic						= 0,
	
	kPGPike_XT_RADIUS_CHAP					= 1,
	kPGPike_XT_OTP							= 2,
	kPGPike_XT_SKEY							= 3,
	
	PGP_ENUM_FORCE( PGPikeXAuthType_ )
};
PGPENUM_TYPEDEF( PGPikeXAuthType_, PGPikeXAuthType );

enum PGPikeEncapsulateMode_
{
	kPGPike_EM_Auto							= 0,
	
	kPGPike_EM_Always						= 1,
	kPGPike_EM_Never						= 2,
	
	PGP_ENUM_FORCE( PGPikeEncapsulateMode_ )
};
PGPENUM_TYPEDEF( PGPikeEncapsulateMode_, PGPikeEncapsulateMode );

typedef struct PGPipsecESPTransform
{
	PGPipsecESPTransformID		cipher;
	PGPUInt32					keyLength;
		/* cipher key bit size, must be 0 for all except AES */ 
	PGPipsecAuthAttribute		authAttr;
	PGPipsecEncapsulation		mode;
} PGPipsecESPTransform;

typedef struct PGPipsecAHTransform
{
	PGPipsecAHTransformID		authAlg;
	PGPipsecAuthAttribute		authAttr;
	PGPipsecEncapsulation		mode;
} PGPipsecAHTransform;

typedef struct PGPipsecIPCOMPTransform
{
	PGPipsecIPCOMPTransformID	compAlg;
} PGPipsecIPCOMPTransform;

typedef struct PGPikeTransform
{
	PGPikeAuthMethod		authMethod;
	PGPikeHash				hash;
	PGPikeCipher			cipher;
	PGPUInt32				keyLength;	/* cipher key bit size, must be 0 for all except AES */ 
	PGPikeGroupID			groupID;
} PGPikeTransform;

typedef struct PGPipsecTransform
{
	PGPBoolean				useESP;
	PGPipsecESPTransform	esp;
	
	PGPBoolean				useAH;
	PGPipsecAHTransform		ah;
	
	PGPBoolean				useIPCOMP;
	PGPipsecIPCOMPTransform	ipcomp;
	
	PGPikeGroupID			groupID;
} PGPipsecTransform;

typedef struct PGPipsecDOIParams
{
	PGPipsecSPI				inSPI;
	PGPipsecSPI				outSPI;
	PGPipsecProtocol		protocol;

	union
	{
		struct
		{
			PGPipsecAHTransform		t;
			PGPByte					inAuthKey[kPGPike_AuthMaximumKeySize];
			PGPByte					outAuthKey[kPGPike_AuthMaximumKeySize];
		} ah;
		
		struct
		{
			PGPipsecESPTransform	t;
			PGPByte					inESPKey[kPGPike_ESPMaximumKeySize];
			PGPByte					outESPKey[kPGPike_ESPMaximumKeySize];
			PGPByte					inAuthKey[kPGPike_AuthMaximumKeySize];
			PGPByte					outAuthKey[kPGPike_AuthMaximumKeySize];
			PGPByte					explicitIV[kPGPike_MaxExplicitIVSize];
		} esp;
		
		struct
		{
			PGPipsecIPCOMPTransform	t;
		} ipcomp;
	} u;	
} PGPipsecDOIParams;

typedef struct PGPipsecSA
{
	struct PGPipsecSA *		nextSA;				/* INTERNAL USE ONLY */
	struct PGPipsecSA *		prevSA;				/* INTERNAL USE ONLY */
	PGPUInt32				ipAddress;			/* PreNATted(Original) IP Address */
	PGPBoolean				destIsRange;		/* dest is IP range */
	PGPUInt32				ipAddrStart;		/* IP address */
	PGPUInt32				ipMaskEnd;			/* mask or end range IP address */
	PGPByte					ipProtocol;			/* 0 if all protocols */
	PGPUInt16				ipPort;				/* 0 if all ports */
	
	PGPBoolean				bNATTraversal;		/* NAT Encapsulation is enabled if true */
	PGPBoolean				bIsLocalIPNAT;		/* if non-zero, our local IP is natted */
	PGPBoolean				bIsRemoteIPNAT;		/* if non-zero, remote IP is NATed */
	PGPUInt32				natIPAddress;		/* NATed Dest IP Address, 0 if not NATed */
	PGPUInt16				nboNATPort;			/* NATed Dest Port value */

	PGPUInt32				assignedIP;			/* if non-zero, use as tunnel IP */
	PGPUInt32				assignedDNS;		/* if non-zero, use as tunnel DNS */
	PGPUInt32				assignedWINS;		/* if non-zero, use as tunnel WINS */
	
	PGPBoolean				initiator;			/* was this SA initiated locally */
	PGPBoolean				activeIn;			/* use for inbound data */
	PGPBoolean				activeOut;			/* use for outbound data */
	PGPUInt32				kbLifeTime;			/* max KB to be sent on this SA */
	PGPUInt32				secLifeTime;		/* max seconds this SA will live */
												/* 0 means no limit for either */
	PGPTime					birthTime;
	
	PGPUInt16				numTransforms;
	PGPipsecDOIParams		transform[kPGPike_MaxTransforms];
	
	PGPTime					termSchedule;	/* SA will be terminated, private */
	PGPByte					userData[kPGPike_UserDataSize];/* for your use */
} PGPipsecSA;

/*	When sent a kPGPike_MT_SARequest, pgpIKE will expect that
	all fields below will be filled in.  The approved member is
	irrelevant in that case.
	
	When you are called with kPGPike_MT_PolicyCheck, pgpIKE will
	set everything to 0 except the IP address.  You are expected
	to fill in the other fields with local policy.  The PolicyCheck
	is used for remote-initiated SA negotiation.  Since you cannot
	predict which remote network might be trying to communicate
	with us, you should leave the destIsRange/ipAddrStart/ipMaskEnd
	and ipPort/ipProtocol fields set to 0 for this message.  You
	will be called to check those later with the ClientIDCheck.
*/
typedef struct PGPikeMTSASetup
{
	PGPBoolean				approved;			/* >  */
	
	PGPUInt32				ipAddress;			/* destination */
	PGPUInt32				localIPAddress;		/* source */
	PGPByte					ipProtocol;
	PGPUInt16				ipPort;
	PGPByte *				sharedKey;			/* null if none, data will be copied */
	PGPSize					sharedKeySize; 		/* w/o NULL-terminate */
	PGPBoolean				aggressive;			/* set for aggressive mode */
	PGPBoolean				lightweight;		/* only for SARequest, set to
													TRUE for only 1 retry */
	/* set the fields below to use tunnel mode */
	PGPBoolean				destIsRange;
	PGPUInt32				ipAddrStart;
	PGPUInt32				ipMaskEnd;
	
	PGPBoolean				virtualIP;			/* whether to use mode-cfg */
	PGPikeAuthStyle			authStyle;			/* whether to use xauth/hybrid */
	PGPikeEncapsulateMode	encapsulateMode;	/* Force NAT Traversal mode */
	
	PGPipsecIdentity		idType;				/* only useful in shared key mode */
	PGPByte *				idData;				/* data will be copied */
	PGPSize					idDataSize;			/* this is the Phase 1 ID */
} PGPikeMTSASetup;

/*	
	When you are called with a kPGPike_MT_ClientIDCheck, all fields
	will be set appropriately.  If the destIsRange/ipAddrStart/ipMaskEnd
	triad is permitted to be represented by the IP address, set
	the approved field to true.  This message is only sent in the case
	where the remote side is the initiator and you have already been
	called with a kPGPike_MT_PolicyCheck message.
*/
typedef struct PGPikeMTClientIDCheck
{
	PGPBoolean				approved;		/* >  */
	
	PGPUInt32				ipAddress;
	PGPByte					ipProtocol;
	PGPUInt16				ipPort;
	
	PGPBoolean				destIsRange;
	PGPUInt32				ipAddrStart;
	PGPUInt32				ipMaskEnd;
} PGPikeMTClientIDCheck;

typedef struct PGPikeMTSAEstablished
{
	PGPipsecSA *			sa;
	PGPBoolean				remoteValid;
	PGPByte *				remoteAuthKey;		/* binary exported key data */
	PGPSize					remoteAuthKeySize;
} PGPikeMTSAEstablished;

typedef struct PGPikeMTSAFailed
{
	PGPUInt32				ipAddress;		/* destination */
	PGPByte					ipProtocol;
	PGPUInt16				ipPort;
	
	PGPBoolean				destIsRange;
	PGPUInt32				ipAddrStart;
	PGPUInt32				ipMaskEnd;
	
} PGPikeMTSAFailed;

typedef struct PGPikeMTCert
{
	PGPUInt32				ipAddress;		/*  < */
	PGPKeyDBRef				baseKeyDB;		/* >  */
	PGPKeyDBObjRef			authObj;		/* >  PGP key or X.509 cert */
	
	PGPBoolean				isPassKey;		/* >  */
	void *					pass;			/* >  null-term if passphrase, Unicode, copied */
	PGPSize					passLength;		/* >  in PGPChar8s */
} PGPikeMTCert;

typedef struct PGPikeMTRemoteCert
{
	PGPBoolean				approved;		/* >  */
	PGPBoolean				valid;			/* >  */
	PGPUInt32				ipAddress;		/*  < */
	PGPKeyDBObjRef			remoteObj;		/*  < PGPkey or X.509 cert */
	PGPKeyDBRef				remoteKeyDB;	/*  < any other keys/certs are part of
													this cert's chain */
} PGPikeMTRemoteCert;

typedef struct PGPikeMTPacket
{
	PGPUInt32				ipAddress;	/* source or destination */
	PGPUInt16				port;		/* usually UDP 500, but might not be */
	PGPSize					packetSize;
	PGPByte *				packet;		/* msg *sender* must free this */
} PGPikeMTPacket;

typedef struct PGPikeMTIdentity
{
	PGPBoolean				active;			/* TRUE = set it, FALSE= dead */
	PGPUInt32				ipAddress;		/* Gateway IP */
	PGPUInt32				assignedIP;
	PGPUInt32				assignedDNS;
	PGPUInt32				assignedWINS;
} PGPikeMTIdentity;

typedef struct PGPikeMTAlert
{
	PGPikeAlert				alert;
	PGPUInt32				ipAddress;
	PGPikeInternalAlert		value;	/* used if alert is kPGPike_AL_None */
	PGPBoolean				remoteGenerated;
} PGPikeMTAlert;

typedef PGPUInt32 PGPikeAESKeyLengthMask;

#define kPGPike_AESKeyLength128 1
#define kPGPike_AESKeyLength192 2
#define kPGPike_AESKeyLength256 4
#define kPGPike_AESKeyLengthAll	7

typedef struct PGPikeAllowedAlgorithms
{
	PGPBoolean				cast5;
	PGPBoolean				tripleDES;
	PGPBoolean				singleDES;
	PGPikeAESKeyLengthMask	aes;
	PGPBoolean				espNULL;
	
	PGPBoolean				sha1;
	PGPBoolean				md5;
	PGPBoolean				sha2_256;
	PGPBoolean				sha2_384;
	PGPBoolean				sha2_512;
	PGPBoolean				noAuth;
		
	PGPBoolean				lzs;
	PGPBoolean				deflate;
	
	PGPBoolean				modpOne768;
	PGPBoolean				modpTwo1024;
	PGPBoolean				modpFive1536;

	PGPBoolean				ec2n163;
	PGPBoolean				ec2n283;

	PGPBoolean				modp2048;
	PGPBoolean				modp3072;
	PGPBoolean				modp4096;
	PGPBoolean				modp6144;
	PGPBoolean				modp8192;

} PGPikeAllowedAlgorithms;

typedef struct PGPikeMTPref
{
	PGPikePref					pref;
	
	union
	{
		struct
		{
			PGPUInt32			kbLifeTimeIKE;
			PGPUInt32			secLifeTimeIKE;
			PGPUInt32			kbLifeTimeIPSEC;
			PGPUInt32			secLifeTimeIPSEC;
		} expiration;			/* kPGPike_PF_Expiration */
		
		struct
		{
			PGPUInt32			numTransforms;
			PGPikeTransform *	t;
		} ikeProposals;			/* kPGPike_PF_IKEProposals */

		
		struct
		{
			PGPUInt32			numTransforms;
			PGPipsecTransform *	t;
		} ipsecProposals;		/* kPGPike_PF_IPSECProposals */
		
		PGPikeAllowedAlgorithms	allowedAlgorithms;
			/*kPGPike_PF_AllowedAlgorithms*/
	} u;
} PGPikeMTPref;

/* This message will be sent to check extended authentication. The buffer
	will be freed after your callback is called, so if you need it,
	copy the data elsewhere. You must retrieve the information and then
	send an AuthCheck message back to IKE with the contents filled in,
	and all contents preserved other than what you filled in. The message
	you send back is your memory, IKE will not free it. */
typedef struct PGPikeMTAuthCheck
{
	PGPBoolean			success;		/* set to true unless user aborts */
	PGPUInt32			gatewayIP;
	PGPikeXAuthType		xauthType;
	PGPUInt16			transactionID;	/* private */
	PGPBoolean			includeType;	/* private */
	
	PGPBoolean			useUserName;
	PGPBoolean			usePassword;
	PGPBoolean			usePasscode;
	PGPBoolean			useMessage;
	PGPBoolean			useChallenge;
	PGPBoolean			useDomain;
	
	PGPChar8			userName[kPGPike_XAuthStringLen];
	PGPChar8			password[kPGPike_XAuthStringLen];
	PGPChar8			passcode[kPGPike_XAuthStringLen];
	PGPChar8			message[kPGPike_XAuthMessageLen];
	PGPChar8			challenge[kPGPike_XAuthStringLen];
	PGPChar8			domain[kPGPike_XAuthStringLen];
	PGPSize				challengeSize;
} PGPikeMTAuthCheck;

enum PGPikeMessageType_
{
	/*
		Message types followed by ">" may be sent to PGPike
		Message typed followed by "<" may be sent by PGPike
	*/
	kPGPike_MT_Idle			= 0,	/* >  call this often		*/
	kPGPike_MT_SARequest,			/* >  PGPikeMTSASetup		*/
	kPGPike_MT_SARequestFailed,		/*  < PGPikeMTSAFailed		*/
	kPGPike_MT_SAEstablished,		/*  < PGPikeMTSAEstablished	*/
	kPGPike_MT_SARekey,				/* >  PGPipsecSA			*/
	kPGPike_MT_SADied,				/* >< PGPipsecSA			*/
	kPGPike_MT_SAUpdate,			/*  < PGPipsecSA			*/
	kPGPike_MT_SAKillAll,			/* >  none					*/
	
	kPGPike_MT_PolicyCheck,			/*  < PGPikeMTSASetup		*/
	kPGPike_MT_ClientIDCheck,		/*  < PGPikeMTClientIDCheck	*/
	kPGPike_MT_AuthCheck,			/* >< PGPikeMTAuthCheck		*/	
	kPGPike_MT_LocalPGPCert,		/*  < PGPikeMTCert			*/
	kPGPike_MT_LocalX509Cert,		/*  < PGPikeMTCert			*/
	kPGPike_MT_RemoteCert,			/*  < PGPikeMTRemoteCert	*/
	kPGPike_MT_Identity,			/*  < PGPikeMTIdentity		*/
	
	kPGPike_MT_Packet,				/* >< PGPikeMTPacket		*/
	kPGPike_MT_Alert,				/*  < PGPikeMTAlert			*/
	
	kPGPike_MT_Pref,				/* >  PGPikeMTPref			*/
	
	kPGPike_MT_DebugLog,			/*  < PGPChar8 *			*/

	PGP_ENUM_FORCE( PGPikeMessageType_ )
};
PGPENUM_TYPEDEF( PGPikeMessageType_, PGPikeMessageType );

typedef PGPError (* PGPikeMessageProcPtr)(
					PGPikeContextRef		ike,
					void *					inUserData,
					PGPikeMessageType		msg,
					void *					data );

PGPError 	PGPNewIKEContext(
					PGPContextRef			context,
					PGPikeMessageProcPtr	ikeMessageProc,
					void *					inUserData,
					PGPikeContextRef *		outRef );

PGPError 	PGPFreeIKEContext(
					PGPikeContextRef		ref );

/*	Any error from PGPikeProcessMessage is fatal.  Non-fatal
	errors are sent through the kPGPike_MT_Alert message.  */

PGPError	PGPikeProcessMessage(
					PGPikeContextRef		ref,
					PGPikeMessageType		msg,
					void *					data );


#if PGP_WIN32
# pragma pack(pop)
#endif

PGP_END_C_DECLARATIONS

#endif /* ] Included_PGPike_h */


/*__Editor_settings____

	Local Variables:
	tab-width: 4
	End:
	vi: ts=4 sw=4
	vim: si
_____________________*/

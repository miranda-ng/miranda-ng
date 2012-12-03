/*
Fingerprint NG (client version) icons module for Miranda NG
Copyright © 2006-12 ghazan, mataes, HierOS, FYR, Bio, nullbie, faith_healer and all respective contributors.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include "global.h"

/************************************************************************/
/* This file contains data about appropriate MirVer values				*/
/************************************************************************/

/*
*	NOTE: Masks can contain '*' or '?' wild symbols
*	Asterics '*' symbol covers 'empty' symbol too e.g WildCompare("Tst","T*st*"), returns TRUE
*	In order to handle situation 'at least one any sybol' use '?*' combination:
*	e.g WildCompare("Tst","T?*st*"), returns FALSE, but both WildCompare("Test","T?*st*") and
*	WildCompare("Teeest","T?*st*") return TRUE.
*
*	Function is 'dirt' case insensitive (it is ignore 5th bit (0x20) so it is no difference
*	beetween some symbols. But it is faster than valid converting to uppercase.
*
*	Mask can contain several submasks. In this case each submask (including first)
*	should start from '|' e.g: "|first*submask|second*mask".
*
*	ORDER OF RECORDS IS IMPORTANT: system search first suitable mask and returns it.
*	e.g. if MirVer is "Miranda IM" and first mask is "*im*" and second is "Miranda *" the
*	result will be client associated with first mask, not second!
*	So in order to avoid such situation, place most generalised masks to latest place.
*
*	In order to get "Unknown" client, last mask should be "?*".
*/

KN_FP_MASK def_kn_fp_mask[] =
{//	{"Client_IconName",			_T("|^*Mask*|*names*"),						_T("Icon caption"),						_T("iconpack name"),			IDI_RESOURCE_ID,		CLIENT_CASE,	OVERLAY?	},
//########################################################################################################################################################################################################
//#################################		MIRANDA		######################################################################################################################################################
//########################################################################################################################################################################################################

	{"client_Miranda_NG_st",	_T("|Miranda*NG*0.92.3.1504*")
								_T("|Miranda*NG*0.93.1.1649*")
								_T("|Miranda*NG*0.93.2.1746*")
								_T("|Miranda*NG*0.93.3.2110*")
								_T("|Miranda*NG*0.93.4.2450*")
								_T("|Miranda*NG*stable*"), 					_T("MirandaNG stable"),					_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG_STABLE,	MIRANDA_CASE	},

	{"client_Miranda_NG",		_T("|*nightly.miranda.im*")
								_T("|*Miranda*NG*"),						_T("MirandaNG"),						_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG,			MIRANDA_CASE	},

	{"client_Miranda_010xx",	_T("Miranda*IM*0.10.*"),					_T("MirandaIM v0.10.x.x"),				_T("ClientIcons_Miranda"),		IDI_MIRANDA_010X,		MIRANDA_CASE	},
	{"client_Miranda_09XX",		_T("|*Miranda*IM*0.9*")
								_T("|*miranda-im.org/caps#*0.9*"),			_T("MirandaIM v0.9.x.x"),				_T("ClientIcons_Miranda"),		IDI_MIRANDA_09XX,		MIRANDA_CASE	},
	{"client_Miranda_08XX",		_T("|^Miranda*0.7*Jabb*")
								_T("|*Miranda*0.8*")
								_T("|*miranda-im.org/caps#*0.8*"),			_T("MirandaIM v0.8.x.x"),				_T("ClientIcons_Miranda"),		IDI_MIRANDA_08XX,		MIRANDA_CASE	},
	{"client_Miranda_07XX",		_T("|*Miranda*0.7*")
								_T("|*miranda-im.org/caps#*0.7*"),			_T("MirandaIM v0.7.x.x"),				_T("ClientIcons_Miranda"),		IDI_MIRANDA_07XX,		MIRANDA_CASE	},
	{"client_Miranda_06XX",		_T("*Miranda*0.6*"),						_T("MirandaIM v0.6.x.x"),				_T("ClientIcons_Miranda"),		IDI_MIRANDA_06XX,		MIRANDA_CASE	},
	{"client_Miranda_05XX",		_T("*Miranda*0.5*"),						_T("MirandaIM v0.5.x.x"),				_T("ClientIcons_Miranda"),		IDI_MIRANDA_05XX,		MIRANDA_CASE	},
	{"client_Miranda_04XX",		_T("*Miranda*0.4*"),						_T("MirandaIM v0.4.x.x"),				_T("ClientIcons_Miranda"),		IDI_MIRANDA_04XX,		MIRANDA_CASE	},

	{"client_Miranda_old",		_T("*Miranda*0.?*"),						_T("MirandaIM (old versions)"),			_T("ClientIcons_Miranda"),		IDI_MIRANDA_OLD,		MIRANDA_CASE	},

	{"client_Miranda_Unknown",	_T("*Miranda*"),							_T("Miranda (unknown)"),				_T("ClientIcons_Miranda"),		IDI_MIRANDA_UNKNOWN,	MIRANDA_CASE	},

//########################################################################################################################################################################################################
//#################################		MULTI-PROTOCOL	##################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_1&1",				_T("|*Pocket*Web*|*1&1*|*1*?nd*1"),			_T("1&1"),								_T("ClientIcons_multiproto"),	IDI_1UND1,				MULTI_CASE,		TRUE	},
	{"client_Asia",				_T("*Asia*"),								_T("Asia"),								_T("ClientIcons_multiproto"),	IDI_ASIA,				MULTI_CASE,		TRUE	},
	{"client_Adium",			_T("|*Adium*")
								_T("|*VUFD6HcFmUT2NxJkBGCiKlZnS3M=*")		// Gabble client?
								_T("|*DdnydQG7RGhP9E3k9Sf+b+bF0zo=*"),		_T("Adium"),							_T("ClientIcons_multiproto"),	IDI_ADIUM,				MULTI_CASE,		TRUE	},
	{"client_AgileMessenger",	_T("*Agile Messenger*"),					_T("Agile Messenger"),					_T("ClientIcons_multiproto"),	IDI_AGILE,				MULTI_CASE,		TRUE	},

	{"client_Appolo",			_T("*Appolo*"),								_T("Appolo IM"),						_T("ClientIcons_multiproto"),	IDI_APPOLO,				MULTI_CASE,		TRUE	},
	{"client_Ayttm",			_T("*Ayttm*"),								_T("Ayttm"),							_T("ClientIcons_multiproto"),	IDI_AYTTM,				MULTI_CASE,		TRUE	},
	{"client_BayanICQ",			_T("|*Bayan*ICQ*|*barobin*"),				_T("BayanICQ"),							_T("ClientIcons_multiproto"),	IDI_BAYANICQ,			MULTI_CASE,		TRUE	},
	{"client_Beejive",			_T("*Beejive*"),							_T("BeejiveIM"),						_T("ClientIcons_multiproto"),	IDI_BEEJIVE,			MULTI_CASE,		TRUE	},
	{"client_Bimoid",			_T("*Bimoid*"),								_T("Bimoid"),							_T("ClientIcons_multiproto"),	IDI_BIMOID,				MULTI_CASE,		TRUE	},
	{"client_BitlBee",			_T("*BitlBee*"),							_T("BitlBee"),							_T("ClientIcons_multiproto"),	IDI_BITLBEE,			MULTI_CASE,		TRUE	},
	{"client_BlackBerry",		_T("|*Berry*|*ICS?\?\?\?\?\?\?\?"),			_T("BlackBerry"),						_T("ClientIcons_multiproto"),	IDI_BLACKBERRY,			MULTI_CASE,		TRUE	},
	{"client_Buddycloud",		_T("*Buddycloud*"),							_T("Buddycloud"),						_T("ClientIcons_multiproto"),	IDI_BUDDYCLOUD,			MULTI_CASE,		TRUE	},
	{"client_Carrier",			_T("*Carrier*"),							_T("Carrier client"),					_T("ClientIcons_multiproto"),	IDI_CARRIER,			MULTI_CASE,		TRUE	},
	{"client_Causerie",			_T("*Causerie*"),							_T("Causerie"),							_T("ClientIcons_multiproto"),	IDI_CAUSERIE,			MULTI_CASE,		TRUE	},
	{"client_CenterIM",			_T("*CenterIM*"),							_T("CenterIM"),							_T("ClientIcons_multiproto"),	IDI_CENTERIM,			MULTI_CASE,		TRUE	},
	{"client_ChatSecure",		_T("*ChatSecure*"),							_T("ChatSecure"),						_T("ClientIcons_multiproto"),	IDI_CHATSECURE,			MULTI_CASE,		TRUE	},
	{"client_Citron",			_T("*Citron*"),								_T("Citron"),							_T("ClientIcons_multiproto"),	IDI_CITRON,				MULTI_CASE,		TRUE	},
	{"client_climm",			_T("*climm*"),								_T("climm"),							_T("ClientIcons_multiproto"),	IDI_CLIMM,				MULTI_CASE,		TRUE	},
	{"client_Digsby",			_T("*Digsby*"),								_T("Digsby"),							_T("ClientIcons_multiproto"),	IDI_DIGSBY,				MULTI_CASE,		TRUE	},
	{"client_EKG2",				_T("*EKG*2*"),								_T("EKG2"),								_T("ClientIcons_multiproto"),	IDI_EKG2,				MULTI_CASE,		TRUE	},
	{"client_EasyMessage",		_T("Easy*Message*"),						_T("Easy Message"),						_T("ClientIcons_multiproto"),	IDI_EASYMESSAGE,		MULTI_CASE,		TRUE	},
	{"client_Empathy",			_T("*Empathy*"),							_T("Empathy"),							_T("ClientIcons_multiproto"),	IDI_EMPATHY,			MULTI_CASE,		TRUE	},
	{"client_Eyeball",			_T("*Eyeball*"),							_T("Eyeball Chat"),						_T("ClientIcons_multiproto"),	IDI_EYEBALL_CHAT,		MULTI_CASE,		TRUE	},
	{"client_eBuddy",			_T("|*eBuddy*|*eMessenger*"),				_T("eBuddy Messenger"),					_T("ClientIcons_multiproto"),	IDI_EBUDDY,				MULTI_CASE,		TRUE	},
	{"client_eM_Client",		_T("*eM*Client*"),							_T("eM Client"),						_T("ClientIcons_multiproto"),	IDI_EM_CLIENT,			MULTI_CASE,		TRUE	},
	{"client_eQo",				_T("*eQo*"),								_T("eQo"),								_T("ClientIcons_multiproto"),	IDI_EQO,				MULTI_CASE,		TRUE	},
	{"client_emesene",			_T("*emesene*"),							_T("emesene"),							_T("ClientIcons_multiproto"),	IDI_EMESENE,			MULTI_CASE,		TRUE	},
	{"client_Fama",				_T("*Fama*"),								_T("Fama IM"),							_T("ClientIcons_multiproto"),	IDI_FAMAIM,				MULTI_CASE,		TRUE	},
	{"client_Fring",			_T("*fring*"),								_T("Fring"),							_T("ClientIcons_multiproto"),	IDI_FRING,				MULTI_CASE,		TRUE	},
	{"client_GMX",				_T("*GMX*"),								_T("GMX MultiMessenger"),				_T("ClientIcons_multiproto"),	IDI_GMX,				MULTI_CASE,		TRUE	},
	{"client_Gaim",				_T("*gaim*"),								_T("Gaim (libgaim)"),					_T("ClientIcons_multiproto"),	IDI_GAIM,				MULTI_CASE,		TRUE	},
	{"client_Galaxium",			_T("*Galaxium*"),							_T("Galaxium"),							_T("ClientIcons_multiproto"),	IDI_GALAXIUM,			MULTI_CASE,		TRUE	},
	{"client_GnuGadu",			_T("Gnu Gadu*"),							_T("GNU Gadu"),							_T("ClientIcons_multiproto"),	IDI_GNUGADU,			MULTI_CASE,		TRUE	},
	{"client_IM2",				_T("*IM2*"),								_T("IM2"),								_T("ClientIcons_multiproto"),	IDI_IM2,				MULTI_CASE,		TRUE	},
	{"client_IMPlus",			_T("|IM+*|IMPLUS*|*IM plus*|@mobile")
								_T("|*+umBU9yx9Cu+R8jvPWIZ3vWD59M=*"),		_T("IM+"),								_T("ClientIcons_multiproto"),	IDI_IMPLUS,				MULTI_CASE,		TRUE	},
	{"client_IMVU",				_T("*IMVU*"),								_T("IMVU"),								_T("ClientIcons_multiproto"),	IDI_IMVU,				MULTI_CASE,		TRUE	},
	{"client_IMadering",		_T("*IMadering*"),							_T("IMadering"),						_T("ClientIcons_multiproto"),	IDI_IMADERING,			MULTI_CASE,		TRUE	},
	{"client_ImoIm",			_T("*imo.im*"),								_T("Imo.im"),							_T("ClientIcons_multiproto"),	IDI_IMOIM,				MULTI_CASE,		TRUE	},
	{"client_Inlux",			_T("*Inlux*"),								_T("Inlux Messenger"),					_T("ClientIcons_multiproto"),	IDI_INLUX,				MULTI_CASE,		TRUE	},
	{"client_Instantbird",		_T("*Instantbird*"),						_T("Instantbird"),						_T("ClientIcons_multiproto"),	IDI_INSTANTBIRD,		MULTI_CASE,		TRUE	},
	{"client_iCall",			_T("*iCall*"),								_T("iCall"),							_T("ClientIcons_multiproto"),	IDI_ICALL,				MULTI_CASE,		TRUE	},
	{"client_iChat",			_T("|*iChat*|**imagent*|*iMessages*"),		_T("iChat"),							_T("ClientIcons_multiproto"),	IDI_ICHAT,				MULTI_CASE,		TRUE	},
	{"client_irssi",			_T("*irssi*"),								_T("irssi"),							_T("ClientIcons_multiproto"),	IDI_IRSSI,				MULTI_CASE,		TRUE	},
	{"client_JBother",			_T("*JBother*"),							_T("JBother"),							_T("ClientIcons_multiproto"),	IDI_JBOTHER,			MULTI_CASE,		TRUE	},
	{"client_JBuddy",			_T("*JBuddy*"),								_T("JBuddy Messenger"),					_T("ClientIcons_multiproto"),	IDI_JBUDDY,				MULTI_CASE,		TRUE	},
	{"client_Jabbear",			_T("*Jabbear*"),							_T("Jabbear"),							_T("ClientIcons_multiproto"),	IDI_JABBEAR,			MULTI_CASE,		TRUE	},
	{"client_Jabbin",			_T("*Jabbin*"),								_T("Jabbin"),							_T("ClientIcons_multiproto"),	IDI_JABBIN,				MULTI_CASE,		TRUE	},
	{"client_Jasmine_IM",		_T("|Jasmine*IM*|*jasmineicq.ru/caps*"),	_T("Jasmine IM"),						_T("ClientIcons_multiproto"),	IDI_JASMINEIM,			MULTI_CASE,		TRUE	},

	{"client_Jimm_Aspro",		_T("*Jimm*Aspro*"),							_T("Jimm Aspro"),						_T("ClientIcons_multiproto"),	IDI_JIMM_ASPRO,			MULTI_CASE,		TRUE	},
	{"client_Jimmy",			_T("*Jimmy*"),								_T("JimmyIM"),							_T("ClientIcons_multiproto"),	IDI_JIMMY,				MULTI_CASE,		TRUE	},

	{"client_KMess",			_T("*KMess*"),								_T("KMess"),							_T("ClientIcons_multiproto"),	IDI_KMESS,				MULTI_CASE,		TRUE	},
	{"client_KoolIM",			_T("*Kool*"),								_T("KoolIM"),							_T("ClientIcons_multiproto"),	IDI_KOOLIM,				MULTI_CASE,		TRUE	},
	{"client_Kopete",			_T("*Kopete*"),								_T("Kopete"),							_T("ClientIcons_multiproto"),	IDI_KOPETE,				MULTI_CASE,		TRUE	},
	{"client_LeechCraft",		_T("|*LeechCraft*")
								_T("|*aNjQWbtza2QtXemMfBS2bwNOtcQ=*"),		_T("LeechCraft"),						_T("ClientIcons_multiproto"),	IDI_LEECHCRAFT,			MULTI_CASE,		TRUE	},
	{"client_MDC",				_T("*MDC*"),								_T("MDC"),								_T("ClientIcons_multiproto"),	IDI_MDC,				MULTI_CASE,		TRUE	},
	{"client_Meebo",			_T("Meebo*"),								_T("Meebo"),							_T("ClientIcons_multiproto"),	IDI_MEEBO,				MULTI_CASE,		TRUE	},
	{"client_Meetro",			_T("Meetro*"),								_T("Meetro"),							_T("ClientIcons_multiproto"),	IDI_MEETRO,				MULTI_CASE,		TRUE	},
	{"client_mChat",			_T("|mChat*|gsICQ*|*mchat.mgslab.com*"),	_T("mChat"),							_T("ClientIcons_multiproto"),	IDI_MCHAT,				MULTI_CASE,		TRUE	},
	{"client_Nimbuzz",			_T("*Nimbuzz*"),							_T("Nimbuzz"),							_T("ClientIcons_multiproto"),	IDI_NIMBUZZ,			MULTI_CASE,		TRUE	},
	{"client_Palringo",			_T("*Palringo*"),							_T("Palringo"),							_T("ClientIcons_multiproto"),	IDI_PALRINGO,			MULTI_CASE,		TRUE	},

	{"client_Pigeon",			_T("*PIGEON*"),								_T("PIGEON!"),							_T("ClientIcons_multiproto"),	IDI_PIGEON,				MULTI_CASE,		TRUE	},
	{"client_PlayXpert",		_T("*PlayXpert*"),							_T("PlayXpert"),						_T("ClientIcons_multiproto"),	IDI_PLAYXPERT,			MULTI_CASE,		TRUE	},
	{"client_Prelude",			_T("*Prelude*"),							_T("Prelude"),							_T("ClientIcons_multiproto"),	IDI_PRELUDE,			MULTI_CASE,		TRUE	},
	{"client_Proteus",			_T("*Proteus*"),							_T("Proteus"),							_T("ClientIcons_multiproto"),	IDI_PROTEUS,			MULTI_CASE,		TRUE	},

	{"client_QIP_Android",		_T("QIP *Android*"),						_T("QIP Mobile Android"),				_T("ClientIcons_multiproto"),	IDI_QIP_ANDROID,		MULTI_CASE,		TRUE	},
	{"client_QIP_iOS",			_T("|QIP*iOS*|QIP*iphone*|QIP*apple*"),		_T("QIP Mobile iOS"),					_T("ClientIcons_multiproto"),	IDI_QIP_IOS,			MULTI_CASE,		TRUE	},
	{"client_QIP_Symbian",		_T("*QIP*Symbian*"),						_T("QIP Mobile Symbian"),				_T("ClientIcons_multiproto"),	IDI_QIP_SYMBIAN,		MULTI_CASE,		TRUE	},
	{"client_QIP_Java",			_T("*QIP*Java*"),							_T("QIP Mobile Java"),					_T("ClientIcons_multiproto"),	IDI_QIP_JAVA,			MULTI_CASE,		TRUE	},
	{"client_QIP_PDA",			_T("|QIP *PDA*|*pda.qip.ru*|*QIP Mobile*"),	_T("QIP Mobile"),						_T("ClientIcons_multiproto"),	IDI_QIP_PDA,			MULTI_CASE,		TRUE	},
	{"client_QIP_2012",			_T("|QIP 2012*|http://qip.ru/caps*"),		_T("QIP 2012"),							_T("ClientIcons_multiproto"),	IDI_QIP_2012,			MULTI_CASE,		TRUE	},
	{"client_QIP_2010",			_T("|QIP 2010*|http://2010.qip.ru*"),		_T("QIP 2010"),							_T("ClientIcons_multiproto"),	IDI_QIP_2010,			MULTI_CASE,		TRUE	},
	{"client_QIP_Infium",		_T("|QIP Infium*|http://*qip*"),			_T("QIP Infium"),						_T("ClientIcons_multiproto"),	IDI_QIP_INFIUM,			MULTI_CASE,		TRUE	},

	{"client_qutIM",			_T("*qutIM*"),								_T("qutIM"),							_T("ClientIcons_multiproto"),	IDI_QUTIM,				MULTI_CASE	},
	{"client_mqutIM",			_T("*mqutIM*"),								_T("mqutIM"),							_T("ClientIcons_multiproto"),	IDI_MQUTIM,				MULTI_CASE	},

	{"client_Qnext",			_T("Qnext*"),								_T("Qnext"),							_T("ClientIcons_multiproto"),	IDI_QNEXT,				MULTI_CASE,		TRUE	},

	{"client_SAPO",				_T("*SAPO*"),								_T("SAPO Messenger"),					_T("ClientIcons_multiproto"),	IDI_SAPO,				MULTI_CASE,		TRUE	},
	{"client_SIM",				_T("|^*Simp*|*SIM*"),						_T("SIM"),								_T("ClientIcons_multiproto"),	IDI_SIM,				MULTI_CASE,		TRUE	},
	{"client_Salut_a_Toi",		_T("*Salut*Toi*"),							_T("Salut a Toi"),						_T("ClientIcons_multiproto"),	IDI_SALUT_A_TOI,		MULTI_CASE,		TRUE	},
	{"client_Shaim",			_T("*Shaim*"),								_T("Shaim"),							_T("ClientIcons_multiproto"),	IDI_SHAIM,				MULTI_CASE,		TRUE	},
	{"client_SieJC",			_T("|SieJC*|Nat*ICQ*|Siemens*Client*"),		_T("Siemens ICQ / Jabber client"),		_T("ClientIcons_multiproto"),	IDI_SIEJC,				MULTI_CASE,		TRUE	},
	{"client_Slick",			_T("Slick*"),								_T("Slick"),							_T("ClientIcons_multiproto"),	IDI_SLICK,				MULTI_CASE,		TRUE	},
	{"client_SrevIM",			_T("*Srev*IM*"),							_T("SrevIM"),							_T("ClientIcons_multiproto"),	IDI_SREVIM,				MULTI_CASE,		TRUE	},

	{"client_Tril_Android",		_T("*Trillian*Android*"),					_T("Trillian Android"),					_T("ClientIcons_multiproto"),	IDI_TRILLIAN_ANDROID,	MULTI_CASE,		TRUE	},
	{"client_Tril_Astra",		_T("Trillian*Astra*"),						_T("Trillian Astra"),					_T("ClientIcons_multiproto"),	IDI_TRILLIAN_ASTRA,		MULTI_CASE,		TRUE	},
	{"client_Trillian_Pro",		_T("Trillian*Pro*"),						_T("Trillian Pro"),						_T("ClientIcons_multiproto"),	IDI_TRILLIAN_PRO,		MULTI_CASE,		TRUE	},
	{"client_Trillian",			_T("*Trillian**"),							_T("Trillian"),							_T("ClientIcons_multiproto"),	IDI_TRILLIAN,			MULTI_CASE,		TRUE	},

	{"client_Tuukle_Chat",		_T("*Tuukle*Chat*|*IM*Gate*"),				_T("Tuukle Chat"),						_T("ClientIcons_multiproto"),	IDI_TUUKLE_CHAT,		MULTI_CASE,		TRUE	},
	{"client_vBuzzer",			_T("*vBuzzer*"),							_T("vBuzzer"),							_T("ClientIcons_multiproto"),	IDI_VBUZZER,			MULTI_CASE,		TRUE	},
	{"client_Virtus",			_T("*Virtus*"),								_T("Virtus"),							_T("ClientIcons_multiproto"),	IDI_VIRTUS,				MULTI_CASE,		TRUE	},
	{"client_uIM",				_T("*uIM*"),								_T("uIM"),								_T("ClientIcons_multiproto"),	IDI_UIM,				MULTI_CASE,		TRUE	},
	{"client_uTalk",			_T("*uTalk*"),								_T("uTalk"),							_T("ClientIcons_multiproto"),	IDI_UTALK,				MULTI_CASE,		TRUE	},
	{"client_WeeChat",			_T("*WeeChat*"),							_T("WeeChat"),							_T("ClientIcons_multiproto"),	IDI_WEECHAT,			MULTI_CASE,		TRUE	},
	{"client_Wippien",			_T("*Wippien*"),							_T("Wippien"),							_T("ClientIcons_multiproto"),	IDI_WIPPIEN,			MULTI_CASE,		TRUE	},

	{"client_YamiGo",			_T("YamiGo*"),								_T("YamiGo"),							_T("ClientIcons_multiproto"),	IDI_YAMIGO,				MULTI_CASE,		TRUE	},
	{"client_Yeigo",			_T("*Yeigo*"),								_T("Yeigo"),							_T("ClientIcons_multiproto"),	IDI_YEIGO,				MULTI_CASE,		TRUE	},
	{"client_Yoono",			_T("*Yoono*"),								_T("Yoono"),							_T("ClientIcons_multiproto"),	IDI_YOONO,				MULTI_CASE,		TRUE	},

//########################################################################################################################################################################################################
//#################################		ICQ		##########################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_D[i]Chat",			_T("D[i]Chat*"),							_T("D[i]Chat"),							_T("ClientIcons_ICQ"),			IDI_DICHAT,				ICQ_CASE,		TRUE	},
	{"client_GnomeICU",			_T("GnomeICU*"),							_T("GnomeICU"),							_T("ClientIcons_ICQ"),			IDI_GNOMEICU,			ICQ_CASE,		TRUE	},
	{"client_Icy_Juice",		_T("*Icy*Juice*"),							_T("Icy Juice"),						_T("ClientIcons_ICQ"),			IDI_ICY_JUICE,			ICQ_CASE,		TRUE	},
	{"client_Jimm",				_T("|*Jimm*|mobicq*"),						_T("Jimm"),								_T("ClientIcons_ICQ"),			IDI_JIMM,				ICQ_CASE,		TRUE	},
	{"client_Licq",				_T("Licq*"),								_T("Licq"),								_T("ClientIcons_ICQ"),			IDI_LICQ,				ICQ_CASE,		TRUE	},
	{"client_LocID",			_T("LocID*"),								_T("LocID"),							_T("ClientIcons_ICQ"),			IDI_LOCID,				ICQ_CASE,		TRUE	},
	{"client_QIP_2005",			_T("QIP*2005*"),							_T("QIP 2005"),							_T("ClientIcons_ICQ"),			IDI_QIP_2005,			ICQ_CASE,		TRUE	},
	{"client_R&Q",				_T("|R&Q*|&RQ*"),							_T("R&Q"),								_T("ClientIcons_ICQ"),			IDI_RANDQ,				ICQ_CASE,		TRUE	},
	{"client_SmartICQ",			_T("Smart*ICQ*"),							_T("SmartICQ"),							_T("ClientIcons_ICQ"),			IDI_SMARTICQ,			ICQ_CASE,		TRUE	},
	{"client_StrICQ",			_T("StrICQ*"),								_T("StrICQ"),							_T("ClientIcons_ICQ"),			IDI_STRICQ,				ICQ_CASE,		TRUE	},

	{"client_spam",				_T("*spam*"),								_T("spam client"),						_T("ClientIcons_ICQ"),			IDI_SPAM,				ICQ_CASE,		TRUE	},
	{"client_libicq2k",			_T("libicq*"),								_T("LibICQ/JIT"),						_T("ClientIcons_ICQ"),			IDI_LIBICQ2000,			ICQ_CASE,		TRUE	},
	{"client_TICQClient",		_T("*TICQ*Client*"),						_T("TICQClient (Delphi lib)"),			_T("ClientIcons_ICQ"),			IDI_TICQ,				ICQ_CASE,		TRUE	},

	{"client_ICQ_all",			_T("|ICQ?|ICQ?.?|ICQ *|ICQ2*|ICQ"),			_T("ICQ client"),						_T("ClientIcons_ICQ"),			IDI_ICQ,				ICQ_CASE	},

//########################################################################################################################################################################################################
//#################################		JABBER		######################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_Akeni",			_T("*Akeni*"),								_T("Akeni"),							_T("ClientIcons_Jabber"),		IDI_AKENI,				JABBER_CASE,	TRUE	},
	{"client_Ambrosia",			_T("*Ambrosia*"),							_T("Ambrosia XMPP Server"),				_T("ClientIcons_Jabber"),		IDI_AMBROSIA,			JABBER_CASE,	TRUE	},
	{"client_AnothRSSBot",		_T("|*Anothrbot*|*Anothr Rss Bot*"),		_T("Anothr Rss Bot"),					_T("ClientIcons_Jabber"),		IDI_ANOTHRSSBOT,		JABBER_CASE,	TRUE	},
	{"client_Aqq",				_T("|*aqq.eu*|aqq*"),						_T("Aqq"),								_T("ClientIcons_Jabber"),		IDI_AQQ,				JABBER_CASE,	TRUE	},
	{"client_BarnOwl",			_T("*Barn*Owl*"),							_T("BarnOwl"),							_T("ClientIcons_Jabber"),		IDI_BARNOWL,			JABBER_CASE,	TRUE	},
	{"client_Beem",				_T("*Beem*"),								_T("Beem"),								_T("ClientIcons_Jabber"),		IDI_BEEM,				JABBER_CASE,	TRUE	},
	{"client_BellSouth",		_T("*BellSouth*"),							_T("BellSouth"),						_T("ClientIcons_Jabber"),		IDI_BELLSOUTH,			JABBER_CASE,	TRUE	},
	{"client_BitWise",			_T("*BitWise*"),							_T("BitWise"),							_T("ClientIcons_Jabber"),		IDI_BITWISE,			JABBER_CASE,	TRUE	},
	{"client_Bombus",			_T("*Bombus*"),								_T("Bombus"),							_T("ClientIcons_Jabber"),		IDI_BOMBUS,				JABBER_CASE,	TRUE	},
	{"client_BombusMod",		_T("|*Bombus*mod*|*bombusmod*"),			_T("BombusMod"),						_T("ClientIcons_Jabber"),		IDI_BOMBUS_MOD,			JABBER_CASE,	TRUE	},
	{"client_BombusNG",			_T("*Bombus*NG*"),							_T("Bombus NG"),						_T("ClientIcons_Jabber"),		IDI_BOMBUS_NG,			JABBER_CASE,	TRUE	},
	{"client_BombusQD",			_T("|*Bombusmod-qd*|*bombus*qd*"),			_T("Bombus QD"),						_T("ClientIcons_Jabber"),		IDI_BOMBUS_QD,			JABBER_CASE,	TRUE	},
	{"client_Bowline",			_T("*Bow*line*"),							_T("Bowline"),							_T("ClientIcons_Jabber"),		IDI_BOWLINE,			JABBER_CASE,	TRUE	},
	{"client_BuddySpace",		_T("Buddy*Space*"),							_T("BuddySpace"),						_T("ClientIcons_Jabber"),		IDI_BUDDYSPACE,			JABBER_CASE,	TRUE	},
	{"client_CJC",				_T("*CJC*"),								_T("CJC"),								_T("ClientIcons_Jabber"),		IDI_CJC,				JABBER_CASE,	TRUE	},
	{"client_CRoom",			_T("*CRoom*"),								_T("CRoom"),							_T("ClientIcons_Jabber"),		IDI_CROOM,				JABBER_CASE,	TRUE	},
	{"client_Candy",			_T("*Candy*"),								_T("Candy"),							_T("ClientIcons_Jabber"),		IDI_CANDY,				JABBER_CASE,	TRUE	},
	{"client_Chatopus",			_T("*Chatopus*"),							_T("Chatopus"),							_T("ClientIcons_Jabber"),		IDI_CHATOPUS,			JABBER_CASE,	TRUE	},
	{"client_Chikka",			_T("*Chikka*"),								_T("Chikka"),							_T("ClientIcons_Jabber"),		IDI_CHIKKA,				JABBER_CASE,	TRUE	},
	{"client_ChitChat",			_T("*Chit*Chat*"),							_T("ChitChat"),							_T("ClientIcons_Jabber"),		IDI_CHITCHAT,			JABBER_CASE,	TRUE	},
	{"client_Claros_Chat",		_T("*Claros*"),								_T("Claros Chat"),						_T("ClientIcons_Jabber"),		IDI_CLAROS_CHAT,		JABBER_CASE,	TRUE	},
	{"client_Coccinella",		_T("*Coccinella*"),							_T("Coccinella"),						_T("ClientIcons_Jabber"),		IDI_COCCINELLA,			JABBER_CASE,	TRUE	},
	{"client_Colibry",			_T("Colibry*"),								_T("Colibry"),							_T("ClientIcons_Jabber"),		IDI_COLIBRY,			JABBER_CASE,	TRUE	},
	{"client_Colloquy",			_T("Colloquy*"),							_T("Colloquy"),							_T("ClientIcons_Jabber"),		IDI_COLLOQUY,			JABBER_CASE,	TRUE	},
	{"client_CommuniGate",		_T("*CommuniGate*"),						_T("CommuniGate Pro"),					_T("ClientIcons_Jabber"),		IDI_COMMUNIGATE,		JABBER_CASE,	TRUE	},
	{"client_Conference",		_T("Conference*"),							_T("Conference Bot (GMail)"),			_T("ClientIcons_Jabber"),		IDI_CONFERENCE,			JABBER_CASE,	TRUE	},
	{"client_Crosstalk",		_T("*Cross*talk*"),							_T("Crosstalk"),						_T("ClientIcons_Jabber"),		IDI_CROSSTALK,			JABBER_CASE,	TRUE	},
	{"client_Cudumar",			_T("*Cudumar*"),							_T("Cudumar"),							_T("ClientIcons_Jabber"),		IDI_CUDUMAR,			JABBER_CASE,	TRUE	},
	{"client_CyclopsChat",		_T("*Cyclops*"),							_T("Cyclops Chat"),						_T("ClientIcons_Jabber"),		IDI_CYCLOPS_CHAT,		JABBER_CASE,	TRUE	},
	{"client_Desyr",			_T("*Desyr*"),								_T("Desyr Messenger"),					_T("ClientIcons_Jabber"),		IDI_DESYR,				JABBER_CASE,	TRUE	},
	{"client_EMess",			_T("*EMess*"),								_T("EMess"),							_T("ClientIcons_Jabber"),		IDI_EMESS,				JABBER_CASE,	TRUE	},
	{"client_Elmer_Bot",		_T("*Elmer*"),								_T("Elmer Bot"),						_T("ClientIcons_Jabber"),		IDI_ELMER,				JABBER_CASE,	TRUE	},
	{"client_Emacs",			_T("|*Jabber.el*|*Emacs*"),					_T("Emacs (Jabber.el)"),				_T("ClientIcons_Jabber"),		IDI_EMACS,				JABBER_CASE,	TRUE	},
	{"client_Exodus",			_T("*Exodus*"),								_T("Exodus"),							_T("ClientIcons_Jabber"),		IDI_EXODUS,				JABBER_CASE,	TRUE	},

	{"client_GCN",				_T("*GCN*"),								_T("GCN"),								_T("ClientIcons_Jabber"),		IDI_GCN,				JABBER_CASE,	TRUE	},
	{"client_GMail",			_T("|*gmail.*|GMail*"),						_T("GMail"),							_T("ClientIcons_Jabber"),		IDI_GMAIL,				JABBER_CASE,	TRUE	},
	{"client_GOIM",				_T("*GOIM*"),								_T("GOIM"),								_T("ClientIcons_Jabber"),		IDI_GOIM,				JABBER_CASE,	TRUE	},
	{"client_GTalk",			_T("|*Talk.v*|*Google*Talk*")
								_T("|*Gtalk*|*-iPhon*"),					_T("GoogleTalk aka GTalk"),				_T("ClientIcons_Jabber"),		IDI_GTALK,				JABBER_CASE,	TRUE	},
	{"client_GTalk_Gadget",		_T("*Talk*Gadget*"),						_T("GTalk Gadget"),						_T("ClientIcons_Jabber"),		IDI_GTALK_GADGET,		JABBER_CASE,	TRUE	},
	{"client_Gabber",			_T("*Gabber*"),								_T("Gabber"),							_T("ClientIcons_Jabber"),		IDI_GABBER,				JABBER_CASE,	TRUE	},
	{"client_Gajim",			_T("*Gajim*"),								_T("Gajim"),							_T("ClientIcons_Jabber"),		IDI_GAJIM,				JABBER_CASE,	TRUE	},
	{"client_Gibberbot",		_T("*Gibber*"),								_T("Gibberbot"),						_T("ClientIcons_Jabber"),		IDI_GIBBERBOT,			JABBER_CASE,	TRUE	},
	{"client_Glu",				_T("|glu*|*glu.net*"),						_T("Glu"),								_T("ClientIcons_Jabber"),		IDI_GLU,				JABBER_CASE,	TRUE	},
	{"client_Gnome",			_T("*Gnome*"),								_T("Gnome"),							_T("ClientIcons_Jabber"),		IDI_GNOME,				JABBER_CASE,	TRUE	},
	{"client_GoTalkMobile",		_T("*Go*Talk*Mobile*"),						_T("GoTalkMobile"),						_T("ClientIcons_Jabber"),		IDI_GOTALKMOBILE,		JABBER_CASE,	TRUE	},
	{"client_Gossip",			_T("*Gossip*"),								_T("Gossip"),							_T("ClientIcons_Jabber"),		IDI_GOSSIP,				JABBER_CASE,	TRUE	},
	{"client_GreenThumb",		_T("gReeNtHumB*"),							_T("GreenThumb"),						_T("ClientIcons_Jabber"),		IDI_GREENTHUMB,			JABBER_CASE,	TRUE	},
	{"client_Gush",				_T("*Gush*"),								_T("Gush"),								_T("ClientIcons_Jabber"),		IDI_GUSH,				JABBER_CASE,	TRUE	},
	{"client_IMCom",			_T("*IMCom*"),								_T("IMCom"),							_T("ClientIcons_Jabber"),		IDI_IMCOM,				JABBER_CASE,	TRUE	},
	{"client_IM_Friendly",		_T("*IM*Friendly*"),						_T("IM Friendly!"),						_T("ClientIcons_Jabber"),		IDI_IM_FRIENDLY,		JABBER_CASE,	TRUE	},
	{"client_Imified",			_T("*someresource*"),						_T("Imified"),							_T("ClientIcons_Jabber"),		IDI_IMIFIED,			JABBER_CASE,	TRUE	},
	{"client_Importal",			_T("*Importal*"),							_T("Importal"),							_T("ClientIcons_Jabber"),		IDI_IMPORTAL,			JABBER_CASE,	TRUE	},
	{"client_InstanT",			_T("*Instan-t*"),							_T("Instan-t"),							_T("ClientIcons_Jabber"),		IDI_INSTANT,			JABBER_CASE,	TRUE	},
	{"client_Interaction",		_T("*Interaction*"),						_T("Interaction"),						_T("ClientIcons_Jabber"),		IDI_INTERACTION,		JABBER_CASE,	TRUE	},
	{"client_iruka",			_T("*Iruka*"),								_T("Iruka"),							_T("ClientIcons_Jabber"),		IDI_IRUKA,				JABBER_CASE,	TRUE	},
	{"client_J2J_Transport",	_T("*J2J*Transport*"),						_T("J2J Transport"),					_T("ClientIcons_Jabber"),		IDI_J2J_TRANSPORT,		JABBER_CASE,	TRUE	},
	{"client_Jamm",				_T("*Jamm*"),								_T("Jamm"),								_T("ClientIcons_Jabber"),		IDI_JAMM,				JABBER_CASE,	TRUE	},
	{"client_JClaim",			_T("*JClaim*"),								_T("JClaim"),							_T("ClientIcons_Jabber"),		IDI_JCLAIM,				JABBER_CASE,	TRUE	},
	{"client_JMC",				_T("JMC*"),									_T("JMC (Jabber Mix Client)"),			_T("ClientIcons_Jabber"),		IDI_JMC,				JABBER_CASE,	TRUE	},
	{"client_JWChat",			_T("*JWChat*"),								_T("JWChat"),							_T("ClientIcons_Jabber"),		IDI_JWCHAT,				JABBER_CASE,	TRUE	},
	{"client_JWGC",				_T("|*JWGC*|Jabber *Gram*"),				_T("JWGC (Jabber WindowGram Client)"),	_T("ClientIcons_Jabber"),		IDI_JWGC,				JABBER_CASE,	TRUE	},
	{"client_Jabba",			_T("*Jabba*"),								_T("Jabba"),							_T("ClientIcons_Jabber"),		IDI_JABBA,				JABBER_CASE,	TRUE	},
	{"client_Jabber",			_T("*Jabber*client*"),						_T("Jabber client"),					_T("ClientIcons_Jabber"),		IDI_JABBER,				JABBER_CASE,	TRUE	},
	{"client_JabberApplet",		_T("Jabber*Applet*"),						_T("JabberApplet"),						_T("ClientIcons_Jabber"),		IDI_JABBER_APPLET,		JABBER_CASE,	TRUE	},
	{"client_JabberBeOS",		_T("Jabber*BeOS*"),							_T("Jabber (BeOS)"),					_T("ClientIcons_Jabber"),		IDI_JABBER_BEOS,		JABBER_CASE,	TRUE	},
	{"client_JabberFoX",		_T("*fox*"),								_T("JabberFoX"),						_T("ClientIcons_Jabber"),		IDI_JABBERFOX,			JABBER_CASE,	TRUE	},
	{"client_JabberMSNGR",		_T("Jabber Messenger*"),					_T("Jabber Messenger"),					_T("ClientIcons_Jabber"),		IDI_JABBER_MESSENGER,	JABBER_CASE,	TRUE	},
	{"client_JabberNaut",		_T("*Jabber*Naut*"),						_T("JabberNaut"),						_T("ClientIcons_Jabber"),		IDI_JABBERNAUT,			JABBER_CASE,	TRUE	},
	{"client_JabberZilla",		_T("*Zilla*"),								_T("JabberZilla"),						_T("ClientIcons_Jabber"),		IDI_JABBERZILLA,		JABBER_CASE,	TRUE	},
	{"client_Jabber_Net",		_T("|*Jabber*Net*|*cursive.net*|*csharp*"),	_T("Jabber-Net"),						_T("ClientIcons_Jabber"),		IDI_JABBER_NET,			JABBER_CASE,	TRUE	},
	{"client_Jabberwocky",		_T("Jabberwocky*"),							_T("Jabberwocky (Amiga)"),				_T("ClientIcons_Jabber"),		IDI_JABBERWOCKY,		JABBER_CASE,	TRUE	},
	{"client_Jabbroid",			_T("*Jabbroid*"),							_T("Jabbroid"),							_T("ClientIcons_Jabber"),		IDI_JABBROID,			JABBER_CASE,	TRUE	},
	{"client_Jajc",				_T("|*Jajc*|Just Another Jabber Client"),	_T("JAJC"),								_T("ClientIcons_Jabber"),		IDI_JAJC,				JABBER_CASE,	TRUE	},
	{"client_Jeti",				_T("*Jeti*"),								_T("Jeti"),								_T("ClientIcons_Jabber"),		IDI_JETI,				JABBER_CASE,	TRUE	},
	{"client_Jitsi",			_T("*Jitsi*"),								_T("Jitsi"),							_T("ClientIcons_Jabber"),		IDI_JITSI,				JABBER_CASE,	TRUE	},
	{"client_Joost",			_T("*Joost*"),								_T("Joost"),							_T("ClientIcons_Jabber"),		IDI_JOOST,				JABBER_CASE,	TRUE	},
	{"client_Kadu",				_T("*Kadu*"),								_T("Kadu"),								_T("ClientIcons_Jabber"),		IDI_KADU,				JABBER_CASE,	TRUE	},
	{"client_Konnekt",			_T("Konnekt*"),								_T("Konnekt"),							_T("ClientIcons_Jabber"),		IDI_KONNEKT,			JABBER_CASE,	TRUE	},
	{"client_LLuna",			_T("LLuna*"),								_T("LLuna"),							_T("ClientIcons_Jabber"),		IDI_LLUNA,				JABBER_CASE,	TRUE	},
	{"client_Lamp",				_T("*Lamp*IM*"),							_T("Lamp IM"),							_T("ClientIcons_Jabber"),		IDI_LAMP_IM,			JABBER_CASE,	TRUE	},
	{"client_Lampiro",			_T("*Lampiro*"),							_T("Lampiro"),							_T("ClientIcons_Jabber"),		IDI_LAMPIRO,			JABBER_CASE,	TRUE	},
	{"client_Landell",			_T("*Landell*"),							_T("Landell"),							_T("ClientIcons_Jabber"),		IDI_LANDELL,			JABBER_CASE,	TRUE	},
	{"client_Leaf",				_T("*Leaf*"),								_T("Leaf Messenger"),					_T("ClientIcons_Jabber"),		IDI_LEAF,				JABBER_CASE,	TRUE	},
	{"client_LinQ",				_T("*LinQ*"),								_T("LinQ"),								_T("ClientIcons_Jabber"),		IDI_LINQ,				JABBER_CASE,	TRUE	},
	{"client_M8Jabber",			_T("*M8Jabber*"),							_T("M8Jabber"),							_T("ClientIcons_Jabber"),		IDI_M8JABBER,			JABBER_CASE,	TRUE	},
	{"client_MCabber",			_T("*mcabber*"),							_T("MCabber"),							_T("ClientIcons_Jabber"),		IDI_MCABBER,			JABBER_CASE,	TRUE	},
	{"client_MGTalk",			_T("|*MGTalk*|*Mobile?\?\?\?\?\?\?\?"),		_T("MGTalk"),							_T("ClientIcons_Jabber"),		IDI_MGTALK,				JABBER_CASE,	TRUE	},
	{"client_MUCkl",			_T("*MUCkl*"),								_T("MUCkl"),							_T("ClientIcons_Jabber"),		IDI_MUCKL,				JABBER_CASE,	TRUE	},
	{"client_Mango",			_T("*Mango*"),								_T("Mango"),							_T("ClientIcons_Jabber"),		IDI_MANGO,				JABBER_CASE,	TRUE	},
	{"client_Mercury",			_T("*Mercury*"),							_T("Mercury Messenger"),				_T("ClientIcons_Jabber"),		IDI_MERCURY_MESSENGER,	JABBER_CASE,	TRUE	},
	{"client_Monal",			_T("*Monal*"),								_T("Monal"),							_T("ClientIcons_Jabber"),		IDI_MONAL,				JABBER_CASE,	TRUE	},
	{"client_MozillaChat",		_T("*Mozilla*Chat*"),						_T("MozillaChat"),						_T("ClientIcons_Jabber"),		IDI_MOZILLACHAT,		JABBER_CASE,	TRUE	},
	{"client_Neos",				_T("Neos*"),								_T("Neos"),								_T("ClientIcons_Jabber"),		IDI_NEOS,				JABBER_CASE,	TRUE	},
	{"client_Nitro",			_T("Nitro*"),								_T("Nitro"),							_T("ClientIcons_Jabber"),		IDI_NITRO,				JABBER_CASE,	TRUE	},
	{"client_Nostromo",			_T("*USCSS*Nostromo*"),						_T("USCSS Nostromo"),					_T("ClientIcons_Jabber"),		IDI_NOSTROMO,			JABBER_CASE,	TRUE	},
	{"client_OM",				_T("OM*"),									_T("OM aka Online Messenger"),			_T("ClientIcons_Jabber"),		IDI_OM,					JABBER_CASE,	TRUE	},
	{"client_OctroTalk",		_T("*Octro*"),								_T("OctroTalk"),						_T("ClientIcons_Jabber"),		IDI_OCTROTALK,			JABBER_CASE,	TRUE	},
	{"client_OneTeam",			_T("*OneTeam*"),							_T("OneTeam"),							_T("ClientIcons_Jabber"),		IDI_ONETEAM,			JABBER_CASE,	TRUE	},
	{"client_Openfire",			_T("*Openfire*"),							_T("Openfire"),							_T("ClientIcons_Jabber"),		IDI_OPENFIRE,			JABBER_CASE,	TRUE	},
	{"client_Fire",				_T("Fire*"),								_T("Fire"),								_T("ClientIcons_Jabber"),		IDI_FIRE,				JABBER_CASE,	TRUE	},
	{"client_Paltalk",			_T("*Paltalk*"),							_T("Paltalk"),							_T("ClientIcons_Jabber"),		IDI_PALTALK,			JABBER_CASE,	TRUE	},
	{"client_Pandion",			_T("|*Pandion*|*Пандион*"),					_T("Pandion"),							_T("ClientIcons_Jabber"),		IDI_PANDION,			JABBER_CASE,	TRUE	},
	{"client_Papla",			_T("*Papla*"),								_T("Papla"),							_T("ClientIcons_Jabber"),		IDI_PAPLA,				JABBER_CASE,	TRUE	},
	{"client_Poezio",			_T("*Poezio*"),								_T("Poezio"),							_T("ClientIcons_Jabber"),		IDI_POEZIO,				JABBER_CASE,	TRUE	},
	{"client_Prosody",			_T("*Prosody*"),							_T("Prosody"),							_T("ClientIcons_Jabber"),		IDI_PROSODY,			JABBER_CASE,	TRUE	},

	{"client_Psi_plus",			_T("|*PSI+*|*psi-dev.googlecode*"),			_T("PSI+"),								_T("ClientIcons_Jabber"),		IDI_PSIPLUS,			JABBER_CASE,	TRUE	},
	{"client_Psi",				_T("*Psi*"),								_T("PSI"),								_T("ClientIcons_Jabber"),		IDI_PSI,				JABBER_CASE,	TRUE	},

	{"client_Psto",				_T("*Psto*"),								_T("Psto.net"),							_T("ClientIcons_Jabber"),		IDI_PSTO,				JABBER_CASE,	TRUE	},
	{"client_Psyc",				_T("*Psyc*"),								_T("Psyc"),								_T("ClientIcons_Jabber"),		IDI_PSYC,				JABBER_CASE,	TRUE	},
	{"client_Pygeon",			_T("*Pygeon*"),								_T("Pygeon"),							_T("ClientIcons_Jabber"),		IDI_PYGEON,				JABBER_CASE,	TRUE	},

	{"client_QTJim",			_T("*QTJim*"),								_T("QTJim"),							_T("ClientIcons_Jabber"),		IDI_QTJIM,				JABBER_CASE,	TRUE	},
	{"client_QuteCom",			_T("*Qute*Com*"),							_T("QuteCom"),							_T("ClientIcons_Jabber"),		IDI_QUTECOM,			JABBER_CASE,	TRUE	},
	{"client_RenRen",			_T("|*WTalkProxy0_0*|*talk.xiaonei.com*"),	_T("RenRen"),							_T("ClientIcons_Jabber"),		IDI_RENREN,				JABBER_CASE,	TRUE	},
	{"client_SBot",				_T("*SBot*"),								_T("SBot"),								_T("ClientIcons_Jabber"),		IDI_SBOT,				JABBER_CASE,	TRUE	},
	{"client_SMTP_Transport",	_T("*smtp*transport*"),						_T("SMTP Transport"),					_T("ClientIcons_Jabber"),		IDI_SMTP_TRANSPORT,		JABBER_CASE,	TRUE	},
	{"client_SamePlace",		_T("*SamePlace*"),							_T("SamePlace"),						_T("ClientIcons_Jabber"),		IDI_SAMEPLACE,			JABBER_CASE,	TRUE	},
	{"client_Sky_Messager",		_T("Sky*Mess*"),							_T("Sky Messager"),						_T("ClientIcons_Jabber"),		IDI_SKYMESSAGER,		JABBER_CASE,	TRUE	},
	{"client_Sky_Messager",		_T("*Sky*Messager*"),						_T("Sky Messager"),						_T("ClientIcons_Jabber"),		IDI_SKYMESSAGER,		JABBER_CASE,	TRUE	},

	{"client_xabber",			_T("|*xabber*")
								_T("|*bWG06mEjKFM5ygtd84Ov95P8VH0=*"),		_T("xabber"),							_T("ClientIcons_Jabber"),		IDI_XABBER,				JABBER_CASE,	TRUE	},

	{"client_Gabble",			_T("*Gabble*"),								_T("Gabble"),							_T("ClientIcons_Jabber"),		IDI_GABBLE,				JABBER_CASE,	TRUE	},

	{"client_Smack",			_T("|*igniterealtime.*smack*|*smack*"),		_T("Smack"),							_T("ClientIcons_Jabber"),		IDI_SMACK,				JABBER_CASE,	TRUE	},

	{"client_SoapBox",			_T("*SoapBox*"),							_T("SoapBox"),							_T("ClientIcons_Jabber"),		IDI_SOAPBOX,			JABBER_CASE,	TRUE	},
	{"client_Spark",			_T("*Spark*"),								_T("Spark"),							_T("ClientIcons_Jabber"),		IDI_SPARK,				JABBER_CASE,	TRUE	},
	{"client_Speakall",			_T("*Speak*all*"),							_T("Speakall"),							_T("ClientIcons_Jabber"),		IDI_SPEAKALL,			JABBER_CASE,	TRUE	},
	{"client_Speeqe",			_T("*Speeqe*"),								_T("Speeqe"),							_T("ClientIcons_Jabber"),		IDI_SPEEQE,				JABBER_CASE,	TRUE	},
	{"client_Spik",				_T("*Spik*"),								_T("Spik"),								_T("ClientIcons_Jabber"),		IDI_SPIK,				JABBER_CASE,	TRUE	},
	{"client_Swift",			_T("*Swift*"),								_T("Swift"),							_T("ClientIcons_Jabber"),		IDI_SWIFT,				JABBER_CASE,	TRUE	},
	{"client_SworIM",			_T("*Swor*IM*"),							_T("SworIM"),							_T("ClientIcons_Jabber"),		IDI_SWORIM,				JABBER_CASE,	TRUE	},
	{"client_Synapse",			_T("*Synapse*"),							_T("Synapse"),							_T("ClientIcons_Jabber"),		IDI_SYNAPSE,			JABBER_CASE,	TRUE	},
	{"client_Talkdroid",		_T("*Talkdroid*"),							_T("Talkdroid"),						_T("ClientIcons_Jabber"),		IDI_TALKDROID,			JABBER_CASE,	TRUE	},
	{"client_Talkonaut",		_T("*Talkonaut*"),							_T("Talkonaut"),						_T("ClientIcons_Jabber"),		IDI_TALKONAUT,			JABBER_CASE,	TRUE	},
	{"client_Tapioca",			_T("*Tapioca*"),							_T("Tapioca"),							_T("ClientIcons_Jabber"),		IDI_TAPIOCA,			JABBER_CASE,	TRUE	},
	{"client_Teabot",			_T("|*teabot*|*teabot.org/bot*"),			_T("Teabot"),							_T("ClientIcons_Jabber"),		IDI_TEABOT,				JABBER_CASE,	TRUE	},
	{"client_Telepathy",		_T("*Telepathy*"),							_T("Telepathy"),						_T("ClientIcons_Jabber"),		IDI_TELEPATHY,			JABBER_CASE,	TRUE	},
	{"client_The_Bee",			_T("*The*Bee*"),							_T("The Bee"),							_T("ClientIcons_Jabber"),		IDI_THEBEE,				JABBER_CASE,	TRUE	},
	{"client_Thunderbird",		_T("*Thunderbi*"),							_T("Thunderbird"),						_T("ClientIcons_Jabber"),		IDI_THUNDERBIRD,		JABBER_CASE,	TRUE	},
	{"client_Tigase",			_T("*Tigase*"),								_T("Tigase"),							_T("ClientIcons_Jabber"),		IDI_TIGASE,				JABBER_CASE,	TRUE	},
	{"client_TipicIM",			_T("Tipic*"),								_T("TipicIM"),							_T("ClientIcons_Jabber"),		IDI_TIPICIM,			JABBER_CASE,	TRUE	},
	{"client_Tkabber",			_T("*Tkabber*"),							_T("Tkabber"),							_T("ClientIcons_Jabber"),		IDI_TKABBER,			JABBER_CASE,	TRUE	},
	{"client_TransactIM",		_T("*Transact*"),							_T("TransactIM"),						_T("ClientIcons_Jabber"),		IDI_TRANSACTIM,			JABBER_CASE,	TRUE	},
	{"client_Translate",		_T("*Translate*"),							_T("Translate component"),				_T("ClientIcons_Jabber"),		IDI_TRANSLATE,			JABBER_CASE,	TRUE	},
	{"client_Triple",			_T("Triple*"),								_T("TripleSoftwareIM (TSIM)"),			_T("ClientIcons_Jabber"),		IDI_TRIPLE_SOFTWARE,	JABBER_CASE,	TRUE	},

	{"client_VK",				_T("|*VKontakte*|*vk.com*"),				_T("VKontakte"),						_T("ClientIcons_Jabber"),		IDI_VK,					JABBER_CASE,	TRUE	},
	{"client_Vacuum",			_T("*Vacuum*"),								_T("Vacuum IM"),						_T("ClientIcons_Jabber"),		IDI_VACUUM,				JABBER_CASE,	TRUE	},
	{"client_V&V",				_T("*V&V*"),								_T("V&V Messenger"),					_T("ClientIcons_Jabber"),		IDI_VANDV,				JABBER_CASE,	TRUE	},
	{"client_Vayusphere",		_T("*Vayusphere*"),							_T("Vayusphere"),						_T("ClientIcons_Jabber"),		IDI_VAYUSPHERE,			JABBER_CASE,	TRUE	},
	{"client_Vysper",			_T("*Vysper*"),								_T("Vysper"),							_T("ClientIcons_Jabber"),		IDI_VYSPER,				JABBER_CASE,	TRUE	},
	{"client_WTW",				_T("**WTW**|*wtw.k2t.eu*"),					_T("WTW"),								_T("ClientIcons_Jabber"),		IDI_WTW,				JABBER_CASE,	TRUE	},
	{"client_WannaChat",		_T("Wanna*Chat*"),							_T("WannaChat"),						_T("ClientIcons_Jabber"),		IDI_WANNACHAT,			JABBER_CASE,	TRUE	},
	{"client_WebEx",			_T("*webex.com*"),							_T("Cisco WebEx Connect"),				_T("ClientIcons_Jabber"),		IDI_WEBEX,				JABBER_CASE,	TRUE	},
	{"client_WhisperIM",		_T("*Whisper*"),							_T("WhisperIM"),						_T("ClientIcons_Jabber"),		IDI_WHISPERIM,			JABBER_CASE,	TRUE	},
	{"client_Wija",				_T("*wija*"),								_T("Wija"),								_T("ClientIcons_Jabber"),		IDI_WIJA,				JABBER_CASE,	TRUE	},
	{"client_Wildfire",			_T("Wildfire*"),							_T("Wildfire"),							_T("ClientIcons_Jabber"),		IDI_WILDFIRE,			JABBER_CASE,	TRUE	},
	{"client_WinJab",			_T("*WinJab*"),								_T("WinJab"),							_T("ClientIcons_Jabber"),		IDI_WINJAB,				JABBER_CASE,	TRUE	},
	{"client_XMPP",				_T("*XMPP*"),								_T("XMPP"),								_T("ClientIcons_Jabber"),		IDI_XMPP,				JABBER_CASE,	TRUE	},
	{"client_Xiffian",			_T("*Xiffian*"),							_T("Xiffian"),							_T("ClientIcons_Jabber"),		IDI_XIFFIAN,			JABBER_CASE,	TRUE	},
	{"client_Yambi",			_T("*Yambi*"),								_T("Yambi"),							_T("ClientIcons_Jabber"),		IDI_YAMBI,				JABBER_CASE,	TRUE	},
	{"client_chat_bots",		_T("*chat*bot*"),							_T("chat bot"),							_T("ClientIcons_Jabber"),		IDI_CHAT_BOT,			JABBER_CASE,	TRUE	},
	{"client_dziObber",			_T("*dzi?bber*"),							_T("dziObber"),							_T("ClientIcons_Jabber"),		IDI_DZIOBBER,			JABBER_CASE,	TRUE	},
	{"client_ejabberd",			_T("*ejabberd*"),							_T("ejabberd"),							_T("ClientIcons_Jabber"),		IDI_EJABBERD,			JABBER_CASE,	TRUE	},
	{"client_emite",			_T("*emite*"),								_T("emite"),							_T("ClientIcons_Jabber"),		IDI_EMITE,				JABBER_CASE,	TRUE	},
	{"client_gYaber",			_T("gYaber*"),								_T("gYaber"),							_T("ClientIcons_Jabber"),		IDI_GYABER,				JABBER_CASE,	TRUE	},
	{"client_glu",				_T("*glu*"),								_T("glu"),								_T("ClientIcons_Jabber"),		IDI_GLU,				JABBER_CASE,	TRUE	},
	{"client_iGoogle",			_T("iGoogle*"),								_T("iGoogle"),							_T("ClientIcons_Jabber"),		IDI_IGOOGLE,			JABBER_CASE,	TRUE	},
	{"client_iJab",				_T("*iJab*"),								_T("iJab"),								_T("ClientIcons_Jabber"),		IDI_IJAB,				JABBER_CASE,	TRUE	},
	{"client_iMeem",			_T("iMeem*"),								_T("iMeem"),							_T("ClientIcons_Jabber"),		IDI_IMEEM,				JABBER_CASE,	TRUE	},
	{"client_iMov",				_T("*imov*"),								_T("iMov"),								_T("ClientIcons_Jabber"),		IDI_IMOV,				JABBER_CASE,	TRUE	},
	{"client_jTalk",			_T("*jTalk*"),								_T("jTalk"),							_T("ClientIcons_Jabber"),		IDI_JTALK,				JABBER_CASE,	TRUE	},
	{"client_jabberDisk",		_T("|*jdisk*|*jabber*Disk*"),				_T("jabberDisk"),						_T("ClientIcons_Jabber"),		IDI_JABBER_DISK,		JABBER_CASE,	TRUE	},
	{"client_jabbim",			_T("*jabbim*"),								_T("Jabbim"),							_T("ClientIcons_Jabber"),		IDI_JABBIM,				JABBER_CASE,	TRUE	},
	{"client_jabiru",			_T("*jabiru*"),								_T("Jabiru"),							_T("ClientIcons_Jabber"),		IDI_JABIRU,				JABBER_CASE,	TRUE	},
	{"client_jappix",			_T("*jappix*"),								_T("jappix"),							_T("ClientIcons_Jabber"),		IDI_JAPPIX,				JABBER_CASE,	TRUE	},
	{"client_jrudevels",		_T("*jrudevels*"),							_T("Jrudevels"),						_T("ClientIcons_Jabber"),		IDI_JRUDEVELS,			JABBER_CASE,	TRUE	},
	{"client_juick",			_T("*juick*"),								_T("Juick"),							_T("ClientIcons_Jabber"),		IDI_JUICK,				JABBER_CASE,	TRUE	},
	{"client_kf",				_T("|^*smack*|*kf*"),						_T("kf jabber"),						_T("ClientIcons_Jabber"),		IDI_KF,					JABBER_CASE,	TRUE	},
	{"client_laffer",			_T("*laffer*"),								_T("Laffer"),							_T("ClientIcons_Jabber"),		IDI_LAFFER,				JABBER_CASE,	TRUE	},
	{"client_mJabber",			_T("*mJabber*"),							_T("mJabber"),							_T("ClientIcons_Jabber"),		IDI_MJABBER,			JABBER_CASE,	TRUE	},
	{"client_meinvz",			_T("*meinvz*"),								_T("MeinVZ"),							_T("ClientIcons_Jabber"),		IDI_MEINVZ,				JABBER_CASE,	TRUE	},
	{"client_moJab",			_T("*moJab*"),								_T("moJab"),							_T("ClientIcons_Jabber"),		IDI_MOJAB,				JABBER_CASE,	TRUE	},
	{"client_mobber",			_T("*mobber*"),								_T("mobber"),							_T("ClientIcons_Jabber"),		IDI_MOBBER,				JABBER_CASE,	TRUE	},
	{"client_myJabber",			_T("*myJabber*"),							_T("myJabber"),							_T("ClientIcons_Jabber"),		IDI_MYJABBER,			JABBER_CASE,	TRUE	},
	{"client_orkut",			_T("*orkut*"),								_T("orkut"),							_T("ClientIcons_Jabber"),		IDI_ORKUT,				JABBER_CASE,	TRUE	},
	{"client_pjc",				_T("|*PJC*|*pjc.googlecode.com*"),			_T("PHP Jabber Client"),				_T("ClientIcons_Jabber"),		IDI_PJC,				JABBER_CASE,	TRUE	},
	{"client_saje",				_T("*saje*"),								_T("saje"),								_T("ClientIcons_Jabber"),		IDI_SAJE,				JABBER_CASE,	TRUE	},
	{"client_schuelervz",		_T("*schuelervz*"),							_T("SchulerVZ"),						_T("ClientIcons_Jabber"),		IDI_SCHULERVZ,			JABBER_CASE,	TRUE	},
	{"client_studivz",			_T("*studivz*"),							_T("StudiVZ"),							_T("ClientIcons_Jabber"),		IDI_STUDIVZ,			JABBER_CASE,	TRUE	},
	{"client_tkchat",			_T("*tkchat*"),								_T("tkchat"),							_T("ClientIcons_Jabber"),		IDI_TKCHAT,				JABBER_CASE,	TRUE	},
//	{"client_uJabber",			_T("*uJabber*"),							_T("uJabber"),							_T("ClientIcons_Jabber"),		IDI_UJABBER,			JABBER_CASE,	TRUE	},
	{"client_uKeeper",			_T("*uKeeper*"),							_T("uKeeper"),							_T("ClientIcons_Jabber"),		IDI_UKEEPER,			JABBER_CASE,	TRUE	},
	{"client_whoisbot",			_T("whoisbot"),								_T("Swissjabber Whois Bot"),			_T("ClientIcons_Jabber"),		IDI_WHOISBOT,			JABBER_CASE,	TRUE	},
	{"client_xeus2",			_T("*xeus 2*"),								_T("xeus 2"),							_T("ClientIcons_Jabber"),		IDI_XEUS2,				JABBER_CASE,	TRUE	},
	{"client_xeus",				_T("*xeus*"),								_T("xeus"),								_T("ClientIcons_Jabber"),		IDI_XEUS,				JABBER_CASE,	TRUE	},
	{"client_yaonline",			_T("|*yandex*|*yaonline*")
								_T("|*Я.Онлайн*|*Яндекс*"),					_T("Ya.Online"),						_T("ClientIcons_Jabber"),		IDI_YAONLINE,			JABBER_CASE,	TRUE	},
	{"client_yaxim",			_T("*yaxim*"),								_T("yaxim"),							_T("ClientIcons_Jabber"),		IDI_YAXIM,				JABBER_CASE,	TRUE	},

//########################################################################################################################################################################################################
//#################################		IRC		##########################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_AmIRC",			_T("*AmIRC*"),								_T("AmIRC"),							_T("ClientIcons_IRC"),			IDI_AMIRC,				IRC_CASE,		TRUE	},
	{"client_Babbel",			_T("*Babbel*"),								_T("Babbel"),							_T("ClientIcons_IRC"),			IDI_BABBEL,				IRC_CASE,		TRUE	},
	{"client_BersIRC",			_T("*BersIRC*"),							_T("BersIRC"),							_T("ClientIcons_IRC"),			IDI_BERSIRC,			IRC_CASE,		TRUE	},
	{"client_ChatZilla",		_T("*ChatZilla*"),							_T("ChatZilla"),						_T("ClientIcons_IRC"),			IDI_CHATZILLA,			IRC_CASE,		TRUE	},
	{"client_Conversation",		_T("*Conversation*"),						_T("Conversation"),						_T("ClientIcons_IRC"),			IDI_CONVERSATION,		IRC_CASE,		TRUE	},
	{"client_Eggdrop",			_T("*Eggdrop*"),							_T("Eggdrop"),							_T("ClientIcons_IRC"),			IDI_EGGDROP,			IRC_CASE,		TRUE	},
	{"client_EggdropRacBot",	_T("*Eggdrop*RacBot*"),						_T("Eggdrop RacBot"),					_T("ClientIcons_IRC"),			IDI_EGGDROP_RACBOT,		IRC_CASE,		TRUE	},
	{"client_FChat",			_T("*FChat*"),								_T("FChat"),							_T("ClientIcons_IRC"),			IDI_FCHAT,				IRC_CASE,		TRUE	},
	{"client_GDPChat",			_T("*GDPChat*"),							_T("GDP Web Chat"),						_T("ClientIcons_IRC"),			IDI_GDPCHAT,			IRC_CASE,		TRUE	},
	{"client_GoPowerTools",		_T("*Go*PowerTools*"),						_T("GoPowerTools"),						_T("ClientIcons_IRC"),			IDI_GOPOWERTOOLS,		IRC_CASE,		TRUE	},
	{"client_HydraIRC",			_T("*Hydra*IRC*"),							_T("HydraIRC"),							_T("ClientIcons_IRC"),			IDI_HYDRA_IRC,			IRC_CASE,		TRUE	},
	{"client_IRCXpro",			_T("*IRCXpro*"),							_T("IRCXpro"),							_T("ClientIcons_IRC"),			IDI_IRCXPRO,			IRC_CASE,		TRUE	},
	{"client_IceChat",			_T("*Ice*Chat*"),							_T("IceChat"),							_T("ClientIcons_IRC"),			IDI_ICECHAT,			IRC_CASE,		TRUE	},
	{"client_KSirc",			_T("*ksirk*"),								_T("KSirc"),							_T("ClientIcons_IRC"),			IDI_KSIRC,				IRC_CASE,		TRUE	},
	{"client_KVIrc",			_T("*KVIrc*"),								_T("KVIrc"),							_T("ClientIcons_IRC"),			IDI_KVIRC,				IRC_CASE,		TRUE	},
	{"client_Klient",			_T("*Klient*"),								_T("Klient"),							_T("ClientIcons_IRC"),			IDI_KLIENT,				IRC_CASE,		TRUE	},
	{"client_Konversation",		_T("*Konversation*"),						_T("Konversation"),						_T("ClientIcons_IRC"),			IDI_KONVERSATION,		IRC_CASE,		TRUE	},
	{"client_MP3Script",		_T("*MP3*Script*"),							_T("MP3 Script for mIRC"),				_T("ClientIcons_IRC"),			IDI_MP3_SCRIPT,			IRC_CASE,		TRUE	},
	{"client_NeoRaTrion",		_T("*NeoRa*Trion*"),						_T("NeoRa Trion"),						_T("ClientIcons_IRC"),			IDI_NEORATRION,			IRC_CASE,		TRUE	},
	{"client_Nettalk",			_T("*Nettalk*"),							_T("Nettalk"),							_T("ClientIcons_IRC"),			IDI_NETTALK,			IRC_CASE,		TRUE	},
	{"client_NoNameScript",		_T("*NoName*Script*"),						_T("NoNameScript"),						_T("ClientIcons_IRC"),			IDI_NONAME_SCRIPT,		IRC_CASE,		TRUE	},
	{"client_Opera",			_T("*Opera*"),								_T("Opera"),							_T("ClientIcons_IRC"),			IDI_OPERA,				IRC_CASE,		TRUE	},
	{"client_PJIRC",			_T("*PJIRC*"),								_T("PJIRC"),							_T("ClientIcons_IRC"),			IDI_PJIRC,				IRC_CASE,		TRUE	},
	{"client_Pirch",			_T("*Pirch*"),								_T("Pirch"),							_T("ClientIcons_IRC"),			IDI_PIRCH,				IRC_CASE,		TRUE	},
	{"client_PocketIRC",		_T("*Pocket*IRC*"),							_T("Pocket IRC"),						_T("ClientIcons_IRC"),			IDI_POCKET_IRC,			IRC_CASE,		TRUE	},
	{"client_ProChat",			_T("*Pro*Chat*"),							_T("ProChat"),							_T("ClientIcons_IRC"),			IDI_PROCHAT,			IRC_CASE,		TRUE	},
	{"client_SmartIRC",			_T("*Smart*IRC*"),							_T("SmartIRC"),							_T("ClientIcons_IRC"),			IDI_SMART_IRC,			IRC_CASE,		TRUE	},
	{"client_Snak",				_T("*Snak*"),								_T("Snak"),								_T("ClientIcons_IRC"),			IDI_SNAK,				IRC_CASE,		TRUE	},
	{"client_SysReset",			_T("*Sys*Reset*"),							_T("SysReset"),							_T("ClientIcons_IRC"),			IDI_SYSRESET,			IRC_CASE,		TRUE	},
	{"client_VircaIRC",			_T("*VircaIRC*"),							_T("VircaIRC"),							_T("ClientIcons_IRC"),			IDI_VIRCAIRC,			IRC_CASE,		TRUE	},
	{"client_VisionIRC",		_T("*VisionIRC*"),							_T("VisionIRC"),						_T("ClientIcons_IRC"),			IDI_VISIONIRC,			IRC_CASE,		TRUE	},
	{"client_VisualIRC",		_T("*VisualIRC*"),							_T("VisualIRC"),						_T("ClientIcons_IRC"),			IDI_VISUALIRC,			IRC_CASE,		TRUE	},
	{"client_VortecIRC",		_T("*VortecIRC*"),							_T("VortecIRC"),						_T("ClientIcons_IRC"),			IDI_VORTECIRC,			IRC_CASE,		TRUE	},
	{"client_WLIrc",			_T("*WLIrc*"),								_T("WLIrc"),							_T("ClientIcons_IRC"),			IDI_WLIRC,				IRC_CASE,		TRUE	},
	{"client_XChatAqua",		_T("*X*Chat*Aqua*"),						_T("X-Chat Aqua"),						_T("ClientIcons_IRC"),			IDI_XCHATAQUA,			IRC_CASE,		TRUE	},
	{"client_XiRCON",			_T("*XiRCON*"),								_T("XiRCON"),							_T("ClientIcons_IRC"),			IDI_XIRCON,				IRC_CASE,		TRUE	},
	{"client_Xirc",				_T("*Xirc*"),								_T("Xirc"),								_T("ClientIcons_IRC"),			IDI_XIRC,				IRC_CASE,		TRUE	},
	{"client_cbirc",			_T("*cbirc*"),								_T("cbirc"),							_T("ClientIcons_IRC"),			IDI_CBIRC,				IRC_CASE,		TRUE	},
	{"client_dIRC",				_T("*dIRC*"),								_T("dIRC"),								_T("ClientIcons_IRC"),			IDI_DIRC,				IRC_CASE,		TRUE	},

	{"client_iroffer_dinoex",	_T("*iroffer*dinoex*"),						_T("iroffer dinoex"),					_T("ClientIcons_IRC"),			IDI_IROFFER_DINOEX,		IRC_CASE,		TRUE	},
	{"client_iroffer",			_T("*iroffer*"),							_T("iroffer"),							_T("ClientIcons_IRC"),			IDI_IROFFER,			IRC_CASE,		TRUE	},

	{"client_ircle",			_T("*ircle*"),								_T("ircle"),							_T("ClientIcons_IRC"),			IDI_IRCLE,				IRC_CASE,		TRUE	},
	{"client_jircii",			_T("*jircii*"),								_T("jircii"),							_T("ClientIcons_IRC"),			IDI_JIRCII,				IRC_CASE,		TRUE	},
	{"client_jmIrc",			_T("*jmIrc*"),								_T("jmIrc"),							_T("ClientIcons_IRC"),			IDI_JMIRC,				IRC_CASE,		TRUE	},
	{"client_mIRC",				_T("*mIRC*"),								_T("mIRC"),								_T("ClientIcons_IRC"),			IDI_MIRC,				IRC_CASE,		TRUE	},
	{"client_pIRC",				_T("*pIRC*"),								_T("pIRC"),								_T("ClientIcons_IRC"),			IDI_PIRC,				IRC_CASE,		TRUE	},
	{"client_piorun",			_T("*piorun*"),								_T("Piorun"),							_T("ClientIcons_IRC"),			IDI_PIORUN,				IRC_CASE,		TRUE	},
	{"client_psyBNC",			_T("*psyBNC*"),								_T("psyBNC"),							_T("ClientIcons_IRC"),			IDI_PSYBNC,				IRC_CASE,		TRUE	},
	{"client_savIRC",			_T("*savIRC*"),								_T("savIRC"),							_T("ClientIcons_IRC"),			IDI_SAVIRC,				IRC_CASE,		TRUE	},
	{"client_wmIRC",			_T("*wmIRC*"),								_T("wmIRC"),							_T("ClientIcons_IRC"),			IDI_WMIRC,				IRC_CASE,		TRUE	},
	{"client_xBitch",			_T("*xBitch*"),								_T("xBitch"),							_T("ClientIcons_IRC"),			IDI_XBITCH,				IRC_CASE,		TRUE	},
	{"client_xChat",			_T("*xChat*"),								_T("xChat"),							_T("ClientIcons_IRC"),			IDI_XCHAT,				IRC_CASE,		TRUE	},
	{"client_zsIRC",			_T("*zsIRC*"),								_T("zsIRC"),							_T("ClientIcons_IRC"),			IDI_ZSIRC,				IRC_CASE,		TRUE	},

	{"client_ZNC",				_T("*ZNC*"),								_T("ZNC"),								_T("ClientIcons_IRC"),			IDI_ZNC,				IRC_CASE,		TRUE	},

	{"client_aMule",			_T("*aMule*"),								_T("aMule"),							_T("ClientIcons_IRC"),			IDI_AMULE,				IRC_CASE,		TRUE	},
	{"client_eMuleMorphXT",		_T("eMule*MorphXT*"),						_T("eMule MorphXT"),					_T("ClientIcons_IRC"),			IDI_EMULE_MORPHXT,		IRC_CASE,		TRUE	},
	{"client_eMuleNeo",			_T("eMule*Neo*"),							_T("eMule Neo"),						_T("ClientIcons_IRC"),			IDI_EMULE_NEO,			IRC_CASE,		TRUE	},
	{"client_eMulePlus",		_T("|eMule*plus*|eMule*+*"),				_T("eMule+"),							_T("ClientIcons_IRC"),			IDI_EMULE_PLUS,			IRC_CASE,		TRUE	},
	{"client_eMuleXtreme",		_T("eMule*Xtreme*"),						_T("eMule Xtreme"),						_T("ClientIcons_IRC"),			IDI_EMULE_XTREME,		IRC_CASE,		TRUE	},
	{"client_eMule",			_T("*eMule*"),								_T("eMule"),							_T("ClientIcons_IRC"),			IDI_EMULE,				IRC_CASE,		TRUE	},

	{"client_IRCUnknown",		_T("*IRC*"),								_T("Unknown IRC client"),				_T("ClientIcons_IRC"),			IDI_IRC,				IRC_CASE,		TRUE	},

//########################################################################################################################################################################################################
//#################################		AIM		##########################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_Aim7",				_T("AIM*7.*"),								_T("AIM v7.x"),							_T("ClientIcons_AIM"),			IDI_AIM_7,				AIM_CASE,		TRUE	},
	{"client_Aim6",				_T("AIM*6.*"),								_T("AIM v6.x"),							_T("ClientIcons_AIM"),			IDI_AIM_6,				AIM_CASE,		TRUE	},

	{"client_AIM_Triton",		_T("*AIM*Triton*"),							_T("AIM Triton"),						_T("ClientIcons_AIM"),			IDI_AIM_TRITON,			AIM_CASE,		TRUE	},
	{"client_AimMac",			_T("AIM*Mac*"),								_T("AIM for MacOS"),					_T("ClientIcons_AIM"),			IDI_AIM_MAC,			AIM_CASE,		TRUE	},
	{"client_AimMobile",		_T("|AIM*Mobile*|AIM*gprs*|Aim*sms*"),		_T("AIM Mobile"),						_T("ClientIcons_AIM"),			IDI_AIM_MOBILE,			AIM_CASE,		TRUE	},
	{"client_Naim",				_T("*naim*"),								_T("Naim"),								_T("ClientIcons_AIM"),			IDI_NAIM,				AIM_CASE,		TRUE	},
	{"client_miniaim",			_T("*miniaim*"),							_T("miniaim"),							_T("ClientIcons_AIM"),			IDI_MINIAIM,			AIM_CASE,		TRUE	},
	{"client_TerraIM",			_T("*Terra*"),								_T("TerraIM"),							_T("ClientIcons_AIM"),			IDI_TERRAIM,			AIM_CASE,		TRUE	},
	{"client_AIM",				_T("AIM*"),									_T("AIM"),								_T("ClientIcons_AIM"),			IDI_AIM_7,				AIM_CASE,		TRUE	},

//########################################################################################################################################################################################################
//#################################		MSN		##########################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_WLM11",			_T("WLM*2011*"),							_T("Windows Live 11"),					_T("ClientIcons_MSN"),			IDI_WLM_10,				MSN_CASE,		TRUE	},
	{"client_WLM10",			_T("WLM*2010*"),							_T("Windows Live 10"),					_T("ClientIcons_MSN"),			IDI_WLM_10,				MSN_CASE,		TRUE	},
	{"client_WLM9",				_T("WLM*9*"),								_T("Windows Live v9.x"),				_T("ClientIcons_MSN"),			IDI_WLM_9,				MSN_CASE,		TRUE	},
	{"client_MSN8",				_T("|WLM*8.*|MSN*8.*"),						_T("Windows Live v8.x"),				_T("ClientIcons_MSN"),			IDI_MSN_8,				MSN_CASE,		TRUE	},
	{"client_MSN7x",			_T("MSN*7.*"),								_T("MSN Messenger v7.x"),				_T("ClientIcons_MSN"),			IDI_MSN_7,				MSN_CASE,		TRUE	},
	{"client_MSN6x",			_T("MSN*6.*"),								_T("MSN Messenger v6.x"),				_T("ClientIcons_MSN"),			IDI_MSN_6,				MSN_CASE,		TRUE	},
	{"client_MSN45",			_T("MSN*4.x-5.x"),							_T("MSN Messenger v4.x-5.x"),			_T("ClientIcons_MSN"),			IDI_MSN_45,				MSN_CASE,		TRUE	},
	{"client_MSN4Mac",			_T("MSN*Mac*"),								_T("MSN for Mac"),						_T("ClientIcons_MSN"),			IDI_MSN_MAC,			MSN_CASE,		TRUE	},
	{"client_aMSN",				_T("*aMSN*"),								_T("aMSN"),								_T("ClientIcons_MSN"),			IDI_AMSN,				MSN_CASE,		TRUE	},
	{"client_MSN_Web",			_T("WebMessenger*"),						_T("WebMessenger"),						_T("ClientIcons_MSN"),			IDI_WEBMESSENGER,		MSN_CASE,		TRUE	},
	{"client_MSN",				_T("|WLM*|MSN*|Windows Live*"),				_T("MSN"),								_T("ClientIcons_MSN"),			IDI_MSN,				MSN_CASE,		TRUE	},

//########################################################################################################################################################################################################
//#################################		MAIL.RU 	######################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_MRA_official",		_T("|magent*|Mail.ru Agent*official*")
								_T("|Mail.ru Agent*PC*")
								_T("|*agent.mail.ru*"),						_T("Mail.Ru Agent (official)"),			_T("ClientIcons_MRA"),			IDI_MAIL_RU_OFFICIAL,	MRA_CASE,		TRUE	},
	{"client_MRA_Mobile",		_T("|wmagent*|MobileAgent*")
								_T("|sagent*|Mail.ru Agent*Symbian*")
								_T("|jagent*"),								_T("Mail.Ru Mobile Agent"),				_T("ClientIcons_MRA"),			IDI_MAIL_RU_MOBILE,		MRA_CASE,		TRUE	},
	{"client_MRA_web",			_T("Web Agent*"),							_T("Mail.Ru Web Agent"),				_T("ClientIcons_MRA"),			IDI_MAIL_RU_WEBAGENT,	MRA_CASE,		TRUE	},
	{"client_MRA_unknown",		_T("Mail.ru Agent*"),						_T("Mail.Ru (unknown client)"),			_T("ClientIcons_MRA"),			IDI_MAIL_RU_UNKNOWN,	MRA_CASE,		TRUE	},

//########################################################################################################################################################################################################
//#################################		YAHOO	##########################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_YahGoMobile",		_T("|Yahoo*Go*|Yahoo*Mobile*"),				_T("Yahoo! Go Mobile"),					_T("ClientIcons_Yahoo"),		IDI_YAHOO_GO_MOBILE,	YAHOO_CASE,		TRUE	},
	{"client_PingBox",			_T("*Yahoo*PingBox*"),						_T("Yahoo PingBox"),					_T("ClientIcons_Yahoo"),		IDI_YAHOO_PINGBOX,		YAHOO_CASE,		TRUE	},
	{"client_YahooWeb",			_T("*Yahoo*Web*Messenger"),					_T("Yahoo Web Messenger"),				_T("ClientIcons_Yahoo"),		IDI_YAHOO_WEB,			YAHOO_CASE,		TRUE	},
	{"client_libyahoo2",		_T("*libyahoo*"),							_T("libyahoo2"),						_T("ClientIcons_Yahoo"),		IDI_LIBYAHOO2,			YAHOO_CASE,		TRUE	},
	{"client_Yahoo",			_T("*Yahoo*"),								_T("Yahoo Messenger"),					_T("ClientIcons_Yahoo"),		IDI_YAHOO,				YAHOO_CASE	},

//########################################################################################################################################################################################################
//#################################		WEATHER		######################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_accu",				_T("*accuweather*"),						_T("AccuWeather"),						_T("ClientIcons_Weather"),		IDI_ACCU,				WEATHER_CASE,	TRUE	},
	{"client_gismeteo",			_T("*gismeteo*"),							_T("GisMeteo"),							_T("ClientIcons_Weather"),		IDI_GISMETEO,			WEATHER_CASE,	TRUE	},
	{"client_intelli",			_T("*intellicast*"),						_T("Intellicast"),						_T("ClientIcons_Weather"),		IDI_INTELLI,			WEATHER_CASE,	TRUE	},
	{"client_meteogid",			_T("|*meteo-gid*|*meteogid*"),				_T("Meteo-Gid"),						_T("ClientIcons_Weather"),		IDI_METEOGID,			WEATHER_CASE,	TRUE	},
	{"client_meteonovosti",		_T("*meteonovosti*"),						_T("Meteonovosti"),						_T("ClientIcons_Weather"),		IDI_METEONOVOSTI,		WEATHER_CASE,	TRUE	},
	{"client_noaa",				_T("*noaa*"),								_T("NOAA Weather"),						_T("ClientIcons_Weather"),		IDI_NOAA,				WEATHER_CASE,	TRUE	},
	{"client_real",				_T("*realmeteo*"),							_T("RealMeteo"),						_T("ClientIcons_Weather"),		IDI_REALMETEO,			WEATHER_CASE,	TRUE	},
	{"client_under",			_T("Weather Underground*"),					_T("Weather Underground"),				_T("ClientIcons_Weather"),		IDI_UNDERGROUND,		WEATHER_CASE,	TRUE	},
	{"client_weatherxml",		_T("*WeatherXML*"),							_T("WeatherXML"),						_T("ClientIcons_Weather"),		IDI_WEATHERXML,			WEATHER_CASE,	TRUE	},
	{"client_wetter",			_T("*wetter*"),								_T("Wetter"),							_T("ClientIcons_Weather"),		IDI_WETTER,				WEATHER_CASE,	TRUE	},
	{"client_yweather",			_T("*Yahoo Weather*"),						_T("Yahoo Weather"),					_T("ClientIcons_Weather"),		IDI_YWEATHER,			WEATHER_CASE,	TRUE	},
	{"client_weather_cn",		_T("*weather.com.cn*"),						_T("Weather CN"),						_T("ClientIcons_Weather"),		IDI_WEATHER_CN,			WEATHER_CASE,	TRUE	},
	{"client_weather",			_T("*weather*"),							_T("Weather"),							_T("ClientIcons_Weather"),		IDI_WEATHER,			WEATHER_CASE,	TRUE	},

//########################################################################################################################################################################################################
//#################################		RSS		##########################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_rss09x",			_T("*RSS*0.9*"),							_T("RSS 0.9x"),							_T("ClientIcons_RSS"),			IDI_RSS09,				RSS_CASE,		TRUE	},
	{"client_rss2",				_T("*RSS*2.*"),								_T("RSS 2"),							_T("ClientIcons_RSS"),			IDI_RSS2,				RSS_CASE,		TRUE	},
	{"client_rss1",				_T("*RSS*1.*"),								_T("RSS 1"),							_T("ClientIcons_RSS"),			IDI_RSS1,				RSS_CASE,		TRUE	},
	{"client_atom3",			_T("*Atom*3.*"),							_T("Atom 3"),							_T("ClientIcons_RSS"),			IDI_ATOM3,				RSS_CASE,		TRUE	},
	{"client_atom1",			_T("*Atom*1.*"),							_T("Atom 1"),							_T("ClientIcons_RSS"),			IDI_ATOM1,				RSS_CASE,		TRUE	},

//########################################################################################################################################################################################################
//#################################		QQ		##########################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_MobileQQ",			_T("|Mobile*QQ*|iPhone*QQ*|QQ WAP*"),		_T("Mobile QQ"),						_T("ClientIcons_QQ"),			IDI_MOBILEQQ,			QQ_CASE,		TRUE	},
	{"client_QQConcept",		_T("QQ*Concept*"),							_T("QQ Concept"),						_T("ClientIcons_QQ"),			IDI_QQCONCEPT,			QQ_CASE,		TRUE	},
	{"client_QQCRM",			_T("QQ*CRM*"),								_T("QQ CRM"),							_T("ClientIcons_QQ"),			IDI_QQCRM,				QQ_CASE,		TRUE	},
	{"client_QQSilver",			_T("QQ*Silverlight*"),						_T("QQ Silverlight"),					_T("ClientIcons_QQ"),			IDI_QQSILVER,			QQ_CASE,		TRUE	},
	{"client_TM2008",			_T("|TM2008*|TM2009*|TM2010*"),				_T("TM 2008 and >"),					_T("ClientIcons_QQ"),			IDI_TM2008,				QQ_CASE	},
	{"client_TM",				_T("TM*"),									_T("TM"),								_T("ClientIcons_QQ"),			IDI_TM,					QQ_CASE	},
	{"client_QQInt",			_T("QQ*International*"),					_T("QQ International"),					_T("ClientIcons_QQ"),			IDI_QQINT,				QQ_CASE	},
	{"client_YamQQ",			_T("YamQQ*"),								_T("Yam QQ"),							_T("ClientIcons_QQ"),			IDI_YAMQQ,				QQ_CASE	},
	{"client_WebQQ",			_T("WebQQ*"),								_T("WebQQ"),							_T("ClientIcons_QQ"),			IDI_WEBQQ,				QQ_CASE	},
	{"client_QQ",				_T("QQ*"),									_T("QQ"),								_T("ClientIcons_QQ"),			IDI_QQ,					QQ_CASE	},

//########################################################################################################################################################################################################
//#################################		GADU-GADU 	######################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_GG",				_T("|Gadu-Gadu*|GG*"),						_T("Gadu-Gadu client"),					_T("ClientIcons_GG"),			IDI_GG,					GG_CASE	},

//########################################################################################################################################################################################################
//#################################		TLEN	##########################################################################################################################################################
//########################################################################################################################################################################################################
	{"client_Tlen",				_T("*Tlen*"),								_T("Tlen.pl client"),					_T("ClientIcons_Tlen"),			IDI_TLEN,				TLEN_CASE	},

//########################################################################################################################################################################################################
//#################################		OTHER CLIENTS	##################################################################################################################################################
//########################################################################################################################################################################################################

	{"client_Android",			_T("*Android*"),							_T("Android"),							_T("ClientIcons_multiproto"),	IDI_ANDROID,			MULTI_CASE,	TRUE	},

	{"client_Facebook",			_T("*Facebook*"),							_T("Facebook"),							_T("ClientIcons_Jabber"),		IDI_FACEBOOK,			OTHER_PROTOS_CASE,	TRUE	},

	{"client_Pidgin",			_T("|*Pidgin*|*libpurple*|Purple*"),		_T("Pidgin (libpurple)"),				_T("ClientIcons_multiproto"),	IDI_PIDGIN,				MULTI_CASE	},

	{"client_Python",			_T("|*Python*|Py*|*ταλιςμαη*|Mrim*"),		_T("Python-based clients"),				_T("ClientIcons_multiproto"),	IDI_PYTHON,				MULTI_CASE	},

	{"client_Twitter",			_T("*Twitter*"),							_T("Twitter"),							_T("ClientIcons_Jabber"),		IDI_TWITTER,			OTHER_PROTOS_CASE,	TRUE	},

	{"client_Skype",			_T("*Skype**"),								_T("Skype"),							_T("ClientIcons_Others"),		IDI_SKYPE,				OTHER_PROTOS_CASE,	TRUE	},

	{"client_Xfire",			_T("*Xfire*"),								_T("Xfire"),							_T("ClientIcons_multiproto"),	IDI_XFIRE,				OTHER_PROTOS_CASE,	TRUE	},

//########################################################################################################################################################################################################
//#################################		UNDEFINED CLIENTS	##############################################################################################################################################
//########################################################################################################################################################################################################

	{"client_Notfound",			_T("Notfound"),								_T("client not found"),					_T(""),							IDI_NOTFOUND,			OTHERS_CASE,	TRUE	},
	{"client_Unknown",			_T("|*Unknown*|..."),						_T("unknown client"),					_T(""),							IDI_UNKNOWN,			OTHERS_CASE,	TRUE	},
	{"client_Undetected",		_T("?*"),									_T("undetected client"),				_T(""),							IDI_UNDETECTED,			OTHERS_CASE,	TRUE	},

};

int DEFAULT_KN_FP_MASK_COUNT = SIZEOF(def_kn_fp_mask);

//########################################################################################################################################################################################################
//#################################		OVERLAYS LAYER #1	##############################################################################################################################################
//########################################################################################################################################################################################################


KN_FP_MASK def_kn_fp_overlays_mask[] =
{//	{"Client_IconName",			_T("|^*Mask*|*names*"),						_T("Icon caption"),						_T("iconpack name"),			IDI_RESOURCE_ID,		CLIENT_CASE,	OVERLAY?	},
//########################################################################################################################################################################################################
//#################################		 MIRANDA PACKS OVERLAYS		######################################################################################################################################
//########################################################################################################################################################################################################
	{"client_AF_pack",			_T("*AF*Pack*"),							_T("AF"),								_T("ClientIcons_packs"),		IDI_MIRANDA_AF,			MIRANDA_PACKS_CASE	},
	{"client_AlfaMaR_pack",		_T("*AlfaMaR*"),							_T("AlfaMaR"),							_T("ClientIcons_packs"),		IDI_MIRANDA_ALFAMAR,	MIRANDA_PACKS_CASE	},
	{"client_Amatory_pack",		_T("*Amatory*"),							_T("Amatory"),							_T("ClientIcons_packs"),		IDI_MIRANDA_AMATORY,	MIRANDA_PACKS_CASE	},
	{"client_BRI_pack",			_T("*bri*edition*"),						_T("Bri edition"),						_T("ClientIcons_packs"),		IDI_MIRANDA_BRI,		MIRANDA_PACKS_CASE	},
	{"client_Devil_pack",		_T("*6.6.6*"),								_T("Devil Suite"),						_T("ClientIcons_packs"),		IDI_MIRANDA_DEVIL,		MIRANDA_PACKS_CASE	},
	{"client_E33_pack",			_T("*[E33*]*"),								_T("E33"),								_T("ClientIcons_packs"),		IDI_MIRANDA_E33,		MIRANDA_PACKS_CASE	},
	{"client_FR_pack",			_T("*FR*Pack*"),							_T("Miranda FR"),						_T("ClientIcons_packs"),		IDI_MIRANDA_FR,			MIRANDA_PACKS_CASE	},
	{"client_Faith_pack",		_T("*Faith*Pack*"),							_T("FaithPack"),						_T("ClientIcons_packs"),		IDI_MIRANDA_FAITH,		MIRANDA_PACKS_CASE	},
	{"client_Final_pack",		_T("*[Final*Pack]*"),						_T("Final pack"),						_T("ClientIcons_packs"),		IDI_MIRANDA_FINAL,		MIRANDA_PACKS_CASE	},
	{"client_Freize_pack",		_T("*Freize*"),								_T("Freize"),							_T("ClientIcons_packs"),		IDI_MIRANDA_FREIZE,		MIRANDA_PACKS_CASE	},
	{"client_Ghost_pack",		_T("*Ghost's*"),							_T("Ghost's pack"),						_T("ClientIcons_packs"),		IDI_MIRANDA_GHOST,		MIRANDA_PACKS_CASE	},
	{"client_HCoffee_pack",		_T("*[HotCoffee]*"),						_T("HotCoffee"),						_T("ClientIcons_packs"),		IDI_MIRANDA_CAPPUCCINO,	MIRANDA_PACKS_CASE	},
	{"client_HierOS_pack",		_T("*HierOS*"),								_T("HierOS"),							_T("ClientIcons_packs"),		IDI_MIRANDA_HIEROS,		MIRANDA_PACKS_CASE	},
	{"client_ICE_pack",			_T("|*miranda*[ice*]|*induction*"),			_T("iCE / Induction"),					_T("ClientIcons_packs"),		IDI_MIRANDA_INDUCTION,	MIRANDA_PACKS_CASE	},
	{"client_KDL_pack",			_T("|*KDL*|*КДЛ*"),							_T("KDL"),								_T("ClientIcons_packs"),		IDI_MIRANDA_KDL,		MIRANDA_PACKS_CASE	},
	{"client_Kolich_pack",		_T("*Kolich*"),								_T("Kolich"),							_T("ClientIcons_packs"),		IDI_MIRANDA_KOLICH,		MIRANDA_PACKS_CASE	},
	{"client_Kuzzman_pack",		_T("*kuzzman*"),							_T("Kuzzman"),							_T("ClientIcons_packs"),		IDI_MIRANDA_KUZZMAN,	MIRANDA_PACKS_CASE	},
	{"client_Lenin_pack",		_T("*[Lenin*]*"),							_T("Lenin pack"),						_T("ClientIcons_packs"),		IDI_MIRANDA_LENINPACK,	MIRANDA_PACKS_CASE	},
	{"client_Lestat_pack",		_T("*[Lpack*]*"),							_T("Lestat pack"),						_T("ClientIcons_packs"),		IDI_MIRANDA_LESTAT,		MIRANDA_PACKS_CASE	},
	{"client_LexSys_pack",		_T("|*Miranda*LS*|*LexSys*"),				_T("LexSys"),							_T("ClientIcons_packs"),		IDI_MIRANDA_LEXSYS,		MIRANDA_PACKS_CASE	},
	{"client_MD_pack",			_T("*MDpack*"),								_T("MDpack"),							_T("ClientIcons_packs"),		IDI_MIRANDA_MD,			MIRANDA_PACKS_CASE	},
	{"client_Mataes_pack",		_T("*Mataes*"),								_T("Mataes pack"),						_T("ClientIcons_packs"),		IDI_MIRANDA_MATAES,		MIRANDA_PACKS_CASE	},
	{"client_Mir_ME_pack",		_T("*[Miranda*ME]*"),						_T("Miranda ME"),						_T("ClientIcons_packs"),		IDI_MIRANDA_ME,			MIRANDA_PACKS_CASE	},
	{"client_Native_pack",		_T("*Native*"),								_T("Native"),							_T("ClientIcons_packs"),		IDI_MIRANDA_NATIVE,		MIRANDA_PACKS_CASE	},
	{"client_New_Style_pack",	_T("*New*Style*"),							_T("New Style"),						_T("ClientIcons_packs"),		IDI_MIRANDA_NEW_STYLE,	MIRANDA_PACKS_CASE	},
	{"client_Pilot_pack",		_T("*Pilot*"),								_T("Pilot"),							_T("ClientIcons_packs"),		IDI_MIRANDA_PILOT,		MIRANDA_PACKS_CASE	},
	{"client_Razunter_pack",	_T("*Razunter*"),							_T("Razunter's Pk"),					_T("ClientIcons_packs"),		IDI_MIRANDA_RAZUNTER,	MIRANDA_PACKS_CASE	},
	{"client_Robyer_pack",		_T("*Robyer*Pack*"),						_T("Robyer pack"),						_T("ClientIcons_packs"),		IDI_MIRANDA_ROBYER,		MIRANDA_PACKS_CASE	},
	{"client_SSS_pack",			_T("*sss*pack*"),							_T("SSS build"),						_T("ClientIcons_packs"),		IDI_MIRANDA_SSS_MOD,	MIRANDA_PACKS_CASE	},
	{"client_Se7ven_pack",		_T("|^*sss*|*[S7*pack]*|*[*S7*]*"),			_T("Se7ven"),							_T("ClientIcons_packs"),		IDI_MIRANDA_SE7VEN,		MIRANDA_PACKS_CASE	},
	{"client_SpellhowleR_pack",	_T("*Spellhowler*"),						_T("xSpellhowleRx pack"),				_T("ClientIcons_packs"),		IDI_MIRANDA_SPELLHOWLER,MIRANDA_PACKS_CASE	},
	{"client_Stalker_pack",		_T("*Stalker*"),							_T("Stalker"),							_T("ClientIcons_packs"),		IDI_MIRANDA_STALKER,	MIRANDA_PACKS_CASE	},
	{"client_Tweety_pack",		_T("*tweety*"),								_T("Tweety"),							_T("ClientIcons_packs"),		IDI_MIRANDA_TWEETY,		MIRANDA_PACKS_CASE	},
	{"client_Umedon_pack",		_T("*Miranda*Umedon*"),						_T("Umedon"),							_T("ClientIcons_packs"),		IDI_MIRANDA_UMEDON,		MIRANDA_PACKS_CASE	},
	{"client_ValeraVi_pack",	_T("*Valera*Vi*"),							_T("ValeraVi"),							_T("ClientIcons_packs"),		IDI_MIRANDA_VALERAVI,	MIRANDA_PACKS_CASE	},
	{"client_Watcher_pack",		_T("*Watcher*"),							_T("Watcher pack"),						_T("ClientIcons_packs"),		IDI_MIRANDA_WATCHER,	MIRANDA_PACKS_CASE	},
	{"client_YAOL_pack",		_T("*yaol*"),								_T("YAOL"),								_T("ClientIcons_packs"),		IDI_MIRANDA_YAOL,		MIRANDA_PACKS_CASE	},
	{"client_dar_veter_pack",	_T("*Dar*veter*"),							_T("Dar_veter pack"),					_T("ClientIcons_packs"),		IDI_MIRANDA_DAR,		MIRANDA_PACKS_CASE	},
	{"client_dmikos_pack",		_T("*dmikos*"),								_T("Dmikos"),							_T("ClientIcons_packs"),		IDI_MIRANDA_DMIKOS,		MIRANDA_PACKS_CASE	},
	{"client_zeleboba_pack",	_T("*zeleboba*"),							_T("zeleboba's"),						_T("ClientIcons_packs"),		IDI_MIRANDA_ZELEBOBA,	MIRANDA_PACKS_CASE	},

//########################################################################################################################################################################################################
//#################################		PROTO OVERLAYS		##############################################################################################################################################
//########################################################################################################################################################################################################
	{"client_AIM_overlay",		_T("|^AIM*|*AIM*"),							_T("AIM overlay"),						_T("ClientIcons_overlays"),		IDI_AIM_OVERLAY,		OVERLAYS_PROTO_CASE	},
	{"client_ICQJ_Plus_over",	_T("|^ICQ|^ICQ*|*ICQ*Plus*"),				_T("ICQJ Plus Mod overlay"),			_T("ClientIcons_overlays"),		IDI_ICQJ_PLUS_OVERLAY,	OVERLAYS_PROTO_CASE	},
	{"client_ICQJ_overlay",		_T("|^ICQ|^ICQ*|*ICQ*"),					_T("ICQJ overlay"),						_T("ClientIcons_overlays"),		IDI_ICQJ_OVERLAY,		OVERLAYS_PROTO_CASE	},
	{"client_IRC_overlay",		_T("|^IRC*|Miranda*IRC*"),					_T("IRC overlay"),						_T("ClientIcons_overlays"),		IDI_IRC_OVERLAY,		OVERLAYS_PROTO_CASE	},
	{"client_JGmail_overlay",	_T("*JGmail*"),								_T("JGmail overlay"),					_T("ClientIcons_overlays"),		IDI_GMAIL_OVERLAY,		OVERLAYS_PROTO_CASE	},
	{"client_JGTalk_overlay",	_T("*JGTalk*"),								_T("JGTalk overlay"),					_T("ClientIcons_overlays"),		IDI_JGTALK_OVERLAY,		OVERLAYS_PROTO_CASE	},
	{"client_Jabber_overlay",	_T("|^jabber*|Miranda*Jabber*|py*jabb*"),	_T("Jabber overlay"),					_T("ClientIcons_overlays"),		IDI_JABBER_OVERLAY,		OVERLAYS_PROTO_CASE	},
	{"client_MSN_overlay",		_T("|^MSN*|*Miranda*MSN*"),					_T("MSN overlay"),						_T("ClientIcons_overlays"),		IDI_MSN_OVERLAY,		OVERLAYS_PROTO_CASE	},
	{"client_QQ_overlay",		_T("|Miranda*QQ*|MIM*QQ*"),					_T("QQ overlay"),						_T("ClientIcons_overlays"),		IDI_QQ_OVERLAY,			OVERLAYS_PROTO_CASE	},
	{"client_Tlen_overlay",		_T("|^tlen*|*tlen*"),						_T("Tlen.pl overlay"),					_T("ClientIcons_overlays"),		IDI_TLEN_OVERLAY,		OVERLAYS_PROTO_CASE	},
	{"client_Yahoo_overlay",	_T("|^Yahoo*|*yahoo*"),						_T("Yahoo overlay"),					_T("ClientIcons_overlays"),		IDI_YAHOO_OVERLAY,		OVERLAYS_PROTO_CASE	},
	{"client_MRA_overlay",		_T("|^MRA*|^Mail.ru*")
								_T("|*MRA*|*Mail.ru*IM*|Mrim*"),			_T("Mail.Ru Agent overlay"),			_T("ClientIcons_overlays"),		IDI_MAILRU_OVERLAY,		OVERLAYS_PROTO_CASE	},

//########################################################################################################################################################################################################
//#################################		CLIENT VERSION OVERLAYS		######################################################################################################################################
//########################################################################################################################################################################################################
	{"client_ICQ3_over",		_T("ICQ*2003*"),							_T("ICQ 2003 overlay"),					_T("ClientIcons_ICQ"),			IDI_ICQ3_OVERLAY,		ICQ_CASE	},
	{"client_ICQ4_over",		_T("|ICQ*4*|ICQ*lite*4*"),					_T("ICQ Lite 4 overlay"),				_T("ClientIcons_ICQ"),			IDI_ICQ4_OVERLAY,		ICQ_CASE	},
	{"client_ICQ5_over",		_T("|ICQ*5*|ICQ*lite*5*|icq5*"),			_T("ICQ v5.x overlay"),					_T("ClientIcons_ICQ"),			IDI_ICQ5_OVERLAY,		ICQ_CASE	},
	{"client_ICQ6_over",		_T("ICQ*6*"),								_T("ICQ v6.x overlay"),					_T("ClientIcons_ICQ"),			IDI_ICQ6_OVERLAY,		ICQ_CASE	},
	{"client_ICQ7_over",		_T("ICQ*7*"),								_T("ICQ v7.x overlay"),					_T("ClientIcons_ICQ"),			IDI_ICQ7_OVERLAY,		ICQ_CASE	},
	{"client_ICQ8_over",		_T("ICQ*8*"),								_T("ICQ v8.x overlay"),					_T("ClientIcons_ICQ"),			IDI_ICQ8_OVERLAY,		ICQ_CASE	},

	{"client_Yahoo_11",			_T("*Yahoo 11*"),							_T("Yahoo v11.x overlay"),				_T("ClientIcons_Yahoo"),		IDI_YAHOO11_OVERLAY,	YAHOO_CASE	},
	{"client_Yahoo_10",			_T("*Yahoo 10*"),							_T("Yahoo v10.x overlay"),				_T("ClientIcons_Yahoo"),		IDI_YAHOO10_OVERLAY,	YAHOO_CASE	},
	{"client_Yahoo_9",			_T("*Yahoo 9*"),							_T("Yahoo v9.x overlay"),				_T("ClientIcons_Yahoo"),		IDI_YAHOO9_OVERLAY,		YAHOO_CASE	},

	{"client_GG_11",			_T("|Gadu-Gadu*11*|GG*11*"),				_T("Gadu-Gadu v11 client"),				_T("ClientIcons_GG"),			IDI_GG11_OVERLAY,		GG_CASE	},
	{"client_GG_10",			_T("|Gadu-Gadu*10*|GG*10"),					_T("Gadu-Gadu v10 client"),				_T("ClientIcons_GG"),			IDI_GG10_OVERLAY,		GG_CASE	},
	{"client_GG_9",				_T("|Gadu-Gadu*9*|GG*9*"),					_T("Gadu-Gadu v9 client"),				_T("ClientIcons_GG"),			IDI_GG9_OVERLAY,		GG_CASE	},
	{"client_GG_8",				_T("|Gadu-Gadu*8*|GG*8*"),					_T("Gadu-Gadu v8 client"),				_T("ClientIcons_GG"),			IDI_GG8_OVERLAY,		GG_CASE	},

//	{"client_Tlen_8",			_T("Tlen Protocol 1?"),						_T("Tlen 8.x overlay"),					_T("ClientIcons_Tlen"),			IDI_TLEN8_OVERLAY,		TLEN_CASE	},
	{"client_Tlen_7",			_T("Tlen Protocol 16"),						_T("Tlen 7.x overlay"),					_T("ClientIcons_Tlen"),			IDI_TLEN7_OVERLAY,		TLEN_CASE	},
	{"client_Tlen_6",			_T("Tlen Protocol 10"),						_T("Tlen 6.x overlay"),					_T("ClientIcons_Tlen"),			IDI_TLEN6_OVERLAY,		TLEN_CASE	},
	{"client_Tlen_5",			_T("Tlen Protocol 7"),						_T("Tlen 5.x overlay"),					_T("ClientIcons_Tlen"),			IDI_TLEN5_OVERLAY,		TLEN_CASE	},

	{"client_QQ_2012",			_T("|QQ2012*|TM2012*"),						_T("TM/QQ 2012 overlay"),				_T("ClientIcons_QQ"),			IDI_QQ_2012_OVERLAY,	QQ_CASE	},
	{"client_QQ_2011",			_T("|QQ2011*|TM2011*"),						_T("TM/QQ 2011 overlay"),				_T("ClientIcons_QQ"),			IDI_QQ_2011_OVERLAY,	QQ_CASE	},
	{"client_QQ_2010",			_T("|QQ2010*|TM2010*"),						_T("TM/QQ 2010 overlay"),				_T("ClientIcons_QQ"),			IDI_QQ_2010_OVERLAY,	QQ_CASE	},
	{"client_QQ_2009",			_T("|QQ2009*|TM2009*"),						_T("TM/QQ 2009 overlay"),				_T("ClientIcons_QQ"),			IDI_QQ_2009_OVERLAY,	QQ_CASE	},
	{"client_QQ_2008",			_T("|QQ2008*|TM2008*"),						_T("TM/QQ 2008 overlay"),				_T("ClientIcons_QQ"),			IDI_QQ_2008_OVERLAY,	QQ_CASE	},
	{"client_QQ_2007",			_T("|QQ2007*|TM2007*"),						_T("TM/QQ 2007 overlay"),				_T("ClientIcons_QQ"),			IDI_QQ_2007_OVERLAY,	QQ_CASE	},
	{"client_QQ_2006",			_T("|QQ2006*|TM2006*"),						_T("TM/QQ 2006 overlay"),				_T("ClientIcons_QQ"),			IDI_QQ_2006_OVERLAY,	QQ_CASE	},
	{"client_QQ_2005",			_T("|QQ2005*|TM2005*"),						_T("TM/QQ 2005 overlay"),				_T("ClientIcons_QQ"),			IDI_QQ_2005_OVERLAY,	QQ_CASE	},
	{"client_QQ_2004",			_T("QQ2004*"),								_T("QQ 2004 overlay"),					_T("ClientIcons_QQ"),			IDI_QQ_2004_OVERLAY,	QQ_CASE	},
	{"client_QQ_2003",			_T("|QQ2003*|TM 3.*"),						_T("TM 3.x / QQ 2003 overlay"),			_T("ClientIcons_QQ"),			IDI_QQ_2003_OVERLAY,	QQ_CASE	},
	{"client_TM_2002",			_T("TM 2.*"),								_T("TM 2.x overlay"),					_T("ClientIcons_QQ"),			IDI_QQ_2002_OVERLAY,	QQ_CASE	},
	{"client_TM_2001",			_T("TM 1.*"),								_T("TM 1.x overlay"),					_T("ClientIcons_QQ"),			IDI_QQ_2001_OVERLAY,	QQ_CASE	},
	{"client_QQ_2000",			_T("QQ2000*"),								_T("QQ 2000 overlay"),					_T("ClientIcons_QQ"),			IDI_QQ_2000_OVERLAY,	QQ_CASE	},

//########################################################################################################################################################################################################
//#################################		PLATFORM OVERLAYS	##############################################################################################################################################
//########################################################################################################################################################################################################
	{"client_on_Win32",			_T("|*Win32*|*windows*")
								_T("|* WM *|wmagent*|*Vista*|*win*7*"),		_T("Windows overlay"),					_T("ClientIcons_overlays"),		IDI_PLATFORM_WIN,		OVERLAYS_PLATFORM_CASE	},
	{"client_on_iOS",			_T("|*ipad*|*iphone*"),						_T("iOS overlay (iPhone/iPad)"),		_T("ClientIcons_overlays"),		IDI_PLATFORM_IOS,		OVERLAYS_PLATFORM_CASE	},
	{"client_on_Mac",			_T("|^*smack*|* Mac *|*mac*"),				_T("MacOS overlay"),					_T("ClientIcons_overlays"),		IDI_PLATFORM_MAC,		OVERLAYS_PLATFORM_CASE	},
	{"client_on_Linux",			_T("*Linux*"),								_T("Linux overlay"),					_T("ClientIcons_overlays"),		IDI_PLATFORM_LINUX,		OVERLAYS_PLATFORM_CASE	},
	{"client_on_Flash",			_T("|*Flash*|*Web*ICQ*"),					_T("Flash overlay"),					_T("ClientIcons_overlays"),		IDI_PLATFORM_FLASH,		OVERLAYS_PLATFORM_CASE	},
	{"client_on_Java",			_T("|*Java*|jagent*|ICQ2Go!*"),				_T("Java overlay"),						_T("ClientIcons_overlays"),		IDI_PLATFORM_JAVA,		OVERLAYS_PLATFORM_CASE	},
	{"client_on_Symbian",		_T("|*Symbian*|sagent*"),					_T("Symbian overlay"),					_T("ClientIcons_overlays"),		IDI_PLATFORM_SYMBIAN,	OVERLAYS_PLATFORM_CASE	},
	{"client_on_Amiga",			_T("*Amiga*"),								_T("Amiga overlay"),					_T("ClientIcons_overlays"),		IDI_PLATFORM_AMIGA,		OVERLAYS_PLATFORM_CASE	},
	{"client_on_Android",		_T("*Android*"),							_T("Android overlay"),					_T("ClientIcons_overlays"),		IDI_PLATFORM_ANDROID,	OVERLAYS_PLATFORM_CASE	},

};

int DEFAULT_KN_FP_OVERLAYS_COUNT = SIZEOF(def_kn_fp_overlays_mask);

//########################################################################################################################################################################################################
//########################################################################################################################################################################################################
//#################################		OVERLAYS LAYER #2		##########################################################################################################################################
//########################################################################################################################################################################################################
//########################################################################################################################################################################################################

KN_FP_MASK def_kn_fp_overlays2_mask[] =
{//	IconName						Mask										Icon caption							NULL ICON_RESOURCE_ID
	{"client_debug_overlay",	_T("|*[*debug*]*|*test*|*тест*"),			_T("debug overlay"),					_T("ClientIcons_overlays"),		IDI_DEBUG_OVERLAY,		OVERLAYS_RESOURCE_CASE	},
	{"client_office_overlay",	_T("|*[*office*]*|*[*офис*]*"),				_T("office overlay"),					_T("ClientIcons_overlays"),		IDI_OFFICE_OVERLAY,		OVERLAYS_RESOURCE_CASE	},
	{"client_mobile_overlay",	_T("|*[*mobile*]*|*[*pda*]*"),				_T("mobile overlay"),					_T("ClientIcons_overlays"),		IDI_MOBILE_OVERLAY,		OVERLAYS_RESOURCE_CASE	},
	{"client_home_overlay",		_T("|*[*home*]*|*[*дом*]*|*[*хоме*]*"),		_T("home overlay"),						_T("ClientIcons_overlays"),		IDI_HOME_OVERLAY,		OVERLAYS_RESOURCE_CASE	},

	{"client_work_overlay",		_T("|*[*work*]*|*wrk*")
								_T("|*[*работа*]*|*ворк*"),					_T("work overlay"),						_T("ClientIcons_overlays"),		IDI_WORK_OVERLAY,		OVERLAYS_RESOURCE_CASE	},

	{"client_note_overlay",		_T("|*[*note*]*|*[*laptop*]*|*ноут*")
								_T("|*[*кирпич*]*|*[*portable*]*")
								_T("|*[*flash*]*|*[*usb*]*"),				_T("notebook overlay"),					_T("ClientIcons_overlays"),		IDI_NOTEBOOK_OVERLAY,	OVERLAYS_RESOURCE_CASE	},

//	{"client_MirNG_09_over",	_T("*Miranda*NG*\?.\?\?.9.*"),	 			_T("MirandaNG v0.9 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG_V9,		MIRANDA_VERSION_CASE	},
//	{"client_MirNG_08_over",	_T("*Miranda*NG*\?.\?\?.8.*"),	 			_T("MirandaNG v0.8 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG_V8,		MIRANDA_VERSION_CASE	},
//	{"client_MirNG_07_over",	_T("*Miranda*NG*\?.\?\?.7.*"),	 			_T("MirandaNG v0.7 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG_V7,		MIRANDA_VERSION_CASE	},

	{"client_MirNG_06_over",	_T("*Miranda*NG*\?.\?\?.6.*"),	 			_T("MirandaNG v0.6 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG_V6,		MIRANDA_VERSION_CASE	},
	{"client_MirNG_05_over",	_T("*Miranda*NG*\?.\?\?.5.*"),	 			_T("MirandaNG v0.5 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG_V5,		MIRANDA_VERSION_CASE	},
	{"client_MirNG_04_over",	_T("*Miranda*NG*\?.\?\?.4.*"),	 			_T("MirandaNG v0.4 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG_V4,		MIRANDA_VERSION_CASE	},
	{"client_MirNG_03_over",	_T("*Miranda*NG*\?.\?\?.3.*"),	 			_T("MirandaNG v0.3 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG_V6,		MIRANDA_VERSION_CASE	},
	{"client_MirNG_02_over",	_T("*Miranda*NG*\?.\?\?.2.*"),	 			_T("MirandaNG v0.2 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG_V5,		MIRANDA_VERSION_CASE	},
	{"client_MirNG_01_over",	_T("*Miranda*NG*\?.\?\?.1.*"),	 			_T("MirandaNG v0.1 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_NG_V4,		MIRANDA_VERSION_CASE	},

	{"client_MirIM_010_over",	_T("*Miranda*0.10.*"),						_T("MirandaIM v0.10 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_IM_V10,		MIRANDA_VERSION_CASE	},
	{"client_MirIM_09_over",	_T("*Miranda*0.9.*"),						_T("MirandaIM v0.9 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_IM_V9,		MIRANDA_VERSION_CASE	},
	{"client_MirIM_08_over",	_T("*Miranda*0.8.*"),						_T("MirandaIM v0.8 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_IM_V8,		MIRANDA_VERSION_CASE	},
	{"client_MirIM_07_over",	_T("*Miranda*0.7.*"),						_T("MirandaIM v0.7 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_IM_V7,		MIRANDA_VERSION_CASE	},
	{"client_MirIM_06_over",	_T("*Miranda*0.6.*"),						_T("MirandaIM v0.6 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_IM_V6,		MIRANDA_VERSION_CASE	},
	{"client_MirIM_05_over",	_T("*Miranda*0.5.*"),						_T("MirandaIM v0.5 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_IM_V5,		MIRANDA_VERSION_CASE	},
	{"client_MirIM_04_over",	_T("*Miranda*0.4.*"),						_T("MirandaIM v0.4 #2 overlay"),		_T("ClientIcons_Miranda"),		IDI_MIRANDA_IM_V4,		MIRANDA_VERSION_CASE	},
};

int DEFAULT_KN_FP_OVERLAYS2_COUNT = SIZEOF(def_kn_fp_overlays2_mask);

//########################################################################################################################################################################################################
//########################################################################################################################################################################################################
//##############################	 OVERLAYS LAYER #3		##############################################################################################################################################
//########################################################################################################################################################################################################
KN_FP_MASK def_kn_fp_overlays3_mask[] =
{//	{"Client_IconName",			_T("|^*Mask*|*names*"),						_T("Icon caption"),						_T("iconpack name"),			IDI_RESOURCE_ID,		CLIENT_CASE,	OVERLAY?	},
	{"client_SecureIM_over",	_T("*Secure*IM*"),							_T("SecureIM overlay"),					_T("ClientIcons_overlays"),		IDI_SECUREIM_OVERLAY,	OVERLAYS_SECURITY_CASE	},

	{"client_platform_x64",		_T("|*x64*|*64*bit*"),						_T("x64 overlay"),						_T("ClientIcons_overlays"),		IDI_PLATFORM_X64,		OVERLAYS_PLATFORM_CASE	},
	{"client_platform_x32",		_T("|*x32*|*32*bit*|*x86*"),				_T("x32 overlay"),						_T("ClientIcons_overlays"),		IDI_PLATFORM_X32,		OVERLAYS_PLATFORM_CASE	},

	{"client_Unicode",			_T("*unicode*"),							_T("Unicode overlay"),					_T("ClientIcons_overlays"),		IDI_UNICODE_CLIENT,		OVERLAYS_RESOURCE_CASE	},
};

int DEFAULT_KN_FP_OVERLAYS3_COUNT = SIZEOF(def_kn_fp_overlays3_mask);

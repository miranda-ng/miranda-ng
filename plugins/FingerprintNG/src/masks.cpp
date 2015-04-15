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

#include <m_version.h>

/************************************************************************/
/* This file contains data about appropriate MirVer values              */
/************************************************************************/

/*
*    NOTE: Masks can contain '*' or '?' wild symbols
*    Asterics '*' symbol covers 'empty' symbol too e.g WildCompare("Tst","T*st*"), returns TRUE
*    In order to handle situation 'at least one any sybol' use '?*' combination:
*    e.g WildCompare("Tst","T?*st*"), returns FALSE, but both WildCompare("Test","T?*st*") and
*    WildCompare("Teeest","T?*st*") return TRUE.
*
*    Function is 'dirt' case insensitive (it is ignore 5th bit (0x20) so it is no difference
*    beetween some symbols. But it is faster than valid converting to uppercase.
*
*    Mask can contain several submasks. In this case each submask (including first)
*    should start from '|' e.g: "|first*submask|second*mask".
*
*    ORDER OF RECORDS IS IMPORTANT: system search first suitable mask and returns it.
*    e.g. if MirVer is "Miranda IM" and first mask is "*im*" and second is "Miranda *" the
*    result will be client associated with first mask, not second!
*    So in order to avoid such situation, place most generalised masks to latest place.
*
*    In order to get "Unknown" client, last mask should be "?*".
*/

#define NIGHTLY_CORE_VERSION _T("|*Miranda*NG*") _T(MIRANDA_VERSION_CORE_STRING) _T("*")

KN_FP_MASK def_kn_fp_mask[] =
{// {"Client_IconName",         _T("|^*Mask*|*names*"),                         _T("Icon caption"),                 IDI_RESOURCE_ID,         CLIENT_CASE,    OVERLAY?    },
	//#########################################################################################################################################################################
	//#################################   MIRANDA   ###########################################################################################################################
	//#########################################################################################################################################################################

	{ "client_Miranda_NG", _T("|*nightly.miranda.im*")
	NIGHTLY_CORE_VERSION, _T("Miranda NG"), IDI_MIRANDA_NG, MIRANDA_CASE },

	{ "client_Miranda_NG_stbl", _T("|Miranda*NG*"), LPGENT("Miranda NG stable"), IDI_MIRANDA_NG_STABLE, MIRANDA_CASE },

	{ "client_Miranda_010xx", _T("Miranda*IM*0.10.*"), _T("Miranda IM v0.10.x.x"), IDI_MIRANDA_010X, MIRANDA_CASE },
	{ "client_Miranda_09XX", _T("|*Miranda*IM*0.9*")
	_T("|*miranda-im.org/caps#*0.9*"), _T("Miranda IM v0.9.x.x"), IDI_MIRANDA_09XX, MIRANDA_CASE },
	{ "client_Miranda_08XX", _T("|^Miranda*0.7*Jabb*")
	_T("|*Miranda*0.8*")
	_T("|*miranda-im.org/caps#*0.8*"), _T("Miranda IM v0.8.x.x"), IDI_MIRANDA_08XX, MIRANDA_CASE },
	{ "client_Miranda_07XX", _T("|*Miranda*0.7*")
	_T("|*miranda-im.org/caps#*0.7*"), _T("Miranda IM v0.7.x.x"), IDI_MIRANDA_07XX, MIRANDA_CASE },
	{ "client_Miranda_06XX", _T("*Miranda*0.6*"), _T("Miranda IM v0.6.x.x"), IDI_MIRANDA_06XX, MIRANDA_CASE },
	{ "client_Miranda_05XX", _T("*Miranda*0.5*"), _T("Miranda IM v0.5.x.x"), IDI_MIRANDA_05XX, MIRANDA_CASE },
	{ "client_Miranda_04XX", _T("*Miranda*0.4*"), _T("Miranda IM v0.4.x.x"), IDI_MIRANDA_04XX, MIRANDA_CASE },

	{ "client_Miranda_old", _T("*Miranda*0.?*"), LPGENT("Miranda IM (old versions)"), IDI_MIRANDA_OLD, MIRANDA_CASE },

	{ "client_Miranda_unknown", _T("*Miranda*"), LPGENT("Miranda (unknown)"), IDI_MIRANDA_UNKNOWN, MIRANDA_CASE },

	//#########################################################################################################################################################################
	//#################################   MULTI-PROTOCOL   ####################################################################################################################
	//#########################################################################################################################################################################
	{ "client_1&1", _T("|*Pocket*Web*|*1&1*|*1*?nd*1"), _T("1&1"), IDI_1UND1, MULTI_CASE, TRUE },
	{ "client_Asia", _T("*Asia*"), _T("Asia"), IDI_ASIA, MULTI_CASE, TRUE },
	{ "client_Adium", _T("|*Adium*")
	_T("|*VUFD6HcFmUT2NxJkBGCiKlZnS3M=*")          // Gabble client?
	_T("|*DdnydQG7RGhP9E3k9Sf+b+bF0zo=*"), _T("Adium"), IDI_ADIUM, MULTI_CASE, TRUE },
	{ "client_AgileMessenger", _T("*Agile Messenger*"), _T("Agile Messenger"), IDI_AGILE, MULTI_CASE, TRUE },

	{ "client_Appolo", _T("*Appolo*"), _T("Appolo IM"), IDI_APPOLO, MULTI_CASE, TRUE },
	{ "client_Ayttm", _T("*Ayttm*"), _T("Ayttm"), IDI_AYTTM, MULTI_CASE, TRUE },
	{ "client_BayanICQ", _T("|*Bayan*ICQ*|*barobin*"), _T("BayanICQ"), IDI_BAYANICQ, MULTI_CASE, TRUE },
	{ "client_Beejive", _T("*Beejive*"), _T("BeejiveIM"), IDI_BEEJIVE, MULTI_CASE, TRUE },
	{ "client_Bimoid", _T("*Bimoid*"), _T("Bimoid"), IDI_BIMOID, MULTI_CASE, TRUE },
	{ "client_BitlBee", _T("*BitlBee*"), _T("BitlBee"), IDI_BITLBEE, MULTI_CASE, TRUE },
	{ "client_BlackBerry", _T("|*Berry*|*ICS?\?\?\?\?\?\?\?"), _T("BlackBerry"), IDI_BLACKBERRY, MULTI_CASE, TRUE },
	{ "client_Buddycloud", _T("*Buddycloud*"), _T("Buddycloud"), IDI_BUDDYCLOUD, MULTI_CASE, TRUE },
	{ "client_Carrier", _T("*Carrier*"), _T("Carrier client"), IDI_CARRIER, MULTI_CASE, TRUE },
	{ "client_Causerie", _T("*Causerie*"), _T("Causerie"), IDI_CAUSERIE, MULTI_CASE, TRUE },
	{ "client_CenterIM", _T("*CenterIM*"), _T("CenterIM"), IDI_CENTERIM, MULTI_CASE, TRUE },
	{ "client_ChatSecure", _T("*ChatSecure*"), _T("ChatSecure"), IDI_CHATSECURE, MULTI_CASE, TRUE },
	{ "client_Citron", _T("*Citron*"), _T("Citron"), IDI_CITRON, MULTI_CASE, TRUE },
	{ "client_climm", _T("*climm*"), _T("climm"), IDI_CLIMM, MULTI_CASE, TRUE },
	{ "client_Digsby", _T("*Digsby*"), _T("Digsby"), IDI_DIGSBY, MULTI_CASE, TRUE },
	{ "client_EKG2", _T("*EKG*2*"), _T("EKG2"), IDI_EKG2, MULTI_CASE, TRUE },
	{ "client_EasyMessage", _T("Easy*Message*"), _T("Easy Message"), IDI_EASYMESSAGE, MULTI_CASE, TRUE },
	{ "client_Empathy", _T("*Empathy*"), _T("Empathy"), IDI_EMPATHY, MULTI_CASE, TRUE },
	{ "client_Eyeball", _T("*Eyeball*"), _T("Eyeball Chat"), IDI_EYEBALL_CHAT, MULTI_CASE, TRUE },
	{ "client_eBuddy", _T("|*eBuddy*|*eMessenger*"), _T("eBuddy Messenger"), IDI_EBUDDY, MULTI_CASE, TRUE },
	{ "client_eM_Client", _T("*eM*Client*"), _T("eM Client"), IDI_EM_CLIENT, MULTI_CASE, TRUE },
	{ "client_eQo", _T("*eQo*"), _T("eQo"), IDI_EQO, MULTI_CASE, TRUE },
	{ "client_emesene", _T("*emesene*"), _T("emesene"), IDI_EMESENE, MULTI_CASE, TRUE },
	{ "client_Fama", _T("*Fama*"), _T("Fama IM"), IDI_FAMAIM, MULTI_CASE, TRUE },
	{ "client_Fring", _T("*fring*"), _T("Fring"), IDI_FRING, MULTI_CASE, TRUE },
	{ "client_GMX", _T("*GMX*"), _T("GMX MultiMessenger"), IDI_GMX, MULTI_CASE, TRUE },
	{ "client_Gaim", _T("*gaim*"), _T("Gaim (libgaim)"), IDI_GAIM, MULTI_CASE, TRUE },
	{ "client_Galaxium", _T("*Galaxium*"), _T("Galaxium"), IDI_GALAXIUM, MULTI_CASE, TRUE },
	{ "client_GnuGadu", _T("Gnu Gadu*"), _T("GNU Gadu"), IDI_GNUGADU, MULTI_CASE, TRUE },
	{ "client_IM2", _T("*IM2*"), _T("IM2"), IDI_IM2, MULTI_CASE, TRUE },
	{ "client_IMPlus", _T("|IM+*|IMPLUS*|*IM plus*|@mobile")
	_T("|*+umBU9yx9Cu+R8jvPWIZ3vWD59M=*"), _T("IM+"), IDI_IMPLUS, MULTI_CASE, TRUE },
	{ "client_IMVU", _T("*IMVU*"), _T("IMVU"), IDI_IMVU, MULTI_CASE, TRUE },
	{ "client_IMadering", _T("*IMadering*"), _T("IMadering"), IDI_IMADERING, MULTI_CASE, TRUE },
	{ "client_imoIM", _T("|*imo.im*|*sleekxmpp.com*"), _T("imo.im"), IDI_IMOIM, MULTI_CASE, TRUE },
	{ "client_Inlux", _T("*Inlux*"), _T("Inlux Messenger"), IDI_INLUX, MULTI_CASE, TRUE },
	{ "client_Instantbird", _T("*Instantbird*"), _T("Instantbird"), IDI_INSTANTBIRD, MULTI_CASE, TRUE },
	{ "client_iCall", _T("*iCall*"), _T("iCall"), IDI_ICALL, MULTI_CASE, TRUE },
	{ "client_iChat", _T("|*iChat*|**imagent*|*iMessages*"), _T("iChat"), IDI_ICHAT, MULTI_CASE, TRUE },
	{ "client_irssi", _T("*irssi*"), _T("irssi"), IDI_IRSSI, MULTI_CASE, TRUE },
	{ "client_JBother", _T("*JBother*"), _T("JBother"), IDI_JBOTHER, MULTI_CASE, TRUE },
	{ "client_JBuddy", _T("*JBuddy*"), _T("JBuddy Messenger"), IDI_JBUDDY, MULTI_CASE, TRUE },
	{ "client_Jabbear", _T("*Jabbear*"), _T("Jabbear"), IDI_JABBEAR, MULTI_CASE, TRUE },
	{ "client_Jabbin", _T("*Jabbin*"), _T("Jabbin"), IDI_JABBIN, MULTI_CASE, TRUE },
	{ "client_Jasmine_IM", _T("|Jasmine*IM*|*jasmineicq.ru/caps*"), _T("Jasmine IM"), IDI_JASMINEIM, MULTI_CASE, TRUE },

	{ "client_Jimm_Aspro", _T("*Jimm*Aspro*"), _T("Jimm Aspro"), IDI_JIMM_ASPRO, MULTI_CASE, TRUE },
	{ "client_Jimmy", _T("*Jimmy*"), _T("JimmyIM"), IDI_JIMMY, MULTI_CASE, TRUE },

	{ "client_KMess", _T("*KMess*"), _T("KMess"), IDI_KMESS, MULTI_CASE, TRUE },
	{ "client_KoolIM", _T("*Kool*"), _T("KoolIM"), IDI_KOOLIM, MULTI_CASE, TRUE },
	{ "client_Kopete", _T("*Kopete*"), _T("Kopete"), IDI_KOPETE, MULTI_CASE, TRUE },
	{ "client_LeechCraft", _T("|*LeechCraft*")
	_T("|*aNjQWbtza2QtXemMfBS2bwNOtcQ=*"), _T("LeechCraft"), IDI_LEECHCRAFT, MULTI_CASE, TRUE },
	{ "client_MDC", _T("*MDC*"), _T("MDC"), IDI_MDC, MULTI_CASE, TRUE },
	{ "client_Meebo", _T("Meebo*"), _T("Meebo"), IDI_MEEBO, MULTI_CASE, TRUE },
	{ "client_Meetro", _T("Meetro*"), _T("Meetro"), IDI_MEETRO, MULTI_CASE, TRUE },
	{ "client_mChat", _T("|mChat*|gsICQ*|*mchat.mgslab.com*"), _T("mChat"), IDI_MCHAT, MULTI_CASE, TRUE },
	{ "client_Nimbuzz", _T("*Nimbuzz*"), _T("Nimbuzz"), IDI_NIMBUZZ, MULTI_CASE, TRUE },
	{ "client_Palringo", _T("*Palringo*"), _T("Palringo"), IDI_PALRINGO, MULTI_CASE, TRUE },

	{ "client_Pigeon", _T("*PIGEON*"), _T("PIGEON!"), IDI_PIGEON, MULTI_CASE, TRUE },
	{ "client_PlayXpert", _T("*PlayXpert*"), _T("PlayXpert"), IDI_PLAYXPERT, MULTI_CASE, TRUE },
	{ "client_Prelude", _T("*Prelude*"), _T("Prelude"), IDI_PRELUDE, MULTI_CASE, TRUE },
	{ "client_Proteus", _T("*Proteus*"), _T("Proteus"), IDI_PROTEUS, MULTI_CASE, TRUE },

	{ "client_QIP_Android", _T("QIP *Android*"), _T("QIP Mobile Android"), IDI_QIP_ANDROID, MULTI_CASE, TRUE },
	{ "client_QIP_iOS", _T("|QIP*iOS*|QIP*iphone*|QIP*apple*"), _T("QIP Mobile iOS"), IDI_QIP_IOS, MULTI_CASE, TRUE },
	{ "client_QIP_Symbian", _T("*QIP*Symbian*"), _T("QIP Mobile Symbian"), IDI_QIP_SYMBIAN, MULTI_CASE, TRUE },
	{ "client_QIP_Java", _T("*QIP*Java*"), _T("QIP Mobile Java"), IDI_QIP_JAVA, MULTI_CASE, TRUE },
	{ "client_QIP_PDA", _T("|QIP *PDA*|*pda.qip.ru*|*QIP Mobile*"), _T("QIP Mobile"), IDI_QIP_PDA, MULTI_CASE, TRUE },
	{ "client_QIP_2012", _T("|QIP 2012*|http://qip.ru/caps*"), _T("QIP 2012"), IDI_QIP_2012, MULTI_CASE, TRUE },
	{ "client_QIP_2010", _T("|QIP 2010*|http://2010.qip.ru*"), _T("QIP 2010"), IDI_QIP_2010, MULTI_CASE, TRUE },
	{ "client_QIP_Infium", _T("|QIP Infium*|http://*qip*"), _T("QIP Infium"), IDI_QIP_INFIUM, MULTI_CASE, TRUE },

	{ "client_qutIM", _T("*qutIM*"), _T("qutIM"), IDI_QUTIM, MULTI_CASE },
	{ "client_mqutIM", _T("*mqutIM*"), _T("mqutIM"), IDI_MQUTIM, MULTI_CASE },

	{ "client_Qnext", _T("Qnext*"), _T("Qnext"), IDI_QNEXT, MULTI_CASE, TRUE },

	{ "client_SAPO", _T("*SAPO*"), _T("SAPO Messenger"), IDI_SAPO, MULTI_CASE, TRUE },
	{ "client_SIM", _T("|^*Simp*|*SIM*"), _T("SIM"), IDI_SIM, MULTI_CASE, TRUE },
	{ "client_Salut_a_Toi", _T("*Salut*Toi*"), _T("Salut a Toi"), IDI_SALUT_A_TOI, MULTI_CASE, TRUE },
	{ "client_Shaim", _T("*Shaim*"), _T("Shaim"), IDI_SHAIM, MULTI_CASE, TRUE },
	{ "client_SieJC", _T("|SieJC*|Nat*ICQ*|Siemens*Client*"), _T("Siemens ICQ / Jabber client"), IDI_SIEJC, MULTI_CASE, TRUE },
	{ "client_Slick", _T("Slick*"), _T("Slick"), IDI_SLICK, MULTI_CASE, TRUE },
	{ "client_SrevIM", _T("*Srev*IM*"), _T("SrevIM"), IDI_SREVIM, MULTI_CASE, TRUE },

	{ "client_Tril_Android", _T("*Trillian*Android*"), _T("Trillian Android"), IDI_TRILLIAN_ANDROID, MULTI_CASE, TRUE },
	{ "client_Tril_Astra", _T("Trillian*Astra*"), _T("Trillian Astra"), IDI_TRILLIAN_ASTRA, MULTI_CASE, TRUE },
	{ "client_Trillian_Pro", _T("Trillian*Pro*"), _T("Trillian Pro"), IDI_TRILLIAN_PRO, MULTI_CASE, TRUE },
	{ "client_Trillian", _T("*Trillian**"), _T("Trillian"), IDI_TRILLIAN, MULTI_CASE, TRUE },

	{ "client_Tuukle_Chat", _T("*Tuukle*Chat*|*IM*Gate*"), _T("Tuukle Chat"), IDI_TUUKLE_CHAT, MULTI_CASE, TRUE },
	{ "client_vBuzzer", _T("*vBuzzer*"), _T("vBuzzer"), IDI_VBUZZER, MULTI_CASE, TRUE },
	{ "client_Virtus", _T("*Virtus*"), _T("Virtus"), IDI_VIRTUS, MULTI_CASE, TRUE },
	{ "client_uIM", _T("*uIM*"), _T("uIM"), IDI_UIM, MULTI_CASE, TRUE },
	{ "client_uTalk", _T("*uTalk*"), _T("uTalk"), IDI_UTALK, MULTI_CASE, TRUE },
	{ "client_WeeChat", _T("*WeeChat*"), _T("WeeChat"), IDI_WEECHAT, MULTI_CASE, TRUE },
	{ "client_Wippien", _T("*Wippien*"), _T("Wippien"), IDI_WIPPIEN, MULTI_CASE, TRUE },

	{ "client_WindowsPhone", _T("*Windows*Phone*"), _T("Windows Phone"), IDI_WINDOWS_PHONE, MULTI_CASE, TRUE },

	{ "client_YamiGo", _T("YamiGo*"), _T("YamiGo"), IDI_YAMIGO, MULTI_CASE, TRUE },
	{ "client_Yeigo", _T("*Yeigo*"), _T("Yeigo"), IDI_YEIGO, MULTI_CASE, TRUE },
	{ "client_Yoono", _T("*Yoono*"), _T("Yoono"), IDI_YOONO, MULTI_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   ICQ   ###############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_D[i]Chat", _T("D[i]Chat*"), _T("D[i]Chat"), IDI_DICHAT, ICQ_CASE, TRUE },
	{ "client_GnomeICU", _T("GnomeICU*"), _T("GnomeICU"), IDI_GNOMEICU, ICQ_CASE, TRUE },
	{ "client_Icy_Juice", _T("*Icy*Juice*"), _T("Icy Juice"), IDI_ICY_JUICE, ICQ_CASE, TRUE },
	{ "client_Jimm", _T("|*Jimm*|mobicq*"), _T("Jimm"), IDI_JIMM, ICQ_CASE, TRUE },
	{ "client_Licq", _T("Licq*"), _T("Licq"), IDI_LICQ, ICQ_CASE, TRUE },
	{ "client_LocID", _T("LocID*"), _T("LocID"), IDI_LOCID, ICQ_CASE, TRUE },
	{ "client_QIP_2005", _T("QIP*2005*"), _T("QIP 2005"), IDI_QIP_2005, ICQ_CASE, TRUE },
	{ "client_R&Q", _T("|R&Q*|&RQ*"), _T("R&Q"), IDI_RANDQ, ICQ_CASE, TRUE },
	{ "client_SmartICQ", _T("Smart*ICQ*"), _T("SmartICQ"), IDI_SMARTICQ, ICQ_CASE, TRUE },
	{ "client_StrICQ", _T("StrICQ*"), _T("StrICQ"), IDI_STRICQ, ICQ_CASE, TRUE },

	{ "client_spam", _T("*spam*"), LPGENT("spam client"), IDI_SPAM, ICQ_CASE, TRUE },
	{ "client_libicq2k", _T("libicq*"), _T("LibICQ/JIT"), IDI_LIBICQ2000, ICQ_CASE, TRUE },
	{ "client_TICQClient", _T("*TICQ*Client*"), _T("TICQClient (Delphi lib)"), IDI_TICQ, ICQ_CASE, TRUE },

	{ "client_ICQ_all", _T("|ICQ?|ICQ?.?|ICQ *|ICQ2*|ICQ"), _T("ICQ client"), IDI_ICQ, ICQ_CASE },

	//#########################################################################################################################################################################
	//#################################   JABBER   ############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_Akeni", _T("*Akeni*"), _T("Akeni"), IDI_AKENI, JABBER_CASE, TRUE },
	{ "client_Ambrosia", _T("*Ambrosia*"), _T("Ambrosia XMPP Server"), IDI_AMBROSIA, JABBER_CASE, TRUE },
	{ "client_AnothRSSBot", _T("|*Anothrbot*|*Anothr Rss Bot*"), _T("Anothr Rss Bot"), IDI_ANOTHRSSBOT, JABBER_CASE, TRUE },
	{ "client_Aqq", _T("|*aqq.eu*|aqq*"), _T("Aqq"), IDI_AQQ, JABBER_CASE, TRUE },
	{ "client_BarnOwl", _T("*Barn*Owl*"), _T("BarnOwl"), IDI_BARNOWL, JABBER_CASE, TRUE },
	{ "client_Beem", _T("*Beem*"), _T("Beem"), IDI_BEEM, JABBER_CASE, TRUE },
	{ "client_BellSouth", _T("*BellSouth*"), _T("BellSouth"), IDI_BELLSOUTH, JABBER_CASE, TRUE },
	{ "client_BitWise", _T("*BitWise*"), _T("BitWise"), IDI_BITWISE, JABBER_CASE, TRUE },
	{ "client_Bombus", _T("*Bombus*"), _T("Bombus"), IDI_BOMBUS, JABBER_CASE, TRUE },
	{ "client_BombusMod", _T("|*Bombus*mod*|*bombusmod*"), _T("BombusMod"), IDI_BOMBUS_MOD, JABBER_CASE, TRUE },
	{ "client_BombusNG", _T("*Bombus*NG*"), _T("Bombus NG"), IDI_BOMBUS_NG, JABBER_CASE, TRUE },
	{ "client_BombusQD", _T("|*Bombusmod-qd*|*bombus*qd*"), _T("Bombus QD"), IDI_BOMBUS_QD, JABBER_CASE, TRUE },
	{ "client_Bowline", _T("*Bow*line*"), _T("Bowline"), IDI_BOWLINE, JABBER_CASE, TRUE },
	{ "client_BuddySpace", _T("Buddy*Space*"), _T("BuddySpace"), IDI_BUDDYSPACE, JABBER_CASE, TRUE },
	{ "client_CJC", _T("*CJC*"), _T("CJC"), IDI_CJC, JABBER_CASE, TRUE },
	{ "client_CRoom", _T("*CRoom*"), _T("CRoom"), IDI_CROOM, JABBER_CASE, TRUE },
	{ "client_Candy", _T("*Candy*"), _T("Candy"), IDI_CANDY, JABBER_CASE, TRUE },
	{ "client_Chatopus", _T("*Chatopus*"), _T("Chatopus"), IDI_CHATOPUS, JABBER_CASE, TRUE },
	{ "client_Chikka", _T("*Chikka*"), _T("Chikka"), IDI_CHIKKA, JABBER_CASE, TRUE },
	{ "client_ChitChat", _T("*Chit*Chat*"), _T("ChitChat"), IDI_CHITCHAT, JABBER_CASE, TRUE },
	{ "client_Claros_Chat", _T("*Claros*"), _T("Claros Chat"), IDI_CLAROS_CHAT, JABBER_CASE, TRUE },
	{ "client_Coccinella", _T("*Coccinella*"), _T("Coccinella"), IDI_COCCINELLA, JABBER_CASE, TRUE },
	{ "client_Colibry", _T("Colibry*"), _T("Colibry"), IDI_COLIBRY, JABBER_CASE, TRUE },
	{ "client_Colloquy", _T("Colloquy*"), _T("Colloquy"), IDI_COLLOQUY, JABBER_CASE, TRUE },
	{ "client_CommuniGate", _T("*CommuniGate*"), _T("CommuniGate Pro"), IDI_COMMUNIGATE, JABBER_CASE, TRUE },
	{ "client_Conference", _T("Conference*"), _T("Conference Bot (GMail)"), IDI_CONFERENCE, JABBER_CASE, TRUE },
	{ "client_Conversations", _T("*conversions*"), _T("Conversations"), IDI_CONVERSATIONS, JABBER_CASE, TRUE },
	{ "client_Crosstalk", _T("*Cross*talk*"), _T("Crosstalk"), IDI_CROSSTALK, JABBER_CASE, TRUE },
	{ "client_Cudumar", _T("*Cudumar*"), _T("Cudumar"), IDI_CUDUMAR, JABBER_CASE, TRUE },
	{ "client_CyclopsChat", _T("*Cyclops*"), _T("Cyclops Chat"), IDI_CYCLOPS_CHAT, JABBER_CASE, TRUE },
	{ "client_Desyr", _T("*Desyr*"), _T("Desyr Messenger"), IDI_DESYR, JABBER_CASE, TRUE },
	{ "client_EMess", _T("*EMess*"), _T("EMess"), IDI_EMESS, JABBER_CASE, TRUE },
	{ "client_Elmer_Bot", _T("*Elmer*"), _T("Elmer Bot"), IDI_ELMER, JABBER_CASE, TRUE },
	{ "client_Emacs", _T("|*Jabber.el*|*Emacs*"), _T("Emacs (Jabber.el)"), IDI_EMACS, JABBER_CASE, TRUE },
	{ "client_Exodus", _T("*Exodus*"), _T("Exodus"), IDI_EXODUS, JABBER_CASE, TRUE },

	{ "client_GCN", _T("*GCN*"), _T("GCN"), IDI_GCN, JABBER_CASE, TRUE },
	{ "client_GMail", _T("|*gmail.*|GMail*"), _T("GMail"), IDI_GMAIL, JABBER_CASE, TRUE },
	{ "client_GOIM", _T("*GOIM*"), _T("GOIM"), IDI_GOIM, JABBER_CASE, TRUE },
	{ "client_GTalk", _T("|*Talk.v*|*Google*Talk*")
	_T("|*Gtalk*|*-iPhon*"), _T("GoogleTalk aka GTalk"), IDI_GTALK, JABBER_CASE, TRUE },
	{ "client_GTalk_Gadget", _T("|^messaging-*|*Talk*Gadget*"), _T("GTalk Gadget"), IDI_GTALK_GADGET, JABBER_CASE, TRUE },
	{ "client_Gabber", _T("*Gabber*"), _T("Gabber"), IDI_GABBER, JABBER_CASE, TRUE },
	{ "client_Gajim", _T("*Gajim*"), _T("Gajim"), IDI_GAJIM, JABBER_CASE, TRUE },
	{ "client_Gibberbot", _T("*Gibber*"), _T("Gibberbot"), IDI_GIBBERBOT, JABBER_CASE, TRUE },
	{ "client_Glu", _T("|glu*|*glu.net*"), _T("Glu"), IDI_GLU, JABBER_CASE, TRUE },
	{ "client_Gnome", _T("*Gnome*"), _T("Gnome"), IDI_GNOME, JABBER_CASE, TRUE },
	{ "client_GoTalkMobile", _T("*Go*Talk*Mobile*"), _T("GoTalkMobile"), IDI_GOTALKMOBILE, JABBER_CASE, TRUE },
	{ "client_Gossip", _T("*Gossip*"), _T("Gossip"), IDI_GOSSIP, JABBER_CASE, TRUE },
	{ "client_GreenThumb", _T("gReeNtHumB*"), _T("GreenThumb"), IDI_GREENTHUMB, JABBER_CASE, TRUE },
	{ "client_Gush", _T("*Gush*"), _T("Gush"), IDI_GUSH, JABBER_CASE, TRUE },
	{ "client_IMCom", _T("*IMCom*"), _T("IMCom"), IDI_IMCOM, JABBER_CASE, TRUE },
	{ "client_IM_Friendly", _T("*IM*Friendly*"), _T("IM Friendly!"), IDI_IM_FRIENDLY, JABBER_CASE, TRUE },
	{ "client_Imified", _T("*someresource*"), _T("Imified"), IDI_IMIFIED, JABBER_CASE, TRUE },
	{ "client_Importal", _T("*Importal*"), _T("Importal"), IDI_IMPORTAL, JABBER_CASE, TRUE },
	{ "client_InstanT", _T("*Instan-t*"), _T("Instan-t"), IDI_INSTANT, JABBER_CASE, TRUE },
	{ "client_Interaction", _T("*Interaction*"), _T("Interaction"), IDI_INTERACTION, JABBER_CASE, TRUE },
	{ "client_iruka", _T("*Iruka*"), _T("Iruka"), IDI_IRUKA, JABBER_CASE, TRUE },
	{ "client_J2J_Transport", _T("*J2J*Transport*"), _T("J2J Transport"), IDI_J2J_TRANSPORT, JABBER_CASE, TRUE },
	{ "client_Jamm", _T("*Jamm*"), _T("Jamm"), IDI_JAMM, JABBER_CASE, TRUE },
	{ "client_JClaim", _T("*JClaim*"), _T("JClaim"), IDI_JCLAIM, JABBER_CASE, TRUE },
	{ "client_JMC", _T("JMC*"), _T("JMC (Jabber Mix Client)"), IDI_JMC, JABBER_CASE, TRUE },
	{ "client_JWChat", _T("*JWChat*"), _T("JWChat"), IDI_JWCHAT, JABBER_CASE, TRUE },
	{ "client_JWGC", _T("|*JWGC*|Jabber *Gram*"), _T("Jabber WindowGram Client"), IDI_JWGC, JABBER_CASE, TRUE },
	{ "client_Jabba", _T("*Jabba*"), _T("Jabba"), IDI_JABBA, JABBER_CASE, TRUE },

	{ "client_JabberApplet", _T("Jabber*Applet*"), _T("JabberApplet"), IDI_JABBER_APPLET, JABBER_CASE, TRUE },
	{ "client_JabberBeOS", _T("Jabber*BeOS*"), _T("Jabber (BeOS)"), IDI_JABBER_BEOS, JABBER_CASE, TRUE },
	{ "client_JabberFoX", _T("*fox*"), _T("JabberFoX"), IDI_JABBERFOX, JABBER_CASE, TRUE },
	{ "client_JabberMSNGR", _T("Jabber Messenger*"), _T("Jabber Messenger"), IDI_JABBER_MESSENGER, JABBER_CASE, TRUE },
	{ "client_JabberNaut", _T("*Jabber*Naut*"), _T("JabberNaut"), IDI_JABBERNAUT, JABBER_CASE, TRUE },
	{ "client_JabberZilla", _T("*Zilla*"), _T("JabberZilla"), IDI_JABBERZILLA, JABBER_CASE, TRUE },
	{ "client_Jabber_Net", _T("|*Jabber*Net*|*cursive.net*|*csharp*"), _T("Jabber-Net"), IDI_JABBER_NET, JABBER_CASE, TRUE },
	{ "client_Jabberwocky", _T("Jabberwocky*"), _T("Jabberwocky (Amiga)"), IDI_JABBERWOCKY, JABBER_CASE, TRUE },
	{ "client_Jabbroid", _T("*Jabbroid*"), _T("Jabbroid"), IDI_JABBROID, JABBER_CASE, TRUE },
	{ "client_Jajc", _T("|*Jajc*|Just Another Jabber Client"), _T("JAJC"), IDI_JAJC, JABBER_CASE, TRUE },
	{ "client_Jeti", _T("*Jeti*"), _T("Jeti"), IDI_JETI, JABBER_CASE, TRUE },
	{ "client_Jitsi", _T("*Jitsi*"), _T("Jitsi"), IDI_JITSI, JABBER_CASE, TRUE },
	{ "client_Joost", _T("*Joost*"), _T("Joost"), IDI_JOOST, JABBER_CASE, TRUE },
	{ "client_Kadu", _T("*Kadu*"), _T("Kadu"), IDI_KADU, JABBER_CASE, TRUE },
	{ "client_Konnekt", _T("Konnekt*"), _T("Konnekt"), IDI_KONNEKT, JABBER_CASE, TRUE },
	{ "client_LLuna", _T("LLuna*"), _T("LLuna"), IDI_LLUNA, JABBER_CASE, TRUE },
	{ "client_Lamp", _T("*Lamp*IM*"), _T("Lamp IM"), IDI_LAMP_IM, JABBER_CASE, TRUE },
	{ "client_Lampiro", _T("*Lampiro*"), _T("Lampiro"), IDI_LAMPIRO, JABBER_CASE, TRUE },
	{ "client_Landell", _T("*Landell*"), _T("Landell"), IDI_LANDELL, JABBER_CASE, TRUE },
	{ "client_Leaf", _T("*Leaf*"), _T("Leaf Messenger"), IDI_LEAF, JABBER_CASE, TRUE },
	{ "client_LinQ", _T("*LinQ*"), _T("LinQ"), IDI_LINQ, JABBER_CASE, TRUE },
	{ "client_M8Jabber", _T("*M8Jabber*"), _T("M8Jabber"), IDI_M8JABBER, JABBER_CASE, TRUE },
	{ "client_MCabber", _T("*mcabber*"), _T("MCabber"), IDI_MCABBER, JABBER_CASE, TRUE },
	{ "client_MGTalk", _T("|*MGTalk*|*Mobile?\?\?\?\?\?\?\?"), _T("MGTalk"), IDI_MGTALK, JABBER_CASE, TRUE },
	{ "client_MUCkl", _T("*MUCkl*"), _T("MUCkl"), IDI_MUCKL, JABBER_CASE, TRUE },
	{ "client_Mango", _T("*Mango*"), _T("Mango"), IDI_MANGO, JABBER_CASE, TRUE },
	{ "client_Mercury", _T("*Mercury*"), _T("Mercury Messenger"), IDI_MERCURY_MESSENGER, JABBER_CASE, TRUE },
	{ "client_Monal", _T("*Monal*"), _T("Monal"), IDI_MONAL, JABBER_CASE, TRUE },
	{ "client_MozillaChat", _T("*Mozilla*Chat*"), _T("MozillaChat"), IDI_MOZILLACHAT, JABBER_CASE, TRUE },
	{ "client_Neos", _T("Neos*"), _T("Neos"), IDI_NEOS, JABBER_CASE, TRUE },
	{ "client_Nitro", _T("Nitro*"), _T("Nitro"), IDI_NITRO, JABBER_CASE, TRUE },
	{ "client_Nostromo", _T("*USCSS*Nostromo*"), _T("USCSS Nostromo"), IDI_NOSTROMO, JABBER_CASE, TRUE },
	{ "client_OM", _T("OM*"), _T("OM aka Online Messenger"), IDI_OM, JABBER_CASE, TRUE },
	{ "client_OctroTalk", _T("*Octro*"), _T("OctroTalk"), IDI_OCTROTALK, JABBER_CASE, TRUE },
	{ "client_OneTeam", _T("*OneTeam*"), _T("OneTeam"), IDI_ONETEAM, JABBER_CASE, TRUE },
	{ "client_Openfire", _T("*Openfire*"), _T("Openfire"), IDI_OPENFIRE, JABBER_CASE, TRUE },
	{ "client_Fire", _T("Fire*"), _T("Fire"), IDI_FIRE, JABBER_CASE, TRUE },
	{ "client_Paltalk", _T("*Paltalk*"), _T("Paltalk"), IDI_PALTALK, JABBER_CASE, TRUE },
	{ "client_Pandion", _T("|*Pandion*|*Пандион*"), _T("Pandion"), IDI_PANDION, JABBER_CASE, TRUE },
	{ "client_Papla", _T("*Papla*"), _T("Papla"), IDI_PAPLA, JABBER_CASE, TRUE },
	{ "client_Poezio", _T("*Poezio*"), _T("Poezio"), IDI_POEZIO, JABBER_CASE, TRUE },
	{ "client_Prosody", _T("*Prosody*"), _T("Prosody"), IDI_PROSODY, JABBER_CASE, TRUE },

	{ "client_Psi_plus", _T("|*PSI+*|*psi-dev.googlecode*"), _T("PSI+"), IDI_PSIPLUS, JABBER_CASE, TRUE },
	{ "client_Psi", _T("*Psi*"), _T("PSI"), IDI_PSI, JABBER_CASE, TRUE },

	{ "client_Psto", _T("*Psto*"), _T("Psto.net"), IDI_PSTO, JABBER_CASE, TRUE },
	{ "client_Psyc", _T("*Psyc*"), _T("Psyc"), IDI_PSYC, JABBER_CASE, TRUE },
	{ "client_Pygeon", _T("*Pygeon*"), _T("Pygeon"), IDI_PYGEON, JABBER_CASE, TRUE },

	{ "client_QTJim", _T("*QTJim*"), _T("QTJim"), IDI_QTJIM, JABBER_CASE, TRUE },
	{ "client_QuteCom", _T("*Qute*Com*"), _T("QuteCom"), IDI_QUTECOM, JABBER_CASE, TRUE },
	{ "client_RenRen", _T("|*WTalkProxy0_0*|*talk.xiaonei.com*"), _T("RenRen"), IDI_RENREN, JABBER_CASE, TRUE },
	{ "client_SBot", _T("*SBot*"), _T("SBot"), IDI_SBOT, JABBER_CASE, TRUE },
	{ "client_SMTP_Transport", _T("*smtp*transport*"), _T("SMTP Transport"), IDI_SMTP_TRANSPORT, JABBER_CASE, TRUE },
	{ "client_SamePlace", _T("*SamePlace*"), _T("SamePlace"), IDI_SAMEPLACE, JABBER_CASE, TRUE },
	{ "client_Sky_Messager", _T("Sky*Mess*"), _T("Sky Messager"), IDI_SKYMESSAGER, JABBER_CASE, TRUE },
	{ "client_Sky_Messager", _T("*Sky*Messager*"), _T("Sky Messager"), IDI_SKYMESSAGER, JABBER_CASE, TRUE },

	{ "client_xabber", _T("|*xabber*")
	_T("|*bWG06mEjKFM5ygtd84Ov95P8VH0=*"), _T("xabber"), IDI_XABBER, JABBER_CASE, TRUE },
	{ "client_Gabble", _T("*Gabble*"), _T("Gabble"), IDI_GABBLE, JABBER_CASE, TRUE },
	{ "client_Smack", _T("|*igniterealtime.*smack*|*smack*"), _T("Smack"), IDI_SMACK, JABBER_CASE, TRUE },

	{ "client_SoapBox", _T("*SoapBox*"), _T("SoapBox"), IDI_SOAPBOX, JABBER_CASE, TRUE },
	{ "client_Spark", _T("*Spark*"), _T("Spark"), IDI_SPARK, JABBER_CASE, TRUE },
	{ "client_Speakall", _T("*Speak*all*"), _T("Speakall"), IDI_SPEAKALL, JABBER_CASE, TRUE },
	{ "client_Speeqe", _T("*Speeqe*"), _T("Speeqe"), IDI_SPEEQE, JABBER_CASE, TRUE },
	{ "client_Spik", _T("*Spik*"), _T("Spik"), IDI_SPIK, JABBER_CASE, TRUE },
	{ "client_Swift", _T("*Swift*"), _T("Swift"), IDI_SWIFT, JABBER_CASE, TRUE },
	{ "client_SworIM", _T("*Swor*IM*"), _T("SworIM"), IDI_SWORIM, JABBER_CASE, TRUE },
	{ "client_Synapse", _T("*Synapse*"), _T("Synapse"), IDI_SYNAPSE, JABBER_CASE, TRUE },
	{ "client_Talkdroid", _T("*Talkdroid*"), _T("Talkdroid"), IDI_TALKDROID, JABBER_CASE, TRUE },
	{ "client_Talkonaut", _T("*Talkonaut*"), _T("Talkonaut"), IDI_TALKONAUT, JABBER_CASE, TRUE },
	{ "client_Tapioca", _T("*Tapioca*"), _T("Tapioca"), IDI_TAPIOCA, JABBER_CASE, TRUE },
	{ "client_Teabot", _T("|*teabot*|*teabot.org/bot*"), _T("Teabot"), IDI_TEABOT, JABBER_CASE, TRUE },
	{ "client_Telepathy", _T("*Telepathy*"), _T("Telepathy"), IDI_TELEPATHY, JABBER_CASE, TRUE },
	{ "client_The_Bee", _T("*The*Bee*"), _T("The Bee"), IDI_THEBEE, JABBER_CASE, TRUE },
	{ "client_Thunderbird", _T("*Thunderbi*"), _T("Thunderbird"), IDI_THUNDERBIRD, JABBER_CASE, TRUE },
	{ "client_Tigase", _T("*Tigase*"), _T("Tigase"), IDI_TIGASE, JABBER_CASE, TRUE },
	{ "client_TipicIM", _T("Tipic*"), _T("TipicIM"), IDI_TIPICIM, JABBER_CASE, TRUE },
	{ "client_Tkabber", _T("*Tkabber*"), _T("Tkabber"), IDI_TKABBER, JABBER_CASE, TRUE },
	{ "client_TransactIM", _T("*Transact*"), _T("TransactIM"), IDI_TRANSACTIM, JABBER_CASE, TRUE },
	{ "client_Translate", _T("*Translate*"), _T("Translate component"), IDI_TRANSLATE, JABBER_CASE, TRUE },
	{ "client_Triple", _T("Triple*"), _T("TripleSoftwareIM (TSIM)"), IDI_TRIPLE_SOFTWARE, JABBER_CASE, TRUE },
	{ "client_Vacuum", _T("*Vacuum*"), _T("Vacuum IM"), IDI_VACUUM, JABBER_CASE, TRUE },
	{ "client_V&V", _T("*V&V*"), _T("V&V Messenger"), IDI_VANDV, JABBER_CASE, TRUE },
	{ "client_Vayusphere", _T("*Vayusphere*"), _T("Vayusphere"), IDI_VAYUSPHERE, JABBER_CASE, TRUE },
	{ "client_Vysper", _T("*Vysper*"), _T("Vysper"), IDI_VYSPER, JABBER_CASE, TRUE },
	{ "client_WTW", _T("**WTW**|*wtw.k2t.eu*"), _T("WTW"), IDI_WTW, JABBER_CASE, TRUE },
	{ "client_WannaChat", _T("Wanna*Chat*"), _T("WannaChat"), IDI_WANNACHAT, JABBER_CASE, TRUE },
	{ "client_WebEx", _T("*webex.com*"), _T("Cisco WebEx Connect"), IDI_WEBEX, JABBER_CASE, TRUE },
	{ "client_WhisperIM", _T("*Whisper*"), _T("WhisperIM"), IDI_WHISPERIM, JABBER_CASE, TRUE },
	{ "client_Wija", _T("*wija*"), _T("Wija"), IDI_WIJA, JABBER_CASE, TRUE },
	{ "client_Wildfire", _T("Wildfire*"), _T("Wildfire"), IDI_WILDFIRE, JABBER_CASE, TRUE },
	{ "client_WinJab", _T("*WinJab*"), _T("WinJab"), IDI_WINJAB, JABBER_CASE, TRUE },
	{ "client_Xiffian", _T("*Xiffian*"), _T("Xiffian"), IDI_XIFFIAN, JABBER_CASE, TRUE },
	{ "client_Yambi", _T("*Yambi*"), _T("Yambi"), IDI_YAMBI, JABBER_CASE, TRUE },
	{ "client_chat_bots", _T("*chat*bot*"), _T("chat bot"), IDI_CHAT_BOT, JABBER_CASE, TRUE },
	{ "client_dziObber", _T("*dzi?bber*"), _T("dziObber"), IDI_DZIOBBER, JABBER_CASE, TRUE },
	{ "client_ejabberd", _T("*ejabberd*"), _T("ejabberd"), IDI_EJABBERD, JABBER_CASE, TRUE },
	{ "client_emite", _T("*emite*"), _T("emite"), IDI_EMITE, JABBER_CASE, TRUE },
	{ "client_gYaber", _T("gYaber*"), _T("gYaber"), IDI_GYABER, JABBER_CASE, TRUE },
	{ "client_glu", _T("*glu*"), _T("glu"), IDI_GLU, JABBER_CASE, TRUE },
	{ "client_iGoogle", _T("iGoogle*"), _T("iGoogle"), IDI_IGOOGLE, JABBER_CASE, TRUE },
	{ "client_iJab", _T("*iJab*"), _T("iJab"), IDI_IJAB, JABBER_CASE, TRUE },
	{ "client_iMeem", _T("iMeem*"), _T("iMeem"), IDI_IMEEM, JABBER_CASE, TRUE },
	{ "client_iMov", _T("*imov*"), _T("iMov"), IDI_IMOV, JABBER_CASE, TRUE },
	{ "client_jTalk", _T("|*jTalk*|http://jtalk*"), _T("jTalk"), IDI_JTALK, JABBER_CASE, TRUE },
	{ "client_jTalkmod", _T("|*jTalkmod*")
	_T("|*glSvJ3yM3M2f53oregNy6fYwocY=*")
	_T("|*XEssZlSs8oF4EcTHU1b8BsVxcPg=*"), _T("jTalkmod"), IDI_JTALKMOD, JABBER_CASE, TRUE },
	{ "client_jabberDisk", _T("|*jdisk*|*jabber*Disk*"), _T("jabberDisk"), IDI_JABBER_DISK, JABBER_CASE, TRUE },
	{ "client_jabbim", _T("*jabbim*"), _T("Jabbim"), IDI_JABBIM, JABBER_CASE, TRUE },
	{ "client_jabiru", _T("*jabiru*"), _T("Jabiru"), IDI_JABIRU, JABBER_CASE, TRUE },
	{ "client_jappix", _T("*jappix*"), _T("jappix"), IDI_JAPPIX, JABBER_CASE, TRUE },
	{ "client_jrudevels", _T("*jrudevels*"), _T("Jrudevels"), IDI_JRUDEVELS, JABBER_CASE, TRUE },
	{ "client_juick", _T("*juick*"), _T("Juick"), IDI_JUICK, JABBER_CASE, TRUE },
	{ "client_kf", _T("|^*smack*|*kf*"), _T("kf jabber"), IDI_KF, JABBER_CASE, TRUE },
	{ "client_laffer", _T("*laffer*"), _T("Laffer"), IDI_LAFFER, JABBER_CASE, TRUE },
	{ "client_mJabber", _T("*mJabber*"), _T("mJabber"), IDI_MJABBER, JABBER_CASE, TRUE },
	{ "client_meinvz", _T("*meinvz*"), _T("MeinVZ"), IDI_MEINVZ, JABBER_CASE, TRUE },
	{ "client_moJab", _T("*moJab*"), _T("moJab"), IDI_MOJAB, JABBER_CASE, TRUE },
	{ "client_mobber", _T("*mobber*"), _T("mobber"), IDI_MOBBER, JABBER_CASE, TRUE },
	{ "client_myJabber", _T("*myJabber*"), _T("myJabber"), IDI_MYJABBER, JABBER_CASE, TRUE },
	{ "client_orkut", _T("*orkut*"), _T("orkut"), IDI_ORKUT, JABBER_CASE, TRUE },
	{ "client_pjc", _T("|*PJC*|*pjc.googlecode.com*"), LPGENT("PHP Jabber Client"), IDI_PJC, JABBER_CASE, TRUE },
	{ "client_saje", _T("*saje*"), _T("saje"), IDI_SAJE, JABBER_CASE, TRUE },
	{ "client_schuelervz", _T("*schuelervz*"), _T("SchulerVZ"), IDI_SCHULERVZ, JABBER_CASE, TRUE },
	{ "client_studivz", _T("*studivz*"), _T("StudiVZ"), IDI_STUDIVZ, JABBER_CASE, TRUE },
	{ "client_tkchat", _T("*tkchat*"), _T("tkchat"), IDI_TKCHAT, JABBER_CASE, TRUE },
	//  {"client_uJabber",          _T("*uJabber*"),                                _T("uJabber"),                      IDI_UJABBER,             JABBER_CASE,        TRUE    },
	{ "client_uKeeper", _T("*uKeeper*"), _T("uKeeper"), IDI_UKEEPER, JABBER_CASE, TRUE },
	{ "client_whoisbot", _T("whoisbot"), _T("Swissjabber Whois Bot"), IDI_WHOISBOT, JABBER_CASE, TRUE },
	{ "client_xeus2", _T("*xeus 2*"), _T("xeus 2"), IDI_XEUS2, JABBER_CASE, TRUE },
	{ "client_xeus", _T("*xeus*"), _T("xeus"), IDI_XEUS, JABBER_CASE, TRUE },
	{ "client_yaonline", _T("|*yandex*|*yaonline*")
	_T("|*Я.Онлайн*|*Яндекс*"), _T("Ya.Online"), IDI_YAONLINE, JABBER_CASE, TRUE },
	{ "client_yaxim", _T("*yaxim*"), _T("yaxim"), IDI_YAXIM, JABBER_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   IRC   ###############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_AmIRC", _T("*AmIRC*"), _T("AmIRC"), IDI_AMIRC, IRC_CASE, TRUE },
	{ "client_Babbel", _T("*Babbel*"), _T("Babbel"), IDI_BABBEL, IRC_CASE, TRUE },
	{ "client_BersIRC", _T("*BersIRC*"), _T("BersIRC"), IDI_BERSIRC, IRC_CASE, TRUE },
	{ "client_ChatZilla", _T("*ChatZilla*"), _T("ChatZilla"), IDI_CHATZILLA, IRC_CASE, TRUE },
	{ "client_Conversation", _T("*Conversation*"), _T("Conversation"), IDI_CONVERSATION, IRC_CASE, TRUE },
	{ "client_Eggdrop", _T("*Eggdrop*"), _T("Eggdrop"), IDI_EGGDROP, IRC_CASE, TRUE },
	{ "client_EggdropRacBot", _T("*Eggdrop*RacBot*"), _T("Eggdrop RacBot"), IDI_EGGDROP_RACBOT, IRC_CASE, TRUE },
	{ "client_FChat", _T("*FChat*"), _T("FChat"), IDI_FCHAT, IRC_CASE, TRUE },
	{ "client_GDPChat", _T("*GDPChat*"), _T("GDP Web Chat"), IDI_GDPCHAT, IRC_CASE, TRUE },
	{ "client_GoPowerTools", _T("*Go*PowerTools*"), _T("GoPowerTools"), IDI_GOPOWERTOOLS, IRC_CASE, TRUE },
	{ "client_HydraIRC", _T("*Hydra*IRC*"), _T("HydraIRC"), IDI_HYDRA_IRC, IRC_CASE, TRUE },
	{ "client_IRCXpro", _T("*IRCXpro*"), _T("IRCXpro"), IDI_IRCXPRO, IRC_CASE, TRUE },
	{ "client_IceChat", _T("*Ice*Chat*"), _T("IceChat"), IDI_ICECHAT, IRC_CASE, TRUE },
	{ "client_KSirc", _T("*ksirk*"), _T("KSirc"), IDI_KSIRC, IRC_CASE, TRUE },
	{ "client_KVIrc", _T("*KVIrc*"), _T("KVIrc"), IDI_KVIRC, IRC_CASE, TRUE },
	{ "client_Klient", _T("*Klient*"), _T("Klient"), IDI_KLIENT, IRC_CASE, TRUE },
	{ "client_Konversation", _T("*Konversation*"), _T("Konversation"), IDI_KONVERSATION, IRC_CASE, TRUE },
	{ "client_MP3Script", _T("*MP3*Script*"), LPGENT("MP3 Script for mIRC"), IDI_MP3_SCRIPT, IRC_CASE, TRUE },
	{ "client_NeoRaTrion", _T("*NeoRa*Trion*"), _T("NeoRa Trion"), IDI_NEORATRION, IRC_CASE, TRUE },
	{ "client_Nettalk", _T("*Nettalk*"), _T("Nettalk"), IDI_NETTALK, IRC_CASE, TRUE },
	{ "client_NoNameScript", _T("*NoName*Script*"), _T("NoNameScript"), IDI_NONAME_SCRIPT, IRC_CASE, TRUE },
	{ "client_Opera", _T("*Opera*"), _T("Opera"), IDI_OPERA, IRC_CASE, TRUE },
	{ "client_PJIRC", _T("*PJIRC*"), _T("PJIRC"), IDI_PJIRC, IRC_CASE, TRUE },
	{ "client_Pirch", _T("*Pirch*"), _T("Pirch"), IDI_PIRCH, IRC_CASE, TRUE },
	{ "client_PocketIRC", _T("*Pocket*IRC*"), _T("Pocket IRC"), IDI_POCKET_IRC, IRC_CASE, TRUE },
	{ "client_ProChat", _T("*Pro*Chat*"), _T("ProChat"), IDI_PROCHAT, IRC_CASE, TRUE },
	{ "client_SmartIRC", _T("*Smart*IRC*"), _T("SmartIRC"), IDI_SMART_IRC, IRC_CASE, TRUE },
	{ "client_Snak", _T("*Snak*"), _T("Snak"), IDI_SNAK, IRC_CASE, TRUE },
	{ "client_SysReset", _T("*Sys*Reset*"), _T("SysReset"), IDI_SYSRESET, IRC_CASE, TRUE },
	{ "client_VircaIRC", _T("*VircaIRC*"), _T("VircaIRC"), IDI_VIRCAIRC, IRC_CASE, TRUE },
	{ "client_VisionIRC", _T("*VisionIRC*"), _T("VisionIRC"), IDI_VISIONIRC, IRC_CASE, TRUE },
	{ "client_VisualIRC", _T("*VisualIRC*"), _T("VisualIRC"), IDI_VISUALIRC, IRC_CASE, TRUE },
	{ "client_VortecIRC", _T("*VortecIRC*"), _T("VortecIRC"), IDI_VORTECIRC, IRC_CASE, TRUE },
	{ "client_WLIrc", _T("*WLIrc*"), _T("WLIrc"), IDI_WLIRC, IRC_CASE, TRUE },
	{ "client_XChatAqua", _T("*X*Chat*Aqua*"), _T("X-Chat Aqua"), IDI_XCHATAQUA, IRC_CASE, TRUE },
	{ "client_XiRCON", _T("*XiRCON*"), _T("XiRCON"), IDI_XIRCON, IRC_CASE, TRUE },
	{ "client_Xirc", _T("*Xirc*"), _T("Xirc"), IDI_XIRC, IRC_CASE, TRUE },
	{ "client_cbirc", _T("*cbirc*"), _T("cbirc"), IDI_CBIRC, IRC_CASE, TRUE },
	{ "client_dIRC", _T("*dIRC*"), _T("dIRC"), IDI_DIRC, IRC_CASE, TRUE },

	{ "client_iroffer_dinoex", _T("*iroffer*dinoex*"), _T("iroffer dinoex"), IDI_IROFFER_DINOEX, IRC_CASE, TRUE },
	{ "client_iroffer", _T("*iroffer*"), _T("iroffer"), IDI_IROFFER, IRC_CASE, TRUE },

	{ "client_ircle", _T("*ircle*"), _T("ircle"), IDI_IRCLE, IRC_CASE, TRUE },
	{ "client_jircii", _T("*jircii*"), _T("jircii"), IDI_JIRCII, IRC_CASE, TRUE },
	{ "client_jmIrc", _T("*jmIrc*"), _T("jmIrc"), IDI_JMIRC, IRC_CASE, TRUE },
	{ "client_mIRC", _T("*mIRC*"), _T("mIRC"), IDI_MIRC, IRC_CASE, TRUE },
	{ "client_pIRC", _T("*pIRC*"), _T("pIRC"), IDI_PIRC, IRC_CASE, TRUE },
	{ "client_piorun", _T("*piorun*"), _T("Piorun"), IDI_PIORUN, IRC_CASE, TRUE },
	{ "client_psyBNC", _T("*psyBNC*"), _T("psyBNC"), IDI_PSYBNC, IRC_CASE, TRUE },
	{ "client_savIRC", _T("*savIRC*"), _T("savIRC"), IDI_SAVIRC, IRC_CASE, TRUE },
	{ "client_wmIRC", _T("*wmIRC*"), _T("wmIRC"), IDI_WMIRC, IRC_CASE, TRUE },
	{ "client_xBitch", _T("*xBitch*"), _T("xBitch"), IDI_XBITCH, IRC_CASE, TRUE },
	{ "client_xChat", _T("*xChat*"), _T("xChat"), IDI_XCHAT, IRC_CASE, TRUE },
	{ "client_zsIRC", _T("*zsIRC*"), _T("zsIRC"), IDI_ZSIRC, IRC_CASE, TRUE },
	{ "client_ZNC", _T("*ZNC*"), _T("ZNC"), IDI_ZNC, IRC_CASE, TRUE },

	{ "client_aMule", _T("*aMule*"), _T("aMule"), IDI_AMULE, IRC_CASE, TRUE },
	{ "client_eMuleMorphXT", _T("eMule*MorphXT*"), _T("eMule MorphXT"), IDI_EMULE_MORPHXT, IRC_CASE, TRUE },
	{ "client_eMuleNeo", _T("eMule*Neo*"), _T("eMule Neo"), IDI_EMULE_NEO, IRC_CASE, TRUE },
	{ "client_eMulePlus", _T("|eMule*plus*|eMule*+*"), _T("eMule+"), IDI_EMULE_PLUS, IRC_CASE, TRUE },
	{ "client_eMuleXtreme", _T("eMule*Xtreme*"), _T("eMule Xtreme"), IDI_EMULE_XTREME, IRC_CASE, TRUE },
	{ "client_eMule", _T("*eMule*"), _T("eMule"), IDI_EMULE, IRC_CASE, TRUE },

	{ "client_IRCUnknown", _T("*IRC*"), _T("Unknown IRC client"), IDI_IRC, IRC_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   AIM   ###############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_Aim7", _T("AIM*7.*"), _T("AIM v7.x"), IDI_AIM_7, AIM_CASE, TRUE },
	{ "client_Aim6", _T("AIM*6.*"), _T("AIM v6.x"), IDI_AIM_6, AIM_CASE, TRUE },

	{ "client_AIM_Triton", _T("*AIM*Triton*"), _T("AIM Triton"), IDI_AIM_TRITON, AIM_CASE, TRUE },
	{ "client_AimMac", _T("AIM*Mac*"), _T("AIM for MacOS"), IDI_AIM_MAC, AIM_CASE, TRUE },
	{ "client_AimMobile", _T("|AIM*Mobile*|AIM*gprs*|Aim*sms*"), _T("AIM Mobile"), IDI_AIM_MOBILE, AIM_CASE, TRUE },
	{ "client_Naim", _T("*naim*"), _T("Naim"), IDI_NAIM, AIM_CASE, TRUE },
	{ "client_miniaim", _T("*miniaim*"), _T("miniaim"), IDI_MINIAIM, AIM_CASE, TRUE },
	{ "client_TerraIM", _T("*Terra*"), _T("TerraIM"), IDI_TERRAIM, AIM_CASE, TRUE },
	{ "client_AIM", _T("AIM*"), _T("AIM"), IDI_AIM_7, AIM_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   MSN   ###############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_WLM11", _T("WLM*2011*"), _T("Windows Live 11"), IDI_WLM_10, MSN_CASE, TRUE },
	{ "client_WLM10", _T("WLM*2010*"), _T("Windows Live 10"), IDI_WLM_10, MSN_CASE, TRUE },
	{ "client_WLM9", _T("WLM*9*"), _T("Windows Live v9.x"), IDI_WLM_9, MSN_CASE, TRUE },
	{ "client_MSN8", _T("|WLM*8.*|MSN*8.*"), _T("Windows Live v8.x"), IDI_MSN_8, MSN_CASE, TRUE },
	{ "client_MSN7x", _T("MSN*7.*"), _T("MSN Messenger v7.x"), IDI_MSN_7, MSN_CASE, TRUE },
	{ "client_MSN6x", _T("MSN*6.*"), _T("MSN Messenger v6.x"), IDI_MSN_6, MSN_CASE, TRUE },
	{ "client_MSN45", _T("MSN*4.x-5.x"), _T("MSN Messenger v4.x-5.x"), IDI_MSN_45, MSN_CASE, TRUE },
	{ "client_MSN4Mac", _T("MSN*Mac*"), _T("MSN for Mac"), IDI_MSN_MAC, MSN_CASE, TRUE },
	{ "client_aMSN", _T("*aMSN*"), _T("aMSN"), IDI_AMSN, MSN_CASE, TRUE },
	{ "client_MSN_Web", _T("WebMessenger*"), _T("WebMessenger"), IDI_WEBMESSENGER, MSN_CASE, TRUE },
	{ "client_MSN", _T("|WLM*|MSN*|Windows Live*"), _T("MSN"), IDI_MSN, MSN_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   MAIL.RU   ###########################################################################################################################
	//#########################################################################################################################################################################
	{ "client_MRA_official", _T("|magent*|Mail.ru Agent*official*")
	_T("|Mail.ru Agent*PC*")
	_T("|*agent.mail.ru*"), LPGENT("Mail.Ru Agent (official)"), IDI_MAIL_RU_OFFICIAL, MRA_CASE, TRUE },
	{ "client_MRA_Mobile", _T("|MobileAgent*|Mail.ru Agent*Symbian*"), _T("Mail.Ru Mobile Agent"), IDI_MAIL_RU_MOBILE, MRA_CASE, TRUE },
	{ "client_MRA_web", _T("|webagent*|*Web Agent*"), _T("Mail.Ru Web Agent"), IDI_MAIL_RU_WEBAGENT, MRA_CASE, TRUE },
	{ "client_MRA_unknown", _T("|Mail.ru Agent*|MRA client"), LPGENT("Mail.Ru (unknown client)"), IDI_MAIL_RU_UNKNOWN, MRA_CASE },

	//#########################################################################################################################################################################
	//#################################   YAHOO   #############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_YahGoMobile", _T("|Yahoo*Go*|Yahoo*Mobile*"), _T("Yahoo! Go Mobile"), IDI_YAHOO_GO_MOBILE, YAHOO_CASE, TRUE },
	{ "client_PingBox", _T("*Yahoo*PingBox*"), _T("Yahoo PingBox"), IDI_YAHOO_PINGBOX, YAHOO_CASE, TRUE },
	{ "client_YahooWeb", _T("*Yahoo*Web*Messenger"), _T("Yahoo Web Messenger"), IDI_YAHOO_WEB, YAHOO_CASE, TRUE },
	{ "client_libyahoo2", _T("*libyahoo*"), _T("libyahoo2"), IDI_LIBYAHOO2, YAHOO_CASE, TRUE },
	{ "client_Yahoo", _T("*Yahoo*"), _T("Yahoo Messenger"), IDI_YAHOO, YAHOO_CASE },

	//#########################################################################################################################################################################
	//#################################   WEATHER   ###########################################################################################################################
	//#########################################################################################################################################################################
	{ "client_accu", _T("*accuweather*"), _T("AccuWeather"), IDI_ACCU, WEATHER_CASE, TRUE },
	{ "client_gismeteo", _T("*gismeteo*"), _T("GisMeteo"), IDI_GISMETEO, WEATHER_CASE, TRUE },
	{ "client_intelli", _T("*intellicast*"), _T("Intellicast"), IDI_INTELLI, WEATHER_CASE, TRUE },
	{ "client_meteogid", _T("|*meteo-gid*|*meteogid*"), _T("Meteo-Gid"), IDI_METEOGID, WEATHER_CASE, TRUE },
	{ "client_meteonovosti", _T("*meteonovosti*"), _T("Meteonovosti"), IDI_METEONOVOSTI, WEATHER_CASE, TRUE },
	{ "client_noaa", _T("*noaa*"), _T("NOAA Weather"), IDI_NOAA, WEATHER_CASE, TRUE },
	{ "client_real", _T("*realmeteo*"), _T("RealMeteo"), IDI_REALMETEO, WEATHER_CASE, TRUE },
	{ "client_under", _T("Weather Underground*"), _T("Weather Underground"), IDI_UNDERGROUND, WEATHER_CASE, TRUE },
	{ "client_weatherxml", _T("*WeatherXML*"), _T("WeatherXML"), IDI_WEATHERXML, WEATHER_CASE, TRUE },
	{ "client_wetter", _T("*wetter*"), _T("Wetter"), IDI_WETTER, WEATHER_CASE, TRUE },
	{ "client_yweather", _T("*Yahoo Weather*"), _T("Yahoo Weather"), IDI_YWEATHER, WEATHER_CASE, TRUE },
	{ "client_weather_cn", _T("*weather.com.cn*"), _T("Weather CN"), IDI_WEATHER_CN, WEATHER_CASE, TRUE },
	{ "client_weather", _T("*weather*"), _T("Weather"), IDI_WEATHER, WEATHER_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   RSS   ###############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_rss09x", _T("*RSS*0.9*"), _T("RSS 0.9x"), IDI_RSS09, RSS_CASE, TRUE },
	{ "client_rss2", _T("*RSS*2.*"), _T("RSS 2"), IDI_RSS2, RSS_CASE, TRUE },
	{ "client_rss1", _T("*RSS*1.*"), _T("RSS 1"), IDI_RSS1, RSS_CASE, TRUE },
	{ "client_atom3", _T("*Atom*3*"), _T("Atom 3"), IDI_ATOM3, RSS_CASE, TRUE },
	{ "client_atom1", _T("*Atom*1*"), _T("Atom 1"), IDI_ATOM1, RSS_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################    QQ   ###############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_MobileQQ", _T("|Mobile*QQ*|iPhone*QQ*|QQ WAP*"), _T("Mobile QQ"), IDI_MOBILEQQ, QQ_CASE, TRUE },
	{ "client_QQConcept", _T("QQ*Concept*"), _T("QQ Concept"), IDI_QQCONCEPT, QQ_CASE, TRUE },
	{ "client_QQCRM", _T("QQ*CRM*"), _T("QQ CRM"), IDI_QQCRM, QQ_CASE, TRUE },
	{ "client_QQSilver", _T("QQ*Silverlight*"), _T("QQ Silverlight"), IDI_QQSILVER, QQ_CASE, TRUE },
	{ "client_TM2008", _T("|TM2008*|TM2009*|TM2010*"), LPGENT("TM 2008 and >"), IDI_TM2008, QQ_CASE },
	{ "client_TM", _T("TM*"), _T("TM"), IDI_TM, QQ_CASE },
	{ "client_QQInt", _T("QQ*International*"), _T("QQ International"), IDI_QQINT, QQ_CASE },
	{ "client_YamQQ", _T("YamQQ*"), _T("Yam QQ"), IDI_YAMQQ, QQ_CASE },
	{ "client_WebQQ", _T("WebQQ*"), _T("WebQQ"), IDI_WEBQQ, QQ_CASE },
	{ "client_QQ", _T("QQ*"), _T("QQ"), IDI_QQ, QQ_CASE },

	//#########################################################################################################################################################################
	//#################################   GADU-GADU   #########################################################################################################################
	//#########################################################################################################################################################################
	{ "client_GG", _T("|Gadu-Gadu*|GG*"), LPGENT("Gadu-Gadu client"), IDI_GG, GG_CASE },

	//#########################################################################################################################################################################
	//#################################   TLEN   ##############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_Tlen", _T("*Tlen*"), LPGENT("Tlen.pl client"), IDI_TLEN, TLEN_CASE },

	//#########################################################################################################################################################################
	//#################################   Facebook   ##########################################################################################################################
	//#########################################################################################################################################################################
	{ "client_Facebook_other", _T("*Facebook*other*"), _T("Facebook (other)"), IDI_FACEBOOK_OTHER, FACEBOOK_CASE },
	{ "client_Facebook_app", _T("*Facebook*App*"), _T("Facebook App"), IDI_FACEBOOK_APP, FACEBOOK_CASE },
	{ "client_Facebook_mess", _T("*Facebook*Messenger*"), _T("Facebook Messenger"), IDI_FACEBOOK_MESSENGER, FACEBOOK_CASE },
	{ "client_Facebook", _T("*Facebook*"), _T("Facebook"), IDI_FACEBOOK, FACEBOOK_CASE },

	//#########################################################################################################################################################################
	//#################################   VKontakte   #########################################################################################################################
	//#########################################################################################################################################################################
	{ "client_VK", _T("|*VKontakte*|*vk.com*"), _T("VKontakte"), IDI_VK, VK_CASE },

	//#########################################################################################################################################################################
	//#################################   OTHER CLIENTS   #####################################################################################################################
	//#########################################################################################################################################################################
	{ "client_Android", _T("Android*"), _T("Android"), IDI_ANDROID, MULTI_CASE, TRUE },

	{ "client_Pidgin", _T("|*Pidgin*|*libpurple*|Purple*"), _T("Pidgin (libpurple)"), IDI_PIDGIN, MULTI_CASE },

	{ "client_Python", _T("|*Python*|Py*|*ταλιςμαη*"), LPGENT("Python-based clients"), IDI_PYTHON, MULTI_CASE },

	{ "client_Jabber", _T("*Jabber*client*"), LPGENT("Jabber client"), IDI_JABBER, JABBER_CASE, TRUE },

	{ "client_XMPP", _T("|*XMPP*|Mrim*|*DRQZ00fz5WPn1gH+*"), LPGENT("XMPP client"), IDI_XMPP, JABBER_CASE },

	{ "client_Hangouts", _T("messaging-*"), _T("Google+ Hangouts"), IDI_HANGOUTS, OTHER_PROTOS_CASE, TRUE },

	{ "client_WhatsApp", _T("*WhatsApp*"), _T("WhatsApp"), IDI_WHATSAPP, OTHER_PROTOS_CASE, TRUE },

	{ "client_Twitter", _T("*Twitter*"), _T("Twitter"), IDI_TWITTER, OTHER_PROTOS_CASE, TRUE },

	{ "client_Skype", _T("*Skype**"), _T("Skype"), IDI_SKYPE, OTHER_PROTOS_CASE },

	{ "client_Xfire", _T("*Xfire*"), _T("Xfire"), IDI_XFIRE, OTHER_PROTOS_CASE, TRUE },
	
	{ "client_Steam", _T("*Steam*"), _T("Steam"), IDI_STEAM, OTHER_PROTOS_CASE },

	//#########################################################################################################################################################################
	//#################################   UNDEFINED CLIENTS   #################################################################################################################
	//#########################################################################################################################################################################

	{ "client_Notfound", _T("Notfound"), LPGENT("Client not found"), IDI_NOTFOUND, OTHERS_CASE, TRUE },
	{ "client_Unknown", _T("|*Unknown*|..."), LPGENT("Unknown client"), IDI_UNKNOWN, OTHERS_CASE, TRUE },
	{ "client_Undetected", _T("?*"), LPGENT("Undetected client"), IDI_UNDETECTED, OTHERS_CASE, TRUE },

};

int DEFAULT_KN_FP_MASK_COUNT = SIZEOF(def_kn_fp_mask);

//#########################################################################################################################################################################
//#################################   OVERLAYS LAYER #1   #################################################################################################################
//#########################################################################################################################################################################

KN_FP_MASK def_kn_fp_overlays_mask[] =
{// {"Client_IconName",         _T("|^*Mask*|*names*"),                         _T("Icon caption"),                 IDI_RESOURCE_ID,         CLIENT_CASE,    OVERLAY?    },
	//#########################################################################################################################################################################
	//#################################   MIRANDA PACKS OVERLAYS   ############################################################################################################
	//#########################################################################################################################################################################
	{ "client_AF_pack", _T("*AF*Pack*"), _T("AF"), IDI_MIRANDA_AF, MIRANDA_PACKS_CASE },
	{ "client_AlfaMaR_pack", _T("*AlfaMaR*"), _T("AlfaMaR"), IDI_MIRANDA_ALFAMAR, MIRANDA_PACKS_CASE },
	{ "client_Amatory_pack", _T("*Amatory*"), _T("Amatory"), IDI_MIRANDA_AMATORY, MIRANDA_PACKS_CASE },
	{ "client_BRI_pack", _T("*bri*edition*"), _T("Bri edition"), IDI_MIRANDA_BRI, MIRANDA_PACKS_CASE },
	{ "client_Devil_pack", _T("*6.6.6*"), _T("Devil Suite"), IDI_MIRANDA_DEVIL, MIRANDA_PACKS_CASE },
	{ "client_E33_pack", _T("*[E33*]*"), _T("E33"), IDI_MIRANDA_E33, MIRANDA_PACKS_CASE },
	{ "client_FR_pack", _T("*FR*Pack*"), _T("Miranda FR"), IDI_MIRANDA_FR, MIRANDA_PACKS_CASE },
	{ "client_Faith_pack", _T("*Faith*Pack*"), _T("FaithPack"), IDI_MIRANDA_FAITH, MIRANDA_PACKS_CASE },
	{ "client_Final_pack", _T("*[Final*Pack]*"), _T("Final pack"), IDI_MIRANDA_FINAL, MIRANDA_PACKS_CASE },
	{ "client_Freize_pack", _T("*Freize*"), _T("Freize"), IDI_MIRANDA_FREIZE, MIRANDA_PACKS_CASE },
	{ "client_Ghost_pack", _T("*Ghost's*"), _T("Ghost's pack"), IDI_MIRANDA_GHOST, MIRANDA_PACKS_CASE },
	{ "client_HCoffee_pack", _T("*[HotCoffee]*"), _T("HotCoffee"), IDI_MIRANDA_CAPPUCCINO, MIRANDA_PACKS_CASE },
	{ "client_HierOS_pack", _T("*HierOS*"), _T("HierOS"), IDI_MIRANDA_HIEROS, MIRANDA_PACKS_CASE },
	{ "client_ICE_pack", _T("|*miranda*[ice*]|*induction*"), _T("iCE / Induction"), IDI_MIRANDA_INDUCTION, MIRANDA_PACKS_CASE },
	{ "client_KDL_pack", _T("|*KDL*|*КДЛ*"), _T("KDL"), IDI_MIRANDA_KDL, MIRANDA_PACKS_CASE },
	{ "client_Kolich_pack", _T("*Kolich*"), _T("Kolich"), IDI_MIRANDA_KOLICH, MIRANDA_PACKS_CASE },
	{ "client_Kuzzman_pack", _T("*kuzzman*"), _T("Kuzzman"), IDI_MIRANDA_KUZZMAN, MIRANDA_PACKS_CASE },
	{ "client_Lenin_pack", _T("*[Lenin*]*"), _T("Lenin pack"), IDI_MIRANDA_LENINPACK, MIRANDA_PACKS_CASE },
	{ "client_Lestat_pack", _T("*[Lpack*]*"), _T("Lestat pack"), IDI_MIRANDA_LESTAT, MIRANDA_PACKS_CASE },
	{ "client_LexSys_pack", _T("|*Miranda*LS*|*LexSys*"), _T("LexSys"), IDI_MIRANDA_LEXSYS, MIRANDA_PACKS_CASE },
	{ "client_MD_pack", _T("*MDpack*"), _T("MDpack"), IDI_MIRANDA_MD, MIRANDA_PACKS_CASE },
	{ "client_Mataes_pack", _T("*Mataes*"), _T("Mataes pack"), IDI_MIRANDA_MATAES, MIRANDA_PACKS_CASE },
	{ "client_Mir_ME_pack", _T("*[Miranda*ME]*"), _T("Miranda ME"), IDI_MIRANDA_ME, MIRANDA_PACKS_CASE },
	{ "client_Native_pack", _T("*Native*"), _T("Native"), IDI_MIRANDA_NATIVE, MIRANDA_PACKS_CASE },
	{ "client_New_Style_pack", _T("*New*Style*"), _T("New Style"), IDI_MIRANDA_NEW_STYLE, MIRANDA_PACKS_CASE },
	{ "client_Pilot_pack", _T("*Pilot*"), _T("Pilot"), IDI_MIRANDA_PILOT, MIRANDA_PACKS_CASE },
	{ "client_Razunter_pack", _T("*Razunter*"), _T("Razunter's Pk"), IDI_MIRANDA_RAZUNTER, MIRANDA_PACKS_CASE },
	{ "client_Robyer_pack", _T("*Robyer*Pack*"), _T("Robyer pack"), IDI_MIRANDA_ROBYER, MIRANDA_PACKS_CASE },
	{ "client_SSS_pack", _T("*sss*pack*"), _T("SSS build"), IDI_MIRANDA_SSS_MOD, MIRANDA_PACKS_CASE },
	{ "client_Se7ven_pack", _T("|^*sss*|*[S7*pack]*|*[*S7*]*"), _T("Se7ven"), IDI_MIRANDA_SE7VEN, MIRANDA_PACKS_CASE },
	{ "client_SpellhowleR_pack", _T("*Spellhowler*"), _T("xSpellhowleRx pack"), IDI_MIRANDA_SPELLHOWLER, MIRANDA_PACKS_CASE },
	{ "client_Stalker_pack", _T("*Stalker*"), _T("Stalker"), IDI_MIRANDA_STALKER, MIRANDA_PACKS_CASE },
	{ "client_Tweety_pack", _T("*tweety*"), _T("Tweety"), IDI_MIRANDA_TWEETY, MIRANDA_PACKS_CASE },
	{ "client_Umedon_pack", _T("*Miranda*Umedon*"), _T("Umedon"), IDI_MIRANDA_UMEDON, MIRANDA_PACKS_CASE },
	{ "client_ValeraVi_pack", _T("*Valera*Vi*"), _T("ValeraVi"), IDI_MIRANDA_VALERAVI, MIRANDA_PACKS_CASE },
	{ "client_Watcher_pack", _T("*Watcher*"), _T("Watcher pack"), IDI_MIRANDA_WATCHER, MIRANDA_PACKS_CASE },
	{ "client_YAOL_pack", _T("*yaol*"), _T("YAOL"), IDI_MIRANDA_YAOL, MIRANDA_PACKS_CASE },
	{ "client_dar_veter_pack", _T("*Dar*veter*"), _T("Dar_veter pack"), IDI_MIRANDA_DAR, MIRANDA_PACKS_CASE },
	{ "client_dmikos_pack", _T("*dmikos*"), _T("Dmikos"), IDI_MIRANDA_DMIKOS, MIRANDA_PACKS_CASE },
	{ "client_zeleboba_pack", _T("*zeleboba*"), _T("zeleboba's"), IDI_MIRANDA_ZELEBOBA, MIRANDA_PACKS_CASE },

	//#########################################################################################################################################################################
	//#################################   PROTO OVERLAYS   ####################################################################################################################
	//#########################################################################################################################################################################
	{ "client_AIM_overlay", _T("|^AIM*|*AIM*"), LPGENT("AIM overlay"), IDI_AIM_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_ICQJ_Plus_over", _T("*ICQ*Plus*"), LPGENT("ICQJ Plus Mod overlay"), IDI_ICQJ_PLUS_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_ICQJ_overlay", _T("|^ICQ|^ICQ*|*ICQ*|ICQ client"), LPGENT("ICQJ overlay"), IDI_ICQJ_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_IRC_overlay", _T("|^IRC*|Miranda*IRC*"), LPGENT("IRC overlay"), IDI_IRC_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_JGmail_overlay", _T("*JGmail*"), LPGENT("JGmail overlay"), IDI_GMAIL_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_JGTalk_overlay", _T("*JGTalk*"), LPGENT("JGTalk overlay"), IDI_JGTALK_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_Jabber_overlay", _T("|^jabber*|Miranda*Jabber*|py*jabb*"), LPGENT("Jabber overlay"), IDI_JABBER_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_MSN_overlay", _T("|^MSN*|*Miranda*MSN*"), LPGENT("MSN overlay"), IDI_MSN_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_QQ_overlay", _T("|Miranda*QQ*|MIM*QQ*"), LPGENT("QQ overlay"), IDI_QQ_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_Tlen_overlay", _T("|^tlen*|*tlen*"), LPGENT("Tlen.pl overlay"), IDI_TLEN_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_Yahoo_overlay", _T("|^Yahoo*|*yahoo*"), LPGENT("Yahoo overlay"), IDI_YAHOO_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_MRA_overlay", _T("|^MRA*|^Mail.ru*")
							_T("|*MRA*|*Mail.ru*IM*|Mrim*")
							_T("|*DRQZ00fz5WPn1gH+*|*nxthUwJANorB6+*|MRA client"), LPGENT("Mail.Ru Agent overlay"), IDI_MAILRU_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_VK_overlay", _T("|Miranda*VKontakte*"), LPGENT("VK overlay"), IDI_VK_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_Skype_overlay", _T("|Miranda*Skype*"), LPGENT("Skype overlay"), IDI_SKYPE_OVERLAY, OVERLAYS_PROTO_CASE },

	//#########################################################################################################################################################################
	//#################################   CLIENT VERSION OVERLAYS   ###########################################################################################################
	//#########################################################################################################################################################################
	{ "client_ICQ3_over", _T("ICQ*2003*"), LPGENT("ICQ 2003 overlay"), IDI_ICQ3_OVERLAY, ICQ_CASE },
	{ "client_ICQ4_over", _T("|ICQ*4*|ICQ*lite*4*"), LPGENT("ICQ Lite 4 overlay"), IDI_ICQ4_OVERLAY, ICQ_CASE },
	{ "client_ICQ5_over", _T("|ICQ*5*|ICQ*lite*5*|icq5*"), LPGENT("ICQ v5.x overlay"), IDI_ICQ5_OVERLAY, ICQ_CASE },
	{ "client_ICQ6_over", _T("ICQ*6*"), LPGENT("ICQ v6.x overlay"), IDI_ICQ6_OVERLAY, ICQ_CASE },
	{ "client_ICQ7_over", _T("ICQ*7*"), LPGENT("ICQ v7.x overlay"), IDI_ICQ7_OVERLAY, ICQ_CASE },
	{ "client_ICQ8_over", _T("ICQ*8*"), LPGENT("ICQ v8.x overlay"), IDI_ICQ8_OVERLAY, ICQ_CASE },

	{ "client_Yahoo_11", _T("*Yahoo 11*"), LPGENT("Yahoo v11.x overlay"), IDI_YAHOO11_OVERLAY, YAHOO_CASE },
	{ "client_Yahoo_10", _T("*Yahoo 10*"), LPGENT("Yahoo v10.x overlay"), IDI_YAHOO10_OVERLAY, YAHOO_CASE },
	{ "client_Yahoo_9", _T("*Yahoo 9*"), LPGENT("Yahoo v9.x overlay"), IDI_YAHOO9_OVERLAY, YAHOO_CASE },

	{ "client_GG_11", _T("|Gadu-Gadu*11*|GG*11*"), LPGENT("Gadu-Gadu v11 client"), IDI_GG11_OVERLAY, GG_CASE },
	{ "client_GG_10", _T("|Gadu-Gadu*10*|GG*10"), LPGENT("Gadu-Gadu v10 client"), IDI_GG10_OVERLAY, GG_CASE },
	{ "client_GG_9", _T("|Gadu-Gadu*9*|GG*9*"), LPGENT("Gadu-Gadu v9 client"), IDI_GG9_OVERLAY, GG_CASE },
	{ "client_GG_8", _T("|Gadu-Gadu*8*|GG*8*"), LPGENT("Gadu-Gadu v8 client"), IDI_GG8_OVERLAY, GG_CASE },

	// {"client_Tlen_8",           _T("Tlen Protocol 1?"),                     LPGENT("Tlen 8.x overlay"),              IDI_TLEN8_OVERLAY,       TLEN_CASE                   },
	{ "client_Tlen_7", _T("Tlen Protocol 16"), LPGENT("Tlen 7.x overlay"), IDI_TLEN7_OVERLAY, TLEN_CASE },
	{ "client_Tlen_6", _T("Tlen Protocol 10"), LPGENT("Tlen 6.x overlay"), IDI_TLEN6_OVERLAY, TLEN_CASE },
	{ "client_Tlen_5", _T("Tlen Protocol 7"), LPGENT("Tlen 5.x overlay"), IDI_TLEN5_OVERLAY, TLEN_CASE },

	{ "client_QQ_2012", _T("|QQ2012*|TM2012*"), LPGENT("TM/QQ 2012 overlay"), IDI_QQ_2012_OVERLAY, QQ_CASE },
	{ "client_QQ_2011", _T("|QQ2011*|TM2011*"), LPGENT("TM/QQ 2011 overlay"), IDI_QQ_2011_OVERLAY, QQ_CASE },
	{ "client_QQ_2010", _T("|QQ2010*|TM2010*"), LPGENT("TM/QQ 2010 overlay"), IDI_QQ_2010_OVERLAY, QQ_CASE },
	{ "client_QQ_2009", _T("|QQ2009*|TM2009*"), LPGENT("TM/QQ 2009 overlay"), IDI_QQ_2009_OVERLAY, QQ_CASE },
	{ "client_QQ_2008", _T("|QQ2008*|TM2008*"), LPGENT("TM/QQ 2008 overlay"), IDI_QQ_2008_OVERLAY, QQ_CASE },
	{ "client_QQ_2007", _T("|QQ2007*|TM2007*"), LPGENT("TM/QQ 2007 overlay"), IDI_QQ_2007_OVERLAY, QQ_CASE },
	{ "client_QQ_2006", _T("|QQ2006*|TM2006*"), LPGENT("TM/QQ 2006 overlay"), IDI_QQ_2006_OVERLAY, QQ_CASE },
	{ "client_QQ_2005", _T("|QQ2005*|TM2005*"), LPGENT("TM/QQ 2005 overlay"), IDI_QQ_2005_OVERLAY, QQ_CASE },
	{ "client_QQ_2004", _T("QQ2004*"), LPGENT("QQ 2004 overlay"), IDI_QQ_2004_OVERLAY, QQ_CASE },
	{ "client_QQ_2003", _T("|QQ2003*|TM 3.*"), LPGENT("TM 3.x / QQ 2003 overlay"), IDI_QQ_2003_OVERLAY, QQ_CASE },
	{ "client_TM_2002", _T("TM 2.*"), LPGENT("TM 2.x overlay"), IDI_QQ_2002_OVERLAY, QQ_CASE },
	{ "client_TM_2001", _T("TM 1.*"), LPGENT("TM 1.x overlay"), IDI_QQ_2001_OVERLAY, QQ_CASE },
	{ "client_QQ_2000", _T("QQ2000*"), LPGENT("QQ 2000 overlay"), IDI_QQ_2000_OVERLAY, QQ_CASE },

	//#########################################################################################################################################################################
	//#################################   PLATFORM OVERLAYS   #################################################################################################################
	//#########################################################################################################################################################################
	{ "client_on_Win32", _T("|*Win*|* WM *|wmagent*|*Vista*"), LPGENT("Windows overlay"), IDI_PLATFORM_WIN, OVERLAYS_PLATFORM_CASE },
	{ "client_on_iOS", _T("|*ipad*|*iphone*|*iOS*"), LPGENT("iOS overlay (iPhone/iPad)"), IDI_PLATFORM_IOS, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Mac", _T("|^*smack*|* Mac *|*mac*"), LPGENT("MacOS overlay"), IDI_PLATFORM_MAC, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Linux", _T("*Linux*"), LPGENT("Linux overlay"), IDI_PLATFORM_LINUX, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Flash", _T("|*Flash*|*Web*ICQ*"), LPGENT("Flash overlay"), IDI_PLATFORM_FLASH, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Java", _T("|*Java*|jagent*|ICQ2Go!*"), LPGENT("Java overlay"), IDI_PLATFORM_JAVA, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Symbian", _T("|*Symbian*|sagent*"), LPGENT("Symbian overlay"), IDI_PLATFORM_SYMBIAN, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Amiga", _T("*Amiga*"), LPGENT("Amiga overlay"), IDI_PLATFORM_AMIGA, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Android", _T("|*Android*|*(android)*"), LPGENT("Android overlay"), IDI_PLATFORM_ANDROID, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Website", _T("|*(website)*|*(Web)*"), LPGENT("Website overlay"), IDI_PLATFORM_WEBSITE, OVERLAYS_PLATFORM_CASE },
	{ "client_on_WinPhone", _T("|*(wphone)*|*(WP)*"), LPGENT("Windows Phone overlay"), IDI_PLATFORM_WINPHONE, OVERLAYS_PLATFORM_CASE },
	{ "client_on_mobile", _T("*(mobile)*"), LPGENT("Mobile overlay"), IDI_PLATFORM_MOBILE, OVERLAYS_PLATFORM_CASE },
};

int DEFAULT_KN_FP_OVERLAYS_COUNT = SIZEOF(def_kn_fp_overlays_mask);

//#########################################################################################################################################################################
//#########################################################################################################################################################################
//#################################   OVERLAYS LAYER #2   #################################################################################################################
//#########################################################################################################################################################################
//#########################################################################################################################################################################

KN_FP_MASK def_kn_fp_overlays2_mask[] =
{// {"Client_IconName",         _T("|^*Mask*|*names*"),                         _T("Icon caption"),                 IDI_RESOURCE_ID,         CLIENT_CASE,    OVERLAY?    },
	{ "client_debug_overlay", _T("|*[*debug*]*|*test*|*тест*"), LPGENT("debug overlay"), IDI_DEBUG_OVERLAY, OVERLAYS_RESOURCE_CASE },
	{ "client_office_overlay", _T("|*[*office*]*|*[*офис*]*"), LPGENT("office overlay"), IDI_OFFICE_OVERLAY, OVERLAYS_RESOURCE_CASE },
	{ "client_mobile_overlay", _T("|*[*mobile*]*|*[*pda*]*"), LPGENT("mobile overlay"), IDI_MOBILE_OVERLAY, OVERLAYS_RESOURCE_CASE },
	{ "client_home_overlay", _T("|*[*home*]*|*[*дом*]*|*[*хоме*]*"), LPGENT("home overlay"), IDI_HOME_OVERLAY, OVERLAYS_RESOURCE_CASE },

	{ "client_work_overlay", _T("|*[*work*]*|*wrk*")
	_T("|*[*работа*]*|*ворк*"), LPGENT("work overlay"), IDI_WORK_OVERLAY, OVERLAYS_RESOURCE_CASE },

	{ "client_note_overlay", _T("|*[*note*]*|*[*laptop*]*")
	_T("|*[*нетбу*]*|*[*ноут*]*|*[*ноте*]*")
	_T("|*[*кирпич*]*|*[*portable*]*")
	_T("|*[*flash*]*|*[*usb*]*"), LPGENT("notebook overlay"), IDI_NOTEBOOK_OVERLAY, OVERLAYS_RESOURCE_CASE },

	//  {"client_MirNG_09_over",    _T("*Miranda*NG*\?.\?\?.9.*"),                  _T("Miranda NG v0.9 #2 overlay"),    IDI_MIRANDA_NG_V9,       MIRANDA_VERSION_CASE        },
	//  {"client_MirNG_08_over",    _T("*Miranda*NG*\?.\?\?.8.*"),                  _T("Miranda NG v0.8 #2 overlay"),    IDI_MIRANDA_NG_V8,       MIRANDA_VERSION_CASE        },
	//  {"client_MirNG_07_over",    _T("*Miranda*NG*\?.\?\?.7.*"),                  _T("Miranda NG v0.7 #2 overlay"),    IDI_MIRANDA_NG_V7,       MIRANDA_VERSION_CASE        },

	{ "client_MirNG_06_over", _T("*Miranda*NG*\?.\?\?.6.*"), LPGENT("Miranda NG v0.6 #2 overlay"), IDI_MIRANDA_NG_V6, MIRANDA_VERSION_CASE },
	{ "client_MirNG_05_over", _T("*Miranda*NG*\?.\?\?.5.*"), LPGENT("Miranda NG v0.5 #2 overlay"), IDI_MIRANDA_NG_V5, MIRANDA_VERSION_CASE },
	{ "client_MirNG_04_over", _T("*Miranda*NG*\?.\?\?.4.*"), LPGENT("Miranda NG v0.4 #2 overlay"), IDI_MIRANDA_NG_V4, MIRANDA_VERSION_CASE },
	{ "client_MirNG_03_over", _T("*Miranda*NG*\?.\?\?.3.*"), LPGENT("Miranda NG v0.3 #2 overlay"), IDI_MIRANDA_NG_V3, MIRANDA_VERSION_CASE },
	{ "client_MirNG_02_over", _T("*Miranda*NG*\?.\?\?.2.*"), LPGENT("Miranda NG v0.2 #2 overlay"), IDI_MIRANDA_NG_V2, MIRANDA_VERSION_CASE },
	{ "client_MirNG_01_over", _T("*Miranda*NG*\?.\?\?.1.*"), LPGENT("Miranda NG v0.1 #2 overlay"), IDI_MIRANDA_NG_V1, MIRANDA_VERSION_CASE },

	{ "client_MirIM_010_over", _T("*Miranda*0.10.*"), LPGENT("Miranda IM v0.10 #2 overlay"), IDI_MIRANDA_IM_V10, MIRANDA_VERSION_CASE },
	{ "client_MirIM_09_over", _T("*Miranda*0.9.*"), LPGENT("Miranda IM v0.9 #2 overlay"), IDI_MIRANDA_IM_V9, MIRANDA_VERSION_CASE },
	{ "client_MirIM_08_over", _T("*Miranda*0.8.*"), LPGENT("Miranda IM v0.8 #2 overlay"), IDI_MIRANDA_IM_V8, MIRANDA_VERSION_CASE },
	{ "client_MirIM_07_over", _T("*Miranda*0.7.*"), LPGENT("Miranda IM v0.7 #2 overlay"), IDI_MIRANDA_IM_V7, MIRANDA_VERSION_CASE },
	{ "client_MirIM_06_over", _T("*Miranda*0.6.*"), LPGENT("Miranda IM v0.6 #2 overlay"), IDI_MIRANDA_IM_V6, MIRANDA_VERSION_CASE },
	{ "client_MirIM_05_over", _T("*Miranda*0.5.*"), LPGENT("Miranda IM v0.5 #2 overlay"), IDI_MIRANDA_IM_V5, MIRANDA_VERSION_CASE },
	{ "client_MirIM_04_over", _T("*Miranda*0.4.*"), LPGENT("Miranda IM v0.4 #2 overlay"), IDI_MIRANDA_IM_V4, MIRANDA_VERSION_CASE },
};

int DEFAULT_KN_FP_OVERLAYS2_COUNT = SIZEOF(def_kn_fp_overlays2_mask);

//#########################################################################################################################################################################
//#########################################################################################################################################################################
//##############################   OVERLAYS LAYER #3   ####################################################################################################################
//#########################################################################################################################################################################
KN_FP_MASK def_kn_fp_overlays3_mask[] =
{// {"Client_IconName",         _T("|^*Mask*|*names*"),                         _T("Icon caption"),                 IDI_RESOURCE_ID,         CLIENT_CASE,    OVERLAY?    },
	{ "client_platform_x64", _T("|*x64*|*64*bit*"), LPGENT("x64 overlay"), IDI_PLATFORM_X64, OVERLAYS_PLATFORM_CASE },
	{ "client_platform_x32", _T("|*x32*|*32*bit*|*x86*"), LPGENT("x32 overlay"), IDI_PLATFORM_X32, OVERLAYS_PLATFORM_CASE },

	{ "client_Unicode", _T("*unicode*"), LPGENT("Unicode overlay"), IDI_UNICODE_CLIENT, OVERLAYS_UNICODE_CASE },
};

int DEFAULT_KN_FP_OVERLAYS3_COUNT = SIZEOF(def_kn_fp_overlays3_mask);

//#########################################################################################################################################################################
//#########################################################################################################################################################################
//##############################   OVERLAYS LAYER #4   ####################################################################################################################
//#########################################################################################################################################################################
KN_FP_MASK def_kn_fp_overlays4_mask[] =
{// {"Client_IconName",         _T("|^*Mask*|*names*"),                         _T("Icon caption"),                 IDI_RESOURCE_ID,         CLIENT_CASE,    OVERLAY?    },
	{ "client_NewGPG_over", _T("*New*GPG*"), LPGENT("NewGPG overlay"), IDI_NEWGPG_OVERLAY, OVERLAYS_SECURITY_CASE },
	{ "client_MirOTR_over", _T("*Mir*OTR*"), LPGENT("MirOTR overlay"), IDI_MIROTR_OVERLAY, OVERLAYS_SECURITY_CASE },
	{ "client_SecureIM_over", _T("*Secure*IM*"), LPGENT("SecureIM overlay"), IDI_SECUREIM_OVERLAY, OVERLAYS_SECURITY_CASE },
};

int DEFAULT_KN_FP_OVERLAYS4_COUNT = SIZEOF(def_kn_fp_overlays4_mask);
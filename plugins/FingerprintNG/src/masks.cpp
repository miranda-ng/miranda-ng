/*
Fingerprint NG (client version) icons module for Miranda NG
Copyright © 2006-22 ghazan, mataes, HierOS, FYR, Bio, nullbie, faith_healer and all respective contributors.

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

#include "stdafx.h"

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

KN_FP_MASK def_kn_fp_mask[] =
{// {"Client_IconName",         L"|^*Mask*|*names*",                         L"Icon caption",                 IDI_RESOURCE_ID,         CLIENT_CASE,    OVERLAY?    },
	//#########################################################################################################################################################################
	//#################################   MIRANDA   ###########################################################################################################################
	//#########################################################################################################################################################################

	{ "client_Miranda_NG", L"|*nightly.miranda.im*" L"|*Miranda*NG*" _A2W(MIRANDA_VERSION_CORE_STRING) L"*", L"Miranda NG", IDI_MIRANDA_NG, MIRANDA_CASE },

	{ "client_Miranda_NG_stbl", L"|Miranda*NG*", LPGENW("Miranda NG stable"), IDI_MIRANDA_NG_STABLE, MIRANDA_CASE },

	{ "client_Miranda_010xx", L"Miranda*IM*0.10.*", L"Miranda IM v0.10.x.x", IDI_MIRANDA_010X, MIRANDA_CASE },
	{ "client_Miranda_09XX", L"|*Miranda*IM*0.9*"
	L"|*miranda-im.org/caps#*0.9*", L"Miranda IM v0.9.x.x", IDI_MIRANDA_09XX, MIRANDA_CASE },
	{ "client_Miranda_08XX", L"|^Miranda*0.7*Jabb*"
	L"|*Miranda*0.8*"
	L"|*miranda-im.org/caps#*0.8*", L"Miranda IM v0.8.x.x", IDI_MIRANDA_08XX, MIRANDA_CASE },
	{ "client_Miranda_07XX", L"|*Miranda*0.7*"
	L"|*miranda-im.org/caps#*0.7*", L"Miranda IM v0.7.x.x", IDI_MIRANDA_07XX, MIRANDA_CASE },
	{ "client_Miranda_06XX", L"*Miranda*0.6*", L"Miranda IM v0.6.x.x", IDI_MIRANDA_06XX, MIRANDA_CASE },
	{ "client_Miranda_05XX", L"*Miranda*0.5*", L"Miranda IM v0.5.x.x", IDI_MIRANDA_05XX, MIRANDA_CASE },
	{ "client_Miranda_04XX", L"*Miranda*0.4*", L"Miranda IM v0.4.x.x", IDI_MIRANDA_04XX, MIRANDA_CASE },

	{ "client_Miranda_old", L"*Miranda*0.?*", LPGENW("Miranda IM (old versions)"), IDI_MIRANDA_OLD, MIRANDA_CASE },

	{ "client_Miranda_unknown", L"*Miranda*", LPGENW("Miranda (unknown)"), IDI_MIRANDA_UNKNOWN, MIRANDA_CASE },

	//#########################################################################################################################################################################
	//#################################   MULTI-PROTOCOL   ####################################################################################################################
	//#########################################################################################################################################################################
	{ "client_1&1", L"|*Pocket*Web*|*1&1*|*1*?nd*1", L"1&1", IDI_1UND1, MULTI_CASE, TRUE },
	{ "client_Asia", L"*Asia*", L"Asia", IDI_ASIA, MULTI_CASE, TRUE },
	{ "client_Adium", L"|*Adium*"
	L"|*VUFD6HcFmUT2NxJkBGCiKlZnS3M=*"          // Gabble client?
	L"|*DdnydQG7RGhP9E3k9Sf+b+bF0zo=*", L"Adium", IDI_ADIUM, MULTI_CASE, TRUE },
	{ "client_AgileMessenger", L"*Agile Messenger*", L"Agile Messenger", IDI_AGILE, MULTI_CASE, TRUE },

	{ "client_Appolo", L"*Appolo*", L"Appolo IM", IDI_APPOLO, MULTI_CASE, TRUE },
	{ "client_Ayttm", L"*Ayttm*", L"Ayttm", IDI_AYTTM, MULTI_CASE, TRUE },
	{ "client_BayanICQ", L"|*Bayan*ICQ*|*barobin*", L"BayanICQ", IDI_BAYANICQ, MULTI_CASE, TRUE },
	{ "client_Beejive", L"*Beejive*", L"BeejiveIM", IDI_BEEJIVE, MULTI_CASE, TRUE },
	{ "client_Bimoid", L"*Bimoid*", L"Bimoid", IDI_BIMOID, MULTI_CASE, TRUE },
	{ "client_BitlBee", L"*BitlBee*", L"BitlBee", IDI_BITLBEE, MULTI_CASE, TRUE },
	{ "client_BlackBerry", L"|*Berry*|*ICS?\?\?\?\?\?\?\?", L"BlackBerry", IDI_BLACKBERRY, MULTI_CASE, TRUE },
	{ "client_Buddycloud", L"*Buddycloud*", L"Buddycloud", IDI_BUDDYCLOUD, MULTI_CASE, TRUE },
	{ "client_Carrier", L"*Carrier*", L"Carrier client", IDI_CARRIER, MULTI_CASE, TRUE },
	{ "client_Causerie", L"*Causerie*", L"Causerie", IDI_CAUSERIE, MULTI_CASE, TRUE },
	{ "client_CenterIM", L"*CenterIM*", L"CenterIM", IDI_CENTERIM, MULTI_CASE, TRUE },
	{ "client_ChatSecure", L"*ChatSecure*", L"ChatSecure", IDI_CHATSECURE, MULTI_CASE, TRUE },
	{ "client_Citron", L"*Citron*", L"Citron", IDI_CITRON, MULTI_CASE, TRUE },
	{ "client_climm", L"*climm*", L"climm", IDI_CLIMM, MULTI_CASE, TRUE },
	{ "client_Digsby", L"*Digsby*", L"Digsby", IDI_DIGSBY, MULTI_CASE, TRUE },
	{ "client_EKG2", L"*EKG*2*", L"EKG2", IDI_EKG2, MULTI_CASE, TRUE },
	{ "client_EasyMessage", L"Easy*Message*", L"Easy Message", IDI_EASYMESSAGE, MULTI_CASE, TRUE },
	{ "client_Empathy", L"*Empathy*", L"Empathy", IDI_EMPATHY, MULTI_CASE, TRUE },
	{ "client_Eyeball", L"*Eyeball*", L"Eyeball Chat", IDI_EYEBALL_CHAT, MULTI_CASE, TRUE },
	{ "client_eBuddy", L"|*eBuddy*|*eMessenger*", L"eBuddy Messenger", IDI_EBUDDY, MULTI_CASE, TRUE },
	{ "client_eM_Client", L"*eM*Client*", L"eM Client", IDI_EM_CLIENT, MULTI_CASE, TRUE },
	{ "client_eQo", L"*eQo*", L"eQo", IDI_EQO, MULTI_CASE, TRUE },
	{ "client_emesene", L"*emesene*", L"emesene", IDI_EMESENE, MULTI_CASE, TRUE },
	{ "client_Fama", L"*Fama*", L"Fama IM", IDI_FAMAIM, MULTI_CASE, TRUE },
	{ "client_Fring", L"*fring*", L"Fring", IDI_FRING, MULTI_CASE, TRUE },
	{ "client_GMX", L"*GMX*", L"GMX MultiMessenger", IDI_GMX, MULTI_CASE, TRUE },
	{ "client_Gaim", L"*gaim*", L"Gaim (libgaim)", IDI_GAIM, MULTI_CASE, TRUE },
	{ "client_Galaxium", L"*Galaxium*", L"Galaxium", IDI_GALAXIUM, MULTI_CASE, TRUE },
	{ "client_GnuGadu", L"Gnu Gadu*", L"GNU Gadu", IDI_GNUGADU, MULTI_CASE, TRUE },
	{ "client_IM2", L"*IM2*", L"IM2", IDI_IM2, MULTI_CASE, TRUE },
	{ "client_IMPlus", L"|IM+*|IMPLUS*|*IM plus*|@mobile"
	L"|*+umBU9yx9Cu+R8jvPWIZ3vWD59M=*", L"IM+", IDI_IMPLUS, MULTI_CASE, TRUE },
	{ "client_IMVU", L"*IMVU*", L"IMVU", IDI_IMVU, MULTI_CASE, TRUE },
	{ "client_IMadering", L"*IMadering*", L"IMadering", IDI_IMADERING, MULTI_CASE, TRUE },
	{ "client_imoIM", L"|*imo.im*|*sleekxmpp.com*", L"imo.im", IDI_IMOIM, MULTI_CASE, TRUE },
	{ "client_Inlux", L"*Inlux*", L"Inlux Messenger", IDI_INLUX, MULTI_CASE, TRUE },
	{ "client_Instantbird", L"*Instantbird*", L"Instantbird", IDI_INSTANTBIRD, MULTI_CASE, TRUE },
	{ "client_iCall", L"*iCall*", L"iCall", IDI_ICALL, MULTI_CASE, TRUE },
	{ "client_iChat", L"|*iChat*|**imagent*|*iMessages*", L"iChat", IDI_ICHAT, MULTI_CASE, TRUE },
	{ "client_irssi", L"*irssi*", L"irssi", IDI_IRSSI, MULTI_CASE, TRUE },
	{ "client_JBother", L"*JBother*", L"JBother", IDI_JBOTHER, MULTI_CASE, TRUE },
	{ "client_JBuddy", L"*JBuddy*", L"JBuddy Messenger", IDI_JBUDDY, MULTI_CASE, TRUE },
	{ "client_Jabbear", L"*Jabbear*", L"Jabbear", IDI_JABBEAR, MULTI_CASE, TRUE },
	{ "client_Jabbin", L"*Jabbin*", L"Jabbin", IDI_JABBIN, MULTI_CASE, TRUE },
	{ "client_Jasmine_IM", L"|Jasmine*IM*|*jasmineicq.ru/caps*", L"Jasmine IM", IDI_JASMINEIM, MULTI_CASE, TRUE },

	{ "client_Jimm_Aspro", L"*Jimm*Aspro*", L"Jimm Aspro", IDI_JIMM_ASPRO, MULTI_CASE, TRUE },
	{ "client_Jimmy", L"*Jimmy*", L"JimmyIM", IDI_JIMMY, MULTI_CASE, TRUE },

	{ "client_KMess", L"*KMess*", L"KMess", IDI_KMESS, MULTI_CASE, TRUE },
	{ "client_KoolIM", L"*Kool*", L"KoolIM", IDI_KOOLIM, MULTI_CASE, TRUE },
	{ "client_Kopete", L"*Kopete*", L"Kopete", IDI_KOPETE, MULTI_CASE, TRUE },
	{ "client_LeechCraft", L"|*LeechCraft*"
	L"|*aNjQWbtza2QtXemMfBS2bwNOtcQ=*", L"LeechCraft", IDI_LEECHCRAFT, MULTI_CASE, TRUE },
	{ "client_MDC", L"*MDC*", L"MDC", IDI_MDC, MULTI_CASE, TRUE },
	{ "client_Meebo", L"Meebo*", L"Meebo", IDI_MEEBO, MULTI_CASE, TRUE },
	{ "client_Meetro", L"Meetro*", L"Meetro", IDI_MEETRO, MULTI_CASE, TRUE },
	{ "client_mChat", L"|mChat*|gsICQ*|*mchat.mgslab.com*", L"mChat", IDI_MCHAT, MULTI_CASE, TRUE },
	{ "client_Nimbuzz", L"*Nimbuzz*", L"Nimbuzz", IDI_NIMBUZZ, MULTI_CASE, TRUE },
	{ "client_Palringo", L"*Palringo*", L"Palringo", IDI_PALRINGO, MULTI_CASE, TRUE },

	{ "client_Pigeon", L"*PIGEON*", L"PIGEON!", IDI_PIGEON, MULTI_CASE, TRUE },
	{ "client_PlayXpert", L"*PlayXpert*", L"PlayXpert", IDI_PLAYXPERT, MULTI_CASE, TRUE },
	{ "client_Prelude", L"*Prelude*", L"Prelude", IDI_PRELUDE, MULTI_CASE, TRUE },
	{ "client_Proteus", L"*Proteus*", L"Proteus", IDI_PROTEUS, MULTI_CASE, TRUE },

	{ "client_QIP_Android", L"QIP *Android*", L"QIP Mobile Android", IDI_QIP_ANDROID, MULTI_CASE, TRUE },
	{ "client_QIP_iOS", L"|QIP*iOS*|QIP*iphone*|QIP*apple*", L"QIP Mobile iOS", IDI_QIP_IOS, MULTI_CASE, TRUE },
	{ "client_QIP_Symbian", L"*QIP*Symbian*", L"QIP Mobile Symbian", IDI_QIP_SYMBIAN, MULTI_CASE, TRUE },
	{ "client_QIP_Java", L"*QIP*Java*", L"QIP Mobile Java", IDI_QIP_JAVA, MULTI_CASE, TRUE },
	{ "client_QIP_PDA", L"|QIP *PDA*|*pda.qip.ru*|*QIP Mobile*", L"QIP Mobile", IDI_QIP_PDA, MULTI_CASE, TRUE },
	{ "client_QIP_2012", L"QIP 2012*", L"QIP 2012", IDI_QIP_2012, MULTI_CASE, TRUE },
	{ "client_QIP_2010", L"QIP 2010*", L"QIP 2010", IDI_QIP_2010, MULTI_CASE, TRUE },
	{ "client_QIP_Infium", L"|QIP Infium*|http://*qip*", L"QIP Infium", IDI_QIP_INFIUM, MULTI_CASE, TRUE },

	{ "client_qutIM", L"*qutIM*", L"qutIM", IDI_QUTIM, MULTI_CASE },
	{ "client_mqutIM", L"*mqutIM*", L"mqutIM", IDI_MQUTIM, MULTI_CASE },

	{ "client_Qnext", L"Qnext*", L"Qnext", IDI_QNEXT, MULTI_CASE, TRUE },

	{ "client_SAPO", L"*SAPO*", L"SAPO Messenger", IDI_SAPO, MULTI_CASE, TRUE },
	{ "client_SIM", L"|^*Simp*|*SIM*", L"SIM", IDI_SIM, MULTI_CASE, TRUE },
	{ "client_Salut_a_Toi", L"*Salut*Toi*", L"Salut a Toi", IDI_SALUT_A_TOI, MULTI_CASE, TRUE },
	{ "client_Shaim", L"*Shaim*", L"Shaim", IDI_SHAIM, MULTI_CASE, TRUE },
	{ "client_SieJC", L"|SieJC*|Nat*ICQ*|Siemens*Client*", L"Siemens ICQ / Jabber client", IDI_SIEJC, MULTI_CASE, TRUE },
	{ "client_Slick", L"Slick*", L"Slick", IDI_SLICK, MULTI_CASE, TRUE },
	{ "client_SrevIM", L"*Srev*IM*", L"SrevIM", IDI_SREVIM, MULTI_CASE, TRUE },

	{ "client_Tril_Android", L"*Trillian*Android*", L"Trillian Android", IDI_TRILLIAN_ANDROID, MULTI_CASE, TRUE },
	{ "client_Tril_Astra", L"Trillian*Astra*", L"Trillian Astra", IDI_TRILLIAN_ASTRA, MULTI_CASE, TRUE },
	{ "client_Trillian_Pro", L"Trillian*Pro*", L"Trillian Pro", IDI_TRILLIAN_PRO, MULTI_CASE, TRUE },
	{ "client_Trillian", L"*Trillian**", L"Trillian", IDI_TRILLIAN, MULTI_CASE, TRUE },

	{ "client_Tuukle_Chat", L"*Tuukle*Chat*|*IM*Gate*", L"Tuukle Chat", IDI_TUUKLE_CHAT, MULTI_CASE, TRUE },
	{ "client_vBuzzer", L"*vBuzzer*", L"vBuzzer", IDI_VBUZZER, MULTI_CASE, TRUE },
	{ "client_Virtus", L"*Virtus*", L"Virtus", IDI_VIRTUS, MULTI_CASE, TRUE },
	{ "client_uIM", L"*uIM*", L"uIM", IDI_UIM, MULTI_CASE, TRUE },
	{ "client_uTalk", L"*uTalk*", L"uTalk", IDI_UTALK, MULTI_CASE, TRUE },
	{ "client_WeeChat", L"*WeeChat*", L"WeeChat", IDI_WEECHAT, MULTI_CASE, TRUE },
	{ "client_Wippien", L"*Wippien*", L"Wippien", IDI_WIPPIEN, MULTI_CASE, TRUE },

	{ "client_WindowsPhone", L"*Windows*Phone*", L"Windows Phone", IDI_WINDOWS_PHONE, MULTI_CASE, TRUE },

	{ "client_YamiGo", L"YamiGo*", L"YamiGo", IDI_YAMIGO, MULTI_CASE, TRUE },
	{ "client_Yeigo", L"*Yeigo*", L"Yeigo", IDI_YEIGO, MULTI_CASE, TRUE },
	{ "client_Yoono", L"*Yoono*", L"Yoono", IDI_YOONO, MULTI_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   ICQ   ###############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_Mandarin_IM", L"Mandarin IM*", L"Mandarin IM", IDI_MANDARIN_IM, ICQ_CASE, TRUE },
	{ "client_R&Q", L"|R&Q*|&RQ*", L"R&Q", IDI_RANDQ, ICQ_CASE, TRUE },

	{ "client_ICQ_all", L"|ICQ?|ICQ?.?|ICQ *|ICQ2*|ICQ", L"ICQ client", IDI_ICQ, ICQ_CASE },

	//#########################################################################################################################################################################
	//#################################   JABBER   ############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_Akeni", L"*Akeni*", L"Akeni", IDI_AKENI, JABBER_CASE, TRUE },
	{ "client_Ambrosia", L"*Ambrosia*", L"Ambrosia XMPP Server", IDI_AMBROSIA, JABBER_CASE, TRUE },
	{ "client_AnothRSSBot", L"|*Anothrbot*|*Anothr Rss Bot*", L"Anothr Rss Bot", IDI_ANOTHRSSBOT, JABBER_CASE, TRUE },
	{ "client_Aqq", L"|*aqq.eu*|aqq*", L"Aqq", IDI_AQQ, JABBER_CASE, TRUE },
	{ "client_BarnOwl", L"*Barn*Owl*", L"BarnOwl", IDI_BARNOWL, JABBER_CASE, TRUE },
	{ "client_Beem", L"*Beem*", L"Beem", IDI_BEEM, JABBER_CASE, TRUE },
	{ "client_BellSouth", L"*BellSouth*", L"BellSouth", IDI_BELLSOUTH, JABBER_CASE, TRUE },
	{ "client_BitWise", L"*BitWise*", L"BitWise", IDI_BITWISE, JABBER_CASE, TRUE },
	{ "client_Bombus", L"*Bombus*", L"Bombus", IDI_BOMBUS, JABBER_CASE, TRUE },
	{ "client_BombusMod", L"|*Bombus*mod*|*bombusmod*", L"BombusMod", IDI_BOMBUS_MOD, JABBER_CASE, TRUE },
	{ "client_BombusNG", L"*Bombus*NG*", L"Bombus NG", IDI_BOMBUS_NG, JABBER_CASE, TRUE },
	{ "client_BombusQD", L"|*Bombusmod-qd*|*bombus*qd*", L"Bombus QD", IDI_BOMBUS_QD, JABBER_CASE, TRUE },
	{ "client_Bowline", L"*Bow*line*", L"Bowline", IDI_BOWLINE, JABBER_CASE, TRUE },
	{ "client_BuddySpace", L"Buddy*Space*", L"BuddySpace", IDI_BUDDYSPACE, JABBER_CASE, TRUE },
	{ "client_CJC", L"*CJC*", L"CJC", IDI_CJC, JABBER_CASE, TRUE },
	{ "client_CRoom", L"*CRoom*", L"CRoom", IDI_CROOM, JABBER_CASE, TRUE },
	{ "client_Candy", L"*Candy*", L"Candy", IDI_CANDY, JABBER_CASE, TRUE },
	{ "client_Chatopus", L"*Chatopus*", L"Chatopus", IDI_CHATOPUS, JABBER_CASE, TRUE },
	{ "client_Chikka", L"*Chikka*", L"Chikka", IDI_CHIKKA, JABBER_CASE, TRUE },
	{ "client_ChitChat", L"*Chit*Chat*", L"ChitChat", IDI_CHITCHAT, JABBER_CASE, TRUE },
	{ "client_Claros_Chat", L"*Claros*", L"Claros Chat", IDI_CLAROS_CHAT, JABBER_CASE, TRUE },
	{ "client_Coccinella", L"*Coccinella*", L"Coccinella", IDI_COCCINELLA, JABBER_CASE, TRUE },
	{ "client_Colibry", L"Colibry*", L"Colibry", IDI_COLIBRY, JABBER_CASE, TRUE },
	{ "client_Colloquy", L"Colloquy*", L"Colloquy", IDI_COLLOQUY, JABBER_CASE, TRUE },
	{ "client_CommuniGate", L"*CommuniGate*", L"CommuniGate Pro", IDI_COMMUNIGATE, JABBER_CASE, TRUE },
	{ "client_Conference", L"Conference*", L"Conference Bot (GMail)", IDI_CONFERENCE, JABBER_CASE, TRUE },
	{ "client_Conversations", L"|*http://conversations.im*|Conversations IM*", L"Conversations", IDI_CONVERSATIONS, JABBER_CASE, TRUE },
	{ "client_Crosstalk", L"*Cross*talk*", L"Crosstalk", IDI_CROSSTALK, JABBER_CASE, TRUE },
	{ "client_Cudumar", L"*Cudumar*", L"Cudumar", IDI_CUDUMAR, JABBER_CASE, TRUE },
	{ "client_CyclopsChat", L"*Cyclops*", L"Cyclops Chat", IDI_CYCLOPS_CHAT, JABBER_CASE, TRUE },
	{ "client_Desyr", L"*Desyr*", L"Desyr Messenger", IDI_DESYR, JABBER_CASE, TRUE },
	{ "client_EMess", L"*EMess*", L"EMess", IDI_EMESS, JABBER_CASE, TRUE },
	{ "client_Elmer_Bot", L"*Elmer*", L"Elmer Bot", IDI_ELMER, JABBER_CASE, TRUE },
	{ "client_Emacs", L"|*Jabber.el*|*Emacs*", L"Emacs (Jabber.el)", IDI_EMACS, JABBER_CASE, TRUE },
	{ "client_Exodus", L"*Exodus*", L"Exodus", IDI_EXODUS, JABBER_CASE, TRUE },

	{ "client_GCN", L"*GCN*", L"GCN", IDI_GCN, JABBER_CASE, TRUE },
	{ "client_GMail", L"|*gmail.*|GMail*", L"GoogleMail", IDI_GMAIL, JABBER_CASE, TRUE },
	{ "client_GOIM", L"*GOIM*", L"GOIM", IDI_GOIM, JABBER_CASE, TRUE },
	{ "client_GTalk", L"|*Talk.v*|*Google*Talk*" L"|*Gtalk*", L"GoogleTalk", IDI_GTALK, JABBER_CASE, TRUE },
	{ "client_GTalk_Gadget", L"|^messaging-*|*Talk*Gadget*", L"GTalk Gadget", IDI_GTALK_GADGET, JABBER_CASE, TRUE },
	{ "client_Gabber", L"*Gabber*", L"Gabber", IDI_GABBER, JABBER_CASE, TRUE },
	{ "client_Gajim", L"*Gajim*", L"Gajim", IDI_GAJIM, JABBER_CASE, TRUE },
	{ "client_Gibberbot", L"*Gibber*", L"Gibberbot", IDI_GIBBERBOT, JABBER_CASE, TRUE },
	{ "client_Glu", L"|glu*|*glu.net*", L"Glu", IDI_GLU, JABBER_CASE, TRUE },
	{ "client_Gnome", L"*Gnome*", L"Gnome", IDI_GNOME, JABBER_CASE, TRUE },
	{ "client_GoTalkMobile", L"*Go*Talk*Mobile*", L"GoTalkMobile", IDI_GOTALKMOBILE, JABBER_CASE, TRUE },
	{ "client_Gossip", L"*Gossip*", L"Gossip", IDI_GOSSIP, JABBER_CASE, TRUE },
	{ "client_GreenThumb", L"gReeNtHumB*", L"GreenThumb", IDI_GREENTHUMB, JABBER_CASE, TRUE },
	{ "client_Gush", L"*Gush*", L"Gush", IDI_GUSH, JABBER_CASE, TRUE },
	{ "client_IMCom", L"*IMCom*", L"IMCom", IDI_IMCOM, JABBER_CASE, TRUE },
	{ "client_IM_Friendly", L"*IM*Friendly*", L"IM Friendly!", IDI_IM_FRIENDLY, JABBER_CASE, TRUE },
	{ "client_Imified", L"*someresource*", L"Imified", IDI_IMIFIED, JABBER_CASE, TRUE },
	{ "client_Importal", L"*Importal*", L"Importal", IDI_IMPORTAL, JABBER_CASE, TRUE },
	{ "client_InstanT", L"*Instan-t*", L"Instan-t", IDI_INSTANT, JABBER_CASE, TRUE },
	{ "client_Interaction", L"*Interaction*", L"Interaction", IDI_INTERACTION, JABBER_CASE, TRUE },
	{ "client_iruka", L"*Iruka*", L"Iruka", IDI_IRUKA, JABBER_CASE, TRUE },
	{ "client_J2J_Transport", L"*J2J*Transport*", L"J2J Transport", IDI_J2J_TRANSPORT, JABBER_CASE, TRUE },
	{ "client_Jamm", L"*Jamm*", L"Jamm", IDI_JAMM, JABBER_CASE, TRUE },
	{ "client_JClaim", L"*JClaim*", L"JClaim", IDI_JCLAIM, JABBER_CASE, TRUE },
	{ "client_JMC", L"JMC*", L"JMC (Jabber Mix Client)", IDI_JMC, JABBER_CASE, TRUE },
	{ "client_JWChat", L"*JWChat*", L"JWChat", IDI_JWCHAT, JABBER_CASE, TRUE },
	{ "client_JWGC", L"|*JWGC*|Jabber *Gram*", L"Jabber WindowGram Client", IDI_JWGC, JABBER_CASE, TRUE },
	{ "client_Jabba", L"*Jabba*", L"Jabba", IDI_JABBA, JABBER_CASE, TRUE },

	{ "client_JabberApplet", L"Jabber*Applet*", L"JabberApplet", IDI_JABBER_APPLET, JABBER_CASE, TRUE },
	{ "client_JabberBeOS", L"Jabber*BeOS*", L"Jabber (BeOS)", IDI_JABBER_BEOS, JABBER_CASE, TRUE },
	{ "client_JabberFoX", L"*fox*", L"JabberFoX", IDI_JABBERFOX, JABBER_CASE, TRUE },
	{ "client_JabberMSNGR", L"Jabber Messenger*", L"Jabber Messenger", IDI_JABBER_MESSENGER, JABBER_CASE, TRUE },
	{ "client_JabberNaut", L"*Jabber*Naut*", L"JabberNaut", IDI_JABBERNAUT, JABBER_CASE, TRUE },
	{ "client_JabberZilla", L"*Zilla*", L"JabberZilla", IDI_JABBERZILLA, JABBER_CASE, TRUE },
	{ "client_Jabber_Net", L"|*Jabber*Net*|*cursive.net*|*csharp*", L"Jabber-Net", IDI_JABBER_NET, JABBER_CASE, TRUE },
	{ "client_Jabberwocky", L"Jabberwocky*", L"Jabberwocky (Amiga)", IDI_JABBERWOCKY, JABBER_CASE, TRUE },
	{ "client_Jabbroid", L"*Jabbroid*", L"Jabbroid", IDI_JABBROID, JABBER_CASE, TRUE },
	{ "client_Jajc", L"|*Jajc*|Just Another Jabber Client", L"JAJC", IDI_JAJC, JABBER_CASE, TRUE },
	{ "client_Jeti", L"*Jeti*", L"Jeti", IDI_JETI, JABBER_CASE, TRUE },
	{ "client_Jitsi", L"*Jitsi*", L"Jitsi", IDI_JITSI, JABBER_CASE, TRUE },
	{ "client_Joost", L"*Joost*", L"Joost", IDI_JOOST, JABBER_CASE, TRUE },
	{ "client_Kadu", L"*Kadu*", L"Kadu", IDI_KADU, JABBER_CASE, TRUE },
	{ "client_Konnekt", L"Konnekt*", L"Konnekt", IDI_KONNEKT, JABBER_CASE, TRUE },
	{ "client_LLuna", L"LLuna*", L"LLuna", IDI_LLUNA, JABBER_CASE, TRUE },
	{ "client_Lamp", L"*Lamp*IM*", L"Lamp IM", IDI_LAMP_IM, JABBER_CASE, TRUE },
	{ "client_Lampiro", L"*Lampiro*", L"Lampiro", IDI_LAMPIRO, JABBER_CASE, TRUE },
	{ "client_Landell", L"*Landell*", L"Landell", IDI_LANDELL, JABBER_CASE, TRUE },
	{ "client_Leaf", L"*Leaf*", L"Leaf Messenger", IDI_LEAF, JABBER_CASE, TRUE },
	{ "client_LinQ", L"*LinQ*", L"LinQ", IDI_LINQ, JABBER_CASE, TRUE },
	{ "client_M8Jabber", L"*M8Jabber*", L"M8Jabber", IDI_M8JABBER, JABBER_CASE, TRUE },
	{ "client_MCabber", L"*mcabber*", L"MCabber", IDI_MCABBER, JABBER_CASE, TRUE },
	{ "client_MGTalk", L"|*MGTalk*|*Mobile?\?\?\?\?\?\?\?", L"MGTalk", IDI_MGTALK, JABBER_CASE, TRUE },
	{ "client_MUCkl", L"*MUCkl*", L"MUCkl", IDI_MUCKL, JABBER_CASE, TRUE },
	{ "client_Mango", L"*Mango*", L"Mango", IDI_MANGO, JABBER_CASE, TRUE },
	{ "client_Mercury", L"*Mercury*", L"Mercury Messenger", IDI_MERCURY_MESSENGER, JABBER_CASE, TRUE },
	{ "client_Monal", L"*Monal*", L"Monal", IDI_MONAL, JABBER_CASE, TRUE },
	{ "client_MozillaChat", L"*Mozilla*Chat*", L"MozillaChat", IDI_MOZILLACHAT, JABBER_CASE, TRUE },
	{ "client_Neos", L"Neos*", L"Neos", IDI_NEOS, JABBER_CASE, TRUE },
	{ "client_Nitro", L"Nitro*", L"Nitro", IDI_NITRO, JABBER_CASE, TRUE },
	{ "client_Nostromo", L"*USCSS*Nostromo*", L"USCSS Nostromo", IDI_NOSTROMO, JABBER_CASE, TRUE },
	{ "client_OM", L"OM*", L"OM aka Online Messenger", IDI_OM, JABBER_CASE, TRUE },
	{ "client_OctroTalk", L"*Octro*", L"OctroTalk", IDI_OCTROTALK, JABBER_CASE, TRUE },
	{ "client_OneTeam", L"*OneTeam*", L"OneTeam", IDI_ONETEAM, JABBER_CASE, TRUE },
	{ "client_Openfire", L"*Openfire*", L"Openfire", IDI_OPENFIRE, JABBER_CASE, TRUE },
	{ "client_Fire", L"Fire*", L"Fire", IDI_FIRE, JABBER_CASE, TRUE },
	{ "client_Paltalk", L"*Paltalk*", L"Paltalk", IDI_PALTALK, JABBER_CASE, TRUE },
	{ "client_Pandion", L"|*Pandion*|*Пандион*", L"Pandion", IDI_PANDION, JABBER_CASE, TRUE },
	{ "client_Papla", L"*Papla*", L"Papla", IDI_PAPLA, JABBER_CASE, TRUE },
	{ "client_Poezio", L"*Poezio*", L"Poezio", IDI_POEZIO, JABBER_CASE, TRUE },
	{ "client_Prosody", L"*Prosody*", L"Prosody", IDI_PROSODY, JABBER_CASE, TRUE },

	{ "client_Psi_plus", L"|*PSI+*|*psi-dev.googlecode*", L"PSI+", IDI_PSIPLUS, JABBER_CASE, TRUE },
	{ "client_Psi", L"*Psi*", L"PSI", IDI_PSI, JABBER_CASE, TRUE },

	{ "client_Psto", L"*Psto*", L"Psto.net", IDI_PSTO, JABBER_CASE, TRUE },
	{ "client_Psyc", L"*Psyc*", L"Psyc", IDI_PSYC, JABBER_CASE, TRUE },
	{ "client_Pygeon", L"*Pygeon*", L"Pygeon", IDI_PYGEON, JABBER_CASE, TRUE },

	{ "client_QTJim", L"*QTJim*", L"QTJim", IDI_QTJIM, JABBER_CASE, TRUE },
	{ "client_QuteCom", L"*Qute*Com*", L"QuteCom", IDI_QUTECOM, JABBER_CASE, TRUE },
	{ "client_RenRen", L"|*WTalkProxy0_0*|*talk.xiaonei.com*", L"RenRen", IDI_RENREN, JABBER_CASE, TRUE },
	{ "client_SBot", L"*SBot*", L"SBot", IDI_SBOT, JABBER_CASE, TRUE },
	{ "client_SMTP_Transport", L"*smtp*transport*", L"SMTP Transport", IDI_SMTP_TRANSPORT, JABBER_CASE, TRUE },
	{ "client_SamePlace", L"*SamePlace*", L"SamePlace", IDI_SAMEPLACE, JABBER_CASE, TRUE },
	{ "client_Sky_Messager", L"Sky*Mess*", L"Sky Messager", IDI_SKYMESSAGER, JABBER_CASE, TRUE },
	{ "client_Sky_Messager", L"*Sky*Messager*", L"Sky Messager", IDI_SKYMESSAGER, JABBER_CASE, TRUE },

	{ "client_xabber", L"|*xabber*"
	L"|*GyIX*", L"xabber", IDI_XABBER, JABBER_CASE, TRUE },
	{ "client_Gabble", L"*Gabble*", L"Gabble", IDI_GABBLE, JABBER_CASE, TRUE },
	{ "client_Smack", L"|*igniterealtime.*smack*|*smack*", L"Smack", IDI_SMACK, JABBER_CASE, TRUE },

	{ "client_SoapBox", L"*SoapBox*", L"SoapBox", IDI_SOAPBOX, JABBER_CASE, TRUE },
	{ "client_Spark", L"*Spark*", L"Spark", IDI_SPARK, JABBER_CASE, TRUE },
	{ "client_Speakall", L"*Speak*all*", L"Speakall", IDI_SPEAKALL, JABBER_CASE, TRUE },
	{ "client_Speeqe", L"*Speeqe*", L"Speeqe", IDI_SPEEQE, JABBER_CASE, TRUE },
	{ "client_Spik", L"*Spik*", L"Spik", IDI_SPIK, JABBER_CASE, TRUE },
	{ "client_Swift", L"*Swift*", L"Swift", IDI_SWIFT, JABBER_CASE, TRUE },
	{ "client_SworIM", L"*Swor*IM*", L"SworIM", IDI_SWORIM, JABBER_CASE, TRUE },
	{ "client_Synapse", L"*Synapse*", L"Synapse", IDI_SYNAPSE, JABBER_CASE, TRUE },
	{ "client_Talkdroid", L"*Talkdroid*", L"Talkdroid", IDI_TALKDROID, JABBER_CASE, TRUE },
	{ "client_Talkonaut", L"*Talkonaut*", L"Talkonaut", IDI_TALKONAUT, JABBER_CASE, TRUE },
	{ "client_Tapioca", L"*Tapioca*", L"Tapioca", IDI_TAPIOCA, JABBER_CASE, TRUE },
	{ "client_Teabot", L"|*teabot*|*teabot.org/bot*", L"Teabot", IDI_TEABOT, JABBER_CASE, TRUE },
	{ "client_Telepathy", L"*Telepathy*", L"Telepathy", IDI_TELEPATHY, JABBER_CASE, TRUE },
	{ "client_The_Bee", L"*The*Bee*", L"The Bee", IDI_THEBEE, JABBER_CASE, TRUE },
	{ "client_Thunderbird", L"*Thunderbi*", L"Thunderbird", IDI_THUNDERBIRD, JABBER_CASE, TRUE },
	{ "client_Tigase", L"*Tigase*", L"Tigase", IDI_TIGASE, JABBER_CASE, TRUE },
	{ "client_TipicIM", L"Tipic*", L"TipicIM", IDI_TIPICIM, JABBER_CASE, TRUE },
	{ "client_Tkabber", L"*Tkabber*", L"Tkabber", IDI_TKABBER, JABBER_CASE, TRUE },
	{ "client_TransactIM", L"*Transact*", L"TransactIM", IDI_TRANSACTIM, JABBER_CASE, TRUE },
	{ "client_Translate", L"*Translate*", L"Translate component", IDI_TRANSLATE, JABBER_CASE, TRUE },
	{ "client_Triple", L"Triple*", L"TripleSoftwareIM (TSIM)", IDI_TRIPLE_SOFTWARE, JABBER_CASE, TRUE },
	{ "client_Vacuum", L"*Vacuum*", L"Vacuum IM", IDI_VACUUM, JABBER_CASE, TRUE },
	{ "client_V&V", L"*V&V*", L"V&V Messenger", IDI_VANDV, JABBER_CASE, TRUE },
	{ "client_Vayusphere", L"*Vayusphere*", L"Vayusphere", IDI_VAYUSPHERE, JABBER_CASE, TRUE },
	{ "client_Vysper", L"*Vysper*", L"Vysper", IDI_VYSPER, JABBER_CASE, TRUE },
	{ "client_WTW", L"**WTW**|*wtw.k2t.eu*", L"WTW", IDI_WTW, JABBER_CASE, TRUE },
	{ "client_WannaChat", L"Wanna*Chat*", L"WannaChat", IDI_WANNACHAT, JABBER_CASE, TRUE },
	{ "client_WebEx", L"*webex.com*", L"Cisco WebEx Connect", IDI_WEBEX, JABBER_CASE, TRUE },
	{ "client_WhisperIM", L"*Whisper*", L"WhisperIM", IDI_WHISPERIM, JABBER_CASE, TRUE },
	{ "client_Wija", L"*wija*", L"Wija", IDI_WIJA, JABBER_CASE, TRUE },
	{ "client_Wildfire", L"Wildfire*", L"Wildfire", IDI_WILDFIRE, JABBER_CASE, TRUE },
	{ "client_WinJab", L"*WinJab*", L"WinJab", IDI_WINJAB, JABBER_CASE, TRUE },
	{ "client_Xiffian", L"*Xiffian*", L"Xiffian", IDI_XIFFIAN, JABBER_CASE, TRUE },
	{ "client_Yambi", L"*Yambi*", L"Yambi", IDI_YAMBI, JABBER_CASE, TRUE },
	{ "client_chat_bots", L"*chat*bot*", L"chat bot", IDI_CHAT_BOT, JABBER_CASE, TRUE },
	{ "client_dziObber", L"*dzi?bber*", L"dziObber", IDI_DZIOBBER, JABBER_CASE, TRUE },
	{ "client_ejabberd", L"*ejabberd*", L"ejabberd", IDI_EJABBERD, JABBER_CASE, TRUE },
	{ "client_emite", L"*emite*", L"emite", IDI_EMITE, JABBER_CASE, TRUE },
	{ "client_gYaber", L"gYaber*", L"gYaber", IDI_GYABER, JABBER_CASE, TRUE },
	{ "client_glu", L"*glu*", L"glu", IDI_GLU, JABBER_CASE, TRUE },
	{ "client_iGoogle", L"iGoogle*", L"iGoogle", IDI_IGOOGLE, JABBER_CASE, TRUE },
	{ "client_iJab", L"*iJab*", L"iJab", IDI_IJAB, JABBER_CASE, TRUE },
	{ "client_iMeem", L"iMeem*", L"iMeem", IDI_IMEEM, JABBER_CASE, TRUE },
	{ "client_iMov", L"*imov*", L"iMov", IDI_IMOV, JABBER_CASE, TRUE },
	{ "client_jTalk", L"|*jTalk*|http://jtalk*", L"jTalk", IDI_JTALK, JABBER_CASE, TRUE },
	{ "client_jTalkmod", L"|*jTalkmod*"
	L"|*glSvJ3yM3M2f53oregNy6fYwocY=*"
	L"|*XEssZlSs8oF4EcTHU1b8BsVxcPg=*", L"jTalkmod", IDI_JTALKMOD, JABBER_CASE, TRUE },
	{ "client_jabberDisk", L"|*jdisk*|*jabber*Disk*", L"jabberDisk", IDI_JABBER_DISK, JABBER_CASE, TRUE },
	{ "client_jabbim", L"*jabbim*", L"Jabbim", IDI_JABBIM, JABBER_CASE, TRUE },
	{ "client_jabiru", L"*jabiru*", L"Jabiru", IDI_JABIRU, JABBER_CASE, TRUE },
	{ "client_jappix", L"*jappix*", L"jappix", IDI_JAPPIX, JABBER_CASE, TRUE },
	{ "client_jrudevels", L"*jrudevels*", L"Jrudevels", IDI_JRUDEVELS, JABBER_CASE, TRUE },
	{ "client_juick", L"*juick*", L"Juick", IDI_JUICK, JABBER_CASE, TRUE },
	{ "client_kf", L"|^*smack*|*kf*", L"kf jabber", IDI_KF, JABBER_CASE, TRUE },
	{ "client_laffer", L"*laffer*", L"Laffer", IDI_LAFFER, JABBER_CASE, TRUE },
	{ "client_mJabber", L"*mJabber*", L"mJabber", IDI_MJABBER, JABBER_CASE, TRUE },
	{ "client_meinvz", L"*meinvz*", L"MeinVZ", IDI_MEINVZ, JABBER_CASE, TRUE },
	{ "client_moJab", L"*moJab*", L"moJab", IDI_MOJAB, JABBER_CASE, TRUE },
	{ "client_mobber", L"*mobber*", L"mobber", IDI_MOBBER, JABBER_CASE, TRUE },
	{ "client_myJabber", L"*myJabber*", L"myJabber", IDI_MYJABBER, JABBER_CASE, TRUE },
	{ "client_orkut", L"*orkut*", L"orkut", IDI_ORKUT, JABBER_CASE, TRUE },
	{ "client_pjc", L"|*PJC*|*pjc.googlecode.com*", LPGENW("PHP Jabber Client"), IDI_PJC, JABBER_CASE, TRUE },
	{ "client_saje", L"*saje*", L"saje", IDI_SAJE, JABBER_CASE, TRUE },
	{ "client_schuelervz", L"*schuelervz*", L"SchulerVZ", IDI_SCHULERVZ, JABBER_CASE, TRUE },
	{ "client_studivz", L"*studivz*", L"StudiVZ", IDI_STUDIVZ, JABBER_CASE, TRUE },
	{ "client_tkchat", L"*tkchat*", L"tkchat", IDI_TKCHAT, JABBER_CASE, TRUE },
	//  {"client_uJabber",          L"*uJabber*",                                L"uJabber",                      IDI_UJABBER,             JABBER_CASE,        TRUE    },
	{ "client_uKeeper", L"*uKeeper*", L"uKeeper", IDI_UKEEPER, JABBER_CASE, TRUE },
	{ "client_whoisbot", L"whoisbot", L"Swissjabber Whois Bot", IDI_WHOISBOT, JABBER_CASE, TRUE },
	{ "client_xeus2", L"*xeus 2*", L"xeus 2", IDI_XEUS2, JABBER_CASE, TRUE },
	{ "client_xeus", L"*xeus*", L"xeus", IDI_XEUS, JABBER_CASE, TRUE },
	{ "client_yaonline", L"|*yandex*|*yaonline*"
	L"|*Я.Онлайн*|*Яндекс*", L"Ya.Online", IDI_YAONLINE, JABBER_CASE, TRUE },
	{ "client_yaxim", L"*yaxim*", L"yaxim", IDI_YAXIM, JABBER_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   IRC   ###############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_AmIRC", L"*AmIRC*", L"AmIRC", IDI_AMIRC, IRC_CASE, TRUE },
	{ "client_Babbel", L"*Babbel*", L"Babbel", IDI_BABBEL, IRC_CASE, TRUE },
	{ "client_BersIRC", L"*BersIRC*", L"BersIRC", IDI_BERSIRC, IRC_CASE, TRUE },
	{ "client_ChatZilla", L"*ChatZilla*", L"ChatZilla", IDI_CHATZILLA, IRC_CASE, TRUE },
	{ "client_Conversation", L"*Conversation*", L"Conversation", IDI_CONVERSATION, IRC_CASE, TRUE },
	{ "client_Eggdrop", L"*Eggdrop*", L"Eggdrop", IDI_EGGDROP, IRC_CASE, TRUE },
	{ "client_EggdropRacBot", L"*Eggdrop*RacBot*", L"Eggdrop RacBot", IDI_EGGDROP_RACBOT, IRC_CASE, TRUE },
	{ "client_FChat", L"*FChat*", L"FChat", IDI_FCHAT, IRC_CASE, TRUE },
	{ "client_GDPChat", L"*GDPChat*", L"GDP Web Chat", IDI_GDPCHAT, IRC_CASE, TRUE },
	{ "client_GoPowerTools", L"*Go*PowerTools*", L"GoPowerTools", IDI_GOPOWERTOOLS, IRC_CASE, TRUE },
	{ "client_HydraIRC", L"*Hydra*IRC*", L"HydraIRC", IDI_HYDRA_IRC, IRC_CASE, TRUE },
	{ "client_IRCXpro", L"*IRCXpro*", L"IRCXpro", IDI_IRCXPRO, IRC_CASE, TRUE },
	{ "client_IceChat", L"*Ice*Chat*", L"IceChat", IDI_ICECHAT, IRC_CASE, TRUE },
	{ "client_KSirc", L"*ksirk*", L"KSirc", IDI_KSIRC, IRC_CASE, TRUE },
	{ "client_KVIrc", L"*KVIrc*", L"KVIrc", IDI_KVIRC, IRC_CASE, TRUE },
	{ "client_Klient", L"*Klient*", L"Klient", IDI_KLIENT, IRC_CASE, TRUE },
	{ "client_Konversation", L"*Konversation*", L"Konversation", IDI_KONVERSATION, IRC_CASE, TRUE },
	{ "client_MP3Script", L"*MP3*Script*", LPGENW("MP3 Script for mIRC"), IDI_MP3_SCRIPT, IRC_CASE, TRUE },
	{ "client_NeoRaTrion", L"*NeoRa*Trion*", L"NeoRa Trion", IDI_NEORATRION, IRC_CASE, TRUE },
	{ "client_Nettalk", L"*Nettalk*", L"Nettalk", IDI_NETTALK, IRC_CASE, TRUE },
	{ "client_NoNameScript", L"*NoName*Script*", L"NoNameScript", IDI_NONAME_SCRIPT, IRC_CASE, TRUE },
	{ "client_Opera", L"*Opera*", L"Opera", IDI_OPERA, IRC_CASE, TRUE },
	{ "client_PJIRC", L"*PJIRC*", L"PJIRC", IDI_PJIRC, IRC_CASE, TRUE },
	{ "client_Pirch", L"*Pirch*", L"Pirch", IDI_PIRCH, IRC_CASE, TRUE },
	{ "client_PocketIRC", L"*Pocket*IRC*", L"Pocket IRC", IDI_POCKET_IRC, IRC_CASE, TRUE },
	{ "client_ProChat", L"*Pro*Chat*", L"ProChat", IDI_PROCHAT, IRC_CASE, TRUE },
	{ "client_SmartIRC", L"*Smart*IRC*", L"SmartIRC", IDI_SMART_IRC, IRC_CASE, TRUE },
	{ "client_Snak", L"*Snak*", L"Snak", IDI_SNAK, IRC_CASE, TRUE },
	{ "client_SysReset", L"*Sys*Reset*", L"SysReset", IDI_SYSRESET, IRC_CASE, TRUE },
	{ "client_VircaIRC", L"*VircaIRC*", L"VircaIRC", IDI_VIRCAIRC, IRC_CASE, TRUE },
	{ "client_VisionIRC", L"*VisionIRC*", L"VisionIRC", IDI_VISIONIRC, IRC_CASE, TRUE },
	{ "client_VisualIRC", L"*VisualIRC*", L"VisualIRC", IDI_VISUALIRC, IRC_CASE, TRUE },
	{ "client_VortecIRC", L"*VortecIRC*", L"VortecIRC", IDI_VORTECIRC, IRC_CASE, TRUE },
	{ "client_WLIrc", L"*WLIrc*", L"WLIrc", IDI_WLIRC, IRC_CASE, TRUE },
	{ "client_XChatAqua", L"*X*Chat*Aqua*", L"X-Chat Aqua", IDI_XCHATAQUA, IRC_CASE, TRUE },
	{ "client_XiRCON", L"*XiRCON*", L"XiRCON", IDI_XIRCON, IRC_CASE, TRUE },
	{ "client_Xirc", L"*Xirc*", L"Xirc", IDI_XIRC, IRC_CASE, TRUE },
	{ "client_cbirc", L"*cbirc*", L"cbirc", IDI_CBIRC, IRC_CASE, TRUE },
	{ "client_dIRC", L"*dIRC*", L"dIRC", IDI_DIRC, IRC_CASE, TRUE },

	{ "client_iroffer_dinoex", L"*iroffer*dinoex*", L"iroffer dinoex", IDI_IROFFER_DINOEX, IRC_CASE, TRUE },
	{ "client_iroffer", L"*iroffer*", L"iroffer", IDI_IROFFER, IRC_CASE, TRUE },

	{ "client_ircle", L"*ircle*", L"ircle", IDI_IRCLE, IRC_CASE, TRUE },
	{ "client_jircii", L"*jircii*", L"jircii", IDI_JIRCII, IRC_CASE, TRUE },
	{ "client_jmIrc", L"*jmIrc*", L"jmIrc", IDI_JMIRC, IRC_CASE, TRUE },
	{ "client_mIRC", L"*mIRC*", L"mIRC", IDI_MIRC, IRC_CASE, TRUE },
	{ "client_pIRC", L"*pIRC*", L"pIRC", IDI_PIRC, IRC_CASE, TRUE },
	{ "client_piorun", L"*piorun*", L"Piorun", IDI_PIORUN, IRC_CASE, TRUE },
	{ "client_psyBNC", L"*psyBNC*", L"psyBNC", IDI_PSYBNC, IRC_CASE, TRUE },
	{ "client_savIRC", L"*savIRC*", L"savIRC", IDI_SAVIRC, IRC_CASE, TRUE },
	{ "client_wmIRC", L"*wmIRC*", L"wmIRC", IDI_WMIRC, IRC_CASE, TRUE },
	{ "client_xBitch", L"*xBitch*", L"xBitch", IDI_XBITCH, IRC_CASE, TRUE },
	{ "client_xChat", L"*xChat*", L"xChat", IDI_XCHAT, IRC_CASE, TRUE },
	{ "client_zsIRC", L"*zsIRC*", L"zsIRC", IDI_ZSIRC, IRC_CASE, TRUE },
	{ "client_ZNC", L"*ZNC*", L"ZNC", IDI_ZNC, IRC_CASE, TRUE },

	{ "client_aMule", L"*aMule*", L"aMule", IDI_AMULE, IRC_CASE, TRUE },
	{ "client_eMuleMorphXT", L"eMule*MorphXT*", L"eMule MorphXT", IDI_EMULE_MORPHXT, IRC_CASE, TRUE },
	{ "client_eMuleNeo", L"eMule*Neo*", L"eMule Neo", IDI_EMULE_NEO, IRC_CASE, TRUE },
	{ "client_eMulePlus", L"|eMule*plus*|eMule*+*", L"eMule+", IDI_EMULE_PLUS, IRC_CASE, TRUE },
	{ "client_eMuleXtreme", L"eMule*Xtreme*", L"eMule Xtreme", IDI_EMULE_XTREME, IRC_CASE, TRUE },
	{ "client_eMule", L"*eMule*", L"eMule", IDI_EMULE, IRC_CASE, TRUE },

	{ "client_IRCUnknown", L"*IRC*", L"Unknown IRC client", IDI_IRC, IRC_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   WEATHER   ###########################################################################################################################
	//#########################################################################################################################################################################
	{ "client_accu", L"*accuweather*", L"AccuWeather", IDI_ACCU, WEATHER_CASE, TRUE },
	{ "client_gismeteo", L"*gismeteo*", L"GisMeteo", IDI_GISMETEO, WEATHER_CASE, TRUE },
	{ "client_intelli", L"*intellicast*", L"Intellicast", IDI_INTELLI, WEATHER_CASE, TRUE },
	{ "client_meteogid", L"|*meteo-gid*|*meteogid*", L"Meteo-Gid", IDI_METEOGID, WEATHER_CASE, TRUE },
	{ "client_meteonovosti", L"*meteonovosti*", L"Meteonovosti", IDI_METEONOVOSTI, WEATHER_CASE, TRUE },
	{ "client_noaa", L"*noaa*", L"NOAA Weather", IDI_NOAA, WEATHER_CASE, TRUE },
	{ "client_real", L"*realmeteo*", L"RealMeteo", IDI_REALMETEO, WEATHER_CASE, TRUE },
	{ "client_under", L"Weather Underground*", L"Weather Underground", IDI_UNDERGROUND, WEATHER_CASE, TRUE },
	{ "client_weatherxml", L"*WeatherXML*", L"WeatherXML", IDI_WEATHERXML, WEATHER_CASE, TRUE },
	{ "client_wetter", L"*wetter*", L"Wetter", IDI_WETTER, WEATHER_CASE, TRUE },
	{ "client_yweather", L"*Yahoo Weather*", L"Yahoo Weather", IDI_YWEATHER, WEATHER_CASE, TRUE },
	{ "client_weather_cn", L"*weather.com.cn*", L"Weather CN", IDI_WEATHER_CN, WEATHER_CASE, TRUE },
	{ "client_weather", L"*weather*", L"Weather", IDI_WEATHER, WEATHER_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   RSS   ###############################################################################################################################
	//#########################################################################################################################################################################
	{ "client_rss09x", L"*RSS*0.9*", L"RSS 0.9x", IDI_RSS09, RSS_CASE, TRUE },
	{ "client_rss2", L"*RSS*2.*", L"RSS 2", IDI_RSS2, RSS_CASE, TRUE },
	{ "client_rss1", L"*RSS*1.*", L"RSS 1", IDI_RSS1, RSS_CASE, TRUE },
	{ "client_atom3", L"*Atom*3*", L"Atom 3", IDI_ATOM3, RSS_CASE, TRUE },
	{ "client_atom1", L"*Atom*1*", L"Atom 1", IDI_ATOM1, RSS_CASE, TRUE },

	//#########################################################################################################################################################################
	//#################################   GADU-GADU   #########################################################################################################################
	//#########################################################################################################################################################################
	{ "client_GG", L"|Gadu-Gadu*|GG*", LPGENW("Gadu-Gadu client"), IDI_GG, GG_CASE },

	//#########################################################################################################################################################################
	//#################################   Facebook   ##########################################################################################################################
	//#########################################################################################################################################################################
	{ "client_Facebook_other", L"*Facebook*other*", L"Facebook (other)", IDI_FACEBOOK_OTHER, FACEBOOK_CASE },
	{ "client_Facebook_app", L"*Facebook*App*", L"Facebook App", IDI_FACEBOOK_APP, FACEBOOK_CASE },
	{ "client_Facebook_mess", L"*Facebook*Messenger*", L"Facebook Messenger", IDI_FACEBOOK_MESSENGER, FACEBOOK_CASE },
	{ "client_Facebook", L"*Facebook*", L"Facebook", IDI_FACEBOOK, FACEBOOK_CASE },

	//#########################################################################################################################################################################
	//#################################   VKontakte   #########################################################################################################################
	//#########################################################################################################################################################################
	{ "client_VK", L"|*VKontakte*|*vk.com*", L"VKontakte", IDI_VK, VK_CASE },
	{ "client_VK_Kate", L"*Kate*Mobile*", L"Kate Mobile", IDI_VK_KATE, VK_CASE },
	{ "client_VK_Messenger", L"VK Messenger*", L"VK Messenger", IDI_VK_MESSENGER, VK_CASE },
	{ "client_VK_Phoenix_Full", L"Phoenix*Full*", L"Phoenix Full", IDI_VK_PHOENIX_FULL, VK_CASE },
	{ "client_VK_Phoenix_Lite", L"Phoenix*Lite*", L"Phoenix Lite", IDI_VK_PHOENIX_LITE, VK_CASE },

	//#########################################################################################################################################################################
	//#################################   OTHER CLIENTS   #####################################################################################################################
	//#########################################################################################################################################################################
	{ "client_Android", L"Android*", L"Android", IDI_ANDROID, MULTI_CASE, TRUE },

	{ "client_Pidgin", L"|*Pidgin*|*libpurple*|Purple*", L"Pidgin (libpurple)", IDI_PIDGIN, MULTI_CASE },

	{ "client_Python", L"|*Python*|Py*|*ταλιςμαη*", LPGENW("Python-based clients"), IDI_PYTHON, MULTI_CASE },

	{ "client_Jabber", L"*Jabber*client*", LPGENW("Jabber client"), IDI_JABBER, JABBER_CASE, TRUE },

	{ "client_XMPP", L"|*XMPP*|Mrim*|*DRQZ00fz5WPn1gH+*", LPGENW("XMPP client"), IDI_XMPP, JABBER_CASE },

	{ "client_Hangouts", L"messaging-*", L"Google+ Hangouts", IDI_HANGOUTS, OTHER_PROTOS_CASE, TRUE },

	{ "client_Twitter", L"*Twitter*", L"Twitter", IDI_TWITTER, OTHER_PROTOS_CASE, TRUE },

	{ "client_Skype", L"*Skype**", L"Skype", IDI_SKYPE, OTHER_PROTOS_CASE },

	{ "client_Steam", L"*Steam*", L"Steam", IDI_STEAM, OTHER_PROTOS_CASE },

	//#########################################################################################################################################################################
	//#################################   UNDEFINED CLIENTS   #################################################################################################################
	//#########################################################################################################################################################################

	{ "client_Notfound", L"Notfound", LPGENW("Client not found"), IDI_NOTFOUND, OTHERS_CASE, TRUE },
	{ "client_Unknown", L"|*Unknown*|...", LPGENW("Unknown client"), IDI_UNKNOWN, OTHERS_CASE, TRUE },
	{ "client_Undetected", L"?*", LPGENW("Undetected client"), IDI_UNDETECTED, OTHERS_CASE, TRUE },

};

int DEFAULT_KN_FP_MASK_COUNT = _countof(def_kn_fp_mask);

//#########################################################################################################################################################################
//#################################   OVERLAYS LAYER #1   #################################################################################################################
//#########################################################################################################################################################################

KN_FP_MASK def_kn_fp_overlays_mask[] =
{// {"Client_IconName",         L"|^*Mask*|*names*",                         L"Icon caption",                 IDI_RESOURCE_ID,         CLIENT_CASE,    OVERLAY?    },
	//#########################################################################################################################################################################
	//#################################   MIRANDA PACKS OVERLAYS   ############################################################################################################
	//#########################################################################################################################################################################
	{ "client_AF_pack", L"*AF*Pack*", L"AF", IDI_MIRANDA_AF, MIRANDA_PACKS_CASE },
	{ "client_AlfaMaR_pack", L"*AlfaMaR*", L"AlfaMaR", IDI_MIRANDA_ALFAMAR, MIRANDA_PACKS_CASE },
	{ "client_Amatory_pack", L"*Amatory*", L"Amatory", IDI_MIRANDA_AMATORY, MIRANDA_PACKS_CASE },
	{ "client_BRI_pack", L"*bri*edition*", L"Bri edition", IDI_MIRANDA_BRI, MIRANDA_PACKS_CASE },
	{ "client_Devil_pack", L"*6.6.6*", L"Devil Suite", IDI_MIRANDA_DEVIL, MIRANDA_PACKS_CASE },
	{ "client_E33_pack", L"*[E33*]*", L"E33", IDI_MIRANDA_E33, MIRANDA_PACKS_CASE },
	{ "client_FR_pack", L"*FR*Pack*", L"Miranda FR", IDI_MIRANDA_FR, MIRANDA_PACKS_CASE },
	{ "client_Faith_pack", L"*Faith*Pack*", L"FaithPack", IDI_MIRANDA_FAITH, MIRANDA_PACKS_CASE },
	{ "client_Final_pack", L"*[Final*Pack]*", L"Final pack", IDI_MIRANDA_FINAL, MIRANDA_PACKS_CASE },
	{ "client_Freize_pack", L"*Freize*", L"Freize", IDI_MIRANDA_FREIZE, MIRANDA_PACKS_CASE },
	{ "client_Ghost_pack", L"*Ghost's*", L"Ghost's pack", IDI_MIRANDA_GHOST, MIRANDA_PACKS_CASE },
	{ "client_HotCoffee_pack", L"*HotCoffee*", L"HotCoffee", IDI_MIRANDA_CAPPUCCINO, MIRANDA_PACKS_CASE },
	{ "client_HierOS_pack", L"*HierOS*", L"HierOS", IDI_MIRANDA_HIEROS, MIRANDA_PACKS_CASE },
	{ "client_ICE_pack", L"|*miranda*[ice*]|*induction*", L"iCE / Induction", IDI_MIRANDA_INDUCTION, MIRANDA_PACKS_CASE },
	{ "client_KDL_pack", L"|*KDL*|*КДЛ*", L"KDL", IDI_MIRANDA_KDL, MIRANDA_PACKS_CASE },
	{ "client_Kolich_pack", L"*Kolich*", L"Kolich", IDI_MIRANDA_KOLICH, MIRANDA_PACKS_CASE },
	{ "client_Kuzzman_pack", L"*kuzzman*", L"Kuzzman", IDI_MIRANDA_KUZZMAN, MIRANDA_PACKS_CASE },
	{ "client_Lenin_pack", L"*[Lenin*]*", L"Lenin pack", IDI_MIRANDA_LENINPACK, MIRANDA_PACKS_CASE },
	{ "client_Lestat_pack", L"*[Lpack*]*", L"Lestat pack", IDI_MIRANDA_LESTAT, MIRANDA_PACKS_CASE },
	{ "client_LexSys_pack", L"|*Miranda*LS*|*LexSys*", L"LexSys", IDI_MIRANDA_LEXSYS, MIRANDA_PACKS_CASE },
	{ "client_MD_pack", L"*MDpack*", L"MDpack", IDI_MIRANDA_MD, MIRANDA_PACKS_CASE },
	{ "client_Mataes_pack", L"*Mataes*", L"Mataes pack", IDI_MIRANDA_MATAES, MIRANDA_PACKS_CASE },
	{ "client_Mir_ME_pack", L"*[Miranda*ME]*", L"Miranda ME", IDI_MIRANDA_ME, MIRANDA_PACKS_CASE },
	{ "client_Native_pack", L"*Native*", L"Native", IDI_MIRANDA_NATIVE, MIRANDA_PACKS_CASE },
	{ "client_New_Style_pack", L"*New*Style*", L"New Style", IDI_MIRANDA_NEW_STYLE, MIRANDA_PACKS_CASE },
	{ "client_Pilot_pack", L"*Pilot*", L"Pilot", IDI_MIRANDA_PILOT, MIRANDA_PACKS_CASE },
	{ "client_Razunter_pack", L"*Razunter*", L"Razunter's Pk", IDI_MIRANDA_RAZUNTER, MIRANDA_PACKS_CASE },
	{ "client_Robyer_pack", L"*Robyer*Pack*", L"Robyer pack", IDI_MIRANDA_ROBYER, MIRANDA_PACKS_CASE },
	{ "client_SSS_pack", L"*sss*pack*", L"SSS build", IDI_MIRANDA_SSS_MOD, MIRANDA_PACKS_CASE },
	{ "client_Se7ven_pack", L"|^*sss*|*[S7*pack]*|*[*S7*]*", L"Se7ven", IDI_MIRANDA_SE7VEN, MIRANDA_PACKS_CASE },
	{ "client_SpellhowleR_pack", L"*Spellhowler*", L"xSpellhowleRx pack", IDI_MIRANDA_SPELLHOWLER, MIRANDA_PACKS_CASE },
	{ "client_Stalker_pack", L"*Stalker*", L"Stalker", IDI_MIRANDA_STALKER, MIRANDA_PACKS_CASE },
	{ "client_Tweety_pack", L"*tweety*", L"Tweety", IDI_MIRANDA_TWEETY, MIRANDA_PACKS_CASE },
	{ "client_Umedon_pack", L"*Miranda*Umedon*", L"Umedon", IDI_MIRANDA_UMEDON, MIRANDA_PACKS_CASE },
	{ "client_ValeraVi_pack", L"*Valera*Vi*", L"ValeraVi", IDI_MIRANDA_VALERAVI, MIRANDA_PACKS_CASE },
	{ "client_Watcher_pack", L"*Watcher*", L"Watcher pack", IDI_MIRANDA_WATCHER, MIRANDA_PACKS_CASE },
	{ "client_YAOL_pack", L"*yaol*", L"YAOL", IDI_MIRANDA_YAOL, MIRANDA_PACKS_CASE },
	{ "client_dar_veter_pack", L"*Dar*veter*", L"Dar_veter pack", IDI_MIRANDA_DAR, MIRANDA_PACKS_CASE },
	{ "client_dmikos_pack", L"*dmikos*", L"Dmikos", IDI_MIRANDA_DMIKOS, MIRANDA_PACKS_CASE },
	{ "client_zeleboba_pack", L"*zeleboba*", L"zeleboba's", IDI_MIRANDA_ZELEBOBA, MIRANDA_PACKS_CASE },

	//#########################################################################################################################################################################
	//#################################   PROTO OVERLAYS   ####################################################################################################################
	//#########################################################################################################################################################################
	{ "client_ICQ_overlay", L"|^ICQ|^ICQ*|*ICQ*", LPGENW("ICQ overlay"), IDI_ICQ_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_IRC_overlay", L"|^IRC*|Miranda*IRC*", LPGENW("IRC overlay"), IDI_IRC_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_JGmail_overlay", L"*JGmail*", LPGENW("JGmail overlay"), IDI_GMAIL_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_JGTalk_overlay", L"*JGTalk*", LPGENW("JGTalk overlay"), IDI_JGTALK_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_Jabber_overlay", L"|^jabber*|Miranda*Jabber*|py*jabb*", LPGENW("Jabber overlay"), IDI_JABBER_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_VK_overlay", L"|Miranda*VKontakte*", LPGENW("VK overlay"), IDI_VK_OVERLAY, OVERLAYS_PROTO_CASE },
	{ "client_Skype_overlay", L"|Miranda*Skype*", LPGENW("Skype overlay"), IDI_SKYPE_OVERLAY, OVERLAYS_PROTO_CASE },

	//#########################################################################################################################################################################
	//#################################   CLIENT VERSION OVERLAYS   ###########################################################################################################
	//#########################################################################################################################################################################
	{ "client_ICQ8_over", L"ICQ*8*", LPGENW("ICQ v8.x overlay"), IDI_ICQ8_OVERLAY, ICQ_CASE },

	{ "client_GG_11", L"|Gadu-Gadu*11*|GG*11*", LPGENW("Gadu-Gadu v11 client"), IDI_GG11_OVERLAY, GG_CASE },
	{ "client_GG_10", L"|Gadu-Gadu*10*|GG*10", LPGENW("Gadu-Gadu v10 client"), IDI_GG10_OVERLAY, GG_CASE },
	{ "client_GG_9", L"|Gadu-Gadu*9*|GG*9*", LPGENW("Gadu-Gadu v9 client"), IDI_GG9_OVERLAY, GG_CASE },
	{ "client_GG_8", L"|Gadu-Gadu*8*|GG*8*", LPGENW("Gadu-Gadu v8 client"), IDI_GG8_OVERLAY, GG_CASE },

	//#########################################################################################################################################################################
	//#################################   PLATFORM OVERLAYS   #################################################################################################################
	//#########################################################################################################################################################################
	{ "client_on_Win32", L"|*Win*|* WM *|wmagent*|*Vista*", LPGENW("Windows overlay"), IDI_PLATFORM_WIN, OVERLAYS_PLATFORM_CASE },
	{ "client_on_iOS", L"|*ipad*|*iphone*|*iOS*", LPGENW("iOS overlay (iPhone/iPad)"), IDI_PLATFORM_IOS, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Mac", L"|^*smack*|* Mac *|*mac*|*OSX*", LPGENW("MacOS overlay"), IDI_PLATFORM_MAC, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Linux", L"*Linux*", LPGENW("Linux overlay"), IDI_PLATFORM_LINUX, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Flash", L"|*Flash*|*Web*ICQ*", LPGENW("Flash overlay"), IDI_PLATFORM_FLASH, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Java", L"|*Java*|jagent*|ICQ2Go!*", LPGENW("Java overlay"), IDI_PLATFORM_JAVA, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Symbian", L"|*Symbian*|sagent*", LPGENW("Symbian overlay"), IDI_PLATFORM_SYMBIAN, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Amiga", L"*Amiga*", LPGENW("Amiga overlay"), IDI_PLATFORM_AMIGA, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Android", L"|*Android*|*(android)*", LPGENW("Android overlay"), IDI_PLATFORM_ANDROID, OVERLAYS_PLATFORM_CASE },
	{ "client_on_Website", L"|*(website)*|*(Web)*", LPGENW("Website overlay"), IDI_PLATFORM_WEBSITE, OVERLAYS_PLATFORM_CASE },
	{ "client_on_WinPhone", L"|*(wphone)*|*(WP)*", LPGENW("Windows Phone overlay"), IDI_PLATFORM_WINPHONE, OVERLAYS_PLATFORM_CASE },
	{ "client_on_mobile", L"*(mobile)*", LPGENW("Mobile overlay"), IDI_PLATFORM_MOBILE, OVERLAYS_PLATFORM_CASE },
};

int DEFAULT_KN_FP_OVERLAYS_COUNT = _countof(def_kn_fp_overlays_mask);

//#########################################################################################################################################################################
//#########################################################################################################################################################################
//#################################   OVERLAYS LAYER #2   #################################################################################################################
//#########################################################################################################################################################################
//#########################################################################################################################################################################

KN_FP_MASK def_kn_fp_overlays2_mask[] =
{// {"Client_IconName",         L"|^*Mask*|*names*",                         L"Icon caption",                 IDI_RESOURCE_ID,         CLIENT_CASE,    OVERLAY?    },
	{ "client_debug_overlay", L"|*[*debug*]*|*test*|*тест*", LPGENW("debug overlay"), IDI_DEBUG_OVERLAY, OVERLAYS_RESOURCE_CASE },
	{ "client_office_overlay", L"|*[*office*]*|*[*офис*]*", LPGENW("office overlay"), IDI_OFFICE_OVERLAY, OVERLAYS_RESOURCE_CASE },
	{ "client_mobile_overlay", L"|*[*mobile*]*|*[*pda*]*", LPGENW("mobile overlay"), IDI_MOBILE_OVERLAY, OVERLAYS_RESOURCE_CASE },
	{ "client_home_overlay", L"|*[*home*]*|*[*дом*]*|*[*хоме*]*", LPGENW("home overlay"), IDI_HOME_OVERLAY, OVERLAYS_RESOURCE_CASE },

	{ "client_work_overlay", L"|*[*work*]*|*wrk*"
	L"|*[*работа*]*|*ворк*", LPGENW("work overlay"), IDI_WORK_OVERLAY, OVERLAYS_RESOURCE_CASE },

	{ "client_note_overlay", L"|*[*note*]*|*[*laptop*]*"
	L"|*[*нетбу*]*|*[*ноут*]*|*[*ноте*]*"
	L"|*[*кирпич*]*|*[*portable*]*"
	L"|*[*flash*]*|*[*usb*]*", LPGENW("notebook overlay"), IDI_NOTEBOOK_OVERLAY, OVERLAYS_RESOURCE_CASE },

	//  {"client_MirNG_09_over",    L"*Miranda*NG*\?.\?\?.9.*",                  L"Miranda NG v0.9 #2 overlay",    IDI_MIRANDA_NG_V9,       MIRANDA_VERSION_CASE        },
	//  {"client_MirNG_08_over",    L"*Miranda*NG*\?.\?\?.8.*",                  L"Miranda NG v0.8 #2 overlay",    IDI_MIRANDA_NG_V8,       MIRANDA_VERSION_CASE        },
	//  {"client_MirNG_07_over",    L"*Miranda*NG*\?.\?\?.7.*",                  L"Miranda NG v0.7 #2 overlay",    IDI_MIRANDA_NG_V7,       MIRANDA_VERSION_CASE        },

	{ "client_MirNG_06_over", L"*Miranda*NG*\?.\?\?.6.*", LPGENW("Miranda NG v0.6 #2 overlay"), IDI_MIRANDA_NG_V6, MIRANDA_VERSION_CASE },
	{ "client_MirNG_05_over", L"*Miranda*NG*\?.\?\?.5.*", LPGENW("Miranda NG v0.5 #2 overlay"), IDI_MIRANDA_NG_V5, MIRANDA_VERSION_CASE },
	{ "client_MirNG_04_over", L"*Miranda*NG*\?.\?\?.4.*", LPGENW("Miranda NG v0.4 #2 overlay"), IDI_MIRANDA_NG_V4, MIRANDA_VERSION_CASE },
	{ "client_MirNG_03_over", L"*Miranda*NG*\?.\?\?.3.*", LPGENW("Miranda NG v0.3 #2 overlay"), IDI_MIRANDA_NG_V3, MIRANDA_VERSION_CASE },
	{ "client_MirNG_02_over", L"*Miranda*NG*\?.\?\?.2.*", LPGENW("Miranda NG v0.2 #2 overlay"), IDI_MIRANDA_NG_V2, MIRANDA_VERSION_CASE },
	{ "client_MirNG_01_over", L"*Miranda*NG*\?.\?\?.1.*", LPGENW("Miranda NG v0.1 #2 overlay"), IDI_MIRANDA_NG_V1, MIRANDA_VERSION_CASE },

	{ "client_MirIM_010_over", L"*Miranda*0.10.*", LPGENW("Miranda IM v0.10 #2 overlay"), IDI_MIRANDA_IM_V10, MIRANDA_VERSION_CASE },
	{ "client_MirIM_09_over", L"*Miranda*0.9.*", LPGENW("Miranda IM v0.9 #2 overlay"), IDI_MIRANDA_IM_V9, MIRANDA_VERSION_CASE },
	{ "client_MirIM_08_over", L"*Miranda*0.8.*", LPGENW("Miranda IM v0.8 #2 overlay"), IDI_MIRANDA_IM_V8, MIRANDA_VERSION_CASE },
	{ "client_MirIM_07_over", L"*Miranda*0.7.*", LPGENW("Miranda IM v0.7 #2 overlay"), IDI_MIRANDA_IM_V7, MIRANDA_VERSION_CASE },
	{ "client_MirIM_06_over", L"*Miranda*0.6.*", LPGENW("Miranda IM v0.6 #2 overlay"), IDI_MIRANDA_IM_V6, MIRANDA_VERSION_CASE },
	{ "client_MirIM_05_over", L"*Miranda*0.5.*", LPGENW("Miranda IM v0.5 #2 overlay"), IDI_MIRANDA_IM_V5, MIRANDA_VERSION_CASE },
	{ "client_MirIM_04_over", L"*Miranda*0.4.*", LPGENW("Miranda IM v0.4 #2 overlay"), IDI_MIRANDA_IM_V4, MIRANDA_VERSION_CASE },
};

int DEFAULT_KN_FP_OVERLAYS2_COUNT = _countof(def_kn_fp_overlays2_mask);

//#########################################################################################################################################################################
//#########################################################################################################################################################################
//##############################   OVERLAYS LAYER #3   ####################################################################################################################
//#########################################################################################################################################################################
KN_FP_MASK def_kn_fp_overlays3_mask[] =
{// {"Client_IconName",         L"|^*Mask*|*names*",                         L"Icon caption",                 IDI_RESOURCE_ID,         CLIENT_CASE,    OVERLAY?    },
	{ "client_platform_x64", L"|*x64*|*64*bit*", LPGENW("x64 overlay"), IDI_PLATFORM_X64, OVERLAYS_PLATFORM_CASE },
	{ "client_platform_x32", L"|*x32*|*32*bit*|*x86*", LPGENW("x32 overlay"), IDI_PLATFORM_X32, OVERLAYS_PLATFORM_CASE },

	{ "client_Unicode", L"*unicode*", LPGENW("Unicode overlay"), IDI_UNICODE_CLIENT, OVERLAYS_UNICODE_CASE },
};

int DEFAULT_KN_FP_OVERLAYS3_COUNT = _countof(def_kn_fp_overlays3_mask);

//#########################################################################################################################################################################
//#########################################################################################################################################################################
//##############################   OVERLAYS LAYER #4   ####################################################################################################################
//#########################################################################################################################################################################
KN_FP_MASK def_kn_fp_overlays4_mask[] =
{// {"Client_IconName",         L"|^*Mask*|*names*",                         L"Icon caption",                 IDI_RESOURCE_ID,         CLIENT_CASE,    OVERLAY?    },
	{ "client_NewGPG_over", L"*New*GPG*", LPGENW("NewGPG overlay"), IDI_NEWGPG_OVERLAY, OVERLAYS_SECURITY_CASE },
	{ "client_MirOTR_over", L"*Mir*OTR*", LPGENW("MirOTR overlay"), IDI_MIROTR_OVERLAY, OVERLAYS_SECURITY_CASE },
	{ "client_SecureIM_over", L"*Secure*IM*", LPGENW("SecureIM overlay"), IDI_SECUREIM_OVERLAY, OVERLAYS_SECURITY_CASE },
};

int DEFAULT_KN_FP_OVERLAYS4_COUNT = _countof(def_kn_fp_overlays4_mask);

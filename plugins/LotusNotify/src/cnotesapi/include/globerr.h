
#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(P128)
#endif

/*********************************************************************/
/*                                                                   */
/* Licensed Materials - Property of IBM                              */
/*                                                                   */
/* L-GHUS-5VMPGW, L-GHUS-5S3PEE                                      */
/* (C) Copyright IBM Corp. 1989, 2005  All Rights Reserved           */
/*                                                                   */
/* US Government Users Restricted Rights - Use, duplication or       */
/* disclosure restricted by GSA ADP Schedule Contract with           */
/* IBM Corp.                                                         */
/*                                                                   */
/*********************************************************************/


#ifndef GLOBAL_ERROR_DEFS
#define GLOBAL_ERROR_DEFS

/*	Define error code packages */	


#ifdef NOERROR
#undef NOERROR
#endif
#define NOERROR 0

#define	PKG_MAIN		0x0000	/* Codes are limited to 0-63 */
#define	PKG_CCONSOLE	0x0040	/* Codes are limited to 0-31 */
#define	PKG_MAILMISC3	0x0060	/* Mailmisc3 codes starting from 0-31 */
#define	PKG_MINDER		0x0080	/* Minder limit to 0 - 95 */
#define PKG_SERVER3		0x00E0	/* Codes are limited to 1 - 31 */
#define PKG_OS			0x0100
#define PKG_NSF			0x0200
#define PKG_NIF			0x0300	/* NIF codes are limited to 0 - 127 */
#define PKG_NSF2		0x0380	/* More NSF codes - limited to 0-127*/
#define PKG_MISC		0x0400	/* Codes are limited to 0 - 191 */
#define PKG_SERVER2  	0x04C0	/* Codes are limited to 0 - 47 */
#define PKG_THUNK		0x04F0	/* Codes are limited to 0 - 15 */
#define PKG_FORMULA		0x0500	/* FORMULA codes are limited to 0-127 */
#define PKG_NSF8		0x0580	/* AVAILABLE limit to 0 - 127 */
#define	PKG_ODS			0x0600	/* ODS codes, limited to 0-47 */
#define	PKG_LSXUI4		0x0630	/* LotusScript Front-end classes, 0 - 47 */
#define	PKG_AGENTS2		0x0660	/* for agents, limited to 0-31 */
#define PKG_SCHUI		0x0680	/* Schedule UI codes are limited to 0 - 31 */
#define PKG_BERT		0x06A0	/* BERT codes are limited to 0 - 31 */
#define PKG_PLUGINS		0x06C0	/* plugin codes are limited to 0 - 15 */

#define PKG_IMAIL		0x06D0	/* IMAIL Client 0 - 47 */

#define	PKG_PKIX		0x0700	/* codes 0 - 95 New CA/PKIX error codes - in pkcs12 dir */
#define PKG_GT			0x0760	/* GraphTrends error codes 0 - 31 */
#define PKG_NIF2		0x0780	/* Second nif error package 0-127 */
#define PKG_CLIENT		0x0800  /* Client codes limited to 0 - 47 */
#define PKG_LSXUI3		0x0830  /* more LSXUI ERROR codes limited to 0 - 15 */
#define PKG_APC			0x0840  /* APC codes limited to 0 - 31 */
#define PKG_DBMISC2		0x0860	/* DBMISC2 codes limited to 0 - 15 */
#define PKG_TASK_LOADER	0x0870	/* Task Loader package */
#define PKG_MAILMISC4	0x0880	/* Mail Misc  limit to 0 - 127 */
#define PKG_SERVER  	0x0900
#define PKG_NETWORK 	0x0A00		/* If you split this range, fix IS_PKG_NETWORK below */
#define	PKG_WMISC_ERR	0x0B00		/* Codes are limited to 0 - 63 */
#define PKG_ACTION		0x0B80		/* Codes are limited to 0 - 63 */
#define PKG_ACTIONPANE	0x0BC0		/* Codes are limited to 0 - 63 */
#define	PKG_EDIT_ERR	0x0C00
#define PKG_VIEW_ERR	0x0D00
#define PKG_MAIL		0x0E00		/* MAIL errors are limited to 0-63 */
#define PKG_BSAFE3		0x0E40	    /* Limited to 0-31 */
#define PKG_BSAFE4		0x0E60	    /* Limited to 0-31 */
#define PKG_CONV		0x0E80		/* CONV errors are limited to 0 - 31 */
#define PKG_NSF4		0x0EA0		/* NSF4 errors are limited to 0 - 31 */
#define PKG_FRAMDES		0x0EC0		/* Frame design errors limited to 0 - 31 */
#define PKG_NETWORK3	0x0EE0		/* AVAILABLE codes are limited to 0 - 31 */
#define PKG_FT			0x0F00		/* FT errors are limited to 0 - 63 */
#define PKG_BSAFE5		0x0F40	    /* Limited to 0-15 */
#define PKG_DBMISC		0x0F50		/* 0 - 47 */
#define PKG_NETWORK2	0x0F80		/* 0 - 127, if this changes, fix IS_PKG_NETWORK below */
#define	PKG_DEBUG		0x1000		/* Debug strings, like for DDE, limited to 0 - 79 */
#define PKG_SSL			0x1040		/* SSL errors 0 - 15 */
#define	PKG_SERVER4 	0x1050		/* SERVER4, limited to 0 - 15 */
#define	PKG_BOOKMARK	0x1060		/* BOOKMARK, limited to 0 - 15 */
#define	PKG_NSF5		0x1070		/* NSF5, limited to 0 - 15 */
#define PKG_NEM_ERR		0x1080		/* NEM, limited to 0 - 127 */
#define PKG_ROUTER		0x1100		/* Errors returned by ROUTERL, 0 - 79 */
#define PKG_MAILMAN		0x1150		/* Errors returned by MAILMAN, 0 - 95 */
#define PKG_LSBE		0x11b0		/* LSXBE errors, 0 - 47 */
#define PKG_LSDO		0x11e0		/* LSDOE errors, 0 - 15 */
#define PKG_LSXDB2		0x11f0		/* LSXDB2 errors, 0 - 15 */
#define PKG_REG2		0x1200		/* REG2 errors, 0 - 127 */
#define PKG_LSIDE		0x1280		/* ide specific errors 0 - 95 */
#define PKG_HTML		0x12D0		/* html parser errors, 0 - 31 */ 
#define PKG_SERVER5		0x12F0		/* PKG_SERVER5 limit to 0 - 15 */
#define PKG_LOG			0x1300
#define PKG_NSF3		0x1380		/* More NSF codes - limited to 0-127*/
#define	PKG_XPC			0x1400
#define PKG_EVENT		0x1500		/* Event codes starting from 0-47 */
#define PKG_FIDE		0x1530		/* Event codes starting from 0-31 */
#define PKG_NETWORK4	0x1550		/* Limited to 0 - 15, if this changes, fix IS_PKG_NETWORK below */
#define PKG_MAILMISC2	0x1560		/* Mailmisc2 codes starting from 0-31 */
#define PKG_BCASE		0x1580		/* Briefcase codes are limited to 0 - 95 */
#define PKG_SECURE2		0x15E0		/* Secure2 limit to 0 - 15 */
#define PKG_BSAFE6		0x15F0		/* Bsafe codes are limited to 0 - 15 */
#define PKG_REPL		0x1600		/* Errors returned by REPLSUB library, limited to 0 - 199 */
#define PKG_ADMIN_ERR2	0x16C8		/* PKG_ADMIN_ERR2 limited to 0 - 55 */
#define	PKG_BSAFE		0x1700		/* BSAFE codes are limited to 0 - 151 */
#define PKG_SERVER7		0x1798		/* PKG_SERVER7 limit to 0 - 103 */
#define	PKG_DESK_ERR	0x1800
#define	PKG_SECURE		0x1900
#define	PKG_AGENT		0x1A00		/* AGENT codes are limited to 0-63 */
#define	PKG_CCONSOLE2	0x1A50		/* CCONSOLE2 codes are limited to 0-15 */
#define PKG_PLAT_STAT_ERR 0x1A60	/* Platform Statistics error package */
#define	PKG_AGENT1		0x1A70		/* AGENT1 codes are limited to 0-15 */
#define	PKG_AGENTS3		0x1A80		/* AGENTS3 codes are limited to 0-47 */
#define	PKG_AGENT2		0x1AB0		/* AGENT2 codes are limited to 0-79 */
#define PKG_XML			0x1B00		/* XML codes: limit to 0 - 255 */
#define PKG_NETDRV		0x1C00		/* If this PKG space is split, fix IS_PKG_NETWORK below */
#define PKG_IMPORT		0x1D00      /* Used for all imports - see IMPKG_xxx below */
#define PKG_EXPORT		0x1E00      /* Used for all exports - see EXPKG_xxx below */
#define PKG_LSXUI2		0x1F00		/* LSXUI2 Codes are limited to 0 - 63 */
#define PKG_REG			0x2000

/* Following 8 groups used for native OS error codes, mapped by OSMapError
	and translated by OSLoadString.  These give better feedback in certain
	cases, but should NEVER be interpreted if STS_REMOTE bit is set,
	indicating that another OS on server might have generated the error! */

#define PKG_NATIVE_FIRST 0x2100
#define PKG_NATIVE_LAST  0x28FF

#ifdef OS2
#define	PKG_OS2_BASE	0x2100		/* + status - 0x0000 (BSEERR base) */
#define	PKG_OS2_BASE2	0x2200		/*             (BSEERR continued) */
#define	PKG_OS2_LANMAN	0x2300		/* + status - 0x0834=2100 (LANMAN base) */
#define	PKG_OS2_LANMAN2	0x2400		/*             (LANMAN continued) */
#define	PKG_OS2_PMWIN	0x2500		/* + status - 0x1000 (PMWIN base) */
#define	PKG_OS2_PMGPI	0x2600		/* + status - 0x2000 (PMGPI base) */
#define	PKG_OS2_PMGPI2	0x2700		/*             (GPI continued) */
#define	PKG_OS2_PMSPL	0x2800		/* + status - 0x4000 (PMSPL base) */
#endif

#define PKG_NSE			0x2900		/* Network script engine */
#define PKG_NSF6		0x29B0		/* NSF6, limited to 0 - 15 */
#define PKG_PERFSTAT	0x29C0		/* PERFSTAT error codes 0-31 */
#define PKG_MISC2	0x29E0		/* MISC2 limit to 0 - 31 */
#define PKG_NETDRVLCL	0x2A00		/* Used for all Network Drivers, if PKG space is split, fix IS_PKG_NETWORK below */
#define	PKG_NTI			0x2B00		/* Used for NTI and its new Net drivers */

#define PKG_VIEWMAP		0x2C00		/* for ViewMap */
#define PKG_BSAFE2		0x2CF0		/* for BSAFE x509 routines */

#define PKG_REPL2		0x2D00		/* for remote debug 0-127 */
#define PKG_RDBGERR		0x2D80		/* for remote debug err messages 0-127 */

#define PKG_AGENTS		0x2E00		/* for agents, limit to 0 - 127 */
#define PKG_DESK_ERR2	0x2E80		/* more Desk limit to 0 - 127 */
#define PKG_LSCRIPT		0x2F00 		/* LotusScript Interface, limit 0-127 */
#define PKG_LSXUI		0x2F80		/* LotusScript Front-end classes, 0-127 */
#define PKG_DSGN		0x3000		/* Database design package, 0-63 */
#define PKG_SERVER6		0x3040		/* PKG_SERVER6 error codes 0 - 63 */
#define PKG_ADMIN_ERR	0x3080		/* Admin facility, 0 - 127 */
#define	PKG_DBD			0x3100		/* database driver error codes, 0 - 199 */
#define PKG_COMPILER7	0x31B8		/* More COMPILER codes, limited to 0-15 */
#define PKG_NEWS		0x31C8		/* News classes, 0 - 15 */
#define PKG_IMAIL_EXT	0x31D8		/* IMail database extensions 0 - 23 */
#define PKG_DAEMON		0x31F0		/* CDaemon codes, 0 - 15 */
#define PKG_COMPILER8	0x3200		/* More COMPILER codes, limit to 0 - 255 */
#define PKG_ADDIN		0x3300		/* For use by mail gateways, etc. */
#define PKG_EDIT_ERR2	0x3400		/* need additional block for edit */
#define PKG_LSCRIPT2	0x3500		/* Lotusscript interface, limit 0 - 127 */
#define	PKG_ADMIN_ERR3	0x3580		/* PKG_ADMIN_ERR3 limited to 0 - 55 */																
#define	PKG_DSGN2		0x35C0		/* PKG_DSGN2 limit to 0 - 63 */	
#define PKG_GRMISC      0x3600      /* Graphics Library 0-31 */
#define PKG_VIMSMI		0x3620		/* VIM and SMI block codes 0-10 */
#define PKG_WEB         0x3640      /* InterNotes client extensions 0-191 */
#define	PKG_ADDIN2		0x3700		/* For extensions to PKG_ADDIN */
#define PKG_NSF9		0x3800		/* NSF9, limited to 0 - 255 */
#define PKG_DESK_ERR4	0x3900		/* DESK error, limit to 0 - 127 */
#define PKG_ORB			0x3980		/* ORB, limit to 0 - 63 */
#define PKG_LSXUI5		0x39C0		/* LSXUI5 Codes are limited to 0 - 63 */
#define PKG_HTTP		0x3A00		/* Web Server. limit to 0 - 199 */
#define PKG_POP3 		0x3AC8		/* POP3.  0 - 19 */
#define PKG_MAILMISC	0x3ADC		/* Mailmisc. 0 - 3 */
#define PKG_SMTP		0x3AE0		/* SMTP.  0 - 7 */
#define PKG_POP3C		0x3AE8		/* POP3 CLIENT.  0-2 */
#define PKG_SMTPC		0x3AEB		/* SMTP.  0 - 6 */
#define PKG_MAILMISC1	0x3AF2		/* MAILMISC.  0 - 13 */
#define PKG_DB2NSF		0x3B00		/* DB2NSF stuff */
#define PKG_SMARTI		0x3C00		/* To make smart icon res unique */
#define PKG_TOOLBAR		PKG_SMARTI  /* R6 Reuse smarticon pkg for new toolbars */
#define PKG_OLE_ERR		0x3D00		/* OLE error codes 0-63*/
									/* Note, PKG_OLE_ERR+0-53 were used in V3 */
#define PKG_MISC3		0x3D40		/* PKG_MISC3 codes are limited to 0-31 */
#define PKG_NSF7		0x3D60		/* PKG_NSF7 codes are limited to 0-31 */
#define PKG_NETWORK5	0x3D80		/* Limited to 0-31, if this area changes, fix IS_PKG_NETWORK below */
#define PKG_DESK_ERR3	0x3DA0		/* PKG_DESK_ERR3 are limited to 0-63 */
#define PKG_EVENT2		0x3DE0		/* Event codes starting from 0-31 */
/* #define PKG_OLE_CMD		0x3E00	 OLE Api Command Descriptions - OBSOLETE in V5 */
									/* Note, PKG_OLE_CMD+0-56 were used in V3 */
#define PKG_JAVAWRAP	0x3E00		/* javawrap error codes 0-63 */
#define PKG_ASSISTANT_ERR 0x3E40	/* Assistant codes limit to 0 - 63 */
#define PKG_JSWRAP		0x3E80		/* javascript wrap error codes 0-63 */
#define PKG_PRINT_ERR	0x3EC0		/* client print error codes limit to 0 - 63 */
#define PKG_EDIT_ERR3	0x3F00		/* More editor codes */
/*	3F00 IS THE LAST PACKAGE THAT CAN BE DEFINED FOR ERROR CODES!
 *  (LOOK FOR HOLES ABOVE).  THE RANGE 0x4000-0x7FFF CANNOT BE USED.
 */


#define	PKG_CODES		255


/* Define offsets within PKG_NETDRVLCL for local network drivers */

/*	On the Unix, since tcp is compiled into Notes, their strings must have
	unique IDs. We define a separate package for all drivers which are optional
	and each optional driver package will have its own offsets for first string
	as defined below. There is some room for expansion, but people adding
	strings should be aware of the extent  of their allocated ID space, and
	change it here if necessary, afterwards recompiling all dependent drivers.
	Under Windows and PM, since each ixport is a separate DLL with separate DS
	space, all offsets are set to 0.
*/

#if defined(UNIX) || defined(MAC)

#define NETPKG_TCP		0		/* Leave at least 20 string offsets for TCP */
#define NETPKG_ATALK	20		/* Leave 35 codes for AppleTalk	*/
#define NETPKG_NWSPX	55

#else
#define NETPKG_TCP		0
#define NETPKG_ATALK	0
#define NETPKG_NWSPX	0
#define NETPKG_NETBIOS	0
#endif


/* Define offsets within PKG_IMPORT and PKG_EXPORT for import/exports */

/*	On the Mac, since all ixports are compiled into Notes, their strings must have unique IDs.
	We avoid using a separate PKG for each by defining the offset of each ixport's first string below.
	There is some room for expansion, but people adding strings should be aware of the extent
	of their allocated ID space, and change it here if necessary, afterwards recompiling all
	dependent ixports!
	Under Windows and PM, since each ixport is a separate DLL with separate DS space,
	all offsets are set to 0.
	Since TARGA, W4W and WMF are not to be ported to the Mac, we don't currently bother with
	defining offsets for them.
*/

#if defined(MAC) || defined(UNIX)	/* On Mac & UNIX, we need to subdivide */

#define IMPKG_IFL	0
#define IMPKG_IPCX	50
#define IMPKG_IPIC	60
#define IMPKG_IRTF	70
#define IMPKG_ISTF	80
#define IMPKG_ISTR	90
#define IMPKG_ITAB	100
#define IMPKG_ITEXT	135
#define IMPKG_ITIFF	145
#define IMPKG_IWKSE	160
#define IMPKG_IWKSV	180
#define IMPKG_ALL	212		/* 18 common strings shared by all Iris IX modules */
#define IMPKG_IBMP	230
#define IMPKG_IGIF	235
#define IMPKG_IW4W	240
#define IMPKG_ISTRNGS 250
#define IMPKG_IJPEG 252

#define EXPKG_XCGM	0
#define EXPKG_XRTF 	10
#define EXPKG_XSTF	20
#define EXPKG_XSTR	65
#define EXPKG_XTAB	90
#define EXPKG_XTEXT	100
#define EXPKG_XTIFF	110
#define EXPKG_XWKS	120
#define EXPKG_ALL	130		/* 20 common strings shared by all Iris IX modules */
#define EXPKG_XW4W	150
#define EXPKG_XVCRD 170
#define EXPKG_XVCRD3 180
#define EXPKG_XCSV  200

#else	/* PC versions - On PC, we don't subdivide - each has own DLL! */

#define IMPKG_IFL 0
#define IMPKG_IPCX 0
#define IMPKG_IPIC 0
#define IMPKG_IRTF 0
#define IMPKG_ISTF 0
#define IMPKG_ISTR 0
#define IMPKG_ITAB 0
#define IMPKG_ITARGA 0
#define IMPKG_ITEXT 0
#define IMPKG_ITIFF 0
#define IMPKG_IWKSE 0
#define IMPKG_IWKSV 0
#define IMPKG_IWMF 0
#define IMPKG_IBMP 0
#define IMPKG_IGIF 0
#define IMPKG_IW4W 0
#define IMPKG_ISTRNGS 0
#define IMPKG_IJPEG 0
#define IMPKG_ALL 32		/* 18 common strings shared by all Iris IX modules */
							/* (Reserve 0-32 for IX-specific strings) */

#define EXPKG_XCGM 0
#define EXPKG_XCSV 0
#define EXPKG_XRTF 0
#define EXPKG_XSTF 0
#define EXPKG_XSTR 0
#define EXPKG_XTAB 0
#define EXPKG_XTEXT 0
#define EXPKG_XTIFF 0
#define EXPKG_XVCRD 0
#define EXPKG_XVCRD3 0
#define EXPKG_XW4W 0
#define EXPKG_XWKS 0
#define EXPKG_ALL 50		/* 20 common strings shared by all Iris IX modules */
							/* (Reserve 0-50 for IX-specific strings) */

#endif



/*	Define error code status flags.  The top two bits of the error code
	are reserved for this purpose */

#define	STS_DISPLAYED	((STATUS) 0x8000)	/* error has already been displayed */
#define ERROR_DISPLAYED(x)	((x) & STS_DISPLAYED)

#define	STS_REMOTE		((STATUS) 0x4000)	/* error came from remote machine */
#define ERROR_REMOTE(x)	((x) & STS_REMOTE)

#define	RAWBLK(x) ((STATUS) ((x)&~(STS_DISPLAYED|STS_REMOTE)))
#define	NOBLK(x) ((STATUS) ((x)&(STS_DISPLAYED|STS_REMOTE)))

#define	ERR_MASK		0x3fff
#define PKG_MASK		0x3f00
#define ERRNUM_MASK		0x00ff

#define STRING_MASK		0x8000

#define IS_STRING(x) ((x)&STRING_MASK ? TRUE : FALSE)	/* is this code a string code? */

/* Remove the DISPLAYED bit - use when logging errors to avoid overlap with string IDs that now (R6+) include the STS_DISPLAYED bit */
#define	ND(x) ((STATUS) ((x) & ~STS_DISPLAYED))

#define	ERR(x) ((STATUS) ((x) & ERR_MASK))		/* Use when comparing errors to a constant - if (ERR(error) == ERR_QUIT) */
#define ERRNUM(x) ((STATUS) ((x) & ERRNUM_MASK)) /* Just the error number, no flags or pkg code. Doesn't work with new PKG codes */

#define PKG(x) ((STATUS) ((x) & PKG_MASK))		/* Get the package code for errors only - obsolete given new pkg codes */
#define IS_PKG(x,pkg) ((BOOL)(((PKG_MASK | pkg) & x) == pkg)) /* Is error x in PKG pkg? */

/*	Define memory allocator hints, which re-use the top 2 bits of
	the BLK_ codes so that we didn't have to add a new argument to
	OSMemAlloc() */

#define	MEM_SHARE		((WORD) 0x8000)	/* Object may be used by multiple processes */
#define	MEM_GROWABLE	((WORD) 0x4000)	/* Object may be OSMemRealloc'ed LARGER */


/*	Define macros which are used when defining error codes to provide text
	for each error message.  The macro normally generates nothing when
	compiled by the C compiler, but will be defined to generate text when
	used in the resource (.RC) file. (See also prestr.h.) */

/*	"errortext" designates a user-displayed message (success, error, warning).
	This are translated, and may be shown to a user.  There should be
	online help for most of these messages. */

#define errortext(code,text)

/*	"helptext" designates the "Menu Help" messages displayed on the top
	line of the screen when you drag across a menu item.  There is usually
	also further online help associated with each of these menu items. */

#define helptext(helpid,code,text)

/*	"stringtext" designates translatable text fragments that need to be
	translated, but are not shown to the user as a "message" (error,
	warning or otherwise).  There should be no online help for these. */

#define stringtext(code,text)

/*	"apitext" designates API-only messages.  This could be translated
	(for foreign API programmers) but NEVER shown to a user. */

#define apitext(code,text)

/*	"debugtext" designates DEBUG-only messages.  This is not to be
	translated and never shown to a user. */

#define debugtext(code,text)

/*	"internaltext" designates STATUS codes used to indicate a condition
	passed between 2 subsystems, but NEVER displayed to a user.  The text
	associated with "internaltext" is NOT EVEN STORED in the executables,
	and thus, never translated or shown to a user. */

#define internaltext(code,text)

/*	"blocktext" designates DEBUG-only descriptions of internal storage
	allocation types.  This is not to be translated and never shown to a user. */

#define blocktext(code,text)

/*	"semtext" designates DEBUG-only descriptions of internal semaphore
	types.  This is not to be translated and never shown to a user. */

#define semtext(code,text)

/*	"donottranslatetext" designates text which must not be translated.
	This text may be included in a resource file in order to facilitate
	configuration changes, such as a string which controls program behavior.
	Or it may be included in order to clearly indicate that the developer
	intends for the text to remain untranslated.
*/

#define donottranslatetext(code,text)

/*	"limitedasciitext" designates text which is constrained to a limited
	subset of ASCII characters.  One use of this macro is for strings used as
	part of Internet protocols which do not allow for non-ascii text.
	The exact limitations will vary with usage, but will usually be
	restricted to 7-bit ascii, excluding '\0'.  These strings MAY be
	translated to other languages, but must keep within the limited ascii
	characters allowed.  For example, translation into French is permitted,
	but no accented characters may be used.
*/
#define limitedasciitext(code,text)

#endif


#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif


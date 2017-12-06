
#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(P128)
#endif

/*********************************************************************/
/*                                                                   */
/* Licensed Materials - Property of IBM                              */
/*                                                                   */
/* L-GHUS-5VMPGW, L-GHUS-5S3PEE                                      */
/* (C) Copyright IBM Corp. 1989, 2004  All Rights Reserved           */
/*                                                                   */
/* US Government Users Restricted Rights - Use, duplication or       */
/* disclosure restricted by GSA ADP Schedule Contract with           */
/* IBM Corp.                                                         */
/*                                                                   */
/*********************************************************************/


#ifndef NAME_DEFS
#define NAME_DEFS

#ifdef __cplusplus
extern "C" {
#endif

/*  
	Definitions pertaining to naming and maximum name lengths
*/

/*	Maximum name length constants. */

/*	MAXPATH is the maximum size of a full filespec, including the server and
	port name (e.g. "NETBEUI!!!A Long Server Name/Iris!!dir1/dir2/file").

	MAXPATH must be capable of representing a full filespec not only on
	the local machine (that we're compiling for), but also capable of
	representing a full filespec on any remote server on any other platform!

	Because a full filespec, including fully expanded server names, can be
	quite long, we have a policy that if the buffer limit is reached when
	constructing a full path, OSPathConstruct (and others) return an error
	to the caller, which gets reported to the user (rather than trying to
	open a truncated filespec).

	The maximum length includes the terminating null so these maximums
	may be used in data declarations.  However, in places where a size
	is specified that indicates the maximum number of characters in the
	name, then the appropriate number minus 1 should be used.
*/

/* Before V4, MAXPATH was 256 for MAC, NLM & UNIX.  For others it was 100.
   Win95 which is now supported by V4 has long filenames, requiring the 
   MAXPATH to be increased to 256.  For old compatibility, we have defined
   OLDMAXPATH which is the same definition of MAXPATH in V3 and below.  The
   MAXPATH define in V4 is now 256 for all platforms.
*/

#if defined(MAC) || defined(NLM) || defined(UNIX)
#define OLDMAXPATH		256			/* Maximum pathname */
#else
#define OLDMAXPATH		100			/* Maximum pathname */
#endif


#if !defined(DOSW16)
#ifdef MAXPATH
#undef MAXPATH						/* Remove previous definition, if any (e.g. UNIX headers) */
#endif
#define MAXPATH			256			/* Maximum pathname */
#else
#define MAXPATH			100			/* Maximum pathname */
#endif

/*	MAXPATH_OLE is the maximum size of a full filespec within OLE, including
	the server and port name.  It is used within Notes OLE in areas where
	the Notes API warning on MAXPATH does not apply.
*/

#if (defined(W32) && !defined(UNIX)) || defined(MAC)	 
#define MAXPATH_OLE     260			/* Maximum pathname for OLE */
#else
#define MAXPATH_OLE     256			/* Maximum pathname for OLE */
#endif


#ifdef MAC
/*	Some Int'l Macs require more than 32 bytes for port name (have seen
	at least 68 chars in a PCMCIA card name).
	Note: for SDK concern, we made change for MAC only.
*/
#define MAXPORTNAME 	64			/* Maximum port name */
#else
#define MAXPORTNAME 	32			/* Maximum port name */
#endif

#define	MAXUSERNAME		256			/* Maximum user name */
#define MAXDOMAINNAME	32 			/* Maximum management domain name */
#define MAXDOMAINPATH	32+1+MAXUSERNAME /* Maximum management domain name component of address (LOTUS:CPD-QA-SERVER) */

#define OLDFILETITLEMAX	33			/* Pre V5 Notefile title string MAX(ASCIZ) */
#define FILETITLEMAX	97			/* Notefile title string MAX(ASCIZ) */

#define DESIGN_LEVELS		2 					/* Cascade can go only one */
												/* level deep parent\sub */
#define DESIGN_LEVEL_MAX	64					/* Maximum size of a level */

#define DESIGN_NAME_MAX	((DESIGN_LEVEL_MAX+1)*DESIGN_LEVELS)	
												/* Guaranteed */
												/* To be the greatest of */
												/* Form, View or Macro */
												/* length. NOTE:  We need */
 												/* space for LEVELS-1 cascade */
												/* characters and a NULL term. */
												/* The +1 takes care of that. */

#define DESIGN_FORM_MAX		DESIGN_NAME_MAX		/* Forms can cascade a level */
#define DESIGN_VIEW_MAX 	DESIGN_NAME_MAX		/* Views can cascade a level */
#define DESIGN_MACRO_MAX 	DESIGN_NAME_MAX		/* Macros can cascade a level */
#define DESIGN_FIELD_MAX 	DESIGN_LEVEL_MAX+1	/* Fields cannot cascade */

#define DESIGN_COMMENT_MAX		256				/* Design element comment max size. */
#define DESIGN_ALL_NAMES_MAX	256				/* All names, including sysnonyms */
#define DESIGN_FOLDER_MAX		DESIGN_VIEW_MAX		/* Same as for views */
#define DESIGN_FOLDER_MAX_NAME	DESIGN_LEVEL_MAX	/* Same as for views */

#define DESIGN_FLAGS_MAX		32


/*	Constants for names used in registering a new workstation/server. */

#define MAXLOCATIONNAME 128
#define MAXCOMMENTNAME 512
#define MAX_ID_PASSWORD 64

/*	Constants used for applying/converting mail system types chosen during
	user registration to the mailtype field in the person document. These
	should be in the same order as the keyword values in this mail type field */


#define MAILSYSTEM_NOTES		0
#define MAILSYSTEM_CCMAIL		1
#define MAILSYSTEM_VIMMAIL		2
#define MAILSYSTEM_NONE			99
/*	Constants for Client Type. */

#define CLIENTTYPE_FULL		0
#define CLIENTTYPE_LIMITED	1
#define CLIENTTYPE_DESKTOP	2



#define MAXNETADR	64


#define RESERVED_PRIV_PREFIX	'('	/* Group names cannot begin with this */
#define RESERVED_CLASS_PREFIX	'['	/* Group names cannot begin with this */

#ifdef MAC
#define MAXFILETITLE	20
#define MAXFILENAME		31
#else
#define MAXFILETITLE	12
#define MAXFILENAME		12
#endif


#ifdef __cplusplus
}
#endif
#endif


#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif


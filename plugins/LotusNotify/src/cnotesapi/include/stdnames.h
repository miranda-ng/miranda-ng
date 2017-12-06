
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


#ifndef STD_NAME_DEFS
#define STD_NAME_DEFS

#ifdef __cplusplus
extern "C" {
#endif

/*	Standard NSF Field Name/Value Definitions for the Notes product */

/* Very global field names */

#define	FIELD_TITLE "$TITLE"
#define	FIELD_FORM	"Form"
#define FIELD_TYPE_TYPE	"Type"
#define	FIELD_LINK	"$REF"
#define	FIELD_UPDATED_BY "$UpdatedBy"
#define FIELD_NAMELIST "$NameList"
#define FIELD_NAMED 	"$Name"
#define	FIELD_URL		"URL"				/* used when copying link to clipboard */
#define	FIELD_UNAME		"$UName"			/* Universal UNID/Name */
#define	FIELD_CREATED	"$Created"			/* Created date, if specified overrides UNID created date for @Created */
#define FIELD_NAVIGATE_URL	"$NavigateToURL"/* used to pass a URL to an embedded web browser control */ 
#define FIELD_RELOAD_REFRESH "$ReloadRefresh"/* used to pass reload/refresh info to embedded web browser control */ 
#define	DESIGN_CLASS "$Class"
#define	DESIGN_MODIFIED "$Modified"
#define	DESIGN_COMMENT FILTER_COMMENT_ITEM
#define	DESIGN_READERS "$Readers"			/* Text list of users allowed to read note */
#define FIELD_ANONYMOUS "$Anonymous"		/* Indicates an anonymous document. */
#define	DESIGN_UPDATERS "$Updaters"			/* Text list of users allowed to update the folder note */
#define FIELD_NOPURGE 	"$NoPurge"			/* Note should never be purged. */
#define	DESIGN_RETAIN_FIELDS "$RetainFields"	/* Text list of fields to retain in a
													design refresh */
#define FIELD_ORIG_SIZE	"$OrigSize"			/* Size of entire note.  Set for
												abstracted note to give user
												info on whether they want to
												download the whole message or
												not. */
#define	FIELD_LANGUAGE "$LANGUAGE"			/* Used to specify the language of the note */
#define FIELD_VIEWLOCALE "$VIEWCOLLATION"
#define FIELD_BASE_TEMPLATE_VERSION "$TemplateBuild"
#define FIELD_BASE_TEMPLATE_NAME "$TemplateBuildName"
#define FIELD_BASE_TEMPLATE_DATE "$TemplateBuildDate"
#define FIELD_NO_SOFT_DELETE "$NoSoftDelete" /* Override soft delete on a per-note basis */
#define FIELD_HIDEINFO "$HideInfo" /* If present in design element, its design is hidden and this item contains the mod time before hiding. */

/* Public Access definitions */
#define FIELD_PUBLICROLE 	"$P"			/* Reader List role name for public users */
#define FIELD_PUBLICACCESS 	"$PublicAccess"	/* Note has public access if ACL_FLAG_PUBLICREADER is set. */
#define FORM_FIELD_PUBLICACCESS 	"$FormPublicAccess"	/* Form Note has public access if ACL_FLAG_PUBLICREADER is set. */
#define	FIELD_PUBLICACCESS_ENABLED	'1'

#define FIELD_LOGVIEWUPDATES 	"$LogViewUpdates"	/* NIF will log all incremental view updates */
#define	FIELD_LOGVIEWUPDATES_ENABLED	'1'

/* Pseudo item names (don't exist in Notes) recognized by NAMELookup */

#define ITEM_NOTEID			"$$NoteID"			/* a NAMELookup item to be returned */
#define ITEM_DBNAME			"$$DBName"			/* address book this entry was found in */
#define ITEM_DBINDEX		"$$DBIndex"			/* 1-based db index upon primary NAB and ABs listed in MAB */
#define ITEM_MODIFIEDTIME	"$$ModifiedTime"	/* TIMEDATE this entry was last modified */
#define ITEM_DOMAINTYPE		"$$DomainType"		/* either NOTES or LDAP */
#define ITEM_UNID			"$$UNID"			/* a NAMELookup item to be returned */
#define ITEM_READERLIST		"$$Readers"			/* Return the readers list */
#define ITEM_NOTESDN		"$$NotesDN"			/* A notes style distinquished name,
												   only returned from LDAP address books */
#define ITEM_ORIGINAL_LDAP_DN "$$OriginalLDAPDN" /* Original LDAP DN (non-normalized) */

#define ITEM_LTPAUSERNAME	"$$LTPAUsername"	/* The name that should be used (in preference
													to the DN) to build an LTPA token for 
													single sign on support.	*/

						


/* field definitions for Data Connection Resources */
	
#define FIELD_DCR_CLASS			"$DCRClass"		/* e.g. RDBMS, ERP */
#define FIELD_DCR_GEN_PROPS	"$DCRGenProps"	/* generic connector properties */
#define FIELD_DCR_CUS_PROPS	"$DCRCusProps" /* custom connector properties  */
#define FIELD_DCR_CUS_DESCS	"$DCRCusDescs" /* descriptors for custom props */
#define DCR_METADATA_RECORD	"$DCRMetadata" /* NSFSearch item id for summary buffer */
#define DCR_METADATA_SEARCH	"$DCRCatalog"	/* Special id used as NSFSearch trigger */	

/* define order of generic properties in FIELD_DCR_GEN_PROPS */
#define DCR_GPROP_TYPE			0
#define DCR_GPROP_SERVER		1
#define DCR_GPROP_DATABASE		2
#define DCR_GPROP_USERNAME		3
#define DCR_GPROP_PASSWORD		4
#define DCR_GPROP_MD_OWNER		5 
#define DCR_GPROP_MD_NAME		6
#define DCR_GPROP_CONNECTIONS	7
#define DCR_GPROP_FLAGS			8
#define DCR_GPROP_OPEN_PROC	9
#define DCR_GPROP_UPDATE_PROC	10
#define DCR_GPROP_CREATE_PROC	11
#define DCR_GPROP_DELETE_PROC	12
#define DCR_GPROP_LAST DCR_GPROP_DELETE_PROC

/* Flag definitions for Data Connection Resources: misc properties */
#define DCR_FLAG_TABLE		'T'
#define DCR_FLAG_VIEW		'V'
#define DCR_FLAG_PROC		'P'
#define DCR_FLAG_UPDATE_CHANGED_FIELDS    'u'
#define DCR_FLAG_BLOCK_KEY_FIELD_UPDATES  'b'	
#define DCR_FLAG_CONFLICT_DETECTION       'c'
#define DCR_FLAG_FIELD_DECS_ERRORS        'e'
#define DCR_FLAG_DISABLE_HTTP_CACHING     'h'
#define DCR_FLAG_IGNORE_MISSING_RECORD    'i'	/* On missing record */
#define DCR_FLAG_CREATE_EXTERNAL_RECORD   'x'
#define DCR_FLAG_TRUNCATE_PRECISION       'p'	/* On data conflict */
#define DCR_FLAG_TRUNCATE_DATA            'd'
#define DCR_FLAG_TRIM_ALL_FIELDS          'a'	/* Space trimming */
#define DCR_FLAG_TRIM_NON_KEY_FIELDS      'k'

	
/*	Define Private note field name, and define the known types */

#define FIELD_PRIVATE_TYPE 	"$Private"

#define	FIELD_PRIVATE_TYPE_QUERY	'q'
#define	FIELD_PRIVATE_TYPE_AGENT	'a'
#define	FIELD_PRIVATE_TYPE_VIEW		'v'
#define	FIELD_PRIVATE_TYPE_FOLDER	'f'

/*	The following items are inserted into a note which cannot be saved to its
	Some special named notes. These values are stored in the FIELD_NAMED item to
	identify a named note. */

#define NAMEDNOTE_PRIVATEDESIGN		"$PrivateDesign"
#define NAMEDNOTE_PROFILE			"$Profile"
#define NAMEDNOTE_IMAP_DELETED_ENTRY "$IMAPResyncDE"
#define NAMEDNOTE_POP_DELETED_ENTRY "$PopDE"


/* 	The following definitions of for the public directory profile note */
#define NAMEDNOTE_PUBLICDIRECTORYPROFILE	"directoryprofile"
#define LEDNAME_ITEM						"LedName"
#define TRUST_DIRCAT_ITEM					"TrustDircat"
#define DIRECTORY_DOMAIN_ITEM				"Domain" 
#define SECURE_INETPASSWORDS_ITEM 			"SecureInetPasswords" 
#define ADMIN_LDAPSERVER_ITEM				"AdminLDAPServer"

/*	The following items are inserted into a note which cannot be saved to its
	original database due to network failure.  When the failure can be
	remedied, the note is read back into memory and fixed up to point
	to correct database.  At that point the items are removed. */

#define PENDINGNAME					"NOTEPEND"
#define PENDINGSAVE_LINK_ITEM		"$ORIGLINK"
#define PENDINGSAVE_FILENAME_ITEM	"$ORIGFILE"

/*	Designer minimum version */
/*	"minimum version" means "you need at least this version to safely save 
	this design element." */

#define DESIGNER_VERSION			"$DesignerVersion"

#define DESIGNER_MIN_VERSION_5		"5"
#define DESIGNER_MIN_VERSION_6		"6"
#define DESIGNER_MIN_VERSION_6_5	"6.5"
#define DESIGNER_MIN_VERSION_7		"7"
#define DESIGNER_MIN_VERSION_8		"8"
#define DESIGNER_MIN_VERSION_9		"9"
#define DESIGNER_MIN_VERSION_10		"A"

/*	Design flags */

#define	DESIGN_FLAGS "$Flags"

/*	Please keep these flags in alphabetic order (based on the flag itself) so that
	we can easily tell which flags to use next. Note that some of these flags apply
	to a particular NOTE_CLASS; others apply to all design elements. The comments
	indicate which is which. In theory, flags that apply to two different NOTE_CLASSes
	could overlap, but for now, try to make each flag unique. */

#define DESIGN_FLAG_ADD					'A'	/*	FORM: Indicates that a subform is in the add subform list */
#define DESIGN_FLAG_ANTIFOLDER			'a'	/*	VIEW: Indicates that a view is an antifolder view */
#define	DESIGN_FLAG_BACKGROUND_FILTER 	'B'	/*	FILTER: Indicates FILTER_TYPE_BACKGROUND is asserted */
#define	DESIGN_FLAG_INITBYDESIGNONLY	'b'	/*	VIEW: Indicates view can be initially built only by designer and above */
#define DESIGN_FLAG_NO_COMPOSE 			'C'	/*	FORM: Indicates a form that is used only for */
											/*		query by form (not on compose menu). */
#define DESIGN_FLAG_CALENDAR_VIEW		'c'	/*	VIEW: Indicates a form is a calendar style view. */
#define DESIGN_FLAG_NO_QUERY 			'D'	/* 	FORM: Indicates a form that should not be used in query by form */
#define DESIGN_FLAG_DEFAULT_DESIGN 		'd'	/* 	ALL: Indicates the default design note for it's class (used for VIEW) */
#define	DESIGN_FLAG_MAIL_FILTER 		'E'	/*	FILTER: Indicates FILTER_TYPE_MAIL is asserted */
#define DESIGN_FLAG_PUBLICANTIFOLDER	'e'	/*	VIEW: Indicates that a view is a public antifolder view */
#define DESIGN_FLAG_FOLDER_VIEW			'F'	/*	VIEW: This is a V4 folder view. */
#define DESIGN_FLAG_V4AGENT 			'f'	/*	FILTER: This is a V4 agent */
#define DESIGN_FLAG_VIEWMAP 			'G'	/*	VIEW: This is ViewMap/GraphicView/Navigator */
#define DESIGN_FLAG_FILE				'g'	/*  FORM: file design element */
#define DESIGN_FLAG_OTHER_DLG			'H'	/*	ALL: Indicates a form that is placed in Other... dialog */
#define DESIGN_FLAG_JAVASCRIPT_LIBRARY  'h' /*  Javascript library. */
#define DESIGN_FLAG_V4PASTE_AGENT		'I'	/*	FILTER: This is a V4 paste agent */
#define DESIGN_FLAG_IMAGE_RESOURCE		'i'	/*	FORM: Note is a shared image resource */
#define DESIGN_FLAG_JAVA_AGENT   		'J' /*  FILTER: If its Java */
#define DESIGN_FLAG_JAVA_AGENT_WITH_SOURCE 'j' /* FILTER: If it is a java agent with java source code. */
#define DESIGN_FLAG_MOBILE_DIGEST		'K'	/* to keep mobile digests out of form lists */
#define DESIGN_FLAG_CONNECTION_RESOURCE		'k'	/* Data Connection Resource (DCR) for 3rd party database */
#define DESIGN_FLAG_LOTUSSCRIPT_AGENT   'L' /*  FILTER: If its LOTUSSCRIPT */
#define DESIGN_FLAG_DELETED_DOCS		'l' /*  VIEW: Indicates that a view is a deleted documents view */
#define	DESIGN_FLAG_QUERY_MACRO_FILTER	'M'	/*	FILTER: Stored FT query AND macro */
#define DESIGN_FLAG_SITEMAP				'm' /*  FILTER: This is a site(m)ap. */
#define DESIGN_FLAG_NEW					'N'	/*  FORM: Indicates that a subform is listed when making a new form.*/
#define DESIGN_FLAG_HIDE_FROM_NOTES		'n' /*  ALL: notes stamped with this flag 
													will be hidden from Notes clients 
													We need a separate value here 
													because it is possible to be
													hidden from V4 AND to be hidden
													from Notes, and clearing one 
													should not clear the other */
#define	DESIGN_FLAG_QUERY_V4_OBJECT 	'O'	/*	FILTER: Indicates V4 search bar query object - used in addition to 'Q' */
#define DESIGN_FLAG_PRIVATE_STOREDESK   'o' /*  VIEW: If Private_1stUse, store the private view in desktop */
#define	DESIGN_FLAG_PRESERVE 			'P'	/*	ALL: related to data dictionary */
#define DESIGN_FLAG_PRIVATE_1STUSE 		'p'	/* 	VIEW: This is a private copy of a private on first use view. */
#define	DESIGN_FLAG_QUERY_FILTER 		'Q'	/*	FILTER: Indicates full text query ONLY, no filter macro */
#define DESIGN_FLAG_AGENT_SHOWINSEARCH	'q'	/*	FILTER: Search part of this agent should be shown in search bar */
#define DESIGN_FLAG_REPLACE_SPECIAL		'R'	/*	SPECIAL: this flag is the opposite of DESIGN_FLAG_PRESERVE, used
												only for the 'About' and 'Using' notes + the icon bitmap in the icon note */
#define DESIGN_FLAG_PROPAGATE_NOCHANGE  'r' /*  DESIGN: this flag is used to propagate the prohibition of design change */
#define DESIGN_FLAG_V4BACKGROUND_MACRO 	'S'	/*	FILTER: This is a V4 background agent */
#define DESIGN_FLAG_SCRIPTLIB			's'	/*	FILTER: A database global script library note */
#define DESIGN_FLAG_VIEW_CATEGORIZED 	'T'	/* 	VIEW: Indicates a view that is categorized on the categories field */
#define DESIGN_FLAG_DATABASESCRIPT		't'	/*	FILTER: A database script note */
#define DESIGN_FLAG_SUBFORM 			'U'	/*	FORM: Indicates that a form is a subform.*/
#define	DESIGN_FLAG_AGENT_RUNASWEBUSER	'u'	/*	FILTER: Indicates agent should run as effective user on web */
#define	DESIGN_FLAG_AGENT_RUNASINVOKER	'u'	/*	FILTER: Indicates agent should run as invoker (generalize the
												web user notion, reuse the flag */
#define DESIGN_FLAG_PRIVATE_IN_DB 		'V'	/* 	ALL: This is a private element stored in the database */
#define DESIGN_FLAG_IMAGE_WELL			'v'	/*	FORM: Used with 'i' to indicate the image is an image well. 
													Used for images with images across, not images down.
													'v' looks like a bucket */
#define DESIGN_FLAG_WEBPAGE				'W'	/*	FORM: Note is a WEBPAGE	*/
#define DESIGN_FLAG_HIDE_FROM_WEB		'w' /*  ALL: notes stamped with this flag 
													will be hidden from WEB clients */
/* WARNING: A formula that build Design Collecion relies on the fact that Agent Data's
			$Flags is the only Desing Collection element whose $Flags="X" */
#define DESIGN_FLAG_V4AGENT_DATA		'X' /*  FILTER: This is a V4 agent data note */
#define DESIGN_FLAG_SUBFORM_NORENDER	'x'	/*	SUBFORM: indicates whether
												we should render a subform in
												the parent form					*/
#define DESIGN_FLAG_NO_MENU 			'Y'	/*	ALL: Indicates that folder/view/etc. should be hidden from menu. */
#define DESIGN_FLAG_SACTIONS			'y'	/*	Shared actions note	*/
#define DESIGN_FLAG_MULTILINGUAL_PRESERVE_HIDDEN 'Z' /* ALL: Used to indicate design element was hidden */
											/*	before the 'Notes Global Designer' modified it. */
											/*	(used with the "!" flag) */
#define DESIGN_FLAG_SERVLET				'z' /*  FILTER: this is a servlet, not an agent! */
#define DESIGN_FLAG_ACCESSVIEW			'z' /*  FORM: reuse obsoleted servlet flag */

#define DESIGN_FLAG_FRAMESET			'#'	/*	FORM: Indicates that this is a frameset note */ 
#define DESIGN_FLAG_MULTILINGUAL_ELEMENT '!'/*	ALL: Indicates this design element supports the */
											/*	'Notes Global Designer' multilingual addin */
#define DESIGN_FLAG_JAVA_RESOURCE		'@'	/*	FORM: Note is a shared Java resource */
#define DESIGN_FLAG_STYLESHEET_RESOURCE	'='	/* Style Sheet Resource (SSR) */
#define DESIGN_FLAG_WEBSERVICE			'{' /* FILTER: web service design element */
#define DESIGN_FLAG_SHARED_COL			'^' /* VIEW: shared column design element */

#define DESIGN_FLAG_HIDE_FROM_MOBILE	'1'	/*	hide this element from mobile clients */
#define DESIGN_FLAG_HIDE_FROM_PORTAL	'2' /*	hide from portal */

#define DESIGN_FLAG_HIDE_FROM_V3		'3'	/*	ALL: notes stamped with this flag 
													will be hidden from V3 client */
#define DESIGN_FLAG_HIDE_FROM_V4		'4'	/*	ALL: notes stamped with this flag 
													will be hidden from V4 client */
#define DESIGN_FLAG_HIDE_FROM_V5		'5'	/* 	FILTER: 'Q5'= hide from V4.5 search list */
											/*	ALL OTHER: notes stamped with this flag 
													will be hidden from V5 client */
#define DESIGN_FLAG_HIDE_FROM_V6		'6'	/*	ALL: notes stamped with this flag 
													will be hidden from V6 client */
#define DESIGN_FLAG_HIDE_FROM_V7		'7'	/*	ALL: notes stamped with this flag 
													will be hidden from V7 client */
#define DESIGN_FLAG_HIDE_FROM_V8		'8'	/*	ALL: notes stamped with this flag 
													will be hidden from V8 client */
#define DESIGN_FLAG_HIDE_FROM_V9		'9'	/*	ALL: notes stamped with this flag 
													will be hidden from V9 client */
#define DESIGN_FLAG_MUTILINGUAL_HIDE	'0'	/*	ALL: notes stamped with this flag 
													will be hidden from the client 
													usage is for different language
													versions of the design list to be
													hidden completely				*/
#define DESIGN_FLAG_WEBHYBRIDDB			'%'	/*	shimmer design docs */

#define DESIGN_FLAG_READONLY			'&'	/*  for files, at least for starters */
#define DESIGN_FLAG_NEEDSREFRESH		'$' /* 	for files, at least for now */
#define DESIGN_FLAG_HTMLFILE			'>' /*	this design element is an html file */
#define DESIGN_FLAG_JSP					'<'	// 	this design element is a jsp 
#define DESIGN_FLAG_QUERYVIEW		'<'  // VIEW - Query View in design list
#define DESIGN_FLAG_DIRECTORY			'/' /*	this file element is a directory */

#define DESIGN_FLAG_PRINTFORM			'?' /*	FORM - used for printing. */
#define DESIGN_FLAG_HIDEFROMDESIGNLIST	'~'	/*	keep this thing out of a design list */
#define DESIGN_FLAG_HIDEONLYFROMDESIGNLIST	'}'	/*	keep this thing out of a design list but allow users to view doc using it */

/*	These are the flags that help determine the type of a design element.
	These flags are used to sub-class the note classes, and cannot be
	changed once they are created (for example, there is no way to change
	a form into a subform). */

#define DESIGN_FLAGS_SUBCLASS		"UW#yi@GFXstmzk=Kg%{^"

/*	These are the flags that can be used to distinguish between two
	design elements that have the same class, subclass (see DESIGN_FLAGS_SUBCLASS),
	and name. */

#define DESIGN_FLAGS_DISTINGUISH	"nw123456789"

/* '+' = ANY of the flags, '-' = NONE of the flags, '*' = ALL of the flags 	*/
/* '(+-*' = a combination of the above.										*/
/*	 Example:  "(+AB-C*DE" = (A OR B) AND (NOT C) AND (D AND E)				*/
/* 		Note: be sure to have +-* placeholders even if no flags for some.	*/
/* 			  ie: "(+-C*DE"   = (NOT C) AND (D AND E).						*/
/*	 Note: "(+-Q*" is equivalent to "-Q" 									*/

#define	DFLAGPAT_V4SEARCHBAR "(+Qq-Bst5nmz*"/* display things editable at V4 search bar; version filtering */
#define	DFLAGPAT_SEARCHBAR "(+QM-st5nmz*" 	/* display things editable at search bar; version filtering */

#define DFLAGPAT_VIEWFORM "-FQMUGXWy#i@0nKg~%z^"	/* display things editable with dialog box; version filtering */
#define DFLAGPAT_VIEWFORM_MENUABLE "-FQMUGXWy#i@40nKg~%z^}"	/* display things showable on the menu */
#define DFLAGPAT_VIEWFORM_ALL_VERSIONS "-FQMUGXWy#i@Kg~%z^}" /* display things editable with dialog box; no version filtering (for design) */
#define DFLAGPAT_PRINTFORM_ALL_VERSIONS "+?" /* display things editable with dialog box; no version filtering (for design) */

#define	DFLAGPAT_TOOLSRUNMACRO "-QXMBESIst5nmz{"/* display things that are runnable; version filtering */
#define DFLAGPAT_AGENTSLIST	"-QXstmz{"		/* display things that show up in agents list. No version filtering (for design) */
#define DFLAGPAT_PASTEAGENTS	"+I"		/* select only paste agents */
#define DFLAGPAT_SCRIPTLIB		 "+sh"		/* display only database global script libraries */
#define DFLAGPAT_SCRIPTLIB_LS	 "(+s-jh*"	/* display only database global LotusScript script libraries */
#define DFLAGPAT_SCRIPTLIB_JAVA	 "*sj"		/* display only database global Java script libraries */
#define DFLAGPAT_SCRIPTLIB_JS	 "+h"		/* display only database global Javascript script libraries */
#define DFLAGPAT_DATABASESCRIPT "+t"		/* display only database level script */

#define DFLAGPAT_SUBFORM "(+U-40n*"		 	/* display only subforms; version filtering	*/
#define DFLAGPAT_SUBFORM_DESIGN "(+U-40*"	/* display only subforms; for design mode, version filtering	*/
#define DFLAGPAT_SUBFORM_ALL_VERSIONS "+U"	/* only subforms; no version filtering */
#define	DFLAGPAT_DBRUNMACRO "+BS"			/* run all background filters */
#define DFLAGPAT_COMPOSE "-C40n"			/* display forms that belong in compose menu; version filtering */
#define DFLAGPAT_NOHIDDENNOTES "-n"			/* select elements not hidden from notes */
#define DFLAGPAT_NOHIDDENWEB "-w"			/* select elements not hidden from web */
#define DFLAGPAT_QUERYBYFORM "-DU40nyz{"	/* display forms that appear in query by form; version filtering */
#define	DFLAGPAT_PRESERVE "+P"				/* related to data dictionary; no version filtering */
#define DFLAGPAT_SUBADD	"(+-40*UA"			/* subforms in the add subform list; no version filtering */
#define DFLAGPAT_SUBNEW "(+-40*UN"			/* subforms that are listed when making a new form.*/
#define DFLAGPAT_VIEW "-FG40n^"				/* display only views */
#define DFLAGPAT_VIEW_DESIGN "-FG40^"		/* display only views, ignore hidden from notes */
#define DFLAGPAT_NOTHIDDEN "-40n"			/* design element is not hidden*/
#define DFLAGPAT_FOLDER "(+-04n*F"			/* display only folders; version filtering */
#define DFLAGPAT_FOLDER_DESIGN "(+-04*F"	/* display only folders; version filtering, ignore hidden notes */
#define DFLAGPAT_FOLDER_ALL_VERSIONS "*F"/* display only folders; no version filtering (for design) */
#define DFLAGPAT_CALENDAR "*c"				/* display only calendar-style views */
#define DFLAGPAT_SHAREDVIEWS "-FGV^40n"		/* display only shared views */
#define DFLAGPAT_SHAREDVIEWSFOLDERS "-G^V40p"	/* display only shared views and folder; all notes & web */
#define DFLAGPAT_SHAREDWEBVIEWS "-FGV40wp^"	/* display only shared views not hidden from web */
#define DFLAGPAT_SHAREDWEBVIEWSFOLDERS "-GV40wp^" /* display only shared views and folders not hidden from web */
#define DFLAGPAT_VIEWS_AND_FOLDERS "-G40n^"	/* display only views and folder; version filtering */
#define DFLAGPAT_VIEWS_AND_FOLDERS_DESIGN "-G40^"	/* display only views and folder; all notes & web */
#define DFLAGPAT_SHARED_COLS "(+-*^"		/* display only shared columns */

#define DFLAGPAT_VIEWMAP "(+-04n*G"			/* display only GraphicViews; version filtering */
#define DFLAGPAT_VIEWMAP_ALL_VERSIONS "*G"	/* display only GraphicViews; no version filtering (for design) */
#define DFLAGPAT_VIEWMAPWEB "(+-04w*G"		/* display only GraphicViews available to web; version filtering */
#define DFLAGPAT_VIEWMAP_DESIGN "(+-04*G" 	/* display only GraphicViews; all notes & web navs */

#define DFLAGPAT_WEBPAGE "(+-*W"			/* display WebPages	*/
#define DFLAGPAT_WEBPAGE_NOTES "(+W-n*"		/* display WebPages	available to notes client */
#define DFLAGPAT_WEBPAGE_WEB "(+W-w*"		/* display WebPages	available to web client */
#define DFLAGPAT_OTHER_DLG "(+-04n*H"			/* display forms that belong in compose menu */
#define DFLAGPAT_CATEGORIZED_VIEW "(+-04n*T"	/* display only categorized views */

#define DFLAGPAT_DEFAULT_DESIGN "+d"		/* detect default design note for it's class (used for VIEW) */
#define DFLAGPAT_FRAMESET "(+-*#"			/* display only Frameset notes */
#define DFLAGPAT_FRAMESET_NOTES "(+#-n*"  	/* Frameset notes available to notes client */
#define DFLAGPAT_FRAMESET_WEB "(+#-w*"  	/* Frameset notes available to web client */
#define DFLAGPAT_SITEMAP	"+m"			/* SiteMap notes (actually, "mQ345") */
#define DFLAGPAT_SITEMAP_NOTES "(+m-n*"		/* sitemap notes available to notes client */
#define DFLAGPAT_SITEMAP_WEB "(+m-w*"		/* sitemap notes available to web client */
#define DFLAGPAT_IMAGE_RESOURCE "+i"		/* display only shared image resources */
#define DFLAGPAT_IMAGE_RES_NOTES "(+i-n*"	/* display only notes visible images */
#define DFLAGPAT_IMAGE_RES_WEB	"(+i-w*"	/* display only web visible images */
#define DFLAGPAT_IMAGE_WELL_RESOURCE "(+-*iv" /* display only shared image resources that 
												have more than one image across */
#define DFLAGPAT_IMAGE_WELL_NOTES "(+-n*iv" /* display only shared image resources that 
												have more than one image across - notes only */
#define DFLAGPAT_IMAGE_WELL_WEB "(+-w*iv" /* display only shared image resources that 
												have more than one image across - web only */
#define DFLAGPAT_JAVA_RESOURCE "+@"			/* display only shared Java resources */
#define DFLAGPAT_JAVA_RESOURCE_NOTES "(+@-n*"	/* display only shared Java resources visible to notes */
#define DFLAGPAT_JAVA_RESOURCE_WEB "(+@-w*"		/* display only shared Java resources visible to web */

#define DFLAGPAT_DATA_CONNECTION_RESOURCE "+k"		/* display only shared data connection resources */
#define DFLAGPAT_DB2ACCESSVIEW	"+z"			/* display only db2 access views */

#define DFLAGPAT_STYLE_SHEET_RESOURCE "+="	/* display only shared style sheet resources */
#define DFLAGPAT_STYLE_SHEETS_NOTES "(+=-n*" /* display only notes visible style sheets */
#define DFLAGPAT_STYLE_SHEETS_WEB "(+=-w*"	/* display only web visible style sheets */
#define DFLAGPAT_FILE			"+g"		/* display only files */
#define DFLAGPAT_FILE_DL		"(+g-~*"	/* list of files that should show in file DL */
#define DFLAGPAT_FILE_NOTES		"(+g-n*"	/* list of notes only files */
#define DFLAGPAT_FILE_WEB		"(+g-w*"	/* list of web only files */
#define DFLAGPAT_HTMLFILES		"(+-*g>"	/* display only html files */
#define DFLAGPAT_HTMLFILES_NOTES "(+-n*g>"	/* htmlfiles that are notes visible */
#define DFLAGPAT_HTMLFILES_WEB	"(+-w*g>"	/* htmlfiles that are web visible */
#define DFLAGPAT_FILE_ELEMS		"(+gi=-/*"	/* files plus images plus style sheets with no directory elements */

#define DFLAGPAT_SERVLET		"+z"		/* servlets */
#define DFLAGPAT_SERVLET_NOTES	"(+z-n*"	/* servlets not hidden from notes */
#define DFLAGPAT_SERVLET_WEB	"(+z-w*"	/* servlets not hidden from the web */

#define DFLAGPAT_WEBSERVICE		"+{"		/* web service */
#define DFLAGPAT_JAVA_WEBSERVICE "(+Jj-*{"	/* java web services */
#define DFLAGPAT_LS_WEBSERVICE   "*{L"		/* lotusscript web services */

#define DFLAGPAT_JSP			"(+-*g<"		/* display only JSP's */

/* Shared actions must be visible to both Notes and the Web since there is
	only one of these puppies - there is no list in the designer to get at
	more than one.  However, for completeness, I'll make the appropriate
	patterns for the day we may want to have separateness. */

#define DFLAGPAT_SACTIONS_DESIGN "+y"
#define DFLAGPAT_SACTIONS_WEB 	 "(+-0*y"
#define DFLAGPAT_SACTIONS_NOTES	 "(+-0*y"


/* Web server patterns */
#define DFLAGPAT_NONWEB "+w70stVXp^"			/* elements that are never used on the web */
#define DFLAGPAT_NONWEB_EXCLUDE "-w70stVXp^"	/* same flags as DFLAGPAT_NONWEB */
/* For the rest, no need to include flags from DFLAGPAT_NONWEB, since
   these flags are excluded in an initial pass. (see insrv\inotes\ndesdict.cpp) */
#define DFLAGPAT_AGENTSWEB "(+-QXstmz{*"		/* agents that can be run from the web */
#define DFLAGPAT_AGNTORWEBSVCWEB "(+-QXstmz*"	/* agents or web services that can be run from the web */
#define DFLAGPAT_WEBSERVICEWEB "+{"			/* web services that can be run from the web */
#define DFLAGPAT_FORMSWEB "-U#Wi@y"			/* forms usable from the web */
#define DFLAGPAT_SUBFORMSWEB "+U"			/* subforms usable from the web */
#define DFLAGPAT_FRAMESETSWEB "+#"			/* frameset from the web */
#define DFLAGPAT_PAGESWEB "+W"				/* web pages from the web */
#define DFLAGPAT_VIEWSWEB "-G"				/* views usable from the web */
#define DFLAGPAT_NAVIGATORSWEB "+G"			/* navigators usable from the web */
#define DFLAGPAT_SHAREDFIELDSWEB "*"		/* shared fields usable from the web */
#define DFLAGPAT_ALLWEB "*"					/* all design elements */
#define DFLAGPAT_NO_FILERES_DIRS "-/"		/* all design elements excluding file resource directories*/
#define DFLAGPAT_FIRSTPATTERNCHAR "(+-*"	/* patterns start with one of these */
#define DFLAGPAT_WEBHYBRIDDB "+%"			/* all WebHybridDb design elements */

/*	a new flags field that (will be) in the design collection.  $Flags is just
	so full that we need some wiggle room!! */
#define DESIGN_FLAGS_EXTENDED	"$FlagsExt"	/* because we're just gonna need it */
#define DESIGN_FLAGEXT_FILE_DEPLOYABLE	'D'	/* for web apps, this file is ready for primetime */
#define DESIGN_FLAGEXT_DONTREFRESH_ON_REDEPLOY	'R' /* for web apps, this file should not be replaced on redeploy */
#define DESIGN_FLAGEXT_NOTE_HAS_DAVPROPERTIES	'P'	/* for WebDAV resources.  The note has a dead properties in the $DavProperties field */
#define DESIGN_FLAGEXT_NOTE_HAS_MSPROPERTIES	'M' /* for WebDAV: indicates that certain MS properties are among the dead properites */
#define DESIGN_FLAGEXT_DAVLOCKNULL				'N' /* for WebDAV lock null resources */
#define DESIGN_FLAGEXT_WEBDAV_HIDDEN			'H' /* for WebDAV: the note is hidden */
#define DESIGN_FLAGEXT_DAVCOMPUTEFORM			'C'	/*	for davs, a compute with form dav */
#define DESIGN_FLAGEXT_DAVATTACH				'A'	/*	for davs, this one supports attachments */
#define DESIGN_FLAGEXT_DAVGMTNORMAL				'Z'	/*	for davs, this one GMT normalizes */
#define DESIGN_FLAGEXT_JAVADEBUG				'D' /*	can reuse D because old use is obsolete */
#define DESIGN_FLAGEXT_PROFILE					'F' /* 	profile code running in this note */

/* DAV item names */
#define DESIGN_DAV_CFNAME		"$DAVComputeForm"
#define DESIGN_DAV_ATTVIEW		"$DAVAttachView"
#define DESIGN_DAV_FLDS			"$DAVFldDesc"
#define DESIGN_DAV_FLDS_LIVE	"$DAVLiveFlds"

/* The agent flag is stored in the new flag so the info is not removed by older Designers when the user edits the agent */
#define DESIGN_FLAGEXT_AGENT_ACTIVATABLE		'a'	/* if the agent is activatable, if it is missing it is not */

/* Compute-defined item names that are actually pseudo-items, requested
   by NIF, that cause Compute to return some info.  These are not actually
   in any document, and if they are, will probably not work properly. */

#define	FIELD_COMPUTE_READERS "$C1$"

/* Mail note item names */

#define MAIL_MESSAGETYPE_ITEM "MessageType"		/* Message type. */
#define MAIL_MESSAGE_TYPE_ITEM "$MessageType"	/* $MessageType item name */
#define MAIL_SENDTO_ITEM "SendTo"				/* SendTo item name */
#define MAIL_COPYTO_ITEM "CopyTo"				/* CopyTo item name */
#define MAIL_REPLYTO_ITEM		"ReplyTo"		/* ReplyTo, used for agents */
#define MAIL_REPLYDATE_ITEM		"ReplyDate"		/* ReplyDate item name */
#define MAIL_RESPONSETO_ITEM	"ResponseTo"	/* ResponseTo, used for agents */
#define MAIL_FROM_ITEM "From"					/* From item name */
#define MAIL_FROMDOMAIN_ITEM "FromDomain"		/* From domain item name */
#define MAIL_SUBJECT_ITEM "Subject"				/* Subject item name */
#define MAIL_DISPLAY_SUBJECT_ITEM "DisplaySubject"	/* Used by reply form */
#define MAIL_COMPOSEDDATE_ITEM "ComposedDate"	/* Composed date item name */
#define MAIL_POSTEDDATE_ITEM "PostedDate"		/* Posted date item name */
#define MAIL_BODY_ITEM "Body"					/* Body item name */
#define MAIL_INTENDEDRECIPIENT_ITEM "IntendedRecipient" /* Intended recipient item */
#define MAIL_ALTINTENDEDRECIPIENT_ITEM "AltIntendedRecipient" /* Alternate Intended recipient item */
#define MAIL_LANGINTENDEDRECIPIENT_ITEM "$LangIntendedRecipient" /* Language Tag Intended Recipient */
#define MAIL_FAILUREREASON_ITEM "FailureReason"		/* Failure reason item */
#define MAIL_DEADFAILUREREASON_ITEM "DeadFailureReason"		/* DEAD Failure reason item */
#define MAIL_RECIPIENTS_ITEM "Recipients"		/* Recipients list item */
#define MAIL_ROUTINGSTATE_ITEM "RoutingState"	/* Routing state */
#define MAIL_ROUTINGSTATEBY_ITEM "RoutingStateBy"	/* Routing state changed by indicator */
#define MAIL_SAVED_FORM_ITEM "MailSavedForm"	/* Delivery report saved form name item */
#define MAIL_BLINDCOPYTO_ITEM "BlindCopyTo"		/* Blind copy to item name */
#define MAIL_DELIVERYPRIORITY_ITEM "DeliveryPriority" /* Delivery priority item name */
#define MAIL_DELIVERYREPORT_ITEM "DeliveryReport"	/* Delivery report request item name */
#define MAIL_DELIVEREDDATE_ITEM "DeliveredDate"	/* Delivered date item name */
#define MAIL_DELIVERYDATE_ITEM "DeliveryDate"	/* Delivery date item name (Confirmation Reports only) */
#define MAIL_FORM_ITEM FIELD_FORM				/* Form name item */
#define MAIL_REPLY_FORM_ITEM	"ReplyForm"		/* Reply form, used for agents */
#define MAIL_CATEGORIES_ITEM "Categories"		/* Categories field */
#define MAIL_FROM_CATEGORIES_ITEM "FromCategories"	/* Sender's Categories field */
#define MAIL_ROUTE_SERVERS_ITEM "RouteServers"	/* List of servers routed thru */
#define MAIL_ROUTE_TIME_ITEM "RouteTimes"		/* List of TIMEDATE_PAIRS describing holding times */
#define MAIL_RECIP_GROUPS_EXP_ITEM "RecipientGroupsExpanded" /* List of recipient group names that have been expanded */
#define MAIL_RETURNRECEIPT_ITEM "ReturnReceipt"	/* Return receipt requested? */
#define MAIL_NAMED_ENCKEY_ITEM "NamedEncryptionKey" /* Named encryption key. */
#define MAIL_ROUTE_HOPS_ITEM "$Hops" 			/* Number of routing hops still allowed. */
#define MAIL_CORRELATION_ITEM "$Correlation" 	/* Arbitrary delivery report correlation value. */
#define MAIL_FORMAT_ITEM "MailFormat"			/* (E)ncapsulated */
												/* (T)ext */
												/* (B)oth */
#define	MAIL_TMP_FROM_ITEM	"TmpFromItem"			/*temp mail from item*/
#define MAIL_IMPORTANCE_ITEM "Importance"		/* Send Priority/Importance item */
#define MAIL_QUALRECIPIENTS_ITEM "QualRecipients"	/* Fully qualified Recipients list item */
#define MAIL_ORIGNALPATH_ITEM "OriginalPath"	/* Original routing path (copy of original message's FromDomain) */
#define MAIL_TRACE_ITEM "$Trace"				/* Identifies message as a trace file */
#define MAIL_ORIG_ITEM	"$Orig"					/* Global Id used for Billing */
#define MAIL_DELIVER_LOOPS_ITEM "$Loops" 		/* Number of forwarding loops still allowed. */
#define MAIL_CLUSTERFAILOVER_ITEM "$MailClusterFailover" /* Server name mail was delivered to after cluster failover */
#define MAIL_UIDL_ITEM	"MailPop3UIDL"
#define MAIL_ENTERSENDTO_ITEM "EnterSendTo"		/* Enter SendTo item name */
#define MAIL_ENTERCOPYTO_ITEM "EnterCopyTo"		/* Enter CopyTo item name */
#define MAIL_ENTERBLINDCOPYTO_ITEM "EnterBlindCopyTo" /* Enter Blind copy to item name */
#define MAIL_INETSENDTO_ITEM "INetSendTo"	  	/* Internet SendTo item name */
#define MAIL_INETCOPYTO_ITEM "INetCopyTo"		/* Internet CopyTo item name */
#define MAIL_INETBLINDCOPYTO_ITEM "INetBlindCopyTo" /* Internet Blind copy to item name */
#define MAIL_ALTSENDTO_ITEM "AltSendTo"	  		/* Alternate Name SendTo item name */
#define MAIL_ALTCOPYTO_ITEM "AltCopyTo"			/* Alternate Name CopyTo item name */
#define MAIL_ALTBLINDCOPYTO_ITEM "AltBlindCopyTo" /* Alternet Blind copy to item name */
#define MAIL_ALTREPLYTO_ITEM "$AltReplyTo"		/* Alternate Reply To item name */
#define MAIL_LANGREPLYTO_ITEM "$LangReplyTo"	/* Language Tag Reply To*/
#define MAIL_ALTPRINCIPAL_ITEM "$AltPrincipal"	/* Alternate Principal item name */
#define MAIL_LANGPRINCIPAL_ITEM "$LangPrincipal"	/* Language tag Principal */
#define MAIL_INETFROM_ITEM "INetFrom"			/* Internet From item name */
#define MAIL_ALTFROM_ITEM "AltFrom"				/* Alternate Name From item name */
#define MAIL_LANGFROM_ITEM "$LangFrom"			/* Language Tag From item name */
#define MAIL_DONOTHOLD_ITEM "$DoNotHold"		/* To prevent holding non-deliverable mail */
#define MAIL_STORAGETO_ITEM "$StorageTo"		/* Storage type To */
#define MAIL_STORAGECC_ITEM "$StorageCc"		/* Storage type Cc */
#define MAIL_STORAGEBCC_ITEM "$StorageBcc"		/* Storage type Bcc */
#define MAIL_LANGTO_ITEM "$LangTo"				/* Language tag To */
#define MAIL_LANGCC_ITEM "$LangCc"				/* Language tag Cc */
#define MAIL_LANGBCC_ITEM "$LangBcc"			/* Language tag Bcc */
#define MAIL_VIEWICON_ITEM			"_ViewIcon"		/* Contains icon number for mail view. Displays in first icon column, from left */
#define MAIL_VIEWICON2_ITEM			"_ViewIcon2"	/* Contains icon number for mail view. Displays in second icon column, from left */
#define MAIL_EXCLUDEFROMVIEW_ITEM	"ExcludeFromView" /* View(s) to exclude note from in mail file */
#define MAIL_PRINCIPAL_ITEM 		"Principal"	/* Mail file from which a note was sent */
#define MAIL_ALTNAMELANGUAGETAGS_ITEM	"$NameLanguageTags"	/* Language Tags item. Single list of Lang Tags for set of current recipients. No Dups. */
#define MAIL_INETREPLYTO_ITEM "$INetReplyTo"   	/* Internet ReplyTo item name */
#define MAIL_INETPRINCIPAL_ITEM "$INetPrincipal" /* Internet Principal item name */
#define MAIL_MAILER_ITEM "$Mailer"				/* Name and version number of the Mailer which sent the message */
#define MAIL_ID_ITEM "$MessageID"				/* Unique ID of this message */
#define ITEM_NAME_DONOTMODIFYINVITEES "tmpDoNotModifyInvitees" /*prevents invitees from being changed on freetime control */
#define	MAIL_CS_INVITE_WAS_DELEGATED_TO_US		"MeetingInviteWasDelegatedToUs" /* Meeting invitation was delegate to us*/
#define	MAIL_CS_INVITE_WAS_DELEGATED_OFF		"MeetingInviteWasDelegatedOff" /* Meeting invitation was delegate to us*/

#define ITEM_NAME_DESIGN_TEMP_COLLATION	"TmpViewDesignCollation"

#define MAIL_APPARENTLY_FROM_ITEM "Apparently_From"
#define MAIL_APPARENTLY_TO_ITEM "Apparently_To"
#define MAIL_DISPOSITION_NOTIFICATION_TO_ITEM "Disposition_Notification_To"
#define MAIL_ORIGINAL_BCC_ITEM "Originalbcc"
#define MAIL_ORIGINAL_CC_ITEM "Originalcc"
#define MAIL_ORIGINAL_FROM_ITEM "OriginalFrom"
#define MAIL_ORIGINAL_PRINCIPAL_ITEM "OriginalPrincipal"
#define MAIL_ORIGINAL_REPLY_TO_ITEM "OriginalReplyTo"
#define MAIL_ORIGINAL_TO_ITEM "OriginalTo"
#define MAIL_RECEIVED_ITEM "Received"
#define MAIL_RESENT_BCC_ITEM "Resent_bcc"
#define MAIL_RESENT_CC_ITEM "Resent_cc"
#define MAIL_RESENT_DATE_ITEM "Resent_Date"
#define MAIL_RESENT_FROM_ITEM "Resent_From"
#define MAIL_RESENT_MESSAGE_ID_ITEM "Resent_Message_ID"
#define MAIL_RESENT_REPLY_TO_ITEM "Resent_Reply_To"
#define MAIL_RESENT_SENDER_ITEM "Resent_Sender"
#define MAIL_RESENT_TO_ITEM "Resent_To"
#define MAIL_RETURN_RECEIPT_TO_ITEM "Return_Receipt_To"
#define MAIL_SENDER_ITEM "Sender"
#define MAIL_MIME_VERSION "MIME_Version"
#define MAIL_IMAP_RFC822_SIZE "IMAP_RFC822Size"
#define MAIL_IMAP_BODYSTRUCTURE "IMAP_BodyStruct"
#define MAIL_CONTENT_TYPE "$Content_Type"
#define MAIL_FIRST_PASS "MailFirstPass"
#define MAIL_MOODS_ITEM "$Moods"
#define MAIL_SENDERTAG_ITEM "SenderTag"
#define MAIL_TMPSENDERTAG_ITEM "tmpSenderTag"
#define MAIL_DISCLAIMED_ITEM "$Disclaimed"

#define MAILBOX_ADMIN_ROLE "[MailboxAdmin]"

#define MAIL_GURU_SCORE_ITEM "DSFScore"
#define MAIL_GURU_RCPT_ITEM "DSFRcpt"
#define MAIL_GURU_TO_ITEM "DSFTo"
#define MAIL_GURU_FROM_ITEM "DSFFrom"
#define MAIL_GURUVOTE_REPORT_ITEM "DSFVoteReport"
#define MAIL_GURUVOTE_VOTE_ITEM "DSFVoteVote"

#define MAIL_DNSBL_FILTER_ITEM "$DNSBLSite"			/* Blacklist tag */
#define MAIL_DNSWL_FILTER_ITEM "$DNSWLSite"			/* Whitelist tag */
#define MAIL_FILTER_ACTIONS_ITEM "$FilterActions"
#define MAIL_FILTER_BY_ITEM "$FilterBy"
#define MAIL_SENDTO_NONRESPONDERS "Non_Responders"
#define	MAIL_CS_LOOPING_FACTOR	"LoopingFactor"	/*temp note item to identify for the template
														 whether a notice has "looped" (I delegated this
														 meeting off and it's come back) or cross-delegate
														 (two people have delegated an invitation for the
														 same meeting to me) */
#define MAIL_REPLYFORWARD_ITEM "$RespondedTo"	/* text item, value = 1 if replied, 2 = forwarded, 3 = both */
#define MAIL_REPLYFORWARD_NONE			"0"
#define MAIL_REPLYFORWARD_REPLY			"1"
#define MAIL_REPLYFORWARD_FORWARD		"2"
#define MAIL_REPLYFORWARD_REPLYFORWARD	"3"

/* POP3 view names */
#define MAIL_POP3_UIDL_VIEW	"($POP3UIDL)"

/*	Appointment form item names */

#define MAIL_APPT_BUSYNAME_ITEM 	"$BusyName"		/* Person/resource the appointment is for */
#define MAIL_APPT_STARTTIME_ITEM	"StartDateTime"	/* Start time of appointment */
#define MAIL_APPT_ENDTIME_ITEM		"EndDateTime"	/* End time of appointment */
#define MAIL_APPT_SEQUENCE_ITEM		"$Sequence"		/* Sequence # of updates */
#define MAIL_APPT_BUSYPRIORITY_ITEM "$BusyPriority"  /* Busy/Penciled-in/etc. */
#define MAIL_APPT_ATTENDEE_ITEM				"Attendees"	/* List of people for which the following items apply */
#define MAIL_APPT_ATTENDEE_STATE_ITEM		"ORGState"	/* Chairman, attendee, resource */
#define MAIL_APPT_ATTENDEE_STATUS_ITEM		"ORGStatus"	/* Deleted, etc. */
#define MAIL_APPT_ATTENDEE_OPTIONAL_ITEM	"OPTIONAL"	/* If TRUE, attendee is optional */
#define MAIL_APPT_PREV_BUSYNAME_ITEM 		"$PrevBusyName"		/* Previous Person/resource(s) the appointment is for */
#define RESOURCE_NAMESPACE_RESOURCES		"($RLookup)"		/* View containing resources sorted by name */
#define RESOURCE_NAMESPACE_RESERVATIONS		"($FindReservation)"	/* View containing reservations by name and apptunid */
#define RESOURCE_NAMESPACE_WAITINGAPPROVAL  "($FindWaitingApproval)" /* Like $FindReservation but contains those waiting for approval */
#define MAIL_REPEAT_LOOKUP_NAMESPACE		"($RepeatLookup)"	/* View containing repeat replies/updates */
#define MAIL_REPEAT_INSTANCE_LOOKUP_NAMESPACE		"($RepeatInstanceLookup)"	/* View containing repeat instances */
#define MAIL_JOURNALLING_FLAG_ITEM			"$JournalResponsibility"  /* If present, message should be journalled.  The
																	   * value indicates if it should be journalled locally
																	   * or on another server.	  */


#define JOURNALLING_FLAG_DONT_JOURNAL        	0
#define JOURNALLING_FLAG_LOCAL_JOURNAL		 	1
#define JOURNALLING_FLAG_REMOTE_JOURNAL		 	2
#define JOURNALLING_FLAG_JOURNAL_COMPLETE	 	3
#define JOURNALLING_FLAG_DONT_JOURNAL_STR   	"0"
#define JOURNALLING_FLAG_LOCAL_JOURNAL_STR  	"1"
#define JOURNALLING_FLAG_REMOTE_JOURNAL_STR		"2"
#define JOURNALLING_FLAG_JOURNAL_COMPLETE_STR	"3"
#define MAIL_JOURNALLING_TEMPLATE_NAME      	"mailjrn.ntf"

#define MAIL_ROUTINGSTATEBY_ROUTER_STR			"1"
#define MAIL_ROUTINGSTATEBY_RULE_STR			"2"

/*  Internet Message Note items and definitions (for Internet Mail and Internet News) */

#define IMSG_RFC822_FILENAME_ITEM 			"$IntMailMsgFileName" /*  Name of the attachment that contains the BLOB.    */
																  /*  This contains the string 'Mail' because of   	    */
																  /*  history and changing it now would affect existing */
																  /*  databases and code.								*/

#define IMSG_ITEMIZE_ERROR_ITEM				"$ItemizeError"		/* non-zero if an error occurred during itemize.  if an */
																/* error occurs, the original internet message is at-   */
																/* tached to the message (IMSG_RFC822_FILENAME_ITEM)    */
																/* and the error text is stuffed into the message's body*/
																/* item.												*/

#define MAIL_TMP_MESSAGE_ID_DOMAIN			"LocalDomain"			/* Used as a place holder on constructed MessageIDs */

#define MAIL_MIME_CHARSET					"MIMEMailCharset"	/* field with charset value */
#define MAIL_MIME_HEADER_CHARSET			"MIMEMailHeaderCharset"	/* Charset for headers: upper 16 bit: charset, lower 16 bit: Encoding */

/*
 *	Person documents contain an item (MessageStorage) which indicates each person's preference for how
 *	messages delivered to them will be stored. Definitions of the valid values for this item are named
 *	IMSG_PREFERENCE_XXX.
 *
 *	Mail messages contain an item ($MessageStorage) which indicates the actual storage format of the
 *	message if it has been converted. Definitions of the valid values for this item are named
 *	IMSG_STORAGE_XXX. This item is deleted from messages when they are delivered by the router.
 *
 *	Routines which store messages or convert messages accept arguments which specify the required
 *	storage format. These routines accept values named IMSG_STORAGE_XXX.
 *
 *	As a performance optimization for the router, there is another item which can exist on a note
 *	which indicates that the note is stored in native MIME. This item is $NoteHasNativeMIME.
 */

#define IMSG_STORAGE_ITEM 					"$MessageStorage"	/* Describes how mail is stored.  See IMSG_STORAGE_XXX */

#define IMSG_STORAGE_UNKNOWN				0xFF /* can't find storage type */
#define IMSG_STORAGE_CDRECORDS				0 	/* Store as cd records only */
#define IMSG_STORAGE_CDRECORDS_AND_RFC822	1 	/* Store as cd records & attachment of original message */
#define IMSG_STORAGE_RFC822					2 	/* Store original message as attachment (no cdrecords) */
#define IMSG_STORAGE_NATIVE_MIME			3	/* Store headers & parts as separate items (without conversion) or CD */
#define IMSG_STORAGE_HAIKU					4	/* Store for Haiku users only */
/* Be sure to update count (IMSG_STORAGE_FORMATS) if new formats are added */

#define IMSG_STORAGE_FORMATS				5	/* Count of known storage formats */

#define IMSG_PREFERENCE_UNKNOWN				0xFF /* can't find preference type */
#define IMSG_PREFERENCE_NOTES				0	/* V4->CD, V5->CD or MIME */
#define IMSG_PREFERENCE_NOTES_AND_MIME		1	/* V4->CD and MIME blob, V5->CD or MIME */
#define IMSG_PREFERENCE_MIME				2	/* V4->MIME blob only, V5->Native MIME */
#define IMSG_PREFERENCE_HAIKU				3	/* V4->N/A, V5->Native MIME+Haiku fields */

#define IMSG_PREFERENCE_FORMATS				4	/* Count of known preference formats */

#define IMSG_RFC822_MSG_SIZE_ITEM			"$RFC822MessageSize"	/* Message size field (used by POP3 client) */
#define IMSG_RFC822_MSG_FILE_NAME			"$RFC822.eml"			/* Attached "MIME BLOB" name */
#define IMSG_SMTP_ORIGINATOR_ITEM			"SMTPOriginator"		/* RFC821 message originator */
#define IMSG_SMTP_RET_HDRS_ITEM				"SMTPReturnHdrs"		/* If HDRS, return only headers with non DSN */
#define IMSG_SMTP_RET_HDRS_HDRS				"HDRS"
#define IMSG_SMTP_ENVID_ITEM				"SMTPEnvid"				/* If ESMTP DSN is supported, ENVID to relay */
#define IMSG_SMTP_DSN_RCPTS_ITEM			"SMTPDSNRecipients"		/* If ESMTP DSN is supported, per rcpt info */
#define IMSG_SMTP_DSN_DELIVERY_STATUS		"SMTPDSNDeliveryStatus"	/* RFC822 formatted delivery status */
#define IMSG_SMTP_DSN_DELIVERY_REASON		"SMTPDSNDeliveryReason"	/* RFC822 free form (i.e., prose) delivery information */
#define IMSG_SMTP_DSN_RECEIVED_ITEM			"SMTPRcvd"				/* Received items from the embedded rfc822 message in a Delivery Status Report */
#define IMSG_SMTP_DSN_TYPE_ITEM				"SMTPDSNType"			/* Type of delivery status report (failed, delayed, relayed etc. */
#define IMSG_SMTP_DSN_DATE_ITEM				"SMTPDSNDate"			/* Posted Date of the delivery status report */
#define IMSG_SMTP_DSN_TYPE_FAILED_STR		"0"
#define IMSG_SMTP_DSN_TYPE_DELAYED_STR		"1"
#define IMSG_SMTP_DSN_TYPE_DELIVERED_STR	"2"
#define IMSG_SMTP_DSN_TYPE_RELAYED_STR		"3"
#define IMSG_SMTP_DSN_TYPE_EXPANDED_STR		"4"

#define ITEM_MIMEHEADERS					"$MIMEHeaders"
#define ITEM_MIMETRACK						"$MIMETrack"
#define ITEM_MIME_ENCAPSULATED				"$MIMEEncapsulated"
#define ITEM_IS_NATIVE_MIME					"$NoteHasNativeMIME"
#define ITEM_NOT_FROM_NOTES					"$SMTPNotFromNotes"
#define ITEM_KEEP_NOTES_ITEMS				"$SMTPKeepNotesItems"

#define ITEM_HEADERS_CONVERTED				"$ExportHeadersConverted"

/*	Calendar profile form related */

/*	On a profile are eight fields that relate to when a users is available:

	The AvailableDays item corresponds to the checkboxes allowing the user
		to choose which days of the week they work.  The resulting item
		will be a textlist which contains the days of the week that they
		work.
	For the days of the week that the user does work, the time ranges are
		read and for each day and assimilated into one item in the busytime
		database that represents the days and time the user or resource
		is available for scheduling.
	The item is encoded as follows:
		DateTime.Lower.Date	- a day that corresponds to the day of the week
					  .Time - start time of an interval for that day the user works
				.Upper.Date - the same day as the Lower
					  .Time - end time of an interval for that day the user works

		These values will repeat for the various times during the day for
			every day of the week that the user works.

	(i.e.	The default configuration will be: (for Monday through Friday)

		A Monday,	9:00 AM - A Monday, 12:00 PM
		A Monday,	1:00 PM - A Monday, 5:00 PM)

	NOTE:	The encoding does not care which real date is used, it just
			determines what day of the week the date correspondes to..
*/

#define MAIL_CALENDAR_PROFILE_WORKDAYS_ITEM				"$WorkDays"
#define MAIL_CALENDAR_PROFILE_WORK_AVAILABLEDAYS_ITEM	"$AvailableDays"
#define MAIL_CALENDAR_PROFILE_TZDISPLAY_ITEM			"fDisplayAltTimeZone"
#define MAIL_CALENDAR_PROFILE_CURTIMEZONE_ITEM			"CurrentTimeZone"
#define MAIL_CALENDAR_PROFILE_ALTTIMEZONE_ITEM			"AltTimeZone"
#define MAIL_CALENDAR_PROFILE_CURTIMEZONELBL_ITEM		"CurrentTZLabel"
#define MAIL_CALENDAR_PROFILE_ROLLINGVIEW_ITEM			"$RollingView"
#define MAIL_CALENDAR_PROFILE_ALTTIMEZONELBL_ITEM		"AltTZLabel"
#define MAIL_CALENDAR_PROFILE_OWNER_ITEM				"Owner"				/* Owner field in calendar profile form */
#define MAIL_CALENDAR_PROFILE_ALTOWNER_ITEM				"AltOwner"			/* AltOwner field in calendar profile form */
#define MAIL_CALENDAR_PROFILE_LANGOWNER_ITEM			"$LangOwner"	    	/* LangOwner field in calendar profile form */
#define MAIL_CALENDAR_PROFILE_WORK_WEEKDAYPREFIX		"$Times"
#define MAIL_CALENDAR_PROFILE_WORK_WEEKDAYSUFFIX		""
#define MAIL_CALENDAR_PROFILE_WORK_SUNDAY_ITEM			"$Times1"
#define MAIL_CALENDAR_PROFILE_WORK_MONDAY_ITEM			"$Times2"
#define MAIL_CALENDAR_PROFILE_WORK_TUESDAY_ITEM			"$Times3"
#define MAIL_CALENDAR_PROFILE_WORK_WEDNESDAY_ITEM 		"$Times4"
#define MAIL_CALENDAR_PROFILE_WORK_THURSDAY_ITEM		"$Times5"	
#define MAIL_CALENDAR_PROFILE_WORK_FRIDAY_ITEM			"$Times6"
#define MAIL_CALENDAR_PROFILE_WORK_SATURDAY_ITEM		"$Times7"
#define MAIL_CALENDAR_PROFILE_ENABLE_ALARMS_ITEM		"EnableAlarms"
#define MAIL_CALENDAR_PROFILE_APPROVALLIST_ITEM			"$ApprovalList"
#define MAIL_CALENDAR_PROFILE_DEFAULTDURATION_ITEM		"DefaultDuration"
#define MAIL_CALENDAR_PROFILE_EXCLUDEFROMALL_ITEM	    "ExcludeFromAll"
#define MAIL_CALENDAR_PROFILE_EXCLUDEFROMSENT_ITEM	    "ExcludeFromSent"
#define MAIL_CALENDAR_PROFILE_FORWARDINVITEENOTIFICATIONTO_ITEM "CalForwardInviteeNotificationTo"
#define MAIL_CALENDAR_PROFILE_FORWARDCHAIRNOTIFICATIONTO_ITEM "CalForwardChairNotificationTo"
#define MAIL_CALENDAR_PROFILE_FORWARDNOTIFICATIONPRIVATEMODE_ITEM "CalForwardPrivateMode"
#define MAIL_CALENDAR_PROFILE_AUTOPROCESSLIST_ITEM			"AutoprocessUserList"
#define MAIL_CALENDAR_PROFILE_AUTOPROCESSTYPE_ITEM			"AutoprocessType"
#define MAIL_CALENDAR_PROFILE_AUTOPROCESSFORWARDTO_ITEM		"AutoprocessForwardTo"
#define MAIL_CALENDAR_PROFILE_AUTOPROCESSMTGS_ITEM			"AutoprocessMeetings"
#define MAIL_CALENDAR_PROFILE_AUTOPROCESSCONFLICTOPTIONS_ITEM "AutoprocessConflictOptions"
#define MAIL_CALENDAR_PROFILE_AUTOPROCESS_ITEMCONFLICTOPTIONS_ITEM "AutoprocessItemConflictOptions"
#define MAIL_CALENDAR_PROFILE_V45AUTOPROCESSLIST_ITEM		"MeetingsPeople"
#define MAIL_CALENDAR_PROFILE_AUTOREMOVEFROMINBOX_ITEM		"AutoRemoveFromInbox"
#define MAIL_CALENDAR_PROFILE_ALARMSENABLED_ITEM			"EnableAlarms"
#define MAIL_CALENDAR_PROFILE_ALARMAPPTLEAD_ITEM			"AppointmentLead"
#define MAIL_CALENDAR_PROFILE_ALARMANNLEAD_ITEM				"AnniversaryLead"
#define MAIL_CALENDAR_PROFILE_ALARMEVENTLEAD_ITEM			"EventLead"
#define MAIL_CALENDAR_PROFILE_ALARMREMINDERLEAD_ITEM		"ReminderLead"
#define MAIL_CALENDAR_PROFILE_ALARMTODOLEAD_ITEM			"TaskLead"
/* Tasks are now called ToDos.  Define the old deprecated name as alias to new */
#define MAIL_CALENDAR_PROFILE_ALARMTASKLEAD_ITEM			MAIL_CALENDAR_PROFILE_ALARMTODOLEAD_ITEM
#define MAIL_CALENDAR_PROFILE_ALARMAPPTENABLED_ITEM			"SetAlarmAppointment"
#define MAIL_CALENDAR_PROFILE_ALARMANNENABLED_ITEM			"SetAlarmAnniversary"
#define MAIL_CALENDAR_PROFILE_ALARMEVENTENABLED_ITEM		"SetAlarmEvent"
#define MAIL_CALENDAR_PROFILE_ALARMREMINDERENABLED_ITEM		"SetAlarmReminder"
#define MAIL_CALENDAR_PROFILE_ALARMTODOENABLED_ITEM			"SetAlarmTask"
/* Tasks are now called ToDos.  Define the old deprecated name as alias to new */
#define MAIL_CALENDAR_PROFILE_ALARMTASKENABLED_ITEM			MAIL_CALENDAR_PROFILE_ALARMTODOENABLED_ITEM			
#define MAIL_CALENDAR_PROFILE_ALARMITEM_DISABLED			'0'
#define MAIL_CALENDAR_PROFILE_ALARMITEM_ENABLED			    '1'
#define MAIL_CALENDAR_PROFILE_CONFLICT_APPOINTMENT_ITEM		"ConflictMeeting"
#define MAIL_CALENDAR_PROFILE_CONFLICT_ANNIVERSARY_ITEM		"ConflictAnniversary"
#define MAIL_CALENDAR_PROFILE_CONFLICT_EVENT_ITEM			"ConflictEvent"
#define MAIL_CALENDAR_PROFILE_CONFLICTITEM_DISABLED			'0'
#define MAIL_CALENDAR_PROFILE_CONFLICTITEM_ENABLED			'1'
#define MAIL_CALENDAR_PROFILE_PREVENTREPLIESINBOX_ITEM		"PreventRepliesFromInbox"
#define MAIL_CALENDAR_PROFILE_CALENDARMANAGER_ITEM			"CalendarManager"
#define MAIL_CALENDAR_PROFILE_MANAGECALENDARSFOR_ITEM		"ManageCalendarsFor"
#define MAIL_CALENDAR_PROFILE_BUSYTIMEHARVESTOPTOUT_ITEM	"BusyTimeHarvestOptOut"
#define MAIL_CALENDAR_PROFILE_BUSYTIMEHARVESTOPTOUT_DISABLED		'0'
#define MAIL_CALENDAR_PROFILE_BUSYTIMEHARVESTOPTOUT_DISABLED_STR	"0"
#define MAIL_CALENDAR_PROFILE_BUSYTIMEHARVESTOPTOUT_ENABLED			'1'
#define MAIL_CALENDAR_PROFILE_BUSYTIMEHARVESTOPTOUT_ENABLED_STR		"1"
#define MAIL_CALENDAR_PROFILE_ALLOWBUSYDETAILSACCESS_ITEM	"AllowBusyDetailsAccess"
#define MAIL_CALENDAR_PROFILE_DETAILINDEX_ITEM              "DetailIndex"
#define MAIL_CALENDAR_PROFILE_DETAILPREFIX		            "Detail_"
#define MAIL_CALENDAR_PROFILE_EXTERNAL_ADDRESS              "ExternalAddress"
#define MAIL_CALENDAR_PROFILE_ALWAYS_AVAILABLE              "AlwaysAvailable"
#define MAIL_CALENDAR_PROFILE_CONTROVERSIALFIELDS_ITEM      "ControversialFields"	/* Controversial Calendar details to harvest */


/*	Mail delegation profile form related */

#define MAIL_DELEGATION_PROFILE_OWNER_ITEM			"Owner"				
#define MAIL_DELEGATION_ALLOWBUSYACCESS_ITEM		"AllowBusyAccess"
#define MAIL_DELEGATION_ALLOWDETAILACCESS_ITEM		"AllowDetailAccess"
#define MAIL_DELEGATION_DISABLEALLDETAILACCESS_ITEM	"DisableAllDetailAccess"

#define MAIL_UIDL_ITEM	"MailPop3UIDL"

/*BEGIN_SAMETIME*/
/* Online meeting (Sametime) Calendaring & Scheduling related item names */
#define MAIL_CS_ST_ONLINEPLACETORESERVE_ITEM	   "OnlinePlaceToReserve"
#define MAIL_CS_ST_ONLINEPLACE_ITEM	               "OnlinePlace"

#define MAIL_CS_ST_SAMETIMESERVER_ITEM             "SametimeServer"
#define MAIL_CS_ST_ONLINEMEETINGFLAG_ITEM          "OnlineMeetingFlag"
#define MAIL_CS_ST_MEETINGTYPE_ITEM                "MeetingType"
#define MAIL_CS_ST_RESTRICTEDTOINVITELIST_ITEM     "RestrictToInviteList"
#define MAIL_CS_ST_RESTRICTEDTOINVITEFLAG_ITEM     "RestrictAttendence"
#define MAIL_CS_ST_ALLOWATTACHMENTVIEWING_ITEM     "AllowAttachmentViewing"
#define MAIL_CS_ST_WHITEBOARDCONTENT_ITEM          "WhiteBoardContent"
#define MAIL_CS_ST_APPTUNIDURL_ITEM                "ApptUNIDURL"
#define MAIL_CS_ST_CONFERENCEDATABASE_ITEM         "ConferenceDatabase"
#define MAIL_CS_ST_ONLINEMEETINGFLAG_ITEM          "OnlineMeetingFlag"
#define MAIL_CS_ST_FILELIST_ITEM                   "FileList"
#define MAIL_CS_ST_SENDATTACHMENTS_ITEM            "SendAttachments"
#define MAIL_CS_ST_AUDIOVIDEOFLAGS_ITEM            "AudioVideoFlags"
#define MAIL_CS_ST_SAMETIMESERVERNETADDRESS_ITEM   "SametimeServerNetAddress"
#define MAIL_CS_ST_MODERATOR_ITEM                  "Moderator"
#define MAIL_CS_ST_PRESENTERS_ITEM                 "Presenters"
#define MAIL_CS_ST_MEETINGPASSWORD_ITEM            "MeetingPassword"
#define MAIL_CS_ST_ONLINEMEETING_ITEM              "OnlineMeeting"

/* Saved transcript related item names */
#define	IM_TRANSCRIPT_ITEM							"$IMTranscript"
#define	IM_ORIGINATOR_ITEM							"$IMOriginator"

/*END_SAMETIME*/




/* Calendaring & Scheduling related item names */
#define MAIL_CS_PERSONAL_NOTES_ITEM		"Notes"				/* Personal meeting notes item */
#define MAIL_CS_KEEPPOSTED_ITEM 		"KeepPosted"		/* Determines if participant wants to be kept notified of updates */
#define MAIL_CS_KEEPPOSTED_DISABLED		'0'
#define MAIL_CS_KEEPPOSTED_ENABLED		'1'

#define MAIL_CS_SEQUENCENUM_ITEM 		"SequenceNum"		/* Sequence # of event notice */
#define MAIL_CS_UPDATE_SEQUENCENUM_ITEM	"UpdateSeq"			/* Update sequence # of event */

#define MAIL_CS_NOTICETYPE_ITEM 		"NoticeType"		/* Type of notice being sent */
#define MAIL_CS_ORIGINAL_NOTICETYPE_ITEM "OriginalNoticeType"

#define MAIL_CS_ASSIGNSTATE_ITEM 		"AssignState"		/* Used by todos for communicating status */
#define MAIL_CS_CSVERSION_ITEM			"$CSVersion"		
#define MAIL_CS_APPOINTMENTTYPE_ITEM	"AppointmentType"
#define MAIL_CS_TODOTYPE_ITEM			"TaskType" 			/* The type of todo. Item is "TaskType" for backwards compat  */
/* Tasks are now called ToDos.  Define the old deprecated name as alias to new */
#define MAIL_CS_TASKTYPE_ITEM			MAIL_CS_TODOTYPE_ITEM
#define	MAIL_CS_CHAIR_ITEM				"Chair"
#define MAIL_CS_CHAIRDOMAIN_ITEM		"ChairDomain"
#define MAIL_CS_DELEGATOR_ITEM			"Delegator"
#define MAIL_CS_ALTDELEGATOR_ITEM		"AltDelegator"
#define MAIL_CS_LANGDELEGATOR_ITEM  	"$LangDelegator"
#define MAIL_CS_DELEGATETO_ITEM		    "Delegee"
#define MAIL_CS_REQUIREDATTENDEES_ITEM	"RequiredAttendees"
#define MAIL_CS_OPTIONALATTENDEES_ITEM	"OptionalAttendees"
#define MAIL_CS_FYIATTENDEES_ITEM		"FYIAttendees"
#define MAIL_CS_REQUIREDASSIGNEES_ITEM  "AssignedTo"
#define MAIL_CS_OPTIONALASSIGNEES_ITEM  "OptionalAssignedTo"
#define	MAIL_CS_ALTCHAIR_ITEM			"AltChair"
#define	MAIL_CS_LANGCHAIR_ITEM	    	"$LangChair"
#define MAIL_CS_FYIASSIGNEES_ITEM		"FYIAssignedTo"
#define MAIL_CS_ROOMTORESERVE_ITEM		"RoomToReserve"
#define MAIL_CS_ROOM_ITEM				"Room"
#define MAIL_CS_RESOURCESTORESERVE_ITEM	"Resources"
#define MAIL_CS_RESOURCES_ITEM			"RequiredResources"
#define MAIL_CS_BOOKFREETIME_ITEM		"BookFreeTime"
#define MAIL_CS_BOOKFREETIME_DISABLED	'0'
#define MAIL_CS_BOOKFREETIME_ENABLED	'1'
#define MAIL_CS_REMOVEDNAMES_ITEM		"Uninvited"
#define MAIL_CS_TOREMOVENAMES_ITEM		"tmpRemoveNames"
#define MAIL_CS_TOREMOVERRNAMES_ITEM	"tmpRemoveRRNames"
#define MAIL_CS_TMPOWNER_ITEM			"tmpOwner"
#define MAIL_CS_TOPIC_ITEM				"Topic"
#define MAIL_CS_DUESTATE_ITEM			"DueState"
#define MAIL_CS_RESOURCENAME_ITEM		"ResourceName"
#define MAIL_CS_STATUS_ITEM			    "Status"
#define MAIL_CS_STARTDATE_ITEM			"StartDate"
#define MAIL_CS_STARTTIME_ITEM			"StartTime"
#define MAIL_CS_STARTTIMEZONE_ITEM		"StartTimeZone"
#define MAIL_CS_ENDDATE_ITEM			"EndDate"
#define MAIL_CS_ENDTIME_ITEM			"EndTime"
#define MAIL_CS_ENDTIMEZONE_ITEM 		"EndTimeZone"
#define MAIL_CS_DUEDATETIME_ITEM		"DueDateTime"
#define MAIL_CS_APPENDSTARTTIME_ITEM	"AppendStartTime"
#define MAIL_CS_APPENDENDTIME_ITEM 		"AppendEndTime"
#define MAIL_CS_APPTUNID_ITEM			"ApptUNID"
#define MAIL_CS_CALENDARDATETIME_ITEM	"CalendarDateTime"
#define MAIL_CS_TIMERANGE_ITEM			"TimeRange"
#define MAIL_CS_NEWDATE_ITEM			"NewDate"
#define MAIL_CS_NEWTIMERANGE_ITEM		"NewTimeRange"
#define MAIL_CS_NEWROOM_ITEM			"NewRoom"
#define MAIL_CS_NEWSTARTDATE_ITEM		"NewStartDate"
#define MAIL_CS_NEWSTARTTIME_ITEM		"NewStartTime"
#define MAIL_CS_NEWENDDATE_ITEM			"NewEndDate"
#define MAIL_CS_NEWENDTIME_ITEM			"NewEndTime"
#define MAIL_CS_NEWSTARTTIMEZONE_ITEM	"NewStartTimeZone"
#define MAIL_CS_NEWENDTIMEZONE_ITEM		"NewEndTimeZone"
#define MAIL_CS_BROADCAST_ITEM			"Broadcast"
#define MAIL_CS_BROADCAST_DISABLED	'0'
#define MAIL_CS_BROADCAST_ENABLED		'1'
#define MAIL_CS_DUEDATE_ITEM			"DueDate"
#define MAIL_CS_FLAGS_ITEM				"$CSFlags"
#define MAIL_CS_MAILINDB_ITEM			"MailInDatabaseList"
#define MAIL_CS_GROUP_REFRESHMODE_ITEM	"$GroupScheduleRefreshMode"
#define MAIL_CS_REFRESH_OPTIMAL			'0'
#define MAIL_CS_REFRESH_FULL		 	'1'
#define MAIL_CS_LEGEND_ENABLED_ITEM 	"$GroupScheduleShowLegend"
#define MAIL_CS_LEGEND_DISABLED			'0'
#define MAIL_CS_LEGEND_ENABLED			'1'
#define MAIL_CS_COMPLETEDDATETIME_ITEM	"CompletedDateTime"
#define MAIL_CS_PREVENTREPLIES_ITEM		"$PreventReplies"
#define MAIL_CS_PREVENTREPLIES_DISABLED	'0'
#define MAIL_CS_PREVENTREPLIES_ENABLED	'1'
#define MAIL_CS_RESERVATION_PURPOSE_ITEM	"Purpose"
#define MAIL_CS_PREVDELEGEE_ITEM		"PrevDelegee"
#define MAIL_CS_PREVENTDELEGATION_ITEM	"PreventDelegate"
#define MAIL_CS_PREVENTDELEGATION_DISABLED	'0'
#define MAIL_CS_PREVENTDELEGATION_ENABLED   '1'
#define MAIL_CS_PREVENTCOUNTER_ITEM		"PreventCounter"
#define MAIL_CS_RESOURCEOWNER_ITEM		"ResourceOwner"
#define MAIL_CS_ALTREQUIRED_ITEM		"AltRequiredNames"
#define MAIL_CS_ALTOPTIONAL_ITEM		"AltOptionalNames"
#define MAIL_CS_ALTFYI_ITEM				"AltFYINames"
#define MAIL_CS_ALTDELEGATETO_ITEM		"AltDelegeeName"
#define MAIL_CS_LANGDELEGATETO_ITEM		"$LangDelegee"
#define MAIL_CS_INETREQUIRED_ITEM		"INetRequiredNames"
#define MAIL_CS_INETOPTIONAL_ITEM		"INetOptionalNames"
#define MAIL_CS_INETFYI_ITEM			"INetFYINames"
#define MAIL_CS_STORAGEREQUIRED_ITEM	"StorageRequiredNames"
#define MAIL_CS_STORAGEOPTIONAL_ITEM	"StorageOptionalNames"
#define MAIL_CS_STORAGEFYI_ITEM			"StorageFYINames"
#define MAIL_CS_STATUS_UPDATE           "StatusUpdate" /* contains comment for accept/decline/delegate with comment */
#define MAIL_CS_TRACK_ITEM				"$CSTrack"
#define MAIL_CS_MANAGEDFOR_ITEM			"ManagedFor"
#define MAIL_CS_AUTOPROCESSINFO_ITEM	"$AutoprocessInfo"
#define MAIL_CS_ACTIONTYPE_ITEM			"ActionType" /* Contains action in progress for Managed Update */
#define MAIL_CS_MGRSECTION_ITEM			"CalendarMgrSection"
#define MAIL_CS_COPYITEMS               "$CSCopyItems"
#define MAIL_CS_SENDMEMOFROM_ITEM		"$CSMemoFrom"
#define MAIL_CS_ORIGINAL_DELEGATOR		"OriginalDelegator"
#define MAIL_CS_DELEGATETO_LIST			"DelegateToList"

#define MAIL_CS_WI_MODIFIED_ITEM			"$WFModified"	/* Used to indicate which Watched Items have been modified*/
#define MAIL_CS_WISL_ITEM					"$CSWISL"		/* Watched Item Sequence List */
#define MAIL_CS_WATCHEDITEMS_ITEM			"$WatchedItems"	/* Tells us which items are supposed to be watched */
#define MAIL_CS_NO_TZONE_ITEM				"NoTZInfo"		/* Used to indicate that there is no Timezone info on the note (TZ info was introduced in R6) */

#define MAIL_CS_MINIVIEW_ITEM			"$MiniView"			/* if one, notice appears in the miniview */
															/* if zero or empty value, notice does not appear in the miniview */
#define MAIL_CS_MINIVIEW_YES			'1'
#define MAIL_CS_MINIVIEW_NO				'0'

#define MAIL_CS_FROMPREFERREDLANG_ITEM	"$FromPreferredLanguage"

#define	MAIL_CS_RNR_REQUEST_STATUS_ITEM	"RQStatus"			/* R7: R&R Request status item */
#define MAIL_CS_RNR_REQUEST_TENTATIVE	'T'					/* R7: Tenatively booked; awaiting RnRMgr approval */
#define MAIL_CS_RNR_REQUEST_ACCEPTED	'A'					/* R7: Accepted; approved and processed by RnRMgr */
#define MAIL_CS_RNR_REQUEST_REJECTED	'R'					/* R7: Rejected; denied by RnRMgr */
#define MAIL_CS_RNR_REQUEST_PROCESSED	'P'					/* R7: Processed; belongs outside R&R so it was processed by RnRMgr */

#define	MAIL_CS_RNR_BLOCKER_ITEM		"$CSBlocker"		/* R7: R&R Request 'blocker' indicator item, put on R7 request that 
															** blocks other reservations beyond a particular date.
															*/

#define RNR_SERVER_PROFILE_FORM			"ServerProfile"		/* R7: R&R Server Profile form name */

/* The following are new R&R related items / values for D7 related to the new
** RnRMgr and R&R autoprocessing and server status.  They are used on R&R server
** profile docs stored in the busytime dB OR in CLDBDIR.NSF
*/
#define RNR_SERVER_FULLYINIT_ITEM		"FullyInitd"		/* R7: R&R Server fully initialized status */
#define RNR_SERVER_FULLYINIT_YES		'Y'					/* R7: R&R Server is "fully initialized" */
#define RNR_SERVER_FULLYINIT_NO			'N'					/* R7: R&R Server is NOT "fully initialized" */

#define RNR_SERVER_TOOKCONTROL_ITEM		"TookControl"		/* R7: TIMEDATE when R&R Server took autoprocessing control */
#define RNR_SERVER_RETURNCONTROL_ITEM	"ReturnControl"		/* R7: TIMEDATE when R&R Server returned autoprocessing control 
															** to the Home Server.
															*/
#define RNR_DECLINE_REASON_ITEM			"DeclineReason"		/* R5(?): Item indication why, typically, owner rejected the reservation.
															** For R7 we now have RnRMgr adding text such as 'Invalid interval...', etc
															*/

/* C&S Repeating Entry item names */

#define MAIL_CS_ORGREPEAT_ITEM					"OrgRepeat"
#define MAIL_CS_ORGTABLE_ITEM					"OrgTable"
#define MAIL_CS_ORGCONFIDENTIAL_ITEM			"OrgConfidential"
#define MAIL_CS_REPEATS_ITEM					"Repeats"
#define MAIL_CS_REPEAT_DATES_ITEM				"RepeatDates"
#define MAIL_CS_REPEAT_END_DATES_ITEM			"RepeatEndDates" 	/* new with Notes6 */
#define MAIL_CS_REPEAT_INSTANCE_DATES_ITEM		"RepeatInstanceDates"
#define MAIL_CS_REPEAT_BASE_DATE_ITEM			"OriginalStartDate"
#define MAIL_CS_REPEAT_START_DATE_ITEM			"RepeatStartDate"
#define MAIL_CS_REPEAT_FOR_ITEM				    "RepeatFor"
#define MAIL_CS_REPEAT_FORUNIT_ITEM			    "RepeatForUnit"
#define MAIL_CS_REPEAT_UNIT_ITEM				"RepeatUnit"
#define MAIL_CS_REPEAT_ADJUST_ITEM				"RepeatAdjust"
#define MAIL_CS_REPEAT_UNTIL_ITEM				"RepeatUntil"
#define MAIL_CS_REPEAT_INTERVAL_ITEM			"RepeatInterval"
#define MAIL_CS_REPEAT_CUSTOM_ITEM				"RepeatCustom"
#define MAIL_CS_REPEAT_WEEKENDS_ITEM			"RepeatWeekends"
#define MAIL_CS_REPEAT_HOW_ITEM				    "RepeatHow"
#define MAIL_CS_REPEAT_STARTFROMEND_ITEM		"RepeatFromEnd"
#define MAIL_CS_REPEAT_IDS_ITEM		            "RepeatIds"
#define MAIL_CS_REPEAT_LOOKUP_ITEM				"$RepeatLookup"
#define MAIL_CS_REPEAT_CHANGEWHICH_ITEM			"RescheduleWhich"
#define MAIL_CS_REPEAT_CHANGEINSTANCEDATES_ITEM "RescheduleInstanceDates" /* new with Notes6 */
#define MAIL_CS_REPEAT_CHANGESTARTDATETIMES_ITEM "RescheduleStartDateTimes" /* new with Notes6 */
#define MAIL_CS_REPEAT_CHANGEENDDATETIMES_ITEM  "RescheduleEndDateTimes" /* new with Notes6 */
#define MAIL_CS_REPEAT_ORIGINALENDDATE_ITEM		"OriginalEndDate"
#define MAIL_CS_REPEAT_PARENTREPEATDATES_ITEM	"ParentRepeatDates"
#define MAIL_CS_REPEAT_PARENTREPEATINSTANCEDATES_ITEM	"ParentRepeatInstanceDates"
#define MAIL_CS_REPEAT_INSTANCE_LOOKUP_ITEM		"$RepeatInstanceLookup"
#define MAIL_CS_REPEAT_CACHEINSTANCE_ITEM		"$RepeatCacheInstanceDate"
#define MAIL_CS_REPEAT_CONFLICTDATES_ITEM		"tmpRepeatConflictDates"
#define MAIL_CS_ORIGINAL_STARTTIMEZONE_ITEM "OriginalStartTimeZone"
#define MAIL_CS_ORIGINAL_ENDTIMEZONE_ITEM "OriginalEndTimeZone"
#define MAIL_CS_REPEAT_RESCHEDULEOPTION			"tmpRepeatAdvancedOption"
#define MAIL_CS_REPEAT_SELECTEDDATES			"tmpSelectedDateList"
#define MAIL_CS_REPEAT_SELECTEDSTARTDATES		"tmpSelectedStartDateList"
#define MAIL_CS_REPEAT_SELECTEDENDDATES			"tmpSelectedEndDateList"

/* Notes 4 specific CS fields */

#define MAIL_CS_4_TO_5UPGRADE                   "$CS4to5upgrade"
#define MAIL_CS_4_TO_5NOT_GERMANE				"$CS4to5UpGradeNotGermane"
#define MAIL_CS_4_TO_5UPGRADE_DRAFT             "$CS4to5upgrade_draft"
#define MAIL_CS_4_REMINDER_TIME                 "ReminderTime"
#define MAIL_CS_4_DURATION                      "Duration"
#define MAIL_CS_4_ORGDONTDOUBLEBOOK             "ORGDONTDOUBLEBOOK"
#define MAIL_CS_4_INVITEE_NAME					"InviteeName"

/* C&S Alarm item names */

#define MAIL_CS_ALARMENABLED_ITEM				"$Alarm"
#define MAIL_CS_ALARMSOUND_ITEM				    "$AlarmSound"
#define MAIL_CS_ALARMDESCRIPTION_ITEM			"$AlarmDescription"
#define MAIL_CS_ALARMSENDTO_ITEM				"$AlarmSendTo"
#define MAIL_CS_ALARMOFFSET_ITEM				"$AlarmOffset"
#define MAIL_CS_ALARMTIME_ITEM					"$AlarmTime"
#define MAIL_CS_ALARMDISABLED_ITEM				"$AlarmDisabled"
#define MAIL_CS_ALARMDISABLED_DISABLED			'0'
#define MAIL_CS_ALARMDISABLED_ENABLED			'1'
#define MAIL_CS_ALARMOFFSETUNIT_ITEM			"$AlarmUnit"
#define MAIL_CS_ALARMMAILOPTIONS_ITEM			"$AlarmMemoOptions"


/* POP3 view names */
#define MAIL_POP3_UIDL_VIEW	"($POP3UIDL)"
#define POP3_VIEW	"($POP3)"

/*	Quota form item names */

#define MAIL_QUOTA_ACTION_ITEM	"$QuotaAction"	/* Action taken on original message causing quota report */
#define MAIL_QUOTA_REPORT_ITEM	"$QuotaType"	/* Over warning or over quota report */
#define MAIL_QUOTA_ORIG_SIZE_ITEM		"$QuotaOrigMsgSize"	/* Size of original note triggering a quota report. */
#define MAIL_QUOTA_ORIG_SUBJECT_ITEM "$QuotaOrigSubject" /* Original Subject */
#define MAIL_QUOTA_ORIG_FROM_ITEM "$QuotaOrigFrom" /* Original From item */
#define MAIL_QUOTA_ORIG_SENDTO_ITEM "$QuotaOrigSendTo" /* Original SendTo item */
#define MAIL_QUOTA_ORIG_COPYTO_ITEM "$QuotaOrigCopyTo" /* Original CopyTo item */
#define MAIL_QUOTA_ORIG_BCCTO_ITEM "$QuotaOrigBccTo" /* Original BlindCopyTo item */
#define MAIL_QUOTA_SIZELIMIT_ITEM "$QuotaSizeLimit" /* DB sizelimit in k bytes */
#define MAIL_QUOTA_WARNINGTHRESHOLD_ITEM "$QuotaWarningThreshold" /* DB warning threshold in k bytes */
#define MAIL_QUOTA_CURRENTUSAGE_ITEM "$QuotaCurrentUsage" /* Current amount of DB usage k bytes */
#define MAIL_QUOTA_METHOD_ITEM "$QuotaMethod" /* Quota enforcement method - filesize or usage*/
#define MAIL_QUOTA_CURRENTSIZE_ITEM "$QuotaCurrentDbSize" /* Current DB size in k bytes */
#define MAIL_ADMIN_TEXT_ITEM "$MailAdminText" /* Administrator specified text content */
#define MAIL_QUOTA_WARNING_TIME_ITEM "$QuotaLastWarningTime" /* Last warning report time */
#define MAIL_QUOTA_ERROR_TIME_ITEM "$QuotaLastErrorTime" /* Last time of error quota report */

/* Mail form names */

#define MAIL_MEMO_FORM "Memo"					/* Standard memo */
#define MAIL_REPLY_FORM "Reply"					/* Standard memo reply */
#define MAIL_REPLY_WITH_HISTORY_FORM "Reply With History"	/* Standard reply w/history memo */
#define MAIL_PHONEMESSAGE_FORM "Phone Message"	/* Phone message */
#define MAIL_DELIVERYREPORT_FORM "Delivery Report" /* Delivery report form name */
#define MAIL_NONDELIVERYREPORT_FORM "NonDelivery Report" /* Non-Delivery report form name */
#define MAIL_RETURNRECEIPT_FORM "Return Receipt" /* Return Receipt form name */
#define MAIL_DATABASEENTRY_FORM "Database Entry" /* mailin database entry form for libraries */
#define MAIL_TRACEREPORT_FORM "Trace Report"	 /* Trace report form name */
#define MAIL_QUOTAREPORT_FORM "Quota Report"	 /* Quota report form name */
#define MAIL_CALENDAR_PROFILE_FORM "CalendarProfile"	/* Calendar profile form name */
#define MAIL_DELEGATION_PROFILE_FORM "DelegationProfile" /* Delegation profile form name */
#define MAIL_APPOINTMENT_FORM "Appointment"		/* C & S Appointment form name */
#define MAIL_NOTICE_FORM "Notice"				/* C & S Meeting Notice form name */
#define MAIL_RESOURCE_RESERVATION_FORM "Reservation" /* C & S Resource reservation form name */
#define MAIL_RESOURCE_PROFILE_FORM "Resource"	/* C & S Calendar resource profile form name */
#define MAIL_TODO_FORM				"Task"		/* C & S Todo form name */
#define MAIL_REPEAT_CACHE_FORM		"RepeatCache"	/* C&S Repeat Cache profile note name */
#define MAIL_TODO_NOTICE_FORM		"TaskNotice"	/* C&S Todo notice workflow form */

/* Tasks are now called ToDos.  Define the old deprecated names as alias to new.
 * The actual form names, e.g., "Task" have not changed to allow on-disk 
 * backwards compatibility
 */
#define MAIL_TASK_FORM				MAIL_TODO_FORM
#define MAIL_TASK_NOTICE_FORM		MAIL_TODO_NOTICE_FORM

#define MAIL_MAILRULE_FORM			"Mailrule"


/* Address Book - "Person" form item names */

#define MAIL_PERSON_FORM "Person"				/* Form and type name */
#define MAIL_FORMTYPE_ITEM "Type"				/* Form type */
#define MAIL_FULLNAME_ITEM 	"FullName"			/* Full name  */
#define MAIL_FIRSTNAME_ITEM "FirstName"			/* First name */
#define MAIL_LASTNAME_ITEM 	"LastName"			/* Last name */
#define MAIL_RANAME_ITEM 	"RAName"			/* RA name */
#define MAIL_MIDDLEINITIAL_ITEM "MiddleInitial"	/* Middle initial or name */
#define MAIL_SHORTNAME_ITEM "ShortName"			/* Short name (for mail gateways) */
#define MAIL_INTERNETADDRESS_ITEM "InternetAddress"		/* InternetAddress */
#define MAIL_OWNER_ITEM 	"Owner"				/* Entry owner name */
#define MAIL_MAILSYSTEM_ITEM "MailSystem"		/* Mail system item (keyword field: 1,2,3,4) */
#define MAIL_MAILSERVER_ITEM "MailServer"		/* Mail server name */
#define MAIL_MAILFILE_ITEM 	"MailFile"			/* Mail file name */
#define MAIL_MAILDOMAIN_ITEM "MailDomain"		/* Mail domain name */
#define MAIL_MAILADDRESS_ITEM "MailAddress"		/* Mail auto-forwarding address */
#define MAIL_PUBLICKEY_ITEM "PublicKey"			/* Public encryption key */
#define MAIL_CERTIFICATE_ITEM "Certificate"		/* Public encryption key */
#define MAIL_ROLLOVER_CERT_ITEM "RolloverCert"	/* Key rollover cert */
#define MAIL_USERCERTIFICATE_ITEM "UserCertificate"		/* X.509 Public encryption key */
#define MAIL_LOCATION_ITEM "Location"			/* Location field */
#define MAIL_COMMENT_ITEM "Comment"				/* Comment field */
#define MAIL_USERID_ATTACHMENT "UserID"			/* User ID attachment file name */
#define MAIL_SERVERID_ATTACHMENT "ServerID"		/* Server ID attachment file name */
#define MAIL_CALENDARDOMAIN_ITEM "CalendarDomain" /* User's calendar domain override */
#define MAIL_NETUSERNAME_ITEM "NetUserName"		 /* Network Acct Name (for dir synching) */
#define MAIL_HTTPPASSWORD_ITEM "HTTPPassword"	 /* http password */
#define MAIL_HASHVERSION_ITEM	"$SecurePassword"	/* Flag that indicates NoteUpdate should hash HTTPPassword field */
#define MAIL_STORAGE_ITEM	"MessageStorage"		/* Specifies how user wants to store the mail.  See IMSG_PREFERENCE_XXX */
#define MAIL_OFFICEPHONE_ITEM "OfficePhoneNumber"   /* Office phone number */
#define MAIL_COMPANYNAME_ITEM "CompanyName"         /* Company name */
#define MAIL_COUNTRY_ITEM "Country"                 /* Country */
#define MAIL_DEPARTMENT_ITEM "Department"           /* Department */
#define MAIL_OTHER_EMAIL1 "MiscPhone1"
#define MAIL_OTHER_EMAIL2 "MiscPhone2"
#define MAIL_OTHER_EMAIL3 "MiscPhone3"
#define MAIL_BIRTHDAY    "Birthday"
#define MAIL_ANNIVERSARY "Anniversary"

#define MAIL_ALTFULLNAME_ITEM				"AltFullName"					/* Alternate FullName */
#define MAIL_ALTFULLNAMELANGUAGE_ITEM		"AltFullNameLanguage"			/* Language Tag. */
#define MAIL_PROPALTCOMMONNAME_ITEM 		"ProposedAltCommonName"			/* Proposed Alternate CommonName */
#define MAIL_PROPALTORGUNIT_ITEM 			"ProposedAltOrgUnit" 			/* Proposed Alternate OrgUnit */
#define MAIL_PROPALTFULLNAMELANGUAGE_ITEM	"ProposedAltFullNameLanguage"	/* Proposed Language Tag. */

#define HTTP_PASSWORD_ITEM					"HTTPPassword"					/* http password */
#define HTTP_PASSWORD_CHANGE_DATE_ITEM 		"HTTPPasswordChangeDate"	 	/* http password last change date*/
#define HTTP_PASSWORD_CHANGE_INTERVAL_ITEM 	"HTTPPasswordChangeInterval"	/* http password change interval*/
#define HTTP_PASSWORD_OPTIONS_ITEM 			"HTTPPasswordOptions"		 	/* http password options	     */
#define HTTP_PASSWORD_QUAILITY_ITEM			"HTTPPasswordQuality"		 	/* http password quality */
#define HTTP_PASSWORD_NOTES_SYNC_ITEM		"HTTPPasswordNotesSync"			/* Sync Notes pw with http pw */

/* Address Book - Mail-in "Database" form item names.  Uses Person form
	fields: FullName, MailServer, MailFile, and MailDomain" */

#define MAIL_MAILINDATABASE_FORM "Database"		/* Form and type name */
#define MAIL_DESCRIPTION_ITEM 	"Description"	/* Mail-in db description item */
#define MAIL_HAIKU_ITEM			"$QuickPlace"	/* QuickPlace mail database */

/* MAPI specific items */

#define MAIL_MAPI_MSGFLAGS_ITEM "$MapiMessageFlags" /* MAPI IMessage PR_MESSAGE_FLAGS Property */
#define MAIL_MAPI_MSGCLASS_ITEM "$MapiMessageClass" /* MAPI IMessage PR_MESSAGE_CLASS Property */
#define MAIL_MAPI_STATUS_ITEM "$MapiStatus" 		/* MAPI PR_MSG_STATUS, PR_STATUS Properties */
#define MAIL_MAPI_ACCESS_ITEM "$MapiAccess" 		/* MAPI PR_ACCESS, PR_ACCESS_LEVEL Properties */
#define MAIL_MAPI_TO_ADDRTYPE_ITEM "$MapiSendToAddrType"/* MAPI Recip PR_ADDRTYPE Property for To List */
#define MAIL_MAPI_CC_ADDRTYPE_ITEM "$MapiSendCcAddrType"/* MAPI Recip PR_ADDRTYPE Property for Cc List*/
#define MAIL_MAPI_BCC_ADDRTYPE_ITEM "$MapiSendBccAddrType"/* MAPI Recip PR_ADDRTYPE Property for Bcc List*/
#define MAIL_MAPI_TO_RESP_ITEM "$MapiSendToResponsibility"/* MAPI Recip PR_RESPONSIBILITY Property for To List */
#define MAIL_MAPI_CC_RESP_ITEM "$MapiSendCcResponsibility"/* MAPI Recip PR_RESPONSIBILITY Property for Cc List */
#define MAIL_MAPI_BCC_RESP_ITEM "$MapiSendBccResponsibility"/* MAPI Recip PR_RESPONSIBILITY Property for Bcc List */
#define MAIL_MAPI_TO_EID_ITEM "$MapiSendToEID"		/* MAPI Recip PR_ENTRYID Property for To List */
#define MAIL_MAPI_CC_EID_ITEM "$MapiSendCcEID"		/* MAPI Recip PR_ENTRYID Property for Cc List */
#define MAIL_MAPI_BCC_EID_ITEM "$MapiSendBccEID"	/* MAPI Recip PR_ENTRYID Property for Bcc List */
#define MAIL_MAPI_TO_OT_ITEM "$MapiSendToObjType"	/* MAPI Recip PR_OBJECT_TYPE Property for To List */
#define MAIL_MAPI_CC_OT_ITEM "$MapiSendCcObjType"	/* MAPI Recip PR_OBJECT_TYPE Property for Cc List */
#define MAIL_MAPI_BCC_OT_ITEM "$MapiSendBccObjType" /* MAPI Recip PR_OBJECT_TYPE Property for Bcc List */
#define MAIL_MAPI_TO_TRACKSTATUS_ITEM "$MapiSendToTrackStatus"/* MAPI Recip PR_RECIPIENT_TRACKSTATUS Property for To List */
#define MAIL_MAPI_CC_TRACKSTATUS_ITEM "$MapiSendCcTrackStatus"/* MAPI Recip PR_RECIPIENT_TRACKSTATUS Property for Cc List */
#define MAIL_MAPI_BCC_TRACKSTATUS_ITEM "$MapiSendBccTrackStatus"/* MAPI Recip PR_RECIPIENT_TRACKSTATUS Property for Bcc List */
#define MAIL_MAPI_TO_FLAGS_ITEM "$MapiSendToFlags"	/* MAPI Recip PR_RECIPIENT_FLAGS Property for To List */
#define MAIL_MAPI_CC_FLAGS_ITEM "$MapiSendCcFlags"	/* MAPI Recip PR_RECIPIENT_FLAGS Property for Cc List */
#define MAIL_MAPI_BCC_FLAGS_ITEM "$MapiSendBccFlags"/* MAPI Recip PR_RECIPIENT_FLAGS Property for Bcc List */
#define MAIL_MAPI_TO_TSTIME_ITEM "$MapiSendToTSTime"/* MAPI Recip PR_RECIPIENT_TRACKSTATUS_TIME Property for To List */
#define MAIL_MAPI_CC_TSTIME_ITEM "$MapiSendCcTSTime"/* MAPI Recip PR_RECIPIENT_TRACKSTATUS_TIME Property for Cc List */
#define MAIL_MAPI_BCC_TSTIME_ITEM "$MapiSendBccTSTime"/* MAPI Recip PR_RECIPIENT_TRACKSTATUS_TIME Property for Bcc List */
#define MAIL_MAPI_TO_ADDR_ITEM "$MapiSendToAddr" 	/* MAPI Recip PR_EMAIL_ADDRESS Property for To List */
#define MAIL_MAPI_CC_ADDR_ITEM "$MapiSendCcAddr" 	/* MAPI Recip PR_EMAIL_ADDRESS Property for Cc List */
#define MAIL_MAPI_BCC_ADDR_ITEM "$MapiSendBccAddr" 	/* MAPI Recip PR_EMAIL_ADDRESS Property for Bcc List */
#define MAIL_MAPI_SENDER_EID_ITEM "$MapiSenderEID"	/* MAPI Sender PR_ENTRYID Property */
#define MAIL_MAPI_SENDER_ADDRTYPE_ITEM "$MapiSenderAddrType"   /* MAPI Sender PR_ADDRTYPE Property */
#define MAIL_MAPI_SENDER_ADDR_ITEM "$MapiSenderAddr"/* MAPI Sender PR_EMAIL_ADDR Property */
#define MAIL_MAPI_SENDER_SEARCHKEY_ITEM "$MapiSenderSearchKey" /* MAPI Sender PR_SEARCH_KEY Property */
#define MAIL_MAPI_SENDER_NAME_ITEM "$MapiSenderName"/* MAPI Sender PR_DISPLAY_NAME Property */
#define MAPI_MAPI_CONTAINER_CLASS_ITEM "$MapiContainerClass" /* MAPI Folder PR_CONTAINER_CLASS Property */

/* Roaming user specific field names*/
#define MAIL_USERDIC "UserDic"			/*the user's personal dictionary item*/
#define USERDIC_FORMAT "UserDicFormat"	/*format of the user dictionary 0 - MAC 1- PC */

/*	Summary item names used to store NNTP information. */

#define NNTP_MESSAGE_ID_ITEM_NAME		"$UName"
#define NNTP_FROM_ITEM_NAME				"$8"
#define	NNTP_SUBJECT_ITEM_NAME			"$9"
#define NNTP_DATE_ITEM_NAME				"$10"
#define NNTP_REFERENCES_ITEM_NAME		"$11"
#define NNTP_PATH_ITEM_NAME				"Path"
#define NNTP_DISTRIBUTIONS_ITEM_NAME	"Distribution"
#define NNTP_SIZE_ITEM_NAME				"NNTP_Size"
#define NNTP_LINES_ITEM_NAME			"Lines"
#define NNTP_ISLMBCS_ITEM_NAME			"$18"

/*	NNTP Client item names */

#define NNTPCL_OUTGOING_ITEM			"Outgoing"
#define NNTPCL_NEWSREADER_ITEM			"$Newsreader"
#define NNTPCL_NEWSGROUPS_ITEM			"Newsgroups"
#define NNTPCL_OUTBOX_VIEW				"($ArticlesToPost)"

/*	Client Type  */

#define FIELD_CLIENTTYPE "ClientType"			/* Client Type - Full or Limited Notes */

/*	Standard Subform Item Name.	*/

#define SUBFORM_ITEM_NAME "$SubForms"
#define SUBFORM_REPIDS_ITEM_NAME "$SubForm_RepIDs"

/*	Define field name common to Server, Person, and Certifier forms */

#define KFM_ITEM_CHANGE_REQUEST			"ChangeRequest"	
#define KFM_ITEM_CHANGE_REQUEST_DATE	"ChangeRequestDate"


/*	Certifier and Cross-certificate Address Book lookup definitions */

#define	KFM_FORM_CERTIFIER			"Certifier"
#define	KFM_FORM_CROSSCERTIFICATE	"CrossCertificate"

#define KFM_ITEM_NAMECOMBO			"NameCombo"
#define KFM_ITEM_ORGCOMBO			"OrgCombo"
#define KFM_ITEM_ISSUEDTO			"IssuedTo"
#define KFM_ITEM_ISSUEDBY			"IssuedBy"
#define KFM_ITEM_CERTIFICATE		"Certificate"
#define KFM_ITEM_USER_CERTIFICATE	"UserCertificate"
												/* Some additional items are */
												/* the same as those defined */
												/* for the "Person" form */

#define KFM_ITEM_CERT_NOENC			"Certificate_NoEnc"
												/* Public encryption key	*/
												/* to be used for signature */
												/* checking only and not for*/
												/* sending encrypted mail 	*/

#define KFM_ITEM_PUBLICKEY			"PublicKey"	/* Public encryption key 	*/
												/* for flat names.			*/
#define KFM_ITEM_INET_PUBKEY		"INetPublicKey"
												/* Public key to be used for*/
												/* "View level" internet	*/
												/* certification.			*/
												

#define CERTIFIERNAMESSPACE         "$Certifiers"
#define LOCAL_CERTIFIERNAMESSPACE   "1\\$Certifiers"
#define CERTIFIERSNAMESSPACE		"$Certifiers"
#define CROSSCERTBYROOTNAMESSPACE	"$CrossCertByRoot"
#define CROSSCERTBYNAMENAMESSPACE	"$CrossCertByName"


#define	KFM_CROSSCERT_ITEM_ISSUEDTO		0
#define	KFM_CROSSCERT_ITEM_ISSUEDBY		1
#define	KFM_CROSSCERT_ITEM_CERTIFICATE	2
#define	KFM_CROSSCERT_ITEM_CERT_NOENC	3
#define KFM_CROSSCERT_ITEM_NOTEID		4
#define	KFM_CROSSCERT_LOOKUPITEMCOUNT	5
#define	KFM_CROSSCERT_LOOKUPITEMS "IssuedTo\0IssuedBy\0Certificate\0Certificate_NoEnc\0$$NoteID"

#define	KFM_CERTIFIER_ITEM_ISSUEDTO		0
#define	KFM_CERTIFIER_ITEM_ISSUEDBY		1
#define	KFM_CERTIFIER_ITEM_CERTIFICATE	2
#define	KFM_CERTIFIER_ITEM_CERT_NOENC	3
#define	KFM_CERTIFIER_LOOKUPITEMCOUNT	4
#define	KFM_CERTIFIER_LOOKUPITEMS "IssuedTo\0IssuedBy\0Certificate\0Certificate_NoEnc"

/*	Names for Wired In Cross Certificates
*/
#define NOTES_TEMPLATE_DEVELOPER_ID			"CN=Lotus Notes Template Development/O=Lotus Notes"
#define NOTES_COMPANION_ID					"O=Lotus Notes Companion Products"
#define NOTES_WHOSAYS_ID					"Notes"
#define BINARYTREE_TEMPLATE_DEVELOPER_ID	"CN=BT Mail and Calendar Migration Tools/O=Lotus Notes Companion Products"
#define SAMETIME_TEMPLATE_DEVELOPER_ID		"CN=Sametime Development/O=Lotus Notes Companion Products"
#define LOTUSFAX_TEMPLATE_DEVELOPER_ID		"CN=Lotus Fax Development/O=Lotus Notes Companion Products"
#define LOTUSUNICOMM_TEMPLATE_DEVELOPER_ID	"CN=Domino Unified Communications Services/O=Lotus Notes Companion Products"

/* ECL Owner Key for admin ecl updates
 *	When this is encoutered on the client during eclrefresh it is replaced
 *  with the current users name to allow admins to define current user rights
 */
#define ECL_KEY_FOR_ECL_OWNER_NAME			"<ECLOwner>"

/*	Lookup items for Network Authentication check.
	
	NOTE:	These items are no longer referenced by the Notes Authentication code.
			However, some NETAUTH_... symbols are referenced by miscellaneous code
			(change as TESTNSF) so we won't delete them here just yet.

			LATER:	We should search out and destroy all references.
*/

#define	NETAUTH_ITEM_CERTIFICATE	 	 0
#define	NETAUTH_ITEM_CERT_NOENC		 	 1
#define	NETAUTH_ITEM_PUBLIC_KEY		 	 2
#define	NETAUTH_ITEM_CHANGEREQUEST	 	 3
#define	NETAUTH_ITEM_USERCERTIFICATE 	 4
#define	NETAUTH_LOOKUPITEMCOUNT		 	 5

#define NETAUTH_LOOKUPITEMS	 "Certificate\0Certificate_NoEnc\0PublicKey\0ChangeRequest\0ChangeRequestDate\0UserCertificate"

#define NETAUTH_NAMESPACE_USERS				"($Users)"
#define	NETAUTH_NAMESPACE_SERVERS			"($Servers)"
#define	NETAUTH_NAMESPACE_SERVERSORUSERS	"($Servers)\0($Users)"
#define	NETAUTH_NAMESPACE_USERSORSERVERS	"($Users)\0($Servers)"
#define	NETAUTH_NAMESPACE_CERTIFIERS		"($Certifiers)"
#define NETAUTH_NAMESPACE_XCERTBYROOT		"($CrossCertByRoot)"

/*  Lookup items for KFMGetPublicKey
*
*	NOTE: Code knows that the last view may not be present (in the case
*	of a Personal N&A book), so $Servers must be last and the code must
*	be changed if any others become optional.
*/

#define KFM_GETPUB_VIEWS		"($Users)\0($Certifiers)\0($Servers)"
#define KFM_GETPUB_NUMVIEWS		3
#define KFM_GETPUB_FIELDS		"Certificate\0Certificate_NoEnc\0PublicKey\0UserCertificate"
#define KFM_GETPUB_NUMFIELDS	4


/* BSAFE NAMELookup Items */
#define SEC_AUTH_ITEMS_LIST  \
							 \
"\
$$NoteID\0\
$$DBName\0\
\
Certificate\0\
Certificate_NoEnc\0\
PublicKey\0\
UserCertificate\0\
\
ChangeRequest\0\
ChangeRequestDate\0\
\
ClientType\0\
\
PasswordChangeInterval\0\
PasswordChangeDate\0\
PasswordDigest\0\
PasswordGracePeriod\0\
\
CheckPassword\0\
\
HTTPPasswordChangeDate\0\
HTTPPassword\0\
\
Policy\0\
Profiles\0\
ClntDgst\0\
NewMailClientUpdateFlag\0\
RoamingUserUpdateFlag\0\
\
ServerName\0\
\
NewRoamStatUpdtFl\0\
RoamStatDwnFl\0\
MailDomain\0\
\
PKMinWidth\0\
PKMaxWidth\0\
PKDefWidth\0\
PKMaxAge\0\
PKMinDate\0\
PKDueDate\0\
PKPriority\0\
PKOldKeyDays\0\
\
$$ModifiedTime\0"

#define SEC_AUTH_ITEM_NOTEID					 0
#define SEC_AUTH_ITEM_DBNAME					 1

#define SEC_AUTH_ITEM_CERTIFICATE				 2
#define SEC_AUTH_ITEM_CERT_NOENC				 3
#define SEC_AUTH_ITEM_PUBLIC_KEY				 4
#define SEC_AUTH_ITEM_USER_CERT					 5

#define SEC_AUTH_ITEM_CHANGE_REQUEST			 6
#define SEC_AUTH_ITEM_CHANGE_REQUEST_DATE		 7

#define SEC_AUTH_ITEM_CLIENT_TYPE				 8

#define SEC_AUTH_ITEM_NOTESPW_CHANGEINTERVAL	 9
#define SEC_AUTH_ITEM_NOTESPW_CHANGEDATE 		10
#define SEC_AUTH_ITEM_NOTESPW_DIGEST			11
#define SEC_AUTH_ITEM_NOTESPW_GRACEPERIOD		12

#define SEC_AUTH_ITEM_NOTESPW_CHECK				13

#define SEC_AUTH_ITEM_HTTPCHANGEDATE			14
#define SEC_AUTH_ITEM_HTTPPASSWORD				15

#define SEC_AUTH_ITEM_POLICY					16
#define SEC_AUTH_ITEM_PROFILES					17
#define SEC_AUTH_ITEM_CLIENTDIGEST				18
#define SEC_AUTH_ITEM_FLAG_NEWMAIL				19
#define SEC_AUTH_ITEM_FLAG_ROAMING				20

#define SEC_AUTH_ITEM_SERVERNAME				21

#define SEC_AUTH_ITEM_FLAG_NEWROAMING			22
#define SEC_AUTH_ITEM_FLAG_ROAMINGDOWN			23
#define SEC_AUTH_ITEM_MAILDOMAIN				24

#define SEC_AUTH_ITEM_PUBKEY_MINWIDTH			25
#define SEC_AUTH_ITEM_PUBKEY_MAXWIDTH			26
#define SEC_AUTH_ITEM_PUBKEY_DEFWIDTH			27
#define SEC_AUTH_ITEM_PUBKEY_MAXAGE				28
#define SEC_AUTH_ITEM_PUBKEY_MINDATE			29
#define SEC_AUTH_ITEM_PUBKEY_DUEDATE			30
#define SEC_AUTH_ITEM_PUBKEY_PRIORITY			31
#define SEC_AUTH_ITEM_PUBKEY_OLDKEYDAYS			32

#define SEC_AUTH_ITEM_MODIFIED_TIME				33

#define SEC_AUTH_ITEMS_COUNT		 			34

#define SEC_CERTTAB_ITEMS_LIST	\
"\
$$NoteID\0\
$$DBName\0\
\
Certificate\0\
Certificate_NoEnc\0\
PublicKey\0\
UserCertificate\0\
ChangeRequest\0\
\
CertificateType\0\
IsHostedOrg\0\
OrgDatabaseDir\0\
OrgPolicy\0\
RegistrationAuthorities\0\
\
RecoveryInfo\0\
RolloverCert\0\
\
GlobalDomainDocument"

#define SEC_CERTTAB_ITEM_NOTEID				 0
#define SEC_CERTTAB_ITEM_DBNAME				 1
											
#define	SEC_CERTTAB_ITEM_CERTIFICATE		 2
#define	SEC_CERTTAB_ITEM_CERT_NOENC			 3
#define SEC_CERTTAB_ITEM_PUBLIC_KEY			 4
#define	SEC_CERTTAB_ITEM_USER_CERT			 5
#define SEC_CERTTAB_ITEM_CHANGE_REQUEST		 6

#define SEC_CERTTAB_ITEM_CERTIFICATE_TYPE	 7	/* really "Certifier Type" */
#define SEC_CERTTAB_ITEM_IS_HOSTED_ORG		 8
#define SEC_CERTTAB_ITEM_ORG_DATABASE_DIR	 9
#define SEC_CERTTAB_ITEM_ORG_POLICY			10
#define SEC_CERTTAB_ITEM_RAS				11

#define	SEC_CERTTAB_ITEM_RECOVERY_INFO		12
#define SEC_CERTTAB_ITEM_ROLLOVER_CERT		13

#define SEC_CERTTAB_ITEM_GLOBALDOMAIN_DOC	14

											
#define SEC_CERTTAB_ITEMS_COUNT		   		15


#define ADMINP_NAB_ITEMS_LIST	\
"\
$$NoteID\0\
$$DBName\0\
\
Certificate\0\
Certificate_NoEnc\0\
PublicKey\0\
UserCertificate\0\
ChangeRequest"

#define ADMINP_NAB_ITEM_NOTEID			 0
#define ADMINP_NAB_ITEM_DBNAME			 1

#define	ADMINP_NAB_ITEM_CERTIFICATE	 	 2
#define	ADMINP_NAB_ITEM_CERT_NOENC		 3
#define ADMINP_NAB_ITEM_PUBLIC_KEY		 4
#define	ADMINP_NAB_ITEM_USER_CERT		 5
#define ADMINP_NAB_ITEM_CHANGE_REQUEST	 6

#define ADMINP_NAB_ITEMS_COUNT			 7

#define NET_GROUP_ITEMS_LIST	\
"\
$$NoteID\0\
$$DBName\0\
\
Certificate\0\
Certificate_NoEnc\0\
PublicKey\0\
UserCertificate\0\
ChangeRequest"

#define NET_GROUP_ITEM_NOTEID			 0
#define NET_GROUP_ITEM_DBNAME			 1

#define	NET_GROUP_ITEM_CERTIFICATE	 	 2
#define	NET_GROUP_ITEM_CERT_NOENC		 3
#define NET_GROUP_ITEM_PUBLIC_KEY		 4
#define	NET_GROUP_ITEM_USER_CERT		 5
#define NET_GROUP_ITEM_CHANGE_REQUEST	 6

#define NET_GROUP_ITEMS_COUNT			 7


/* S/MIME items in address book records. */
#define SMIME_USER_CAPABILITIES_ITEM "SmimeUserCapabilities"

/*	Address Book - Items and names used by security panel */

#define SECPANEL_PERSONDOCITEM_NAME					0
#define SECPANEL_PERSONDOCITEM_SHNAME				1
#define SECPANEL_PERSONDOCITEM_INETADDR				2
#define SECPANEL_PERSONDOCITEM_ENCRMAIL				3
#define SECPANEL_PERSONDOCITEM_STOREMAIL			4
#define SECPANEL_PERSONDOCITEM_CHECKPASSWORD		5
#define SECPANEL_PERSONDOCITEM_PASS_GRACE_PERIOD	6
#define SECPANEL_PERSONDOCITEM_PASS_CHANGE_INTERVAL	7
#define SECPANEL_PERSONLOOKUPITEMCOUNT		8
#define SECPANEL_PERSONLOOKUPITEMS      "FullName\0ShortName\0InternetAddress\0EncryptIncomingMail\0MessageStorage\0CheckPassword\0PasswordGracePeriod\0PasswordChangeInterval"

/* Userless NAB definitions */
#define USERLESS_FORMULA "Type!=\"Person\" & Type != \"Database\" & Type != \"Group\" & (@IsUnAvailable(FullName) | @IsAvailable(CfgDoc) | Type = \"Certifier\")"
#define USERLESS_FORMULA_V2 "Type!=\"Person\" & Type != \"Database\" & Type != \"Group\" & (@IsUnAvailable(FullName) | @IsAvailable(ConfigDocument) | Type = \"Certifier\")"
#define USERLESS_FORMULA_V1 "((Type!=\"Person\" & Type != \"Database\" & Type != \"Group\" ) & (@IsUnAvailable(FullName ) & Type != \"Certifier\")) | @IsAvailable(CfgDoc)"
#define USERLESS_FORMULA_V3 "((Type!=\"Person\" & Type != \"Database\" & Type != \"Group\" ) & (@IsUnAvailable(FullName ) & Type != \"Certifier\")) | @IsAvailable(ConfigDocument)"


#define FILE_ID_VIEW			"$FileIdentifications"

/*  Lookup items for Profiles */

#define PROFILENAMESSPACE	"$Profiles"
#define PROFILENAME     "ProfileName"
#define PROFILES_ITEM "Profiles"

/* dblink fields in the Setup Profile note */
#define PROFILE_DATABASELINK_ITEM	"DatabaseLinks"
#define PROFILE_REPLICALINK_ITEM	"ReplicaLinks"
#define PROFILE_LNABLINK_ITEM		"LNABLinks"
#define POLICY_WELCOMELINK_ITEM		"WelcomeLink"

/* Smart Upgrade Tracking Options in Desktop Settings Policy */
#define POLICY_DESKTOP_SU_LOCATION_ITEM			"SULoc"
#define POLICY_DESKTOP_SU_REMOVE_ITEM			"SURem"
#define POLICY_DESKTOP_SU_RETENTION_ITEM		"SUDays"

/* Diagnostic Collection Options in Destop Settings Policy */
#define POLICY_DESKTOP_LOCATION_ITEM			"DCLoc"
#define POLICY_DESKTOP_LOCATION_ITEM_IP			"DCLoc$IP"
#define POLICY_DESKTOP_LOCATION_ITEM_PO			"DCLoc$PO"
#define POLICY_DESKTOP_PROMPT_ITEM				"DCPmpt"
#define POLICY_DESKTOP_PROMPT_ITEM_IP			"DCPmpt$IP"
#define POLICY_DESKTOP_PROMPT_ITEM_PO			"DCPmpt$PO"
#define POLICY_DESKTOP_REMOVE_ITEM				"DCRem"
#define POLICY_DESKTOP_REMOVE_ITEM_IP			"DCRem$IP"
#define POLICY_DESKTOP_REMOVE_ITEM_PO			"DCRem$PO"
#define POLICY_DESKTOP_CRASHCOMMENTS_ITEM	 	"DCCom"
#define POLICY_DESKTOP_CRASHCOMMENTS_ITEM_IP	"DCCom$IP"
#define POLICY_DESKTOP_CRASHCOMMENTS_ITEM_PO 	"DCCom$PO"
#define POLICY_DESKTOP_CRASHMSGSIZE_ITEM		"DCMsgSize"
#define POLICY_DESKTOP_CRASHMSGSIZE_ITEM_IP		"DCMsgSize$IP"
#define POLICY_DESKTOP_CRASHMSGSIZE_ITEM_PO		"DCMsgSize$PO"
#define POLICY_DESKTOP_CRASHNSDSIZE_ITEM		"DCNSDSize"
#define POLICY_DESKTOP_CRASHNSDSIZE_ITEM_IP		"DCNSDSize$IP"
#define POLICY_DESKTOP_CRASHNSDSIZE_ITEM_PO		"DCNSDSize$PO"
#define POLICY_DESKTOP_DEBUGOUTFILE_ITEM		"DCDO"
#define POLICY_DESKTOP_DEBUGOUTFILE_ITEM_IP		"DCDO$IP"
#define POLICY_DESKTOP_DEBUGOUTFILE_ITEM_PO		"DCDO$PO"
#define POLICY_DESKTOP_RETENTION_ITEM			"DCDays"
#define POLICY_DESKTOP_RETENTION_ITEM_IP		"DCDays$IP"
#define POLICY_DESKTOP_RETENTION_ITEM_PO		"DCDays$PO"
#define POLICY_DESKTOP_ADC_FILES_ITEM			"DCFiles"
#define POLICY_DESKTOP_ADC_FILES_ITEM_IP		"DCFiles$IP"
#define POLICY_DESKTOP_ADC_FILES_ITEM_PO		"DCFiles$PO"

/* Address Book - "Server" form item names */

#define MAIL_SERVER_FORM "Server"				/* Form and type name */
#define MAIL_SERVERNAME_ITEM "ServerName"		/* Server name */
#define MAIL_DOMAINNAME_ITEM "Domain"			/* Domain server is in */
#define MAIL_NETWORKNAME_ITEM "Network"			/* Network server is on */
#define MAIL_ADMINNAME_ITEM "Administrator" 	/* Server's administrator(s) */
#define MAIL_SERVERTITLE_ITEM "ServerTitle"		/* Server title */
#define MAIL_SERVERPORTS_ITEM "Ports"			/* Enabled ports in server record */
#define MAIL_SMTPLISTENER_ITEM "SMTPListenerEnabled" /* Does this server run an SMTP listener? */
#define MAIL_FULLHOSTNAME_ITEM "SMTPFullHostDomain"	/*	Host name of the server */
#define MAIL_NETADDRESSES_ITEM "NetAddresses"	/* Network Addresses */

/* Address Book - "Certifier" form item names */

#define MAIL_CERTIFIER_TYPE "Certifier"						/* Document type of "Certifier" */
#define MAIL_CERTIFIERTYPE_ITEM "Type"						/* Certifier type */
#define MAIL_CERTIFIERISSUEDTO_ITEM "IssuedTo"				/* Certifier issued to (to get Org Name) */
#define MAIL_CERTIFIERGLOBALDOMAIN_ITEM "GlobalDomainDocument"	/* Name of Global Domain for organization */
#define MAIL_CERTIFIERISORG_ITEM "IsHostedOrg"					/* is certifier an organization */
#define MAIL_CERTIFIERDBSTORAGE_ITEM "OrgDatabaseDir"			/* storage location of organization */
#define MAIL_CERTIFIERPOLICY_ITEM "OrgPolicy"					/* explicit policy for organization */

/* Address Book - "X400Server" form item names */

#define MAIL_X400_SERVER_FORM "Server"				/* Form and type name */
#define MAIL_X400_SERVERNAME_ITEM "X400ServerName"		/* X400 MTA server name */
#define MAIL_X400_MTANAME_ITEM "X400MTAName"			/* X400 MTA name */
#define MAIL_X400_GLOBAL_DOMAIN_ITEM "X400GlobalDomain" /* Global domain identifier */ 
#define MAIL_X400_COUNTRY_ITEM "X400Country"			/* Country name */
#define MAIL_X400_ADMD_ITEM "X400ADMD"					/* Administration Management Domain name*/
#define MAIL_X400_PRMD_ITEM "X400Domain"				/* Private Domain name */
#define MAIL_X400_CREDENTIALS_ITEM "X400Credentials"	/* MTA credentials */
#define MAIL_X400_SUPP_APP_CONTEXTS_ITEM "X400SupportedApplicationContexts"	/* Supported app. contexts */
#define MAIL_X400_DEL_CONTENT_LENGTH_ITEM "X400DeliverContentLength"	/* Delivery content length */
#define MAIL_X400_DEL_CONTENT_TYPE_ITEM "X400DeliverContentTypes"	 	/* Delivery content types */
#define MAIL_X400_PSEL_ITEM "X400PSEL"					/* Presentation selector */
#define MAIL_X400_SSEL_ITEM "X400SSEL"					/* Session selector */
#define MAIL_X400_TSEL_ITEM "X400TSEL"					/* Transport selector */
#define MAIL_X400_IPADDRESS_ITEM "X400IPAddress" 		/* Network IP address for the MTA */
#define MAIL_X400_X121ADDRESS_ITEM "X400X121Address" 	/* X.121 Network address for the MTA */

/* Address Book - "Domain" form item names */

#define MAIL_DOMAIN_FORM "Domain"							/* Form and type name */
#define MAIL_DOMAINTYPE_ITEM "DomainType"					/* Domain type */
#define MAIL_DOMAINADJDOMAINNAME_ITEM "AdjacentDomainName" 	/* Adjacent domain name */
#define MAIL_DOMAINDOMAINNAME_ITEM "DomainName"				/* Domain name */
#define MAIL_DOMAINNEXTDOMAIN_ITEM "NextDomain"				/* Next domain to domain */
#define MAIL_DOMAINMAILSERVER_ITEM "MailServer"				/* Server containing mailbox */
#define MAIL_DOMAINMAILFILE_ITEM "MailFile"					/* File name of mailbox */
#define	MAIL_DOMAINALLOW_ITEM "AllowFromDomains"			/* Allow list */
#define	MAIL_DOMAINDENY_ITEM "DenyFromDomains"				/* Deny list */
#define MAIL_DOMAINCALENDARSERVER_ITEM "CalendarServer"		/* Calendar gateway server */
#define MAIL_DOMAINCALENDARSYSTEM_ITEM "CalendarSystem"		/* Calendar gateway system */


#define MAIL_DOMAINCALENDARDETAILS_ITEM "CalDtls"	/* Calendar details to harvest */
#define MAIL_DOMAINNONCONTROVERSIALFIELDS_ITEM "CalDtls"	/* Non-Controversial Calendar details to harvest */

/* Address Book - "Connection" form item names */

#define MAIL_LOCAL_CONNECTION_FORM "Local"		/* Local Connection form name */
#define MAIL_REMOTE_CONNECTION_FORM "Remote"	/* Remote Connection form name */
#define MAIL_CONNECTION_TYPE "Connection"		/* Connection entry type */
#define MAIL_CONNECTION_TYPE_ITEM "ConnectionType" /* Connection type item */
#define MAIL_SOURCE_ITEM "Source"				/* Source computer name */
#define MAIL_SOURCEDOMAIN_ITEM "SourceDomain"	/* Source domain name */
#define MAIL_DESTINATION_ITEM "Destination"		/* Destination computer name */
#define MAIL_DESTINATIONDOMAIN_ITEM "DestinationDomain"	/* Destination domain name */
#define MAIL_PORTNAME_ITEM "PortName"			/* Port name */
#define MAIL_PHONENUMBER_ITEM "PhoneNumber" 	/* Phone number */
#define MAIL_COUNTRYCODE_ITEM "Country"		 	/* Phone number */
#define MAIL_AREACODE_ITEM "AreaCode"		 	/* Phone number */
#define MAIL_AREACODEINPUT_ITEM "AreaCodeInput"	/* Phone number */
#define MAIL_ENABLED_ITEM "Enabled"				/* Enabled/disabled item */
#define MAIL_TASKS_ITEM "Tasks"					/* Tasks item */
#define MAIL_INTERVAL_ITEM "Interval"			/* Interval item */
#define MAIL_SCHEDULE_ITEM "Schedule"			/* Schedule item */
#define MAIL_COST_ITEM "Cost"					/* Routing cost item */
#define MAIL_REPLICATION_TYPE_ITEM "RepType"	/* Replication type, e.g. pull/pull */
#define MAIL_LINKINFO_ITEM "LinkInfo"			/* For X.25 conn recs */
#define MAIL_PASSTHRUSERVER_ITEM "PassthruServer" /* For passthru conn recs */
#define MAIL_LOCALPHONENUMBER_ITEM "LocalPhoneNumber"	/* set this in connrec for form compute */

/* Address Book - "Group" form item names */

#define MAIL_GROUP_FORM "Group"					/* Form and type name */
#define MAIL_GROUP_TYPE "GroupType"					/* Form and type name */
#define MAIL_LISTNAME_ITEM "ListName"			/* Distribution list name */
#define MAIL_LISTDESCRIPTION_ITEM "ListDescription"	/* Distribution list name */
#define MAIL_MEMBERS_ITEM "Members"				/* Members names name */
#define MAIL_LISTADMINISTRATOR_ITEM "Administrator" /* Group administrator */
/* Part of SPR GOTO54JT5Q - Need this to get list owner for LDAP Group details display */
#define MAIL_LISTOWNER_ITEM "ListOwner"			/* List owner */
/* Part of SPR GOTO54JT5Q - Need this to get administrator for LDAP Group details display */
#define MAIL_LISTADMIN_ITEM "LocalAdmin"		/* Group administrator */

/* Address Book - "External Domain Network Information" form item names */

#define MAIL_EXTERNAL_DOMAIN_NETWORK_INFORMATION_FORM "ExternalDomainNetworkInformation"	/* Form and type name */

/* Address Book - "User Setup Profile" form item names */

#define PROFILE_FORM "Profile"				/* Form and type name */

/* Address Book - "Holiday" form item names */

#define HOLIDAY_FORM "Holiday"				/* Form and type name */

/* Address Book - "(Alternate Language Personal Information)" form item names */

#define LDAP_ALT_LANGUAGE_PERSONAL_INFO_FORM "AltLanguagePersonalInfo" /* Form and type name */

/* Address Book - "(FileProtection)" form item names */

#define FILE_PROTECTION_FORM "FileProtection" /* Form and type name */

/* Address Book - "(Mapping)" form item names */

#define MAPPING_FORM "Mapping" /* Form and type name */

/* Address Book - "(Virtual Server)" form item names */

#define VIRTUAL_SERVER_FORM "VirtualServer" /* Form and type name */

/* Address Book - "Users" name space, items, and names */
/* Address Book - local $Users namespace */
#define LOCAL_USERNAMESSPACE "1\\$Users"
#define USERNAMESSPACE "$Users"
#define USERNAMESSPACE_ALT "($Users)"
#define TYPEAHEADNAMESSPACE "$NamesFieldLookup"

/* DB2 - domudf namelookup items */
#define DB2_NAMELOOKUPITEMCOUNT     2
#define DB2_NAMELOOKUPITEMS         "FullName\0DB2UserName"

#define MAIL_NAMELOOKUPITEMCOUNT			17
#define MAIL_NAMELOOKUPSIMPLEITEMCOUNT		12	/* Excludes PublicKey */
#define MAIL_LOCALNAMELOOKUPSIMPLEITEMCOUNT 13 	/* Includes LNAB Entry */
#define MAIL_LOCALNAMELOOKUPITEMCOUNT		19	/* Includes LNAB fields */

#define MAIL_NAMELOOKUPITEMS	\
"FullName\0MailDomain\0MailAddress\0Location\0ListName\0Members\0InternetAddress\0AltFullName\0MessageStorage\0AltFullNameLanguage\0\
$$NoteID\0$$DBName\0$$LNAB\0PublicKey\0Certificate\0UserCertificate\0$$LNABHasCertificates\0$$LNABHasUserCertificates\0SmimeUserCapabilities\0"

#define FULLNAMEITEM			0
#define DOMAINNAMEITEM			1
#define MAILADDRESSITEM			2
#define LOCATIONITEM			3
#define LISTNAMEITEM			4
#define MEMBERSITEM				5
#define INTERNETADDRESSITEM		6
#define	ALTFULLNAMEITEM			7
#define STORAGETYPEITEM			8
#define ALTFULLNAMELANGUAGEITEM	9
#define NOTEIDITEM				10
#define DBNAMEITEM				11
#define LNABENTRY				12
#define PUBLICKEYITEM			13
#define CERTIFICATEITEM			14
#define USERCERTIFICATEITEM		15
#define HASCERTIFICATEITEM		16
#define HASUSERCERTIFICATEITEM	17
#define SMIMECAPABILITYITEM		18


/*	Mail Group Expansion NAMELookup items (router\expander.c) */
#define MAIL_GEX_NAMELOOKUPITEMCOUNT 5
#define MAIL_GEX_NAMELOOKUPITEMS "FullName\0ListName\0Members\0MailAddress\0$$Readers"
#define GEX_FULLNAMEITEM			0
#define GEX_LISTNAMEITEM			1
#define GEX_MEMBERSITEM				2
#define GEX_MAILADDRESSITEM			3
#define GEX_READERSITEM				4 

/* Lightweight Address book definitions */
#define LNAB_ENTRY_Y					'Y'
#define LNAB_ENTRY_YES					"Y"
#define LNAB_ENTRY_NO					"N"
#define LNAB_ENTRY_ITEM					"$$LNAB"
#define LNAB_HASCERTIFICATES_ITEM		"$$LNABHasCertificates"
#define LNAB_HASUSERCERTIFICATES_ITEM 	"$$LNABHasUserCertificates"
#define FIELD_JIT_ITEM					"$$JIT"
#define FIELD_JITFAILED_ITEM			"$$JITFailed"

#define FIELD_FROM_BOOKMARK_ITEM		"$$FromBookmark"
#define FIELD_SAVE_NOTE_CLASS_ITEM		"$$SaveNoteClass"
#define FIELD_OUTLINE_POSITION_ITEM		"OutlinePosition"

#define MAIL_DELIVERYLOOKUPITEMCOUNT	12
#define MAIL_DELIVERYSIMPLEITEMCOUNT 	 7			/* Excludes PublicKey */
#define MAIL_DELIVERYLOOKUPITEMS 		 \
"FullName\0MailAddress\0MailServer\0MailFile\0ShortName\0ListName\0$$Readers\0\
PublicKey\0EncryptIncomingMail\0MailDomain\0Certificate\0UserCertificate"

#define MAIL_DELIVERYFULLNAMEITEM	 		 0
#define MAIL_DELIVERYMAILADDRESSITEM		 1
#define MAIL_DELIVERYMAILSERVERITEM			 2
#define MAIL_DELIVERYMAILFILEITEM			 3
#define MAIL_DELIVERYSHORTNAMEITEM			 4
#define MAIL_DELIVERYLISTNAMEITEM			 5
#define MAIL_DELIVERYREADERSITEM			 6
#define MAIL_DELIVERYPUBLICINFO				 7
#define MAIL_DELIVERYENCRYPTINCOMING		 8
#define MAIL_DELIVERYMAILDOMAINITEM			 9
#define MAIL_DELIVERYCERTIFICATEITEM		10
#define MAIL_DELIVERYUSERCERTIFICATEITEM	11


/* Address Book - "Domains" name space, items, and names */

#define DOMAINNAMESSPACE "$Domains"
#define ALIASDOMAINNAMESSPACE "($Domains)"

#define MAIL_DOMAINLOOKUPITEMS		 "DomainName\0NextDomain\0MailServer\0MailFile"
#define MAIL_DOMAINNAMEITEM			 0
#define MAIL_DOMAINNEXTDOMAINITEM	 1
#define MAIL_DOMAINMAILSERVERITEM	 2
#define MAIL_DOMAINMAILFILEITEM		 3
#define MAIL_DOMAINLOOKUPITEMCOUNT	 4

#define MAIL_DEFAULTGLOBALDOMAIN_ITEM		"DefaultGlobalDomain"
#define MAIL_SMTPDOMAINSUFFIXES_ITEM		"SMTPDomainSuffixes"
#define MAIL_GLOBALDOMAIN_ITEM				"GlobalDomain"
#define MAIL_SMTPADDRESSFORMAT_ITEM			"SMTP822Format"
#define MAIL_SMTPNOTESDOMAININCLUDED_ITEM	"SMTPNotesDomainIncluded"
#define MAIL_SMTPNOTESDOMAINSEPCHAR_ITEM	"SMTPNotesDomainSepChar"
#define MAIL_SMTPNOTESDOMAINPOS_ITEM		"SMTPNotesDomainPos"
#define MAIL_SMTPADDRLOOKUP_ITEM			"SMTPAddrLookup"
#define MAIL_SMTPLOCALPART_ITEM				"SMTPLocalPart"
#define	MAIL_SMTPNOTESDOMAINLIST_ITEM		"NotesDomainList"
#define MAIL_SMTPALIASSEPCHAR_ITEM			"DomainSeparatorChar"
#define MAIL_SMTPADDRFORMAT_ADDRONLY		'1'
#define MAIL_SMTPADDRFORMAT_NAMEANDADDR		'2'
#define MAIL_SMTPLOCALPART_FULL 			'0'
#define MAIL_SMTPLOCALPART_INTEGER_FULL 	0
#define MAIL_SMTPLOCALDOMAINSINCLUDED_99	"99"
#define MAIL_SMTPLOCALDOMAINSINCLUDED_1		"1"
#define MAIL_SMTPLOCALDOMAINSINCLUDED_0		"0"
#define MAIL_SMTPLOCALPART_CN 				'1'
#define MAIL_SMTPLOCALPART_INTEGER_CN		1
#define MAIL_SMTPLOCALPART_SHORTNAME 		'2'
#define MAIL_SMTPLOCALPART_INTEGER_SHORTNAME 2
#define MAIL_SMTPLOCALDOMAINPOS_LEFT 		'1'
#define MAIL_SMTPLOCALDOMAINPOS_RIGHT 		'2'
#define MAIL_SMTPLOCALDOMAINSEPCHAR 		'%'
#define MAIL_SMTPLOCALDOMAINS_NONE			0
#define MAIL_SMTPLOCALDOMAINS_ONE			1
#define MAIL_SMTPLOCALDOMAINS_ALL			127
#define MAIL_SMTPADDRLOOKUP_DISABLED		'0'
#define MAIL_SMTPADDRLOOKUP_ENABLED			'1'

#define SMTPMTA_SUBFORM		"$SMTPServerFormSubForm"  /* Used by setup */

/* Address Book - "Servers" name space, items, and names */

#define SERVERNAMESSPACE		"$Servers"
#define SERVERNAMESSPACE_1		"1\\$Servers"
#define DIRECTORIESNAMESSPACE	"$Directories"
#define DIRECTORIESNAMESSPACE_1	"1\\$Directories"


#define MAIL_SERVERLOOKUPITEMS 		"ServerName\0Domain\0Network"
#define MAIL_SERVERNAMEITEM			0
#define MAIL_SERVERDOMAINITEM		1
#define MAIL_SERVERNETWORKITEM		2
#define MAIL_SERVERLOOKUPITEMCOUNT 	3

/* Address Book - "$People" name space, items, and names */

#define MAIL_PEOPLENAMESPACE "$People"

#define MAIL_PEOPLELOOKUPITEMS		"FullName"
#define MAIL_PEOPLEFULLNAMEITEM		0
#define MAIL_PEOPLELOOKUPITEMCOUNT	1

/* Address Book - "$Groups" name space, items, and names */

#define MAIL_GROUPSNAMESPACE		"$Groups"
#define MAIL_GROUPSNAMESPACE_1		"1\\$Groups"

#define REGISTER_GROUPSNAMESPACE "($RegisterGroups)"


/* Group type definitions */
#define GROUP_TYPE_MULTIPURPOSE		'0'
#define GROUP_TYPE_MAILONLY			'1'
#define GROUP_TYPE_ACCESSCONTROL	'2' 
#define GROUP_TYPE_DENYLIST			'3'

#define MAIL_GROUPSLOOKUPITEMCOUNT 1
#define MAIL_GROUPSLOOKUPITEMS "ListName"
#define MAIL_GROUPSGROUPNAMEITEM 0

/* Address Book - "$NameFieldLookup" name space, items, and names (Typeahead) */
#define LOCAL_MAIL_NAMEFIELD_LOOKUP "1\\$NamesFieldLookup"
#define MAIL_NAMEFIELD_LOOKUP	"$NamesFieldLookup"
#define TYPEAHEAD_STATUS_ITEM "TypeaheadStatus"
#define DISABLE_TYPEAHEAD	"DISABLE_TYPEAHEAD"

/* Address Book - Name field composite lookup */

#define MAIL_NAMEFIELD_COMPOSITE_NAMESSPACE	"$Users\0$Groups\0$ServersLookup"
#define NAMEFIELDCOMPOSITENAMESPACES 3

/* Address Book - Server lookup */

#define MAIL_SERVERSLOOKUP_NAMESSPACE	"$ServersLookup"

#define SERVERGROUPS_NAMESSPACE			"($ServerGroups)"



/*  Address Book - public views displayed by the admin subprogram */

#define ADMIN_GROUPS_VIEW 			"Groups"
#define ADMIN_PEOPLE_VIEW 			"People"
#define ADMIN_SERVERS_VIEW			"Servers"
#define ADMIN_CONFIGURATIONS_VIEW 	"Configurations"
#define ADMIN_WEBCONFIGURATIONS_VIEW "WebConfigurations"
#define ADMIN_CONNECTIONS_VIEW 		"Connections"
#define ADMIN_PROGRAMS_VIEW 		"Programs"
#define ADMIN_DOMAINS_VIEW 			"Domains"
#define ADMIN_CLUSTERS_VIEW   		"Clusters"
#define ADMIN_CERTIFICATES_VIEW		"Certificates"
#define ADMIN_LICENSES_VIEW   		"Licenses"
#define ADMIN_HOLIDAYS_VIEW   		"Holidays"
#define HIDDEN_HOLIDAYS_VIEW   		"($Holidays)"
#define HIDDEN_NOTESINI_VIEW   		"($ServerParameters)"
#define ADMIN_EXT_DOMAIN_NET_INFO_VIEW "ExternalDomainNetworkInformation"
#define ADMIN_REP_EVENTS_VIEW		"Replication Events"
#define ADMIN_MAIL_USERS			"Mail Users"
#define ADMIN_MAILBOX_VIEW			"Mail"
#define ADMIN_POLICIES_VIEW			"Policies"
#define ADMIN_SETTINGS_VIEW			"Settings"
#define ADMIN_DIRECTORIES_VIEW		"Directories"
#define ADMIN_AGGDIRCFG_VIEW		"AggregateDirConfig"
#define ADMIN_CERT_AUTH_VIEW		"Server\\Certificate Authorities"
#define ADMIN_FILE_ID_VIEW			"FileIdentifications"
#define ADMIN_HOSTED_ORGS_VIEW		"HostedOrganizations"
#define ADMIN_SITES_VIEW			"InternetSites"
#define ADMIN_CERT_EXP_VIEW			"PeopleCertExpiration"

/*	Address Book - Items and names used by Admin Panel */

#define ADMIN_SERVERTYPEITEM			0
#define ADMIN_SERVERNAMEITEM			1
#define ADMIN_SERVERTITLEITEM			2
#define ADMIN_SERVERDOMAINITEM			3
#define ADMIN_SERVERNETWORKITEM			4
#define ADMIN_SERVERCLUSTERNAMEITEM		5
#define ADMIN_SERVERADMINISTRATORITEM	6
#define ADMIN_SERVERVERSIONITEM			7
#define ADMIN_SERVEROSITEM				8
#define ADMIN_SERVERHOSTNAMEITEM		9
#define ADMIN_SERVERDB2INSTANCEITEM		10
#define ADMIN_SERVERDB2DIRECTORYITEM	11
#define ADMIN_SERVERDB2DATABASEITEM		12
#define ADMIN_SERVERDB2SCHEMAITEM		13
#define ADMIN_SERVERDB2HOSTITEM			14
#define ADMIN_SERVERDB2PORTITEM			15
#define ADMIN_SERVERDB2DEFAULTITEM		16
#define ADMIN_SERVERDB2UDFSRVITEM		17
#define ADMIN_SERVERDB2UDFPATHITEM		18
#define ADMIN_SERVERDB2ACCESSITEM		19
#define ADMIN_SERVERLOOKUPITEMCOUNT	   	20
#define ADMIN_SERVERLOOKUPITEMS "Type\0ServerName\0ServerTitle\0Domain\0Network\0ClusterName\0Administrator\0ServerBuildNumber\0ServerPlatform\0SMTPFullHostDomain\0DB2Inst\0DB2Dir\0DB2Db\0DB2Sche\0DB2Host\0DB2Port\0DB2Def\0DB2UDFSrv\0DB2UDFPath\0DB2AccessServer"

#define ADMIN_SERVERGROUPLISTITEM			0
#define ADMIN_SERVERGROUPLOOKUPITEMCOUNT	1
#define ADMIN_SERVERGROUPLOOKUPITEMS 		"ListName"


/*	Address Book - Items and names outputted to admindata.xml for the Java controller */

#define ADMINDATA_SERVERTYPEITEM			0
#define ADMINDATA_SERVERNAMEITEM			1
#define ADMINDATA_SERVERTITLEITEM			2
#define ADMINDATA_SERVERDOMAINITEM			3
#define ADMINDATA_SERVERNETWORKITEM			4
#define ADMINDATA_SERVERCLUSTERNAMEITEM		5
#define ADMINDATA_SERVERADMINISTRATORITEM	6
#define ADMINDATA_SERVERVERSIONITEM			7
#define ADMINDATA_SERVEROSITEM				8
#define ADMINDATA_SERVERHOSTNAMEITEM		9
#define ADMINDATA_SERVERCONTROLLERIP		10
#define ADMINDATA_SERVERCONTROLLERPORT		11
#define ADMINDATA_SERVERDB2ACCESSITEM		12
#define ADMINDATA_SERVERDB2SCHEMAITEM		13
#define ADMINDATA_SERVERLOOKUPITEMCOUNT	   	14
#define ADMINDATA_SERVERLOOKUPITEMS "Type\0ServerName\0ServerTitle\0Domain\0Network\0ClusterName\0Administrator\0ServerBuildNumber\0ServerPlatform\0SMTPFullHostDomain\0DC_IP_Address\0DC_Port\0DB2AccessServer\0DB2Sche"

/* Databases opened and displayed by the admin subprogram */

#define ADMIN_LOG_FILE				"log.nsf"
#define ADMIN_CATALOG5_FILE			"catalog.nsf"
#define ADMIN_CATALOG_FILE			"catalog.nsf"
#define ADMIN_EVENTS_FILE			"statrep.nsf"
#define ADMIN_EVENT4_CONFIG_FILE	"events4.nsf"
#define ADMIN_EVENT4_SAV_FILE		"events4.sav"  /* In R5, we save off the users EVENTS4.NSF as EVENTS4.SAV */
#define ADMIN_EVENT_CONFIG_FILE		"events4.nsf"
#define ADMIN_EVENT_CONFIG_TEMPLATE	"events4.ntf"
#define ADMIN_CERT_LOG_FILE			"certlog.nsf"
#define ADMIN_CERT_LOG_TEMPLATE		"certlog.ntf"
#define ADMIN_WEB_LOG_FILE			"domlog.nsf"
#define ADMIN_WEBCONFIG_FILE		"domcfg.nsf"
#define ADMIN_CERT_CA_FILE			"certca.nsf"
#define ADMIN_CERT_SRV_FILE			"certsrv.nsf"
#define ADMIN_REPORTS				"reports.nsf"
#define ADMIN_DECS_CFG				"decsadm.nsf"
#define ADMIN_DOLS_CFG				"doladmin.nsf"
#define ADMIN_FILE					"domadmin.nsf"
#define ADMIN_LDAP_SCHEMA			"schema.nsf"
#define ADMIN_LDAP_R5_SCHEMA		"schema50.nsf"
#define ADMIN_SMART_UPDATE_FILE		"smupdate.nsf"
#define ADMIN_LICENSE_TRK_FILE		"userlicenses.nsf"
#define ADMIN_CERT_PUB_FILE			"certpub.nsf"
#define ADMIN_CERT_REQ_FILE			"certreq.nsf"

#define ADMIN_FAULTREPORTS_FILE 	"lndfr.nsf"
#define ADMIN_FAULTREPORTS_TEMPLATE	"lndfr.ntf"

#define ADMIN_REPORT_FORM			"faReport"

/* SMTP MTA files */
#define ADMIN_SMTP_BOX				"smtp.box"
#define ADMIN_SMTP_IQ				"smtpibwq.box"
#define ADMIN_SMTP_OQ				"smtpobwq.box"

/* cc:Mail MTA files */
#define ADMIN_CCMTA_BOX				"ccmail.box"
#define ADMIN_CCMTA_WQ				"ccmcwq.box"

/* X.400 MTA files */
#define ADMIN_X400_BOX				"x400.box"
#define ADMIN_X400_DEAD				"dead.box"
#define ADMIN_X400_LOG				"x400log.nsf"
#define ADMIN_X400_MADMAN			"madman.nsf"
	
/* Softswitch files */
#define ADMIN_SSW					"ssw.nsf"

/* Defense Messaging System files */
#define ADMIN_DMS_BOX				"dms.box"
#define ADMIN_DMS_HOLD				"dmshold.box"

/* MTA Config dbs */
#define ADMIN_MTA_CFG				"mtatbls.nsf"  /* MTA configuration tables, used by smtp, ccmail and x400 mta */
#define ADMIN_MTA_FORMS				"mtaforms.nsf" /* MTA forms, also used by all mta's */

/* View in MAB used by admin */
#define ADMIN_DA_VIEW				"Directory Assistance"

/* Schedule items in the events4.nsf */

#define EVENT_SCHED_TYPE_ITEM				"$sched.def"
#define EVENT_SCHED_INTERVAL_UNIT_ITEM		"$sched.unit"
#define EVENT_SCHED_INTERVAL_ITEM			"$sched.freq"
#define EVENT_SCHED_24X7_ITEM				"$sched.always"
#define EVENT_SCHED_WEEKDAYS_ITEM			"$sched.days"
#define EVENT_SCHED_WEEKDAY_ITEM			"$sched.single.day"
#define EVENT_SCHED_START_HOURS_ITEM		"$sched.start"
#define EVENT_SCHED_END_HOURS_ITEM			"$sched.end"
#define EVENT_SCHED_TIME_ITEM				"$sched.spec.time"
#define EVENT_SCHED_MONTHDAY_ITEM			"$sched.dom"
#define EVENT_SCHED_MISS_ITEM				"$sched.miss"

#define EVENT_PROBE_INTERVAL_SECONDS		1
#define EVENT_PROBE_INTERVAL_MINUTES		2
#define EVENT_PROBE_DEFAULT_WEEKLY_DAY	"2"
#define EVENT_PROBE_DEFAULT_MONTH_DAY	"1"

#define EVENT_PROBE_TARGETSERVERS           "TargetServers"
#define EVENT_PROBE_PROBESERVER		        "ProbeServer"

#define EVENT_PROBE_MISS_IGNORE				1
#define EVENT_PROBE_MISS_STARTUP			2
#define EVENT_PROBE_MISS_SCHED				3

#define EVENT_PROBE_INTERVAL_SCHED			1
#define EVENT_PROBE_DAILY_SCHED				2
#define EVENT_PROBE_WEEKLY_SCHED			3
#define EVENT_PROBE_MONTHLY_SCHED			4

#define EVENT_PROBE_APP_ITEM				"ProbeApp"
#define EVENT_PROBE_DESCRIPTION_ITEM		"description"
#define EVENT_PROBE_AREA_ITEM				"DNTType"
#define EVENT_PROBE_AREA_SUBTYPE_ITEM		"SubType"
#define EVENT_PROBE_DLLNAME_ITEM			"probedll"
#define EVENT_PROBE_DLLFUNC_ITEM			"func"
#define EVENT_PROBE_DISABLED_ITEM			"Disabled"
#define EVENT_PROBE_PUID_ITEM				"puid"
#define EVENT_PROBE_FORM_ITEM				"Form"

#define ISPY_MAIL_FAIL_TIMEOUT				"msg.nrpc.timeout.fail"
#define ISPY_MAIL_WARN_HIGH_TIMEOUT			"msg.nrpc.timeout.warn.high"
#define ISPY_MAIL_WARN_LOW_TIMEOUT			"msg.nrpc.timeout.warn.low"
#define ISPY_MAIL_FAIL_ENABLE				"msg.nrpc.fail.enable"
#define ISPY_MAIL_WARN_HIGH_ENABLE			"msg.nrpc.warn.high.enable"
#define ISPY_MAIL_WARN_LOW_ENABLE			"msg.nrpc.warn.low.enable"
#define ISPY_MAIL_TARGET_RECIPIENT			"msg.targetrecipient"
#define ISPY_MAIL_MAIL_STATISTIC_NAME		"MailProbeStatisticName"
#define ISPY_MAIL_SOURCE_SERVER				"SourceServerName"
#define ISPY_MAIL_ALL_SELF_MONITOR			"AllSelfMonitor"
#define ISPY_MAIL_SHOW_HOPS					"ShowHops"
#define ISPY_PROBE_METHOD					"ProbeMethod"

#define ISPY_TCP_TARGET_SERVER			"TargetServerName"
#define ISPY_TCP_TCP_TIMEOUT			"TCPTimeOut"
#define ISPY_TCP_SSL_TIMEOUT			"SSLTimeOut"
#define ISPY_TCP_HTTP_URL				"HTTPUrl"
#define ISPY_TCP_HTTP_URL_FETCH			"HTTPUrlFetch"
#define ISPY_TCP_HTTPSSL_URL			"HTTPSSLUrl"
#define ISPY_TCP_HTTPSSL_URL_FETCH		"HTTPSSLUrlFetch"
#define ISPY_TCP_NNTP_COMMAND_SEND		"NNTPCommandSend"
#define ISPY_TCP_NNTP_COMMAND			"NNTPCommand"
#define ISPY_TCP_NNTP_PARAMETERS		"NNTPParameters"
#define ISPY_TCP_NNTP_GROUP				"NNTPGroup"
#define ISPY_TCP_NNTPSSL_COMMAND_SEND	"NNTPSSLCommandSend"
#define ISPY_TCP_NNTPSSL_COMMAND		"NNTPSSLCommand"
#define ISPY_TCP_NNTPSSL_PARAMETERS		"NNTPSSLParameters"
#define ISPY_TCP_NNTPSSL_GROUP			"NNTPSSLGroup"
#define ISPY_TCP_SOURCE_SERVER			"SourceServerName"
#define ISPY_TCP_ALL_SELF_MONITOR		"AllSelfMonitor"
#define ISPY_TCP_SERVICES				"Services"
#define ISPY_TCP_ALL_SERVICES			"AllServices"
#define ISPY_TCP_ALL_TARGET_SERVERS		"AllTargetServers"

#define ISPY_MAIL_POLLER_FUNC				"GetMailPollerProbe"
#define ISPY_TCP_POLLER_FUNC				"GetTCPPollerProbe"

#define EVENT_PROBE_TARGETDBS				"TargetDbs"

#define	DDM_PROBE_TYPE_AGENT				0
#define	DDM_PROBE_TYPE_APPLICATION			1
#define	DDM_PROBE_TYPE_DIRECTORIES			2
#define	DDM_PROBE_TYPE_MESSAGING			3
#define	DDM_PROBE_TYPE_REPLICATION			4
#define	DDM_PROBE_TYPE_SECURITY				5
#define	DDM_PROBE_TYPE_WEB					6
#define	DDM_PROBE_TYPE_ADMINP   			7

#define	DDM_PROBE_TEXT_AGENT				"application code" /* formerlly "agent", made more inclusive for applets,web services */
#define	DDM_PROBE_TEXT_APPLICATION			"application"
#define	DDM_PROBE_TEXT_DATABASE				"database"
#define	DDM_PROBE_TEXT_DIRECTORIES			"directory"
#define	DDM_PROBE_TEXT_MESSAGING			"messaging"
#define	DDM_PROBE_TEXT_OS					"operating system"
#define	DDM_PROBE_TEXT_REPLICATION			"replication"
#define	DDM_PROBE_TEXT_SECURITY				"security"
#define	DDM_PROBE_TEXT_WEB					"web"
#define	DDM_PROBE_TEXT_ADMINP   			"server"

#define	DDM_PROBE_SUBTYPE_TEXT_COMPACT		"compact"
#define	DDM_PROBE_SUBTYPE_TEXT_DB			"Database"
#define	DDM_PROBE_SUBTYPE_TEXT_DESIGN		"design"
#define	DDM_PROBE_SUBTYPE_TEXT_CONFIG		"Configuration"
#define	DDM_PROBE_SUBTYPE_TEXT_BPCONFIG		"bestpractices"
#define	DDM_PROBE_SUBTYPE_TEXT_CONFIGREVIEW	"review"
#define	DDM_PROBE_SUBTYPE_TEXT_DBREVIEW		"dbreview"
#define	DDM_PROBE_SUBTYPE_TEXT_ADMINP		"adminp"


#define DIR_PROBE_SUBTYPE_TEXT_LDAP_VIEW_UPDATE		"ldapviewupdate"
#define DIR_PROBE_SUBTYPE_TEXT_LDAP_SEARCH_RESPONSE	"ldapsearchresponse"
#define DIR_PROBE_SUBTYPE_TEXT_DA_SEARCH_RESPONSE	"secldapsearchresponse"
#define DIR_PROBE_SUBTYPE_TEXT_DD_SEARCH_RESPONSE	"nrpcsearchresponse"
#define DIR_PROBE_SUBTYPE_TEXT_AVAILABILITY			"availability"
#define DIR_PROBE_SUBTYPE_TEXT_AGGREGATION			"aggregation"
#define DIR_PROBE_SUBTYPE_TEXT_AGGREGATION_SCHED	"aggregationsched"
#define DIR_PROBE_TARGET_TEXT_NAMES_VIEW_UPDATE		"NamesViewUpdate"

#define DDM_TSK_CHECK_FUNC	"GetLegacyProbeTaskCheck"
#define DDM_SRV_ACCESS_FUNC	"GetLegacyProbeServerAccess"
#define DDM_FILE_CHECK_FUNC	"GetLegacyProbeFileCheck"

#define DDM_TARGET_PORT		"TargetPort"
#define DDM_TARGET_DOMAIN	"TargetDomain"	 
#define DDM_TARGET_REPID	"TargetReplicaID"	 

#define DDM_JS_APP_EVENT			0
#define DDM_JS_APP_ISPY				1
#define DDM_JS_APP_COUNT			2

#define EVENT_PROBE_APP_EVENT		1
#define EVENT_PROBE_APP_ISPY		2
#define EVENT_PROBE_APP_EMBEDDED	3

#define EVENT_PROBE_TARGET_SPECIAL_PREFIX			'$'
/* DDM special target servers */
#define EVENT_PROBE_SERVER_SPECIAL_ADMIN				"$AdminServer"
#define EVENT_PROBE_SERVER_SPECIAL_LDAPSERVER			"$LDAPServer"
#define EVENT_PROBE_SERVER_SPECIAL_AGGSERVER_SCHED		"$ScheduledDircatServer"
#define EVENT_PROBE_SERVER_SPECIAL_IMAPSERVER			"$IMAPServer"
#define EVENT_PROBE_SERVER_SPECIAL_POP3SERVER			"$POP3Server"
#define EVENT_PROBE_SERVER_SPECIAL_SMTPINSERVER			"$SMTPInServer"
#define EVENT_PROBE_SERVER_SPECIAL_MAILROUTINGSERVER	"$MailRoutingServer"

#define EVENTS_DDM_PROBE_VIEW				"($DDMConfig)"

#define DDM_REPL_FLAGS_PULL				0x0001
#define DDM_REPL_FLAGS_PUSH				0x0002

#define DDM_REPL_FLAGS_NOTE_CONFLICT	0x0004
#define DDM_REPL_FLAGS_NOTE_ERR			0x0008
#define DDM_REPL_FLAGS_DISABLE_ON_ORIG 	0x0010
#define DDM_REPL_FLAGS_DISABLE_ON_TARG 	0x0020
#define DDM_REPL_FLAGS_NOTE_NONDATA 	0x0040

#define DDM_CONFIG_CHECK_TYPE_REPLCHECK	0x0001
#define DDM_CONFIG_CHECK_TYPE_ERROR		0x0002
#define DDM_REPL_FLAGS_NOTE_CONFLICT	0x0004

/* View in events5 displayed by the monitor subprogram */

#define ADMIN_STATSEL_VIEW			"($StatSelection)"
#define ADMIN_NUM_STATSEL_VIEW		"($NumericalStatSelection)"
#define ADMIN_STATISTIC_VIEW		"($StatisticsPer)"
#define ADMIN_NUMBER_TYPE			"Number"
#define EVENTS_VIEW					"($Events)"
#define ADMIN_NUM_STATSEL_R5_VIEW	"($R5Statistics)"
#define ADMIN_NUM_STATSEL_R6_VIEW	"($D6Statistics)"

#define ADMIN_TRENDS_ALL_VIEW			"($StatTrendsAll)"
#define ADMIN_TRENDS_SERVERS_VIEW		"($StatTrendsServer)"
#define ADMIN_TRENDS_DAILY_VIEW			"($StatTrendsServerDaily)"
#define ADMIN_TRENDS_HOURLY_VIEW		"($StatTrendsServerHourly)"
#define ADMIN_TRENDS_DB_VIEW			"($StatTrendsDatabase)"
#define ADMIN_TRENDS_USERS_VIEW 		"($StatTrendsUser)"
#define ADMIN_TRENDS_PROFILE_VIEW 		"($StatTrendsActivityProfile)"
#define ADMIN_TRENDS_CONNECTIONS_VIEW	"($StatTrendsConnections)"
#define ADMIN_LOADBAL_VIEW				"($LoadBalanceStats)"
#define ADMIN_LOADBAL_PROFILES_VIEW		"($LoadBalanceProfiles)"

#define ADMIN_STATNAME_ITEM			"StatName"
#define ADMIN_HASTRENDED_ITEM		"HasTrended"
#define ADMIN_HASPRIME_ITEM 		"HasPrime"
#define ADMIN_UNITS_ITEM			"Units"
#define ADMIN_DESCRIPTION_ITEM		"Description"
#define ADMIN_PROFILESTAT_ITEM		"IsProfileStat"
#define ADMIN_LOADBALSTAT_ITEM		"IsLoadBalance"
#define ADMIN_LOADBALCAT_ITEM		"LoadBalancingCategory"
#define ADMIN_LOADBALNAME_ITEM		"LoadBalanceName"
#define ADMIN_COMPOSITESTATS_ITEM	"CompositeStats"

#define ADMIN_NUMBERLIST_TYPE		"NumberList"
#define ADMIN_TEXTLIST_TYPE 		"TextList"
#define ADMIN_TEXT_TYPE             "Text"
#define ADMIN_TIMEDATE_TYPE         "Time"
#define ADMIN_SMART_UPDT_LINK_ITEM	"SmartUpdDBLink"
#define ADMIN_SU_GOVERNOR_ENABLED	"SUGEnabled"
#define ADMIN_SU_GOVERNOR_MAX_USER	"SUGMaxUser"


/* Form and view in domadmin used by trends charts */
#define ADMIN_TRENDS_PROFILES_FORM	"GraphTrendsProfile"
#define ADMIN_TRENDS_PROFILES_VIEW	"($GraphTrendsProfiles)"

/* Trends profile types */
#define ADMIN_TRENDS_PROFILE_SERVER		"Server"
#define ADMIN_TRENDS_PROFILE_DB			"Database"
#define ADMIN_TRENDS_PROFILE_USER		"User"
#define ADMIN_TRENDS_PROFILE_CONNECTION	"Connection"
#define ADMIN_TRENDS_PROFILE_HISTORY	"History"
#define ADMIN_TRENDS_PROFILE_DAILY		"Daily"
#define ADMIN_TRENDS_PROFILE_HOURLY		"Hourly"
#define ADMIN_TRENDS_PROFILE_ACTIVITY	"Activity"

/*	Database containing UNAME database directory */

#define	UNAME_FILE					"unames.nsf"
	
/*  Address Book - "ServerAccess" namespace, items, and names */

#define ACCESS_GROUPS_NAMESPACE		"$ServerAccess"
#define ACCESS_GROUPS_NAMESPACE_1	"1\\$ServerAccess"

#define MAIL_GROUPS_NAMESPACE		"$MailGroups"
#define MAIL_GROUPS_NAMESPACE_1		"1\\$MailGroups"

#define ACCESS_ITEM_LISTNAME				0
#define ACCESS_ITEM_DBINDEX					1
#define ACCESS_ITEM_DOMAINTYPE				2
#define ACCESS_GROUPEXP_LOOKUPITEMCOUNT		3
#define ACCESS_LISTNAME_ITEM				"ListName"
#define ACCESS_GROUPEXP_LOOKUPITEMS			"ListName\0$$DBIndex\0$$DomainType"

/*  Address Book - $Users name space, to look up server access */

#define ACCESS_LOOKUP_ITEMS		"FullName"
#define ACCESS_ITEM_FULLNAME	0
#define ACCESS_ITEM_COUNT		1

/*  Address Book -- $Adminp name space, for admin proxy agent */
#define ADMINP_NAMESPACE "$Adminp"

/*  Address Book configuration parameters for the administration process */
#define ADMINP_CONFIG_MAX_THREADS	"AdminPMaxThreads" /* number */
#define ADMINP_CONFIG_INTERVAL	 	"AdminPInterval"   /* number */
#define ADMINP_CONFIG_DAILY_TIME	"AdminPDailyTime"  /* time   */
#define ADMINP_CONFIG_WEEK_DAYS		"AdminPWeekDays"   /* text list */
#define ADMINP_CONFIG_DELAYED_TIME  "AdminPDelayedTime"/* time */
#define ADMINP_CONFIG_DEL_MAIL_FILE "AdminPMailFileDelInt" /* number */
#define ADMINP_CONFIG_SUSPEND_RANGE "AdminPSuspendRange" /* timedate range */
#define ADMINP_CONFIG_SAVE_NRESP	"AdminPLogSetting"
#define ADMINP_CONFIG_NAME_CHANGE_X	"AdminPNameChangeExpiration"

#define ADMINP_LOCAL_ADMIN			"LocalAdmin"
#define ADMINP_ALLOW_ACCESS			"AllowAccess"
#define ADMINP_DENY_ACCESS			"DenyAccess"
#define ADMINP_CREATE_ACCESS		"CreateAccess"
#define ADMINP_REPLICA_ACCESS		"ReplicaAccess"
#define ADMINP_PT_ACCESS			"PTAccess"
#define ADMINP_PT_CLIENTS			"PTClients"
#define ADMINP_PT_CALLERS			"PTCallers"
#define ADMINP_PT_TARGETS			"PTTargets"
#define ADMINP_LIST_OWNER			"ListOwner"
#define ADMINP_PASSTHRU_SERVER		"PassthruServer"
#define ADMINP_SMTP_ADMINISTRATOR	"SMTPAdministrator"
#define ADMINP_GROUP_TYPE			"GroupType"
#define ADMINP_PRIVATE_LIST     	"PrivateList"
#define ADMINP_RESTRICTED_LIST		"RestrictedList"
#define ADMINP_UNRESTRICTED_LIST	"UnrestrictedList"
#define ADMINP_SERVERS_CPU_COUNT	"ServerCPUCount"
#define ADMINP_X509_CERT_PRESENT	"$X509CertIsPresent"
#define ADMINP_CCMAIL_ADMINISTRATOR	"CCMAdmin"
#define ADMINP_REQUESTING_SERVER_ITEM	"RequestingServer"

#define ADMINP_ACCESS_SERVER_INPUT_ITEM		"AccessServerInput"
#define ADMINP_ALLOW_NOTES_ADDR_ITEM	"AllowNotesAddressesToMail"
#define ADMINP_DENY_NOTES_ADDR_ITEM		"DenyNotesAddressesToMail"
#define ADMINP_NOI_RESTRICTED_ITEM		"NOIRestrictedList"
#define ADMINP_NOI_UNRESTRICTED_ITEM	"NOIUnrestrictedList"
#define ADMINP_REPORT_RECIPIENTS_ITEM	"ReportRecipients"

#define ADMINP_STATS_CPU_COUNT		"CPU.Count"
#define ADMINP_SERVER_PLATFORM		"ServerPlatform"
#define ADMINP_SERVER_DIRECTORY_NAME "ServerDirectoryName"
#define ADMINP_SERVER_CONFIG_NAB	 "CfgNABonly"

#define ADMINP_PROFILE_NOTEID 				"%NoteID"
#define ADMINP_PROFILE_SIGNATURE_ITEM 		"$Sig_Signature"
#define ADMINP_XDOMAIN_TRUSTED_ADMINS_ITEM 	"CrossDomainTrustedAdmins"
#define ADMINP_DIR_PROFILE_NAME				"DirectoryProfile"

/* values for resource database */
#define ADMINP_RESOURCE_DOCUMENT	"Resource"
#define ADMINP_RESOURCE_NAME_ITEM	"ResourceName"
#define ADMINP_PROGRAM_FORM			"Program"

/* known values of fields in the proxy database document form */
#define AdminpDelete 							"0"
#define AdminpRenameInTheACL					"1"
#define AdminpCopyPublicKey 					"2"
#define AdminpStoreServerVersion				"3"
#define AdminpRenameServerInNAB 				"4"
#define AdminpRenameUserInNAB 					"5"
#define AdminpMoveUserInHier 					"6"
#define AdminpDeleteStats						"7"
#define AdminpInitiateNABChange					"8"
#define AdminpRecertServerInNAB					"9"
#define AdminpRecertUserInNAB					"10"
#define AdminpServerClusterAdd					"11"
#define AdminpServerClusterRemove				"12"
#define AdminpCreateReplicas					"13"
#define AdminpMoveReplicas						"14"
#define AdminpPendedDeleteForMove				"15"
#define AdminpDeleteInPersonDocs				"16"
#define AdminpDeleteInTheACL					"17"
#define AdminpDeleteInReadersAuthors			"18"
#define AdminpRenameInPersonDocs				"19"
#define AdminpRenameInReadersAuthors			"20"
#define AdminpDeleteMailFile					"21"
#define AdminpApproveMailFileInfo				"22"
#define AdminpDeleteUnlinkedMailFile			"23"
#define AdminpCreateMailFile					"24"
#define AdminpMonitorMovedReplica				"25"
#define AdminpDeleteChangeRequests				"26"
#define AdminpGetMailFileInfo					"27"
#define AdminpRequestDeleteMailFile				"28"
#define AdminpResourceAdd						"29"
#define AdminpResourceDelete					"30"
#define AdminpApproveResourceDelete				"31"
#define AdminpCreateReplicasCheckAccess			"32"
#define AdminpMoveReplicasCheckAccess			"33"
#define AdminpSetPasswordFields					"34"
#define AdminpUpdateUserPW						"35"
#define AdminpUpdateServerPW					"36"
#define AdminpSetMABField						"37"
#define AdminpRenamePersonInFreeTime			"38"
#define AdminpRenamePersonInMailFile			"39"
#define AdminpRenameGroupInNAB					"40"
#define AdminpRenameGroupInPersonDocs			"41"
#define AdminpRenameGroupInTheACL				"42"
#define AdminpRenameGroupInReadersAuthors		"43"
#define AdminpAddPersonsX509Certificate			"44"
#define AdminpCheckMailServersAccess			"45"
#define AdminpUpgradeUser						"46"
#define AdminpCopyExternalDomainAddresses		"47"	
#define AdminpPromoteMailServersAccess			"48"
#define AdminpCreateNewMailFileReplica			"49"
#define AdminpAddNewMailFileFields				"50"
#define AdminpMonitorNewMailFileFields			"51"
#define AdminpReplaceMailFileFields				"52"
#define AdminpLastPushToNewMailServer			"53"
#define AdminpDeletePersonInNAB					"54"
#define AdminpDeleteServerInNAB      			"55"
#define AdminpDeleteGroupInNAB      			"56"
#define AdminpDelegateMailFile      			"57"
#define AdminpApproveDeletePersonInNAB      	"58"
#define AdminpApproveDeleteServerInNAB      	"59"
#define AdminpApproveRenamePersonInNAB      	"60"
#define AdminpApproveRenameServerInNAB      	"61"
#define AdminpResourceModify					"62"
#define AdminpUpdateNetworkTables				"63"
#define AdminpCreateISPYMailInDb				"64"
#define AdminpNCMoveReplicasCheckAccess			"65"
#define AdminpNCMoveReplicas					"66"
#define AdminpStoreServerCPUCount				"67"
#define AdminpRenamePersonInUnreadList			"68"
#define AdminpDeleteReplicaAfterMove			"69"
#define AdminpSetDNSFullHostName				"70"
#define AdminpStoreServerPlatform				"71"
#define AdminpApproveDeleteDesignElements		"72"
#define AdminpRequestDeleteDesignElements		"73"
#define AdminpDeleteDesignElements				"74"
#define AdminpApproveDeleteMovedReplica			"75"
#define AdminpRequestDeleteMovedReplica			"76"
#define AdminpSetDomainCatalog					"77"
#define AdminpWebDelegateMailFile				"78"
#define AdminpGetFileInfo						"79"
#define AdminpRequestDeleteFile					"80"
#define AdminpDeleteFile						"81"
#define AdminpApproveFileInfo					"82"
#define AdminpSetWebAdminFields					"83"
#define AdminpAcceleratedCreateReplica			"84"
#define AdminpSetConfigNAB						"85"
#define AdminpStoreServerDirectoryName			"86"
#define	AdminpCreateRoamingUserRoamingFiles		"87"
#define	AdminpPromoteRoamingServersAccess		"88"
#define	AdminpReplaceRoamingServerField			"89"
#define	AdminpMonitorMovedRoamingReplica		"90"
#define	AdminpCreateRoamingReplStubs			"91"
#define	AdminpRemoveRoamingUserRoamingFiles		"92"
#define	AdminpCheckRoamingServerAccess			"93"
#define	AdminpCreateRoamingReplicas				"94"
#define AdminpCertPublicationRequest			"95"
#define AdminpCrlPublicationRequest				"96"
#define AdminpUserModifyRequest					"97"
#define AdminpCertRemoveRequest					"98"
#define AdminpPolicyPublicationRequest			"99"	
#define AdminpLastPushToNewRoamingServer		"100"
#define AdminpSignDatabase						"101"
#define AdminpCAConfigPublicationRequest 		"102"	
#define AdminpCrlRemoveRequest					"103"
#define AdminpDelegateIMAPMailFiles				"104"
#define AdminpCAConfigToBeSigned           		"105"
#define AdminpRejectRenameUserInNAB				"106"
#define AdminpRetractNameChange           		"107"
#define AdminpEnableMailAgent					"108"
#define AdminpReportServerUse					"109"
#define AdminpRejectRetractNameChange			"110"
#define AdminpDeleteServerFromCatalog			"111"
#define AdminpCopyTrendsRecord					"112"
#define AdminpDeletePolicy						"113"
#define AdminpApproveRetractNameChange          "114"
#define AdminpApproveRecertify   		        "115"
#define AdminpApproveNameChange   			    "116"
#define AdminpApproveNewPublicKeys         		"117"
#define AdminpInitiateWebNameChange				"118"
#define AdminpRenameWebNameInTheACL				"119"
#define AdminpRenameWebNameInNAB 				"120"
#define AdminpRenameWebNameInPersonDocs			"121"
#define AdminpRenameWebNameInReadersAuthors		"122"
#define AdminpRenameWebNameInFreeTime			"123"
#define AdminpRenameWebNameInMailFile			"124"
#define AdminpRenameWebNameInUnreadList			"125"
#define AdminpRemoveNameChangeInLDAPDir			"126"
#define AdminpChangeHTTPPasswordRequest         "127"
#define AdminpDefineServerMonitorQuery 			"128"
#define AdminpCollectServerMonitorData 			"129"
#define AdminpConsolidateServerMonitorData 		"130"
#define AdminpCreateIMAPDelegations		 		"131"
#define AdminpDeleteHostedOrg                   "132"
#define AdminpUpdateRoamingState				"133"
#define AdminpUpdateRoamingFields				"134"
#define AdminpCreateHostedOrgStorage		 	"135"
#define AdminpRecertCrossCert					"136"
#define AdminpCreateObjStore		 			"137"
#define AdminpDeleteHostedOrgStorageGetInfo	 	"138"
#define AdminpApproveDeleteHostedOrgStorage	 	"139"
#define AdminpDeleteHostedOrgStorage		 	"140"
#define AdminpRecertCAInNAB 					"141"
#define AdminpFindNameInDomain					"142"
#define AdminpVerifyHostedOrgStorage			"143"
#define AdminpAddGroup							"144"
#define AdminpRecoveryIdRequest					"145"
#define AdminpPublishRecoveryInfo				"146"
#define AdminpDeletePersonInUnreadList			"147"
#define AdminpMonitorRoamingReplStubs			"148"
#define AdminpDelegateOnAdminServer				"149"
#define AdminpCreateReplicasCheckExeTime		"150"
#define AdminpMoveReplicasCheckExeTime			"151"
#define AdminpMoveMailCheckExeTime				"152"
#define AdminpNCMoveReplicasCheckExeTime		"153"
#define AdminpRegNewUserWithPolicy				"154"
#define AdminpRegNewUser						"155"
#define AdminpUpdateServerKeyring				"156"
#define AdminpEnableSSLPorts					"157"
#define AdminpNewAgentsMachine					"158"
#define AdminpXCertPublicationRequest			"159"
#define AdminpWebEnableMailAgent				"160"
#define AdminpUpdateReplicaSettings				"161"
#define AdminpRenameInSharedAgents				"162"
#define AdminpWebMailSetSoftDeletionTime		"163"
#define AdminpRenameInAgentsReadersField		"164"
#define AdminpDeleteInAgentsReadersField		"165"
#define AdminpMonitorServerSSLStatus			"166"
#define AdminpDelegateOnHomeServer				"167"
#define AdminpSetFaultRecoverySettings			"168"
#define AdminpCertNewServerKey					"169"
#define AdminpCertNewPersonKey					"170"
#define AdminpCertNewCertifierKey				"171"
#define AdminpAddDB2ToServerDoc					"172"
#define AdminpMonitorDB2ReplStub				"173"
#define AdminpDB2SetID							"174"
#define AdminpDB2MoveContainer					"175"
#define AdminpRenamePersonInDesignElements		"176"
#define AdminpDeletePersonInDesignElements		"177"
#define AdminpDB2AccessConnection				"178"
#define AdminpRenameWebNameInDesignElements		"179"
#define AdminpRenameGroupInDesignElements		"180"
											
#define AdminpDeleteWord 			 			0
#define AdminpRenameInTheACLWord 	 			1
#define AdminpCopyPublicKeyWord 	 			2
#define AdminpStoreServerVersionWord 			3
#define AdminpRenameServerInNABWord  			4
#define AdminpRenameUserInNABWord	 			5
#define AdminpMoveUserInHierWord 	 			6
#define AdminpDeleteStatsWord		 			7
#define AdminpInitiateNABChangeWord	 			8
#define AdminpRecertServerInNABWord	 			9
#define AdminpRecertUserInNABWord	 			10
#define AdminpServerClusterAddWord				11
#define AdminpServerClusterRemoveWord			12
#define AdminpCreateReplicasWord				13
#define AdminpMoveReplicasWord					14
#define AdminpPendedDeleteForMoveWord			15
#define AdminpDeleteInPersonDocsWord			16
#define AdminpDeleteInTheACLWord				17
#define AdminpDeleteInReadersAuthorsWord		18
#define AdminpRenameInPersonDocsWord			19
#define AdminpRenameInReadersAuthorsWord		20
#define AdminpDeleteMailFileWord				21
#define AdminpApproveMailFileInfoWord			22
#define AdminpDeleteUnlinkedMailFileWord		23
#define AdminpCreateMailFileWord				24
#define AdminpMonitorMovedReplicaWord			25
#define AdminpDeleteChangeRequestsWord			26
#define AdminpGetMailFileInfoWord				27
#define AdminpRequestDeleteMailFileWord			28
#define AdminpResourceAddWord					29
#define AdminpResourceDeleteWord				30
#define AdminpApproveResourceDeleteWord			31
#define AdminpCreateReplicasCheckAccessWord		32
#define AdminpMoveReplicasCheckAccessWord		33
#define AdminpSetPasswordFieldsWord				34
#define AdminpUpdateUserPWWord					35
#define AdminpUpdateServerPWWord				36
#define AdminpSetMABFieldWord					37
#define AdminpRenamePersonInFreeTimeWord		38
#define AdminpRenamePersonInMailFileWord 		39
#define AdminpRenameGroupInNABWord	 			40
#define AdminpRenameGroupInPersonDocsWord		41
#define AdminpRenameGroupInTheACLWord			42
#define AdminpRenameGroupInReadersAuthorsWord	43
#define AdminpAddPersonsX509CertificateWord		44

#define AdminpNewAdminpRequestFormat 			45

#define AdminpCheckMailServersAccessWord 		45
#define AdminpUpgradeUserWord			 		46
#define AdminpCopyExternalDomainAddressesWord	47
#define AdminpPromoteMailServersAccessWord		48
#define AdminpCreateNewMailFileReplicaWord		49
#define AdminpAddNewMailFileFieldsWord			50
#define AdminpMonitorNewMailFileFieldsWord		51
#define AdminpReplaceMailFileFieldsWord			52
#define AdminpLastPushToNewMailServerWord		53
#define AdminpDeletePersonInNABWord				54
#define AdminpDeleteServerInNABWord				55
#define AdminpDeleteGroupInNABWord				56
#define AdminpDelegateMailFileWord      		57
#define AdminpApproveDeletePersonInNABWord      58
#define AdminpApproveDeleteServerInNABWord      59
#define AdminpApproveRenamePersonInNABWord      60
#define AdminpApproveRenameServerInNABWord      61
#define AdminpResourceModifyWord				62
#define AdminpUpdateNetworkTablesWord			63
#define AdminpCreateISPYMailInDbWord			64
#define AdminpNCMoveReplicasCheckAccessWord		65
#define AdminpNCMoveReplicasWord				66
#define AdminpStoreServerCPUCountWord			67
#define AdminpRenamePersonInUnreadListWord		68
#define AdminpDeleteReplicaAfterMoveWord		69
#define AdminpSetDNSFullHostNameWord			70
#define AdminpStoreServerPlatformWord			71
#define AdminpApproveDeleteDesignElementsWord	72
#define AdminpRequestDeleteDesignElementsWord	73
#define AdminpDeleteDesignElementsWord			74
#define AdminpApproveDeleteMovedReplicaWord		75
#define AdminpRequestDeleteMovedReplicaWord		76
#define AdminpSetDomainCatalogWord				77
#define AdminpWebDelegateMailFileWord			78
#define AdminpGetFileInfoWord					79
#define AdminpRequestDeleteFileWord				80
#define AdminpDeleteFileWord					81
#define AdminpApproveFileInfoWord				82
#define AdminpSetWebAdminFieldsWord				83
#define AdminpAcceleratedCreateReplicaWord		84
#define AdminpSetConfigNABWord					85
#define AdminpStoreServerDirectoryNameWord		86
#define	AdminpCreateRoamingUserRoamingFilesWord	87
#define	AdminpPromoteRoamingServersAccessWord	88
#define	AdminpReplaceRoamingServerFieldWord		89
#define	AdminpMonitorMovedRoamingReplicaWord	90
#define	AdminpCreateRoamingReplStubsWord		91
#define	AdminpRemoveRoamingUserRoamingFilesWord	92
#define	AdminpCheckRoamingServerAccessWord		93
#define	AdminpCreateRoamingReplicasWord			94
#define AdminpCertPublicationRequestWord		95
#define AdminpCrlPublicationRequestWord			96	/* AdminpCrlPublicationRequestWord */
#define AdminpUserModifyRequestWord				97
#define AdminpCertRemoveRequestWord				98
#define AdminpPolicyPublicationRequestWord		99	
#define AdminpLastPushToNewRoamingServerWord	100
#define AdminpSignDatabaseWord					101
#define AdminpCAConfigPublicationRequestWord   	102	/* AdminpCAConfigPublicationRequestWord */
#define AdminpCrlRemoveRequestWord				103
#define AdminpDelegateIMAPMailFilesWord			104
#define AdminpCAConfigToBeSignedWord           	105
#define AdminpRejectRenameUserInNABWord			106
#define AdminpRetractNameChangeWord           	107
#define AdminpEnableMailAgentWord				108
#define AdminpReportServerUseWord				109
#define AdminpRejectRetractNameChangeWord		110
#define AdminpDeleteServerFromCatalogWord		111
#define AdminpCopyTrendsRecordWord				112
#define AdminpDeletePolicyWord 				    113
#define AdminpApproveRetractNameChangeWord      114
#define AdminpApproveRecertifyWord   		    115
#define AdminpApproveNameChangeWord   			116
#define AdminpApproveNewPublicKeysWord        	117
#define AdminpInitiateWebNameChangeWord			118
#define AdminpRenameWebNameInTheACLWord			119
#define AdminpRenameWebNameInNABWord 			120
#define AdminpRenameWebNameInPersonDocsWord		121
#define AdminpRenameWebNameInReadersAuthorsWord	122
#define AdminpRenameWebNameInFreeTimeWord		123
#define AdminpRenameWebNameInMailFileWord		124
#define AdminpRenameWebNameInUnreadListWord		125
#define AdminpRemoveNameChangeInLDAPDirWord		126
#define AdminpChangeHTTPPasswordRequestWord     127
#define AdminpDefineServerMonitorQueryWord 		128
#define AdminpCollectServerMonitorDataWord 		129
#define AdminpConsolidateServerMonitorDataWord 	130
#define AdminpCreateIMAPDelegationsWord		 	131
#define AdminpDeleteHostedOrgWord				132
#define AdminpUpdateRoamingStateWord			133
#define AdminpUpdateRoamingFieldsWord			134
#define AdminpCreateHostedOrgStorageWord	 	135
#define AdminpRecertCrossCertWord				136
#define AdminpCreateObjStoreWord	 			137
#define AdminpDeleteHostedOrgStorageGetInfoWord	 138
#define AdminpApproveDeleteHostedOrgStorageWord	 139
#define AdminpDeleteHostedOrgStorageWord		 140
#define AdminpRecertCAInNABWord					141
#define AdminpFindNameInDomainWord				142
#define AdminpVerifyHostedOrgStorageWord		143
#define AdminpAddGroupWord						144
#define AdminpRecoveryIdRequestWord				145
#define AdminpPublishRecoveryInfoWord			146
#define AdminpDeletePersonInUnreadListWord		147
#define AdminpMonitorRoamingReplStubsWord		148
#define AdminpDelegateOnAdminServerWord			149
#define AdminpCreateReplicasCheckExeTimeWord	150
#define AdminpMoveReplicasCheckExeTimeWord		151
#define AdminpMoveMailCheckExeTimeWord			152
#define AdminpNCMoveReplicasCheckExeTimeWord	153
#define AdminpRegNewUserWithPolicyWord			154
#define AdminpRegNewUserWord					155
#define AdminpUpdateServerKeyringWord			156
#define AdminpEnableSSLPortsWord				157
#define AdminpNewAgentsMachineWord				158
#define AdminpXCertPublicationRequestWord		159
#define AdminpWebEnableMailAgentWord			160
#define AdminpUpdateReplicaSettingsWord			161
#define AdminpRenameInSharedAgentsWord			162
#define AdminpWebMailSetSoftDeletionTimeWord	163
#define AdminpRenameInAgentsReadersFieldWord	164
#define AdminpDeleteInAgentsReadersFieldWord	165
#define AdminpMonitorServerSSLStatusWord		166
#define AdminpDelegateOnHomeServerWord			167
#define AdminpSetFaultRecoverySettingsWord		168
#define AdminpCertNewServerKeyWord				169
#define AdminpCertNewPersonKeyWord				170
#define AdminpCertNewCertifierKeyWord			171
#define AdminpAddDB2ToServerDocWord				172
#define AdminpMonitorDB2ReplStubWord			173
#define AdminpDB2SetIDWord						174
#define AdminpDB2MoveContainerWord				175
#define AdminpRenamePersonInDesignElementsWord	176
#define AdminpDeletePersonInDesignElementsWord	177
#define AdminpDB2AccessConnectionWord			178
#define AdminpRenameWebNameInDesignElementsWord	179
#define AdminpRenameGroupInDesignElementsWord	180

#define ADMINP 									"Adminp"
#define ADMINP_ALL_SERVERS						"*"
#define ADMINP_DELETE_MAIL_FILE_ON_HOME_SERVER  "1"
#define ADMINP_DELETE_MAIL_FILE_REPLICAS		"2"
#define ADMINP_DELETE_ONE_MAIL_FILE_REPLICA		"3"
#define ADMINP_DELETE_FILE_REPLICAS				"4"
#define ADMINP_DELETE_ONE_FILE_REPLICA			"5"
#define ADMINP_DISABLED							"0"
#define ADMINP_ENABLED							"1"

/* defines for "AdminpChangeHTTPPasswordRequest" sub-functions */
#define ADMINP_CHANGE_HTTP_PASSWORD_NEW			0
#define ADMINP_CHANGE_HTTP_PASSWORD_SYNC		1

/* the view we use from the proxy database */
#define ADMINP_MAIN_VIEW 						"All Requests by Action"
#define ADMINP_BY_SERVER_RESPONSES_VIEW			"($LoggedEvents)"
#define ADMINP_ANY_SERVER_RESPONSES_VIEW 		"($LoggedAdminServerEvents)"
#define ADMINP_REQUESTS_VIEW					"($Requests)"
#define ADMINP_RENAME_IN_ACL_DELETE_VIEW		"($RenameInACLAndDeleteRequests)"
#define ADMINP_RECEIVED_REQUESTS_VIEW			"($ReceivedRequests)"
#define ADMINP_RECEIVE_CONFIG_VIEW				"($ReceiveCrossDomainRequests)"
#define ADMINP_SUBMIT_CONFIG_VIEW	   			"($SubmitCrossDomainRequests)"
#define	ADMINP_ORIG_UNID_VIEW					"$Requests by UNID"

/* fields in the proxy database document form */

#define ADMINP_PROXY_DOCUMENT 					"AdminRequest"
#define ADMINP_PROXY_XDOMAIN_DOCUMENT 			"CrossDomainAdminRequest"
#define ADMINP_DOC_SERVER_ITEM					"ProxyServer"
#define ADMINP_ROAMING_SERVER_ITEM				"ProxyRoamingServer"
#define ADMINP_ACTION_ITEM						"ProxyAction"
#define ADMINP_DOC_SERVER_BUILD_NUMBER_ITEM		"ProxyServerBuildNumber"
#define ADMINP_DOC_SOURCE_SERVER_ITEM			"ProxySourceServer"
#define ADMINP_DOC_DEST_SERVER_ITEM				"ProxyDestinationServer"
#define ADMINP_DOC_CLUSTER_NAME_ITEM			"ProxyClusterName"
#define ADMINP_NAME_LIST_ITEM 					"ProxyNameList"
#define ADMINP_DOC_AUTHOR_ITEM 	  				"ProxyAuthor"
#define ADMINP_DOC_PROCESS_ITEM					"ProxyProcess"
#define ADMINP_DOC_TARGET_CERTIFIER_ITEM		"ProxyTargetCertifier"
#define ADMINP_DOC_CERTIFICATE_ITEM 			"ProxyCertificate"
#define ADMINP_DOC_ROLLOVER_CERT_ITEM 			"ProxyRolloverCert"
#define ADMINP_DOC_CHANGE_REQUEST_ITEM			"ProxyChangeRequest"
#define ADMINP_DOC_CHANGE_SIGNATURE_ITEM		"ProxyChangeSignature"
#define ADMINP_DOC_REPLICA_ID_ITEM				"ProxyReplicaId"
#define ADMINP_ROAMING_DB_REPIDS				"ProxyRoamingReplicaIds"
#define ADMINP_ROAMING_DB_LIST					"ProxyRoamingReplicaList"
#define ADMINP_ROAMING_DB_DISPLAY_LIST			"ProxyRoamingPendingReplicaList"
#define ADMINP_ROAMING_DB_PROMOTE_LIST			"ProxyRoamingPromoteServerList"
#define ADMINP_ROAMING_DB_REMOVE_LIST			"ProxyRoamingRemoveReplicaList"
#define ADMINP_DOC_DATABASE_PATH_ITEM			"ProxyDatabasePath"
#define ADMINP_ROAMING_DATABASE_PATH_ITEM		"ProxyRoamingDatabasePath"
#define ADMINP_ROAMING_DATABASE_DEST_PATH_ITEM	"ProxyRoamingDestDatabasePath"
#define	ADMINP_ROAMING_ADDRESS_BOOK_ITEM		"ProxyRoamingAddressBook"
#define	ADMINP_ROAMING_STATE_CHANGE_ITEM		"ProxyRoamingStateChange"
#define ADMINP_ROAMING_MODE_ITEM				"ProxyRoamingMode"
#define ADMINP_ROAMING_SETTING_ITEM				"ProxyRoamingSetting"
#define ADMINP_ROAMING_CLEANUP_PERIOD_ITEM		"ProxyRoamingCleanupPeriod"
#define ADMINP_ROAMING_ID_IN_NAB_ITEM			"ProxyRoamingIdInNab"
#define ADMINP_ROAMING_CLIENT_PROMPT_ITEM		"ProxyRoamingClientPrompt"
#define ADMINP_DOC_DATABASE_SOURCE_PATH_ITEM	"ProxyDatabaseSourcePath"
#define ADMINP_DATABASE_DESTINATION_PATH_ITEM	"ProxyDestinationDatabasePath"
#define ADMINP_DOC_DATABASE_NAME_ITEM			"ProxyDatabaseName"
#define ADMINP_DOC_COMMENTS_ITEM				"ProxyRequestComments"
#define ADMINP_DOC_DELETE_MAIL_FILE_ITEM		"ProxyDeleteMailfile"
#define ADMINP_DOC_DELETE_FILE_ITEM				"ProxyDeleteFile"
#define ADMINP_DOC_HOME_SERVER_ITEM				"ProxyHomeServer"
#define ADMINP_DOC_EXECUTION_TIME_ITEM			"ProxyExecutionTime"
#define ADMINP_DOC_MAIL_SERVER_ITEM				"ProxySourceServer"
#define ADMINP_DOC_DATABASE_ACL_ITEM			"ProxyDatabaseACL"
#define ADMINP_DOC_DATABASE_CLASS_ITEM			"ProxyDatabaseTemplateName"
#define ADMINP_DOC_RESOURCE_SITE_ITEM			"ProxyResourceSite"
#define ADMINP_DOC_RESOURCE_TYPE_ITEM			"ProxyResourceType"
#define ADMINP_DOC_RESOURCE_CAPACITY_ITEM		"ProxyResourceCapacity"
#define ADMINP_DOC_RESOURCE_DESCRIPTION_ITEM	"ProxyResourceDescription"
#define ADMINP_DOC_RESOURCE_DOMAIN_ITEM			"ProxyResourceDomain"
#define ADMINP_DOC_PASS_PASSWORD_ITEM			"ProxyPassword"
#define ADMINP_DOC_PASS_CHECK_PASSWORD_ITEM		"ProxyPasswordCheck"
#define ADMINP_DOC_PASS_GRACE_PERIOD_ITEM		"ProxyPasswordGracePeriod"
#define ADMINP_DOC_PASS_CHANGE_INTERVAL_ITEM	"ProxyPasswordChangeInterval"
#define ADMINP_DOC_PASS_QUALITY_IS_LENGTH_ITEM	"ProxyPasswordQualityIsLength"
#define ADMINP_DOC_PASS_QUALITY_ITEM			"ProxyPasswordQuality"
#define ADMINP_DOC_PASS_HISTORY_COUNT_ITEM		"ProxyPasswordHistoryCount"
#define ADMINP_DOC_PASS_HTTP_GRACE_PERIOD_ITEM	"ProxyHTTPPasswordGracePeriod"
#define ADMINP_DOC_PASS_HTTP_CHANGE_INTERVAL_ITEM "ProxyHTTPPasswordChangeInterval"
#define ADMINP_DOC_PASS_HTTP_OPTIONS_ITEM 		"ProxyHTTPPasswordOptions"
#define ADMINP_DOC_PASS_HTTP_QUALITY_ITEM 		"ProxyHTTPPasswordQuality"
#define ADMINP_DOC_PASS_HTTP_FUNCTION_ITEM		"ProxyHTTPPasswordFunction"
#define ADMINP_DOC_PASS_HTTP_SYNCH_ITEM			"ProxyHTTPPasswordNotesSync"
#define ADMINP_DOC_PASS_HTTP_QUALITY_IS_LENGTH_ITEM	"ProxyHTTPPasswordQualityIsLength"
#define ADMINP_DOC_APPROVAL_FLAG_ITEM			"ApprovalFlag"
#define ADMINP_DOC_IMMEDIATE_FLAG_ITEM			"ProxyImmediateFlag"
#define ADMINP_NEW_GROUP_NAME_ITEM 				"ProxyNewGroupName"
#define ADMINP_DOC_PERSONS_CERT_ITEM 			"ProxyUserCertificate"
#define ADMINP_NAME_IN_X509_CERT_ITEM			"ProxyNameInX509Certificate"
#define ADMINP_DOC_DISPLAY_DESIGN_ELEMENTS_ITEM	"ProxyDesignElementList"
#define ADMINP_DOC_PRIVATE_DESIGN_ELEMENTS_ITEM	"$ProxyPrivateDesignElements"
#define ADMINP_DOC_DELETED_OBJECT_ITEM			"$ProxyDeletedObject"
#define ADMINP_DOC_MOVE_MAIL_DELETE_ITEM		"$ProxyMoveMailDelete"
#define ADMINP_DOC_ACL_FLAG_ITEM				"ACLFlag"
#define ADMINP_DOC_APPROVED_ITEM				"$ProxyApproved"
#define ADMINP_NAME_IN_CRL_ISSUEDBY_ITEM		"ProxyNameInCRLIssuedBy"
#define ADMINP_DOC_CERTIFIERS_CRL_ITEM 			"ProxyCertifierCrl"
#define ADMINP_PUBLISH_REQUESTOR_NOTEID_ITEM 	"ProxyRequestorNoteId"
#define ADMINP_CERTIFICATE_TYPE_ITEM 			"ProxyCertificateType"
#define ADMINP_PUBLISH_ACTION_ITEM 				"ProxyPublishAction"
#define ADMINP_PA_NAME_ITEM 					"ProxyPAName"
#define ADMINP_DOC_FALLBACK_REQUEST_ITEM		"ProxyReplicaFallbackRequest"
#define	ADMINP_FORMAT_PREF_INCOMING_MAIL_ITEM	"ProxyFormatPrefIncomingMail"
#define	ADMINP_ENCRYPT_INCOMING_MAIL_ITEM		"ProxyEncryptIncomingMail"
#define	ADMINP_INTERNET_PASSWORD_DIGEST_ITEM	"ProxyInternetPasswordDigest"
#define	ADMINP_USERNAME_ITEM					"ProxyUserName"
#define	ADMINP_ACCOUNT_ITEM						"ProxyAccountName"
#define	ADMINP_POLICY_NAME_ITEM					"ProxyPolicyName"
#define	ADMINP_POLICY_DB_NAME_ITEM				"ProxyPolicyDBName"
#define	ADMINP_POLICY_DB_ATTACHMENT_ITEM		"ProxyPolicyDBAttachment"
#define	ADMINP_POLICY_RA_LIST_ITEM				"RegistrationAuthorities"
#define	ADMINP_POLICY_CA_LIST_ITEM				"CAAdministrators"
#define	ADMINP_POLICY_CCS_STAMP_ITEM			"ProxyPolicyCCSStamp"
#define	ADMINP_POLICY_SERVERNAME_ITEM			"ProxyPolicyServerName"
#define ADMINP_POLICY_LOCKID_ITEM				"ProxyPolicyLockId"
#define	ADMINP_POLICY_ICLNAME_ITEM				"ProxyPolicyICLName"
#define	ADMINP_PUBLISH_FLAGS_ITEM				"ProxyPublishFlags"
#define	ADMINP_PUBLISH_CERT_FIELD_ITEM			"ProxyPublishCertField"
#define ADMINP_DOC_CLUSTER_MATES				"ProxyClusterMates"
#define ADMINP_DOC_CLUSTER_MATES_PATH			"ProxyClusterMatesPath"
#define ADMINP_DOC_CLUSTER_DELETE_REP			"ProxyClusterDeleteRep"
#define	ADMINP_DOC_IMAP_DELEGATEES_ITEM			"ProxyIMAPDelegatees"
#define	ADMINP_DOC_IMAP_MAILFILES_ITEM			"ProxyIMAPDelegateesMailFiles"
#define ADMINP_DOC_SECNAB_PATH_ITEM				"ProxySecondaryDirectoryPath"
#define ADMINP_DOC_SECNAB_NAME_ITEM				"ProxySecondaryDirectoryName"
#define ADMINP_DOC_SECNAB_REPLICA_ID_ITEM		"ProxySecondaryDirectoryReplicaId"
#define ADMINP_DOC_RETRACT_NAME_UNID_ITEM		"ProxyRetractNameChangeUNID"
#define ADMINP_NEW_WEB_NAME_ITEM				"ProxyNewWebName"
#define ADMINP_DOC_NEW_WEB_FIRST_NAME_ITEM		"ProxyNewWebFirstName"
#define ADMINP_DOC_NEW_WEB_MI_ITEM				"ProxyNewWebMI"
#define ADMINP_DOC_NEW_WEB_LAST_NAME_ITEM		"ProxyNewWebLastName"
#define	ADMINP_NEW_HTTPPASSWORD_ITEM			"ProxyNewHTPPPassword"
#define	ADMINP_OLD_HTTPPASSWORD_ITEM			"ProxyOldHTPPPassword"
#define ADMINP_DOC_TASK_TYPE_LIST_ITEM			"ProxyTaskTypes"
#define ADMINP_DOC_TASK_SEVERITY_ITEM			"ProxyTaskSeverity"
#define ADMINP_DOC_STATS_TYPE_LIST_ITEM			"ProxyStatisticTypes"
#define ADMINP_DOC_SMON_SERVERS_ITEM			"ProxyMonitoredServer"
#define ADMINP_DOC_STATREP_SERVER_ITEM			"ProxyStaterepServer"
#define ADMINP_DOC_SMON_TIMEFRAME_START_ITEM	"ProxyMonitorTimeFrameStart"
#define ADMINP_DOC_SMON_TIMEFRAME_END_ITEM		"ProxyMonitorTimeFrameEnd"
#define ADMINP_DOC_SMON_PARENT_UNID_ITEM		"ProxyMonitorParentUNID"
#define ADMINP_DOC_SERVER_RESTARTS_ITEM	  		"ProxyServerRestarts"
#define ADMINP_DOC_IGNORE_STATISTICS_ITEM	  	"ProxyIgnoreTheseStatistics"
#define ADMINP_DOC_IGNORE_EVENTS_ITEM	  		"ProxyIgnoreTheseEvents"
#define ADMINP_DOC_DIRECTORY_SERVER_NAME_ITEM	"ProxyDirectoryServerName"
#define ADMINP_DOC_DIRECTORY_NAME_ITEM			"ProxyDirectoryName"
#define ADMINP_DOC_DIRECTORY_TITLE_ITEM			"ProxyDirectoryTitle"
#define ADMINP_DOC_DIRECTORY_REPLICA_ID_ITEM	"$ProxyDirectoryReplicaId"
#define ADMINP_DOC_DIRECTORY_NOTE_UNID_ITEM		"ProxyDirectoryNoteUNID"
#define ADMINP_DOC_MAIL_SERVER_DNS_ITEM			"ProxyMailServerDNS"
#define ADMINP_DOC_MAIL_DOMAIN_ITEM				"ProxyMailDomain"
#define ADMINP_DOC_IS_WEB_MAIL_USER_ITEM		"ProxyIsWebMailUser"
#define ADMINP_DOC_HOSTED_ORG_ITEM				"ProxyHostedOrg"
#define ADMINP_DOC_HOSTED_ORG_ACTION_ITEM		"ProxyHostedOrgAction"
#define ADMINP_DOC_HOSTED_ORG_STORAGE_ITEM		"ProxyHostedOrgStorage"
#define ADMINP_DOC_WEB_NAME_CHANGE_EXPIRE_ITEM	"ProxyWebNameChangeExpires"
#define ADMINP_DOC_SHORT_NAME_ITEM				"ProxyShortName"
#define ADMINP_DOC_INTERNET_ADDRESS_ITEM		"ProxyInternetAddress"
#define ADMINP_DOC_NEW_DOMAIN_ITEM				"ProxyNewDomain"
#define ADMINP_DOC_OLD_DOMAIN_ITEM				"ProxyOldDomain"
#define ADMINP_HTTP_PASSWORD_SYNC_DATE_ITEM		"ProxyHTTPSyncDate"
#define ADMINP_DOC_AGENT_TO_SIGN_ITEM			"ProxyAgentToSign"
#define ADMINP_NEW_RESOURCE_NAME_ITEM			"ProxyNewResourceName"
#define ADMINP_DOC_RETIRE_DB_ITEM				"ProxyRetireDb"
#define ADMINP_CASCADE_DESIGN_ELEMENTS_ITEM		"ProxyCascadeDesignElements"
#define ADMINP_DOC_CREATE_TRENDS_REQ			"ProxyCreateTrends"
#define ADMINP_MAJOR_VERSION_NUMBER				"MajVer"
#define ADMINP_MINOR_VERSION_NUMBER				"MinVer"
#define ADMINP_QMR_VERSION_NUMBER				"QMRVer"
#define ADMINP_QMU_VERSION_NUMBER				"QMUVer"
#define ADMINP_HOT_VERSION_NUMBER				"HotVer"
#define ADMINP_FIXP_VERSION_NUMBER				"FixPVer"
#define ADMINP_FLAGS_VERSION_NUMBER				"FlagsVer"
#define ADMINP_DOC_MEMBER_LIST_ITEM				"ProxyMemberList"
#define ADMINP_DOC_AGENT_ACTIVATABLE_ITEM		"ProxyAgentActivatable"
#define ADMINP_DOC_AGENT_ENABLE_ITEM			"ProxyAgentEnable"
#define ADMINP_RECOVERY_INFO					"RecoveryInfo"
#define ADMINP_RECOVERY_INFO_TIMESTAMP			"RecoveryInfoTimeStamp"
#define ADMINP_CLIENT_HASH						"ClntDgst"
#define ADMINP_CLIENT_RECORD_DYNCONFIG			"ClientInfoDynconfig"
#define ADMINP_PROXY_MAINTAIN_DBQUOTA			"ProxyMaintainDBQuota"
#define ADMINP_DBQUOTA_WARNING_ITEM				"ProxyDBQuotaWarning"
#define ADMINP_DBQUOTA_LIMIT_ITEM				"ProxyDBQuotaLimit"
#define ADMINP_DB2_DEFAULT_ITEM					"ProxyDB2Default"
#define ADMINP_DB2_RESTART_ITEM					"ProxyDB2RestartServer"
#define ADMINP_DB2_ACCESS_COMMAND				"ProxyDB2AccessCommand"
#define ADMINP_DB2_ACCESS_DESTSRV				"ProxyDB2AccessDestServer"
#define ADMINP_DB2_ACCESS_DESTDOM				"ProxyDB2AccessDestDomain"
#define ADMINP_DB2_ACCESS_SRCSRV				"ProxyDB2AccessSrcServer"
#define ADMINP_DB2_ACCESS_SRCDOM				"ProxyDB2AccessSrcDomain"
#define ADMINP_DB2_ACCESS_OPTNET				"ProxyDB2AccessOptNetAddress"
#define ADMINP_DB2_ACCESS_PORTNAM				"ProxyDB2AccessPortName"

/* fields in the delete hosted organization adminp request */
#define ADMINP_ORG_NAME_ITEM                    "Fullname"
#define ADMINP_ORG_DIR_ITEM                     "Pathname"

/* fields in the proxy database log form */
#define ADMINP_PROXY_LOG 						"AdminLog"
#define ADMINP_LOG_ACTION_REQUESTOR_ITEM 		"ProxyActionRequestor"
#define ADMINP_LOG_SERVER_NAME_ITEM 			"ProxyServerName"	
#define ADMINP_LOG_ACTION_START_TIME_ITEM 		"ActionStartTime"
#define ADMINP_LOG_ACTION_END_TIME_ITEM			"ActionEndTime"
#define ADMINP_LOG_DB_LIST_ITEM					"ProxyDbList"
#define ADMINP_LOG_STATUS_ITEM					"ProxyStatus"
#define ADMINP_LOG_STATUS_CODES_ITEM			"ProxyStatusCodes"
#define ADMINP_LOG_MODIFIED_RESPONSE_ITEM		"ModifiedFlag"
#define ADMINP_LOG_SHOW_MODIFIED_ITEM			"ShowModified"
#define ADMINP_LOG_OLD_NAME_ITEM				"OldName"
#define ADMINP_LOG_NEW_NAME_ITEM				"NewName"
#define ADMINP_LOG_ACTION_COMMENTS_ITEM			"ActionComments"
#define ADMINP_LOG_SHOW_ERROR					"ErrorFlag"
#define ADMINP_LOG_REPLICA_CREATION_TIME		"ReplicaCreationTime"
#define ADMINP_LOG_CREATED_DELETE_REQUEST		"DeleteRequestCreated"
#define ADMINP_LOG_SIGNED_NOTES					"ProxySignedDocs"
#define ADMINP_LOG_IN_PROGRESS					"AdminPInProgress"
#define ADMINP_LOG_SENT_TO_DOMAINS				"ProxySentToDomains"
#define ADMINP_LOG_ADD_IN_NAME					"ProxyAddInProcess"
#define ADMINP_LOG_ADD_IN_SIGNER				"ProxyAddInSigner"
#define ADMINP_LOG_PRIVATE_AGENTS				"ProxyPrivateAgents"
#define ADMINP_LOG_PRIVATE_FOLDERS				"ProxyPrivateFolders"
#define ADMINP_LOG_PRIVATE_VIEWS				"ProxyPrivateViews"
#define ADMINP_LOG_SHARED_AGENTS				"ProxySharedAgents"
#define ADMINP_LOG_NAME_EXPIRATION_ITEM			"ProxyNameExpiration"
#define ADMINP_LOG_ERROR_DBS					"$AdminpErrorDbs"
#define ADMINP_LOG_NO_ERROR_DBS					"$AdminpNoErrorDbs"
#define ADMINP_LOG_DESIGN_DBS					"$AdminpDesignDbs"
#define ADMINP_LOG_DESIGN_DB_LINKS				"$AdminpDesignDbLinks"
#define ADMINP_LOG_AGENT_REPLY_TO				"$AgentReplyTo"
#define ADMINP_LOG_RETRY_TIME_ITEM				"ProxyRetryTime"
#define ADMINP_LOG_ORG_LIST						"ProxyOrgList"
#define ADMINP_LOG_FOUND_IN_ECL					"ProxyFoundECLDocs"
#define ADMINP_LOG_SHARED_FOLDERS				"ProxySharedFolders"
#define ADMINP_LOG_SHARED_VIEWS					"ProxySharedViews"
#define ADMINP_LOG_SHARED_AGENTS_MODIFIED		"ProxySharedAgentsModified"
#define ADMINP_LOG_SHARED_FORMS					"ProxySharedForms"
#define ADMINP_LOG_UNSCHEDULED_AGENTS			"ProxyUnscheduledAgents"
#define ADMINP_LOG_AGENTS_WITH_READERS			"ProxyAgentsWithReaders"
#define ADMINP_LOG_DIRECTORY_ERROR_ITEM			"ProxyDirectoryError"
#define ADMINP_LOG_DIRECTORY_NOERROR_ITEM		"ProxyDirectoryNoError"
#define ADMINP_LOG_DIR_ERROR_DISP_ITEM			"ProxyDirectoryErrorDisp"
#define ADMINP_LOG_FOUND_IN_DIRECTORY			"ProxyFoundNABDocs"
#define ADMINP_LOG_LAST_ENTRY_REMOVED_ITEM		"ProxyLastEntryRemoved"
#define ADMINP_LOG_EXPIRED_NAME_CHANGES			"ProxyExpiredNameChanges"

/* fields in the proxy database profiles */

#define ADMINP_PROFILE_TO_DOMAINS				"ToDomains"
#define ADMINP_PROFILE_FROM_DOMAINS				"FromDomains"
#define ADMINP_PROFILE_INBOUND_REP_SERVERS		"InboundReplicaServers"
#define ADMINP_PROFILE_OUTBOUND_REP_SERVERS		"OutboundReplicaServers"
#define ADMINP_PROFILE_OUTBOUND_REP_DOMAINS		"OutboundReplicaDomains"
#define ADMINP_INBOUND_APPROVED_SIGNERS			"InboundApprovedSigners"
#define ADMINP_OUTBOUND_APPROVED_SIGNERS		"OutboundApprovedSigners"

/* fields in the new user registration requests */
/* REG_USER_INFO fields */
#define ADMINP_NEW_USER_ORG_UNIT_ITEM			"ProxyNewUserOrgUnit"
#define ADMINP_NEW_USER_ALTORG_UNIT_ITEM		"ProxyNewUserAltOrgUnit"
#define ADMINP_NEW_USER_ALTNAME_ITEM			"ProxyNewUserAltName"
#define ADMINP_NEW_USER_ALTLANGUAGE_ITEM		"ProxyNewUserAltLanguageName"
#define ADMINP_NEW_USER_FIRST_NAME_ITEM			"ProxyNewUserFirstName"
#define ADMINP_NEW_USER_MIDDLE_NAME_ITEM		"ProxyNewUserMiddleName"
#define ADMINP_NEW_USER_LAST_NAME_ITEM			"ProxyNewUserLastName"
#define	ADMINP_NEW_USER_PASSWORD_ITEM			"ProxyNewUserPassword"
#define ADMINP_NEW_USER_PASSWORD_QUALITY_ITEM 	"ProxyNewUserPasswordQuality"
#define ADMINP_NEW_USER_SHORT_NAME_ITEM			"ProxyNewUserShortName"
#define ADMINP_NEW_USER_IADDRESS_ITEM			"ProxyNewUserInternetAddress"
#define ADMINP_NEW_USER_PREFLANGUAGE_ITEM		"ProxyNewUserPrefLanguage"

/* REG_MAIL_INFO fields */
#define ADMINP_NEW_USER_MAIL_SYSTEM_ITEM 		"ProxyNewUserMailSystem"
#define ADMINP_NEW_USER_MAIL_OWNER_ACCESS_ITEM	"ProxyNewUserMailOwnerAccess"
#define ADMINP_NEW_USER_DBQUOTA_LIMIT_ITEM		"ProxyNewUserDbQuotaSizeLimit"
#define ADMINP_NEW_USER_DBQUOTA_WARNING_ITEM 	"ProxyNewUserDbQuotaWarningThreshold"
#define ADMINP_NEW_USER_MAIL_SERVER_ITEM 		"ProxyNewUserMailServer"
#define ADMINP_NEW_USER_MAIL_FILE_ITEM 			"ProxyNewUserMailFile"
#define ADMINP_NEW_USER_MAIL_TEMPLATE_ITEM 		"ProxyNewUserMailTemplate"
#define ADMINP_NEW_USER_MAIL_FORWARD_ITEM 		"ProxyNewUserMailForwardAddress"
#define ADMINP_NEW_USER_MAIL_MANAGER_ITEM 		"ProxyNewUserMailManager"
#define ADMINP_NEW_USER_MAIL_REP_SERVERS_ITEM	"ProxyNewUserMailReplicaServers"

/* REG_ROAMING_INFO fields */
#define ADMINP_NEW_USER_ROAMING_SERVER_ITEM			"ProxyNewUserRoamingServer"
#define ADMINP_NEW_USER_ROAMING_SUBDIR_ITEM			"ProxyNewUserRoamingSubDir"
#define ADMINP_NEW_USER_ROAMN_DB_ITEM				"ProxyNewUserRoamingDB"
#define ADMINP_NEW_USER_ROAMN_MODE_ITEM 			"ProxyNewUserRoamingMode"
#define ADMINP_NEW_USER_ROAMN_CLEANUP_MODE_ITEM		"ProxyNewUserRoamingCleanupMode"
#define ADMINP_NEW_USER_ROAMN_CLEANUP_PERIOD_ITEM	"ProxyNewUserRoamingCleanupPeriod"
#define ADMINP_NEW_USER_ROAMN_REP_SERVERS_ITEM		"ProxyNewUserRoamingReplicaServers"

#define ADMINP_NEW_USER_IDTYPE_ITEM			"ProxyNewUserIDType"
#define ADMINP_NEW_USER_IDEXPIRATION_ITEM	"ProxyNewUserIDExpiration"
#define ADMINP_NEW_USER_GROUPS_ITEM			"ProxyNewUserGroups"
#define ADMINP_NEW_USER_ID_ITEM				"ProxyNewUserIDFile"
#define ADMINP_NEW_USER_LOCATION_ITEM		"ProxyNewUserLocation"
#define ADMINP_NEW_USER_COMMENT_ITEM		"ProxyNewUserComment"
#define ADMINP_NEW_USER_PROFILE_ITEM		"ProxyNewUserProfile"
#define ADMINP_NEW_USER_LOCAL_ADMIN_ITEM	"ProxyNewUserLocalAdmin"
#define ADMINP_NEW_USER_FLAGS_ITEM			"ProxyNewUserFlags"
#define ADMINP_NEW_USER_EXTFLAGS_ITEM		"ProxyNewUserExtFlags"

/* server keyring update request */
#define ADMINP_PRIVATE_KEY_ITEM				"ProxyPrivateKey"
#define ADMINP_KEYRING_PASSWORD_ITEM		"ProxyKeyringPassword"
#define ADMINP_KEYRING_FILE_ITEM			"ProxyKeyringFile"
#define ADMINP_SUBJECT_ITEM					"ProxySubjectName"
#define ADMINP_ISSUER_ITEM					"ProxyIssuerName"

/* fields in the proxy database log form */
#define ADMINP_PROXY_NAMEINDIR_REPORT			"AdminNameInDirRpt"
#define ADMINP_PROXY_NAMEINACL_REPORT			"AdminNameInAclRpt"
#define ADMINP_PROXY_NAMEINAGENT_REPORT			"AdminNameInAgentRpt"
#define ADMINP_PROXY_NAMEINFOLDER_REPORT		"AdminNameInFolderRpt"
#define ADMINP_PROXY_NAMEINNAMEFLD_REPORT		"AdminNameInNamesFldRpt"
#define ADMINP_PROXY_NAMEINXACL_REPORT			"AdminNameInxAclRpt"
#define ADMINP_PROXY_NAMEINECL_REPORT			"AdminNameInEclRpt"
#define ADMINP_PROXY_NAMEINPOLICY_REPORT		"AdminNameInPolicyRpt"

#define ADMINP_MAIL_FILE_TITLE_ITEM ADMINP_DOC_DATABASE_NAME_ITEM

/* generic admin4.nsf fields */
#define ADMINP_PROXY_TEXT_ITEM_1				"ProxyTextItem1"
#define ADMINP_PROXY_TEXT_ITEM_2				"ProxyTextItem2"
#define ADMINP_PROXY_TEXT_ITEM_3				"ProxyTextItem3"
#define ADMINP_PROXY_TEXT_ITEM_4				"ProxyTextItem4"
#define ADMINP_PROXY_TEXT_ITEM_5				"ProxyTextItem5"
#define ADMINP_PROXY_TEXT_ITEM_6				"ProxyTextItem6"
#define ADMINP_PROXY_TEXT_ITEM_7				"ProxyTextItem7"
#define ADMINP_PROXY_TEXT_ITEM_8				"ProxyTextItem8"
#define ADMINP_PROXY_TEXT_ITEM_9				"ProxyTextItem9"
#define ADMINP_PROXY_TEXT_ITEM_10				"ProxyTextItem10"

#define ADMINP_PROXY_DATE_ITEM_1				"ProxyDateItem1"
#define ADMINP_PROXY_DATE_ITEM_2				"ProxyDateItem2"
#define ADMINP_PROXY_DATE_ITEM_3				"ProxyDateItem3"

#define ADMINP_PROXY_NUM_ITEM_1					"ProxyNumItem1"
#define ADMINP_PROXY_NUM_ITEM_2					"ProxyNumItem2"

#define ADMINP_PROXY_CREATE_FT_INDEX			"ProxyCreateFullTextIndex"
#define ADMINP_PROXY_PACKED_FTI_OPTIONS			"ProxyPackedFullTextIndexOptions"
#define ADMINP_PROXY_COPY_ACL					"ProxyCopyACL"
#define ADMINP_PROXY_PACKED_ACL					"ProxyPackedACL"
#define ADMINP_PROXY_PACKED_ACL_HISTORY			"ProxyPackedACLHistory"
#define ADMINP_PROXY_PACKED_ACL_HISTORY_COUNT	"ProxyPackedACLHistoryCount"
#define ADMINP_PROXY_LINK_DEST_TO_SCOS			"ProxyLinkDestinationToSCOS"
#define ADMINP_PROXY_MAILFILE_ACCESS			"ProxyMailfileAccessLevel"
#define ADMINP_PROXY_MAILREPLICA_SERVERS		"ProxyMailReplicaServers"
#define ADMINP_PROXY_ROAMNREPLICA_SERVERS		"ProxyRoamingReplicaServers"
#define ADMINP_PROXY_OVERRIDE_DEF_DATASTORE		"ProxyOverrideDefaultDatastore"
#define ADMINP_PROXY_DB2_PASSWORD_ITEM			"ProxyDB2Password"
#define ADMINP_PROXY_ALTERNATE_NAME_ITEM		"ProxyAlternateName"
#define ADMINP_PROXY_ALTERNATE_NAME_TAG_ITEM	"ProxyAlternateNameTag"

/* name and address book fields and field values adminp needs */ 
#define ADMINP_NAB_SERVER_BUILD_NUMBER 			"ServerBuildNumber"
#define ADMINP_NAB_DENY_ACCESS_GROUP 			"3"

#define ADMINP_NAB_PASS_CHECK_PASSWORD			"CheckPassword"
#define ADMINP_NAB_PASS_GRACE_PERIOD			"PasswordGracePeriod"
#define ADMINP_NAB_PASS_CHANGE_INTERVAL			"PasswordChangeInterval"
#define ADMINP_NAB_PASS_QUALITY					"PasswordQuality"
#define ADMINP_NAB_PASS_QUALITY_IS_LENGTH		"PwdQltyIsLen"
#define ADMINP_NAB_PASS_HISTORY_COUNT			"PwdHistCnt"
#define ADMINP_NAB_PASS_WEB_GRACE_PERIOD		"HTTPPasswordGracePeriod"
#define ADMINP_NAB_PASS_WEB_CHANGE_INTERVAL		"HTTPPasswordChangeInterval"
#define ADMINP_NAB_PASS_WEB_OPTIONS				"HTTPPasswordOptions"
#define ADMINP_NAB_PASS_WEB_QUALITY				"HTTPPasswordQuality"
#define ADMINP_NAB_PASS_WEB_SYNCH				"HTTPPasswordNotesSync"
#define ADMINP_NAB_PASS_WEB_QUALITY_IS_LENGTH	"HTTPPasswordQualityIsLength"
#define ADMINP_NAB_PASS_WEB_FORCE_CHANGE		"HTTPPasswordForceChange"

#define ADMINP_NAB_MASTER_ADDRESS_BOOK			"MasterAddressBook"

#define ADMINP_SERVER_CONNECTION_FORM			"Server\\Connection"
#define ADMINP_NETWORK_CONNECTION_FORM			"Network Connection"
#define ADMINP_REMOTE_CONNECTION_X25_FORM		"Remote Connection (X25)"
#define ADMINP_REMOTE_CONNECTION_ISDN_FORM		"Remote Connection (ISDN)"

/* address book stuff for resources used by adminp */
#define ADMINP_NAB_RESOURCE_FLAG_ITEM			"ResourceFlag"
#define ADMINP_NAB_DOCUMENT_ACCESS_ITEM			"DocumentAccess"
#define ADMINP_NAB_RESOURCE_CAPACITY_ITEM		"ResourceCapacity"
#define ADMINP_NAB_RESOURCE_TYPE_ITEM			"ResourceType"
#define ADMINP_NAB_RESOURCE_TYPE_ROOM			"1"
#define ADMINP_NAB_RESOURCE_TYPE_RESOURCE		"2"
#define ADMINP_NAB_RESOURCE_DOMAIN_ITEM			"MailDomain"
#define ADMINP_NAB_RESOURCE_CONFERENCE_DB_ITEM	"ConfDB"
#define ADMINP_NAB_RESOURCE_AFLAG_ITEM			"AudioFlg"
#define ADMINP_NAB_RESOURCE_VFLAG_ITEM			"VidFlg"
#define ADMINP_NAB_RESOURCE_AVFLAG_ITEM			"AVFlg"
#define ADMINP_NAB_RESOURCE_AVSELLIST_ITEM		"AVSlctLst"

/* address book stuff used for name change retraction */
#define ADMINP_NAB_OLD_MAIL_CERTIFICATE_ITEM			"AdminpOldCertificate"
#define ADMINP_NAB_OLD_MAIL_FIRSTNAME_ITEM				"AdminpOldFirstName"
#define ADMINP_NAB_OLD_MAIL_LASTNAME_ITEM				"AdminpOldLastName"
#define ADMINP_NAB_OLD_MAIL_MIDDLEINITIAL_ITEM			"AdminpOldMI"
#define ADMINP_NAB_OLD_MAIL_FULLNAME_ITEM				"AdminpOldFullName"
#define ADMINP_NAB_OLD_MAIL_OWNER_ITEM					"AdminpOldOwner"
#define ADMINP_NAB_OLD_MAIL_ALTFULLNAME_ITEM			"AdminpOldAltFullName"
#define ADMINP_NAB_OLD_MAIL_ALTFULLNAMELANGUAGE_ITEM	"AdminpOldAltFullNameLanguage"
#define ADMINP_NAB_OLD_MAIL_INTERNETADDRESS_ITEM 		"AdminpOldInternetAddress"	
#define ADMINP_NAB_OLD_MAIL_SHORTNAME_ITEM 				"AdminpOldShortName"

#define ADMINP_NAB_OLD_WEB_NAME_ITEM					"$AdminpOldWebName"
#define ADMINP_NAB_OLD_WEB_NAME_EXPIRES_ITEM			"$AdminpOldWebNameExpires"

#define ADMINP_EVENT_TYPE_ITEM							"Type"

#define TARGETSERVERSNAMESPACE 							"$TargetServers"
#define MESSAGESBYTYPENAMESPACE 						"$MessagesByType"

#define EVENT_TARGET_SERVER_NAME_ITEM					"TargetServerName"
#define EVENT_COLLECTING_SERVER_ITEM					"SourceServerName"
#define EVENT_REMOTE_DESTINATION_DB_ITEM				"RemoteDestinationDb"
#define EVENT_ORIGINAL_TEXT_ITEM						"OriginalText"
#define EVENT_SEVERITY_ITEM								"Severity"
#define EVENT_MONITOR_NUMBER_ITEM						"MonitorNumber"
#define EVENT_CODE_ITEM									"Code"
#define EVENT_TEXT_MATCH_ITEM							"EventTextMatch"

#define EVENT_UNSPECIFIED_SERVERS						"2"
#define EVENT_ALL_SERVERS								"1"

#define RESOURCES_NAMESPACE 		"$Resources"
#define RESOURCES_VIEW		 		"($Resources)"
#define ROOMS_NAMESPACE				"$Rooms"
#define ROOMS_VIEW					"($Rooms)"
#define ROOMS_PICKLIST_COLUMN	  	1
#define RESOURCES_PICKLIST_COLUMN 	2

/* SPR DDEY4ZSN4K - Add Online Meeting addressing support */
#define ONLINE_MEETINGS_NAMESPACE	"$OnlineMeetingPlaces"
#define ONLINE_MEETINGS_VIEW		"($OnlineMeetingPlaces)"
#define ONLINE_MEETINGS_PICKLIST_COLUMN	  	1

#define ADMINP_FLAG_SET				"1"
#define ADMINP_DOCUMENT_ACCESS_SET  "[NetModifier]"
#define ADMINP_ROOM					"1" 
#define ADMINP_RESOURCE				"2" 
#define ADMINP_ONLINE_RESOURCE		'3'

#define USER_MODIFIER				 "[UserModifier]"
#define POLICY_CREATOR				 "[PolicyCreator]"
#define POLICY_MODIFIER				 "[PolicyModifier]"
#define GROUP_MODIFIER 				 "[GroupModifier]"
#define GROUP_CREATOR				 "[GroupCreator]"


/* address book stuff for move mail file */
#define ADMINP_NEW_MAILFILE_ITEM			"NewMailFile"
#define ADMINP_NEW_MAILSERVER_ITEM			"NewMailServer"
#define ADMINP_OLD_MAILFILE_ITEM			"OldMailFile"
#define ADMINP_OLD_MAILSERVER_ITEM			"OldMailServer"
#define ADMINP_NEW_MAIL_CLIENT_UPDATE_ITEM	"NewMailClientUpdateFlag"
/* address book stuff for roaming user delete, move, and status change */
#define ADMINP_ROAMING_STATUS_UPDATE_ITEM	"RoamingStatusUpdateItem"
#define ADMINP_NEW_ROAMINGSTATUS_UPDATE_ITEM	"NewRoamStatUpdtFl"
#define ADMINP_NEW_ROAMINGSTATUS_DOWNGRD_ITEM	"RoamStatDwnFl"
#define ADMINP_OLD_ROAMINGSERVER_ITEM		"OldRoamSrvr"
#define ADMINP_OLD_ROAMINGDIRECTORY_ITEM	"OldRoamDir"
#define ADMINP_ROAM_UPGRADE_CASCADE_INFO	"RoamUpgdCscdInfo"		/* Item name for Request cascade information list */
#define ADMINP_ROAM_MOVE_CASCADE_INFO		"RoamMoveCscdInfo"		/* Item name for Request cascade information list */
#define ADMINP_MAIL_MOVE_CASCADE_INFO		"MailMoveCscdInfo"		/* Item name for Request cascade information list */

/* Original Request information for Blocking cascaded requests */
#define	ADMINP_ORIGINATING_REQUEST_UNID				"ProxyOriginatingRequestUNID"
#define	ADMINP_ORIGINATING_UNID						ADMINP_ORIGINATING_REQUEST_UNID
#define ADMINP_ORIGINATING_REQUEST_AUTHORID			"ProxyOriginatingAuthor"
#define ADMINP_SAVED_ORIGINATING_REQUEST_AUTHORID	ADMINP_ORIGINATING_REQUEST_AUTHORID
#define ADMINP_ORIGINATING_REQUEST_FULLNAME			"FullName"
#define ADMINP_ORIGINATING_REQUEST_ORG				"ProxyOriginatingOrganization"
#define ADMINP_ORIGINATING_REQUEST_INTERNET_DOMAIN	"ProxyOriginatingInternetDomain"
#define ADMINP_ORIGINATING_REQUEST_EXPECTED_LIST	"ProxyOriginatingReqsExpected"
#define ADMINP_ORIGINATING_REQUEST_POSSIBLE_LIST	"ProxyOriginatingReqsPossible"
#define ADMINP_ORIGINATING_TIMEDATE_ITEM			"ProxyOriginatingTimeDate"

#define ADMINP_SERV_MON_QUERY						"ServMonQuery"
#define ADMINP_SERV_MON_REPORT						"ServMonReport"
#define ADMINP_SERV_MON_SERV_LIST_ITEM				"Server.List"
#define ADMINP_SERV_MON_RESTRICTED_SERV_LIST_ITEM	"Server.Restricted.List"

#define ADMINP_SERV_MON_STAT_RESULTS				"ServMonStats"
#define ADMINP_SERV_MON_STAT_CATEGORY_ITEM			"StatisticCategory"
#define ADMINP_SERV_MON_STAT_NAME_ITEM				"Statistic.Name"
#define ADMINP_SERV_MON_STAT_MIN_ITEM				"Statistic.Min.Value"
#define ADMINP_SERV_MON_STAT_MIN_TD_ITEM			"Statistic.Min.Time"
#define ADMINP_SERV_MON_STAT_MAX_ITEM				"Statistic.Max.Value"
#define ADMINP_SERV_MON_STAT_MAX_TD_ITEM			"Statistic.Max.Time"
#define ADMINP_SERV_MON_STAT_ZEROVAL_ITEM			"Statistic.Zero.Value"
#define ADMINP_SERV_MON_STAT_ZEROVAL_TD_ITEM		"Statistic.Zero.Time"
#define ADMINP_SERV_MON_STAT_MINDELTA_ITEM			"Statistic.Min.Difference"
#define ADMINP_SERV_MON_STAT_MINDELTA_TERM1_ITEM	"Statistic.Min.Diff.Term1"
#define ADMINP_SERV_MON_STAT_MINDELTA_TERM2_ITEM	"Statistic.Min.Diff.Term2"
#define ADMINP_SERV_MON_STAT_MINDELTA_LOW_TD_ITEM	"Statistic.Min.Difference.Low"
#define ADMINP_SERV_MON_STAT_MINDELTA_UPP_TD_ITEM	"Statistic.Min.Difference.High"
#define ADMINP_SERV_MON_STAT_MAXDELTA_ITEM			"Statistic.Max.Difference"
#define ADMINP_SERV_MON_STAT_MAXDELTA_TERM1_ITEM	"Statistic.Max.Diff.Term1"
#define ADMINP_SERV_MON_STAT_MAXDELTA_TERM2_ITEM	"Statistic.Max.Diff.Term2"
#define ADMINP_SERV_MON_STAT_MAXDELTA_LOW_TD_ITEM	"Statistic.Max.Difference.Low"
#define ADMINP_SERV_MON_STAT_MAXDELTA_UPP_TD_ITEM	"Statistic.Max.Difference.High"
#define ADMINP_SERV_MON_STAT_ZERODELTA_ITEM			"Statistic.Zero.Difference"
#define ADMINP_SERV_MON_STAT_ZERODELTA_LOW_TD_ITEM	"Statistic.Zero.Difference.Low"
#define ADMINP_SERV_MON_STAT_ZERODELTA_UPP_TD_ITEM	"Statistic.Zero.Difference.High"

#define ADMINP_SERV_MON_EVENT_RESULTS				"ServMonResults"
#define ADMINP_SERV_MON_EVENT_STATUS_ITEM			"Event.Name"
#define ADMINP_SERV_MON_EVENT_TYPE_ITEM				"Event.Type"
#define ADMINP_SERV_MON_EVENT_ADDIN_ITEM			"Event.Addin"
#define ADMINP_SERV_MON_EVENT_SEVERITY_ITEM			"Event.Severity"
#define ADMINP_SERV_MON_EVENT_COUNT_ITEM			"Event.Count"
#define ADMINP_SERV_MON_EVENT_ERROR_MSG_ITEM		"Event.Error.Message"

#define	ADMINP_CATALOG_NO_NAMES			"0"
#define ADMINP_CATALOG_READERS_AUTHORS 	"1"
#define ADMINP_CATALOG_NAMES			"2"

/*	Address Book - local $Programs namespace */
#define LOCAL_PROGRAMS_NAMESPACE "1\\$Programs"
#define PROGRAMSNAMESPACE "$Programs"
#define	PROG_CMD_LINE_ITEM	"CmdLine"

/*	Address Book - local $Locations namespace */
#define LOCAL_LOCATIONS_NAMESPACE "1\\$Locations"
#define LOCATIONSNAMESPACE "$Locations"

/*	Address Book - local $Connections namespace */
#define LOCAL_CONNECTIONS_NAMESPACE "1\\$Connections"
#define CONNECTIONSNAMESPACE "$Connections"


/*	Address Book - "Connections" namespace */


#define SOURCE_WILDCARD "*"

#define NAME_CONNECTIONLOOKUPITEMCOUNT	9
#define NAME_CONNECTIONLOOKUPITEMS "Source\0SourceDomain\0Destination\0DestinationDomain\0PortName\0Tasks\0Enabled\0Cost\0ConnectionType"
#define NAME_CONNECTIONSRCITEM	0
#define NAME_CONNECTIONSRCDOMAINITEM 1
#define NAME_CONNECTIONDSTITEM	2
#define NAME_CONNECTIONDSTDOMAINITEM 3
#define NAME_CONNECTIONPORTITEM	4
#define NAME_CONNECTIONTASKSITEM 5
#define NAME_CONNECTIONENABLEDITEM 6
#define NAME_CONNECTIONCOSTITEM 7
#define NAME_CONNECTIONTYPEITEM 8

#define DIALNAMELOOKUPITEMCOUNT	5
#define DIALNAMELOOKUPITEMS "Destination\0PhoneNumber\0PortName\0LinkInfo\0LinkScriptInfo\0CreditCardPrefix\0CreditCardSuffix"
#define DESTINATIONNAMEITEM		0
#define PHONENUMBERITEM			1
#define	PORTNAMEITEM			2
#define	LINKINFOITEM			3
#define	LINKSCRIPTITEM			4
#define CARDPREFIXITEM			5
#define CARDSUFFIXITEM			6

#define PT_NAMELOOKUPITEMCOUNT	3	/*	For Passthru */
#define PT_NAMELOOKUPITEMS "Destination\0PassthruServer\0Source"
#define PT_NAMEITEM_DST			0
#define PT_NAMEITEM_VIA			1
#define PT_NAMEITEM_SRC			2

/*	Address Book - "Connections" Application field values */

#define ROUTER_APPNAME	"Mail Routing"		/* Router application name */
#define PULL_ROUTER_APPNAME	"Pull Routing"	/* Pull Routing application name */
#define X400_APPNAME	"X400 Mail Routing"	/* X.400 MTA application name */
#define CCMAIL_APPNAME	"ccMail Routing"	/* ccMail MTA application name */
#define SMTP_APPNAME	"SMTP Mail Routing"	/* SMTP MTA application name */
#define REPL_APPNAME 	"Replication"		/* Replicator application name */
#define RUNPGM_APPLNAME	"Run Program"		/* Run program application name */
#define WKSREP_APPNAME	"WksRep"			/* Workstation Rep/Briefcase application name */
#define WKSREPHI_APPNAME "WksRepHI"			/* High-priority Rep/Briefcase application name */
#define CLREPL_APPNAME	"Cluster Replication" /* Cluster Replicator application name */
#define NNTP_APPNAME	"NNTP Feed"			/* NNTP Feed application name */
#define DIRCAT_APPNAME	"Directory Cataloger"	/* DIrectory Catalog Aggregator */
#define DOMAIN_INDEXER_APPNAME	"Domain Indexer"	/* Domain Indexer */
#define AUTODIALER_APPNAME	"AutoDialer"	/* AutoDialer */
#define AMGR_APPNAME	"Run Agent"			/* Agent manager name */
#define EVENT_APPNAME	"Event Monitor"
#define RUNJAVA_APPNAME	"runjava"			/* shell to run java class as an add in */
#define ISPY_APPNAME	"ISpy"				/* a runjava task */
#define RMEVAL_APPNAME	"RMEval"			/* a runjava task */
#define COMPACT_APPNAME	"Compact"			/* Database compactor */
#define DESIGN_APPNAME	"Designer"			/* Database design */
#define COLLECTOR_APPNAME	"Statistic Collector"	/* Statistic Collector */

/* Connection record items not defined above */

#define CONNREC_USAGE_PRIORITY_ITEM "ConnectionRecordFirst" /* "Normal" use connection record first */
#define CONNREC_LOCATION			"ConnectionLocation" /* only for locations */
#define CONNREC_OPTIONAL_NET_ADDRESS_ITEM "OptionalNetworkAddress" /* Optional network address field */
#define CONNREC_REMOTE_DTE_ADDRESS_ITEM "RemoteDTEAddress" /* DTE address for X.25 */
#define CONNREC_FCODE1_ITEM "fcode_1" /* X.25 facility code */
#define CONNREC_FCODE2_ITEM "fcode_2" /* X.25 facility code */
#define CONNREC_FCODE3_ITEM "fcode_3" /* X.25 facility code */
#define CONNREC_FCODE4_ITEM "fcode_4" /* X.25 facility code */
#define CONNREC_FVAL1_ITEM "fval_1" /* X.25 facility code value */
#define CONNREC_FVAL2_ITEM "fval_2" /* X.25 facility code value */
#define CONNREC_FVAL3_ITEM "fval_3" /* X.25 facility code value */
#define CONNREC_FVAL4_ITEM "fval_4" /* X.25 facility code value */
#define CONNREC_SCRIPTNAME_ITEM "ScriptName" /* Login script filename */
#define CONNREC_LINKSCRIPT_INFO_ITEM "LinkScriptInfo" /* Login script information */
#define CONNREC_SCRIPTARG1_ITEM "ScriptArg1" /* Login script argument */
#define CONNREC_SCRIPTARG2_ITEM "ScriptArg2" /* Login script argument */
#define CONNREC_SCRIPTARG3_ITEM "ScriptArg3" /* Login script argument */
#define CONNREC_SCRIPTARG4_ITEM "ScriptArg4" /* Login script argument */
#define CONNREC_WEEKDAYS_ITEM "WeekDays" /* Call schedule WeekDays item */
#define CONNREC_REP_PRIORITY_ITEM "RepPriority" /* Replication priority item */
#define CONNREC_REP_TYPE_ITEM "RepType" /* Replication type item */
#define CONNREC_REP_FILELIST_ITEM "Filenames" /* Replication filename list item */
#define CONNREC_REP_EXCLUDE_FILELIST_ITEM	"ExclFNames"	/* Excluse the list in "Filenames" item */
#define CONNREC_REP_TIME_LIMIT_ITEM "TimeLimit" /* Replication time limit item */
#define CONNREC_NNTP_TYPE_ITEM "NNTPFeedType" 			/* NNTP Feed type item */
#define CONNREC_NNTP_AUTH_ITEM "NNTPAuthentication" 	/* NNTP Authentication item */
#define CONNREC_NNTP_ENCRYPT_ITEM "NNTPChannelEncrypt" 	/* NNTP channel encrypt item */
#define CONNREC_NNTP_CREATE_DBS_ITEM "NNTPCreateUsenetDBs" /* NNTP create db's item */
#define CONNREC_NNTP_NEWSGROUP_SUBDIR_ITEM "NNTPNewsgroupSubDir" /* NNTP newsgroup subdirectory item */
#define CONNREC_NNTP_NEWSGROUPS_ITEM "NNTPNewsgroups" 	/* NNTP newsgroups item */
#define CONNREC_NNTP_SITES_ITEM "NNTPSites" 			/* NNTP sites item */
#define CONNREC_NNTP_TIMELIMIT_ITEM "NNTPTimeLimit" 	/* NNTP Feed time limit item */
#define CONNREC_NNTP_USER_ITEM "NNTPUserName"			/* NNTP user name for feed */
#define CONNREC_NNTP_PASSWD_ITEM "NNTPPassword"			/* NNTP password for feed */
#define CONNREC_DIALUP_SERVER_NAME_ITEM "DialViaName"	/* Dial Up Server name item */
#define CONNREC_CONNECTION_MODE_ITEM	"ConnectionMode"  /* Direct or Dial-Up connection mode */
#define CONNREC_ROUTING_TYPE_ITEM		"RouterType"
#define CONNREC_ROUTING_SMTPPULL_ITEM	"SMTPPullRouting"	/* will be enabled if SMTP Push/Pull or Pull Only */
#define CONNREC_PULL_TIMEOUT_ITEM		"PullRoutingTimeout"	/* Time to wait for an acknowledgement of the pull request */
#define CONNREC_PULL_THIS_SERVER		"PullThisServer"	/* Include this server in pull request */
#define CONNREC_PULL_ALL_PRIMARY 		"PullAllPrimary"	/* Include all primary internet domains in pull request */
#define CONNREC_PULL_ALL_ALTERNATE 		"PullAllAlternates"	/* Include all alternate internet domains in pull request */
#define CONNREC_PULL_CHOOSE_SPECIFIC	"PullChooseSpecific"	/* Include list specified in PullSpecificList in pull request */
#define CONNREC_PULL_SPECIFIC_LIST		"PullSpecificList"	/* Include list of domains in pull request */


/* Connection record Mail Routing types */

#define CONNRECMAILTYPE_PUSHWAIT 0
#define CONNRECMAILTYPE_PULLPUSH 1
#define CONNRECMAILTYPE_PULL 2
#define CONNRECMAILTYPE_PUSH 3

/* Connection record types */

#define CONNRECTYPE_NETWORK '0'
#define CONNRECTYPE_REMOTE '1'
#define CONNRECTYPE_PASSTHRU '2'
#define CONNRECTYPE_REMOTE_LAN_SERVICE '5'
#define CONNRECTYPE_X25 '6'
#define CONNRECTYPE_ISDN '7'
#define CONNRECTYPE_HUNT_GROUP '9'
#define CONNRECTYPE_NNTP 'a'

/* Network connection record item names */

#define CONNREC_TYPE 		"ConnectionType"
#define CONNREC_SOURCE 		"Source"
#define CONNREC_DESTINATION "Destination"
#define CONNREC_PORT 		"PortName"
#define CONNREC_LANPORT		"LanPortName"
#define CONNREC_ADDRESS 	"OptionalNetworkAddress"
#define CONNREC_COMMENTS 	"Comments"
#define CONNREC_ENABLED 	"Enabled"

#define CONNREC_RLAN_SERVICE "RemoteLanService"
#define CONNREC_RLAN_DRIVERS "DisplayDriverList"
#define CONNREC_RLAN_STATIC	"StaticTag"
#define CONNREC_RLAN_ITEMS	"RLANItems"
#define	CONNREC_RLAN_NAME	"RLANTag"
#define CONNREC_RLAN_LOGIN	"RLAN1"
#define CONNREC_RLAN_PASSWORD	"RLAN2"
#define CONNREC_RLAN_NUMBER	"RLAN3"
#define CONNREC_RLAN_DOMAIN	"RLAN7"
#define CONNREC_RLAN_ITEMS_RAS	"7"

#define CONNREC_LANSERVICEDISPLAY "LANSERVICEDISPLAY"
#define CONNREC_RLAN_ORIGINAL_SERVICE	"OriginalService"

#define CONNREC_RLAN_SERVICE_RAS_TEXT	"RAS"
#define CONNREC_RLAN_SERVICE_ARA_TEXT	"ARA"

/* Connection record precedence */
#define	CONNRECFIRST	'1'
#define	CONNRECLOW	'0'

/*  Connection modes */

#define CONNREC_MODE_DIALUP		1
#define CONNREC_MODE_DIRECT 	0

/*  Authentication types */

#define CONNREC_AUTH_NONE		0
#define CONNREC_AUTH_PASSWORD   1


/*  Connection record fields required for Phonebook feature */

#define CONNREC_LAN_PORTNAME		"LanPortName"
#define CONNREC_DIAL_AREA_CODE		"DialAreaCode"
#define CONNREC_LOCAL_PHONE_NUMBER	"LocalPhoneNumber"
#define CONNREC_REMOTE_PORT_NAME	"RemotePortName"
#define CONNREC_ACCOUNTNAME			"ConnectionAccount"

/*	Address Book - "Connections" (and other views) Enabled field values */

#define	KWD_DISABLED 	"Disabled"				/* Obsolete */
#define	KWD_NO 			"No"					/* Obsolete */
#define VALUE_NO		'0'
#define VALUE_DISABLED	'0'
#define VALUE_ENABLED	'1'
#define VALUE_STARTUP	'2'

/* Account form/view stuff in personal address book */

#define ACCOUNTSNAMESPACE "$Accounts"

/* fields in the form */
#define ACCOUNT_FORM		"Account"
#define ACCOUNT_NAME		"AccountName"
#define ACCOUNT_SERVERNAME	"AccountServer"
#define ACCOUNT_USERNAME	"AccountLoginName"
#define ACCOUNT_PASSWORD	"AccountPassword"
#define ACCOUNT_PROTOCOL	"AccountProtocol"
#define ACCOUNT_FILENAME	"AccountFilename"
#define ACCOUNT_SSL			"SSLStatus"
#define ACCOUNT_LOCATION	"AccountLocation"
#define ACCOUNT_CONNECTION	"ConnectionMode"
#define ACCOUNT_PORT		"AccountPortNumber"
#define ACCOUNT_POPDELMAIL	"PopDeleteMail"
#define ACCOUNT_SSLSITECERTS 	"SSLSiteCerts"
#define ACCOUNT_SSLEXPIREDCERTS	"SSLExpiredCerts"
#define ACCOUNT_SSLPROTOCOLVER	"SSLProtocolVersion"
#define ACCOUNT_SSLSENDCERTS	"SSLSendCertificates"
#define ACCOUNT_SSLSERVERAUTH	"SSLServerAuthenticate"
#define ACCOUNT_IMAPDRAFT		"IMAPDrafts"
#define ACCOUNT_IMAPSENT		"IMAPSent"
#define ACCOUNT_REPLHISTORY		"AccountReplicationHistory"
#define ACCOUNT_REPLICAS		"AccountReplicas"
#define ACCOUNT_IMAPFIRSTOPEN	"IMAPFirstOpen"
#define ACCOUNT_IMAPDELMAIL		"IMAPDeleteMail"
#define ACCOUNT_REPL_MAX_PULL	"AccountReplicationMaxPull"

#define ACCOUNT_SSL_ON		"1"
#define	ACCOUNT_SSL_OFF		"0"

/* used to get appropriate accounts for NAMELookups */
#define ACCOUNT_LOOKUPITEMS	"AccountName\0AccountProtocol\0AccountLocation\0AccountFilename\0AccountServer\0IMAPMode\0%AccountFileList"
#define ACCOUNT_NAMEITEM			0
#define ACCOUNT_PROTOCOLITEM		1
#define ACCOUNT_LOCATIONITEM		2
#define ACCOUNT_DATABASEITEM		3		/*	This is the proxy file name only */
#define ACCOUNT_SERVERITEM			4
#define ACCOUNT_IMAPMODEITEM		5
#define ACCOUNT_FILELISTITEM		6		/*	This is a list including the proxy file name and all replicas */
#define ACCOUNT_LOOKUPITEMCOUNT		7

/*   Address Book views used by Mail Address in V3. */

#define PEOPLE_GROUPS_FLAT_VIEW      "($PeopleGroupsFlat)"

/*   Address Book view and column used by Mail Address in V4. */

#define PEOPLE_GROUPS_HIER_VIEW      	"($PeopleGroupsHier)"
#define MAIL_ADDRESS_VIEW				"($PeopleGroupsFlat)"
#define PEOPLE_GROUPS_CORP_HIER_VIEW	"($PeopleGroupsCorpHier)"
#define PEOPLE_GROUPS_BY_LANG_VIEW		"($PeopleGroupsByLang)"

#define MAIL_ADDRESS_PICKLIST_COLUMN	2
#define MAIL_NAME_PICKLIST_COLUMN		3

#define MAIL_ADDRESS_NAME_COLUMN		"NPName"
#define MAIL_ADDRESS_ADDRESS_COLUMN		"MAMailAddress"
#define MAIL_ADDRESS_ALT_NAME_COLUMN	"AltFullName"

#define EXTENDED_ACCESS_CONTROL_VIEW      	"($XACL)"


/*  ($PeoplesGroupsHier) column position,  be careful with modifying these.
		LDAP server uses this view and assumes these positions */
#define MAIL_ADDRESS_HIER_COLUMN		3
#define MAIL_NAME_HIER_COLUMN			4
#define MAIL_NAME_HIER_TYPE				5
#define MAIL_NAME_HIER_PUBLICKEY		6
#define MAIL_HIER_MAILADDRESS			"$25"
#define MAIL_HIER_CN					"$11"
#define MAIL_HIER_DN					"$24" /* This name had changed to "NPName" in R5.0 
												but has changed back because of compatibility */
#define USERS_FULLNAME_COLUMN			"$23" /* Users Full/List Name column named */

/* #define MAIL_HIER_DN						"NPName" */
#define MAIL_HIER_TYPE					"Type"
#define MAIL_HIER_PUBLICKEY				"$20"

/* Person record field names */
#define PERSON_HOME_STREETADDRESS		"StreetAddress"
#define PERSON_HOME_CITY				"City"
#define PERSON_HOME_STATE				"State"
#define PERSON_HOME_ZIP					"Zip"
#define PERSON_HOME_COUNTRY				"Country"
#define PERSON_HOME_FAX					"HomeFAXPhoneNumber"
#define PERSON_SPOUSE					"Spouse"
#define PERSON_CHILDREN					"Children"
#define PERSON_OFFICE_STREETADDRESS		"OfficeStreetAddress"
#define PERSON_OFFICE_CITY				"OfficeCity"
#define PERSON_OFFICE_STATE				"OfficeState"
#define PERSON_OFFICE_ZIP				"OfficeZip"
#define PERSON_OFFICE_COUNTRY			"OfficeCountry"
#define	PERSON_OFFICE_FAX				"OfficeFAXPhoneNumber"
#define PERSON_OFFICE_NUMBER			"OfficeNumber"
#define PERSON_GENERATION_QUALIFIER		"Suffix"
#define PERSON_PERSONAL_TITLE			"Title"
#define PERSON_JOB_TITLE				"JobTitle"
#define PERSON_MANAGER					"Manager"
#define PERSON_CELL_PHONE			  	"CellPhoneNumber"
#define PERSON_PAGER					"PhoneNumber_6"
#define PERSON_ASSISTANT				"Assistant"
#define PERSON_ENCRYPT_INCOMING_MAIL	"EncryptIncomingMail"
#define PERSON_X400_ADDRESS				"x400Address"
#define PERSON_WEB_SITE					"WebSite"
#define PERSON_EMPLOYEE_ID				"EmployeeID"
#define PERSON_ALTFULLNAMESORT          "AltFullNameSort"               /* Phonetic Name */
#define PERSON_PERSONAL_PAGER           "PersPager"
#define PERSON_BRIEFCASE                "Briefcase"


/*	Address Book view used by ServerConfig (to refresh Notes.INI parameters). */

#define SERVER_CONFIG_NAMESPACE		"$ServerConfig"
#define SERVER_CONFIG_NAMESPACE_1	"1\\$ServerConfig"
#define SERVER_CONFIG_VIEW			"($ServerConfig)"
#define SERVER_ACCESS_VIEW			"($ServerAccess)"

/* Server Configuration form items */
#define SERVER_CONFIG_FORM			"ServerConfig"
#define CONFIG_SERVERNAME_ITEM 		"ServerName"		/* Server name */
#define CONFIG_MAILEXT_ITEM			"MailExternalSMTP"

/* LDAP related definitions */
#define LDAP_ACCESS_CONTROL_ITEM		"LDAPAccessControl"
#define LDAP_MAXENTRIES_ITEM			"LDAPMaxEntries"
#define LDAP_MINCHAR_ITEM				"LDAPMinChar"
#define LDAP_TIMEOUT_ITEM				"LDAPTimeout"
#define LDAP_PORT_ITEM					"LDAP_Port"
#define LDAP_PORTSTATUS_ITEM			"LDAP_PortStatus"
#define LDAP_SSLPORT_ITEM				"LDAP_SSLPort"
#define LDAP_SSLSTATUS_ITEM				"LDAP_SSLStatus"
#define LDAP_USESSL_ITEM				"LDAP_UseSSL"
#define LDAP_TCP_NAME_PASSWORD_ITEM		"LDAP_TCPNP"
#define LDAP_ALLOWANONYMOUS_ITEM		"LDAP_AllowAnonymous"
#define LDAP_ENFORCE_ACCESS_ITEM		"LDAP_EnforceAccess"
#define LDAP_SEARCH_ITEM				"LDAP_Search"
#define LDAP_AUTHENTICATION_TYPE_ITEM	"LDAP_AuthenticationType"
#define LDAP_GET_CLIENT_CERT_ITEM		"LDAP_SSLCert"
#define LDAP_SSL_NAME_PASSWORD_ITEM		"LDAP_SSLNP"
#define	LDAP_SSLANONYMOUS_ITEM			"LDAP_SSLAnonymous"
#define LDAP_CONFIG_FORM				"($LDAPSettings)"
#define LDAP_AMBIGUOUS_WRITE_ITEM		"LDAPAmbiguousWrite"
#define LDAP_CLIENT_ACCESS_ITEM			"LDAPAllowWriteAccess"
#define LDAP_OPTIMIZEVIEW_ITEM			"LDAPOptimizeSearch"
#define LDAP_QUERYEXTENDEDOBJECT_ITEM	"LDAPOptimizeSearch" /* "LDAPQueryExtendedObjects" - Post R5.0 */
#define LDAP_QUERYALTLANG_ITEM			"LDAPQueryAltLangInfo"
#define LDAP_VIEW_TYPE					"%LDAPType"
#define LDAP_EXTENDED_OBJECTCLASS_ITEM	"$objectclass"
#define LDAP_ENFORCE_SCHEMA_ITEM		"LDAPEnfrcSchema"
#define LDAP_FT_INDEX_ITEM				"LDAPFTI"
#define LDAP_MAXREFERRALS_ITEM		 	"LDAPMaxRefs"			
#define LDAP_UTF8RESULTS_ITEM 		 	"LDAPUTF8Res"		  	
#define LDAP_STRICTRFCADHERENCE_ITEM 	"LDAPStrictRFC" 	
#define LDAP_LOGFIELDSIZELIMIT_ITEM		"LDAPLogFldSzLim"
#define LDAP_DEREFALIASESENABLED_ITEM	"LDAPDerefAliasesEnabled"


/* LDAP View names */
#define LDAP_CN_VIEW	"($LDAPCN)"
#define LDAP_CN_VIEW_ALT "$LDAPCN"
#define LDAP_S_VIEW		"($LDAPS)"
#define LDAP_G_VIEW		"($LDAPG)"
#define LDAP_HIER_VIEW	"($LDAPHIER)"
#define LDAP_RDN_HIER_VIEW	"($LDAPRDNHIER)"
#define HIGH_SECURITY_AUTH_VIEW	"($HIGH_SECURITY_AUTH)"
#define LDAP_ALIAS_VIEW	"($LDAPAlias)"

/* LDAP Directory Assistance defines */
#define DA_LDAPURL_ITEM		"LDAPURL"	 /* LDAP Server Flag */
#define DA_LDAP_MAX_URL_LEN	256

#define DA_LDAPADMINDN_ITEM				"LDAPAdminDN"
#define DA_LDAPADMINPASSWORD_ITEM		"LDAPAdminPassword"

#define DA_LDAPPORT_ITEM		"LDAPPort"		 /* LDAP Port Number */

/* Column names */
#define LDAP_DN_COLUMN "%DN"


#define MSG_CONFIG_FORM					"($MessagingSettings)"
#define CAT_CONFIG_FORM					"($CatalogerSettings)"


/* Address Book - Items used for Message Tracking Configuration */

#define MT_ENABLED_ITEM					"MTEnabled"
#define MT_NOTRACKING_FOR_ITEM			"MTNoTrackingFor"
#define MT_ALLOWEDTO_TRACK_ITEM			"MTAllowTracking"
#define MT_SUBJECTS_ENABLED_ITEM		"MTLogSubjects"
#define MT_NOSUBJECTS_FOR_ITEM			"MTNoSubjectsFor"
#define MT_ALLOWEDTO_TRACKSUBJECTS_ITEM "MTAllowSubjects"
#define MT_ALLOWEDTO_CAUSEDIALING_ITEM	"MTCallers"
#define MT_COLLECTOR_INTERVAL_ITEM		"MTInterval"
#define MT_MAXRESPONSES					"MTMaxResponses"

/* Address Book - Items used for Mail Journalling Configuration */
#define MAIL_JOURNALLING_ENABLED_ITEM        "JrnlEnbld"
#define MAIL_JOURNALLING_METHOD_ITEM         "JrnlMthd"
#define MAIL_JOURNALLING_DB_NAME_ITEM        "JrnlDBName"
#define MAIL_JOURNALLING_DESTINATION_ITEM    "JrnlDest"
#define MAIL_JOURNALLING_SCOS_ITEM			 "JournallingSCOSReconsitute"
#define MAIL_JOURNALLING_ENCRYPT_EXCL_ITEM	 "JrnlFldEncryptExcl"
#define MAIL_JOURNALLING_CERTIFICATE_ITEM	 "JrnlCert"
#define MAIL_JOURNALLING_DB_METHOD_ITEM 	 "JrnlDBMthd"
#define MAIL_JOURNALLING_DB_SIZE_ITEM	 	 "JrnlDBSz"
#define MAIL_JOURNALLING_DB_PURGE_INT_ITEM   "JrnlDBPurgeInt"
#define MAIL_JOURNALLING_PERIODICITY_ITEM    "JrnlDBPrd"

/* Address Book & INI - Items used for Transactional Logging Configuration INI */

#define TRANSLOG_STATUS					"TRANSLOG_Status"
#define TRANSLOG_PATH  					"TRANSLOG_Path"
#define TRANSLOG_MAXSIZE				"TRANSLOG_MaxSize"
#define TRANSLOG_PERFORMANCE			"TRANSLOG_Performance"
#define TRANSLOG_STYLE					"TRANSLOG_Style"
#define TRANSLOG_USEALL					"TRANSLOG_UseAll"
#define TRANSLOG_AUTOFIXUP				"TRANSLOG_AutoFixup"
#define TRANSLOG_RECREATE_LOGCTRL		"TRANSLOG_Recreate_Logctrl"
#define TRANSLOG_MEDIAONLY				"TRANSLOG_MediaOnly"

/* Address Book & INI - Items used for shared mail server doc and Configuration INI */
#define SCOS_DIR_DEF					"SCOS_DIR_1"
#define SCOS_DIR_ACTIVE_DEF				"SCOS_DIR_ACTIVE_FILES_1"

/* Address Book & INI - Items used for quota enforcement server doc and Configuration INI */
#define NSF_QUOTA_METHOD_INI			"NSF_QUOTA_METHOD"
#define NSF_QUOTA_METHOD_DOC			"QtaMthd"

/* Address Book & INI - Items used to keep compute in check */
#define	FORMULA_TIMEOUT			"FormulaTimeout"

/*	Address Book - Items and names used by name server */

#define NS_SERVERNAMEITEM			0
#define NS_SERVERTITLEITEM			1
#define NS_SERVERNETWORKITEM		2
#define NS_SERVERNETADDRESSESITEM	3
#define NS_SERVERPORTSITEM			4
#define NS_SERVERLOOKUPITEMCOUNT	5
#define NS_SERVERLOOKUPITEMS "ServerName\0ServerTitle\0Network\0NetAddresses\0Ports"

/*	Address Book - Items and names used by DbQuotaSet to verify access */

#define DBQUOTA_SERVERADMINITEM			0
#define DBQUOTA_SERVERLOOKUPITEMCOUNT	1
#define DBQUOTA_SERVERLOOKUPITEMS      "Administrator"

/*	Address Book - Items and names used by server during initialization */

#define INIT_SERVERNAMEITEM				 0
#define INIT_SERVERTITLEITEM			 1
#define INIT_SERVERADMINITEM			 2
										 
#define	INIT_SERVERCERTIFICATE			 3
#define	INIT_SERVERCHANGEREQUEST		 4
									
#define INIT_SERVERALLOWITEM			 5
#define INIT_SERVERDENYITEM				 6
#define INIT_SERVERCREATEITEM			 7
#define INIT_SERVERREPLICAITEM			 8
									
#define INIT_SERVERPTTARGETITEM			 9
#define INIT_SERVERPTCLIENTITEM			10
#define INIT_SERVERPTCALLERITEM			11
#define INIT_SERVERPTACCESSITEM			12

#define INIT_SERVER_WHITELIST			13
#define INIT_SERVER_WHITELISTLOG		14
#define INIT_SERVER_ANONYMOUSACCESS		15
										
#define INIT_SERVER_BUILDNUMBER			16
/*
*/
#define INIT_SERVER_MAJORVERSION		17
#define INIT_SERVER_MINORVERSION		18
#define INIT_SERVER_QMRVERSION			19
#define INIT_SERVER_QMUVERSION			20
#define INIT_SERVER_HOTFIXVERSION		21
#define INIT_SERVER_FIXPVERSION			22
#define INIT_SERVER_FLAGSVERSION		23
/*
*/
#define INIT_SERVERRESTRICTEDLIST		24
#define INIT_SERVERUNRESTRICTEDLIST		25

#define INIT_SERVER_CHECK_CLIENT_PW 	26
#define INIT_SERVER_CLUSTERNAME			27

#define INIT_SERVERMONITORALLOWLIST		28
#define INIT_SERVERMONITORDENYLIST		29

#define INIT_SERVERSMTPENABLED			30

#define INIT_TRANSLOG_STATUS			31 /* Items used for Transactional Logging Configuration */
#define INIT_TRANSLOG_PATH  			32
#define INIT_TRANSLOG_MAXSIZE			33
#define INIT_TRANSLOG_PERFORMANCE		34
#define INIT_TRANSLOG_STYLE				35
#define INIT_TRANSLOG_USEALL			36
#define INIT_TRANSLOG_AUTOFIXUP			37

#define	INIT_SERVER_MIN_NOTES_PW		38
#define INIT_SERVER_ENABLE_HTTPSYNC		39
#define INIT_QUOTA_METHOD				40
#define INIT_FORMULA_TIMEOUT			41

#define INIT_SERVER_PUBKEY_MINWIDTH		42
#define INIT_SERVER_PUBKEY_MAXWIDTH		43
#define INIT_SERVER_PUBKEY_DEFWIDTH		44
#define INIT_SERVER_PUBKEY_MAXAGE		45
#define INIT_SERVER_PUBKEY_MINDATE		46
#define INIT_SERVER_PUBKEY_DUEDATE		47
#define INIT_SERVER_PUBKEY_PRIORITY		48
#define INIT_SERVER_PUBKEY_OLDKEYDAYS	49

#define INIT_DB2_ITEM_INIT				50
#define INIT_DB2_ITEM_DIRECTORY			51
#define INIT_DB2_ITEM_INSTANCE			52
#define INIT_DB2_ITEM_DB				53
#define INIT_DB2_ITEM_SCHEMA			54
#define INIT_DB2_ITEM_DEFAULT			55
#define INIT_DB2_ITEM_PREF				56
#define INIT_DB2_ITEM_UDFSRV			57
#define INIT_DB2_ITEM_UDFPATH			58
#define INIT_DB2_ITEM_DB2GRPMAX			59

#define INIT_SERVERLOOKUPITEMCOUNT		60
#define INIT_SERVERLOOKUPITEMS      	\
"ServerName\0ServerTitle\0Administrator\0\
Certificate\0ChangeRequest\0\
AllowAccess\0DenyAccess\0CreateAccess\0ReplicaAccess\0\
PTTargets\0PTClients\0PTCallers\0PTAccess\0\
WhiteList\0WhiteListLog\0AnonymousAccess\0\
ServerBuildNumber\0MajVer\0MinVer\0QMRVer\0QMUVer\0HotVer\0FixPVer\0FlagsVer\0\
RestrictedList\0UnrestrictedList\0\
ServerCheckPasswords\0ClusterName\0AllowMonitors\0DenyMonitors\0\
SMTPListenerEnabled\0\
TRANSLOG_Status\0TRANSLOG_Path\0TRANSLOG_MaxSize\0TRANSLOG_Performance\0\
TRANSLOG_Style\0TRANSLOG_UseAll\0TRANSLOG_AutoFixup\0\
MinumumNotesPW\0EnableHTTPSyncWithNotesPW\0QtaMthd\0FormulaTimeOut\0\
PKMinWidth\0\
PKMaxWidth\0\
PKDefWidth\0\
PKMaxAge\0\
PKMinDate\0\
PKDueDate\0\
PKPriority\0\
PKOldKeyDays\0\
DB2Init\0\
DB2Dir\0\
DB2Inst\0\
DB2Db\0\
DB2Sche\0\
DB2Def\0\
DB2Pref\0\
DB2UDFSrv\0\
DB2UDFPath\0\
DB2GroupMax\0"

/*	Address Book - Items and names used by cconsole to verify access */

#define CCONSOLE_SERVERADMINITEM			0
#define CCONSOLE_SERVERLOOKUPITEMCOUNT	1
#define CCONSOLE_SERVERLOOKUPITEMS      "Administrator"

/* 
 * Address Book - Items and names used by Assist to verify agent execution 
 * access.
 */
                                        
#define ASSIST_PRIVATE_ITEM             0
#define ASSIST_RESTRICTED_ITEM          1
#define ASSIST_UNRESTRICTED_ITEM        2
#define ASSIST_START_ITEM               3
#define ASSIST_END_ITEM                 4
#define ASSIST_MAX_START_ITEM			5
#define ASSIST_MAX_END_ITEM				6

#define ASSIST_LOOKUP_AMGR_ITEM_COUNT   7
#define ASSIST_LOOKUP_AMGR_ITEMS        "PrivateList\0RestrictedList\0UnrestrictedList\0StartTime\0EndTime\0MaxStartTime\0MaxEndTime"

#define ASSIST_PRIVATE_LIST_NAME        "PRIVATE_LIST"
#define ASSIST_RESTRICTED_LIST_NAME     "RESTRICTED_LIST"
#define ASSIST_UNRESTRICTED_LIST_NAME   "UNRESTRICTED_LIST"

/* 
 * Address Book - Items and names used by Agent Manager to verify agent
 * execution access.
 */

#define AMGR_RELOAD_ITEM            		0
#define AMGR_NORMAL_START_ITEM             	1
#define AMGR_NORMAL_END_ITEM               	2
#define AMGR_NORMAL_CONCURRENT_TASKS_ITEM  	3
#define AMGR_NORMAL_MAX_EXECUTION_ITEM     	4
#define AMGR_NORMAL_PERCENT_TIME_ITEM      	5
#define AMGR_MAX_START_ITEM             	6
#define AMGR_MAX_END_ITEM               	7
#define AMGR_MAX_CONCURRENT_TASKS_ITEM  	8
#define AMGR_MAX_MAX_EXECUTION_ITEM     	9
#define AMGR_MAX_PERCENT_TIME_ITEM     		10
#define AMGR_CREATE_DB_ITEM         		11
#define AMGR_PRIVATE_ITEM           		12
#define AMGR_RESTRICTED_ITEM        		13
#define AMGR_UNRESTRICTED_ITEM      		14
#define AMGR_ALLOWACCESS_ITEM      			15
#define AMGR_DENYACCESS_ITEM      			16
#define AMGR_CREATEREPLICA_DB_ITEM         	17
#define AMGR_SERVERS_NOTEID_ITEM         	18 /* expanded lookup to include servers id note */
#define AMGR_SERVERS_NAPATH_ITEM         	19 /* expanded lookup to include N&A path */


#define AMGR_LOOKUP_ITEM_COUNT      18
#define AMGR_LOOKUP_ITEMS           \
"ReloadTime\0StartTime\0EndTime\0ConcurrentAgents\0MaxExecution\0\
PercentTime\0MaxStartTime\0MaxEndTime\0MaxConcurrentAgents\0\
MaxMaxExecution\0MaxPercentTime\0CreateAccess\0PrivateList\0\
RestrictedList\0UnrestrictedList\0AllowAccess\0DenyAccess\0\
ReplicaAccess"

/* extended lookup used for web agents and synch new mail agents, i.e. API based invocation */
#define AMGR_EXT_LOOKUP_ITEM_COUNT      20
#define AMGR_EXT_LOOKUP_ITEMS           \
"ReloadTime\0StartTime\0EndTime\0ConcurrentAgents\0MaxExecution\0\
PercentTime\0MaxStartTime\0MaxEndTime\0MaxConcurrentAgents\0\
MaxMaxExecution\0MaxPercentTime\0CreateAccess\0PrivateList\0\
RestrictedList\0UnrestrictedList\0AllowAccess\0DenyAccess\0\
ReplicaAccess\0$$NoteId\0$$DBName"

/* Address book - Contains port configuration and other parameters used by
 * DIIOP process.
 */
#define DIIOP_TCP_PORT_STR				"IIOP_Port"
#define DIIOP_TCP_PORT_STATUS_STR		"IIOP_PortStatus"
#define DIIOP_SSL_PORT_STR				"IIOP_SSLPort"
#define DIIOP_SSL_PORT_STATUS_STR		"IIOP_SSLStatus"
#define DIIOP_IOR_HOST_STR				"IIOP_IORHost"
#define DIIOP_IDLE_MINS_ALLOWED_STR		"IIOP_IdleMinsAllowed"
#define DIIOP_EXTERNAL_HTML_DIR_STR		"IIOP_HtmlDir"
#define HTTP_HTML_DIR_STR				"HTTP_HtmlDir"
#define HTTP_SSL_KEYFILE_STR			"HTTP_SSLKeyFile"

#define INTERNET_AUTH_VIEW				"HTTP_AuthView"


/* Address book - Items used by the Domino back-end objects for security
 * configuration
 */

#define LSBE_BROWSE_FLAG				0
#define LSBE_VIEW						1
#define LSBE_IIOP_TCP_NAME_AND_PASSWD	2
#define LSBE_IIOP_TCP_ANONYMOUS			3
#define LSBE_IIOP_SSL_NAME_AND_PASSWD	4
#define LSBE_IIOP_SSL_ANONYMOUS			5
#define LSBE_HTTP_SESSION_AUTH			6
#define LSBE_HTTP_SSO_CONFIG			7

#define LSBE_SECURITY_ITEM_COUNT		8
#define LSBE_SECURITY_ITEMS			"HTTP_DatabaseBrowsing\0HTTP_AuthView\0\
IIOP_TCPNP\0IIOP_AllowAnonymous\0\
IIOP_SSLNP\0IIOP_SSLAnonymous\0HTTP_enableSessionAuth\0HTTP_SSOCfg"

/* Address book - Items used by the Domino back-end objects for getting person
 * information
 */
#define LSBE_HTTPPASSWORD				0
#define LSBE_FULLNAME					1
#define LSBE_ALTFULLNAME				2
#define LSBE_ALTFULLNAMELANG			3
#define LSBE_DOMAINTYPE					4
#define LSBE_PERSON_ITEM_COUNT			5
#define LSBE_FULL_PERSON_ITEMS			"HTTPPassword\0FullName\0AltFullName\0AltFullNameLanguage\0$$DomainType\0$$DBIndex"
#define LSBE_LIM_PERSON_ITEMS			"HTTPPassword\0%DN\0AltFullName\0AltFullNameLanguage\0$$DomainType\0$$DBIndex"

/* Address book - Lightweight third-party Authentication Items
 */

#define LTPA_SVITEMS				"HTTP_enableSessionAuth\0HTTP_WebSSOConfig"
#define LTPA_SVITEM_SESSIONAUTH		0
#define LTPA_SVITEM_WEBSSOCONFIG	1
#define LTPA_SVITEMS_COUNT			2

#define LTPA_NAMESPACE				"($WebSSOConfigs)"

#define LTPA_WSTOKENNAME			"LtpaToken"

#define LTPA_TOKENNAME				"Ltpa_TokenName"
#define LTPA_TOKENDOMAIN			"Ltpa_TokenDomain"
#define LTPA_TOKENEXPR				"Ltpa_TokenExpiration"
#define LTPA_DOMSECRET				"Ltpa_DominoSecret"
#define LTPA_DOMSECRETNEXT			"Ltpa_DominoSecretNext"
#define LTPA_DOMSECRETPREV			"Ltpa_DominoSecretPrev"
#define LTPA_WSENABLED				"Ltpa_WSEnabled"
#define LTPA_WS3DESDATA				"Ltpa_WS3DESData"
#define LTPA_WSRSADATA				"Ltpa_WSRSAData"
#define LTPA_WSREALM				"Ltpa_WSRealm"
#define LTPA_WSVERSION				"Ltpa_WSVersion"
#define LTPA_ROLLOVERENABLED		"Ltpa_RolloverEnabled"
#define LTPA_LASTROLLOVERDATE		"Ltpa_LastRolloverDate"
#define LTPA_IDLETIMEOUTMAX			"Ltpa_IdleTimeoutMax"
#define LTPA_IDLETIMEOUTMIN			"Ltpa_IdleTimeoutMin"
#define LTPA_NAMEMAPPING			"Ltpa_MapNm"

#define LTPA_MAXTOKENLEN		2048
#define LTPA_MAXTOKENNAMELEN	64
#define LTPA_MAXTOKENDOMAINLEN	MAX_TCP_HOST_NAME
#define	LTPA_MAXWSREALMLEN		256


/*	Address book - Items used by Calendaring & Scheduling for user lookup
	and domain lookup. */

#define SCHED_USERLOOKUPITEMCOUNT			6
#define SCHED_USERLOOKUPITEMS "FullName\0ListName\0MailAddress\0MailDomain\0MailServer\0CalendarDomain"
#define SCHED_USERLOOKUPFULLNAMEITEM		0
#define SCHED_USERLOOKUPLISTNAMEITEM		1
#define SCHED_USERLOOKUPMAILADDRESSITEM		2
#define SCHED_USERLOOKUPMAILDOMAINITEM		3
#define SCHED_USERLOOKUPMAILSERVERITEM		4
#define SCHED_USERLOOKUPCALENDARDOMAINITEM	5

#define SCHED_DOMAINLOOKUPITEMCOUNT					3
#define SCHED_DOMAINLOOKUPITEMS "DomainType\0CalendarServer\0CalendarSystem"
#define SCHED_DOMAINLOOKUPDOMAINTYPEITEM			0
#define SCHED_DOMAINLOOKUPCALENDARSERVERITEM		1
#define SCHED_DOMAINLOOKUPCALENDARSYSTEMITEM		2

#define SCHEDULE_MGR_LOOKUP_ITEM_COUNT			7
#define SCHEDULE_MGR_LOOKUP_ITEMS				"FullName\0MailServer\0MailFile\0MailDomain\0CalendarDomain\0Type\0MailAddress"
#define SCHEDULE_MGR_LOOKUP_ITEM_MAILSERVER		1
#define SCHEDULE_MGR_LOOKUP_ITEM_MAILFILE		2
#define SCHEDULE_MGR_LOOKUP_ITEM_MAILDOMAIN     3
#define SCHEDULE_MGR_LOOKUP_ITEM_CALENDARDOMAIN 4
#define SCHEDULE_MGR_LOOKUP_ITEM_TYPE			5
#define SCHEDULE_MGR_LOOKUP_ITEM_MAILADDRESS    6

/*	View note item names */

#define VIEW_TITLE_ITEM 				FIELD_TITLE				/* Title */
#define VIEW_COLLECTION_ITEM			"$Collection"			/* Contains object ID */
#define VIEW_CONTAINER_ITEM				"$ViewContainer"		/* Contains storage object ID */
#define VIEW_FORMULA_ITEM				"$Formula"				/* Selection formula buffer */
#define VIEW_FORMULA_TIME_ITEM			"$FormulaTV"			/* If present, formula is "time-relative" (@NOW-based) */
#define VIEW_CLASSES_ITEM 				"$FormulaClass" 		/* Formula note classes */
#define VIEW_COLLATION_ITEM				"$Collation"			/* Collation buffer */
#define VIEW_TOTALS_ITEM				"$Totals"				/* Subtotalling specification */
#define VIEW_VIEW_FORMAT_ITEM 			"$ViewFormat" 			/* View table format item */
#define VIEW_INDEX_ITEM 				"$Index"				/* Index disposition options */
#define VIEW_DESIGN_VER_ITEM_V2			"$DesignVersion"		/* Design note version, for V2 backward compatibility */
#define VIEW_DESIGN_VER_ITEM			"$Version"				/* Design note version */
#define VIEW_NOTEREF_ITEM 				FIELD_LINK				/* Note Reference item */
#define VIEW_CONFLICT_ITEM				"$Conflict"				/* Replication update conflict */
#define VIEW_FORM_FORMULA_ITEM			"$FormFormula"			/* Form Formula */
#define VIEW_COMMENT_ITEM				"$Comment"				/* View comment. */
#define VIEW_SELQUERY_ITEM				"$SelQuery"				/* View selection query object */
#define VIEW_LASTSEENUID_ITEM			"$LastSeenUID"			/* Internet Replication Last Seen UID */
#define ITEM_NAME_REFOPT				"$RefOptions"			/* ALlow a ref to original 
																	note to be included. */
#define VIEW_FORMAT_CUSTOMIZE_ITEM		"$CustomViewFormat"
#define VIEW_INHERITED_FROM_ITEM		"$ViewInheritedFrom"	/* what was this view/folder based on */
#define VIEW_SQLQUERY_ITEM				"$ViewSQLQuery"		/* SQL Query for View Selection */

/*	Calendar view print customization items... */
#define VIEW_FORMAT_CALDAYPRINT_ITEM		"$CalDayPrintCustomFormat"
#define VIEW_FORMAT_CALWEEKPRINT_ITEM		"$CalWeekPrintCustomFormat"
#define VIEW_FORMAT_CALMONTHPRINT_ITEM		"$CalMonthPrintCustomFormat"
#define VIEW_FORMAT_CALWORKWEEKPRINT_ITEM	"$CalWorkWeekPrintCustomFormat"
#define VIEW_FORMAT_CALROLLINGPRINT_ITEM	"$CalRollingPrintCustomFormat"
#define VIEW_FORMAT_CALLISTPRINT_ITEM		"$CalListPrintCustomFormat"

/* Some custom view information stored in bookmark cache. */
#define VIEW_FORMAT_CUSTOM_FLAGS_ITEM		"$ViewCustomFlags"
#define VIEW_FORMAT_CUSTOM_NEWESTMAIL_TIME "$NewestMailTime"	/* what time the last refresh was done. */
#define VIEW_FORMAT_CUSTOM_NEWESTMAIL_UNID "$NewestMailUNID"	/* what is the unid of topmost newest mail */

/* Calendar style view items */

#define VIEW_CALENDAR_FORMAT_ITEM  		"$CalendarFormat"		/* Calendar View format item */

#define VIEW_FORMAT_DAILY_CAL_ITEM		"$DailyCalViewFormat"
#define VIEW_FORMAT_WEEKLY_CAL_ITEM		"$WeeklyCalViewFormat"
#define VIEW_FORMAT_MONTHLY_CAL_ITEM	"$MonthlyCalViewFormat"
#define VIEW_FORMAT_ROLLING_CAL_ITEM	"$RollingCalViewFormat"
#define VIEW_FORMAT_LIST_CAL_ITEM		"$ListCalViewFormat"

/* Folder-related view items */
/* Pre-build 126 items, can be removed before V4 ship - only exposed internally */
#define VIEW_UNID_SELECTION_ITEM		"$UNIDSelection"		/* List of UNID's which are in view. */
#define VIEW_USE_UNID_SELECTION_ITEM	"$UseUNIDSelection"		/* If present, uses UNID selection table. */

#define VIEW_FOLDER_OBJECT				"$FolderObject"			/*  If present, is ODS version of FOHEADER and 
																	set of entries, see dbfolder.h.  The view
																	may have additional items with suffixes
																	on this item name, e.g., $FolderObject1 */
#define VIEW_FOLDER_IDTABLE				"$FolderIDTable"		/*  If present, is ID table representing the
																	contents of the folder */
#define VIEW_ANTIFOLDER_IDTABLE			"$AntiFolderIDTable"	
#define VIEW_VIEWLOGGING				"$ViewLogging"			/* If present, states whether to log backing btree */


/* Used only within NSF for single copy template */
#define NOTE_REFERENCE_ITEM				"$NoteReference"		/* Reference to template note that this note inherits from */


/*	Hidden view name prefix/postfix strings. */

#define HIDDEN_VIEW_NAME_PREFIX		"("
#define HIDDEN_VIEW_NAME_POSTFIX	")"

/* ViewMap note item names */

#define VIEWMAP_DATASET_ITEM	"$ViewMapDataset"	/* Contains ViewMap dataset data */
#define VIEWMAP_LAYOUT_ITEM		"$ViewMapLayout"	/* Contains layout objects */
#define VIEWMAP_IMAGEMAP_ITEM	"$NavImagemap"		/* Contains the navigator's imagemap */

/*	UName special fields */

#define	UNAME_PATH_ITEM			"Path"
#define	UNAME_SERVER_ITEM		"Server"
#define	UNAME_DB_ITEM			"Database"

/*	Actions */

#define ACTION_ITEM "$ACTIONS"
#define V5ACTION_ITEM "$V5ACTIONS"

/* Filter note item names */

#define	FILTER_TYPE_ITEM		"$Type"
#define	FILTER_COMMENT_ITEM		"$Comment"
#define	FILTER_FORMULA_ITEM		"$Formula"		/*	Required */
#define	FILTER_FORMULA2_ITEM	"$Formula2"		/*	Second one, optional */
#define	FILTER_SCAN_ITEM		"$Scan"			/*	all in view, selected in view, all in db, unread in db	*/
#define	FILTER_HIGHLIGHT_ITEM	"$Highlight"	/*	'1' if hilites enabled, '0' to remove */
#define	FILTER_QUERY_ITEM		"$Query"		/*	Full text query string used to refine notes to do */
#define FILTER_OPERATION_ITEM	"$Operation"	/*	update, create new, select	*/
#define FILTER_PERIOD_ITEM		"$Period"		/*	hourly, daily, or weekly	*/
#define FILTER_LEFTTODO_ITEM	"$LeftToDo"		/*	Data structure describing what's left to do */
#define FILTER_MACHINE_ITEM		"$MachineName"	/*	name of machine on which this background filter may run */

/*	filter types	*/

#define FILTER_TYPE_MENU 		0
#define FILTER_TYPE_BACKGROUND	1
#define FILTER_TYPE_MAIL 		2
#define FILTER_TYPE_ONCE		3

/*	filter scan options	*/

#define FILTER_SCAN_ALL  		0
#define FILTER_SCAN_UNREAD 		1
#define FILTER_SCAN_VIEW 		2
#define FILTER_SCAN_SELECTED 	3
#define FILTER_SCAN_MAIL		4
#define FILTER_SCAN_NEW			5

#define FILTER_OP_UPDATE  	0
#define FILTER_OP_SELECT  	1
#define FILTER_OP_NEW_COPY	2

/*	filter periods	*/

#define	PERIOD_HOURLY	0
#define	PERIOD_DAILY	1
#define	PERIOD_WEEKLY	2
#define	PERIOD_DISABLED	3

/*	Query note item names. */

/*	If the user has typed in a query, it is stored in $SimpleQuery; If
	the user has created a query using the Query Builder, it is stored
	in the $Builder... fields. */

#define QUERY_SIMPLEQUERY_ITEM	"$SimpleQuery"	/*	simple query */
#define QUERY_BUILDERQUERY_ITEM	"$BuilderQuery"	/*	Query Builder: text list of */
												/*	different portions of query: */
												/*	any of these, all of these, */
												/*	none of these, author */

#define QUERY_BUILDERDATE_ITEM	"$BuilderDate"	/*	Date before or after */
#define QUERY_BUILDERFLAGS_ITEM	"$BuilderFlags"	/*	query builder flags */
#define QUERY_FIELDQUERY_ITEM	"$FieldQuery"	/*	query by form query */

#define QUERY_FIELDACTION_ITEM	"$FieldAction"	/*	action by form struct */
#define QUERY_FORMULAACTION_ITEM	"$FormulaAction"	/*	@function action formula */

#define QUERY_BUILDERFLAG_NEAR			'N'		/*	Apply near to 'all of these' */
#define QUERY_BUILDERFLAG_DATEBEFORE	'B'		/*	Before $BuilderDate (as opposed to after) */

/* 	V4 Search Bar queries */

#define QUERY_V4_OBJECTQUERY_ITEM	"$V4QueryObject" /* Query object built by the V4 Search Builder */
#define QUERY_V4_OPTIONS_ITEM		"$V4QueryOptions" /* Query options from the V4 Search Builder */

/*	V4 Assistants */

#define ASSIST_INFO_ITEM		"$AssistInfo"	/*	info (TYPE_ASSISTANT_INFO) */
#define ASSIST_TYPE_ITEM		"$AssistType"	/*	Type of assistant - related to action type */
#define ASSIST_QUERY_ITEM		"$AssistQuery"	/*	assistant query item */
#define ASSIST_ACTION_ITEM		"$AssistAction"	/*	assistant action item */
#define ASSIST_LASTRUN_ITEM		"$AssistLastRun"	/*	TimeDate of last run */
#define ASSIST_DOCCOUNT_ITEM	"$AssistDocCount"	/*	Number of docs run on last run */
#define ASSIST_RUNINFO_ITEM		"$AssistRunInfo"	/*	Run information object */
#define ASSIST_EXACTION_ITEM	"$AssistAction_Ex"	/*	assistant action item - extra data */
#define ASSIST_VERSION_ITEM		"$AssistVersion"	/*	TIMEDATE of when the agent design 
														(as opposed to enable/disable state)
														was changed */
#define ASSIST_FORMAT_VER		"$AssistFormatVer"	/* format of the agent structure */
#define ASSIST_FORMAT_VER5040	"05040"				/* introduced in 5.04 */
#define ASSIST_FORMAT_VER5030	"05030"				/* introduced in 5.03 */


/* Types for ASSIST_TYPE_ITEM - the following defines are used to determine
 * what type of assistant we are dealing with -- ie, what is the main
 * action type that the user has defined.  The formula ones are for
 * backwards compatability with V3.
 *  See ods.h for the SIG_ACTION_xxx values used for regular V4 generic agents.
 */

#define ASSIST_SIG_ACTION_NONE	 		   -1	/* No action defined */
/* formula, update only -FILTER_OP_UPDATE 	0 */
/* formula, select only -FILTER_OP_SELECT  	1 */
/* formula, new copy    -FILTER_OP_NEW_COPY 2 */

/*	The default search form and default action forms are used by query by form
	and action by form. */

#define DEFAULT_QUERY_BY_FORM		"$DefaultSearch"
#define DEFAULT_ACTION_BY_FORM		"$DefaultAction"

/* Misc Flags */

#define ASSIST_FLAG_ENABLED			'E'
#define ASSIST_FLAG_DISABLED		'D'
#define ASSIST_FLAG_NEWCOPY			'N'
#define ASSIST_FLAG_HIDDEN			'H'
#define ASSIST_FLAG_PRIVATE			'P'
#define ASSIST_FLAG_THREAD			'T'
#define ASSIST_FLAG_ALLOW_REMOTE_DEBUGGING	'R'
#define ASSIST_FLAG_ALLOW_UNSECURE_LS_LIBS	'L'
#define ASSIST_FLAG_AGENT_RUNASWEBUSER	'u'		/* signed "shadow" flag to verify value of unsigned DESIGN_FLAG_AGENT_RUNASWEBUSER*/
#define ASSIST_FLAG_AGENT_RUNASSIGNER	's'		/* signed "shadow" flag to verify value of unsigned DESIGN_FLAG_AGENT_RUNASWEBUSER*/
#define ASSIST_FLAGS_ITEM		"$AssistFlags"	/*	Misc flags */
#define ASSIST_FLAGS_ITEM2		"$AssistFlags2"	/* Unsigned Misc flags */
#define	ASSIST_ONBEHALFOF		"$OnBehalfOf"	/* used by agents to specify whose authority the agents
													runs under (other than the signer) */
#define ASSIST_RESTRICTED		"$Restricted"	/* agent designer can force the agent to have restricted
												   rights, even if the signer has unrestricted rights , or
												   raise their rights to be full admin + unrestricted */

#define ASSIST_RESTRICTED_RESTRICTED	 0x00000001 /* base value for restricted */
#define ASSIST_RESTRICTED_UNRESTRICTED	 0x00000000 /* base value for unrestricted */
#define ASSIST_RESTRICTED_NOT_FORCED	 0x00000000	/* used when the item is not used to force restrictions in the agent 
													   agent can be unrestricted, if the signiture allows it */
#define ASSIST_RESTRICTED_FORCED		 0x00000001	/* used when the designer wants to force the agent to be only restricted
												   note: this flag has no effect when the agent signer is restricted or
												   has no rights to run the agents */
#define ASSIST_RESTRICTED_FULLADMIN		 0x00000002	/* unrestricted (low bit of zero, plus fulladmin */
#define ASSIST_RESTRICTED_NOT_PRESENT	 0x10000000	/* (in-memory) used when the item is not present in the agent */
#define ASSIST_RESTRICTED_FORCED_USED	 0x20000000	/* (in-memory) set when the flag was used to lower permissions */
#define ASSIST_RESTRICTED_SCRIPT_LIBRARY 0x40000000	/* (in-memory) used when the agent rights are lowered because of the script library */


#define ASSIST_MAIL_ITEM		"$AssistMail"	/*	If this field exists in a mail note, it means that */
												/*	mail message was created by an agent. */

#define ASSIST_TRIGGER_ITEM		"$AssistTrigger"	/*	(Read-only) Assistant's trigger type */
#define FORM_SCRIPT_OBJECT		"$$FormScript_O"	/* Script object code item */

/* These were moved here from \edit\dmem.h */
#define FORM_SCRIPT_ITEM_NAME "$$FormScript"
#define	DOC_SCRIPT_ITEM		"$Script"
#define DOC_SCRIPT_ITEM_OBJECT "$$Script_O"
#define DOC_ACTION_ITEM		"$$FormAction"
#define DOC_SCRIPT_NAME		"$$ScriptName"
#define DOC_SCRIPT_BREAKPOINTS		"$Breakpoints"

/* View Script items */
#define VIEW_SCRIPT_NAME		"$ViewScript"
#define VIEW_SCRIPT_OBJECT		"$ViewScript_O"
#define VIEW_GLOBAL_SCRIPT_NAME		"$ViewGlobalScript"
#define VIEW_GLOBAL_SCRIPT_OBJECT	"$ViewGlobalScript_O"
#define VIEW_ACTION_ITEM_NAME	"$ViewAction" 	/* Formula for view script event */
#define VIEW_ACTION_EVENT_COUNT	14	/* ie, $ViewAction0-$ViewAction10 - one per view script event that can contain formulas */

/* Database Script items */
#define DB_SCRIPT_DESIGN_NAME	"Database Script"
#define DB_SCRIPT_NAME		"$DBScript"
#define DB_SCRIPT_OBJECT	"$DBScript_O"
#define DB_ACTION_ITEM_NAME	"$DBAction"

/* Script Library items */
#define SCRIPTLIB_ITEM_NAME	"$ScriptLib"
#define SCRIPTLIB_OBJECT	"$ScriptLib_O"
#define	JAVASCRIPTLIBRARY_CODE "$JavaScriptLibrary"

/* Servlet items */
#define SERVLET_FLAGS_ITEM	"$ServletFlags"
#define SERVLET_FLAG_HASSOURCE		'S'		/* has real source, not just included files */

/* Web Service items */
//#define WEBSERVICE_NAMESPACE "$WSNameSpace"
#define WEBSERVICE_PORTTYPE "$WSPortType"
#define WEBSERVICE_ELTNAME "$WSEltName"
#define WEBSERVICE_PORTNAME "$WSPortName"
#define WEBSERVICE_PORTCLASS "$WSPortClass"

#define WEBSERVICE_FLAGS_ITEM	"$WebServiceFlags"
#define WEBSERVICE_FLAG_SOAPDEFAULT	'S'
#define WEBSERVICE_FLAG_SOAPOPERATION 'O'
#define WEBSERVICE_FLAG_DOCBIND 'D'
#define WEBSERVICE_FLAG_WRAPBIND 'W'
#define WEBSERVICE_FLAG_LITERALBIND 'L'
#define WEBSERVICE_FLAG_XMLDATAMAP 'X'
#define WEBSERVICE_FLAG_XMLDOCTYPE 'Y'
#define WEBSERVICE_FLAG_ISVALID 'V'
#define WEBSERVICE_FLAG_FROZEN		'F'
#define WEBSERVICE_FLAG_VALIDATION	'N'
#define WEBSERVICE_FLAG_BADCOMPILE	'C'

/* Replication formula note item names */

#define REPLFORMULA_SERVER_ITEM		FIELD_TITLE
#define REPLFORMULA_VERSION_ITEM	"$ReplVersion"
#define REPLFORMULA_SOURCE_SERVERS	"$ReplSrcServers"
#define REPLFORMULA_NOTECLASS_ITEM	"$ReplClassMasks"
#define REPLFORMULA_FORMULA_ITEM 	"$ReplFormula"
#define REPLFORMULA_VIEW_ITEM		"$ReplView"	/* V4 feature; sel repl by view */
#define REPLFORMULA_PRIVATE_FOLDER_ITEM	"$ReplPrivateFolder" 	/* V4 feature; sel repl by private folder */
#define REPLFORMULA_FIELDS_ITEM		"$ReplFields"	/* V4.5 feature; sel repl by fields */
#define REPLFORMULA_LOCAL_FOLDER_ITEM	"$ReplLocalFolder"

#define REPL_LOCAL_FOLDER_DEFAULT	"($KeepLocal)"

#define	REPLICATION_FORM				"$Replication"		/* Name of replication form */
#define REPLFORMULA_FORM_ITEM			"$ReplForm"			/* V6 feature; sel repl by form */
#define REPLFORMULA_FORM_NOTUSED		"0"
#define REPLFORMULA_FORM_USED			"1"
#define REPLFORMULA_FORM_FORMULA_ITEM	"$ReplFormFormula"


#define FIELD_NAME_DELIMITER ';'
#define VIEW_NAME_DELIMITER ';'
#define VIEW_NAME_DELIMETER VIEW_NAME_DELIMITER /* (misspelled!) */

/* Special replication flags */
 
#define REPLFLAGS_ITEM 	"$ReplFlags"
#define REPL_FLAG_NOREPL	'n'

/* Icon note item names */

#define DESK_ICON_ITEM "IconBitmap"
#define LSECFLAGS_ITEM 	"$LSECFlags"
#define DEFAULT_NAVIGATOR_ITEM "$DefaultNavigator"
#define DEFAULT_WEB_NAVIGATOR_ITEM "$DefaultWebNavigator"
#define DEFAULT_WEBPAGE_ITEM "$DefaultWebPage"
#define DEFAULT_OUTLINE_ITEM "$DefaultOutline"
#define DEFAULT_FRAMESET_ITEM "$DefaultFrameset"
#define DEFAULT_WEB_FRAMESET_ITEM "$DefaultWebFrameset"
#define DEFAULT_WEB_PAGE_ITEM "$DefaultWebLaunchPage"
#define ICON_TEMPLATEMODTIME_ITEM "$TemplateModTime"
#define ICON_TEMPLATESERVERNAME_ITEM "$TemplateServerName"
#define ICON_TEMPLATEFILENAME_ITEM "$TemplateFileName"
#define ICON_UPDATEDBYLIMIT	"$UpdatedByLimit"
#define ICON_REVISIONSLIMIT	"$RevisionsLimit"
#define ICON_SOFTDELETE_EXPIRE_HOURS "$SoftDeleteExpireHours"
#define LAUNCH_VIEWNAME_ITEM "$LaunchViewName"
#define LAUNCH_WEB_VIEWNAME_ITEM "$LaunchWebViewName"
#define LAUNCH_DOCLINK_ITEM "$LaunchDoclink"
#define LAUNCH_WEB_DOCLINK_ITEM "$LaunchWebDoclink"
#define LAUNCH_WEB_DOCLINK_ANCHOR_ITEM "$LaunchWebDoclinkAnchor"
#define DEFAULT_LANGUAGE	"$DefaultLanguage"
#define DEFAULT_COLLATION	"$DefaultCollation"
#define COLLATION_TYPE	"$CollationType"
							 
/*	Character flags for replicated local security property ($LSECFlags field
	in the icon note */

#define CHFLAG_SET_LOCALSEC		's'			/* should set local access control */
#define CHFLAG_CLEAR_LOCALSEC	'c'			/* should clear local access control */
#define CHFLAG_COMPACT_LOCALSEC 'p'			/* compact should set local access control */

/* 	character flags for the "replicated file header" ($Flags field in the 
	icon note) */

/*	NOTE: These flags are not in alphabetical order. There are more of them than
	you think. Keep looking until you see a comment saying the set is over. */

/* ----< This is the beginning of the CHFLAGs >---- */

#define	ICON_FLAGS	DESIGN_FLAGS

/* icon flags */
#define CHFLAG_LAUNCH_APP			'a'
#define CHFLAG_MAILBOX_TYPE			'A'
#define CHFLAG_ADDRESS_BOOK_TYPE	'b'
#define CHFLAG_LIGHT_ADDRESS_BOOK_TYPE	'B'
#define CHFLAG_SHOW_CHANGED_POLICY	'c'
#define CHFLAG_LAUNCH_DOCLINK		'd'
#define CHFLAG_LAUNCH_FRAMESET		'F'
#define CHFLAG_RESTRICT_STORED_FORMS 'f'
#define CHFLAG_LIBRARY_TYPE			'g'
#define CHFLAG_DEFERRED_LOADED_IMAGES 'i'
#define CHFLAG_PERSONALJOURNAL_TYPE	'j'
#define CHFLAG_LAUNCH_NAVIGATOR		'l'
#define CHFLAG_SEARCH_SCOPE_TYPE	'm'
#define CHFLAG_NEVER_SHOW_POLICY	'n'
#define CHFLAG_LAUNCH_OUTLINE		'o'	/* Launch Outline as the navigator */
#define CHFLAG_ALWAYS_SHOW_POLICY	'p'
#define CHFLAG_MAILFILE_TYPE		'r'
#define CHFLAG_SOLO_NAVIGATOR		's'
#define CHFLAG_OBSOLETE_DONTUSE		't'	
#define CHFLAG_SITEDB_TYPE			'u'
										/* 'refresh' flag from build 130-133 */
										/* has been superceded by a flag in */
										/* ICON_FLAGS_NOREFRESH, below.  */
										/* don't use this one anymore... */
#define CHFLAG_NO_SHOW_VIEW			'v'

#define CHFLAG_LAUNCH_WEBPAGE		'W'	/* Launch navigator of type Web Page */
#define CHFLAG_PANE_BIGFOLDER		'w'	/* preview pane orientation; mutually exclusive with below */
#define CHFLAG_PANE_SMALLVIEW		'x' /* preview pane orientation; mutually exclusive with above*/
#define CHFLAG_PANE_MAXIMIZED		'y'	/* preview pane size */

/* web-specific launch flags */
#define CHFLAG_WEB_LAUNCH_DOCLINK	'D'
#define CHFLAG_WEB_LAUNCH_NAVIGATOR	'L'
#define CHFLAG_WEB_ALWAYS_SHOW_POLICY 'P'
#define CHFLAG_WEB_LAUNCH_THISDOCLINK 'T'
#define CHFLAG_WEB_LAUNCH_VIEWNAME	'V'
#define CHFLAG_WEB_USEJAVASCRIPT    'J'
#define CHFLAG_WEB_USESSLCONN		'X'
#define CHFLAG_WEB_LAUNCH_PAGE		'E'
#define CHFLAG_WEB_LAUNCH_FRAMESET	'S'
#define CHFLAG_RESTORE_VIEW			'K'

#define CHFLAG_MULTILINGUAL_DB	'M'
#define CHFLAG_ALLOW_DECS_CONNECTIONS	'C'
										
#define CHFLAG_NOUNREAD_MARKS			'z'
#define CHFLAG_DISABLE_RESPONSE_INFO	'1'
#define CHFLAG_FORM_BUCKET_OPT			'2'
#define CHFLAG_MAINTAIN_LAST_ACCESSED	'3'
#define CHFLAG_DELETES_ARE_SOFT			'4'
		
#define CHFLAG_IS_LOCK_DB				'5'
#define CHFLAG_IS_DESIGN_LOCK_DB		'6'		
#define CHFLAG_LARGE_UNKTABLE			'7'
#define CHFLAG_HIDE_FROM_WEB			'8'
#define CHFLAG_IS_WEBAPPLICATION		'9'

#define CHFLAG_REFRESH_ON_ADMIN			'e'

#define CHFLAG_COPY_PROFILES			'G'	/*	For templates - copy profile notes on creating database from this one 
												and when refreshing databases from it */
#define CHFLAG_LZ1						'Z'	/* LZ1 compression db option. */
#define CHFLAG_REPL_UNREAD_CLUSTERED		'Q'
#define CHFLAG_REPL_UNREAD_EVERYWHERE	'U'	/* Icon flag for enabling replication of unread marks - on AND off cluster */
#define CHFLAG_TRACK_REPLYFORWARD		'h'	/* inherited track reply/forward flag */

/* ----< This is the end of the CHFLAGs >---- */

/* 	unlike the $Flags field, this field doesn't get
	refreshed/replaced on a design refresh/replace 
	and they don't inherit from parent templates
*/
#define ICON_FLAGS_NOREFRESH	"$FlagsNoRefresh"

#define CHFLAG_NR_ADV_TEMPLATE			't'
#define CHFLAG_NR_SINGLE_COPY_TEMPLATE	's'

#define CHFLAG_NR_DONTTRACK_REPLYFORWARD	'r'	/* need separate flags here because */
#define CHFLAG_NR_DOTRACK_REPLYFORWARD	'f'		/* we need to know if user turned off OR on to override template setting */
#define CHFLAG_NR_REPLUNREAD_NEVER		'v'
#define CHFLAG_NR_REPLUNREAD_CLUSTERED	'Q'
#define CHFLAG_NR_REPLUNREAD_EVERYWHERE 'U'

/* Special replication flags */
 
#define REPLFLAGS_ITEM 	"$ReplFlags"
#define REPL_FLAG_NOREPL	'n'

/* Auto-merge replication conflicts */

#define ITEM_CONFLICT_ACTION 	"$ConflictAction"	/* enable auto-merge conflicts if 1 */
#define 	CONFLICT_AUTOMERGE	'1'
#define 	CONFLICT_NONE		'2'
#define		CONFLICT_BEST_MERGE	'3'

#define ITEM_CONFLICT_ITEMS 	"$ConflictItems"

/* Form note item names */

#define	ITEM_NAME_TEMPLATE "$Body"			/* form item to hold form CD */
#define	ITEM_NAME_DOCUMENT "$Info"			/* document header info */
#define	ITEM_NAME_TEMPLATE_NAME FIELD_TITLE	/* form title item */
#define	ITEM_NAME_FORMLINK "$FormLinks"		/* form link table */
#define	ITEM_NAME_FIELDS "$Fields"			/* field name table */
#define ITEM_NAME_FORMPRIVS "$FormPrivs"	/* form privileges */
#define ITEM_NAME_FORMUSERS "$FormUsers"	/* text list of users allowed to use the form */
#define ITEM_NAME_FRAMESET "$FrameSet"		/* form item to hold form Frameset definition */
#define ITEM_NAME_FRAMEINFO	"$FrameInfo"	/* frameset used to open form */

/* Special form flags */						
 
#define ITEM_NAME_KEEP_PRIVATE "$KeepPrivate"	
#define PRIVATE_FLAG_YES	'1'				/* $KeepPrivate = TRUE  force disabling of printing, mail forwarding and edit copy */
#define PRIVATE_FLAG_YES_RESEND	'2'			/* $KeepPrivate = TRUE  same as PRIVATE_FLAG_YES except allow resend */

#define ITEM_NAME_DATEPICKER_MONTH "$DatePickerMonth"	
#define ITEM_NAME_DATEPICKER_DAY "$DatePickerDay"	
#define ITEM_NAME_DATEPICKER_YEAR "$DatePickerYear"	
#define ITEM_NAME_HEADERAREA "$HeaderArea"	
#define ITEM_NAME_FOOTERAREA "$FooterArea"	
#define ITEM_NAME_REGIONFRAMESET "$RegionFrameset"	
#define ITEM_NAME_BACKGROUNDGRAPHIC "$Background"	
#define ITEM_NAME_BACKGROUNDGRAPHICR5 "$BackgroundR5"	
#define ITEM_NAME_BACKGROUNDGRAPHICBODY "$BackgroundBody"	
#define ITEM_NAME_PAPERCOLOR "$PaperColor"	
#define ITEM_NAME_PAPERCOLOREX "$PaperColorEx"
#define ITEM_NAME_LAYER "$Layer"	
#define ITEM_NAME_LAYERTEXT "$LayerText"	
#define ITEM_NAME_RESTRICTBKOVERRIDE "$NoBackgroundOverride" 
#define RESTRICTBK_FLAG_NOOVERRIDE '1'		/* $NoBackgroundOverride = TRUE Don't allow user to override document background */
#define	ITEM_NAME_HTMLCODE "$HTMLCode"
#define	ITEM_NAME_HTMLCODE_CLIENT "$HTMLCodeClient"
#define ITEM_NAME_DAVNAME "$DAVName"

#define ITEM_NAME_AUTO_EDIT_NOTE "$AutoEditMode"	
#define AUTO_EDIT_FLAG_YES	'1'				/* $AutoEditMode = TRUE  force edit mode on open regardless of Form flag */

#define ITEM_NAME_SHOW_NAVIGATIONBAR "$ShowNavigationBar"  /* Display the URL navigation Bar */
#define ITEM_NAME_HIDE_SCROLL_BARS 	"$HideScrollBars"
#define WINDOW_SCROLL_BARS_NONE 	'1'
#define WINDOW_SCROLL_BARS_HORZ 	'2'
#define WINDOW_SCROLL_BARS_VERT 	'3'

#define ITEM_NAME_BACKGROUNDGRAPHIC_REPEAT "$BackgroundRepeat"
#define ITEM_NAME_BACKGROUNDGRAPHIC_REPEATBODY "$BackgroundRepeatBody"
#define ITEM_NAME_USER_BACKGROUNDGRAPHIC_REPEAT "$UserBackgroundRepeat"
#define ITEM_NAME_USER_BACKGROUNDGRAPHIC_REPEATBODY "$UserBackgroundRepeatBody"
#define BACKGROUNDGRAPHIC_REPEAT_ONCE	'1'
#define BACKGROUNDGRAPHIC_REPEAT_VERT	'2'
#define BACKGROUNDGRAPHIC_REPEAT_HORIZ	'3'
#define BACKGROUNDGRAPHIC_REPEAT_BOTH	'4'
#define BACKGROUNDGRAPHIC_REPEAT_SIZE	'5'
#define BACKGROUNDGRAPHIC_REPEAT_CENTER	'6'

#define ITEM_NAME_HIDE_GRAPHIC_IN_EDIT_MODE "$HideGraphicInEditMode"


#define ITEM_NAME_RFSAVEINFO	"$RFSaveInfo"



#define ITEM_NAME_VERSION_OPT "$VersionOpt"	/* Over-ride the Form flags for versioning. */
#define VERSION_FLAG_NONE	'0'				/* $Version = 0, None */
#define VERSION_FLAG_MURESP	'1'				/* $Version = 1, Manual - Update becomes response */
#define VERSION_FLAG_AURESP	'2'				/* $Version = 2, Auto   - Update becomes response */
#define VERSION_FLAG_MUPAR	'3'				/* $Version = 3, Manual - Update becomes parent */
#define VERSION_FLAG_AUPAR	'4'				/* $Version = 4, Auto   - Update becomes parent */
#define VERSION_FLAG_MUSIB	'5'				/* $Version = 5, Manual - Update becomes sibling */
#define VERSION_FLAG_AUSIB	'6'				/* $Version = 6, Auto   - Update becomes sibling */


/* Document note item names */

#define	ITEM_NAME_TEMPLATE_USED FIELD_FORM	/* form name used to create note, user-visible */
#define ITEM_NAME_NOTEREF FIELD_LINK		/* optional reference to another note */
#define ITEM_NAME_VERREF "$VERREF"			/* optional reference to master version note */
#define	ITEM_NAME_LINK "$Links"				/* note link table */
#define ITEM_NAME_REVISIONS "$Revisions"	/* Revision history */
#define ITEM_NAME_CONFLICT VIEW_CONFLICT_ITEM /* Replication update conflict */
#define ITEM_NAME_AUTHORS "$Authors"		/* text list of users allowed to modify document below editor access */
#define ITEM_NAME_WRITERS "$Writers"		/* text list of users allowed to modify document */
#define ITEM_NAME_PROVISIONAL_WRITERS "$PWriters"	/* text list of users provisionally allowed to modify document */
#define ITEM_NAME_TEMP_PROVISIONAL_WRITERS "$PTWriters"	/* text list of users provisionally allowed to temporarily modify document */
#define ITEM_NAME_WRITERSDATE "$WritersDate"	/* timedate of when any form of $[X]Writers was taken out */

/* Document and form note item names, all items are optional */

#define	ITEM_NAME_FONTS "$Fonts"			/* font table */
#define ITEM_NAME_HEADER "$Header"			/* print page header */
#define ITEM_NAME_FOOTER "$Footer"			/* print page footer */
#define ITEM_NAME_HFFLAGS "$HFFlags"		/* header/footer flags */
	#define	HFFLAGS_NOPRINTONFIRSTPAGE	'1'		/* suppress printing header/footer on first page */
	#define	HFFLAGS_DIRECTION_RTL	'R'		/* header/footer is RTL */
#define ITEM_NAME_WINDOWTITLE "$WindowTitle"/* window title */
#define	ITEM_NAME_ATTACHMENT "$FILE" 		/* file attachment, MUST STAY UPPER-CASE BECAUSE IT'S SIGNED! */
#define ITEM_NAME_HTMLHEADTAG "$HTMLHeadTag"/* Override for HTML head tag */
#define ITEM_NAME_HTMLBODYTAG "$HTMLBodyTag"/* Override for HTML body tag */
#define	ITEM_NAME_WEBQUERYSAVE "$WEBQuerySave" /*WebQuerySave formula */
#define	ITEM_NAME_WEBQUERYOPEN "$WEBQueryOpen" /*WebQueryOpen formula */
#define ITEM_NAME_APPHELPFORMULA "$AppHelpFormula" /* Help doc to load */
#define ITEM_NAME_STYLESHEETLIST "$StyleSheetList"/* Style Sheet List */
#define ITEM_NAME_DEFAULTDECSINFO "$DefaultDECSInfo" /* default DECS info for all form fields */
#define ITEM_NAME_EXTERNAL_FIELDS "$ExternalFields" /* # of DECS fields in form */
#define ITEM_NAME_CLIENTEVENTINFO "$ClientEventTag"
#define ITEM_NAME_WEBEVENTINFO "$WebEventTag"

#define ITEM_NAME_WEBFLAGS	"$WebFlags"		/* Web related flags for form or document */
#define WEBFLAG_NOTE_USEAPPLET_INBROWSER	'B'		/* use appropriate applet when serving to browser */
#define WEBFLAG_NOTE_IS_HTML		'H'		/* treat this document or form as plain HTML, do not convert styled text to HTML */
#define WEBFLAG_NOTE_CONTAINS_VIEW	'V'		/* optimization for web server: this note contains an embedded view */
#define WEBFLAG_NOTE_HTML_ALL_FLDS	'F'		/* gen'd HTML for all fields */
#define WEBFLAG_NOTE_CONTAINS_JSBUTTON	'J'	/* Generate <FORM> tag */
#define WEBFLAG_NOTE_ALLOW_DOC_SELECTIONS 'S'

/*	THESE NEXT TWO FLAGS ARE OBSOLETE - SHOULD BE REMOVED ASAP */
#define WEBFLAG_NOTE_FILE_DEPLOYABLE	'D'	/* for web apps, this file is ready for primetime */
#define WEBFLAG_NOTE_DONTREFRESH_ON_REDEPLOY	'R' /* for web apps, this file should not be replaced on redeploy */
#define ITEM_NAME_WEBPUBLISHING	"$WebPublishing" 
#define	ITEM_NAME_TEXTPROPERTIES "$TextProperties"			/* Text Properties table */
#define ITEM_NAME_DAVPROPERTIES "$DavProps"			/* WebDAV dead properties */
#define ITEM_NAME_DAVNAMESPACES "$DavNamespaces"	/* WebDAV namespaces for dead properties */



/* Document note Sign/Seal item names */

#define	ITEM_NAME_NOTE_SIGNATURE "$Signature"	
#define	ITEM_NAME_NOTE_SIG_PREFIX "$Sig_"	/*	Prefix for multiple signatures. */
#define	ITEM_NAME_NOTE_STOREDFORM_SIG "$SIG$Form" /* stored form signature */
#define	ITEM_NAME_NOTE_STOREDSUBFORM_SIG_PREFIX "$SIG$" /* stored subform signature prefix - followed by subform name*/
#define	ITEM_NAME_NOTE_STOREDFORM_SIG_PREFIX "$SIG" /* stored form and subform signature prefix - followed by either $FORM or the subform name*/
#define	ITEM_NAME_NOTE_SEAL "$Seal"
#define	ITEM_NAME_NOTE_SEALDATA "$SealData"
#define ITEM_NAME_NOTE_SEALNAMES "SecretEncryptionKeys"
#define ITEM_NAME_NOTE_SEALUSERS "PublicEncryptionKeys"
#define	ITEM_NAME_NOTE_FORCESIGN "Sign"
#define	ITEM_NAME_NOTE_FORCESEAL "Encrypt"
#define	ITEM_NAME_NOTE_FORCEMAIL "MailOptions"
#define	ITEM_NAME_NOTE_FORCESAVE "SaveOptions"
#define	ITEM_NAME_NOTE_FORCESEALSAVED "EncryptSaved"
#define	ITEM_NAME_NOTE_MAILSAVE	"MailSaveOptions"
#define	ITEM_NAME_NOTE_FOLDERADD "FolderOptions"
/* also see EncryptionStatus and SignatureStatus below */

/* Item name suffixes for design note items that are add to data note when "store form with document" is used. */

#define STOREDFORM_ITEM_SUFFIX		"_StoredForm"
#define STOREDSUBFORM_ITEM_SUFFIX	"_StoredSubform" /* A number 1 - Number of subforms will also be append... _StoredSubform1 */
#define ITEM_NAME_STOREDFORM_CRC	"$StoredFormCRC"	/* A CRC on the first $Body item of a stored form in doc document. Used to detect down stream client changes. */

#define ITEM_NAME_STOREDFORM_REPID	"$Form_RepId_SF" /* Replica id of a form's originating database. */

/* 
 * This item name (which is '$Body_StoredForm') is the "new" way of storing a form with a document. This item
 * and all items with the _StoredForm and/or _StoredSubformX suffix supercede the the original item names
 * in documents that have stored forms unless a downstream client... pre 5.0.6... makes a change as detected 
 * with $StoredFormCRC as mentioned above.
 */
#define	ITEM_NAME_TEMPLATE_STOREDFORM  ITEM_NAME_TEMPLATE STOREDFORM_ITEM_SUFFIX

/* Group expansion item and legal values */

#define	ITEM_NAME_NOTE_GROUPEXP	 "ExpandPersonalGroups"	/* For backward compatibility */
#define ITEM_NAME_NOTE_EXPANDGROUPS	 "$ExpandGroups"
#define MAIL_DONT_EXPAND_GROUPS			'0'
#define MAIL_EXPAND_LOCAL_GROUPS		'1'
#define MAIL_EXPAND_PUBLIC_GROUPS		'2'
#define MAIL_EXPAND_LOCAL_AND_PUBLIC_GROUPS	'3'

/* Search term highlights item name prefix.  An item name is 
	concatenated to this; e.g. $Highlights_Body.  */

#define ITEM_NAME_HIGHLIGHTS	"$Highlights_"

/* Import/Export document item names */

#define IMPORT_BODY_ITEM_NAME "Body"
#define IMPORT_FORM_ITEM_NAME FIELD_FORM
#define NEW_FORM_ITEM_NAME FIELD_FORM

/*	Help index note item names */

#define ITEM_OBJECT	"$Section"
#define	HELP_VIEW_NAME "(All)"
#define APP_HELP_DEFAULT_VIEW_NAME "(Help)"

/*	Help view names */

#define HELP_VIEW_TOC "Top10"		/*	Table of contents view */
#define HELP_VIEW_MSG "MSG"		/*	Messages view */
#define HELP_VIEW_ATFUNC "REF"	/*	@Functions view */
#define HELP_VIEW_INDEX "IDX"	/*	Index view */
#define HELP_VIEW_NEWS "NEWS" 	/* Release Notes view */
#define HELP_VIEW_V3CMD "CLASSIC" 	/* V3 Menu Command Equivalents */

/* Help navigator names */

#define HELP_NAV_HOW "Help Topics - How"
#define HELP_NAV_INDEX "Help Topics - Index"
#define HELP_NAV_MSG "Help Topics - Trouble"
#define HELP_NAV_ATFUNC "Help Topics - Tell Me"
#define HELP_NAV_KBD	"Help Topics - Tell Me"

/*	Notes Help Template String */
#define HELP_TEMPLATE_STRING	"NotesHelp"

/*	Catalog form names */

#define NOTEFILE_FORM_NAME			"Notefile"
#define CONNECTION_FORM_NAME		"Connection"

/*	Catalog view names */
#define	CATALOG_VIEW_REPLICA_ID "Databases by _Replica ID"
#define CATALOG_VIEW_CATEGORY	"Databases by _Category"
#define CATALOG_VIEW_MULTIDBBIT	"($MultiDbIndex)"

/*	Catalog document note item names */

#define CATALOG_ITEM_SERVER		 	"Server"
#define CATALOG_ITEM_REMOTE_SERVER 	"RemoteServer"
#define CATALOG_ITEM_PATHNAME 		"Pathname"
#define CATALOG_ITEM_TITLE  		"Title"
#define CATALOG_ITEM_REPLICAID  	"ReplicaID"
#define CATALOG_ITEM_POLICY_DATE 	"PolicyDate"
#define CATALOG_ITEM_LICENSEID		"LicenseID"
#define CATALOG_ITEM_MANAGERS		"Managers"
#define CATALOG_ITEM_BODY			"Body"
#define CATALOG_ITEM_VIEW_INDEX		"View"
#define CATALOG_ITEM_CATEGORY1		"Category1"
#define CATALOG_ITEM_CATEGORY2		"Category2"
#define CATALOG_ITEM_CATEGORY3		"Category3"
#define CATALOG_ITEM_CATEGORY4		"Category4"
#define CATALOG_ITEM_CATEGORY5		"Category5"
#define CATALOG_ITEM_CAT			MAIL_CATEGORIES_ITEM
#define CATALOG_ITEM_FULLNAME		"FullName"
#define CATALOG_ITEM_META_TITLE			"Title"
#define CATALOG_ITEM_META_CREATOR		"Creator"
#define CATALOG_ITEM_META_DESCRIPTION	"Description"
#define CATALOG_ITEM_META_TYPE			"MetaType"
#define CATALOG_ITEM_META_IDENTIFIER	"URL"
#define CATALOG_ITEM_META_CREATION_DATE	"CreationDate"
#define CATALOG_ITEM_META_CATEGORIES	"Categories"
#define CATALOG_ITEM_ADMIN_SERVER		"DbAdminServer"
#define CATALOG_ITEM_ADMIN_SERVER_NAMES	"DbAdminServerNames"
/* for agents */
#define CATALOG_ITEM_AGENT_AGENTNAME	"AgentName"				
#define CATALOG_ITEM_AGENT_DB			"AgentDb"				
#define CATALOG_ITEM_AGENT_SIGNER		"AgentSigner"				
#define CATALOG_ITEM_AGENT_ONBEHALF		"AgentOnBehalf"
#define CATALOG_ITEM_AGENT_INTERVALTYPE "AgentIntervalType"
#define CATALOG_ITEM_AGENT_INTERVAL		"AgentInterval"
#define CATALOG_ITEM_AGENT_TIME1		"AgentTimeOne"
#define CATALOG_ITEM_AGENT_TIME2		"AgentTimeTwo"
#define CATALOG_ITEM_AGENT_STARTTIME	"AgentStartTime"
#define CATALOG_ITEM_AGENT_ENDTIME		"AgentEndTime"
#define CATALOG_ITEM_AGENT_NOWEEKEND	"AgentNoWeekend"
#define CATALOG_ITEM_AGENT_PERSONAL		"AgentPersonal"
#define CATALOG_ITEM_AGENT_COUNT		"AgentCount"
#define CATALOG_ITEM_AGENT_FIELDCOUNT	"AgentFieldCount"
#define CATALOG_ITEM_AGENT_TRIGGERTYPE	"AgentTriggerType"
#define CATALOG_ITEM_AGENT_SEARCHTYPE	"AgentSearchType"
#define	CATALOG_ITEM_AGENT_SERVER		"AgentServer"
#define	CATALOG_ITEM_AGENT_LASTRUN		"AgentLastRunTime"
#define	CATALOG_ITEM_AGENT_DOCPROCESSED	"AgentDocsProcessed"
#define	CATALOG_ITEM_AGENT_ENABLED		"AgentEnabled"
#define	CATALOG_ITEM_AGENT_ACTIVATABLE	"AgentActivatable"
#define	CATALOG_ITEM_AGENT_RUNASWEB		"AgentRunAsWebUser"
#define	CATALOG_ITEM_AGENT_RESTRICTED	"AgentRestricted"
#define	CATALOG_ITEM_AGENT_LANGUAGE		"AgentLanguage"



/*	Multi DB searching document forms, item names, and item values */

#define MULTIDB_FORM_CONFIG			"ConfigurationForm"
#define MULTIDB_FORM_ENTRY			"Database"
#define MULTIDB_ITEM_ACL			"ACL"
#define MULTIDB_ITEM_CATEGORIES		"DatabaseCategories"
#define MULTIDB_ITEM_CONFIGLINK		"ConfigLink"
#define MULTIDB_ITEM_DOMAIN			"Domain"
#define MULTIDB_ITEM_INDEXOPTIONS	"IndexOptions"
#define MULTIDB_ITEM_PATHNAME		"Pathname"
#define MULTIDB_ITEM_REPLICAID		"ReplicaID"
#define MULTIDB_ITEM_SCOPE			"Scope"
#define MULTIDB_ITEM_SERVER			"Server"
#define MULTIDB_ITEM_TITLE			"Title"
#define MULTIDB_ITEM_VIEWCHOICE		"DbViews"
#define MULTIDB_ITEM_VIEWLIST		"DatabaseViews"
#define MULTIDB_INDEX_NONE			"0"
#define MULTIDB_INDEX_SUMMARY		"1"
#define MULTIDB_INDEX_FULLNOTE		"2"
#define MULTIDB_INDEX_ATTACHMENT	"3"
#define MULTIDB_SCOPE_DATABASE		"0"
#define MULTIDB_SCOPE_DIRECTORY		"1"
#define MULTIDB_SCOPE_SERVER		"2"
#define MULTIDB_SCOPE_DOMAIN		"3"
#define	MULTIDB_VIEW_REPLICAID 		"$ReplicaID"

/*	Library document note item names */

#define LIBRARY_ITEM_AUTHOR			"Author"
#define LIBRARY_ITEM_SERVER		 	"Server"
#define LIBRARY_ITEM_PATHNAME 		"Pathname"
#define LIBRARY_ITEM_TITLE  		"Title"
#define LIBRARY_ITEM_REPLICAID  	"ReplicaID"
#define LIBRARY_ITEM_ABSTRACT		"Abstract"
#define LIBRARY_PATHNAME			"LibraryPathName"
#define LIBRARY_TITLE				"LibraryTitle"
#define LIBRARY_SENDTO				"SendTo"
#define LIBRARY_LIBRARIAN			"Librarian"

/* Library Form Names */
#define LIBRARIAN_FORM_NAME			"Librarian"

/* Library View Names */
#define LIBRARY_REPID_VIEW		   	"($RepID)"	
#define LIBRARY_LIBRARIAN_VIEW		"($Librarians)"	

/* Disk directory search item names and values */

#define DBDIR_PATH_ITEM 		"$Path"		 		/* Path name */
#define DBDIR_TYPE_ITEM 		"$Type"				/* Type item */
#define DBDIR_TYPE_ITEM_DIRECTORY "$DIR"			/* Directory type name */
#define DBDIR_TYPE_ITEM_NOTEFILE "$NOTEFILE"		/* Notefile type name */
#define DBDIR_TYPE_ITEM_OLDNOTEFILE "$OLDNOTEFILE"
#define DBDIR_INFO_ITEM 		"$Info"				/* Info item */
#define DBDIR_LENGTH_ITEM		"$Length"
#define DBDIR_MODIFIED_ITEM		"$Modified"
#define DBDIR_PROPERTIES_ITEM 	"$Properties"
#define DBDIR_LINK_ITEM 		"$LinkInfo"
#define DBDIR_DIRECTORY_LINK 	"$DIRLINK"			/* Directory Link */
#define DBDIR_DATABASE_LINK 	"$DBLINK"			/* Database Link */
#define DBDIR_DBOPTIONS_ITEM 	"$DBOPTIONS"		/* Database Options */
#define DBDIR_DBVERSIONS_ITEM 	"$DBVERSIONS"		/* Database Major and Minor versions */
#define DBDIR_DBCREATED_ITEM 	"$DBCREATED"		/* Database Created date	*/
#define DBDIR_LASTFIXUP_ITEM	"$LASTFIXUP"		/* Last database fixup time	*/
#define DBDIR_QUOTALIMIT_ITEM	"$QUOTALIMIT"		/* Database quota limit	*/
#define DBDIR_QUOTAWARNING_ITEM	"$QUOTAWARNING"		/* Database quota warning	*/
#define DBDIR_MAXDBSIZE_ITEM	"$MAXDBSIZE"		/* Database MAximum Size - 1Gb, 2GB, 3GB, 4GB	*/
#define DBDIR_PHYSICALPATH_ITEM	"$PHYSICALPATH"		/* Physical file path (OS level path)	*/
#define DBDIR_ISLOGGED_ITEM		"$ISLOGGED"			/* Is the db logged? */
#define DBDIR_LENGTH2_ITEM		"$Length2"			/* File Size as NSFDISKPOS */
#define DBDIR_DBOPTIONS2_ITEM 	"$DBOPTIONS2"		/* Database Options[1] */
#define DBDIR_DBOPTIONS3_ITEM 	"$DBOPTIONS3"		/* Database Options[2] */
#define DBDIR_DBOPTIONS4_ITEM 	"$DBOPTIONS4"		/* Database Options[3] */
#define DBDIR_DATAMODIFIED_ITEM "$DATAMOD"			/* Data Modified time */
#define DBDIR_NONDATAMODIFIED_ITEM "$NONDATAMOD"	/* Non Data Modified time */
#define DBDIR_CURRENTUSAGE_ITEM	"$CURRENTUSAGE"		/* CurrentSpace Utilization	*/
#define DBDIR_STORAGEPATH_ITEM  "$STORAGEPATH"		/* Raw file path (all links translated) */
#define DBDIR_REPLFLAGS_ITEM	"$ReplFlags"		/* Replica Flags */
#define DBDIR_DB2_DATABASE_LINK "$DB2LINK"			/* DB2 Database Link */

/* Form names for log database. */

#define	LOG_EVENT_FORM 		"Events"
#define	LOG_REPLICA_FORM 	"Replication"
#define	LOG_MAIL_FORM 		"Mail Routing"
#define	LOG_SESSION_FORM 	"Session"
#define LOG_USAGE_FORM		"Activity"
#define LOG_SIZE_FORM		"Size"
#define LOG_PHONE_FORM		"Phone Call"
#define LOG_PHONE_IN_FORM	"Phone Call - Incoming"
#define LOG_PHONE_OUT_FORM	"Phone Call - Outgoing"
#define	LOG_PT_FORM 		"Passthru Connection"
#define	LOG_OBJSTORE_USAGE_FORM 	"ObjStoreUsageForm"
/* This is the form name used by the log package "activity logging API" */
#define LOG_ACTIVITY_FORM	"ActivityData"

/* View names for log database. */

#define	LOG_OBJSTORE_USAGE_VIEW 		"ObjStoreUsageView"
#define	LOG_MAIL_ROUTING_EVENTS_VIEW	"MailRoutingEvents"
#define LOG_ACTIVITY_DATA_VIEW			"($ActivityData)"
#define LOG_ACTIVITY_SCHEMA_VIEW		"($ActivitySchema)"

/*	Item names used by the log facility itself or commonly used
	by different packages that call the log facility. */

#define LOG_ITEM_STARTTIME			"StartTime"
#define LOG_ITEM_STARTTIME_TEXT		"StartTimeText"
#define LOG_ITEM_ENDTIME			"FinishTime"
#define LOG_ITEM_BODY				"Body"
#define LOG_ITEM_SERVER 			"Server"
#define LOG_ITEM_PORT				"Port"
#define LOG_ITEM_EVENTS				"Events"
#define LOG_ITEM_EVENT_LIST			"EventList"
#define LOG_ITEM_EVENT_STATUS		"EventStatus"
#define LOG_ITEM_EVENT_TYPE			"EventType"
#define LOG_ITEM_EVENT_SEV			"EventSeverity"
#define LOG_ITEM_EVENT_TIME			"EventTime"
#define LOG_ITEM_EVENT_TIME_SIZE	"EventTimeSize"
#define LOG_ITEM_EVENT_ADDERR		"EventAdditionalErrorCode"
#define LOG_ITEM_EVENT_ADDERR_TYPE	"EventAdditionalErrorType"
#define LOG_ITEM_EVENT_ADDERR_SEV	"EventAdditionalErrorSeverity"
#define LOG_ITEM_EVENT_ADDIN		"EventAddinName"
#define LOG_ITEM_EVENT_PARAMETERS	"EventParameters"
#define LOG_ITEM_PATHNAME			"Pathname"
#define LOG_ITEM_TITLE				"Title"
#define LOG_ITEM_CAT				MAIL_CATEGORIES_ITEM
#define LOG_ITEM_BYTESIN			"BytesIn"
#define LOG_ITEM_BYTESOUT			"BytesOut"

/* Item names for replicator notes in log file. */

#define REPLICA_ITEM_SOURCE_SERVER	"SourceServer"
#define REPLICA_ITEM_INITIATED_BY	"InitiatedBy"

/* Item names for session notes in log file. */

#define SESSION_ITEM_USERNAME		"UserName"
#define SESSION_ITEM_READS			"Reads"
#define SESSION_ITEM_WRITES			"Writes"
#define SESSION_ITEM_SENT_FROM		"SentFromServer"
#define SESSION_ITEM_SENT_TO		"SentToServer"
#define SESSION_ITEM_UNCOMPSENT_FROM "SentFromServerUncompressed"
#define SESSION_ITEM_UNCOMP_SENT_TO	"SentToServerUncompressed"
#define SESSION_ITEM_COMPSENT_FROM "SentFromServerCompressed"
#define SESSION_ITEM_COMP_SENT_TO	"SentToServerCompressed"

#define SESSION_ITEM_TRANSACTIONS	"Transactions"

/* Item names for escrow log */

#define	ESCROW_SENDTO				"Escrow Agent"

#define	ESCROW_USER					"User"
#define	ESCROW_SERVER				"Server"
#define	ESCROW_CERTIFIER			"Certifier"

#define	ESCROW_LOCATION_ITEM		"Location"
#define	ESCROW_COMMENT_ITEM			"Comment"
	
/* Item names for database activity note in log file */

#define ACTIVITY_ITEM_MANAGERS		"Managers"
#define ACTIVITY_ITEM_VIEWTITLES	"ViewTitles"	/* Obsolete in R6 */
#define ACTIVITY_ITEM_VIEWSIZES		"ViewSizes"		/* Obsolete in R6 */
#define ACTIVITY_ITEM_ALLVIEWINFO	"AllViewInfo"	/* New in R6 */
#define ACTIVITY_ITEM_DISKSPACE		"DiskSpace"
#define ACTIVITY_ITEM_DISKUSAGE		"DiskUsage"
#define ACTIVITY_ITEM_PERIOD		"Period"
#define ACTIVITY_ITEM_PERIOD_USES	"PeriodUses"
#define ACTIVITY_ITEM_PERIOD_READS	"PeriodReads"
#define ACTIVITY_ITEM_PERIOD_WRITES	"PeriodWrites"
#define ACTIVITY_ITEM_DAY_USES		"DayUses"
#define ACTIVITY_ITEM_DAY_READS		"DayReads"
#define ACTIVITY_ITEM_DAY_WRITES	"DayWrites"
#define ACTIVITY_ITEM_WEEK_USES		"WeekUses"
#define ACTIVITY_ITEM_WEEK_READS	"WeekReads"
#define ACTIVITY_ITEM_WEEK_WRITES	"WeekWrites"
#define ACTIVITY_ITEM_MONTH_USES	"MonthUses"
#define ACTIVITY_ITEM_MONTH_READS	"MonthReads"
#define ACTIVITY_ITEM_MONTH_WRITES	"MonthWrites"

/* Items associated with server activity in the log */

#define SRVACT_ITEM_ACTIVITY_NAME	"ActivityName"
#define SRVACT_ITEM_ACTIVITY_FIELDS	"ActivityFields"
#define SRVACT_ITEM_ACTIVITY_TYPES	"ActivityTypes"
#define SRVACT_ITEM_ACTIVITY_SCHEMA	"ActivitySchema"


/* Item names for unet notes in log file. */
#define UNIXNET_ITEM_BODY	"UnixBody"

#if 0

/* Address Book - "X400Server" form item names */

#define MAIL_X400_SERVER_FORM "Server"				/* Form and type name */
#define MAIL_X400_SERVERNAME_ITEM "ServerName"		/* X400 MTA name */
#define MAIL_X400_GLOBAL_DOMAIN_ITEM "GlobalDomain" /* Global domain identifier */ 
#define MAIL_X400_COUNTRY_ITEM "Country"			/* Country name */
#define MAIL_X400_ADMD_ITEM "ADMD"					/* Administration Management Domain name*/
#define MAIL_X400_PRMD_ITEM "Domain"				/* Private Domain name */
#define MAIL_X400_CREDENTIALS_ITEM "Credentials"	/* MTA credentials */
#define MAIL_X400_SUPP_APP_CONTEXTS_ITEM "SupportedApplicationsContext"	/* Supported app. contexts */
#define MAIL_X400_DEL_CONTENT_LENGTH_ITEM "DeliveryContentLength" 	/* Delivery content length */
#define MAIL_X400_DEL_CONTENT_TYPE_ITEM "DeliveryContentTypes"	 	/* Delivery content types */
#define MAIL_X400_PSEL_ITEM "PSEL"					/* Presentation selector */
#define MAIL_X400_SSEL_ITEM "SSEL"					/* Session selector */
#define MAIL_X400_TSEL_ITEM "TSEL"					/* Transport selector */
#define MAIL_X400_NADDRESSES_ITEM "NAddresses"		/* Network addresses */

#endif



/*	Public Queue Names */

#define	TASK_QUEUE_PREFIX	"MQ$"			/* Prepended to "addin" task */
											/* name to form task's queue name */

#define SERVER_QUEUE_NAME	"_SERVER"						/* DB Server */
#define REPL_QUEUE_NAME		TASK_QUEUE_PREFIX"REPLICATOR"	/* Replicator */
#define ROUTER_QUEUE_NAME	TASK_QUEUE_PREFIX"ROUTER"		/* Mail Router */
#define UPDATE_QUEUE_NAME 	TASK_QUEUE_PREFIX"INDEXER"		/* Index views & full text process */
#define LOGIN_QUEUE_NAME 	TASK_QUEUE_PREFIX"LOGIN"		/* Login Process */
#define EVENT_QUEUE_NAME	TASK_QUEUE_PREFIX"EVENT"		/* Event process */
#define REPORT_QUEUE_NAME	TASK_QUEUE_PREFIX"REPORTER"		/* Report process */
#define CLREPL_QUEUE_NAME	TASK_QUEUE_PREFIX"CLREPL"		/* Cluster Replicator */
#define FIXUP_QUEUE_NAME	TASK_QUEUE_PREFIX"FIXUP"		/* Fixup */
#define COLLECT_QUEUE_NAME  TASK_QUEUE_PREFIX"COLLECTOR"	/* Collector*/
#define NOI_QUEUE_NAME		TASK_QUEUE_PREFIX"DIIOP"		/* NOI Process */
#define ALARM_QUEUE_NAME	TASK_QUEUE_PREFIX"ALARMS"		/* Alarms Cache daemon */
#define MONITOR_QUEUE_NAME	TASK_QUEUE_PREFIX"MONITOR"		/* Monitor */
#define MONALARM_QUEUE_NAME	TASK_QUEUE_PREFIX"MONITORALARM"		/* Monitor */
#define APDAEMON_REQ_QUEUE	TASK_QUEUE_PREFIX"APDAEMONREQ"				/* Admin Panel Daemon (Request Queue) */
#define APDAEMON_FILERES_QUEUE	TASK_QUEUE_PREFIX"APDAEMONFILERESPONSE"		/* Admin Panel Daemon (File Response Queue) */
#define APDAEMON_FILEREQ_QUEUE	TASK_QUEUE_PREFIX"APDAEMONFILEREQUEST"		/* Admin Panel Daemon (Server Response Queue) */
#define BKTASKS_QUEUE_NAME	TASK_QUEUE_PREFIX"BKTASKS"		/* bktasks */
#define RZINTER_QUEUE_NAME	TASK_QUEUE_PREFIX"RZINTER"		/* Red Zone Interface to Collector */
#define RZEXTRA_QUEUE_NAME	TASK_QUEUE_PREFIX"RZEXTRA"		/* Red Zone Extra MQ */
#define RZBG_QUEUE_NAME	TASK_QUEUE_PREFIX"RZBG"				/* Red Zone Background MQ */
#define RZBGEXTRA_QUEUE_NAME	TASK_QUEUE_PREFIX"RZBGEXTRA"	/* Red Zone Background Extra MQ */
#define REALTIME_STATS_QUEUE_NAME	TASK_QUEUE_PREFIX"REALTIME"		/* Monitor */
#define RUNJAVA_QUEUE_NAME	TASK_QUEUE_PREFIX"RUNJAVA"      /* Runjava (used by ISpy) */
#define STATS_QUEUE_NAME	TASK_QUEUE_PREFIX"STATS"		/* Runjava (used by ISpy) */
#define LOG_SEARCH_QUEUE_NAME	TASK_QUEUE_PREFIX"LOGSEARCH"      /* Runjava (used by ISpy) */
#define DAEMON_EVENT_QUEUE_NAME	TASK_QUEUE_PREFIX"DAEMONEVENT"		/* Event process */
#define DAEMON_COLLECT_QUEUE_NAME  TASK_QUEUE_PREFIX"DAEMONCOLLECTOR"	/* Collector*/
#define DIRCAT_QUEUE_NAME	TASK_QUEUE_PREFIX"DIRCAT"		/* Dircat */
#define RMEVAL_QUEUE_NAME	TASK_QUEUE_PREFIX"RMEVAL"		/* RMEval */
#define SCRREPL_QUEUE_NAME	TASK_QUEUE_PREFIX"SCR"		/* Streaming Cluster Replicator */



/*  Names for server access lists and other stuff from NOTES.INI */

#define SERVER_TITLE_NAME			"SERVER_TITLE"
#define ALLOW_ACCESS_NAME			"ALLOW_ACCESS"
#define DENY_ACCESS_NAME		  	"DENY_ACCESS"
#define CREATE_FILE_ACCESS_NAME		"CREATE_FILE_ACCESS"
#define CREATE_REPLICA_ACCESS_NAME	"CREATE_REPLICA_ACCESS"
#define ADMIN_ACCESS_NAME			"ADMIN_ACCESS"
#define ALLOW_PASSTHRU_TARGET_NAME	"ALLOW_PASSTHRU_TARGETS"
#define ALLOW_PASSTHRU_CLIENT_NAME	"ALLOW_PASSTHRU_CLIENTS"
#define ALLOW_PASSTHRU_CALLER_NAME	"ALLOW_PASSTHRU_CALLERS"
#define ALLOW_PASSTHRU_ACCESS_NAME	"ALLOW_PASSTHRU_ACCESS"
#define ALLOW_RESTRICTED_LOTUSCRIPT_NAME	"ALLOW_RESTRICTED_LOTUSSCRIPT"
#define ALLOW_UNRESTRICTED_LOTUSCRIPT_NAME	"ALLOW_UNRESTRICTED_LOTUSSCRIPT"
#define ALLOW_MONITORS_NAME			"ALLOW_MONITORS"
#define DENY_MONITORS_NAME		  	"DENY_MONITORS"

/*  Field Names for server access lists and other stuff from NAMES.NSF */

#define ALLOW_ACCESS_ITEM			"AllowAccess"
#define DENY_ACCESS_ITEM		  	"DenyAccess"
#define CREATE_FILE_ACCESS_ITEM		"CreateAccess"
#define CREATE_REPLICA_ACCESS_ITEM	"ReplicaAccess"
#define CREATE_TEMPLATE_ACCESS_ITEM	"TemplateAccess"
#define ADMIN_ACCESS_ITEM			"Administrator"
#define ALLOW_PASSTHRU_TARGET_ITEM	"PTTargets"
#define ALLOW_PASSTHRU_CLIENT_ITEM	"PTClients"
#define ALLOW_PASSTHRU_CALLER_ITEM	"PTCallers"
#define ALLOW_PASSTHRU_ACCESS_ITEM	"PTAccess"
#define ALLOW_RESTRICTED_LOTUSCRIPT_ITEM	"RestrictedList"
#define ALLOW_UNRESTRICTED_LOTUSCRIPT_ITEM	"UnrestrictedList"
#define ALLOW_ON_BEHALF_ITEM		"OnBehalfOfLst"
#define ALLOW_ON_BEHALF_INVOKER_ITEM		"OnBehalfOfInvokerLst"
#define ALLOW_PERSONAL_ITEM			"PrivateList"
#define ALLOW_LIBRARIES_ITEM		"LibsLst"
#define ALLOW_REMOTE_HNAMES_ITEM 	"TrustedSrvrs"
#define ALLOW_MONITORS_ITEM			"AllowMonitors"
#define DENY_MONITORS_ITEM		  	"DenyMonitors"
#define FULL_ADMIN_ACCESS_ITEM		"FullAdmin"
#define DB_ADMIN_ACCESS_ITEM		"DBAdmin"
#define REMOTE_ADMIN_ACCESS_ITEM	"RemoteAdmin"
#define VO_ADMIN_ACCESS_ITEM		"VOAdmin"
#define WEB_ADMIN_ACCESS_ITEM		"BrowserAdminAccess"
#define NNTP_ADMIN_ACCESS_ITEM		"NNTP_Admin"
#define	SYS_ADMIN_ACCESS_ITEM		"SysAdmin"
#define	SYS_ADMIN_RES_ACCESS_ITEM	"ResSysAdmin"
#define	SYS_ADMIN_RES_COMMANDS_ITEM	"ResSystemCmds"

/*	Names for folder pane. */

#define CATEGORIES_AS_FOLDERS		"CategoriesAsFolders"

/*	Location information */

#define LOCATION_DEFAULT	"Location"
#define LOCATION_TYPE		"LocationType"
#define LOCATION_TRAVELPROMPT "TravelPrompt"
#define LOCATION_FORMTYPE	"Location"
#define LOCATION_REPLICATIONFORM	"$ReplicationSubform"
#define LOCATION_VIEW		"($Locations)"
#define LOCATION_NAME		"Name"
#define LOCATION_SOURCE		"Source"
#define LOCATION_USERID		"UserID"
#define LOCATION_TIMEZONE	"TimeZone"
#define LOCATION_R5TIMEZONE "R5TimeZone"
#define LOCATION_USEOSTZ	"UseOSTZ"
#define LOCATION_DST		"DST"
#define LOCATION_DOMAIN		"Domain"
#define LOCATION_MAILSERVER	"MailServer"
#define LOCATION_DIRSERVER	"DirectoryServer"
#define LOCATION_MAILFILE	"MailFile"
#define LOCATION_MAILFILE_LASTSEQ	"$MailFileSeq"
#define LOCATION_DEFPASSTHRU "DefaultPassthruServer"
#define LOCATION_OUTSIDELINE "OutsideLine"
#define LOCATION_CARDPREFIX	"CallPre"	/* Number to dial to get carriers dial tone (10ATT or 1800...) */
#define LOCATION_CARDSUFFIX	"Suffix"	/* Number to identify user;s account (i.e. User's Home Phone + PIN) */
#define LOCATION_INTERNATIONALPREFIX	"InternationalPrefix"
#define LOCATION_LONGDISTANCEPREFIX		"LongDistancePrefix"
#define LOCATION_COUNTRY	"Country"
#define LOCATION_AREACODE	"AreaCode"
#define LOCATION_MAILTYPE	"MailType"
#define LOCATION_MAILTHRESHOLD		"MailThreshold"
#define LOCATION_TCPNAMESERVER		"TcpNameServer"
#define LOCATION_TCPNAMESERVERHOST	"TcpNameServerHost"

#define LOCATION_REPLICATION_ENABLED	"ReplicationEnabled"
#define LOCATION_REPLICATE_IMMEDIATE	"ReplicateImmediate"
#define LOCATION_REPLICATE_AT_START		"ReplicateAtStart"
#define LOCATION_PROMPT_AT_START		"PromptAtStart"
#define LOCATION_REPLICATE_AT_END		"ReplicateAtEnd"
#define LOCATION_PROMPT_AT_END			"PrmptAtEnd"
#define LOCATION_OUTBOX_AT_END			"ReplicateOutboxAtEnd"
#define LOCATION_TOBESENT_AT_END		"ReplicateToBeSentAtEnd"

#define LOCATION_USESPRIORITYSCHEDULE	"HiPriorityReplication"

#define LOCATION_SCHEDULE				"Enabled"
#define LOCATION_PRIORITYSCHEDULE		"Enabled_HI"
#define LOCATION_SCHEDULE_DURATION		"Schedule"
#define LOCATION_SCHEDULE_DURATION_HI	"Schedule_HI"
#define LOCATION_INTERVAL				"Interval"
#define LOCATION_INTERVAL_HI			"Interval_HI"
#define LOCATION_WEEKDAYS_LIST			"WeekDays"
#define LOCATION_WEEKDAYS_LIST_HI		"WeekDays_HI"

#define LOCATION_NDSNAMESERVER			"NDSNameServer"
#define LOCATION_NDSNAMESERVERADDRESS	"NDSNameServerAddress"
#define LOCATION_NETBIOSNAMESERVER		"NetBIOSNameServer"
#define LOCATION_SOCKSPROXY 			"Proxy_SOCKs"
#define LOCATION_SSLPROXY 				"Proxy_SSL"
#define LOCATION_NOTESHTTPPROXY			"Proxy_NotesHttp"
#define LOCATION_PROXYUSERID 			"Proxy_LoginName"
#define LOCATION_PROXYPASSWORD 			"Proxy_Password"
#define LOCATION_PROXYAUTH_ENABLED 	   	"ProxyAuthenticationFlag"

#define LOCATION_IMAGES		"Images"
#define LOCATION_USERNAME_FLAG 			"UserNameFlag"	/* Primary/Alternate UserName Mode. 0 = Primary, 1 = Alternate */

/*	The following field is only meaningful if the user has workstation based
	mail, if they do not, then it is assumed that server addressing (i.e.
	resolution of the names, and allowing the use of server N&A books when
	addressing mail, is always available).  If the user is using a local
	mail file, but has network connectivity, then they may want to allow
	the use of server addressing.  If the user has no connectivity, then'
	this should be assumed to be off - regardless of its setting.  DWC */

#define LOCATION_SERVERADDRESSING		"MailAddressing"

		/* InterNotes/Web Location Items */

#define LOCATION_INTERNOTES					"InterNotes"
#define LOCATION_WEBLOCALDB					"WebDbName"
#define LOCATION_WEBSERVERDB				"WebDbName"
#define LOCATION_WEBRETRIEVERLOCATION		"WebLocation"
#define LOCATION_WEBUPDATECACHE				"WebUpdateCache"
#define LOCATION_WEBRETRIEVERLOGGING		"WebLogLevel"
#define LOCATION_WEBRETRIEVER				"WebRetriever"
#define LOCATION_WEBRETRIEVERS				"WebRetrievers"
#define LOCATION_WEBRETRIEVERPATH			"WebRetrieverPath"
#define LOCATION_WEBHTTPPROXY				"Proxy_HTTP"
#define LOCATION_WEBFTPPROXY				"Proxy_FTP"
#define LOCATION_WEBGOPHERPROXY				"Proxy_Gopher"
#define LOCATION_WEBNOPROXY					"No_Proxy"
#define LOCATION_PROXYFLAG					"ProxyFlag"

#define LOCATION_SSLSITECERT				"SSLSiteCerts"
#define LOCATION_SSLEXPIREDCERTS			"SSLExpiredCerts"
#define LOCATION_SSLPROTOCOLVER				"SSLProtocolVersion"
#define LOCATION_SSLSENDCERTS				"SSLSendCertificates"

		/* Java Location Items */

#define LOCATION_JAVASECUREDOMAINLIST		"JavaSecureDomain"
#define LOCATION_JAVAALLOWINSECURE			"JavaAllowInSecureDomain"
#define LOCATION_JAVAALLOWINOTHER			"JavaAllowInOtherDomains"
#define LOCATION_JAVATRUSTPROXY				"JavaTrustHTTPProxy"

		/* Internet Mail Location Items */

#define LOCATION_IMAIL_PROTOCOL				"MailServerProtocol"
#define LOCATION_IMAIL_USER					"ImailUsername"
#define LOCATION_IMAIL_PASSWORD				"ImailPassword"
#define LOCATION_IMAIL_SERVERLIST			"ImailServer"
#define LOCATION_IMAIL_SMTP_SERVER			"ImailSMTPServer"
#define LOCATION_IMAIL_PORT					"ImailPort"
#define LOCATION_IMAIL_USE_SSL				"ImailUseSSL"
#define LOCATION_IMAIL_ADDRESS				"ImailAddress"
#define LOCATION_IMAIL_ADDRESS_UPDATED_BY	"ImailAddressUpdatedBy"
#define LOCATION_IMAIL_POP_DEL_MAIL			"PopDeleteMail"
#define LOCATION_IMAIL_SMTP_ROUTE			"SMTPRoute"
#define LOCATION_IMAIL_INTERNET_DOMAIN		"InternetDomain"
#define LOCATION_IMAIL_ATTACH_ENCODE		"SMTPEncodingScheme"
#define LOCATION_IMAIL_MAC_ATTACH_ENCODE	"SMTPMacAttachConvertOption"
#define LOCATION_IMAIL_MSGFORMAT			"MessageFormat"
	/* If this field is set, this location's internet mail can be changed for existing users
		with information from the directory person record on the home server */
#define	LOCATION_IMAIL_ACCEPTUPDATES		"ImailAcceptUpdates"


		/* Admin Client Location Items */

#define LOCATION_ADMIN_MONITOR_REMENABLE	"EnableRemoteMonitor"
#define LOCATION_ADMIN_MONITOR_REMSERVER	"RemoteMonitorServer"
#define LOCATION_ADMIN_MONITOR_COLLINTER	"DataCollectionInterval"
#define LOCATION_ADMIN_MONITOR_AUTOACTIVATE	"AutoActivateMonitor"
#define LOCATION_ADMIN_FAVORITE_SERVERS		"AdminFavoriteServers"
#define LOCATION_ADMIN_STAT_GEN_REPORTS		"GenerateStatReports"
#define LOCATION_ADMIN_STAT_REP_INTERVAL	"ReportInterval"
#define LOCATION_ADMIN_STAT_GEN_ALARMS		"GenerateStatAlarms"
#define LOCATION_ADMIN_STAT_ALARM_INTERVAL	"AlarmInterval"
#define LOCATION_ADMIN_CHART_SAMEAS_MON		"ChartIntervalIsSameAsMonitor"
#define LOCATION_ADMIN_CHART_INTERVAL		"ChartInterval"


/*	Personal database names... */
#define LOCATION_BOOKMARK_DB		"BookmarkDb"
#define LOCATION_SUBSCRIPTION_DB	"HeadlineDb"


/* Prior to build 125, in the location record, the ports that were listed
	were those that were enabled.  Beginning in build 125, the ports that
	are listed (internally by a different item) are those that are disabled.
	The location form will however present the enabled ports by selecting the
	ports which are not specifed in the disabled item.  When saved, it is
	inverted back to mean the ports that are disabled.  In this manner,
	newly added ports will be enabled for all locations (by default). */

#define LOCATION_AUTONETCONFIG				"AutoNetConfig"
#define LOCATION_PORTS			"PortName"	  		/* Prior to 125 */
#define LOCATION_DISABLEDPORTS	"DisabledPorts"		/* 125 and later */
#define LOCATION_NAMELOOKUPMODE "NameLookupMode"
#define LOCATION_NAMELOOKUPPREF	"NameLookupPref"
#define LOCATION_EXHAUSTIVENAMELOOKUP	"ExhaustiveNameLookup"
#define LOCATION_MAIL_ADDR_FILE "MailAddressFile"	/* Last N&A Book used at this location */
#define LOCATION_VERSION	"$LocationVersion"
#define LOCATION_BCASELIST	"$BCaseList"
#define LOCATION_DIALING_SERVERS "$DialingServers"
#define LOCATION_DIALING_RULES   "$DialingRules"
#define LOCATION_STACKFILENAMES	"$StackFileNames"	/* Filename of topmost
														replica icon at this
														location for this
														'stack', */
/*	The V5 list is a parallel list of a database and the last replica.  It
	is independent of the V4 list to avoid differnces that may be necessary
	between the 2 lists. */

#define LOCATION_LASTFILENAMES	"$LastFileNames"	/* Filename of topmost
														replica icon at this
														location for this
														'stack', */
#define LOCATION_LASTREPLICAIDS "$LastReplicaIDs"	/* Replica ID of topmost
														replica icon at this
														location for this
														'stack', */
#define LOCATION_LASTACCESSED	"$LastAccessedDB"	/* Time/Date of last use
														of this DB. */

#define LOCATION_SAVED_SERVER	"$SavedServers"		/* Server name */
#define LOCATION_SAVED_PORT		"$SavedPorts"		/* Port server reached on */
#define LOCATION_SAVED_ADDRESS	"$SavedAddresses"	/* Server address on this port */
#define LOCATION_SAVED_DATE		"$SavedDate"		/* Julian Date */
#define LOCATION_SAVED_LAST_TRIED_DATE	"$SavedTriedDate"	/* Julian Date */
#define LOCATION_RLAN_IDLE_TIMEOUT	"RLANIdleTimeout"	/* timeout for RLAN hangup */

/* The following field was introduced in build 133 so that new icons added
	to the desktop get bubbled to the top of the stack if appropriate.  New
	icons should be at the top of the stack if:

	the DB icon is mew, is for a server DB, the location has a LAN port and
		the current top of the stack is a local database.
	the DB icon is mew, is for a local DB, the location has no LAN port and
		the current top of the stack is a server database.
*/
#define LOCATION_STACKCHECKED		"$StackChecked"		

/* If this field is set, this location can be changed for existing users
	with information from the person record and setup profile records for this
	user on their home server */

#define	LOCATION_ACCEPTUPDATES		"AcceptUpdates"

/*	Sametime server */

#define LOCATION_SAMETIMESERVER		"SametimeServer"

#define LOCATION_SAMETIME_PORT				"SametimePort"
#define LOCATION_SAMETIME_PROTOCOL			"SametimeProtocol"
#define LOCATION_SAMETIME_PROXY_TYPE		"SametimeProxyType"
#define LOCATION_SAMETIME_PROXY_SERVER		"SametimeProxyServer"
#define LOCATION_SAMETIME_PROXY_PORT		"SametimeProxyPort"
#define LOCATION_SAMETIME_PROXY_RESOLVE		"SametimeServerNameResolve"
#define LOCATION_SAMETIME_PROXY_USERNAME	"SametimeProxyUsername"
#define LOCATION_SAMETIME_PROXY_PASSWORD	"SametimeProxyPassword"
#define LOCATION_SAMETIME_LOGON_WHEN		"SametimeLogonWhen"
#define LOCATION_SAMETIME_PASSWORD			"SametimePassword"
#define LOCATION_SAMETIME_USERNAME			"SametimeUsername"

/*	Cached mail file replica ID information.  This information can only
		be used if the last mailfile name agrees with the present one.  That
		would mean that the user has not changed the file name and or server
		name since the time we squirreled the information away. */

#define LOCATION_LAST_MAILFILE_NAME		"$LastMailPath"
#define LOCATION_LAST_MAILFILE_REPID	"$LastMailRepID"



/*	Catalog server - used for Domain Search */

#define CATALOG_FILE_V5				"catalog.nsf"
#define LOCATION_CATALOGSERVER		"CatalogServer"
#define DOMAINQUERY_FORM_NAME		"DomainQuery"
#define LOCAL_DOMAIN_CATALOG_SERVERS "LocalDomainCatalogServers"
#define DOMAIN_INDEXER_SCOPE		"CatalogIndexerScope"
#define DOMAIN_CATALOG_FIELD		"CatalogType"
#define DOMAIN_INDEXER_DIRECTORY	"CatalogIndexerDirectory"
#define DOMAIN_CATALOG_NAME			"Domain Catalog"
#define DOMAIN_CATALOG_SCOPE		"CatalogDomainScope"
#define CATALOG_SERVERLOOKUPITEMCOUNT	2
#define CATALOG_SERVERLOOKUPITEMS		"CatalogType\0Administrator\0"

#define BOOKMARK_DBNAME				"bookmark.nsf"
#define BOOKMARK_TEMPLATENAME				"bookmark.ntf"
#define BOOKMARK_FINDDB_FORMNAME	"DatabaseSearch"

/* some bookmark views */
#define BOOKMARK_LAYOUTS_VIEW		"(Layouts)"
#define BOOKMARK_URLS_VIEW			"(URLs)"
#define BOOKMARK_DOWNLOADS_VIEW		"(Downloads)"

/* This is a field on URL bookmark notes to give revision info */
#define BOOKMARK_URL_MODIFIED		"LotusModified"
#define BOOKMARK_URL_502			"5.02"

/*	Fields used to store the list of folders into which a document is to be
	saved.  Note that @Command([ChooseFolders]) reads and writes these fields,
	so you do not normally need to manipulate them in a document.

	Note also that to a user, there are only two sets of folders: Shared and
	Private, with the distinction that some Private folders are stored in the
	databse, and some on the workstation in the desktop file (when the user
	cannot store them in the database due to access restrictions). */

#define FOLDERS_SHARED			"$Folders"			  	/*	List of shared folders. */
#define FOLDERS_PRIVATE			"$PrivateFolders"	  	/*	List of private folders (in database). */
#define FOLDERS_PRIVATE_LOCAL	"$LocalPrivateFolders"	/*	List of private folders (in desktop). */

/* Item names of fields in Mail template: */

#define MAILREPORT_ITEM	"$DeliveryReport"		/* TBD */

/* Item Name of view format note item */

#define ITEM_NAME_VIEW_FORMATNOTE	"$FormatNote"	/* UNID of note views design was originally copied from */

/*	Profile note & field name items on view format note.  The profile field
	contains a user-definable formula for a column */

#define VIEW_COLUMN_PROFILE_DOC			"$ColumnProfileDoc"
#define VIEW_COLUMN_FORMAT_ITEM			"$ColumnFormatItem"

/*	Standard folder names. */

#define FOLDER_INBOX			"($Inbox)"
#define FOLDER_DRAFTS			"($Drafts)"
#define FOLDER_SENT				"($Sent)"
#define FOLDER_ALL				"($All)"
#define FOLDER_CALENDAR			"($Calendar)"
#define FOLDER_TODO				"($ToDo)"
#define FOLDER_MEETINGS			"($Meetings)"
#define FOLDER_TRASH			"($Trash)"
#define FOLDER_PROFILE			"($Profiles)"
#define FOLDER_ALARMS			"($Alarms)"
#define FOLDER_SOFT_DELETE		"($SoftDeletions)"
#define FOLDER_CONTACTS			"($Contacts)"
#define FOLDER_RULES			"(Rules)"
#define FOLDER_GROUP_CALENDARS	"(Group Calendars)"
#define FOLDER_STATIONERY		"(Stationery)"
#define FOLDER_FOLLOW_UP		"($FollowUp)"
#define FOLDER_MEETINGS_BY_CATEGORY	"($Meetings By Category)"
#define FOLDER_MEETINGS_BY_MONTH	"($Meetings By Month)"
#define FOLDER_MEETINGS_ALL_BY_DATE	"($Meetings All By Date)"
#define FOLDER_MEETINGS_BY_ENTRY_TYPE	"($Meetings By Entry Type)"
#define FOLDER_MEETINGS_BY_STATUS	"($Meetings By Status)"
#define FOLDER_MEETINGS_ALL_BY_DATE2 "($MeetingsAllByDate2)"
#define FOLDER_MEETINGS_ON_MY_CALENDAR	"($MeetingsOnMyCalendar)"
#define FOLDER_MINIVIEW_NOTICES	"Miniview - Notices2"
#define FOLDER_JUNKMAIL			"($JunkMail)"
#define VIEW_FOLLOW_UP			"($Follow-Up)"

#define COMMON_SCRIPT_LIBRARY_NAME	"Common"

#define FOLDER_INBOX_NAME		"$Inbox"
#define FOLDER_DRAFTS_NAME		"$Drafts"
#define FOLDER_SENT_NAME		"$Sent"
#define FOLDER_ALL_MAIL			"$All"
#define FOLDER_CALENDAR_NAME	"$Calendar"
#define FOLDER_TODO_NAME		"$ToDo"
#define FOLDER_MEETINGS_NAME	"$Meetings"
#define FOLDER_TRASH_NAME		"$Trash"
#define FOLDER_PROFILE_NAME		"$Profiles"
#define FOLDER_ALARMS_NAME		"$Alarms"
#define FOLDER_SOFT_DELETE_NAME	"$SoftDeletions"
#define FOLDER_CONTACTS_NAME	"$Contacts"
#define FOLDER_RULES_NAME		"Rules"
#define FOLDER_GROUP_CALENDARS_NAME	"Group Calendars"
#define FOLDER_STATIONERY_NAME	"Stationery"
#define FOLDER_FOLLOW_UP_NAME	"$FollowUp"
#define FOLDER_MEETINGS_BY_CATEGORY_NAME	"$Meetings by Category"
#define FOLDER_MEETINGS_BY_MONTH_NAME		"$Meetings By Month"
#define FOLDER_MEETINGS_ALL_BY_DATE_NAME		"$Meetings All By Date"
#define FOLDER_MEETINGS_BY_ENTRY_TYPE_NAME	"$Meetings By Entry Type"
#define FOLDER_MEETINGS_BY_STATUS_NAME		"$Meetings By Status"
#define FOLDER_MEETINGS_ALL_BY_DATE2_NAME		"$MeetingsAllByDate2"
#define FOLDER_MEETINGS_ON_MY_CALENDAR_NAME		"$MeetingsOnMyCalendar"
#define FOLDER_JUNKMAIL_NAME	"$JunkMail"
#define VIEW_FOLLOW_UP_NAME		"$Follow-Up"
#define VIEW_IMTRANSCRIPTS_NAME	"$IMTranscripts"
#define VIEW_MAILTHREADS_NAME	"Mail Threads"

#define FOLDER_INBOX_ALIAS		"Inbox"
#define FOLDER_DRAFTS_ALIAS		"Drafts"
#define FOLDER_SENT_ALIAS		"Sent"
#define FOLDER_ALL_MAIL_ALIAS	"All"
#define FOLDER_CALENDAR_ALIAS	"Calendar"
#define FOLDER_TODO_ALIAS		"Tasks"
#define FOLDER_MEETINGS_ALIAS	"Meetings"
#define FOLDER_TRASH_ALIAS		"Trash"
#define FOLDER_JUNKMAIL_ALIAS	"JunkMail"
#define FOLDER_PROFILE_ALIAS	"Profiles"
#define FOLDER_ALARMS_ALIAS		"Alarms"
#define FOLDER_SOFT_DELETE_ALIAS "SoftDeletions"
#define FOLDER_CONTACTS_ALIAS	"People"
#define FOLDER_RULES_ALIAS		"Rules"
#define FOLDER_GROUP_CALENDARS_ALIAS	"(GroupCalendarFolder)"
#define FOLDER_STATIONERY_ALIAS	"Stationery"
#define FOLDER_MEETINGS_BY_CATEGORY_ALIAS	"Meetings by Category"
#define FOLDER_MEETINGS_BY_MONTH_ALIAS		"Meetings by Month"
#define FOLDER_MEETINGS_ALL_BY_DATE_ALIAS		"MeetingsAllByDate"
#define FOLDER_MEETINGS_BY_ENTRY_TYPE_ALIAS	"MeetingsByEntryType"
#define FOLDER_MEETINGS_BY_STATUS_ALIAS		"MeetingsByStatus"

/*	Site database view name */

#define SITE_DBLOOKUP_VIEW		"($DbLookup)"

/*	Router Shared Mail Object Store names */

#define SHARED_MAIL_NAME			"SHARED_MAIL"				/* config variable for shared mail */
#define ROUTER_OBJECT_STORE_NAME	"mailobj.nsf"				/* object store link file used by the router */
#define ROUTER_OBJECT_STORE_DBNAME	"mailobj1.nsf"				/* object store database created by the router */



/* Cluster Name and Address Book lookup definitions */

#define CLUSTERS_CLUSTER_VIEW_1		"1\\$Clusters"
#define CLUSTERS_CLUSTER_VIEW		"$Clusters"
#define CLUSTERS_CLUSTER_ITEM_NAME	"ClusterName"
#define CLUSTERS_CLUSTER_COL_NAME	"$4"						/* Name of the ClusterName column in the $Clusters view */
#define CLUSTERS_CLUSTER_ITEM		0

#define CLUSTERS_SERVERS_VIEW_1		"1\\$Servers"
#define CLUSTERS_SERVERS_VIEW		"$Servers"
#define CLUSTERS_SERVER_ITEM_NAME	"ServerName"
#define CLUSTERS_SERVER_COL_NAME	"$0"						/* Name of the ServerName column in the $Clusters view */
#define CLUSTERS_SERVER_ITEM		0

#define CLUSTERS_CLREPID_ITEM_NAME	"ClRepID"
#define CLUSTERS_CLREPID_ITEM		0

/* Cluster Database Directory definitions */

#define CLUSTERS_SERVER_VIEW		"$Server"
#define CLUSTERS_PATHNAME_VIEW		"$Pathname"
#define CLUSTERS_REPID_VIEW			"$ReplicaID"
#define CLUSTERS_DIRTITLE_VIEW      "$DirectoryTitle"

#define CLUSTERS_SERVER_FIELD		"Server"
#define CLUSTERS_REPID_FIELD		"ReplicaID"
#define CLUSTERS_PATHNAME_FIELD		"PathName"
#define CLUSTERS_TITLE_FIELD  		"Title"
#define CLUSTERS_OUTOFSERVICE_FIELD	"DatabaseOutOfService"
#define CLUSTERS_REPL_FIELD			"ClusterReplicate"
#define CLUSTERS_FAILBYPATH_FIELD	"OnlyFailoverbyPathname"
#define CLUSTERS_DELETE_FIELD		"MarkedForDelete"

#define CLUSTERS_DBDIR_NAME				"cldbdir.nsf"
#define CLUSTERS_DBDIR_TEMPLATE_NAME	"cldbdir4.ntf"

/* Values for CLUSTERS_OUTOFSERVICE_FIELD */
#define CLUSTERS_IN_SERVICE 		"0"
#define CLUSTERS_OUT_OF_SERVICE 	"1"

/* Values for CLUSTERS_CLREPL_FIELD */
#define CLUSTERS_CLUSTER_REPLICATE_ON	"1"
#define CLUSTERS_CLUSTER_REPLICATE_OFF	"0"


/* User registration document and DUS (Domino Upgrade Service) field definitions */

/* Fields related to the basics user registration dialog pane */
#define USERREG_FULLNAME_ITEM 			"FullName"
#define USERREG_FIRSTNAME_ITEM			"FirstName"
#define USERREG_LASTNAME_ITEM			"LastName"
#define USERREG_MIDDLEINITIAL_ITEM		"MiddleInitial"
#define USERREG_SHORTNAME_ITEM			"ShortName"
#define USERREG_PASSWORD_ITEM			"UserPassword"
#define USERREG_POLICY_ITEM				"ExplicitPolicy"
#define USERREG_ORG_POLICY_ITEM			"OrgPolicy"

/* Comment field in the 'Other' user registration dialog pane */
#define USERREG_COMMENT_ITEM  		"Comment"

/* DUS related fields */
#define USERREG_DUSUSERID_ITEM	 	  	"NUAUserID"
#define USERREG_DUSNAME_ITEM		  	"NUAName"
#define USERREG_DUSALIASNAMES_ITEM 	  	"DUSFullNameAliases"
#define USERREG_DUSMAILFILE_ITEM	   	"NUAMailFile"
#define USERREG_DUSGROUPID_ITEM		  	"GroupID"
#define USERREG_DUSPARENTGROUPS_ITEM  	"ParentGroups"		/* for groups with parent groups */
#define USERREG_PERSONNOTEHANDLE_ITEM 	"PersonNoteHandle"	/* note handle to new person note in NAB created by user reg */

/* Advance info item fields */
#define USERREG_DUSADVANCEDINFO_ITEM 	"NUAAdvancedInfo"	/*	The DUS must set the DUS_ADVANCEDINFO_ITEM below to "1" if
																 any of the USERREG_XXX fields below are set by the DUS */
#define USERREG_STORAGE_ITEM			"MessageStorage"	/* Specifies how user wants to store the mail.  See USERREG_STORAGE_XXX below */
#define USERREG_STORAGE_UNKNOWN					0xFF		/* can't find storage type */
#define USERREG_STORAGE_CDRECORDS				0 			/* Store as cd records only */
#define USERREG_STORAGE_CDRECORDS_AND_RFC822	1 			/* Store as cd records & attachment of original message */
#define USERREG_STORAGE_RFC822					2 			/* Store original message as attachment (no cdrecords) */
#define USERREG_STORAGE_NATIVE_MIME				3			/* Store headers & parts as separate items (without conversion) or CD */
#define USERREG_STORAGE_NATIVE_MIME_ONLY		4			/* Store headers & parts as separate items (without conversion) only */

#define USERREG_PERSONAL_TITLE			"Title"			/* keyword choices are Mr., Ms., Mrs., Miss, Dr. or Prof. */
#define USERREG_GENERATION_QUALIFIER	"Suffix"		/* keyword choices are I, II, III, Jr., or Sr. */

#define USERREG_HOME_STREETADDRESS		"StreetAddress"
#define USERREG_HOME_CITY				"City"
#define USERREG_HOME_STATE				"State"
#define USERREG_HOME_ZIP				"Zip"
#define USERREG_HOME_COUNTRY			"Country"
#define USERREG_PHONENUMBER_ITEM		"PhoneNumber"
#define USERREG_HOME_FAX				"HomeFAXPhoneNumber"
#define USERREG_SPOUSE					"Spouse"
#define USERREG_CHILDREN				"Children"

#define USERREG_COMPANYNAME_ITEM		"CompanyName"
#define USERREG_JOB_TITLE				"JobTitle"
#define USERREG_DEPARTMENT_ITEM			"Department"
#define USERREG_MANAGER					"Manager"
#define USERREG_OFFICEPHONE_ITEM		"OfficePhoneNumber"
#define USERREG_CELL_PHONE			  	"CellPhoneNumber"
#define USERREG_PAGER					"PhoneNumber_6"
#define	USERREG_OFFICE_FAX				"OfficeFAXPhoneNumber"
#define USERREG_ASSISTANT				"Assistant"
#define USERREG_OFFICE_STREETADDRESS	"OfficeStreetAddress"
#define USERREG_OFFICE_CITY				"OfficeCity"
#define USERREG_OFFICE_STATE			"OfficeState"
#define USERREG_OFFICE_ZIP				"OfficeZip"
#define USERREG_OFFICE_COUNTRY			"OfficeCountry"
#define USERREG_OFFICE_NUMBER			"OfficeNumber"
#define USERREG_EMPLOYEEID_ITEM			"EmployeeID"

#define USERREG_ENCRYPT_INCOMING_MAIL	"EncryptIncomingMail" /* keyword choices are "Yes" | "1" or "No" | "0" */
#define USERREG_X400_ADDRESS			"x400Address"
#define USERREG_WEB_SITE				"WebSite"
#define USERREG_CALENDARDOMAIN_ITEM		"CalendarDomain" /* User's calendar domain override */
/* END of User registration document and DUS field definitions */


/*	Last specified path and username for alternate mail logon dialog. */

#define ALT_MAIL_LAST_PATH			"AltMailLastPath"
#define ALT_MAIL_LAST_NAME			"AltMailLastName"

/*	Convert Utility Constants. */

#define ITEM_NAME_CONVERT_FORM	FIELD_FORM
#define ITEM_NAME_CONVERT_DATE	"ConvertedDate"
#define ITEM_NAME_HIDDEN_DOC	"HiddenDocument"

/*	Names stored as UNKs if a note has rarely used item names. */

#define	RARELY_USED_NAME		"$RarelyUsedName"
#define	RARELY_USED_TABLE		"$RarelyUsedTable"
#define RARELY_USED_V4_ONLY_ITEM "$RUsedR4Only"

#define AGENT_HSCRIPT_ITEM		"$AgentHScript"
#define AGENT_HSCRIPTOBJ_ITEM	"$AgentHScript_O"

/*	Local schedule retrieval defintions */

#define	LOCSCHED_FORM_NAME		"LocalSchedSettings"
#define LOCSCHED_PROFILE_NAME	"LocalSchedSettings"

/*	Fields in local schedule database (busytime) for local 
	schedule synchronization via replicator page. */


#define	ITEM_LOCSCHED_LIST			"LocalSchedList"
#define ITEM_LOCSCHED_DAYSTOFETCH	"LocalSchedDaysToFetch"
#define ITEM_LOCSCHED_SUPPRESS		"LocalSchedSuppressMinutes"

/* OLE "special" Items used for ActiveDoc/RichText rendering */

#define OLE_ITEM_OLEOBJFIELD			"$OLEObjField"
#define OLE_ITEM_OLEOBJPROGID			"$OLEObjProgID"
#define OLE_ITEM_OLEOBJRICHTEXTFIELD	"$OLEObjRichTextField"

/* Enables any OLE object to be in-place activated in preview pane mode.  Used
	to activate OLE controls like WEB Browser */
#define OLE_PREVIEW_ACTIVATE			"$OLEPreviewActivate"

/*Site Database design and item names*/
#define SITE_FORM_NAME "Database"
#define SITE_REPLICAID_VIEW_NAME 	"($ReplicaID)"
#define SITE_PROFILE_NAME 			"Site"

#define SITE_TITLE_ITEM_NAME "Title"
#define SITE_REALTITLE_ITEM_NAME "RealTitle"
#define SITE_DBICON_ITEM_NAME "dbicon"
#define SITE_SEQUENCE_ITEM_NAME "dbsequence"
#define SITE_REPLICAID_ITEM_NAME "replicaid"
#define SITE_SEQUENCE_ITEM_NAME 	"dbsequence"
#define SITE_SERVERHINT_ITEM_NAME "$ServerHint"

/* Web Browser control well known action name and form name */
#define OLE_URLNAVIGATE_ACTION		"$ControlURLNavigate"
#define OLE_WEBBROWSER_FORM			"WebBrowserForm"		/* Used if IE 3 is installed */
#define OLE_WEBBROWSER4_FORM		"WebBrowser4Form"		/* Used if IE 4 is installed */
#define OLE_MOZILLABROWSER_FORM		"MozillaForm"			/* Used if NS 6 Mozilla control is installed */

/* Distinguished Field Names */
#define DN_COUNTRY	"C"
#define DN_ORGANIZATION "O"
#define DN_ORGANIZATIONAL_UNIT "OU"
#define DN_COMMON_NAME "CN"
#define DN_SURNAME "S"
#define DN_USER_ID "UID"
#define DN_STREET_ADDRESS "STREET"
#define DN_LOCALITY "L"
#define DN_STATE "ST"
#define DN_DOMAIN_COMPONENT "DC"



/*	Items used to store folder references. */

#define	ITEM_FOLDER_REF				"$FolderRef"
#define	ITEM_FOLDER_REF_ID			"$FolderRefID"
#define	ITEM_FOLDER_REF_FLAGS		"$FolderRefFlags"

/*	Items used to store r6 imap folder references. */

#define	IMAP_ITEM_FOLDER_REF		"$IMAPFolderRef"
#define	IMAP_ITEM_FOLDER_REF_ID		"$IMAPFolderRefID"
#define	IMAP_ITEM_FOLDER_REF_FLAGS	"$IMAPFolderRefFlags"

/* DON'T CHANGE FOLLOWING VALUES.  IMAP SERVER DEPENDS ON THESE VALUES. */
/*	Folder reference flag values. */

#define	FOLDER_REF_FLAG_SEEN		0x00000001
#define	FOLDER_REF_FLAG_ANSWERED	0x00000002
#define	FOLDER_REF_FLAG_FLAGGED		0x00000004
#define	FOLDER_REF_FLAG_DELETED		0x00000008
#define	FOLDER_REF_FLAG_DRAFT		0x00000010

#define	FOLDER_REF_FLAG_NOT_RECENT	0x00000020
#define	FOLDER_REF_FLAG_RECENT		0x00000040		/*	Unique flag value that is not stored */

#define FOLDER_REF_FLAG_MDNSENT		0x00000080
#define FOLDER_REF_FLAG_KEYWORD		0x00000100

/*	Name of the folder reference information collection. */

#define	FOLDER_REF_INFO_COLLECTION	"$FolderRefInfo"

/*	Collations in the folder reference information collection. */

#define	FOLDER_UNID_REF_ID_COLLATION_NUMBER		1
#define	FOLDER_UNID_REF_ID_COLLATION_LEVELS		2

#define FOLDER_UNID_NNTP_DATE_COLLATION_NUMBER	2
#define FOLDER_UNID_NNTP_DATE_COLLATION_LEVELS	2

/*	Summary item names used to store folder reference information. */

#define	FOLDER_REF_UNID_ITEM_NAME		"$105"
#define	FOLDER_REF_REF_ID_ITEM_NAME 	"$FolderRefID"
#define	FOLDER_REF_FLAGS_ITEM_NAME		"$FolderRefFlags"

/*  Summarty item names used to store NNTP folder reference information. */

#define FOLDER_REF_NNTP_SUBJECT_ITEM_NAME 		"$110"
#define FOLDER_REF_NNTP_FROM_ITEM_NAME			"$109"
#define FOLDER_REF_NNTP_DATE_ITEM_NAME			"$113"
#define FOLDER_REF_NNTP_MESSAGE_ID_ITEM_NAME	"$uname"
#define FOLDER_REF_NNTP_REFERENCES_ITEM_NAME	"$114"
#define FOLDER_REF_NNTP_SIZE_ITEM_NAME			"NNTP_Size"		
#define FOLDER_REF_NNTP_LINES_ITEM_NAME			"Lines"
#define FOLDER_REF_NNTP_ISLMBCS_ITEM_NAME		"$115"
#define FOLDER_REF_NNTP_DISTRIBUTION_ITEM_NAME	"Distribution"
#define FOLDER_REF_NNTP_PATH_ITEM_NAME			"Path"


/*	Folder flag values. */

#define	FOLDER_FLAG_NOINFERIORS		0x00000001
#define	FOLDER_FLAG_NOSELECT		0x00000002
#define	FOLDER_FLAG_MARKED			0x00000004
#define	FOLDER_FLAG_UNMARKED		0x00000008

#define	FOLDER_FLAG_SYSTEM			0x00000010	/*	Set if folder name starts with '$' */
#define	FOLDER_FLAG_HIDDEN			0x00000020	/*	Set if folder name starts with '(' */

#define FOLDER_FLAG_NNTP_NEWSGROUP	0x00000040
#define FOLDER_FLAG_HASCHILDREN		0x00000080	/* set if folder has children */

/*	A special, unique folder flag value, that is never stored, used to indicate
	that any value of folder flags are to be considered a match. */

#define	FOLDER_FLAG_ANY				0x80000000

/*	A mask of bits that are used to qualify a folder name. */

#define	FOLDER_FLAG_MATCH_MASK		(FOLDER_FLAG_SYSTEM | FOLDER_FLAG_HIDDEN)

/*	A bit that is used to indicate that any value of folder flags are to be
	considered a match.  */

#define	FOLDER_FLAG_MATCH_ANY		(FOLDER_FLAG_ANY)


/*	Name of the folder information collection. */

#define	FOLDER_INFO_COLLECTION		"$FolderInfo"

/*	Collations in the folder information collection. */

#define	FOLDER_NAME_COLLATION_NUMBER			1
#define	FOLDER_NAME_COLLATION_LEVELS			1

#define	FOLDER_UNID_COLLATION_NUMBER			2
#define	FOLDER_UNID_COLLATION_LEVELS			1

/*	Collations in the IMAIL resync information collection. */

#define	RESYNC_INFO_NOTEID_COLLATION_NUMBER		1
#define	RESYNC_INFO_NOTEID_COLLATION_LEVELS		1

#define	RESYNC_INFO_UID_COLLATION_NUMBER		2
#define	RESYNC_INFO_UID_COLLATION_LEVELS		1

#define	RESYNC_INFO_MID_COLLATION_NUMBER		3
#define	RESYNC_INFO_MID_COLLATION_LEVELS		1

/*	Summary item names used to store folder information. */

#define	FOLDER_NAME_ITEM_NAME			"$106"
#define	FOLDER_UNID_ITEM_NAME	 		"$103"
#define	FOLDER_FLAGS_ITEM_NAME			"$107"

/*	Items used to store folder information. */

#define	ITEM_FOLDER_FLAGS			"$FolderFlags"

/*	Length of the text representation of an UNID in hex format. */

#define	UNID_TEXT_LENGTH			(2 * sizeof(UNID))

/*  IMAP subscription list */

#define IMAP_PROFILE_SUBSCRIPTION	"IMAPProfileSubscription"
#define ITEM_IMAP_SUBSCRIPTION		"$IMAPSubscription"

/*	IMAP NAMESPACE Configuration Items */

#define IMAP_CFGREC_NS_SHAREDDBLINKS		"IMAPNSShrdDbLinks"
#define IMAP_CFGREC_NS_OTHERSUNREAD			"IMAPNSOthersUnrd"
#define IMAP_PROFILE_NS_OTHERUSERS			"IMAPNSOtherUsers"

/*	IMAP Replication Items */

#define IMAP_SEQNO_ITEM_NAME				"$IMAPSeqNo"
#define	IMAP_LASTFOLDERUNID_ITEM_NAME		"$IMAPLastFolderUNID"
#define	IMAP_UID_ITEM_NAME					"$IMAPUID"
#define IMAP_FLAGS_ITEM_NAME				"$IMAPFlags"
#define IMAP_DISCFLAGS_ITEM_NAME   			"$IMAPDiscFlags"
#define IMAP_MSGID_ITEM_NAME				"$IMAPMsgID"

/*	IMAP Replication $IMAPResyncInfo view column names */

#define IMAP_SEQNO_COLUMN					"IMAPSeqNo"
#define	IMAP_LASTFOLDERUNID_COLUMN			"IMAPLastFolderUNID"
#define	IMAP_UID_COLUMN						"IMAPUID"
#define IMAP_FLAGS_COLUMN					"IMAPFlags"
#define IMAP_DISCFLAGS_COLUMN				"IMAPDiscFlags"
#define IMAP_MSGID_COLUMN					"IMAPMsgID"

/*	NNTP Replication Items */

#define	NNTP_FOLDERUNID_ITEM_NAME			"$NNTPFolderUNID"
#define	NNTP_UID_ITEM_NAME					"$NNTPUID"
#define NNTP_FLAGS_ITEM_NAME				"$NNTPFlags"
#define NNTP_MSGID_ITEM_NAME				"$UName"
#define FIELD_PARENT_UNAME					"$ParentUName"

/*	NNTP Replication $NNTPResyncInfo view column names */

#define	NNTP_FOLDERUNID_COLUMN				"NNTPFolderUNID"
#define	NNTP_UID_COLUMN						"NNTPUID"
#define NNTP_FLAGS_COLUMN					"NNTPFlags"
#define NNTP_MSGID_COLUMN					"UName"

/*	Name of the IMAP Resync information collection. */

#define	IMAP_RESYNC_INFO_COLLECTION			"$IMAPResyncInfo"

/*	Name of the NNTP Resync information collection. */

#define	NNTP_RESYNC_INFO_COLLECTION			"$NNTPResyncInfo"

/*	components (java applets, etc. ) */

#define	COMPONENTS_LIST_ITEM	"$Components"

/* Setup */
#define SETUP_SERVERLOOKUP_VIEW	"($ServersLookup)"

/*	Admin Panel	*/

#define	NETWORKS_NAMESPACE				"$Networks"
#define INIT_NETWORKSLOOKUPITEMCOUNT	2
#define INIT_NETWORKSLOOKUPITEMS      	"Network\0ServerName"
#define INIT_NETWORKSITEM_NETWORK      	0
#define INIT_NETWORKSITEM_SERVER      	1

#define POLICIES_NAMESPACE				"$Policies"
#define POLICIES_NAMESPACE_ALT			"($Policies)"
#define POLICY_MASTER_POLICY_FORM		"PolicyMaster"
#define POLICY_ARCHIVE_SETTINGS_FORM	"PolicyArchive"
#define POLICY_SETUP_SETTINGS_FORM		"PolicySetup"
#define POLICY_REG_SETTINGS_FORM		"PolicyRegistration"
#define POLICY_DESKTOP_SETTINGS_FORM	"PolicyDesktop"
#define POLICY_SECURITY_SETTINGS_FORM	"PolicySecurity"
#define POLICY_SYNOPSIS_FORM			"Synopsis"

/* Domain Type */

#define NOTES_DOMAIN_TYPE 	"NOTES"
#define LDAP_DOMAIN_TYPE	"LDAP"



/* Headline Items */

#define HEADLINE_DEFAULTVIEW_ITEM		"($Headlines)"			/* Default view to use to create headlines views */

#define HEADLINE_VIEW_ITEM				"$HLView"				/* Headline marker item for headlines view */
#define HEADLINE_SUMMARY_ITEM			"$HLNoteSummary"		/* Headline summary item */
#define HEADLINE_MODIFIED_ITEM			"$HLNoteModified"		/* Headline last modified time (time) */
#define HEADLINE_UNID_ITEM				"$HLNoteUNID"			/* Headline UNID (text) */
#define HEADLINE_SEARCHMATCH_ITEM		"$HLNoteSearchMatch"	/* Headline search match item (text format below) */
#define HEADLINE_REF_ITEM				"$HLNoteREF"			/* Headline saved reference (no main topic found) */

/* Subscription Items */

#define SUBSCRIPTION_VIEW				"$Subscriptions"	/* Alias of subscriptions view */
#define	SUBSCRIPTION_FORM				"$Subscription"		/* Name of subscription form */
#define SUBSCRIPTION_HEADLINES_VIEW		"$HeadlinesView"	/* Designer specified view for headlines */
#define	FIELD_FORM_DBID					"$FormDatabaseID"

#define SUBSCRIPTION_VIEW_ITEM			"$HLSubscription"	/* Marker item for subscriptions */
#define SUBSCRIPTION_TITLE				"$HLTitle"			/* Title of subscription */

#define SUBSCRIPTION_DISABLED_ITEM		"$HLDisabled"		/* Disabled Flag, checked before enabled flag */
#define SUBSCRIPTION_ENABLED_ITEM		"$HLEnabled"		/* Enabled Flag, only checked if disabled flag not found */
#define SUBSCRIPTION_DISABLED			'0'					/* Enabled Flag - disabled */
#define SUBSCRIPTION_ENABLED			'1'					/* Enabled Flag - enabled */

#define SUBSCRIPTION_TYPE_ITEM			"$HLType"			/* Subscription Type */
#define SUBSCRIPTION_TYPE_DB			'1'					/* Subscription Type - DB monitoring */
#define SUBSCRIPTION_TYPE_VIEW			'2'					/* Subscription Type - view */
#define SUBSCRIPTION_TYPE_URL			'3'					/* Subscription Type - URL */
#define SUBSCRIPTION_KIND_DB			1					/* Subscription Kind - DB monitoring */
#define SUBSCRIPTION_KIND_VIEW			2					/* Subscription Kind - view */
#define SUBSCRIPTION_KIND_URL			3					/* Subscription Kind - URL */
#define SUBSCRIPTION_UNID_ITEM			"$HLUnid"			/* Subscription UNID for resync */
#define SUBSCRIPTION_MODIFIED_ITEM		"$HLModifiedTime"	/* Subscription Modified time for resync */


#define SUBSCRIPTION_TARGETFRAME_ITEM	"$HLTargetFrame"	/* Frame to use for the subscription */
#define SUBSCRIPTION_FRAME_DB			"$HLMonitorFrame"	/* Default frame to use for monitor subscriptions */
#define SUBSCRIPTION_FRAME_VIEW			"$HLViewFrame"		/* Default frame to use for view subscriptions */
#define SUBSCRIPTION_FRAME_URL			"$HLURLFrame"		/* Default frame to use for URL subscriptions */

#define SUBSCRIPTION_URL_ITEM			"$HLURL"			/* URL for View and URL subscription types */

#define SUBSCRIPTION_FORMULA_ITEM		"$HLFormula"		/* Monitor Formula */
#define SUBSCRIPTION_FULLTEXT_ITEM		"$HLFulltext"		/* Monitor Full Text */

#define SUBSCRIPTION_OPTIONS_ITEM		"$HLOptions"		/* Monitor Options Flag */
#define SUBSCRIPTION_OPTIONS_NONE		'0'					/* Monitor Options Flag - none */
#define SUBSCRIPTION_OPTIONS_SUMMARY	'1'					/* Monitor Options Flag - summary */
#define SUBSCRIPTION_OPTIONS_LOCAL		'2'					/* Monitor Options Flag - local */
#define SUBSCRIPTION_OPTIONS_ALL		'3'					/* Monitor Options Flag - summary&local */


#define SUBSCRIPTION_MONITORTYPE_ITEM	"$HLMonitorType"	/* Monitor Type Flag */
#define SUBSCRIPTION_MONITORTYPE_FORMULA	'1'				/* Monitor Type Flag - formula */
#define SUBSCRIPTION_MONITORTYPE_FULLTEXT	'2'				/* Monitor Type Flag - full text */
#define SUBSCRIPTION_DB_ITEM			"$HLMonitorDB"		/* Database to Monitor */
#define SUBSCRIPTION_DB_VIEW_ITEM		"$HLHeadlineView"	/* User specified view for headlines */

#define SUBSCRIPTION_CREATE			"$HLCreateSubscription"	/* Flag to indicate a partial subscription that */
															/* needs more information to be created */
#define SUBSCRIPTION_CREATE_MAIL		"mail"				/* Creating a special mail file subscription */

#define SUBSCRIPTION_LASTCHECKED_ITEM	"$HLMLastChecked"	/* Last time the monitor DB was checked */
#define SUBSCRIPTION_CLIENTID_ITEM		"$HLMClientId"		/* Last ClientId used */
#define SUBSCRIPTION_SERVER_ITEM		"$HLMServer"		/* Original server to be monitored */
#define SUBSCRIPTION_LASTSERVER_ITEM	"$HLMLastServer"	/* Last server monitored */
#define SUBSCRIPTION_LASTPATH_ITEM		"$HLMLastPath"		/* Last path monitored */
#define SUBSCRIPTION_MONITORID_ITEM		"$HLMMonitorId"		/* Last MonitorId used */
#define SUBSCRIPTION_ERROR_ITEM			"$HLMError"			/* Last error encountered while initilaizing */
#define SUBSCRIPTION_SERVER_INFO		"$HLMServerInfo"	/* Information about last servers monitored */

#define SUBSCRIPTION_DBNAME				"Headline.nsf"


/*	Framesets used in the client */
#define CLIENT_FRAMESET_LAYOUT		"ClientLayout"
#define CLIENT_FRAMESET_MORE		"ClientMore"
#define CLIENT_FRAMESET_BMPAGE		"BMPage"

/*	Frameset used in mail */
#define MAIL_FRAMESET_MAIL			"MailFS"
/*  And by calendar */
#define MAIL_FRAMESET_CALENDAR		"CalendarFS"
/*  And by ToDo */
#define MAIL_FRAMESET_TODO			"ToDoFS"

#define Bookmark_IntroPageName 		"Intro"	/* webpage name in bookmark to startup */
#define Bookmark_StartupFramesetName "Home"	/* frameset name in bookmark to startup */

/*	Special frame names used by the client */

#define NAVIGATOR_PANE				"NotesNavigator"
#define VIEW_PANE					"NotesView"
#define PREVIEW_PANE				"NotesPreview"
#define EMBEDDED_PREVIEW_PANE		"EmbeddedNotesPreview"
#define USE_DEFAULT_FRAME_TARGETING "UseNotesDefaultFrameTargeting"
#define CALENDAR_PANE				"CalendarView"

/*	Names used for various client types which can exist */
#define DESIGNER_CLIENT				"Designer"
#define ADMIN_CLIENT				"Admin"
#define NOTES_CLIENT				"Notes"
#define DGW_CLIENT					"DGW"

/* For use in search results template forms */
#define SEARCH_QUERY "Query"
#define SEARCH_START "Start"
#define SEARCH_COUNT "Count"
#define SEARCH_HITS "Hits"
#define SEARCH_TOTAL_HITS "TotalHits"
#define SEARCH_RESULT_LIMIT "SearchMax"
#define SEARCH_WV "SearchWV"
#define SEARCH_THESAURUS "SearchThesaurus"
#define SEARCH_FUZZY "SearchFuzzy"
#define SEARCH_ORDER "SearchOrder"
#define SEARCH_SORT_OPTIONS "SortOptions"
#define SEARCH_OTHER_OPTIONS "OtherOptions"
#define SEARCH_BODY "Body"
#define SEARCH_ENTRYFORM_NAME "SearchEntry"
#define SEARCH_BODY_WEB ITEM_NAME_EMBEDDED_VIEW
#define SEARCH_VIEW "SearchView"
#define SEARCH_SCOPE "Scope"

/* Dublin Core Meta Data */
#define ITEM_META_TITLE					"$$Title"
#define ITEM_META_CREATOR				"$$Creator"
#define ITEM_META_DESCRIPTION			"$$Description"
#define ITEM_META_TYPE					"$$Type"
#define ITEM_META_CATEGORIES			"$$Categories"

/* Document Content view in the Domain Catalog */
#define CATALOG_VIEW_CONTENT_CATEGORY "$DocumentContent"

/*	Shared Resources */

#define ITEM_NAME_IMAGE_DATA			"$ImageData"
#define ITEM_NAME_IMAGE_NAMES			"$ImageNames"
#define ITEM_NAME_IMAGES_WIDE			"$ImagesWide"
#define ITEM_NAME_IMAGES_HIGH			"$ImagesHigh"
#define ITEM_NAME_IMAGES_COLORIZE		"$ImagesColorize"
#define ITEM_NAME_IMAGES_WEB_BROWSER_COMPATIBLE "$WebBrowserCompatible"

#define ITEM_NAME_JAVA_FILES			"$JavaFiles"

#define ITEM_NAME_STYLE_SHEET_DATA		"$StyleSheetData"
#define ITEM_NAME_STYLE_SHEET_NAME		"$StyleSheetName"

#define ITEM_NAME_FILE_DATA				"$FileData"
#define ITEM_NAME_FILE_NAMES			"$FileNames"
#define ITEM_NAME_FILE_WEBPATH			"$WebFilePath"
#define ITEM_NAME_FILE_EDITFILE			"$EditFilePath"
#define ITEM_NAME_FILE_EDITOR			"$FileEditor"

#define ITEM_NAME_FILE_SIZE				"$FileSize"
#define ITEM_NAME_FILE_MIMETYPE			"$MimeType"
#define ITEM_NAME_FILE_MIMECHARSET		"$MimeCharSet"
#define ITEM_NAME_FILE_MODINFO			"$FileModDT"

/*	defadmin.ntf definitions. */

#define ITEM_TASK_LOAD_LOADCMD			"$Task_LoadCmd"
#define ITEM_TASK_TELLCMD				"$Task_TellCmd"
#define ITEM_TASK_LOAD_NAME				"Task_name"
#define ITEM_TASK_LOAD_FILENAME			"Task_filename"
#define ITEM_TASK_LOAD_DESCRIPTION		"Task_description"
#define ITEM_TASK_LOAD_HASUI			"Task_hasLoadUI"
#define ITEM_TASK_TELL_HASUI			"Task_hasTellUI"
#define ITEM_TASK_MONITOR_NAME			"Task_MonitorName"
#define ITEM_TASK_NO_START_ASP			"DisableStartForASP"

/* Special image resource name which will... */

#define IMAGE_DBICON_NAME	"$Icon"			/* use Database's icon if the image does not exist. */
#define IMAGE_OLEICON_NAME	"$OLEIcon"		/* use associated executable's icon if an image dosn't exist */

/* For reading International MIME settings in NAB */
#define MIMEI18N_FLD_CONFIG_ENABLED			"MIMEOptionsEnabled"
#define MIMEI18N_FLD_PRIMARY_GROUP			"CVS_PrimaryGroup"
#define MIMEI18N_FLD_SECONDARY_GROUPS		"CVS_SecondaryGroups"
#define MIMEI18N_FLD_INP_ALIAS_ALIAS  		"CVSI_CharsetAlias%d"
#define MIMEI18N_FLD_INP_ALIAS_ACTUAL 		"CVSI_CharsetActual%d"
#define MIMEI18N_FLD_EXP_ALIAS_ALIAS  		"CVSO_CharsetAlias%d"
#define MIMEI18N_FLD_EXP_ALIAS_ACTUAL 		"CVSO_CharsetActual%d"
#define MIMEI18N_FLD_CHARSET_DETECT			"SMTPCharSetDetect"
#define MIMEI18N_FLD_8BIT_FALLBACK			"CVSI_NonMIMECharSet"
#define MIMEI18N_FLD_INP_GROUP_FONT_PROP 	"CVSI_%s_F_P"
#define MIMEI18N_FLD_INP_GROUP_FONT_MONO 	"CVSI_%s_F_M"
#define MIMEI18N_FLD_INP_GROUP_FONT_PLAIN 	"CVSI_%s_F_T"
#define MIMEI18N_FLD_INP_GROUP_FONT_PLAIN_SIZE "CVSI_%s_F_Size"
#define MIMEI18N_FLD_INP_GROUP_FONT_HTML_SIZE "CVSI_%s_F_HSize"
#define MIMEI18N_FLD_EXP_GROUP_CSET_HEAD 	"CVSO_%s_CS_H"
#define MIMEI18N_FLD_EXP_GROUP_CSET_BODY 	"CVSO_%s_CS_B"
#define MIMEI18N_FLD_EXP_GROUP_ENC_HEAD 	"CVSO_%s_ENC_H"
#define MIMEI18N_FLD_EXP_GROUP_ENC_BODY 	"CVSO_%s_ENC_B"
/* _FALLBACK_MODE is obsolete, now using _MULTILINGUAL_MODE */
#define MIMEI18N_FLD_EXP_FALLBACK_MODE		"CVS_MLMessageHandling"
#define MIMEI18N_FLD_EXP_MULTILINGUAL_MODE	"CVS_MultilingualMsgHandling"
#define MIMEI18N_LOCATION_I18N_VIEW 		"($InternationalMIMESettings)"

/* these are obsolete, now using _MULTILINGUAL_xxx */
#define MIMEI18N_FALLBACK_UTF8				1
#define MIMEI18N_FALLBACK_FALLBACK_CHAR		2
#define MIMEI18N_FALLBACK_UNICODE_ENTITIES	3
#define MIMEI18N_FALLBACK_REFUSE			4

#define MIMEI18N_MULTILINGUAL_UNICODE		1
#define MIMEI18N_MULTILINGUAL_BEST_MATCH	2

/*
 * some standard mime charset names used by the XmlReformatter
 * and other xml code
 */
#define	MIME_CHARSET_UTF8		"UTF-8"
#define	MIME_CHARSET_UTF16		"UTF-16"
#define	MIME_CHARSET_UTF16LE	"UTF-16LE"
#define	MIME_CHARSET_UTF16BE	"UTF-16BE"

/* these are NOTES.INI variables used on the client only -- not server
 * which has this stuff in a config note
 */
#define MIME_MULTILINGUAL_MODE_VAR 	 "MIMEMultilingualMode"
#define MIME_PROMPT_MULTILINGUAL_VAR "MIMEPromptMultilingual"

/* NOTES.INI variables - show user preference about displaying
 * some Plug-In related warnings
 */
#define PluginsWarningDialog	0
#define PluginsWarningStatusBar	1
#define PluginsWarningDoNotShow	2

#define PluginsWarningOption "PluginsWarningOption"

/* see net/resolver.c for use of these */
#define HTTP_SERVER_LOOKUP_ITEMS "HTTP_HostName\0SMTPFullHostDomain\0HTTP_NormalMode\0HTTP_SSLMode\0HTTP_Port\0HTTP_SSLPort\0HTTP_RedirectUseHTTPS\0HTTP_RedirectServerName\0HTTP_RedirectPortNo\0HTTP_CompanionStack\0Enabled_0\0Protocol_0\0NetAddr_0\0Enabled_1\0Protocol_1\0NetAddr_1\0Enabled_2\0Protocol_2\0NetAddr_2\0Enabled_3\0Protocol_3\0NetAddr_3\0Enabled_4\0Protocol_4\0NetAddr_4\0Enabled_5\0Protocol_5\0NetAddr_5\0Enabled_6\0Protocol_6\0NetAddr_6\0Enabled_7\0Protocol_7\0NetAddr_7"
#define HTTP_SERVER_LOOKUP_ITEM_COUNT 30
#define HTTP_SERVER_LOOKUP_ITEM_HTTPHOSTNAME 0
#define HTTP_SERVER_LOOKUP_ITEM_FULLHOSTNAME 1
#define HTTP_SERVER_LOOKUP_ITEM_NORMALMODE 2
#define HTTP_SERVER_LOOKUP_ITEM_SSLMODE 3
#define HTTP_SERVER_LOOKUP_ITEM_PORT 4
#define HTTP_SERVER_LOOKUP_ITEM_SSLPORT 5
#define HTTP_SERVER_LOOKUP_ITEM_REDIRECTUSEHTTPS 6
#define HTTP_SERVER_LOOKUP_ITEM_REDIRECTSERVERNAME 7
#define HTTP_SERVER_LOOKUP_ITEM_REDIRECTPORTNO 8
#define HTTP_SERVER_LOOKUP_ITEM_COMPANIONSTACK 9
#define HTTP_SERVER_LOOKUP_ITEM_OFFS_START 10
#define HTTP_SERVER_LOOKUP_ITEM_OFFS_ENABLED 0
#define HTTP_SERVER_LOOKUP_ITEM_OFFS_PROTOCOL 1
#define HTTP_SERVER_LOOKUP_ITEM_OFFS_NETADDR 2
#define HTTP_SERVER_LOOKUP_ITEM_OFFS_MAX 3

/* Fault Recovery Address Book items */
#define FR_LOOKUP_ITEM_COUNT 7
#define FR_LOOKUP_ITEMS	\
"FREnbld\0FltRcvryCrsh\0FltRcvryMin\0\
FltRcvryMax\0FltRcvryNot\0FltRcvryScrpt\0NSDEnbld"

#define FR_ENABLED_ITEM 0
#define FR_MAX_CRASHES_ITEM 1
#define FR_CRASH_TIME_LIMIT_ITEM 2
#define FR_CLEANUPSCRIPT_TIME_LIMIT_ITEM 3
#define FR_NOTIFY_ITEM 4
#define FR_CLEANUPSCRIPT_ITEM 5
#define NSD_ENABLED_ITEM 6

#define FR_LOOKUP_ITEM_COUNT_ADC 	2
#define FR_LOOKUP_ITEMS_ADC			"FltRcvryNot\0NSDEnbld"
#define FR_NOTIFY_ITEM_ADC			0
#define FR_NSD_ENABLED_ITEM_ADC		1

#define	FA_NUM_LOOKUP_ITEMS	2
#define FA_LOOKUP_ITEMS		"MailServer\0MailFile"
#define FA_LOOKUP_SERVER		0
#define FA_LOOKUP_FILE		1

#define ADMINP_FR_ENABLED_ITEM						"FREnbld"			
#define ADMINP_FR_MAX_CRASHES_ITEM					"FltRcvryCrsh"
#define ADMINP_FR_CRASH_TIME_LIMIT_ITEM				"FltRcvryMin"
#define ADMINP_FR_CLEANUPSCRIPT_TIME_LIMIT_ITEM		"FltRcvryMax"
#define ADMINP_FR_NOTIFY_ITEM						"FltRcvryNot"
#define ADMINP_FR_CLEANUPSCRIPT_ITEM				"FltRcvryScrpt"
#define ADMINP_NSD_ENABLED_ITEM						"NSDEnbld"			

#ifdef __cplusplus
}
#endif

#endif /* STD_NAME_DEFS */

#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif


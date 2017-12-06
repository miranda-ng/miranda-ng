
#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(P128)
#endif

/*********************************************************************/
/*                                                                   */
/* Licensed Materials - Property of IBM                              */
/*                                                                   */
/* L-GHUS-5VMPGW, L-GHUS-5S3PEE                                      */
/* (C) Copyright IBM Corp. 2005  All Rights Reserved                 */
/*                                                                   */
/* US Government Users Restricted Rights - Use, duplication or       */
/* disclosure restricted by GSA ADP Schedule Contract with           */
/* IBM Corp.                                                         */
/*                                                                   */
/*********************************************************************/


#ifndef NSF_STR_DEFS
#define NSF_STR_DEFS

#ifndef NSF_ERR_DEFS
#include "nsferr.h"
#endif

/* errorContext strings for error logging 0 - 255 */
#define CONTEXT_DB2NSF_INIT				PKG_NSF_STR+1
	stringtext(CONTEXT_DB2NSF_INIT,		"Failed during DB2NSF initialization.")
#define CONTEXT_DB2DROPDB2				PKG_NSF_STR+3
	stringtext(CONTEXT_DB2DROPDB2,		"Failed dropping DB2 Database.")
#define CONTEXT_DB2CREATE_SERV_TAB		PKG_NSF_STR+4
	stringtext(CONTEXT_DB2CREATE_SERV_TAB,	"Failed creating DB2 server table.")
#define CONTEXT_DB2DROP_SERV_TAB		PKG_NSF_STR+5
	stringtext(CONTEXT_DB2DROP_SERV_TAB,	"Failed dropping DB2 server table.")
#define CONTEXT_DB2CREATE_FILE_TAB		PKG_NSF_STR+6
	stringtext(CONTEXT_DB2CREATE_FILE_TAB,	"Failed creating DB2 file table.")
#define CONTEXT_DB2DROP_FILE_TAB		PKG_NSF_STR+7
	stringtext(CONTEXT_DB2DROP_FILE_TAB,	"Failed dropping DB2 file table.")
#define CONTEXT_DB2WRITE_OBJ_HEADER		PKG_NSF_STR+8
	stringtext(CONTEXT_DB2WRITE_OBJ_HEADER,	"Failed writing DB2 object header.")
#define CONTEXT_DB2READ_OBJ_HEADER		PKG_NSF_STR+9
	stringtext(CONTEXT_DB2READ_OBJ_HEADER,	"Failed reading DB2 object header.")
#define CONTEXT_DB2MODIFY_OBJ_HEADER	PKG_NSF_STR+10
	stringtext(CONTEXT_DB2MODIFY_OBJ_HEADER,	"Failed modifying DB2 object header.")
#define CONTEXT_DB2WRITE_OBJ			PKG_NSF_STR+11
	stringtext(CONTEXT_DB2WRITE_OBJ,	"Failed writing DB2 object.")
#define CONTEXT_DB2CLEAR_OBJ			PKG_NSF_STR+12
	stringtext(CONTEXT_DB2CLEAR_OBJ,	"Failed clearing DB2 object.")
#define CONTEXT_DB2READ_OBJ				PKG_NSF_STR+13
	stringtext(CONTEXT_DB2READ_OBJ,		"Failed reading DB2 object.")
#define CONTEXT_DB2DELETE_OBJ			PKG_NSF_STR+14
	stringtext(CONTEXT_DB2DELETE_OBJ,	"Failed deleting DB2 object.")
#define CONTEXT_DB2MODIFY_RRV			PKG_NSF_STR+15
	stringtext(CONTEXT_DB2MODIFY_RRV,	"Failed modifying the RRV (record relocation vector).")
#define CONTEXT_DB2WRITE_OBJ_INFO		PKG_NSF_STR+16
	stringtext(CONTEXT_DB2WRITE_OBJ_INFO,	"Failed writing DB2 object info.")	
#define CONTEXT_DB2I_U_NOBJ				PKG_NSF_STR+17
	stringtext(CONTEXT_DB2I_U_NOBJ,		"Failed inserting or updating DB2 named object.")
#define CONTEXT_DB2DEL_NOBJ				PKG_NSF_STR+18
	stringtext(CONTEXT_DB2DEL_NOBJ,		"Failed deleting DB2 named object.")
#define CONTEXT_DB2READ_NOBJ			PKG_NSF_STR+19
	stringtext(CONTEXT_DB2READ_NOBJ,	"Failed reading DB2 named object.")
#define CONTEXT_DB2ENUM_NOBJ			PKG_NSF_STR+20
	stringtext(CONTEXT_DB2ENUM_NOBJ,	"Failed enumerating DB2 named object.")
#define CONTEXT_DB2RECOVER_FHEADER		PKG_NSF_STR+21
	stringtext(CONTEXT_DB2RECOVER_FHEADER,	"Failed when DB2 was recovering the file header.")
#define CONTEXT_DB2WRITE_FILE_HEADER	PKG_NSF_STR+22
	stringtext(CONTEXT_DB2WRITE_FILE_HEADER,	"Failed writing DB2 file header.")
#define CONTEXT_DB2CREATE_FILE_HEADER	PKG_NSF_STR+23
	stringtext(CONTEXT_DB2CREATE_FILE_HEADER,	"Failed creating DB2 file header.")
#define CONTEXT_DB2DELETE_CATALOG_ENT	PKG_NSF_STR+24
	stringtext(CONTEXT_DB2DELETE_CATALOG_ENT,	"Failed deleting DB2 catalog entry.")
#define CONTEXT_DB2READ_UNK				PKG_NSF_STR+25
	stringtext(CONTEXT_DB2READ_UNK,		"Failed when DB2 was reading UNK table.")
#define CONTEXT_DB2WRITE_UNK			PKG_NSF_STR+26
	stringtext(CONTEXT_DB2WRITE_UNK,	"Failed when DB2 was writing UNK table.")
#define CONTEXT_DB2REFRESH_DIR			PKG_NSF_STR+27
	stringtext(CONTEXT_DB2REFRESH_DIR,	"Failed when DB2 was refreshing directory.")
#define CONTEXT_DB2CONNPOOL_INIT		PKG_NSF_STR+28
	stringtext(CONTEXT_DB2CONNPOOL_INIT,	"Failed when DB2 was initializing the connection pool.")
#define CONTEXT_DB2CP_CREATE_ENT		PKG_NSF_STR+29
	stringtext(CONTEXT_DB2CP_CREATE_ENT,	"Failed when DB2 was creating an entry in the connection pool.")
#define CONTEXT_DB2CP_FREE_ENT			PKG_NSF_STR+30
	stringtext(CONTEXT_DB2CP_FREE_ENT,	"Failed when DB2 was freeing an entry in the connection pool.")
#define CONTEXT_DB2GET_NSF_SCH			PKG_NSF_STR+31
	stringtext(CONTEXT_DB2GET_NSF_SCH,	"Failed when DB2 was getting NSF Schema.")
#define CONTEXT_DB2GET_NSF_TBS			PKG_NSF_STR+32
	stringtext(CONTEXT_DB2GET_NSF_TBS,	"Failed when DB2 was getting NSF TableSpace.")
#define CONTEXT_DB2TERM_SHR_CONN		PKG_NSF_STR+33
	stringtext(CONTEXT_DB2TERM_SHR_CONN,	"Failed when DB2 was terminating shared connections.")
#define CONTEXT_DB2INIT_SHR_CONN		PKG_NSF_STR+34
	stringtext(CONTEXT_DB2INIT_SHR_CONN,	"Failed when DB2 was initializing shared connections.")
#define CONTEXT_DB2EXIST_NSF			PKG_NSF_STR+35
	stringtext(CONTEXT_DB2EXIST_NSF,		"Failed when DB2 was checking the existence of NSF.")
#define CONTEXT_DB2EXIST_NSFIDENT		PKG_NSF_STR+36
	stringtext(CONTEXT_DB2EXIST_NSFIDENT,	"Failed when DB2 checked for NSF identifier.")
#define CONTEXT_DB2SET_ISOLATION		PKG_NSF_STR+37
	stringtext(CONTEXT_DB2SET_ISOLATION,	"Failed when setting DB2 isolation level.")
#define CONTEXT_DB2ALLOC_SESSION		PKG_NSF_STR+38
	stringtext(CONTEXT_DB2ALLOC_SESSION,	"Failed when DB2 was allocating the session.")
#define CONTEXT_DB2CREATE_SAVEPT		PKG_NSF_STR+39
	stringtext(CONTEXT_DB2CREATE_SAVEPT,	"Failed when DB2 was creating save points.")
#define CONTEXT_DB2ROLLB_SAVEPT			PKG_NSF_STR+40
	stringtext(CONTEXT_DB2ROLLB_SAVEPT,	"Failed when DB2 was rolling back save points.")
#define CONTEXT_DB2RELEASE_SAVEPT		PKG_NSF_STR+41
	stringtext(CONTEXT_DB2RELEASE_SAVEPT,	"Failed when DB2 was releasing save points.")
#define CONTEXT_DB2GET_HSTMT			PKG_NSF_STR+42
	stringtext(CONTEXT_DB2GET_HSTMT,	"Failed when DB2 was getting statement handle.")
#define CONTEXT_DB2UPD_FILE_PATH		PKG_NSF_STR+43
	stringtext(CONTEXT_DB2UPD_FILE_PATH,	"Failed when DB2 was updating file paths.")
#define CONTEXT_DB2EXEC_NIFCONNECT		PKG_NSF_STR+44
	stringtext(CONTEXT_DB2EXEC_NIFCONNECT,	"Failed when DB2 was executing NIFConnect.")
#define CONTEXT_DB2GET_PROP				PKG_NSF_STR+45
	stringtext(CONTEXT_DB2GET_PROP,		"Failed when DB2 was getting properties.")
#define CONTEXT_DB2SET_PROP_SRCTARG		PKG_NSF_STR+46
	stringtext(CONTEXT_DB2SET_PROP_SRCTARG,		"Failed when DB2 was setting properties.")
#define CONTEXT_DB2GET_PROP_BLOB		PKG_NSF_STR+47
	stringtext(CONTEXT_DB2GET_PROP_BLOB,		"Failed when DB2 was getting blobs.")
#define CONTEXT_DB2CLOSE_CURSOR			PKG_NSF_STR+48
	stringtext(CONTEXT_DB2CLOSE_CURSOR,		"Failed when DB2 was closing cursor.")
#define CONTEXT_DB2READ_NOTE			PKG_NSF_STR+49
	stringtext(CONTEXT_DB2READ_NOTE,	"Failed when DB2 was reading a note.")
#define CONTEXT_DB2WRITE_RNOTE			PKG_NSF_STR+50
	stringtext(CONTEXT_DB2WRITE_RNOTE,	"Failed when DB2 was writing a row in a note.")
#define CONTEXT_DB2DELETE_RNOTE			PKG_NSF_STR+51
	stringtext(CONTEXT_DB2DELETE_RNOTE,	"Failed when DB2 was deleting a row in a note.")
#define CONTEXT_DB2SEL_NSUMMARY			PKG_NSF_STR+52
	stringtext(CONTEXT_DB2SEL_NSUMMARY,	"Failed when DB2 was selecting note summary set from ID table.")
#define CONTEXT_DB2SEL_NSUMMARY_SET		PKG_NSF_STR+53
	stringtext(CONTEXT_DB2SEL_NSUMMARY_SET,	"Failed when DB2 was selecting note summary set.")
#define CONTEXT_DB2SEL_NOTE			    PKG_NSF_STR+54
	stringtext(CONTEXT_DB2SEL_NOTE,		"Failed when DB2 was selecting a note.")
#define CONTEXT_DB2GET_NOTE_SROW		PKG_NSF_STR+55
	stringtext(CONTEXT_DB2GET_NOTE_SROW,		"Failed when DB2 was fetching a note summary row.")
#define CONTEXT_DB2GET_NOTE			    PKG_NSF_STR+56
	stringtext(CONTEXT_DB2GET_NOTE,		"Failed when DB2 was fetching a note.")
#define CONTEXT_DB2UNID_FINDE			PKG_NSF_STR+57
	stringtext(CONTEXT_DB2UNID_FINDE,	"Failed when DB2 was looking for UNID entry.")
#define CONTEXT_DB2_FINDE_BYNID			PKG_NSF_STR+58
	stringtext(CONTEXT_DB2_FINDE_BYNID,	"Failed when DB2 was looking for an entry by note id.")
#define MSG_DB2NSF_SCHEMA_UPGRADING		PKG_NSF_STR+59
	stringtext(MSG_DB2NSF_SCHEMA_UPGRADING,"Upgrading schema for DB2NSF %s.  This may take several minutes." )
#define CONTEXT_DB2DELETE_QUEUE_ENT		PKG_NSF_STR+60
	stringtext(CONTEXT_DB2DELETE_QUEUE_ENT,	"Failed deleting DB2 queue entry.")
#define CONTEXT_DB2RELOCATE_TS			PKG_NSF_STR+61
	stringtext(CONTEXT_DB2RELOCATE_TS,	"Failed to Relocate tablespace container.")
#define ERR_DB2NSF_QUIESCED			    PKG_NSF_STR+62
	stringtext(ERR_DB2NSF_QUIESCED,	    "Unable to Connect to quiesced database.")
#define CONTEXT_DB2COLLECT_COLINFO		PKG_NSF_STR+63
	stringtext(CONTEXT_DB2COLLECT_COLINFO,	"Failed when DB2 was collecting column info.")
#define CONTEXT_DB2PAIFIND_ENTRY		PKG_NSF_STR+64
	stringtext(CONTEXT_DB2PAIFIND_ENTRY,	"Failed when DB2 was finding a named object entry.")
#define CONTEXT_DB2PAIFIND_COUNT		PKG_NSF_STR+65
	stringtext(CONTEXT_DB2PAIFIND_COUNT,	"Failed when DB2 was finding the count of named object entries.")
#define CONTEXT_DB2PAI_INS_UPD			PKG_NSF_STR+66
	stringtext(CONTEXT_DB2PAI_INS_UPD,	"Failed when DB2 was inserting/updating a named object entry.")
#define CONTEXT_DB2PAI_DELETE			PKG_NSF_STR+67
	stringtext(CONTEXT_DB2PAI_DELETE,	"Failed when DB2 was deleting a named object entry.")
#define CONTEXT_DB2CREATE_TAB			PKG_NSF_STR+68
	stringtext(CONTEXT_DB2CREATE_TAB,	"Failed when creating DB2 table.")
#define CONTEXT_DB2CREATE_VIEW			PKG_NSF_STR+69
	stringtext(CONTEXT_DB2CREATE_VIEW,	"Failed when creating DB2 view.")
#define CONTEXT_DB2CREATE_DEL			PKG_NSF_STR+70
	stringtext(CONTEXT_DB2CREATE_DEL,	"Failed when creating DB2 delete.")
#define CONTEXT_DB2CREATE_DELTRIG		PKG_NSF_STR+71
	stringtext(CONTEXT_DB2CREATE_DELTRIG,	"Failed when creating DB2 delete trigger.")
#define CONTEXT_DB2CREATE_INS			PKG_NSF_STR+72
	stringtext(CONTEXT_DB2CREATE_INS,	"Failed when creating DB2 insert.")	
#define CONTEXT_DB2CREATE_INSTRIG		PKG_NSF_STR+73
	stringtext(CONTEXT_DB2CREATE_INSTRIG,	"Failed when creating DB2 insert trigger.")	
#define CONTEXT_DB2CREATE_UPD			PKG_NSF_STR+74
	stringtext(CONTEXT_DB2CREATE_UPD,	"Failed when creating DB2 update.")	
#define CONTEXT_DB2CREATE_UPDTRIG		PKG_NSF_STR+75
	stringtext(CONTEXT_DB2CREATE_UPDTRIG,	"Failed when creating DB2 update trigger.")	
#define CONTEXT_DB2LOAD_ACC_DEF			PKG_NSF_STR+76
	stringtext(CONTEXT_DB2LOAD_ACC_DEF,	"Failed when loading the access definition.")	
#define CONTEXT_DB2CREATE_EXPTAB		PKG_NSF_STR+77
	stringtext(CONTEXT_DB2CREATE_EXPTAB,	"Failed when creating DB2 export table.")	
#define CONTEXT_DB2INS_MAPENT			PKG_NSF_STR+78
	stringtext(CONTEXT_DB2INS_MAPENT,	"Failed when inserting DB2 map entry.")	
#define CONTEXT_DB2DROP_EXPTAB			PKG_NSF_STR+79
	stringtext(CONTEXT_DB2DROP_EXPTAB,	"Failed when dropping DB2 export table.")	
#define CONTEXT_DB2DROP_EXPENT			PKG_NSF_STR+80
	stringtext(CONTEXT_DB2DROP_EXPENT,	"Failed when dropping DB2 exported entity.")	
#define CONTEXT_DB2LOAD_ACCTAB			PKG_NSF_STR+81
	stringtext(CONTEXT_DB2LOAD_ACCTAB,	"Failed when loading the access definition records in the table.")
#define CONTEXT_DB2INS_ACCTAB			PKG_NSF_STR+82
	stringtext(CONTEXT_DB2INS_ACCTAB,	"Failed when inserting in the access table.")
#define CONTEXT_DB2INS_ACCTAB_ROW		PKG_NSF_STR+83
	stringtext(CONTEXT_DB2INS_ACCTAB_ROW,	"Failed when inserting an access table row.")
#define CONTEXT_DB2UPD_ACCTAB			PKG_NSF_STR+84
	stringtext(CONTEXT_DB2UPD_ACCTAB,	"Failed when updating the access table.")
#define CONTEXT_DB2DEL_FROM_ACCTAB		PKG_NSF_STR+85
	stringtext(CONTEXT_DB2DEL_FROM_ACCTAB,	"Failed when deleting from access table.")
#define CONTEXT_DB2LOAD_SB_PROP			PKG_NSF_STR+86
	stringtext(CONTEXT_DB2LOAD_SB_PROP,	"Failed loading super block property.")
#define CONTEXT_DB2WRITE_SB_PROP		PKG_NSF_STR+87
	stringtext(CONTEXT_DB2WRITE_SB_PROP,	"Failed writing super block property.")
#define CONTEXT_DB2TERM_SCONN			PKG_NSF_STR+88
	stringtext(CONTEXT_DB2TERM_SCONN,	"Failed terminating shared connections.")
#define CONTEXT_DB2FREE_NSUMSET			PKG_NSF_STR+89
	stringtext(CONTEXT_DB2FREE_NSUMSET,	"Failed when DB2 was freeing note summary set.")
/* AVAILABLE */
#define CONTEXT_DB2READ_FILE_HEADER		PKG_NSF_STR+93
	stringtext(CONTEXT_DB2READ_FILE_HEADER,	"Failed reading DB2 file header.")
#define CONTEXT_DB2FREE_HSTMT			PKG_NSF_STR+94
	stringtext(CONTEXT_DB2FREE_HSTMT,	"Failed when  DB2 was freeing statement handle.")
#define CONTEXT_DB2_ENABLE_UDF			PKG_NSF_STR+95
	stringtext(CONTEXT_DB2_ENABLE_UDF,	"Failed to enable user defined functions.")
#define CONTEXT_DB2_ADMIN_UDF			PKG_NSF_STR+96
	stringtext(CONTEXT_DB2_ADMIN_UDF,	"Failed while administering DB2 Access DLL.")
#define CONTEXT_DB2GRPSEM				PKG_NSF_STR+97
	stringtext(CONTEXT_DB2GRPSEM,		"Error encountered in group semaphore management." )
#define CONTEXT_DB2_QUERY_VIEWS			PKG_NSF_STR+98
	stringtext(CONTEXT_DB2_QUERY_VIEWS,	"Error encountered in DB2 Query Views" )
#define CONTEXT_DB2RECOVER_SOFT_DEL_LIST PKG_NSF_STR+99
	stringtext(CONTEXT_DB2RECOVER_SOFT_DEL_LIST,	"Failed recovering DB2 soft deleted note list.")
#define CONTEXT_DB2CATALOG				PKG_NSF_STR+100
	stringtext(CONTEXT_DB2CATALOG,		"Error creating DB2 catalog entry" )
#define MSG_DB2NSF_CATALOG_UPGRADED		PKG_NSF_STR+101
	stringtext(MSG_DB2NSF_CATALOG_UPGRADED,"Successfully upgraded schema %s from Version %s to Version %s." )
#define CONTEXT_TABLE_SCHEMA_UPGRADE	PKG_NSF_STR+102
	stringtext(CONTEXT_TABLE_SCHEMA_UPGRADE,		"Error upgrading table schema: " )
#define MSG_DB2NSF_SCHEMA_UPGRADED		PKG_NSF_STR+103
	stringtext(MSG_DB2NSF_SCHEMA_UPGRADED,"Successfully upgraded tables for schema %s from Version %s to Version %s." )
#define CONTEXT_DB2EXTEND_TS			PKG_NSF_STR+104
	stringtext(CONTEXT_DB2EXTEND_TS,     "Failed to extend tablespace." )
#define CONTEXT_DB2GET_NSF_SIZE			PKG_NSF_STR+105
	stringtext(CONTEXT_DB2GET_NSF_SIZE,	"Failed calculating DB2 NSF size.")

#define MSG_DB2_DEFAULT_BANNER          PKG_NSF_STR+106
	stringtext(MSG_DB2_DEFAULT_BANNER,	"%n--default----------------%n")
#define MSG_DB2_HANDLELOCATIONPRINT_TAG	PKG_NSF_STR+107
	stringtext(MSG_DB2_HANDLELOCATIONPRINT_TAG,  "  cliRC= %d,  line=%d,  file=%s%n")
#define MSG_DB2_HANDLEDIAGNOSTICPRINT_TAG		PKG_NSF_STR+108
	stringtext(MSG_DB2_HANDLEDIAGNOSTICPRINT_TAG,"%n%5d  SQLSTATE= %s, Native Error Code= %ld, message:")
#define MSG_DB2_DB2READNOTE1			PKG_NSF_STR+109
	stringtext(MSG_DB2_DB2READNOTE1,	"Error reading note (NoteID: NT%08lx) (Schema: %s)%n")
#define MSG_DB2_DB2WRITENOTE1			PKG_NSF_STR+110
	stringtext(MSG_DB2_DB2WRITENOTE1,	"Error writing note (NoteID: NT%08lx)(schema: %s)%n")
#define MSG_DB2_DB2DELETENOTE1			PKG_NSF_STR+111
	stringtext(MSG_DB2_DB2DELETENOTE1,	"Error deleting note (NoteID: NT%08lx)%n")

/* there may be some available PKG_NSF_STR offsets above... */

/* this is the *current* max; PKG_NSF_STR has an offset range of 0 - 127 */
#define STR_NSF_STR_MAX					PKG_NSF_STR+112
	stringtext(STR_NSF_STR_MAX,			"This is the last error message in the NSF_STR range")
			
#endif /*NSF_STR_DEFS*/


#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif


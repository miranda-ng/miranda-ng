
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


#ifndef NSF_ERR_DEFS
#define NSF_ERR_DEFS

/* DB2 Note Storage File Error Code Definitions 0x3B00 */
#ifndef NSF_STR_DEFS
#include "nsfstr.h"
#endif

#define ERR_DB2NSF_NOT_IMPL				PKG_DB2NSF+0
	errortext(ERR_DB2NSF_NOT_IMPL,	    "This feature not yet implemented for DB2 based .nsf(s)")
#define ERR_DB2NSF_FAILURE				PKG_DB2NSF+1
	errortext(ERR_DB2NSF_FAILURE,	    "General NSFDB2 error.")
#define ERR_DB2NSF_CORRUPT				PKG_DB2NSF+2
	errortext(ERR_DB2NSF_CORRUPT,	    "Corrupt/invalid data found in DB2 - unable to continue")
#define ERR_DB2NSF_CLI_ERROR			PKG_DB2NSF+3
	errortext(ERR_DB2NSF_CLI_ERROR,	    "DB2 CLI/SQL error")
#define ERR_DB2NSF_INI					PKG_DB2NSF+4
	errortext(ERR_DB2NSF_INI,		    "Invalid or missing DB2/MI .ini file entries")
#define ERR_DB2NSF_NAME_LENGTH			PKG_DB2NSF+5
	errortext(ERR_DB2NSF_NAME_LENGTH,	"Name exceeds DB2 maximum identifier length")
#define ERR_DB2NSF_BYPASS_XACT			PKG_DB2NSF+6
	errortext(ERR_DB2NSF_BYPASS_XACT,	"Internal error - should not appear")
#define ERR_DB2NSF_NO_BEGIN_XACT		PKG_DB2NSF+7
	errortext(ERR_DB2NSF_NO_BEGIN_XACT,	"There is no active transaction to end")
#define ERR_DB2NSF_PARMS_NEEDED			PKG_DB2NSF+10
	errortext(ERR_DB2NSF_PARMS_NEEDED,	"Parameters needed for SQL-based view")
#define ERR_DB2NSF_SUM_NOTE_SET			PKG_DB2NSF+12
	errortext(ERR_DB2NSF_SUM_NOTE_SET,	"Unable to create selected DB2 note set.")
#define ERR_DB2NSF_NIF_READONLY			PKG_DB2NSF+13
	errortext(ERR_DB2NSF_NIF_READONLY,	"Update or commit of read-only DB2 NIF connection")
#define ERR_DB2NSF_ACCESS_ROWLEN		PKG_DB2NSF+14
	errortext(ERR_DB2NSF_ACCESS_ROWLEN,	"Column Length or Total Access Table Row Length exceeds maximum allowed by DB2.")
#define ERR_DB2NSF_ACCESS_DEFTYPES		PKG_DB2NSF+15
	errortext( ERR_DB2NSF_ACCESS_DEFTYPES, "Incompatible DB2 data type and Notes data type or data format options, fieldname: ")
#define ERR_DB2NSF_ACCESS_FIELDLEN		PKG_DB2NSF+16
	errortext( ERR_DB2NSF_ACCESS_FIELDLEN, "Access Table field data exceeds defined length")
#define ERR_DB2NSF_INVALID_LENGTH_READ		PKG_DB2NSF+17
	errortext(ERR_DB2NSF_INVALID_LENGTH_READ, "Number of bytes read is smaller than expected.")
#define ERR_DB2NSF_NOT_ENABLED_FOR_DB2	PKG_DB2NSF+18
	errortext( ERR_DB2NSF_NOT_ENABLED_FOR_DB2, "Domino server is not DB2-enabled or DB2 services were unable to initialize when the server started up.")
#define ERR_DB2NSF_CANNOT_USE_DATASTORE	PKG_DB2NSF+19
	errortext( ERR_DB2NSF_CANNOT_USE_DATASTORE, "You need Server Administrator access to use the requested datastore")
#define ERR_DB2NSF_TABLESPACE_IN_USE	PKG_DB2NSF+20
	errortext( ERR_DB2NSF_TABLESPACE_IN_USE, "Requested tablespace already in use")
#define ERR_DB2NSF_BAD_OPTIONS2_FLAGS	PKG_DB2NSF+21
	apitext( ERR_DB2NSF_BAD_OPTIONS2_FLAGS, "Invalid Options2 flags value")
#define ERR_DB2NSF_INCONSISTENT_COLLECTION PKG_DB2NSF+22
	apitext( ERR_DB2NSF_INCONSISTENT_COLLECTION, "DB2-based Collection is corrupt")
#define ERR_DB2NSF_OPEN_NOT_ALLOWED     PKG_DB2NSF+23
    apitext( ERR_DB2NSF_OPEN_NOT_ALLOWED, "A DB2-based nsf cannot be opened locally.  It must be opened through the DB2-enabled Domino Server.")
#define ERR_DB2NSF_SERVICES_UNAVAILABLE			PKG_DB2NSF+24
    apitext( ERR_DB2NSF_SERVICES_UNAVAILABLE,	"DB2 services may be unavailable or the DB2-based nsf may be corrupt.")
#define ERR_DB2NSF_XLATE_POOLFULL		PKG_DB2NSF+25
	errortext( ERR_DB2NSF_XLATE_POOLFULL,	"Insufficient memory - DB2 xlate pool is full.")
#define ERR_DB2NSF_IMAP_NOT_SUPPORTED   PKG_DB2NSF+26
	apitext( ERR_DB2NSF_IMAP_NOT_SUPPORTED, "IMAP is not supported for DB2-based nsfs.")
#define ERR_DB2NSF_NO_OVERRIDE_RIGHTS   PKG_DB2NSF+27
	apitext( ERR_DB2NSF_NO_OVERRIDE_RIGHTS, "User does not have datastore override rights.")
#define ERR_DB2NSF_QUERY_TOO_LONG		PKG_DB2NSF+28
	apitext( ERR_DB2NSF_QUERY_TOO_LONG, "DB2 query exceeds maximum length." )
#define ERR_DB2NSF_BUF_TOO_SMALL        PKG_DB2NSF+29
    apitext(ERR_DB2NSF_BUF_TOO_SMALL, "Allocated buffer too small")
#define ERR_DB2NSF_INIT_FAILURE	        PKG_DB2NSF+30
	apitext( ERR_DB2NSF_INIT_FAILURE,   "Unable to initialize DB2 services.  DB2-based nsfs will be unusable." )

/* Error codes from the XML mapping file starting with 0x3B30*/
#define  ERR_DB2NSF_WARNING			PKG_DB2NSF+48
	errortext(ERR_DB2NSF_WARNING,		"DB2-SQL warning exception.")
#define  ERR_DB2NSF_NODATA			PKG_DB2NSF+49
	errortext(ERR_DB2NSF_NODATA,		"DB2-SQL no data exception.")
#define  ERR_DB2NSF_DYNAMIC_SQL			PKG_DB2NSF+50
	errortext(ERR_DB2NSF_DYNAMIC_SQL,	"DB2-Dynamic SQL error.")
#define  ERR_DB2NSF_CONNECTION			PKG_DB2NSF+51
	errortext(ERR_DB2NSF_CONNECTION,	"DB2-Connection Exception.")
#define  ERR_DB2NSF_TRIGGER			PKG_DB2NSF+52
	errortext(ERR_DB2NSF_TRIGGER,		"DB2-Triggered action exception.")
#define  ERR_DB2NSF_FEATURE			PKG_DB2NSF+53
	errortext(ERR_DB2NSF_FEATURE,		"DB2-Feature not supported.")
#define  ERR_DB2NSF_TYPE			PKG_DB2NSF+54
	errortext(ERR_DB2NSF_TYPE,		"DB2-Invalid target type specification.")
#define  ERR_DB2NSF_SCHEMA			PKG_DB2NSF+55
	errortext(ERR_DB2NSF_SCHEMA,		"DB2-Invalid Schema name list specification.")
#define  ERR_DB2NSF_TOKEN			PKG_DB2NSF+56
	errortext(ERR_DB2NSF_TOKEN,		"DB2-Invalid token.")
#define  ERR_DB2NSF_0K00			PKG_DB2NSF+57
	errortext(ERR_DB2NSF_0K00,		"DB2-0K00 exception")
#define  ERR_DB2NSF_TRIG_PROHIBIT		PKG_DB2NSF+58
	errortext(ERR_DB2NSF_TRIG_PROHIBIT,	"DB2-Prohibited statement encountered during trigger.")
#define  ERR_DB2NSF_CASE			PKG_DB2NSF+59
	errortext(ERR_DB2NSF_CASE,		"DB2-Case not found for case statement.")
#define  ERR_DB2NSF_CARDINALITY			PKG_DB2NSF+60
	errortext(ERR_DB2NSF_CARDINALITY,	"DB2-Cardinality violation.")
#define  ERR_DB2NSF_DATA			PKG_DB2NSF+61
	errortext(ERR_DB2NSF_DATA,		"DB2-Data exception.")
#define  ERR_DB2NSF_CONSTRAINT			PKG_DB2NSF+62
	errortext(ERR_DB2NSF_CONSTRAINT,	"DB2-Constraint violation.")
#define  ERR_DB2NSF_INV_CURS_STATE		PKG_DB2NSF+63
	errortext(ERR_DB2NSF_INV_CURS_STATE,	"DB2-Invalid cursor state.")
#define  ERR_DB2NSF_INV_TRAN_STATE		PKG_DB2NSF+64
	errortext(ERR_DB2NSF_INV_TRAN_STATE,	"DB2-Invalid transaction state.")
#define  ERR_DB2NSF_INV_STMT_ID			PKG_DB2NSF+65
	errortext(ERR_DB2NSF_INV_STMT_ID,	"DB2-Invalid SQL statement identifier.")
#define  ERR_DB2NSF_INV_AUTH_SPEC		PKG_DB2NSF+66
	errortext(ERR_DB2NSF_INV_AUTH_SPEC,	"DB2-Invalid authorization specification.")
#define  ERR_DB2NSF_INV_TRAN_TERMINATION 	PKG_DB2NSF+67
	errortext(ERR_DB2NSF_INV_TRAN_TERMINATION,	"DB2-Invalid transaction termination.")
#define  ERR_DB2NSF_INV_CONN_NAME		PKG_DB2NSF+68
	errortext(ERR_DB2NSF_INV_CONN_NAME,	"DB2-Invalid connection name.")
#define  ERR_DB2NSF_INV_CURS_NAME		PKG_DB2NSF+69
	errortext(ERR_DB2NSF_INV_CURS_NAME,	"DB2-Invalid cursor name.")
#define  ERR_DB2NSF_36001			PKG_DB2NSF+70
	errortext(ERR_DB2NSF_36001,		"DB2-36001 error.")
#define  ERR_DB2NSF_SYNTAX			PKG_DB2NSF+71
	errortext(ERR_DB2NSF_SYNTAX,		"DB2-Syntax error.")
#define  ERR_DB2NSF_EXT_FUNC_EX			PKG_DB2NSF+72
	errortext(ERR_DB2NSF_EXT_FUNC_EX,	"DB2-External function exception.")
#define  ERR_DB2NSF_EXT_FUNC_CALL_EX		PKG_DB2NSF+73
	errortext(ERR_DB2NSF_EXT_FUNC_CALL_EX,	"DB2-External function call exception.")
#define  ERR_DB2NSF_SAVEP_EX			PKG_DB2NSF+74
	errortext(ERR_DB2NSF_SAVEP_EX,		"DB2-Savepoint exception.")
#define  ERR_DB2NSF_SERIALIZATION		PKG_DB2NSF+75
	errortext(ERR_DB2NSF_SERIALIZATION,	"DB2-Serialization failure.")
#define  ERR_DB2NSF_AUTHORIZATION		PKG_DB2NSF+76
	errortext(ERR_DB2NSF_AUTHORIZATION,	"DB2-Authorization violation.")
#define  ERR_DB2NSF_VIEW_DEF			PKG_DB2NSF+77
	errortext(ERR_DB2NSF_VIEW_DEF,		"DB2-View definition error.")
#define  ERR_DB2NSF_46XXX			PKG_DB2NSF+78
	errortext(ERR_DB2NSF_46XXX,		"DB2-46XXX error.")
#define  ERR_DB2NSF_INV_APP_STATE		PKG_DB2NSF+79
	errortext(ERR_DB2NSF_INV_APP_STATE,	"DB2-Invalid application state.")
#define  ERR_DB2NSF_DUP_UNDEF_NAME		PKG_DB2NSF+80
	errortext(ERR_DB2NSF_DUP_UNDEF_NAME,	"DB2-Duplicate or undefined name.")
#define  ERR_DB2NSF_INV_OPERAND			PKG_DB2NSF+81
	errortext(ERR_DB2NSF_INV_OPERAND,	"DB2-Invalid operand or inconsistent specification.")
#define  ERR_DB2NSF_LIMIT_EXCEEDED		PKG_DB2NSF+82
	errortext(ERR_DB2NSF_LIMIT_EXCEEDED,	"DB2-SQL or Product limit exceeded.")
#define  ERR_DB2NSF_PREREQ_STATE		PKG_DB2NSF+83
	errortext(ERR_DB2NSF_PREREQ_STATE,	"DB2-Object not in prerequisite state.")
#define  ERR_DB2NSF_MISC_RESTRICTIONS		PKG_DB2NSF+84
	errortext(ERR_DB2NSF_MISC_RESTRICTIONS,	"DB2-Miscellaneous SQL or product restrictions.")
#define  ERR_DB2NSF_RESOURCE			PKG_DB2NSF+85
	errortext(ERR_DB2NSF_RESOURCE,		"DB2-Resource not available or operator intervention.")
#define  ERR_DB2NSF_SYSTEM			PKG_DB2NSF+86
	errortext(ERR_DB2NSF_SYSTEM,		"DB2-System error.")
#define  ERR_DB2NSF_NOXMLMAP			PKG_DB2NSF+87
	errortext(ERR_DB2NSF_NOXMLMAP,		"File db2errmp.xml not found.")
#define  ERR_DB2NSF_MAPSUCCESS			PKG_DB2NSF+88
	errortext(ERR_DB2NSF_MAPSUCCESS,	"The map for DB2 errors was successfully created.\n")
#define  ERR_DB2NSF_XML_PARSE			PKG_DB2NSF+89
	errortext(ERR_DB2NSF_XML_PARSE, 	"The parsing of XML DB2 error file failed; the map for DB2 errors was not created.")
#define  ERR_DB2NSF_GENERIC_FAILURE		PKG_DB2NSF+90
	errortext(ERR_DB2NSF_GENERIC_FAILURE, 	"DB2 NSF failure.")
#define  ERR_DB2NSF_ENABLED				PKG_DB2NSF+91
	stringtext(ERR_DB2NSF_ENABLED, 		"DB2 Server:|Not Enabled|Enabled|DB2 Access:")
#define IDX_DB2_ENABLED_SERVER			1
#define IDX_DB2_ENABLED_NOT_EN			2
#define IDX_DB2_ENABLED_EN				3
#define IDX_DB2_ENABLED_ACCESS			4
#define  ERR_DB2NSF_DB					PKG_DB2NSF+92
	stringtext(ERR_DB2NSF_DB, 			"DB2 Database: ")
#define  ERR_DB2NSF_BUFFERPOOL			PKG_DB2NSF+93
	errortext(ERR_DB2NSF_BUFFERPOOL,	"DB2 - Buffer pool not allocated, using a buffer pool that may be too small.")
#define  ERR_DB2NSF_ENABLE_STR			PKG_DB2NSF+94
	errortext(ERR_DB2NSF_ENABLE_STR,	"DB2 SERVER ENABLE LOG\n")
#define  ERR_DB2NSF_ENABLE_STATE		PKG_DB2NSF+95
	errortext(ERR_DB2NSF_ENABLE_STATE,	"Server doc update|Notes.ini update|Server id update|Catalog node|Load DB2 Library|Create user account|Add DB2DOM group to SYSCTRL_GROUP|Send server doc update request to Admin Process")
#define IDX_DB2_ENABLE_SRVDOC			1
#define IDX_DB2_ENABLE_INI				2
#define IDX_DB2_ENABLE_ID				3
#define IDX_DB2_ENABLE_CATNODE			4
#define IDX_DB2_ENABLE_LOADLIB			5
#define IDX_DB2_ENABLE_USER				6
#define IDX_DB2_ENABLE_GROUP			7
#define IDX_DB2_ENABLE_SRVDOC_ADMINP	8

#define  ERR_DB2NSF_ENABLE_STATUS		PKG_DB2NSF+96
	errortext(ERR_DB2NSF_ENABLE_STATUS,	"TASK -- |started\n|completed\n|failed with the preceding error(s)\n|%s%sfailed with error (%s)\n")
#define IDX_DB2_ENABLE_TASK			1
#define IDX_DB2_ENABLE_START		2
#define IDX_DB2_ENABLE_COMP			3
#define IDX_DB2_ENABLE_ERROR		4
#define IDX_DB2_ENABLE_ERROR_STRING	5
#define  ERR_DB2NSF_XMLFILE			PKG_DB2NSF+97
	errortext(ERR_DB2NSF_XMLFILE,	"The XML file db2errmp.xml might be incomplete or corrupt.  A partial error map was created.")
#define	 ERR_DB2NSF_TABLESPACE_MISSING	PKG_DB2NSF+98
	errortext(ERR_DB2NSF_TABLESPACE_MISSING, "Tablespace (%s) does not exist")
#define  ERR_DB2NSF_SCHEMA_MISSING		PKG_DB2NSF+99
    errortext(ERR_DB2NSF_SCHEMA_MISSING, "Schema (%s) does not exist")
#define ERR_DB2NSF_DAVPOP_BUSY			PKG_DB2NSF+100
	errortext(ERR_DB2NSF_DAVPOP_BUSY,	"DB2 Access View Populator is already handling this population request." )
#define ERR_DB2NSF_XACL_NOT_SUPPORTED	PKG_DB2NSF+101
	errortext(ERR_DB2NSF_XACL_NOT_SUPPORTED,	"Extended ACL is not supported for DB2-based nsfs." )
#define ERR_DB2CREATE_TS				PKG_DB2NSF+102
	errortext(ERR_DB2CREATE_TS,	    	"Tablespace container path is too long.")
#define ERR_DB2LOAD_ACC_DEF				PKG_DB2NSF+103
	errortext(ERR_DB2LOAD_ACC_DEF,		"Failed when loading the access definition.")	
#define ERR_DB2READ_NOTE				PKG_DB2NSF+104
	errortext(ERR_DB2READ_NOTE,			"Failed when DB2 was reading a note.")
#define ERR_DB2WRITE_RNOTE				PKG_DB2NSF+105
	errortext(ERR_DB2WRITE_RNOTE,		"Failed when DB2 was writing a row in a note.")
#define ERR_DB2GET_NSF_SIZE				PKG_DB2NSF+106
	errortext(ERR_DB2GET_NSF_SIZE,		"Failed calculating DB2 NSF size.")
#define ERR_DB2CREATEDB2				PKG_DB2NSF+107
	errortext(ERR_DB2CREATEDB2,			"Failed creating DB2 Database.")
#define ERR_DB2NSF_WRONGSCHEMA			PKG_DB2NSF+108
	errortext(ERR_DB2NSF_WRONGSCHEMA,	"DB2 Database has the incorrect schema level and cannot be used by this server." )
#define ERR_DB2ACC_CN_NAME			    PKG_DB2NSF+109
	errortext(ERR_DB2ACC_CN_NAME,	    "Failed to obtain full canonical name from person document." )
#define ERR_DB2ACC_DB2_NAME			    PKG_DB2NSF+110
	errortext(ERR_DB2ACC_DB2_NAME,	    "Failed to obtain DB2 account name from person document." )
#define ERR_DB2_MULTIPLENAMES			PKG_DB2NSF+111
	errortext(ERR_DB2_MULTIPLENAMES,	"Invalid DB2 user name mapping.  Multiple Notes users mapped to a single DB2 user are not allowed. Contact your administrator.")
#define ERR_DB2_DB2ACCESSMAPPEDERR		PKG_DB2NSF+112
	errortext(ERR_DB2_DB2ACCESSMAPPEDERR,	"A DB2 Access Error occurred.  Server log will contain more information.")
#define ERR_DB2NSF_DB2_CFG_SET		    PKG_DB2NSF+113
	errortext(ERR_DB2NSF_DB2_CFG_SET,   "Error setting configuration parms for DB2 database." )
#define ERR_DB2NSF_FORMAT				PKG_DB2NSF+114
	errortext(ERR_DB2NSF_FORMAT,		"%s  %s  %s: %s - %s - %s" )

/* there may be some available PKG_DB2NSF offsets above... */
/* this is the *current* max; PKG_DB2NSF offsets range from 0 to 255 */
#define ERR_DB2NSF_MAX				PKG_DB2NSF+115
	errortext(ERR_DB2NSF_MAX,	"This is the last error message in the DB2NSF error range")

/* #endif */

/*	End of Error Codes */

/*	Formula Compiler Error Codes */

#define	ERR_FORMULA_COMPILATION	PKG_FORMULA+1
	errortext(ERR_FORMULA_COMPILATION, "Formula Error")
#define	ERR_INVALID_OPERATOR	PKG_FORMULA+2
	errortext(ERR_INVALID_OPERATOR, "Invalid operator")
#define	ERR_INVALID_ARG_TYPE	PKG_FORMULA+3
	errortext(ERR_INVALID_ARG_TYPE, "Invalid type of argument")
#define	ERR_TOO_MANY_ARGS		PKG_FORMULA+4
	errortext(ERR_TOO_MANY_ARGS, "Too many arguments for @Function")
#define	ERR_FN_OR_OP_EXPECTED	PKG_FORMULA+5
	errortext(ERR_FN_OR_OP_EXPECTED, "@Function or operator expected, or @Function does not require an argument")
#define	ERR_INSUFFICIENT_ARGS	PKG_FORMULA+6
	errortext(ERR_INSUFFICIENT_ARGS, "Insufficient arguments for @Function")
#define	ERR_NOT_UNARY			PKG_FORMULA+7
	errortext(ERR_NOT_UNARY, 	"Inappropriate (unary) usage of an operator")
#define	ERR_NOT_BINARY			PKG_FORMULA+8
	errortext(ERR_NOT_BINARY,	"Inappropriate (binary) usage of an operator")
#define	ERR_NESTED_ASSIGN		PKG_FORMULA+9
	errortext(ERR_NESTED_ASSIGN,"Assignment operators must not be preceded by other operators")
#define	ERR_PREMATURE_EOI		PKG_FORMULA+10
	errortext(ERR_PREMATURE_EOI,"Formula incomplete")
#define	ERR_OP_EXPECTED			PKG_FORMULA+11
	errortext(ERR_OP_EXPECTED, "An operator or semicolon was expected but none was encountered")
#define	ERR_INVALID_FORMULA		PKG_FORMULA+12
	errortext(ERR_INVALID_FORMULA, "Invalid formula structure")
#define	ERR_PAREN_EXPECTED		PKG_FORMULA+13
	errortext(ERR_PAREN_EXPECTED, "Left parenthesis expected")
#define	ERR_EXTRA_RPAREN		PKG_FORMULA+14
	errortext(ERR_EXTRA_RPAREN, "Extra right parenthesis")
#define	ERR_MISSING_RPAREN		PKG_FORMULA+15
	errortext(ERR_MISSING_RPAREN, "Missing right parenthesis")
#define	ERR_ASSIGNMENT_POS		PKG_FORMULA+16
	errortext(ERR_ASSIGNMENT_POS, "FIELD must be followed by a variable name and :=")
#define	ERR_ASSIGNMENT_TARGET	PKG_FORMULA+17
	errortext(ERR_ASSIGNMENT_TARGET, ":= must be immediately preceded by a field or variable name")
#define	ERR_ASSIGNMENT_MODIFIER	PKG_FORMULA+18
	errortext(ERR_ASSIGNMENT_MODIFIER, "Only ENVIRONMENT, DEFAULT, or FIELD can be used as a modifier to :=")
#define	ERR_MISSING_ASS_SEP		PKG_FORMULA+19
	errortext(ERR_MISSING_ASS_SEP, "Missing semicolon")
#define	ERR_REM_FORMAT			PKG_FORMULA+20
	errortext(ERR_REM_FORMAT, "REM must be followed by a quoted string containing the remark.")
#define	ERR_NO_MAIN_EXPRESSION	PKG_FORMULA+21
	errortext(ERR_NO_MAIN_EXPRESSION, "No main or selection expression in formula")
#define	ERR_NOT_A_FUNCTION		PKG_FORMULA+22
	errortext(ERR_NOT_A_FUNCTION, "Passing arguments to a non-@Function or to an @Function that doesn't require arguments")
#define	ERR_UNTERMINATED_LITERAL PKG_FORMULA+23
	errortext(ERR_UNTERMINATED_LITERAL, "End of formula reached before end of quoted string, date/time or array accessor")
#define	ERR_IF_ODD_ARGS			PKG_FORMULA+24
	errortext(ERR_IF_ODD_ARGS, "@If must have an odd number of arguments")
#define	ERR_ONE_MAIN_EXPR		PKG_FORMULA+25
	errortext(ERR_ONE_MAIN_EXPR, "Only one SELECT expression is allowed")
#define	ERR_NO_FORMULA			PKG_FORMULA+26
	errortext(ERR_NO_FORMULA,   "No formula specified")
#define	ERR_UNKNOWN_FUNCTION	PKG_FORMULA+27
	errortext(ERR_UNKNOWN_FUNCTION, "Unknown @Function")
#define	ERR_SUMMITEMS_MERGE		PKG_FORMULA+28
	debugtext(ERR_SUMMITEMS_MERGE, "EXPRESSION_SUMMITEMS present multiple times!")
#define	ERR_DUPLICATE_IVAR		PKG_FORMULA+29
	errortext(ERR_DUPLICATE_IVAR, "Variable already defined")
#define	ERR_ARGS_EXPECTED		PKG_FORMULA+30
	errortext(ERR_ARGS_EXPECTED, "@Function arguments expected but none were supplied")
#define	ERR_BIG_LITERAL PKG_FORMULA+31
	errortext(ERR_BIG_LITERAL, "Quoted string is too long.")
#define	ERR_MISSING_ASS			PKG_FORMULA+32
	errortext(ERR_MISSING_ASS, ":= must be used in conjunction with FIELD, ENVIRONMENT, or DEFAULT")
#define	ERR_UNKNOWN_FKWD		PKG_FORMULA+33
	errortext(ERR_UNKNOWN_FKWD, "Unknown [KeyWord] for @Function")
#define	ERR_NO_SELECT_EXPR		PKG_FORMULA+34
	errortext(ERR_NO_SELECT_EXPR, "SELECT is inappropriate for this type of formula")
#define ERR_FORMULA_HIDDEN		PKG_FORMULA+35
	errortext(ERR_FORMULA_HIDDEN, "Formula is hidden")
#define ERR_IVAR_NAME_TOO_LONG		PKG_FORMULA+36
	errortext(ERR_IVAR_NAME_TOO_LONG, "Variable name is too long")
#define ERR_EXTRA_BRACKET		PKG_FORMULA+37
	errortext(ERR_EXTRA_BRACKET, "Extra right bracket found")
#define ERR_FIELD_HAS_NO_VALUE	PKG_FORMULA+38
	errortext(ERR_FIELD_HAS_NO_VALUE, "Field name requires a value")

/*	Compute Error Codes */

#define	ERR_OP_INVALID			PKG_FORMULA+50
	errortext(ERR_OP_INVALID, "Invalid formula: unknown function/operator")
#define	ERR_OP_UNIMPLEMENTED	PKG_FORMULA+51
	errortext(ERR_OP_UNIMPLEMENTED, "Invalid formula: unimplemented function/operator")
#define	ERR_INVALID_TYPE_FOR_OP	PKG_FORMULA+52
	errortext(ERR_INVALID_TYPE_FOR_OP, "Incorrect data type for operator or @Function")
#define	ERR_ARGS_INVALID		PKG_FORMULA+53
	errortext(ERR_ARGS_INVALID, "Invalid formula format: bad number of arguments")
#define	ERR_FORMULA_GENERATED	PKG_FORMULA+54
	errortext(ERR_FORMULA_GENERATED, "ERROR specified in formula")
#define	ERR_STRING_TOO_LONG		PKG_FORMULA+55
	errortext(ERR_STRING_TOO_LONG, "String values cannot be longer than 65535 bytes")
#define	ERR_NO_MAIN_RESULT		PKG_FORMULA+56
	errortext(ERR_NO_MAIN_RESULT, "unable to compute formula value")
#define	ERR_USERNAME			PKG_FORMULA+57
	errortext(ERR_USERNAME,		 "USERNAME is not configured properly")
#define	ERR_SUBSET				PKG_FORMULA+58
	errortext(ERR_SUBSET,		 "The second argument to @Subset must not be zero")
#define	ERR_FORMULA_EXEC		PKG_FORMULA+59
	errortext(ERR_FORMULA_EXEC,	 "Indirect Formula Computation not allowed")
#define	ERR_TEXT_EXPECTED		PKG_FORMULA+60
	errortext(ERR_TEXT_EXPECTED, "Incorrect data type for operator or @Function: Text expected")
#define	ERR_NUMBER_EXPECTED		PKG_FORMULA+61
	errortext(ERR_NUMBER_EXPECTED, "Incorrect data type for operator or @Function: Number expected")
#define	ERR_TIME_EXPECTED		PKG_FORMULA+62
	errortext(ERR_TIME_EXPECTED, "Incorrect data type for operator or @Function: Time/Date expected")
#define	ERR_NTD_EXPECTED		PKG_FORMULA+63
	errortext(ERR_NTD_EXPECTED, "Incorrect data type: Text, Number, or Time/Date expected")
#define	ERR_DATETIME_FMT		PKG_FORMULA+64
	errortext(ERR_DATETIME_FMT, "The @Date and @Time functions must have exactly 1, 3, or 6 arguments.")
#define	ERR_BINOP_TYPES			PKG_FORMULA+65
	errortext(ERR_BINOP_TYPES, 	"Comparison operators must be supplied two values of the same data type.")
#define	ERR_NONAMES				PKG_FORMULA+66
	errortext(ERR_NONAMES,	 	"This view is obsolete and requires missing field list.  Please edit and re-save the view.")
#define	ERR_COMPUTE_LOCALMEM	PKG_FORMULA+67
	errortext(ERR_COMPUTE_LOCALMEM,	"Formula or value requires too much memory")
#define	ERR_COMPUTE_FULL		PKG_FORMULA+68
	errortext(ERR_COMPUTE_FULL,	"Too many concurrent formula evaluations; please retry")
#define	ERR_COMPUTE_EXTENSION	PKG_FORMULA+69
	errortext(ERR_COMPUTE_EXTENSION,"Unknown @Function Extension")
#define	ERR_KEYWORD_EXPECTED	PKG_FORMULA+70
	errortext(ERR_KEYWORD_EXPECTED, "Incorrect data type for operator or @Function: [KeyWord] expected")
#define	ERR_KEYWORD_CONTEXT		PKG_FORMULA+71
	errortext(ERR_KEYWORD_CONTEXT, "The specified [KeyWord] is not appropriate when used with this @Function")
#define	ERR_PROMPT				PKG_FORMULA+72
	errortext(ERR_PROMPT,		"The first argument to @Prompt must be between 1 and 6")
#define	ERR_TOTIME				PKG_FORMULA+73
	errortext(ERR_TOTIME,		"The value cannot be converted to a Time/Date.")
#define	ERR_UNAVAILABLE			PKG_FORMULA+74
	internaltext(ERR_UNAVAILABLE, 	"formula has encountered @DeleteField or @Unavailable")
#define	ERR_TONUMBER				PKG_FORMULA+75
	errortext(ERR_TONUMBER,		"The value cannot be converted to a Number.")
#define ERR_GETDOCFIELD_NOTENOTFOUND	PKG_FORMULA+76
	errortext(ERR_GETDOCFIELD_NOTENOTFOUND,	"Document could not be found.")
#define	ERR_COMPUTE_FIELD_TOO_BIG PKG_FORMULA+77
	errortext(ERR_COMPUTE_FIELD_TOO_BIG,"Fields referenced or generated by a formula may not exceed 20,000 bytes.")
#define	ERR_ARCHIVE_NO_ARCH_CRITERA	PKG_FORMULA+78
	errortext(ERR_ARCHIVE_NO_ARCH_CRITERA,"No archive criteria found")
#define	ERR_DESCENDANTS			PKG_FORMULA+79
	errortext(ERR_DESCENDANTS,"This database must be compacted to support the use of @AllChildren or @AllDescendants in a formula.")
#define	ERR_PICK_LIST_STYLE		PKG_FORMULA+80
	errortext(ERR_PICK_LIST_STYLE, "The first argument to @PickList must specify a valid @PickList style.")
#define ERR_STRING_BADUNID		 PKG_FORMULA+81
	errortext(ERR_STRING_BADUNID,	"Invalid UNID; UNID must be a 32-digit hex string.")
#define	ERR_ABSTRACT_BADSIZE	PKG_FORMULA+82
	errortext(ERR_ABSTRACT_BADSIZE, "The buffer size argument of @Abstract must be greater than zero")
#define	ERR_EXPLODE_BADRANGE	PKG_FORMULA+83
	errortext(ERR_EXPLODE_BADRANGE, "The time-date range argument of @Explode spans too many days")
#define	ERR_COMPUTE_LENGTH_WORD	PKG_FORMULA+84
	errortext(ERR_COMPUTE_LENGTH_WORD, "The result of a computation cannot exceed 65535 bytes")
#define ERR_ADMIN_SELECTION		PKG_FORMULA+85
	errortext(ERR_ADMIN_SELECTION,	"The currently selected note is not from the correct view.")
#define	ERR_URLHISTORY			PKG_FORMULA+86
	errortext(ERR_URLHISTORY,	"The first argument to @URLHistory must be between 1 and 5")
#define ERR_ADMIN_NO_REQ_FIELD	PKG_FORMULA+87
	errortext(ERR_ADMIN_NO_REQ_FIELD,	"A selected note is missing a required field and cannot be acted upon by the Administration Process.")
#define ERR_FUNCTION_CONTEXT	PKG_FORMULA+88
	errortext(ERR_FUNCTION_CONTEXT, "@Function is not valid in this context")
#define ERR_FUNCTION_NOUICOMMANDS	PKG_FORMULA+89
	errortext(ERR_FUNCTION_NOUICOMMANDS,	"@Commands and other UI functions are not allowed in this context")
#define ERR_NSF_COMPUTE_NOASSIGN		PKG_FORMULA+90
	errortext(ERR_NSF_COMPUTE_NOASSIGN,	"You are not allowed to modify fields in this context.")
#define ERR_NSF_COMPUTE_NOSIDE			PKG_FORMULA+91
	errortext(ERR_NSF_COMPUTE_NOSIDE,	"Formulas with side-effects are not allowed in this context.")
#define LOG_MAILSEND_NO_MATCH			PKG_FORMULA+92
	errortext(LOG_MAILSEND_NO_MATCH,   	"Unable to send mail to %s, no match found in Name & Address Book(s)")
#define LOG_MAILSEND_AMBIG_MATCH	   	PKG_FORMULA+93
	errortext(LOG_MAILSEND_AMBIG_MATCH,	"Unable to send mail to %s, multiple matches found in Name & Address Book(s)")
#define	ERR_TOO_MANY_ELEMENTS	PKG_FORMULA+94
	errortext(ERR_TOO_MANY_ELEMENTS, "List values cannot have more than 65535 elements when storing in an item")
#define	ERR_ARRAY_INDEX	PKG_FORMULA+95
	errortext(ERR_ARRAY_INDEX, "Array index out of bounds")
#define	ERR_EVAL_RECURSION_DEPTH PKG_FORMULA+96
	errortext(ERR_EVAL_RECURSION_DEPTH, "@Eval has exceeded the maximum number of recursive calls.")
#define	ERR_COMPUTE_RECURSE_LIMIT		PKG_FORMULA+97
	errortext(ERR_COMPUTE_RECURSE_LIMIT,	"Too many recursive formula evaluations.")
#define	ERR_COMPUTE_TIMEOUT		PKG_FORMULA+98
	errortext(ERR_COMPUTE_TIMEOUT,	"The formula has exceeded the maximum execution time.")
#define	ERR_COMPUTE_MEMORY_MAX		PKG_FORMULA+99
	errortext(ERR_COMPUTE_MEMORY_MAX,	"The formula has exceeded the maximum allowable memory usage.")
#define	ERR_ARCHIVE_SRC_IS_DEST		PKG_FORMULA+100
	errortext(ERR_ARCHIVE_SRC_IS_DEST,"You are not allowed to archive to and from the same database.")
#define ERR_ARC_NO_PROFILE		PKG_FORMULA+101
	errortext(ERR_ARC_NO_PROFILE,	"Archive profile note not found.")

#define	ERR_ARCHIVE_NOT_MIN_SERVER_VER			PKG_FORMULA+102
	errortext(ERR_ARCHIVE_NOT_MIN_SERVER_VER,	"This action is not supported on this version of the Domino Server.")
#define ERR_ARCHIVE_FIELDS_NOT_SIGNED 	 	PKG_FORMULA+103
	errortext(ERR_ARCHIVE_FIELDS_NOT_SIGNED,	"All of the required fields in your archive settings have not been properly signed.  Please re-save your archive settings to correct this problem.")

#define ERR_ARCHIVE_FIELDS_NOT_FOUND 	 	PKG_FORMULA+104
	errortext(ERR_ARCHIVE_FIELDS_NOT_FOUND,	"All of the required fields in your archive settings have not been found.  Please re-save your archive settings to correct this problem.")
#define ERR_COMPUTE_CHARSET_CONVERSION          PKG_FORMULA+105
        errortext(ERR_COMPUTE_CHARSET_CONVERSION, "Error converting string from LMBCS to UNICODE")
#define ERR_ARCHIVE_NOT_ON_SRC_SRV				PKG_FORMULA+106
	errortext(ERR_ARCHIVE_NOT_ON_SRC_SRV,		"The source server specified in your archive settings is not this server. Archiving will not be performed.")
#define ERR_CORRUPTED_COMPUTE_INSTANCE       PKG_FORMULA+107
        errortext(ERR_CORRUPTED_COMPUTE_INSTANCE,       "Internal error:  Corrupted formula instance detected")
#define ERR_COMPUTE_STACK_OVERFLOW      PKG_FORMULA+108
	errortext(ERR_COMPUTE_STACK_OVERFLOW, "Stack overflow evaluating formula. Try simplifying the formula.")
#define	ERR_ARCHIVE_CRIT_NO_SUP_ON_SRC			PKG_FORMULA+109
	errortext(ERR_ARCHIVE_CRIT_NO_SUP_ON_SRC,	"'Older than' selection criteria is not supported on the server chosen to archive from, because its version is prior to 7.0.  One or more enabled criteria use this selection criteria and will not be executed.")
	
/* Note PKG_FORMULA is limited to 0 - 127 */

/* Note Storage File Error Code Definitions */

#define ERR_RM_FAIL_RESTART_OPEN PKG_NSF+0
	errortext(ERR_RM_FAIL_RESTART_OPEN, "Recovery Manager: Logged Database cannot be accessed during Restart/Media Recovery Phase")
#define	ERR_NOT_NSF			PKG_NSF+1
	errortext(ERR_NOT_NSF,	"File is not a database")
#define	ERR_IGNORE_NOTE		PKG_NSF+2
	internaltext(ERR_IGNORE_NOTE,	"ignore this note during copy")
#define ERR_WRITEONLY		PKG_NSF+3
	errortext(ERR_WRITEONLY,"Write-only database")
#define	ERR_READONLY		PKG_NSF+4
	errortext(ERR_READONLY,	"Read-only database")
#define ERR_CONFLICT		PKG_NSF+5
	errortext(ERR_CONFLICT,	"Someone else modified this document at the same time")
#define ERR_STRUCT			PKG_NSF+6
	errortext(ERR_STRUCT,	"The program cannot interpret this file due to an invalid structure")
#define ERR_DOC_STRUCT		PKG_NSF+7
	errortext(ERR_DOC_STRUCT,"Document has invalid structure")
#define ERR_VALUE_FLAGS		PKG_NSF+8
	internaltext(ERR_VALUE_FLAGS,"Invalid combination of flags")
#define ERR_BUCKET_FULL		PKG_NSF+9
	errortext(ERR_BUCKET_FULL,"The text fields of the document are too large to be stored.  Please reduce their size.")
#define	ERR_UNK				PKG_NSF+10
	internaltext(ERR_UNK,	"UNK does not exist")
#define ERR_BUCKET				PKG_NSF+11
	internaltext(ERR_BUCKET,"Invalid pos, not in BUCKET areas")
#define ERR_BUCKET_NOT_IN_MEM	PKG_NSF+12
	internaltext(ERR_BUCKET_NOT_IN_MEM, "(bucket not found)")
#define ERR_NO_FILE_SHARING	PKG_NSF+13
	errortext(ERR_NO_FILE_SHARING,	"This database is currently being used by someone else.  In order to share a Notes database, all users must use a Domino Server instead of a File Server.")
#define ERR_NSF_NOTE_POOLFULL	PKG_NSF+14
	errortext(ERR_NSF_NOTE_POOLFULL, "Insufficient memory - Open Note Pool is full.")
#define ERR_NOTE_EMPTY		PKG_NSF+15
	errortext(ERR_NOTE_EMPTY,	"Document has nothing to undelete")
#define ERR_NO_MORE_MATCHES	PKG_NSF+16
	internaltext(ERR_NO_MORE_MATCHES, "(Search status - no more to do)")
#define ERR_FORMULA			PKG_NSF+17
	errortext(ERR_FORMULA,	"Invalid formula format")
#define ERR_RM_PARTIAL_WRITE_DETECTED	PKG_NSF+18
	errortext(ERR_RM_PARTIAL_WRITE_DETECTED, "Recovery Manager: Partial Write detected during Redo, marking database Corrupt.")
#define ERR_OPERATOR		PKG_NSF+19
	errortext(ERR_OPERATOR,	"Invalid operator for data type")
#define ERR_NOTE_ID			PKG_NSF+20
	errortext(ERR_NOTE_ID,	"Invalid document identifier")
#define ERR_NEW_NAME_KEY	PKG_NSF+21
	internaltext(ERR_NEW_NAME_KEY,"New name key being added")
#define ERR_EVENT_TOO_BIG	PKG_NSF+22
	apitext(ERR_EVENT_TOO_BIG,"Event struct size plus data is bigger than 256 bytes")
#define DBD_NOCACHE_STRING	PKG_NSF+23
	stringtext(DBD_NOCACHE_STRING,"NoCache")
#define ERR_DBD_TOO_MANY_DRIVERS PKG_NSF+24
	errortext(ERR_DBD_TOO_MANY_DRIVERS,"You may not use that many different database drivers in any one application.")
#define ERR_NSF_VERSION		PKG_NSF+25
	errortext(ERR_NSF_VERSION, "Invalid NSF version")
#define	ERR_SEARCH_FLAGS		PKG_NSF+26
	apitext(ERR_SEARCH_FLAGS, "Search flags are incompatible")
#define	ERR_FULL_RECALC		PKG_NSF+27
	internaltext(ERR_FULL_RECALC, "(full recalc necessary)")
#define	ERR_DIRECTORY		PKG_NSF+28
	errortext(ERR_DIRECTORY,	"This function is inappropriate for file system directories.")
#define	ERR_NOT_DIRECTORY	PKG_NSF+29
	errortext(ERR_NOT_DIRECTORY, "This function is only appropriate for directories.")
#define	ERR_ITEM_DEF_TYPE	PKG_NSF+30
	internaltext(ERR_ITEM_DEF_TYPE, "Cannot add item def - type unknown")
#define	ERR_BUCKET_STRUCT		PKG_NSF+31
	internaltext(ERR_BUCKET_STRUCT,	"Invalid structure in bucket")
#define	ERR_NOT_LOCAL		PKG_NSF+32
	apitext(ERR_NOT_LOCAL,	"Function is not supported for remote databases")
#define	ERR_DUP_SUMM_ITEM	PKG_NSF+33
	errortext(ERR_DUP_SUMM_ITEM, "Duplicate field in document")
#define	ERR_ITEM_NOT_FOUND	PKG_NSF+34
	apitext(ERR_ITEM_NOT_FOUND,"Note item not found")
#define ERR_ENTRY_NOT_FOUND		PKG_NSF+35
	apitext(ERR_ENTRY_NOT_FOUND,	"Cache entry not found")
#define	ERR_FMARKER			PKG_NSF+36
	internaltext(ERR_FMARKER,"Missing formula marker in merge formula")
#define	ERR_NOTE_DELETED	PKG_NSF+37
	errortext(ERR_NOTE_DELETED,	"Document has been deleted")
#define	ERR_UNK_TYPE		PKG_NSF+38
	errortext(ERR_UNK_TYPE,	"Field type (UNK) stored in document is invalid")
#define	ERR_INVALID_NOTE	PKG_NSF+39
	errortext(ERR_INVALID_NOTE,	"Invalid or nonexistent document")
#define	ERR_AUXCLASS		PKG_NSF+40
	internaltext(ERR_AUXCLASS,	"Invalid usage of Auxiliary Class Field")
#define	ERR_NSFOPEN			PKG_NSF+41
	errortext(ERR_NSFOPEN,	"Database is currently in use by you or another user")
#define	ERR_NOTE_NOT_EMPTY	PKG_NSF+42
	internaltext(ERR_NOTE_NOT_EMPTY,"note pool not deleted because it is not empty")
#define	ERR_NOTE_NONDISCARDABLE PKG_NSF+43
	internaltext(ERR_NOTE_NONDISCARDABLE,"note pool not discardable")
#define	ERR_NOTE_DISCARDED	PKG_NSF+44
	internaltext(ERR_NOTE_DISCARDED,"note pool has been discarded")
#define	ERR_NSF_INFO_SIZE	PKG_NSF+45
	errortext(ERR_NSF_INFO_SIZE,"Database title is too large; please use a shorter title.")
#define	ERR_NSF_IN_USE		PKG_NSF+46
	errortext(ERR_NSF_IN_USE,"Cannot do that to an NSF that may be in use")
#define	ERR_INVALID_NAME	PKG_NSF+47
	errortext(ERR_INVALID_NAME,"Remote pathname must be relative to Data Directory")
#define ERR_NSF_POOLFULL	PKG_NSF+48
	errortext(ERR_NSF_POOLFULL, "Insufficient memory - NSF pool is full.")
#define	ERR_SUMMARY_TOO_BIG PKG_NSF+49
	errortext(ERR_SUMMARY_TOO_BIG,"Field is too large (32K) or View's column & selection formulas are too large")
#define	ERR_DBCLASS			PKG_NSF+50
	errortext(ERR_DBCLASS,	"Invalid NSF Class")
#define	ERR_TOOHUGE			PKG_NSF+51
	errortext(ERR_TOOHUGE,	"Object too large to allocate given file params")
#define	ERR_NIFNOTE			PKG_NSF+52
	errortext(ERR_NIFNOTE,	"Cannot create document in this type of file")
#define	ERR_NOFILESPACE		PKG_NSF+53
	errortext(ERR_NOFILESPACE,"Cannot allocate database object - database space exhausted")
#define	ERR_BADOBJECT		PKG_NSF+54
	internaltext(ERR_BADOBJECT,"Invalid Object ID")
#define ERR_TRANSLOG_MISSING_SEGS		PKG_NSF+55
	errortext(ERR_TRANSLOG_MISSING_SEGS,	"Missing or damaged transaction log segment.")
#define	ERR_NSF_CORRUPT		PKG_NSF+56
	errortext(ERR_NSF_CORRUPT,"Database is corrupt -- Cannot allocate space")
#define	ERR_SOURCE_DB_CLOSED PKG_NSF+57
	errortext(ERR_SOURCE_DB_CLOSED,"Cannot access the document's original database to copy attachments")
#define	ERR_ENCODING		PKG_NSF+58
	errortext(ERR_ENCODING,"Unknown type of compression technique")
#define	ERR_NO_CLIENT		PKG_NSF+59
	debugtext(ERR_NO_CLIENT,"Remote operations not allowed in standalone programs")
#define	ERR_NO_MODIFIED_NOTES PKG_NSF+60
	errortext(ERR_NO_MODIFIED_NOTES,"No documents have been modified since specified time.")
#define	ERR_NO_STAMPED_NOTES PKG_NSF+61
	errortext(ERR_NO_STAMPED_NOTES,"No documents were categorized")
#define	ERR_NO_ITEM_IN_NOTE	PKG_NSF+62
	internaltext(ERR_NO_ITEM_IN_NOTE,"(no such item in document)")
#define	ERR_ITEM_DATATYPE	PKG_NSF+63
	errortext(ERR_ITEM_DATATYPE,"field in note has wrong datatype")
#define	ERR_ITEM_LENGTH		PKG_NSF+64
	errortext(ERR_ITEM_LENGTH,"Field length stored in document is incorrect")
#define	ERR_BAD_PARAM		PKG_NSF+65
	internaltext(ERR_BAD_PARAM,	 "(invalid usage - see NSF documentation)")
#define	ERR_SPECIAL_ID		PKG_NSF+66
	errortext(ERR_SPECIAL_ID,	 "Special database object cannot be located")
#define	ERR_INVALID_ITEMLEN	PKG_NSF+67
	errortext(ERR_INVALID_ITEMLEN,	"Document is damaged or obsolete (incorrect field length)")
#define	ERR_INVALID_ITEMTYPE	PKG_NSF+68
	errortext(ERR_INVALID_ITEMTYPE,	"Document is damaged or obsolete (unrecognized data type)")
#define	ERR_COMPILER_LOAD		PKG_NSF+69
	errortext(ERR_COMPILER_LOAD,	"Insufficient memory to load Formula Compiler/Decompiler, or cannot locate the appropriate program file")
#define	ERR_NOACCESS			PKG_NSF+70
	errortext(ERR_NOACCESS,			"You are not authorized to perform that operation")
#define	ERR_NO_HELP_INDEX		PKG_NSF+71
	errortext(ERR_NO_HELP_INDEX,	"Help Index cannot be located")
#define	ERR_HELP_SECTION		PKG_NSF+72
	errortext(ERR_HELP_SECTION,		"Notes can't automatically display Help on this section, but you can open the Notes Help database and search its Index.")
#define	ERR_HELP_CATEGORY		PKG_NSF+73
	errortext(ERR_HELP_CATEGORY,	"Notes can't automatically display Help on this subject, but you can open the Notes Help database and search its Index.  For Help on server administration, add and open the Domino Administrator Help database.")
#define	ERR_HELP_TOOBIG			PKG_NSF+74
	internaltext(ERR_HELP_TOOBIG,	"(help string too large)")
#define	ERR_OPEN_FILE			PKG_NSF+75
	errortext(ERR_OPEN_FILE,		"You are not authorized to access that database")
#define	ERR_DEL_FILE			PKG_NSF+76
	errortext(ERR_DEL_FILE,			"You are not authorized to delete that database")
#define	ERR_UPDATE_CLASS		PKG_NSF+77
	internaltext(ERR_UPDATE_CLASS,	"(you cannot change the class of a note once stored)")
#define	ERR_NOT_AUTHOR			PKG_NSF+78
	errortext(ERR_NOT_AUTHOR,		"You cannot update or delete the document(s) since you are not listed as an allowable Author for this document")
#define	ERR_OBJECT_TRUNCATED	PKG_NSF+79
	errortext(ERR_OBJECT_TRUNCATED,	"File object is truncated - file may have been damaged")
#define	ERR_DBFREE_ARGS			PKG_NSF+80
	debugtext(ERR_DBFREE_ARGS,	"(DbFree: Invalid Disk Position/Size)")
#define	ERR_NTUPDATE_0OID		PKG_NSF+81
	debugtext(ERR_NTUPDATE_0OID,"(NSFNoteUpdate: 0 OID field!)")
#define	ERR_NO_HELP_LINKS		PKG_NSF+82
	errortext(ERR_NO_HELP_LINKS,"Help Indexing Information is not available")
#define	ERR_FMARKERSRC			PKG_NSF+83
	apitext(ERR_FMARKERSRC,		"Formula markers not allowed in merge source formula")
#define	ERR_EXISTS_DIR			PKG_NSF+84
	errortext(ERR_EXISTS_DIR,	"Cannot create database - the specified filename is a directory")
#define	ERR_NTCREATE_LICENSE	PKG_NSF+85
	errortext(ERR_NTCREATE_LICENSE, "Cannot create a document without a valid user license")
#define	ERR_CREATE_FILE			PKG_NSF+86
	errortext(ERR_CREATE_FILE,	"You are not authorized to create new databases on this server")
#define	ERR_CREATE_REP_FILE		PKG_NSF+87
	errortext(ERR_CREATE_REP_FILE,"You are not authorized to create new replica databases on this server")
#define	ERR_DIGEST_LENGTH		PKG_NSF+88
	debugtext(ERR_DIGEST_LENGTH,"Message digest overflowed buffer!")
#define	ERR_NOTE_NOT_SIGNED		PKG_NSF+89
	errortext(ERR_NOTE_NOT_SIGNED,"Document is not signed.")
#define	ERR_NOTE_INVSIG1		PKG_NSF+90
	errortext(ERR_NOTE_INVSIG1,	"Signature on document is invalid (digest length)")
#define	ERR_NOTE_INVSIG2		PKG_NSF+91
	errortext(ERR_NOTE_INVSIG2,	"Document has been modified or corrupted since signed! (data)")
#define	ERR_NOTE_INVSIG3		PKG_NSF+92
	errortext(ERR_NOTE_INVSIG3,"You and signer have no Certificates in common; signer cannot be assumed to be trustworthy.")
#define	ERR_NOTE_INVSIG4		PKG_NSF+93
	errortext(ERR_NOTE_INVSIG4,	"Signature on document is invalid (certifier digest length)")
#define	ERR_NOTE_INVSIG5		PKG_NSF+94
	errortext(ERR_NOTE_INVSIG5,	"Signature has been modified or corrupted since document was signed!")
#define	ERR_NOTE_INVSIG6		PKG_NSF+95
	errortext(ERR_NOTE_INVSIG6,	"Signature on document is invalid (digest length)")
#define	ERR_NOTE_INVSIG7		PKG_NSF+96
	errortext(ERR_NOTE_INVSIG7,	"Document has been modified or corrupted since signed! (signature)")
#define	ERR_NOTE_BADATTSIGN		PKG_NSF+97
	errortext(ERR_NOTE_BADATTSIGN,	"Attachment has been modified or corrupted since signed!")
#define	ERR_NOTE_NOT_SEALED		PKG_NSF+98
	errortext(ERR_NOTE_NOT_SEALED,	"Document is not encrypted.")
#define	ERR_NOTE_ALREADY_SEALED	PKG_NSF+99
	errortext(ERR_NOTE_ALREADY_SEALED,"You cannot encrypt a document that is already encrypted.")
#define	ERR_NOEXTRACT_ENCRYPTED	PKG_NSF+100
	errortext(ERR_NOEXTRACT_ENCRYPTED,"You must supply the bulk decryption key in order to extract this file object.")
#define	ERR_NOT_SEALED_FOR_YOU	PKG_NSF+101
	errortext(ERR_NOT_SEALED_FOR_YOU,"You cannot access portions of this document because it is encrypted and was not intended for you, or you do not have the decryption key.")
#define	ERR_ALREADY_LOCKED		PKG_NSF+102
	errortext(ERR_ALREADY_LOCKED,"Database is currently being replicated or copied elsewhere")
#define	LOG_OVERDBSIZEWARNTHRESHOLD PKG_NSF+103
	errortext(LOG_OVERDBSIZEWARNTHRESHOLD, "Warning, database %s has exceeded its warning size threshold of %d KB by %k.")
#define	ERR_DB_NOT_OPEN			PKG_NSF+104
	errortext(ERR_DB_NOT_OPEN,"Specified database is not currently open")
#define	ERR_STS_STRINGTOOLONG	PKG_NSF+105
	errortext(ERR_STS_STRINGTOOLONG,"Text search string is too long")
#define	ERR_NOACTIVITY			PKG_NSF+106
	errortext(ERR_NOACTIVITY,	"User activity recording is disabled")
#define	ERR_FIELDNAME_TOO_LONG	PKG_NSF+107
	errortext(ERR_FIELDNAME_TOO_LONG,"Field names must be 250 characters or less")
#define	ERR_COMPHALT1			PKG_NSF+108
	debugtext(ERR_COMPHALT1,	"(not enough items written)")
#define	ERR_COMPHALT2			PKG_NSF+109
	debugtext(ERR_COMPHALT2,	"(incorrect length written)")
#define	ERR_COMPHALT3			PKG_NSF+110
	debugtext(ERR_COMPHALT3,	"(unevaluated default summary value)")
#define	ERR_NO_DELETED_NOTES	PKG_NSF+111
	errortext(ERR_NO_DELETED_NOTES,"No documents were deleted")
#define	ERR_DATA_ONLY			PKG_NSF+112
	internaltext(ERR_DATA_ONLY,	"(only data notes may be deleted using this call)")
#define	ERR_REMOTE_UNID			PKG_NSF+113
	internaltext(ERR_REMOTE_UNID,"(you cannot obtain the UNIDs using this call to remote databases)")
#define	ERR_MISSING_PRIVS		PKG_NSF+114
	errortext(ERR_MISSING_PRIVS,"All privileges are required for this operation")
#define	ERR_INVALID_ITEMUNK		PKG_NSF+115
	errortext(ERR_INVALID_ITEMUNK,	"Document is damaged or obsolete (unrecognized field)")
#define ERR_FIXUP_DOC			PKG_NSF+116
	errortext(ERR_FIXUP_DOC,	"Document NT%08lx in database %p is damaged: %e")
#define ERR_FIXUP_DOC_DELETED	PKG_NSF+117
	errortext(ERR_FIXUP_DOC_DELETED,"Document (UNID OF%08lx:%08lx-ON%08lx:%08lx) in database %p has been deleted")
#define ERR_INVALID_UNK_TBL		PKG_NSF+118
	errortext(ERR_INVALID_UNK_TBL,	"Field name table (UNK table) is damaged")
#define ERR_UNK_TBL_REPAIRED	PKG_NSF+119
	errortext(ERR_UNK_TBL_REPAIRED,	"Field name table (UNK table) has been repaired")
#define	ERR_NO_NAMED_KEY		PKG_NSF+120
	errortext(ERR_NO_NAMED_KEY,	"You cannot access portions of this document because it was encrypted and you do not have any of the keys")
#define	ERR_BAD_KEY_DATA		PKG_NSF+121
	errortext(ERR_BAD_KEY_DATA,	"You are unable to decrypt this document because your key is incorrect")
#define ERR_FIXUP_DOC_ITEM		PKG_NSF+122
	errortext(ERR_FIXUP_DOC_ITEM,	"Document NT%08lx in database %p is damaged: %e (Field %s, Datatype %04x)")
#define ERR_PURGING				PKG_NSF+123
	stringtext(ERR_PURGING,			"Releasing unused storage in database %s...")
#define ERR_PURGING_DOCS		PKG_NSF+124
	stringtext(ERR_PURGING_DOCS,		"Purging old documents from database %s...")
#define ERR_PURGE_QUERY			PKG_NSF+125
	errortext(ERR_PURGE_QUERY,		"The Replication Cutoff Date indicates that documents before %z should be purged from the database %s.  Would you like it to be done now?")
#define ERR_ITEM_NAME_HUGE		PKG_NSF+126
	errortext(ERR_ITEM_NAME_HUGE,	"You can only Sign documents whose Field names are < 512 bytes.")
#define ERR_NO_ENCRYPT_FIELDS	PKG_NSF+127
	errortext(ERR_NO_ENCRYPT_FIELDS,"Document does not specify any fields to be encrypted")
#define ERR_FIXING				PKG_NSF+128
	stringtext(ERR_FIXING,			"Performing consistency check on %s...")
#define	ERR_OBJ_GONE			PKG_NSF+129
	errortext(ERR_OBJ_GONE,		"Database object has been deleted")
#define ERR_FORM_DELETED		PKG_NSF+130
	errortext(ERR_FORM_DELETED,"Form has been deleted")
#define ERR_NULL_ITEM_HANDLE	PKG_NSF+131
	debugtext(ERR_NULL_ITEM_HANDLE,"NSFItemAppendByBLOCKID: bhValue of NULL")
#define ERR_FIXUP_BUCKET_REPAIRED	PKG_NSF+132
	errortext(ERR_FIXUP_BUCKET_REPAIRED,"Partially-saved document in database %p has been deleted")
#define	ERR_FILEMAX				PKG_NSF+133
	errortext(ERR_FILEMAX,		"Database (.nsf) has grown too large; use compact to reduce the file size or use use File New Replica to recreate your file with larger capacity.")
#define	ERR_NSF_CORRUPT2		PKG_NSF+134
	errortext(ERR_NSF_CORRUPT2,"Database has been corrupted and can't be repaired; cannot open")
#define	ERR_NSF_UNKTBL2BIG		PKG_NSF+135
	errortext(ERR_NSF_UNKTBL2BIG,"Cannot store document - database has too many unique field names.  Please set the 'Allow more fields in database' option or ask your administrator to compact the database.")
#define	ERR_FILEMAXV1			PKG_NSF+136
	errortext(ERR_FILEMAXV1,	"Notes Version 1 database (.nsf) has grown larger than 40MB; use File Replication New Replica to recreate your file as a Notes Version 4 database (.nsf) capable of 4GB.")
#define	ERR_DESKMAXV1			PKG_NSF+137
	errortext(ERR_DESKMAXV1,	"Notes Version 1 workspace file (DESKTOP.DSK) has grown larger than 1MB; you can delete DESKTOP.DSK if you wish a new one to be created that is capable of 50MB of private views.")
#define	ERR_DESKMAX				PKG_NSF+138
	errortext(ERR_DESKMAX,		"Workspace file (DESKTOP.DSK) has grown too large; remove some of your private views or icons.")
#define ERR_TBD					PKG_NSF+139
	errortext(ERR_TBD,			"Function to-be-defined")
#define ERR_BTNODE_TOO_SMALL	PKG_NSF+140
	errortext(ERR_BTNODE_TOO_SMALL,	"B-tree node too small - Can't hold 3 entries")
#define ERR_BTNODE_TOO_BIG		PKG_NSF+141
	errortext(ERR_BTNODE_TOO_BIG,	"B-tree node is larger than the maximum slot size")
#define ERR_INVALID_BTREE		PKG_NSF+142
	errortext(ERR_INVALID_BTREE, "B-tree structure is invalid")
#define ERR_NO_BUFFER			PKG_NSF+143
#ifdef MAC
	errortext(ERR_NO_BUFFER,"There is not enough memory for a view or database buffer.  Please close any unneeded windows.  You may also increase Notes' available memory by quitting and choosing 'Get Info...' from the Finder.")
#else
	errortext(ERR_NO_BUFFER,"There is not enough memory for a view or database buffer. Increase NSF_BUFFER_POOL_SIZE.")
#endif
#define ERR_INVALID_PAGE		PKG_NSF+144
	errortext(ERR_INVALID_PAGE,	"Page format is incorrect")
#define ERR_MAXBTDEPTH			PKG_NSF+145
	errortext(ERR_MAXBTDEPTH,	"Maximum depth of b-tree index reached")
#define ERR_SHARE_CONFLICT		PKG_NSF+146
	errortext(ERR_SHARE_CONFLICT, "Operation can't be performed while container is in use")
#define ERR_BAD_SNO				PKG_NSF+147
	errortext(ERR_BAD_SNO,		"Attempt to use an invalid slot number")
#define ERR_RM_CANNOT_RECOVER	PKG_NSF+148
	errortext(ERR_RM_CANNOT_RECOVER, "Recovery Manager: Unable to recover DB")
#define ERR_BAD_DBP				PKG_NSF+149
	errortext(ERR_BAD_DBP,		"Attempt to use an invalid database pointer")
#define ERR_BT_NOTOPEN			PKG_NSF+150
	errortext(ERR_BT_NOTOPEN,	"B-tree index has not been opened")
#define ERR_BAD_PNOVEC			PKG_NSF+151
	errortext(ERR_BAD_PNOVEC,	"Invalid PNO vector - position == 0")
#define ERR_NOTA_IDB			PKG_NSF+152
	debugtext(ERR_NOTA_IDB,		"Block is not a IDB")
#define ERR_NOTA_DBBUF			PKG_NSF+153
	debugtext(ERR_NOTA_DBBUF,	"Block is not a DBBUF")
#define ERR_NOTA_DBBUFCTL		PKG_NSF+154
	debugtext(ERR_NOTA_DBBUFCTL, "Block is not a DBBUFCTL")
#define ERR_NOTA_DBCONT			PKG_NSF+155
	debugtext(ERR_NOTA_DBCONT,	"Block is not a DBCONT")
#define ERR_FILE_ALLOCATED		PKG_NSF+156
	errortext(ERR_FILE_ALLOCATED, "Container not found - new context allocated")
#define ERR_NOTA_DBB_HASH		PKG_NSF+157
	debugtext(ERR_NOTA_DBB_HASH, "Block is not a DBB_HASH")
#define ERR_NO_SPACE			PKG_NSF+158
	errortext(ERR_NO_SPACE,		"No space on page for slot")
#define ERR_INTEGRITY			PKG_NSF+159
	errortext(ERR_INTEGRITY,	"The integrity of a database storage container has been lost - the container will be rebuilt.")
#define	ERR_NSF_NOV2  			PKG_NSF+160
	errortext(ERR_NSF_NOV2,	"This function cannot be performed in conjunction with a Notes Version 2 Server")
#define	ERR_NSF_INTERRUPT  		PKG_NSF+161
	errortext(ERR_NSF_INTERRUPT,"Your connection to the server unexpectedly dropped; please retry the operation")
#define NSFSTR_LINK_STR1		PKG_NSF+162
	stringtext(NSFSTR_LINK_STR1,"Database")
#define NSFSTR_LINK_STR2		PKG_NSF+163
	stringtext(NSFSTR_LINK_STR2,"View")
#define NSFSTR_LINK_STR3		PKG_NSF+164
	stringtext(NSFSTR_LINK_STR3,"Document")
#define ERR_ALREADY_ALLOC		PKG_NSF+165
	internaltext(ERR_ALREADY_ALLOC,	"(RRV already allocated)")
#define ERR_COMPACT_MODE		PKG_NSF+166
	errortext(ERR_COMPACT_MODE,	"Function cannot be performed while database is being compacted")
#define ERR_DBD_NON_NOTES		PKG_NSF+167
	errortext(ERR_DBD_NON_NOTES,"You may not use that database driver")
#define NSFSTR_COMPACT_PROGRESS	PKG_NSF+168
	stringtext(NSFSTR_COMPACT_PROGRESS,"Compacting database")
#define STR_FILTER_NEVER		PKG_NSF+169
	stringtext(STR_FILTER_NEVER,"Never")
#define STR_FILTER_HOURLY		PKG_NSF+170
	stringtext(STR_FILTER_HOURLY,"hourly")
#define STR_FILTER_DAILY		PKG_NSF+171
	stringtext(STR_FILTER_DAILY,"daily")
#define STR_FILTER_WEEKLY		PKG_NSF+172
	stringtext(STR_FILTER_WEEKLY,"weekly")
#define	ERR_OBJECT_LENGTH		PKG_NSF+173
	errortext(ERR_OBJECT_LENGTH,"Document attachment is invalid")
#define ERR_BUFPOOL_TOO_SMALL	PKG_NSF+174
#ifdef MAC 
	errortext(ERR_BUFPOOL_TOO_SMALL,"Insufficient memory for a view or database buffer.  Please close any unneeded windows.  You may also increase Notes' available memory by quitting and choosing 'Get Info...' from the Finder.")
#else
	errortext(ERR_BUFPOOL_TOO_SMALL,"Insufficient memory for a view or database buffer.")
#endif
#define	ERR_NSF_CORRUPT_RRVMAP	PKG_NSF+175
	errortext(ERR_NSF_CORRUPT_RRVMAP,"Database is damaged and can't be repaired (RRV buckets bad)")
#define	ERR_HDL_LVL0			PKG_NSF+176
	errortext(ERR_HDL_LVL0,		"Maximum number of concurrently open objects has been exceeded")
#define	ERR_HDL_PARAM			PKG_NSF+177
	errortext(ERR_HDL_PARAM,	"No associated pointer to object")
#define	ERR_HDL_NULL			PKG_NSF+178
	errortext(ERR_HDL_NULL,		"Null object handle")
#define	ERR_HDL_OOR				PKG_NSF+179
	errortext(ERR_HDL_OOR,		"Object handle out of range")
#define	ERR_HDL_DEALLOC		 	PKG_NSF+180
	errortext(ERR_HDL_DEALLOC,	"Object handle is invalid")
#define ERR_CANNOT_OPEN_NSF		PKG_NSF+181
	errortext(ERR_CANNOT_OPEN_NSF, "File not found or not a Notes database")
#define ERR_REALLOC_PNOVEC		PKG_NSF+182
	errortext(ERR_REALLOC_PNOVEC,	"Unable to enlarge a view container.  Increase NSF_BUFFER_POOL_SIZE.")
#define ERR_BAD_PNO				PKG_NSF+183
	errortext(ERR_BAD_PNO,		"Attempt to use an invalid page number")
#define ERR_DUPLICATE_UNID		PKG_NSF+184
	errortext(ERR_DUPLICATE_UNID,"Database already contains a document with this ID (UNID)")
#define ERR_DB_NOT_LOADED		PKG_NSF+185
	internaltext(ERR_DB_NOT_LOADED,"(DbDemandLoadByHandle must be called)")
#define ERR_FIXUP_UNID_CHANGED	PKG_NSF+186
	errortext(ERR_FIXUP_UNID_CHANGED,"Document NT%08lx in database %p has been assigned a new UNID")
#define ERR_NO_UNID_INDEX		PKG_NSF+187
	internaltext(ERR_NO_UNID_INDEX,"(UNID index has not been created or opened)")
#define	ERR_NSF_CORRUPT_UNIDIDX	PKG_NSF+188
	errortext(ERR_NSF_CORRUPT_UNIDIDX,"Database is damaged and can't be repaired (UNID index)")
#define ERR_FIXUP_REBUILD_UNID	PKG_NSF+189
	errortext(ERR_FIXUP_REBUILD_UNID,"Rebuild UNID index in database %p")
#define ERR_INVALID_EHASH		PKG_NSF+190
	errortext(ERR_INVALID_EHASH, "Extendible hash index structure is invalid")
#define ERR_USERDATA_TRUNC		PKG_NSF+191
	errortext(ERR_USERDATA_TRUNC, "Container user data buffer will be truncated")
#define ERR_EH_CORRUPT			PKG_NSF+192
	errortext(ERR_EH_CORRUPT,	"Extendible hash index is corrupt and can't be used")
#define ERR_CANT_CHANGE_UNID	PKG_NSF+193
	apitext(ERR_CANT_CHANGE_UNID,"Can't change document's UNID")
#define ERR_EXP_DATE			PKG_NSF+194
	errortext(ERR_EXP_DATE,		"Database's expiration date has passed")
#define ERR_DUP_TPL				PKG_NSF+195
	errortext(ERR_DUP_TPL,		"WARNING: Both %p and %p claim to be Design Template '%.*s'")
#define ERR_CONTAINER_LIMIT		PKG_NSF+196
	errortext(ERR_CONTAINER_LIMIT, "Maximum size of a storage container reached")
#define ERR_OBJECT_CANNOT_BE_ZERO		PKG_NSF+197
	errortext(ERR_OBJECT_CANNOT_BE_ZERO, "Object size cannot be zero")
#define	ERR_NOTE_INVSIG8		PKG_NSF+198
	errortext(ERR_NOTE_INVSIG8,	"The Address Book does not contain any cross certificates capable of validating the signature.")
#define ERR_BC_POOL_FULL		PKG_NSF+199
	errortext(ERR_BC_POOL_FULL, "Insufficient memory - The NSF buffer control pool is full.")
#define	ERR_UPDATE_ATT			PKG_NSF+200
	errortext(ERR_UPDATE_ATT,	"One or more of the source document's attachment are missing.  Run Fixup to delete the document in the source database.")
#define ERR_LINK_STR1		   	PKG_NSF+201
	stringtext(ERR_LINK_STR1,	"Database")
#define ERR_LINK_STR2		   	PKG_NSF+202
	stringtext(ERR_LINK_STR2,	"View")
#define ERR_LINK_STR3		   	PKG_NSF+203
	stringtext(ERR_LINK_STR3,	"Document")
#define	ERR_NSFOPENEXCLUSIVE	PKG_NSF+204
	errortext(ERR_NSFOPENEXCLUSIVE,	"Database is being Compacted; Compact must finish before use.")
/* The following status is 'sent' to the replicator via a break procedure */
/* to partially cancel the replication.  The replicator should stop */
/* processing the current database and start processing the next database. */
/* This is used by background/briefcase to skip to the next database in */
/* the replication list.  As it turns out, the replicator carries on for */
/* errors other than CANCEL and SERVER_NOT_RESPONDING.  It does not */
/* specifically look for this error code. */
#define ERR_SKIP				PKG_NSF+205
	errortext(ERR_SKIP,		"Next operation started at your request")
#define ERR_ASYNC_DELIVERY_SEQNO PKG_NSF+206
	internaltext(ERR_ASYNC_DELIVERY_SEQNO,"(no need to do this transaction in V4)")
#define ERR_DBCLOSE_THREAD		PKG_NSF+207
	errortext(ERR_DBCLOSE_THREAD,"NSFDbClose THREADID != NSFDbOpen THREADID!")
#define ERR_EM_ILLEGALAID  		PKG_NSF+208
	errortext(ERR_EM_ILLEGALAID,"Illegal or uninitialized addin id")
#define ERR_EM_ILLEGAL	 		PKG_NSF+209
	errortext(ERR_EM_ILLEGAL,"Extension number out of bounds")
#define ERR_EM_ILLEGALFLAGS 	PKG_NSF+210
	errortext(ERR_EM_ILLEGALFLAGS,"Extension registration flags illegal")
#define ERR_EM_TOOMANYADDINS 	PKG_NSF+211
	errortext(ERR_EM_TOOMANYADDINS,"Extension Manager has maximum addins already")
#define ERR_EM_NOTAUTHORIZED PKG_NSF+212
	errortext(ERR_EM_NOTAUTHORIZED,"Not authorized to register extension")
#define ERR_EM_CONTINUE 		PKG_NSF+213
	internaltext(ERR_EM_CONTINUE,"continue")
#define ERR_EVENTQUEUE_NOTOPENED PKG_NSF+214
	errortext(ERR_EVENTQUEUE_NOTOPENED,"Event Manager Queue not opened")
#define ERR_EVENTQNEEDS_RESET 	PKG_NSF+215
	errortext(ERR_EVENTQNEEDS_RESET,"Event Manager data structures in error")
#define ERR_EVENTQUEUE_ISFULL 	PKG_NSF+216
	errortext(ERR_EVENTQUEUE_ISFULL,"Event Manager Queue is full")
#define ERR_EVENTQUEUE_MEMORY 	PKG_NSF+217
	errortext(ERR_EVENTQUEUE_MEMORY,"Event Manager Queue memory full")
#define ERR_REPL_UPDATE_RACE	PKG_NSF+218
	errortext(ERR_REPL_UPDATE_RACE,	"Multiple replicator update race")
#define ERR_NO_INCR_UPDATE		PKG_NSF+219
	errortext(ERR_NO_INCR_UPDATE, "Incremental updates disallowed")
#define	ERR_NOTE_INVSIG9		PKG_NSF+220
	errortext(ERR_NOTE_INVSIG9,	"Signature on document is invalid (inconsistent field signatures)")
#define ERR_BTVERIFY			PKG_NSF+221
	errortext(ERR_BTVERIFY,		"NSF BT: Verification error in view %s in database %s.  See NOTES.BRP for dump.")
#define ERR_BTVERIFY_NODESIZE	PKG_NSF+222
	errortext(ERR_BTVERIFY_NODESIZE,	"BTVerifyNode: ERROR in %s, node size is %d - expected %d")
#define ERR_BTVERIFY_LEVEL		PKG_NSF+223
	errortext(ERR_BTVERIFY_LEVEL, "BTVerifyNode: ERROR in %s, level is %d - expected %d")
#define ERR_BTVERIFY_PASTEND	PKG_NSF+224
	errortext(ERR_BTVERIFY_PASTEND, "BTVerifyNode: ERROR in %s, entry %d is past the end of the node")
#define ERR_BTVERIFY_DATA_PASTEND	PKG_NSF+225
	errortext(ERR_BTVERIFY_DATA_PASTEND, "BTVerifyNode: ERROR in %s, Data for entry %d is past the end of the node")
#define ERR_BTVERIFY_PIN_DATA	PKG_NSF+226
	errortext(ERR_BTVERIFY_PIN_DATA,	"BTVerifyNode: ERROR %e pinning BT_DATA %s in node %s")
#define ERR_BTVERIFY_LDATA_SIZE	PKG_NSF+227
	errortext(ERR_BTVERIFY_LDATA_SIZE, "BTVerifyNode: ERROR BT_DATA %s in node %s has size %d, sum of chunks is %d")
#define ERR_BTVERIFY_UNPIN_DATA	PKG_NSF+228
	errortext(ERR_BTVERIFY_UNPIN_DATA,	"BTVerifyNode: ERROR %e unpinning BT_DATA %s in node %s")
#define ERR_BTVERIFY_CARDINALITY	PKG_NSF+229
	errortext(ERR_BTVERIFY_CARDINALITY,	"BTVerifyNode: ERROR in %s, cardinality is %lu - expected %lu")
#define ERR_BTVERIFY_MAXFALSE	PKG_NSF+230
	errortext(ERR_BTVERIFY_MAXFALSE,	"BTVerifyNode: ERROR in %s, MaxEntry is FALSE - expected TRUE")
#define ERR_BTVERIFY_MAXTRUE	PKG_NSF+231
	errortext(ERR_BTVERIFY_MAXTRUE,		"BTVerifyNode: ERROR in %s, MaxEntry is TRUE - expected FALSE")
#define ERR_BTVERIFY_MAXKEYSIZE	PKG_NSF+232
	errortext(ERR_BTVERIFY_MAXKEYSIZE,	"BTVerifyNode: ERROR in %s, MaxKeySize is %d - expected %d")
#define ERR_BTVERIFY_MAXKEYVALUE	PKG_NSF+233
	errortext(ERR_BTVERIFY_MAXKEYVALUE,	"BTVerifyNode: ERROR in %s, maximum key value is wrong")
#define ERR_BTVERIFY_PINSUBINDEX	PKG_NSF+234
	errortext(ERR_BTVERIFY_PINSUBINDEX,	"BTVerifyNode: ERROR %e pinning sub-index descriptor %s in node %s")
#define ERR_BTVERIFY_UNPINSUBINDEX	PKG_NSF+235
	errortext(ERR_BTVERIFY_UNPINSUBINDEX,	"BTVerifyNode: ERROR %e unpinning sub-index descriptor %s in node %s")
#define ERR_FILEEXT_INVALID_READCOUNT	PKG_NSF+236
	errortext(ERR_FILEEXT_INVALID_READCOUNT, "Extendable File object: invalid read count")
#define ERR_FILEEXT_ALREADY_OPEN	PKG_NSF+237
	errortext(ERR_FILEEXT_ALREADY_OPEN, "Extendable File object: already open")
#define ERR_FILEEXT_NOT_OPEN		PKG_NSF+238
	errortext(ERR_FILEEXT_NOT_OPEN, "Extendable File object: object must be open to perform this function")
#define	ERR_DIRMAN_NOT_INITIALIZED	PKG_NSF+239
	errortext(ERR_DIRMAN_NOT_INITIALIZED,"Directory Manager Not Initialized.")
#define	ERR_NOSTUB_HAS_RESPONSES	PKG_NSF+240
	errortext(ERR_NOSTUB_HAS_RESPONSES,	"A note with responses cannot be deleted with UPDATE_NOSTUB")
#define	ERR_RESPONSE_CYCLE			PKG_NSF+241
	errortext(ERR_RESPONSE_CYCLE,		"This operation creates a cycle in the response hierarchy")
#define ERR_NSF_DIRMAN_POOLFULL		PKG_NSF+242
	errortext(ERR_NSF_DIRMAN_POOLFULL, "Insufficient memory - NSF directory manager pool is full.")
#define ERR_NSF_BUF_POOLFULL		PKG_NSF+243
	errortext(ERR_NSF_BUF_POOLFULL, "Insufficient memory - NSF buffer pool is full.  Increase the value of the NSF_BUFFER_POOL_SIZE environment variable.")
#define ERR_EVENTQ_POOLFULL			PKG_NSF+244
	errortext(ERR_EVENTQ_POOLFULL, "Insufficient memory - Event Manager Pool is full.")
#define ERR_DBINDEX_POOLFULL		PKG_NSF+245
	errortext(ERR_DBINDEX_POOLFULL, "Insufficient memory - too many databases on server - Database List Pool is full.")
#define ERR_SYNC_POOLFULL			PKG_NSF+246
	errortext(ERR_SYNC_POOLFULL, 	"Insufficient memory - Database Sync Pool is full.")
#define ERR_EM_POOLFULL				PKG_NSF+247
	errortext(ERR_EM_POOLFULL,	 	"Insufficient memory - Extension Manager Data Pool is full.")
#define	ERR_LOCALSEC_DBOPENFAILURE		PKG_NSF+248
	errortext(ERR_LOCALSEC_DBOPENFAILURE,	"This database has local access protection and you are not authorized to access it")
#define	ERR_LOCALSEC_NOTSUPPORTED		PKG_NSF+249
	errortext(ERR_LOCALSEC_NOTSUPPORTED, "The local security feature is not supported for the database or server")
#define	ERR_LOCALSEC_ALREADYSET			PKG_NSF+250
	errortext(ERR_LOCALSEC_ALREADYSET, "This database already has local security set up and can not be set again")
#define ERR_LOCALSEC_NOUSERPUBKEY		PKG_NSF+251
	errortext(ERR_LOCALSEC_NOUSERPUBKEY, "Can not find the user or public key information in the Name and Address Book")
#define	ERR_LOCALSEC_INVALIDSTATE	PKG_NSF+252
	errortext(ERR_LOCALSEC_INVALIDSTATE, "Invalid local security state")
#define ERR_FIXUP_DUP_DELETED		PKG_NSF+253
	errortext(ERR_FIXUP_DUP_DELETED,"A duplicate of document NT%08lx in database %p has been deleted")
#define	ERR_OVERDISKQUOTA		PKG_NSF+254
	errortext(ERR_OVERDISKQUOTA,"Unable to write to database because database would exceed its disk quota.")
#define	LOG_OVERDISKQUOTA		PKG_NSF+255
	errortext(LOG_OVERDISKQUOTA,"Unable to write to database - database %s would exceed its disk quota of %d KB by %k.")

/* We're full here for PKG_NSF - Max is 255! Start using PKG_NSF2 below...*/


/* More Note Storage File Error Code Definitions */

#define NSFSTR_TRANSLOG_FORMATTING		PKG_NSF2+0
	stringtext(NSFSTR_TRANSLOG_FORMATTING,	"Please wait, creating new transaction logs in directory: ")
#define	ERR_LOCAL_ENCRYPT_MUST_COMPACT	PKG_NSF2+1
	errortext(ERR_LOCAL_ENCRYPT_MUST_COMPACT, "You must compact the database in order to encrypt all documents.")
#define	ERR_LOCAL_DECRYPT_MUST_COMPACT	PKG_NSF2+2
	errortext(ERR_LOCAL_DECRYPT_MUST_COMPACT, "You must compact the database in order to decrypt all documents.")
#define	ERR_IS_OBJSTORE					PKG_NSF2+3
	errortext(ERR_IS_OBJSTORE,			 "The database required for this operation is an object store.")
#define	ERR_OBJSTORE_UPD_REPLICA		PKG_NSF2+4
	errortext(ERR_OBJSTORE_UPD_REPLICA, "The database and the object store for this update are replicas.")
#define	ERR_ITEM_MISMATCH				PKG_NSF2+5
	errortext(ERR_ITEM_MISMATCH,		"An item's value or datatype differs from the same item in the object store note.")
#define	ERR_LOCALSEC_NEEDCOMPACT		PKG_NSF2+6
	errortext(ERR_LOCALSEC_NEEDCOMPACT, "You should compact your database in order to complete the local security setting")
#define	ERR_LOCALSEC_UNKNOWNMETHOD		PKG_NSF2+7
	errortext(ERR_LOCALSEC_UNKNOWNMETHOD, "The specified encryption method is invalid or not supported by this version of software")
#define	ERR_OBJSTORE_NOTSPLIT			PKG_NSF2+8
	errortext(ERR_OBJSTORE_NOTSPLIT,	"Cannot split note (NoteID %d) between database %p and object store %p ")
#define	ERR_BAD_DBUHASHTBL_DELETION		PKG_NSF2+9
	internaltext(ERR_BAD_DBUHASHTBL_DELETION, "The wrong entry was deleted from the dbu hash table.")
#define ERR_EM_MAX_EXTENSIONS			PKG_NSF2+10
	errortext(ERR_EM_MAX_EXTENSIONS,	"Extension Manager exceeded maximum number of Recursion IDs")
#define	ERR_NAMED_OBJECT_CORRUPT		PKG_NSF2+11
	errortext(ERR_NAMED_OBJECT_CORRUPT, "Named Object corrupt.")
#define	ERR_COMPACT_INTERRUPTED			PKG_NSF2+12
	errortext(ERR_COMPACT_INTERRUPTED,	"Compaction of the database was stopped prematurely because another user modified it while it was being compacted.")
#define	ERR_VALIDATION_FAILED			PKG_NSF2+13
	apitext(ERR_VALIDATION_FAILED,	"Validation failed.")
#define	ERR_NSF_HEADER_CORRUPTED		PKG_NSF2+14
	errortext(ERR_NSF_HEADER_CORRUPTED,	"The Notes database file header is corrupted")
#define	ERR_NO_SHARED_ITEMS				PKG_NSF2+15
	errortext(ERR_NO_SHARED_ITEMS,		"The note contains no items that can be shared with the note in the object store.")
#define ERR_LOCAL_ACCESS_AUTHN			PKG_NSF2+16
	errortext(ERR_LOCAL_ACCESS_AUTHN,	"Your ID failed authentication check. Access is denied")
#define ERR_IDTABLE_LENGTH_MISMATCH		PKG_NSF2+17
	errortext(ERR_IDTABLE_LENGTH_MISMATCH,	"The internal length of the ID table does not match the length expected")
#define ERR_FOLDER_DIRECTORY_CORRUPT	PKG_NSF2+18
	errortext(ERR_FOLDER_DIRECTORY_CORRUPT,	"Folder is damaged. Please close and reopen database to repair it.")
#define ERR_FOLDER_NOFREESPACE			PKG_NSF2+19
	errortext(ERR_FOLDER_NOFREESPACE,	"Folder has no free space but should have.")
#define ERR_REPL_BLOCK_FULL				PKG_NSF2+20
	errortext(ERR_REPL_BLOCK_FULL,		"Folder replication block is full, will be reallocated.")
#define ERR_ILLEGAL_FOLDER_COPY			PKG_NSF2+21
	errortext(ERR_ILLEGAL_FOLDER_COPY,	"Source and destination folders must reside in same DB.")
#define ERR_FOLDER_CORRUPT				PKG_NSF2+22
	errortext(ERR_FOLDER_CORRUPT,	"Folder has been damaged.  Please close and reopen database to have it repaired.")
#define ERR_FOLDER_TOO_BIG				PKG_NSF2+23
	errortext(ERR_FOLDER_TOO_BIG,	"Folder is larger than supported, cannot perform operation.")
#define ERR_NO_FOLDER_REPLICATION		PKG_NSF2+24
	errortext(ERR_NO_FOLDER_REPLICATION,	"Folder replication not supported by remote server.")
#define ERR_TOO_MANY_FOLDER_UPDATES		PKG_NSF2+25
	errortext(ERR_TOO_MANY_FOLDER_UPDATES,	"Too many folder additions or removals at one time")
#define ERR_FOLDER_ACCESS_DENIED		PKG_NSF2+26
	errortext(ERR_FOLDER_ACCESS_DENIED,	"No privilege to update folder")
#define ERR_NOTE_CANT_BE_FOLDER			PKG_NSF2+27
	errortext(ERR_NOTE_CANT_BE_FOLDER,	"Note must be public or private view to be a folder")
#define	ERR_MAXSIZE_NOTSUPPORTED 		PKG_NSF2+28
	errortext(ERR_MAXSIZE_NOTSUPPORTED,		"The server does not support Notes database with maximum size greater than 1 GB")
#define	ERR_OBJSTORE_NOACCESS			PKG_NSF2+29
	errortext(ERR_OBJSTORE_NOACCESS,	"You do not have access to the object store that is used by this note")
#define	ERR_OBJSTORE_NOT_FOUND			PKG_NSF2+30
	errortext(ERR_OBJSTORE_NOT_FOUND,	"The object store note that is used by this note was not found.  Run the object store COLLECT task on this database.")
#define	ERR_CANNOT_OPEN_OBJSTORE		PKG_NSF2+31
	errortext(ERR_CANNOT_OPEN_OBJSTORE,	"The object store that is used by this note was not found or is not a Notes database")
#define	ERR_OBJSTORE_NOTE_NOACCESS			PKG_NSF2+32
	errortext(ERR_OBJSTORE_NOTE_NOACCESS,	"You do not have access to the object store note that is used by this note")
#define	ERR_OBJSTORE_NOTE_DELETED			PKG_NSF2+33
	errortext(ERR_OBJSTORE_NOTE_DELETED,	"The object store note that is used by this note has been deleted.  Run the object store COLLECT task on this database.")
#define	ERR_USER_HANDLED_CONFLICT		PKG_NSF2+34
	errortext(ERR_USER_HANDLED_CONFLICT,"User Has Handled their own Conflict.")
#define ERR_FIXUP_GHOST					PKG_NSF2+35
	errortext(ERR_FIXUP_GHOST,			"Fixed header of document NT%08lx in database %p so it will replicate properly.")
#define	ERR_INVALID_CD_FILE				PKG_NSF2+36
	errortext(ERR_INVALID_CD_FILE,		"Incorrectly formatted Composite records in file.")
#define ERR_FIXUP_FOUND_UNREADLIST		PKG_NSF2+37
	errortext(ERR_FIXUP_FOUND_UNREADLIST,	"Unread list for user %a in database %p is damaged: %e")
#define ERR_DELETE_ENTRY				PKG_NSF2+38
	errortext(ERR_DELETE_ENTRY,			"Internal only - delete named table entry.")
#define ERR_UPDATE_ENTRY				PKG_NSF2+39
	errortext(ERR_UPDATE_ENTRY,			"Internal only - update named table entry.")
#define ERR_TOO_MANY_FOLDERS			PKG_NSF2+40
	errortext(ERR_TOO_MANY_FOLDERS,		"Exceeded maximum folder count in database.")
#define ERR_FIXUP_DEL_UNREADLIST		PKG_NSF2+41
	errortext(ERR_FIXUP_DEL_UNREADLIST,	"Unread list for user %a in database %p is damaged and has been deleted: %e")
#define ERR_FIXUP_NAMEDNOTE				PKG_NSF2+42
	errortext(ERR_FIXUP_NAMEDNOTE,		"Named note NT%08lx in database %p is damaged.")
#define ERR_NSF_FOLDER_POOLFULL			PKG_NSF2+43
	errortext(ERR_NSF_FOLDER_POOLFULL, "Insufficient memory - NSF folder pool is full.")
#define ERR_FIXUP_FOLDER_PURGED			PKG_NSF2+44
	errortext(ERR_FIXUP_FOLDER_PURGED,	"Folder NT%08lx in database %p has been purged due to corruption: %s")
#define ERR_FIXUP_FOLDER_IDTABLE		PKG_NSF2+45
	errortext(ERR_FIXUP_FOLDER_IDTABLE,	"Folder NT%08lx in database %p has corrupt ID table, will be rebuilt.  Error: %e.")
#define ERR_FOLDERS_UPTODATE			PKG_NSF2+46
	errortext(ERR_FOLDERS_UPTODATE,		"Folders in database are up to date")
#define ERR_RTR_NOTSET					PKG_NSF2+47
	errortext(ERR_RTR_NOTSET,			"RTR Context not set for this database")
#define ERR_RTR_ALREADYSET 				PKG_NSF2+48
	errortext(ERR_RTR_ALREADYSET,		"RTR Context already set for this database")
#define ERR_NSF_COMPUTE_NOENVIRONMENT	PKG_NSF2+49
	errortext(ERR_NSF_COMPUTE_NOENVIRONMENT,	"You are not allowed to modify environment variables.")
#define ERR_NSF_REMOTE_URL_QUEUE		PKG_NSF2+50
	errortext(ERR_NSF_REMOTE_URL_QUEUE, "Cannot queue a URL Note Open to a remote database.")
#define ERR_NOTE_NOT_FOLDER				PKG_NSF2+51
	errortext(ERR_NOTE_NOT_FOLDER,		"Attempt to perform folder operation on non-folder note.")
#define ERR_NO_VIEWS_IN_OBJSTORE	  	PKG_NSF2+52
	errortext(ERR_NO_VIEWS_IN_OBJSTORE, "Views cannot be added to an object store.")
#define	ERR_NOTE_INVSIG10				PKG_NSF2+53
	errortext(ERR_NOTE_INVSIG10,		"Document has been modified or corrupted since signed! (extended signature)")
#define ERR_NEEDS_FIXUP					PKG_NSF2+54
	errortext(ERR_NEEDS_FIXUP,			"This database cannot be opened because a consistency check of it is needed.")
#define ERR_FIXUP_IN_PROGRESS	   		PKG_NSF2+55
	errortext(ERR_FIXUP_IN_PROGRESS,	"This database cannot be opened because a consistency check of it is in progress.")
#define	ERR_FILTER_CONTINUING	 		PKG_NSF2+56
	errortext(ERR_FILTER_CONTINUING,	"Error running agent on NoteID %lu - %e.  Continuing...")
#define ERR_OBJSTORE_TITLE				PKG_NSF2+57
	errortext(ERR_OBJSTORE_TITLE,		"Object Store for ")
#define	ERR_NSF_CORRUPT_ACL				PKG_NSF2+58
	errortext(ERR_NSF_CORRUPT_ACL,		"Database ACL is damaged and can't be repaired.")
#define ERR_CONTVERIFY_FREESPACEWRONG	PKG_NSF2+59
	errortext(ERR_CONTVERIFY_FREESPACEWRONG,	"BufAllocSpace: Mismatch in free space, container: %d, (%s).")
#define ERR_DBUHASH_POOLFULL			PKG_NSF2+60
	errortext(ERR_DBUHASH_POOLFULL,		"NSF DBU hash table pool is full.")
#define ERR_RSP_ADDTO_HIERFOLD			PKG_NSF2+61
	errortext(ERR_RSP_ADDTO_HIERFOLD, "To move a response document to a folder that shows response hierarchy you need to move the topmost parent of the response")
#define ERR_RSP_REMFROM_HIERFOLD		PKG_NSF2+62
	errortext(ERR_RSP_REMFROM_HIERFOLD, "To remove a response document from a folder that shows response hierarchy you must also remove the topmost parent of the response")
#define ERR_RSP_REMFROM_HIERFOLD_WARN	PKG_NSF2+63
	errortext(ERR_RSP_REMFROM_HIERFOLD_WARN, "All related response documents are being removed from this folder along with their parents.")
#define ERR_RSP_ADDTO_HIERFOLD_WARN		PKG_NSF2+64
	errortext(ERR_RSP_ADDTO_HIERFOLD_WARN, "All related response documents are being added to this folder along with their parents.")
#define NSFSTR_ENCRYPT_PROGRESS			PKG_NSF2+65
	stringtext(NSFSTR_ENCRYPT_PROGRESS,"Encrypting database")
#define NSFSTR_DECRYPT_PROGRESS			PKG_NSF2+66
	stringtext(NSFSTR_DECRYPT_PROGRESS,"Decrypting database")
#define	ERR_NOT_OBJSTORE				PKG_NSF2+67
	errortext(ERR_NOT_OBJSTORE,			 "The object store required for this operation is not an object store.")
#define	ERR_VIEW_UPDATE					PKG_NSF2+68
	errortext(ERR_VIEW_UPDATE,			 "View and Design notes can only be deleted by using NIFUpdateNote or NIFDeleteNote.")
#define	ERR_NOTE_CLASS					PKG_NSF2+69
	errortext(ERR_NOTE_CLASS,			"Document has invalid Note Class")
#define ERR_RM_DOING_RESTART		    PKG_NSF2+70
	errortext(ERR_RM_DOING_RESTART,		"Recovery Manager: Recovery being performed for DB")
#define	ERR_BDB_CORRUPT					PKG_NSF2+71
	errortext(ERR_BDB_CORRUPT,			"Database is damaged and can't be repaired (BDB)")
#define	ERR_INVALID_PARENT_NOTE			PKG_NSF2+72
	errortext(ERR_INVALID_PARENT_NOTE,	"Invalid or nonexistent parent document")
#define	ERR_NOT_CHILD_NOTE				PKG_NSF2+73
	errortext(ERR_NOT_CHILD_NOTE,		"This document is not a child of its parent document")
#define	ERR_LINK_FORMAT					PKG_NSF2+74
	errortext(ERR_LINK_FORMAT,			"Notes Document Link is not formatted properly, or it is not a Notes Document Link")
#define	ERR_DB_MARKED_FOR_DELETE		PKG_NSF2+75
	errortext(ERR_DB_MARKED_FOR_DELETE,	"This database has been marked for delete and cannot be modified")
#define	ERR_RETRY_RW					PKG_NSF2+76
	internaltext(ERR_RETRY_RW,			"Retry this operation with the RW sem")
#define	ERR_UNAME_TEXT					PKG_NSF2+77
	errortext(ERR_UNAME_TEXT,			"Invalid type of data for a Unique Document Name")
#define	ERR_UNAME_LENGTH	  			PKG_NSF2+78
	errortext(ERR_UNAME_LENGTH,			"Unique Document Name is too long")
#define	ERR_UNAME_DIRFULL	  			PKG_NSF2+79
	errortext(ERR_UNAME_DIRFULL,		"Insufficient space in Unique Document Name Directory Pool")
#define	ERR_UNAME_NO_DB		  			PKG_NSF2+80
	errortext(ERR_UNAME_NO_DB,			"Document Database Not Found")
#define	ERR_UNAME_NO_DOC	   			PKG_NSF2+81
	errortext(ERR_UNAME_NO_DOC,			"Document Not Found")
#define ERR_NSF_SCHED_POOLFULL			PKG_NSF2+82
	errortext(ERR_NSF_SCHED_POOLFULL,	"Insufficient memory - Schedule vpool is full.")
#define    ERR_NSF_NO_WEB_CACHE			PKG_NSF2+83
errortext( ERR_NSF_NO_WEB_CACHE,		"The Web retriever database cache could not be opened.  Please check your InterNotes Server value in your current location.")
#define ERR_FIXUP_FOUND_HUGE_UNREADLIST		PKG_NSF2+84
	errortext(ERR_FIXUP_FOUND_HUGE_UNREADLIST,	"Unread list for user %a in database %p is in the new format")
#define ERR_FIXUP_UNLATCH_FAILED		PKG_NSF2+85
	errortext(ERR_FIXUP_UNLATCH_FAILED,	"Unable to change unread list for user %a in database %p to the old format: %e")
#define ERR_FIXUP_UNLATCHED				PKG_NSF2+86
	errortext(ERR_FIXUP_UNLATCHED,		"Changed unread list for user %a in database %p to the old format")
/*	NOTE class names */
#define ERR_NOTE_DESIGN_CLASS_NAME		PKG_NSF2+87
	errortext(ERR_NOTE_DESIGN_CLASS_NAME,"design")
#define ERR_NOTE_VIEW_CLASS_NAME		PKG_NSF2+88
	errortext(ERR_NOTE_VIEW_CLASS_NAME,	"view")
#define ERR_NOTE_DVIEW_CLASS_NAME		PKG_NSF2+89
	errortext(ERR_NOTE_DVIEW_CLASS_NAME,"default view")
#define ERR_NOTE_CLASS_NAME				PKG_NSF2+90
	errortext(ERR_NOTE_CLASS_NAME,		"class")
#define ERR_NOTE_DDESIGN_CLASS_NAME		PKG_NSF2+91
	errortext(ERR_NOTE_DDESIGN_CLASS_NAME,"default design")
#define ERR_NOTE_PVIEW_CLASS_NAME		PKG_NSF2+92
	errortext(ERR_NOTE_PVIEW_CLASS_NAME,"private view")
#define ERR_NOTE_NO_CLASS_NAME			PKG_NSF2+93
	errortext(ERR_NOTE_NO_CLASS_NAME,	"private design")
#define ERR_FTM_NOTSET					PKG_NSF2+94
	errortext(ERR_FTM_NOTSET,			"FTM Context not set for this database")
#define ERR_FTM_ALREADYSET 				PKG_NSF2+95
	errortext(ERR_FTM_ALREADYSET,		"FTM Context already set for this database")
#define ERR_SCHOBJCNTNR					PKG_NSF2+96
	errortext(ERR_SCHOBJCNTNR,	"schedule object container error")
#define ERR_SCHOBJCNTNR_GATEWAY		 	PKG_NSF2+97
	errortext(ERR_SCHOBJCNTNR_GATEWAY,	"schedule object container gateway error")
#define ERR_SCHOBJCNTNR_PARTIAL		 	PKG_NSF2+98
	errortext(ERR_SCHOBJCNTNR_PARTIAL,	"schedule container partially filled")
#define ERR_SCHED_INVALIDDOMAIN			PKG_NSF2+99
	errortext(ERR_SCHED_INVALIDDOMAIN,	"Invalid domain type for scheduling request")
#define ERR_SCHED_NAMENOTUNIQUE			PKG_NSF2+100
	errortext(ERR_SCHED_NAMENOTUNIQUE,	"User name in scheduling request not unique in Name and Address Book")
#define ERR_SCHED_NOSUCHUSER			PKG_NSF2+101
	errortext(ERR_SCHED_NOSUCHUSER,		"User name in scheduling request not found in Name and Address Book")
#define ERR_SCHED_NOGROUP				PKG_NSF2+102
	errortext(ERR_SCHED_NOGROUP,		"Error processing scheduling request, group name not supported")
#define ERR_SCHED_DOMAINNOTFOUND		PKG_NSF2+103
	errortext(ERR_SCHED_DOMAINNOTFOUND,	"Can't find domain document in Domino Directory for the scheduling request")
#define ERR_SCHED_CANTFINDSCHEDULE		PKG_NSF2+104
	errortext(ERR_SCHED_CANTFINDSCHEDULE, "Can't find schedule record for requested user")
#define ERR_SCHOBJ_NOCOMPOSITE			PKG_NSF2+105
	errortext(ERR_SCHOBJ_NOCOMPOSITE,	"No composite schedule was generated")
#define ERR_SCHOBJ_NOSCHEDLIST			PKG_NSF2+106
	errortext(ERR_SCHOBJ_NOSCHEDLIST,	"No schedule list entry is available for this object")
#define ERR_SCHOBJ_LOCALNOCOMPOSITE		PKG_NSF2+107
	errortext(ERR_SCHOBJ_LOCALNOCOMPOSITE,	"No composite schedule was generated locally")
#define ERR_SCHED_APPT_NOTFOUND			PKG_NSF2+108
	errortext(ERR_SCHED_APPT_NOTFOUND,	"Appointment record not found")
#define ERR_SCHRQST_TIMEOUT				PKG_NSF2+109
	errortext(ERR_SCHRQST_TIMEOUT,		"schedule request timed out")
#define ERR_SCHOBJ_NOTEXIST				PKG_NSF2+110
	errortext(ERR_SCHOBJ_NOTEXIST,		"schedule object doesn't exist")
#define ERR_SCHED_CHAIN_LOOP			PKG_NSF2+111
	errortext(ERR_SCHED_CHAIN_LOOP,		"Schedule request chaining loop detected")
#define	ERR_NSF_CORRUPT_ECL				PKG_NSF2+112
	errortext(ERR_NSF_CORRUPT_ECL,		"ECL is damaged and can't be repaired")
#define	ERR_NSF_COMPUTE_ECL_ABORT		PKG_NSF2+113
	errortext(ERR_NSF_COMPUTE_ECL_ABORT,	"Operation aborted at your request")
#define ERR_BITMAPCHECKSUM_BAD			PKG_NSF2+114
	errortext(ERR_BITMAPCHECKSUM_BAD,	"Bitmap checksum is incorrect")
#define ERR_BAD_BUCKETPOS				PKG_NSF2+115
	internaltext(ERR_BAD_BUCKETPOS,		"Bad bucket position")
#define ERR_DBCACHE_ENTRY_RESERVED		PKG_NSF2+116
	internaltext(ERR_DBCACHE_ENTRY_RESERVED, "DbCache entry reserved")
#define ERR_BUCKET_CORRUPT				PKG_NSF2+117
	internaltext(ERR_BUCKET_CORRUPT, 	"Bucket is corrupt, perhaps partially written")
#define ERR_RM_STOP_LOGGING				PKG_NSF2+118
	errortext(ERR_RM_STOP_LOGGING, 		"Recovery Manager: Transactional Logging being disabled after this restart")
#define ERR_NSF_BUCKET_POOLFULL			PKG_NSF2+119
	errortext(ERR_NSF_BUCKET_POOLFULL, "Insufficient memory - NSF bucket buffer pool is full.")
#define ERR_INVALID_BUCKET_TYPE			PKG_NSF2+120
	errortext(ERR_INVALID_BUCKET_TYPE, "Invalid Bucket Type")
#define	ERR_NETACCOUNT_LASTNAME			PKG_NSF2+121
	errortext(ERR_NETACCOUNT_LASTNAME,	"A valid last name string must be supplied for the second parameter of @NetAccount")
#define ERR_CANTINVALIDATE_BUFFER		PKG_NSF2+122
	internaltext(ERR_CANTINVALIDATE_BUFFER,	"Can't invalidate buffer, pin count > 1")
#define ERR_WRONG_PROCESSGROUP			PKG_NSF2+123
	internaltext(ERR_WRONG_PROCESSGROUP,		"Ager is in wrong process group")
#define ERR_ITEM_TIME_CORRUPT			PKG_NSF2+124
	internaltext(ERR_ITEM_TIME_CORRUPT,	"Item does not have a valid time")
#define ERR_SYNC_TIME_NOT_FOUND			PKG_NSF2+125
	internaltext(ERR_SYNC_TIME_NOT_FOUND, "Documents do not have a common revision time")
#define STR_FREE_TIME_DB_TITLE			PKG_NSF2+126
	stringtext(STR_FREE_TIME_DB_TITLE, 	"Local free time info")
#define ERR_SCHED_MULTIPLE_DOMAINS		PKG_NSF2+127
	errortext(ERR_SCHED_MULTIPLE_DOMAINS, "Warning: Multiple documents detected for domain '%s' in Domino Directory for the scheduling request.")
/* PKG_NSF2 codes limited to 0-127 */


#define ERR_TRANSLOG_NO_SPACE			PKG_NSF3+0
	errortext(ERR_TRANSLOG_NO_SPACE,	"Insufficient disk space to create transaction log.")
#define ERR_SCHED_QUERY_SMALLER_DATE_RANGE		PKG_NSF3+1
	errortext(ERR_SCHED_QUERY_SMALLER_DATE_RANGE, "The date range for this busy time query contains too much busy time data. Query a shorter range of days.")
#define ERR_SCHED_RANGEFULL				PKG_NSF3+2
	errortext(ERR_SCHED_RANGEFULL, "The range is full.")
#define ERR_SCHED_TOOMUCHDATA			PKG_NSF3+3
	errortext(ERR_SCHED_TOOMUCHDATA, 	"Too much data is being added to this object. Please break it up into chunks.")
#define ERR_NULL_DBHANDLE				PKG_NSF3+4
	errortext(ERR_NULL_DBHANDLE,		"The database handle is NULL")
#define ERR_INVALID_BUCKET_NUMBER  		PKG_NSF3+5
	errortext(ERR_INVALID_BUCKET_NUMBER,"Invalid Bucket Number")
#define ERR_SCHED_WARNING	  			PKG_NSF3+6
	errortext(ERR_SCHED_WARNING,		"Warning: schedule retrieval error for %s - ")
#define ERR_PROFILE_ENUM_POOLFULL  		PKG_NSF3+7
	errortext(ERR_PROFILE_ENUM_POOLFULL,"Profile document enumeration pool is full")
#define ERR_OBJSTORE_NOT_ENCRYPTED		PKG_NSF3+8
	internaltext(ERR_OBJSTORE_NOT_ENCRYPTED,"Object Store not marked encrypted and must be")
#define ERR_DBCACHE_TOO_BIG				PKG_NSF3+9
	errortext(ERR_DBCACHE_TOO_BIG,		"NSF_DBCACHE_MAX_ENTRIES parameter too large, memory request failed")
#define ERR_NOLOCKINPAGECOMPRESS		PKG_NSF3+10
	errortext(ERR_NOLOCKINPAGECOMPRESS,"Db write semaphore not held during attempted page compression")
#define ERR_CANTDOINPLACECOMPACT		PKG_NSF3+11
	errortext(ERR_CANTDOINPLACECOMPACT,"In-place database compaction can only be done with V5 and beyond databases")
#define ERR_NOBUCKETSKIMMING			PKG_NSF3+12
	errortext(ERR_NOBUCKETSKIMMING,	"Operation cannot be performed at the current time - database compaction in progress.")
#define ERR_CANTCOPYUNKTABLE			PKG_NSF3+13
	internaltext(ERR_CANTCOPYUNKTABLE,	"Unable to copy UNK table")
#define ERR_UBMPOOLFULL					PKG_NSF3+14
	errortext(ERR_UBMPOOLFULL,"The buffer pool is too full to bring in another page.")
#define ERR_UBMPOOLMAXED				PKG_NSF3+15
	errortext(ERR_UBMPOOLMAXED,"The buffer pool is extremely full; delay the thread.")
#define ERR_UBMBACKOFF					PKG_NSF3+16
	errortext(ERR_UBMBACKOFF,"Buffer manager clock failed to find a buffer.  Retry pin operation.")
#define ERR_UBMPAGENOTFOUND				PKG_NSF3+17
	errortext(ERR_UBMPAGENOTFOUND,"Page is not currently buffered.")
#define ERR_UBMBADPAGESIZE				PKG_NSF3+18
	errortext(ERR_UBMBADPAGESIZE,"Page size requested is not allowed.")
#define ERR_UBMGROUPTBLFULL				PKG_NSF3+19
	errortext(ERR_UBMGROUPTBLFULL,"Universal Buffer Manager group table is full.")
#define ERR_UBMBADGROUPID				PKG_NSF3+20
	errortext(ERR_UBMBADGROUPID,"Using invalid (old) group id.")
#define ERR_UBMPAGEPINNED				PKG_NSF3+21
	errortext(ERR_UBMPAGEPINNED,"Page pinned during group operation.")
#define ERR_BKTDESCPAGE_CORRUPT			PKG_NSF3+22
	internaltext(ERR_BKTDESCPAGE_CORRUPT, 	"Bucket descriptor page is corrupt, perhaps partially written")
#define ERR_UBMTEST						PKG_NSF3+23
	errortext(ERR_UBMTEST,"Miscellaneous error running testnsf for ubm.")
#define ERR_NOINCREMENTALANTIFOLDER		PKG_NSF3+24
	errortext(ERR_NOINCREMENTALANTIFOLDER, "Anti-folder view must be rebuilt")
#define ERR_UBMBADGAD					PKG_NSF3+25
	errortext(ERR_UBMBADGAD,"Group Access Data structure size is too large.")
#define ERR_UBMNOGROUPINFO				PKG_NSF3+26
	errortext(ERR_UBMNOGROUPINFO,"Group Access Data cannot be found.")
#define ERR_UBMNOSPACE					PKG_NSF3+27
	errortext(ERR_UBMNOSPACE,"Group Access Data bpool cannot be created.")
#define ERR_CANNOT_COMPACT_DB_VERSION	PKG_NSF3+28
	errortext(ERR_CANNOT_COMPACT_DB_VERSION, "Cannot compact pre-V3 databases to V5 or later databases - use New Replica instead.")
#define ERR_CANT_COMPACT_INDEX_VERSION	PKG_NSF3+29
	errortext(ERR_CANT_COMPACT_INDEX_VERSION, "Cannot compact database.  Please use compact -d or updall -r followed by compact.")
#define ERR_RRVBKT_CORRUPT				PKG_NSF3+30
	errortext(ERR_RRVBKT_CORRUPT, 		"RRV bucket is corrupt.")
#define ERR_BITMAP_CORRUPT				PKG_NSF3+31
	errortext(ERR_BITMAP_CORRUPT, 		"Allocation bitmap is corrupt.")
#define ERR_SALVAGE_FAILED				PKG_NSF3+32
	internaltext(ERR_SALVAGE_FAILED, 	"Salvage attempt failed, database severely corrupted.")
#define ERR_UBMBUFFERTOOSMALL			PKG_NSF3+33
	errortext(ERR_UBMBUFFERTOOSMALL,"Group Data buffer provided by caller is too small.")
#define ERR_UBMCANTLOCK					PKG_NSF3+34
	errortext(ERR_UBMCANTLOCK,"UBM I/O Method could not get conditional semaphore.")
#define ERR_UBMNOFD						PKG_NSF3+35
	errortext(ERR_UBMNOFD,"UBM I/O Method thread does not have file open.")
#define ERR_UBMWRITEBACKANDDISCARD		PKG_NSF3+36
	errortext(ERR_UBMWRITEBACKANDDISCARD,"Simultaneous setting of writeback and discard on buffer.")
#define ERR_UBMTOOMANYPAGES				PKG_NSF3+37
	errortext(ERR_UBMTOOMANYPAGES,"Too many pages in prefetch page list - list truncated.")
#define ERR_PAGE_PINNED					PKG_NSF3+38
	internaltext(ERR_PAGE_PINNED,	 	"Operation cannot be performed because the page is pinned")
#define ERR_UBMPAGENOTPINNED			PKG_NSF3+39
	errortext(ERR_UBMPAGENOTPINNED,"Trying to unpinned an unpinned page.")
#define ERR_UBMBADGTABLESEG				PKG_NSF3+40
	errortext(ERR_UBMBADGTABLESEG,"Universal Buffer Manager group table segment is null.")
#define ERR_NSF_CANT_BACKUP				PKG_NSF3+41
	errortext(ERR_NSF_CANT_BACKUP,"Cannot perform write due to error recording before image for full backup.")
#define ERR_CANT_BACKUP_DURING_COMPACT	PKG_NSF3+42
	errortext(ERR_CANT_BACKUP_DURING_COMPACT, "Cannot backup a database while it is being compacted.")

#define ERR_BAD_SUPERBLOCK				PKG_NSF3+43
	errortext(ERR_BAD_SUPERBLOCK, "SuperBlock is corrupt on disk - Restart recovery is required.")
#define ERR_MQ_POOL_FULL				PKG_NSF3+44
	errortext(ERR_MQ_POOL_FULL,"Logger memory pool is full.")
#define ERR_UNABLE_TO_RENAME			PKG_NSF3+45
	errortext(ERR_UNABLE_TO_RENAME,		"Compaction failed: Unable to rename %p back to %p: %e - You must rename it yourself.")
#define ERR_UNABLE_TO_DELETE			PKG_NSF3+46
	errortext(ERR_UNABLE_TO_DELETE,		"Compaction failed: Unable to delete %p")
#define ERR_DB_ALREADY_OPEN		  		PKG_NSF3+47
	errortext(ERR_DB_ALREADY_OPEN,		"Database is already open")
#define	ERR_UNAME_NOT_UNIQUE			PKG_NSF3+48
	errortext(ERR_UNAME_NOT_UNIQUE,		"Unique Document Name is already used")
#define	ERR_UNAME_ALREADY_NAMED			PKG_NSF3+49
	errortext(ERR_UNAME_ALREADY_NAMED,	"Note already has a Unique Document Name")
#define ERR_NOTE_ALREADY_OPEN			PKG_NSF3+50
	errortext(ERR_NOTE_ALREADY_OPEN,	"Note is already open")
#define ERR_FIXUP_FOLDER_PURGED_NO_DELETE			PKG_NSF3+51
	errortext(ERR_FIXUP_FOLDER_PURGED_NO_DELETE,	"Folder %s (NT%08lx) in database %p is corrupted. This folder will be purged on the next access of this database: %s")
#define ERR_UNABLE_TO_CREATE_MSGID		PKG_NSF3+52
	errortext(ERR_UNABLE_TO_CREATE_MSGID,	"Unable to create a message-id")
#define ERR_NNTP_IN_DB_INDEX			PKG_NSF3+53
	internaltext(ERR_NNTP_IN_DB_INDEX,	"The News/NNTP database name was found in the db index")
#define ERR_NNTP_NO_MORE_DBNAMES		PKG_NSF3+54
	errortext(ERR_NNTP_NO_MORE_DBNAMES,	"Could not create a News/NNTP database name")
#define ERR_NOTE_HIT_64K_COMPUTE_LIMIT	PKG_NSF3+55
	errortext(ERR_NOTE_HIT_64K_COMPUTE_LIMIT,	"Note ID %lu has items that are causing the Compute subsystem to generate a 64K error.  This note will not be included in search results.")
#define ERR_NNTP_NEWSGROUP_EXISTS		PKG_NSF3+56
	errortext(ERR_NNTP_NEWSGROUP_EXISTS,"An NNTP Newsgroup of that name already exists")
#define ERR_NNTP_BAD_NEWSGROUP_NAME		PKG_NSF3+57
	errortext(ERR_NNTP_BAD_NEWSGROUP_NAME,"Invalid characters in Newsgroup name")
#define MSG_NNTP_ARCHIVE_HELPER			PKG_NSF3+58
	errortext(MSG_NNTP_ARCHIVE_HELPER,	"An existing archive database has been specified.  Please use the following options to specify what should be done to the original archive database.")
#define ERR_NULL_NOTEHANDLE				PKG_NSF3+59
	errortext(ERR_NULL_NOTEHANDLE,		"The note handle is NULL.")
#define ERR_ITEM_NEWNAME_TOO_LONG		PKG_NSF3+60
	errortext(ERR_ITEM_NEWNAME_TOO_LONG,"The new item name is too long to fit over the existing name.")
#define ERR_DBALLOC_SIZE_ZERO			PKG_NSF3+61
	errortext(ERR_DBALLOC_SIZE_ZERO,	"Detected zero length file space allocation.")
#define ERR_FILE_MODIFIED_ATOS			PKG_NSF3+62
	errortext(ERR_FILE_MODIFIED_ATOS,	"File modified at operating-system level while cached, please retry open.")
#define ERR_FIXUP_NEEDED				PKG_NSF3+63
	errortext(ERR_FIXUP_NEEDED,			"Database Fixup needed: %s.")
#define ERR_RM_FILTER_LOG_RECORD		PKG_NSF3+64
	internaltext(ERR_RM_FILTER_LOG_RECORD, "Recovery Manager: Filtered current Log Record")
#define ERR_RM_FIXUP_DB					PKG_NSF3+65
	errortext(ERR_RM_FIXUP_DB,			"Recovery Manager: Database is damaged and requires Fixup -j or Media Recovery")
#define ERR_RM_DOT_ENTRY_NOT_FOUND		PKG_NSF3+66
	internaltext(ERR_RM_DOT_ENTRY_NOT_FOUND, "Recovery Manager: Dirty Object not found it table")
#define ERR_RM_DOT_POOL_FAILURE			PKG_NSF3+67
	errortext(ERR_RM_DOT_POOL_FAILURE,	"Recovery Manager: Insufficient memory - Dirty Object Pool is full")
#define ERR_RM_LOG_FULL					PKG_NSF3+68
	errortext(ERR_RM_LOG_FULL,			"Recovery Manager: Log File is Full, try again later")
#define ERR_RM_GENERAL_FAILURE			PKG_NSF3+69
	errortext(ERR_RM_GENERAL_FAILURE,	"Recovery Manager: General failure")
#define ERR_LR_RECOVERY_FAILURE			PKG_NSF3+70
	errortext(ERR_LR_RECOVERY_FAILURE,	"Recovery : General failure")
#define ERR_ONLY_V5_DATABASES			PKG_NSF3+71
	errortext(ERR_ONLY_V5_DATABASES, 	"Function not supported for pre-R5 format databases")
#define ERR_NBRO_CORRUPT				PKG_NSF3+72
	internaltext(ERR_NBRO_CORRUPT,	 	"Non-bucketized response object is corrupt, perhaps partially written")
#define ERR_NBNS_CORRUPT				PKG_NSF3+73
	internaltext(ERR_NBNS_CORRUPT,	 	"Non-bucketized nonsummary data is corrupt, perhaps partially written")
#define ERR_DUPLICATE_ATTACHMENT		PKG_NSF3+74
	errortext(ERR_DUPLICATE_ATTACHMENT,	"Attachment found on more than one document.")
#define ERR_UBMLASTUNPIN				PKG_NSF3+75
	errortext(ERR_UBMLASTUNPIN,			"Attempted last unpin of discarded buffer under roll forward")
#define ERR_RUN_FIXUP					PKG_NSF3+76
	errortext(ERR_RUN_FIXUP,			"Please run Fixup on database and retry operation.")
#define ERR_NOFOLDERREORG_DURING_COMPACT PKG_NSF3+77
	errortext(ERR_NOFOLDERREORG_DURING_COMPACT,	"Folders may not be reorganized during compaction, please retry.")
#define ERR_MONITORS_NOT_ALLOWED		PKG_NSF3+78
	errortext(ERR_MONITORS_NOT_ALLOWED,	"Monitors are not allowed on this database.")
#define ERR_MORE_EVENTS					PKG_NSF3+79
	errortext(ERR_MORE_EVENTS,			"More events are available for this client.")
#define ERR_NO_SUCH_CLIENT				PKG_NSF3+80
	errortext(ERR_NO_SUCH_CLIENT,		"Monitor client not known by server.")
#define ERR_NSF_MONITOR_POOLFULL		PKG_NSF3+81
	errortext(ERR_NSF_MONITOR_POOLFULL, "Insufficient memory - NSF monitor pool is full.")
#define ERR_MONITOR_ALREADY_REGISTERED	PKG_NSF3+82
	errortext(ERR_MONITOR_ALREADY_REGISTERED, "Monitor has already been registered, no action taken.")
#define ERR_UNKNOWN_MONITOR_CLIENT 		PKG_NSF3+83
	errortext(ERR_UNKNOWN_MONITOR_CLIENT,  "Monitor client not known.")
#define ERR_OLD_INDEX_VERSION			PKG_NSF3+84
	internaltext(ERR_OLD_INDEX_VERSION, "Encountered old index (container) version - ignoring.")
#define ERR_IGNORE_SRFILL_ERROR			PKG_NSF3+85
	errortext(ERR_IGNORE_SRFILL_ERROR,	"Note ID NT%08lx - %e.  This note will not be included in search results.")
#define ERR_OLE_STORAGE_TYPE			PKG_NSF3+86
	internaltext(ERR_OLE_STORAGE_TYPE,	"OLE storage type not correct.")
#define ERR_MIME_PART_EXPECTED			PKG_NSF3+87
	errortext(ERR_MIME_PART_EXPECTED,	"Incorrect data type: MIME part expected.")
#define ERR_MIME_PARSE_MORE_DATA		PKG_NSF3+88
	internaltext(ERR_MIME_PARSE_MORE_DATA,"Internal only - MIME parser needs more data.")
#define ERR_MIME_PARSE_BAD_DATA			PKG_NSF3+89
	errortext(ERR_MIME_PARSE_BAD_DATA,	"Incorrect format in MIME data.")
#define ERR_MIME_PARSE_NOT_FOUND		PKG_NSF3+90
	internaltext(ERR_MIME_PARSE_NOT_FOUND,"Internal only - String not found in MIME data.")
#define ERR_MIME_PART_OVERFLOW			PKG_NSF3+91
	errortext(ERR_MIME_PART_OVERFLOW,	"Data too large for MIME part item.")
#define ERR_NO_SUCH_TRANSLOG			PKG_NSF3+92
	errortext(ERR_NO_SUCH_TRANSLOG,		"The specified transaction log file is not in the list of files to be archived.")
#define ERR_NO_ARCHIVED_EXTENT 			PKG_NSF3+93
	errortext(ERR_NO_ARCHIVED_EXTENT,	"No extent on the archive list has been archived.")
#define ERR_NO_TRANSLOGS_TO_ARCHIVE 		PKG_NSF3+94
	errortext(ERR_NO_TRANSLOGS_TO_ARCHIVE,	"No transaction log files are waiting to be archived.")
#define ERR_RM_LOG_FORMAT			    PKG_NSF3+95
	errortext(ERR_RM_LOG_FORMAT,     	"Recovery Manager: Log format not supported.")
#define ERR_NO_ARCHIVE_SPECIFIED		PKG_NSF3+96
	errortext(ERR_NO_ARCHIVE_SPECIFIED,	"No archive database specified")
#define ERR_CONTAINER_CORRUPT			PKG_NSF3+97
	internaltext(ERR_CONTAINER_CORRUPT,	"Database container is corrupt and must be rebuilt.")
#define ERR_DBCACHE_CLOSED 				PKG_NSF3+98
	internaltext(ERR_DBCACHE_CLOSED,	"Db Cache is not open")
#define ERR_RM_MQ_LOG_ERROR			    PKG_NSF3+99
	errortext(ERR_RM_MQ_LOG_ERROR,     	"Recovery Manager: Error from Logging Subsystem.")
#define ERR_NSF_CORRUPT_NAMEDOBJECT		PKG_NSF3+100
	errortext(ERR_NSF_CORRUPT_NAMEDOBJECT, "Database is damaged and can't be repaired (Named Object Table).")
#define ERR_EH_DELETE_ENTRY				PKG_NSF3+101
	internaltext(ERR_EH_DELETE_ENTRY,	"Internal only - delete ehash entry.")
#define ERR_EH_UPDATE_ENTRY				PKG_NSF3+102
	internaltext(ERR_EH_UPDATE_ENTRY,	"Internal only - update ehash entry.")
#define ERR_RM_END_OF_LOG			    PKG_NSF3+103
	internaltext(ERR_RM_END_OF_LOG,    	"Recovery Manager: Read past End of Log.")
#define ERR_RM_SCAN_IN_PROGRESS			PKG_NSF3+104
	errortext(ERR_RM_SCAN_IN_PROGRESS, 	"Recovery Manager: Log scan in progress, try again later.")
#define ERR_RM_LOG_SPACE_CRITICAL		PKG_NSF3+105
	errortext(ERR_RM_LOG_SPACE_CRITICAL, "Recovery Manager: Log space is becoming critical.")
#define ERR_DBINUSE				   	 	PKG_NSF3+106
	errortext(ERR_DBINUSE,  	   		"The database is in use and cannot be taken off-line.")
#define ERR_RM_LATE_BACKUP				PKG_NSF3+107
	errortext(ERR_RM_LATE_BACKUP,		"Recovery Manager: Backup was later than recovery point in time.")

#define STR_ARC_CREATED_BY				PKG_NSF3+108
	stringtext(STR_ARC_CREATED_BY,		"\t%s (created by %s on %z)\n\n|\tError creating document link for note ID %lu : %s\n\n")
#define IDX_ARC_DOCLINK_CREATED_BY		1
#define IDX_ARC_DOCLINK_ERROR			2

/* Use OSLoadSubString to load substrings - english total length<80! */
#define STR_ARC_MISC					PKG_NSF3+109
	stringtext(STR_ARC_MISC, 			"Local| (Archive) |DocLink to|Archive Logs|day|days|Archive Logs for %s")
																				  
#define IDX_ARC_VALUE_LOCAL	1
#define IDX_ARC_MISC_TITLE	2
#define IDX_ARC_MISC_LINK	3
#define IDX_ARC_LOG_TITLE	4
#define IDX_ARC_MISC_DAY    5
#define IDX_ARC_MISC_DAYS   6
#define IDX_ARC_LOG_TITLE_FOR 7

#define STR_ARC_TRAILER_TITLE 			PKG_NSF3+110
	stringtext(STR_ARC_TRAILER_TITLE,	"Archive log for documents")
#define STR_ARC_COMPLETED				PKG_NSF3+111
	stringtext(STR_ARC_COMPLETED,		"from %a (%p) to %a (%p) completed successfully on %z.")
#define STR_ARC_EXPIRED_DOCS			PKG_NSF3+112
	stringtext(STR_ARC_EXPIRED_DOCS,	"Expired documents are archived after")
#define STR_ARC_INACTIVE_DOCS			PKG_NSF3+113
	stringtext(STR_ARC_INACTIVE_DOCS,	"Inactive documents are archived after")
#define STR_ARC_MODIFIED_DOCS			PKG_NSF3+114
	stringtext(STR_ARC_MODIFIED_DOCS,	"Modified documents are archived after")
#define ERR_RM_OPEN_BACKUP				PKG_NSF3+115
	errortext(ERR_RM_OPEN_BACKUP,		"Recovery Manager: You must restore or fixup a logged backup before use.")
#define ERR_RM_DBIID_IN_USE				PKG_NSF3+116
	errortext(ERR_RM_DBIID_IN_USE,		"Recovery Manager: Database ID already in use, must use fixup to reassign.")
#define ERR_RM_DOWN_LEVEL				PKG_NSF3+117
	errortext(ERR_RM_DOWN_LEVEL,		"Recovery Manager: Database is not latest copy.")
#define ERR_RM_NOTE_LRFAILURE			PKG_NSF3+118
	errortext(ERR_RM_NOTE_LRFAILURE,	"Recovery Manager: General Failure doing Note Undo/Redo.")
#define ERR_RM_ZAPLSN_NEEDED			PKG_NSF3+119
	internaltext(ERR_RM_ZAPLSN_NEEDED,	"Recovery Manager: Database from different log cannot be verified until normal open.")
#define ERR_RM_RECD_TRUNCATED			PKG_NSF3+120
	internaltext(ERR_RM_RECD_TRUNCATED,	"Recovery Manager: Warning - Log Record Read was Truncated.")
#define ERR_DBOFFLINE					PKG_NSF3+121
	errortext(ERR_DBOFFLINE,			"The database is being taken off-line and cannot be opened.")
#define ERR_RM_RECOVER_NONBACKUP		PKG_NSF3+122
	errortext(ERR_RM_RECOVER_NONBACKUP,	"Recovery Manager: Recovery only supported for Backup Files.")
#define ERR_NO_ARCHIVELOG_SPECIFIED		PKG_NSF3+123
	errortext(ERR_NO_ARCHIVELOG_SPECIFIED,	"No archive log database specified")
#define ERR_NOT_LOCK_DB					PKG_NSF3+124
	errortext(ERR_NOT_LOCK_DB,			"Attempted a lock operation on a DB that doesn't support locking")
#define ERR_NOTE_LOCKED					PKG_NSF3+125
	errortext(ERR_NOTE_LOCKED,			"The document is already locked by %s")
#define ERR_RM_LOG_PARSE_TRUNCATED		PKG_NSF3+126
	internaltext(ERR_RM_LOG_PARSE_TRUNCATED,	"Recovery Manager: Log parse of segments was truncated.")
#define ERR_SERVER_NOT_TRANSLOGGED		PKG_NSF3+127
	errortext(ERR_SERVER_NOT_TRANSLOGGED,	"Transactional Logging must be enabled for this function.")

/* PKG_NSF3 codes limited to 0-127 */

#define ERR_RM_SKIP_FIXUP 				PKG_NSF4+0
	errortext(ERR_RM_SKIP_FIXUP,		"Recovery Manager: Preserving backups by skipping fixup of logged DB without -j switch, DB=%p")
#define ERR_TRANSLOG_NOT_ARCHIVED		PKG_NSF4+1
	internaltext(ERR_TRANSLOG_NOT_ARCHIVED,	"Transaction log not archived.")
#define ERR_RM_NEW_DBIID				PKG_NSF4+2
	errortext(ERR_RM_NEW_DBIID,			"Recovery Manager: Assigning new DBIID for %p (need new backup for media recovery).")
#define ERR_RM_MEDIA_RECOVERY			PKG_NSF4+3
	errortext(ERR_RM_MEDIA_RECOVERY,	"Recovery Manager: Media Recovery complete for %p, last update applied %z.")
#define ERR_BACKUP_ALREADY_IN_PROGRESS	PKG_NSF4+4
	errortext(ERR_BACKUP_ALREADY_IN_PROGRESS,	"Attempt to backup a database that is currently being backed up.")
#define ERR_NOT_DOING_BACKUP			PKG_NSF4+5
	errortext(ERR_NOT_DOING_BACKUP,		"Requested operation only allowed while database is being backed up.")
#define ERR_INVALID_CHANGE_INFO_CONTEXT	PKG_NSF4+6
	errortext(ERR_INVALID_CHANGE_INFO_CONTEXT,	"Change3 Information Context is invalid.")
#define ERR_CORRUPT_CHANGE_INFO			PKG_NSF4+7
	errortext(ERR_CORRUPT_CHANGE_INFO,	"The change information is corrupt or not associated with this file.")
#define ERR_RM_RESTART_RECOVERY			PKG_NSF4+8
	errortext(ERR_RM_RESTART_RECOVERY,	"Recovery Manager: Restart Recovery complete. (%d/%d databases needed full/partial recovery)")
#define ERR_RM_DUP_DBIID       			PKG_NSF4+9
	errortext(ERR_RM_DUP_DBIID,			"Recovery Manager: Warning duplicate DBIID detected: DB1=%p DB2=%p")
#define ERR_RM_STATUS_ANALYSIS 			PKG_NSF4+10
	errortext(ERR_RM_STATUS_ANALYSIS,	"Restart Analysis")
#define ERR_RM_DB_MISSING 				PKG_NSF4+11
	errortext(ERR_RM_DB_MISSING,		"Recovery Manager: Database Instance ID not found for what was")
#define ERR_ADMIN_REQ_DB_NOT_INIT		PKG_NSF4+12
	errortext(ERR_ADMIN_REQ_DB_NOT_INIT,"The Administration Request Database cannot be opened; it has not yet been initialized by the Administration Process.")
#define ERR_SELF_REFERENCING_DOCUMENT	PKG_NSF4+13
	errortext(ERR_SELF_REFERENCING_DOCUMENT ,"Attempt to create a self-referencing document.")
#define ERR_TRANSLOG_ARCHIVE_IN_PROGRESS	PKG_NSF4+14
	errortext(ERR_TRANSLOG_ARCHIVE_IN_PROGRESS,		"Archiving of transaction logs already in progress.")
#define ERR_UBMCANTEVICT				PKG_NSF4+15
	errortext(ERR_UBMCANTEVICT, 		"UBM I/O Method could not evict page.")
#define	ERR_FILEDOCMAX					PKG_NSF4+16
	errortext(ERR_FILEDOCMAX,			"Database (.nsf) has too many documents.")
#define ERR_RM_STATUS_REDO 				PKG_NSF4+17
	errortext(ERR_RM_STATUS_REDO,		"Restart Replay")
#define ERR_RM_STATUS_MREDO 			PKG_NSF4+18
	errortext(ERR_RM_STATUS_MREDO,		"Media Recovery Replay")
#define ERR_EH_AT_MAX					PKG_NSF4+19
	internaltext(ERR_EH_AT_MAX,			 "Operation cannot be performed because ehash has reached max size.")
#define ERR_EH_DATA_TOO_BIG				PKG_NSF4+20
	internaltext(ERR_EH_DATA_TOO_BIG,	 "Data is too large for ehash")
#define ERR_STOP_ENUM					PKG_NSF4+21
	internaltext(ERR_STOP_ENUM,			 "Internal only - stop enumeration (of design notes, named object, ehash, etc)")
#define ERR_BUFFER_TOO_SMALL			PKG_NSF4+22
    internaltext(ERR_BUFFER_TOO_SMALL,	 "Internal only - buffer caller provided is too small")
#define STR_RM_CHECKPOINT_THREAD		PKG_NSF4+23
    stringtext(STR_RM_CHECKPOINT_THREAD, "Recovery Manager Checkpoint Thread")
#define STR_RM_FLUSH_THREAD				PKG_NSF4+24
    stringtext(STR_RM_FLUSH_THREAD,		"Recovery Manager DB Flushing Thread")
#define ERR_CVS_INIT_FAILED				PKG_NSF4+25
	errortext(ERR_CVS_INIT_FAILED,		"CVS initialization failed.")
#define ERR_DUPLICATE_NOTEID			PKG_NSF4+26
	errortext(ERR_DUPLICATE_NOTEID,		"Database already contains a document with this ID (NOTEID)")
#define ERR_ARC_INCORRECT_PROFILE		PKG_NSF4+27
	errortext(ERR_ARC_INCORRECT_PROFILE,	"Incorrect archive profile.")
#define ERR_ARC_REQ_SRC_ACCESS			PKG_NSF4+28
	errortext(ERR_ARC_REQ_SRC_ACCESS,	"Signer does not have the required access rights to the source database.")
#define ERR_ARC_REQ_ARC_ACCESS			PKG_NSF4+29
	errortext(ERR_ARC_REQ_ARC_ACCESS,	"Signer does not have the required access rights to the archive database.")
#define ERR_ARC_INCORRECT_SVR			PKG_NSF4+30
	errortext(ERR_ARC_INCORRECT_SVR,	"Archiving server not configured properly.")
#define ERR_ARC_ARCFILE_NOT_ON_SVR		PKG_NSF4+31
	errortext(ERR_ARC_ARCFILE_NOT_ON_SVR,	"Archive must be on archiving server.")

/* PKG_NSF4 codes limited to 0-31 */

#define ERR_ARC_NOT_ENABLED				PKG_NSF5+0
	errortext(ERR_ARC_NOT_ENABLED,		"Archiving is not enabled.")

#define	ERR_QUOTA_LESS_THAN_WARNING		PKG_NSF5+1
	errortext(ERR_QUOTA_LESS_THAN_WARNING,	"Cannot set quota to less than warning.")

/* Use OSLoadSubString to load substrings - english total length<80!
	Append strings onto this guy but don't delete or change the order */
#define STR_ADDIN_NSF_GEN_MSG_1			PKG_NSF5+2
	stringtext(STR_ADDIN_NSF_GEN_MSG_1,"Creating Administration Requests database")
#define IDX_GEN_CREATE_ADMIN4		1

#define ERR_ADDRESS_MISMATCH			PKG_NSF5+3
	errortext(ERR_ADDRESS_MISMATCH,		"Signer address mismatch")

#define ERR_MIXED_SECURITY				PKG_NSF5+4
	errortext(ERR_MIXED_SECURITY,		"Cannot SMIME encrypt a note that has already been Notes signed or encrypted.")
#define NSFSTR_TRANSLOG_DAMAGED			PKG_NSF5+5
	stringtext(NSFSTR_TRANSLOG_DAMAGED,	"Transaction log is damaged!")
#define NSFSTR_TRANSLOG_BAD_LOCATION	PKG_NSF5+6
	stringtext(NSFSTR_TRANSLOG_BAD_LOCATION,	"Transaction log path does not exist or is not accessible.")
#define NSFSTR_TRANSLOG_NEW_PATH		PKG_NSF5+7
	stringtext(NSFSTR_TRANSLOG_NEW_PATH,"Unable to create or access transaction log path")
#define NSFSTR_TRANSLOG_OLD_PATH		PKG_NSF5+8
	stringtext(NSFSTR_TRANSLOG_OLD_PATH,"Path to transaction log does not match actual path, using old path. Please UPDATE SERVER RECORD.")
#define ERR_DUPLICATE_UNK				PKG_NSF5+9
	internaltext(ERR_DUPLICATE_UNK,		"Duplicate UNK found in UNK table")
#define ERR_CMD_SHOW_NSFPOOL			PKG_NSF5+10
	stringtext(ERR_CMD_SHOW_NSFPOOL,	 "NSFPOOL")
#define ERR_HELP_SHOW_NSFPOOL			PKG_NSF5+11
	stringtext(ERR_HELP_SHOW_NSFPOOL, 	"* Show the contents of the Server's NSF pool.")
#define ERR_MIME_NO_822HDR_NAME			PKG_NSF5+12
	errortext(ERR_MIME_NO_822HDR_NAME,	"Invalid or missing RFC822 header name.")
#define ERR_MIME_NO_822HDR_DELIM		PKG_NSF5+13
	errortext(ERR_MIME_NO_822HDR_DELIM,	"Invalid or missing RFC822 header delimiter.")
#define ERR_AGER_FILE_OPEN_FAILED		PKG_NSF5+14
	internaltext(ERR_AGER_FILE_OPEN_FAILED, "Ager failed to open database.")
#define ERR_CHILD_MUST_BE_DATA_NOTE		PKG_NSF5+15
	errortext(ERR_CHILD_MUST_BE_DATA_NOTE, "Children of DATA notes must be DATA notes.")

	/* PKG_NSF5 codes limited to 0-15 */

#define ERR_EM_SUCCESS	 		PKG_NSF6+0
	internaltext(ERR_EM_SUCCESS, "success")
#define ERR_NSF_NOT_SUPPORTED			PKG_NSF6+1
	errortext(ERR_NSF_NOT_SUPPORTED, "This routine is not supported on this platform.")
#define	ERR_LOG_RESPONSE_CYCLE			PKG_NSF6+2
	errortext(ERR_LOG_RESPONSE_CYCLE,	"This operation on %p (NoteID = %lu) creates a cycle in the response hierarchy")
#define ERR_INVALID_BT_CONTEXT_SIG		PKG_NSF6+3
	errortext(ERR_INVALID_BT_CONTEXT_SIG,		"Invalid btree context signature")
#define ERR_INVALID_BT_ENTRY			PKG_NSF6+4
	errortext(ERR_INVALID_BT_ENTRY,				"Invalid btree entry")
#define ERR_INVALID_BT_LEVEL			PKG_NSF6+5
	errortext(ERR_INVALID_BT_LEVEL,				"Invalid btree level")
#define ERR_INVALID_BT_SIBLING_DIR		PKG_NSF6+6
	errortext(ERR_INVALID_BT_SIBLING_DIR,		"Invalid btree sibling direction")
#define ERR_CORRUPT_BT_CONTEXT_SIG		PKG_NSF6+7
	errortext(ERR_CORRUPT_BT_CONTEXT_SIG,		"Btree context signature marked corrupt")
#define ERR_NULL_BT_CONTEXT_SIG			PKG_NSF6+8
	errortext(ERR_NULL_BT_CONTEXT_SIG,			"Btree context signature is NULL")
#define ERR_INVALID_BT_NODESIZE			PKG_NSF6+9
	errortext(ERR_INVALID_BT_NODESIZE,			"Invalid btree node size")
#define ERR_INVALID_LONGDATA_SIZE		PKG_NSF6+10
	errortext(ERR_INVALID_LONGDATA_SIZE,		"Invalid btree longdata size")
#define ERR_BT_ENTRY_NOT_MAX			PKG_NSF6+11
	errortext(ERR_BT_ENTRY_NOT_MAX,				"Invalid btree max entry")
#define ERR_INVALID_BT_SUBINDEX_REFCNT	PKG_NSF6+12
	errortext(ERR_INVALID_BT_SUBINDEX_REFCNT,	"Invalid btree subindex reference count")
#define ERR_INVALID_BT_DESC_DATASIZE	PKG_NSF6+13
	errortext(ERR_INVALID_BT_DESC_DATASIZE,		"Invalid btree descriptor data size")
#define ERR_INVALID_BT_DATASIZE			PKG_NSF6+14
	errortext(ERR_INVALID_BT_DATASIZE,			"Invalid btree entry data size")
#define ERR_INVALID_BT_KEYSIZE			PKG_NSF6+15
	errortext(ERR_INVALID_BT_KEYSIZE,			"Invalid btree entry key size")

	/* PKG_NSF6 codes limited to 0-15 */

#define ERR_BT_SPLIT_SLOTTED_NODE		PKG_NSF7+0
	errortext(ERR_BT_SPLIT_SLOTTED_NODE,		"Invalid btree split of a slotted node")
#define ERR_BT_SPLIT_POINT				PKG_NSF7+1
	errortext(ERR_BT_SPLIT_POINT,				"Invalid btree split point")
#define ERR_BT_BAD_POSITION				PKG_NSF7+2
	errortext(ERR_BT_BAD_POSITION,				"Invalid btree node position")
#define ERR_BT_DELETE_NONEMPTY_ROOT		PKG_NSF7+3
	errortext(ERR_BT_DELETE_NONEMPTY_ROOT,		"Invalid btree delete of a nonempty root node")
#define ERR_BT_TRAVERSAL_BROKEN			PKG_NSF7+4
	errortext(ERR_BT_TRAVERSAL_BROKEN,			"Invalid btree traversal")
#define ERR_BT_BAD_CARDINALITY			PKG_NSF7+5
	errortext(ERR_BT_BAD_CARDINALITY,			"Invalid btree cardinality")
#define ERR_INVALID_BT_DESC				PKG_NSF7+6
	errortext(ERR_INVALID_BT_DESC,				"Invalid btree descriptor")
#define ERR_INVALID_BT_ADDR				PKG_NSF7+7
	errortext(ERR_INVALID_BT_ADDR,				"Invalid btree address")
#define	ERR_CMD_SET_SCOS			PKG_NSF7+8
	stringtext(ERR_CMD_SET_SCOS,"SCOS [Database Path] [Active/Inactive]")
#define	ERR_HELP_SET_SCOS			PKG_NSF7+9
	stringtext(ERR_HELP_SET_SCOS,"Activate/Inactivate SCOS database")
#define ERR_ROUTER_SCOS_INVALID		(PKG_NSF7+10)
	errortext(ERR_ROUTER_SCOS_INVALID,	"\nRouter: obsolete shared mail command.")
#define ERR_SCOS_NUM_FILES_REQ		PKG_NSF7+11
	errortext(ERR_SCOS_NUM_FILES_REQ, "\nRequested files (%d) for %s exceeds the server maximum %d files.\nCreating: %d file(s).\n")
#define ERR_SCOS_DIR_MAX_FILE_SPEC		PKG_NSF7+12
	errortext(ERR_SCOS_DIR_MAX_FILE_SPEC, "SCOS_DIR_MAX_FILES_%d")
#define ERR_SCOS_DIR_MAX_SIZE_SPEC		PKG_NSF7+13
	errortext(ERR_SCOS_DIR_MAX_SIZE_SPEC, "SCOS_DIR_MAX_SIZE_MB_%d")
#define ERR_SCOS_DIR_MAX_FILE_SIZE_SPEC	PKG_NSF7+14
	errortext(ERR_SCOS_DIR_MAX_FILE_SIZE_SPEC, "SCOS_DIR_MAX_FILE_SIZE_MB_%d")
#define ERR_SCOS_NOT_RUNNING	PKG_NSF7+15
	errortext(ERR_SCOS_NOT_RUNNING,	"The SCOS database is not running.\n")
#define ERR_DBG_SHOW_HEAD		PKG_NSF7+16
	errortext(ERR_DBG_SHOW_HEAD,"\n\nDatabase\t\tAvailability\tState\t\t\tSize\n")
#define ERR_NOTE_ALREADY_LOCKED_BY_LOCKER	PKG_NSF7+17
	errortext(ERR_NOTE_ALREADY_LOCKED_BY_LOCKER,	"You already have the document locked")
#define ERR_NOTE_NOT_LOCKED_BY_LOCKER	   	PKG_NSF7+18
	errortext(ERR_NOTE_NOT_LOCKED_BY_LOCKER,	"The document is not locked by you")
#define ERR_DBG_KB				PKG_NSF7+19
	errortext(ERR_DBG_KB,		 "KB")
#define ERR_DBG_MB				PKG_NSF7+20
	errortext(ERR_DBG_MB,		 "MB")
#define ERR_DBG_INV_PATH		PKG_NSF7+21
	errortext(ERR_DBG_INV_PATH, "\nInvalid path: %s")
#define ERR_DBG_ACTIVE			PKG_NSF7+22
	errortext(ERR_DBG_ACTIVE,	 "Active  ")
#define ERR_DBG_INACTIVE		PKG_NSF7+23
	errortext(ERR_DBG_INACTIVE, "Inactive")
#define ERR_NO_MORE_RRVS				PKG_NSF7+24
	errortext(ERR_NO_MORE_RRVS,			"Cannot allocate RRV, please create new replica.")
#define ERR_IMAP_ALREADYENABLED			PKG_NSF7+25
	errortext(ERR_IMAP_ALREADYENABLED,	"Database is already IMAP enabled")
#define ERR_IMAP_ALREADYDISABLED		PKG_NSF7+26
	errortext(ERR_IMAP_ALREADYDISABLED,	"Database is already IMAP disabled")
#define ERR_INVALID_FOLDER_REFCOUNT_OP	PKG_NSF7+27
	errortext(ERR_INVALID_FOLDER_REFCOUNT_OP,	"Invalid folder refcount operation")
#define ERR_NOACCESS_FOR_REPORCOPY			PKG_NSF7+28
	errortext(ERR_NOACCESS_FOR_REPORCOPY,	"You are not authorized to replicate or copy data from this database.")
#define ERR_DBG_OPEN_DELIVERY			PKG_NSF7+29
	errortext(ERR_DBG_OPEN_DELIVERY ,   "open for delivery  ")
#define ERR_DBG_CLOSED_DELIVERY		PKG_NSF7+30
	errortext(ERR_DBG_CLOSED_DELIVERY , "closed for delivery")
#define ERR_DBG_TOTAL_SIZE				PKG_NSF7+31
	errortext(ERR_DBG_TOTAL_SIZE, "\n\nTotal Database Disk Size in Directory: %d.%02d MB")

	/* PKG_NSF7 codes limited to 0-31 */

#define ERR_DBG_AVAIL_SIZE				PKG_NSF8+0
	errortext(ERR_DBG_AVAIL_SIZE, 		"\nTotal Database Disk Available in Directory: %d.%02d MB")
#define ERR_DBG_FREE_SIZE				PKG_NSF8+1
	errortext(ERR_DBG_FREE_SIZE, 		"\n\nTotal Database Internal Free Space for Directory: %d.%02d MB\n\n")
#define ERR_DBG_ENABLED		PKG_NSF8+2
	errortext(ERR_DBG_ENABLED ,  "Enabled  ")
#define ERR_DBG_DISABLED		PKG_NSF8+3
	errortext(ERR_DBG_DISABLED , "Error    ")
#define ERR_SCOS_DIR			PKG_NSF8+4
	errortext(ERR_SCOS_DIR,		 "\nRequested zero files (%d), or max size (%d) for the SCOS directory %s.\n")
#define ERR_SCOS_FILE_SIZE		PKG_NSF8+5
	errortext(ERR_SCOS_FILE_SIZE,"\nSCOS database specification out of range in the notes.ini file. The size of the database is in megabyte units.")
#define ERR_SCOS_SIZE_FILES		PKG_NSF8+6
	errortext(ERR_SCOS_SIZE_FILES, "\nSCOS directory %s - total size (%d) exceeds maximum (%d).\n")
#define ERR_SCOS_NO_DIR			PKG_NSF8+7
	errortext(ERR_SCOS_NO_DIR, 	 "\nDirectory %s ignored, the directory (or link) does not exist.\n")
#define ERR_DBG_INV_SPEC		PKG_NSF8+8
	errortext(ERR_DBG_INV_SPEC, "\nDirectory %s ignored, invalid specification - %s\n")
#define ERR_SCOS_CREATE			PKG_NSF8+9
	errortext(ERR_SCOS_CREATE,   "\nCreating SCOS file: %s.\n")
#define ERR_DBG_DUPE			PKG_NSF8+10
	errortext(ERR_DBG_DUPE, 	"Duplicate")
#define ERR_SCOS_NO_CREATE		PKG_NSF8+11
	errortext(ERR_SCOS_NO_CREATE, "\nCould not create SCOS database %s.\n")
#define ERR_SCOS_DIR_SPEC				PKG_NSF8+12
	errortext(ERR_SCOS_DIR_SPEC,		"SCOS_DIR_%d")
#define ERR_SCOS_DIR_NUM_FILE_SPEC		PKG_NSF8+13
	errortext(ERR_SCOS_DIR_NUM_FILE_SPEC, "SCOS_DIR_ACTIVE_FILES_%d")
#define ERR_DBG_SHOW_DIR				PKG_NSF8+14
	errortext(ERR_DBG_SHOW_DIR,			"\n\nDirectory: %s - %s")
#define ERR_DBG_SHOW_REQ				PKG_NSF8+15
	errortext(ERR_DBG_SHOW_REQ, "\nNumber of delivery databases requested: %d.\nNumber of databases: %d\nMaximum Directory Size: %d MB")
#define ERR_CMD_SHOW_SCOS			PKG_NSF8+16
	stringtext(ERR_CMD_SHOW_SCOS,"SCOS")
#define	ERR_HELP_SHOW_SCOS	 	PKG_NSF8+17
	stringtext(ERR_HELP_SHOW_SCOS,"Single copy object store information")
#define ERR_NSFIMAP_FOLDERSLIST_POOLFULL		PKG_NSF8+18
	errortext(ERR_NSFIMAP_FOLDERSLIST_POOLFULL,	"AVL tree for IMAP folder names is full.")
#define ERR_NSFIMAP_INVALID_ARG			PKG_NSF8+19
	errortext(ERR_NSFIMAP_INVALID_ARG,					"Invalid IMAP command argument")
#define ERR_NSFIMAP_INTERNAL_SEARCH		(PKG_NSF8+20)
	errortext(ERR_NSFIMAP_INTERNAL_SEARCH,				"Internal IMAP search error")
#define ERR_NSFIMAP_UNSUPPORTED_CHARSET	(PKG_NSF8+21)
	errortext(ERR_NSFIMAP_UNSUPPORTED_CHARSET,			"Unsupported CHARSET")
#define ERR_NSFIMAP_SEARCH_UNSUPPORTED_CRITERIA_SIZE	(PKG_NSF8+22)
	errortext(ERR_NSFIMAP_SEARCH_UNSUPPORTED_CRITERIA_SIZE,	"Searching by size (SMALLER, LARGER) is not supported")
#define ERR_NSFIMAP_SEARCH_UNSUPPORTED_CRITERIA			(PKG_NSF8+23)
	errortext(ERR_NSFIMAP_SEARCH_UNSUPPORTED_CRITERIA,	"Unsupported search criteria")
#define ERR_NOT_DBIMAP_ENABLED			PKG_NSF8+24
	errortext(ERR_NOT_DBIMAP_ENABLED,	"the database has not been enabled for IMAP support")
#define ERR_DBG_TOO_LARGE				PKG_NSF8+25
	errortext(ERR_DBG_TOO_LARGE , 		"\nTotal size of the databases in directory is too large.")
#define ERR_IMAPFOLDER_NOT_FOUND		PKG_NSF8+26
	errortext(ERR_IMAPFOLDER_NOT_FOUND,	"Folder not found in IMAP name space")
#define ERR_INVALID_IMAP_FLAGS			PKG_NSF8+27
	errortext(ERR_INVALID_IMAP_FLAGS,	"Invalid IMAP flags value")
#define ERR_INVALID_DBIMAP_CMD			PKG_NSF8+28
	errortext(ERR_INVALID_DBIMAP_CMD,	"Invalid IMAP command flag")
#define ERR_EM_RELOAD_NOTE 				PKG_NSF8+29
	internaltext(ERR_EM_RELOAD_NOTE,	"Reload hNote after NSFNoteUpdate")
#define ERR_SCOS_DUPE_FILE				PKG_NSF8+30
	errortext(ERR_SCOS_DUPE_FILE, 		"\nSCOS database ignored: %s duplicate name to: %s\n")
#define ERR_NO_SERVER_FOR_DB			PKG_NSF8+31
	errortext(ERR_NO_SERVER_FOR_DB,		"No server containing a replica of the database is available")
#define ERR_DB_NOT_LOCATED				PKG_NSF8+32
	errortext(ERR_DB_NOT_LOCATED,		"Database can not be located in the Cluster")
#define	ERR_ENTRY_NOT_HASHED			PKG_NSF8+33
	errortext(ERR_ENTRY_NOT_HASHED,		"Entry is not Hashed")
#define	ERR_HASH_NOT_CLEANED			PKG_NSF8+34
	errortext(ERR_HASH_NOT_CLEANED,		"There are no entries to be cleaned from Hash")
#define ERR_NSFXACL_OBJXACL_NOT_FOUND	PKG_NSF8+35
        errortext(ERR_NSFXACL_OBJXACL_NOT_FOUND,    "extended ACL not found in the object")
#define ERR_NSF_XACL_POOLFULL			PKG_NSF8+36
	errortext(ERR_NSF_XACL_POOLFULL,	"Insufficient memory - NSF extended ACL pool is full.")
#define	ERR_SERVER_NOT_CLUSTERED		PKG_NSF8+37
	errortext(ERR_SERVER_NOT_CLUSTERED,	"Server is not in the Cluster")
#define ERR_CLCACHE_NOT_INITIALIZED		PKG_NSF8+38
	errortext(ERR_CLCACHE_NOT_INITIALIZED, "NSF Cluster Name Cache is not initialized yet")
#define ERR_NSF_XACL_UNIDREBUILD		PKG_NSF8+39
	errortext(ERR_NSF_XACL_UNIDREBUILD, "Extended access control option has changed. Rebuilding UNID Index in database %s...")
#define ERR_XML_EXCEPTION               PKG_NSF8+40
    errortext(ERR_XML_EXCEPTION,        "\nXML parser exception occurred")
#define ERR_XML_BUF_TOO_SMALL           PKG_NSF8+41
    errortext(ERR_XML_BUF_TOO_SMALL,    "\nAllocated buffer too small")
#define ERR_XML_WRONG_NODE_TYPE         PKG_NSF8+42
    errortext(ERR_XML_WRONG_NODE_TYPE,  "\nUnrecognized node type")
#define ERR_XML_CANNOT_OPEN_FILE        PKG_NSF8+43
    errortext(ERR_XML_CANNOT_OPEN_FILE, "\nCannot open the file")
#define ERR_NOINPLACECOMPACT			PKG_NSF8+44
	errortext(ERR_NOINPLACECOMPACT,		"In-place database compaction could not be performed as requested")
#define ERR_INVALID_SEQNUM				PKG_NSF8+45
	errortext(ERR_INVALID_SEQNUM,		"Invalid IMAP sequence number")
#define ERR_DN_NOT_FOUND                PKG_NSF8+46
    errortext(ERR_DN_NOT_FOUND,			"Note DN not found")
#define ERR_XML_ITEM_NOT_FOUND          PKG_NSF8+47
    errortext(ERR_XML_ITEM_NOT_FOUND,   "\nNote Item could not be found")
#define ERR_XML_ITEM_EXISTS             PKG_NSF8+48
    errortext(ERR_XML_ITEM_EXISTS,      "\nNote Item already exists")
#define ERR_XML_SAXEXCEPTION            PKG_NSF8+49
    errortext(ERR_XML_SAXEXCEPTION,     "\nXML SAX parser exception")
#define ERR_SCHED_CANT_LOOKUP_USER			PKG_NSF8+50
	errortext(ERR_SCHED_CANT_LOOKUP_USER,	"Error looking up user %a in Domino Directory for scheduling request")
#define ERR_SCHED_USER_NOT_IN_NAB			PKG_NSF8+51
	errortext(ERR_SCHED_USER_NOT_IN_NAB,	"Cannot find user in Domino Directory")
#define ERR_SCHED_MULTIPLE_USERS			PKG_NSF8+52
	errortext(ERR_SCHED_MULTIPLE_USERS,		"Warning: Multiple matches found for %a in Domino Directory, using first one found")
#define ERR_SCHED_USER_IS_FORWARDED			PKG_NSF8+53
	errortext(ERR_SCHED_USER_IS_FORWARDED,	"User's mail is being forwarded to another location")
#define ERR_SCHED_USER_NOT_IN_CALDOMAIN		PKG_NSF8+54
	errortext(ERR_SCHED_USER_NOT_IN_CALDOMAIN,	"User's calendar is not in this domain")
#define ERR_NO_MASTER_LOCK_DB					PKG_NSF8+55
	errortext(ERR_NO_MASTER_LOCK_DB,			"Unable to connect to Master Lock Database")
#define ERR_LOCK_PROMOTED						PKG_NSF8+56
	errortext(ERR_LOCK_PROMOTED,				"A provisional lock in database '%s' has been promoted to a hard lock. Document link is attached to this message")
#define ERR_LOCK_NOT_PROMOTED					PKG_NSF8+57
  	errortext(ERR_LOCK_NOT_PROMOTED,			"A provisional lock in database '%s' has NOT been promoted to a hard lock. There was a conflict detected. Your changes may be found in body of this message.  A document link to the original document is attached to the end of the message")
#define ERR_CHANGES_PROMOTED						PKG_NSF8+58
	errortext(ERR_CHANGES_PROMOTED,				"Changes made to database '%s' have been incorporated into the database with no conflicts.  Document link is attached to message")
#define ERR_CHANGES_NOT_PROMOTED					PKG_NSF8+59
	errortext(ERR_CHANGES_NOT_PROMOTED,			"Changes made to database '%s' have NOT been incorporated into the database. There was a conflict detected. Your changes may be found in body of this message.  A document link to the original document is attached to the end of the message.")
#define ERR_NO_CLHASH						PKG_NSF8+60
	errortext(ERR_NO_CLHASH,				"Cluster Hash Table has not been Initialized yet")
#define ERR_DUP_SINGLE_COPY_TEMPLATE	PKG_NSF8+61
	errortext(ERR_DUP_SINGLE_COPY_TEMPLATE,	"Multiple single copy templates with the same name are not allowed.")
#define ERR_SINGLE_COPY_TEMPLATE_NAME		PKG_NSF8+62
	errortext(ERR_SINGLE_COPY_TEMPLATE_NAME, "Single copy template must have a unique master template name, and the file name must end in '.nt*'.")
#define ERR_RESYNC_SINGLE_COPY_TEMPLATE		PKG_NSF8+63
	errortext(ERR_RESYNC_SINGLE_COPY_TEMPLATE, "Need to run design refresh on database %s to bring it up to date with its single copy template.")
#define ERR_FIXING_DONE						PKG_NSF8+64
	errortext(ERR_FIXING_DONE,					"Completed consistency check on %s")
#define ERR_LKMGR_NOT_FOUND				PKG_NSF8+65
	internaltext(ERR_LKMGR_NOT_FOUND,	"Lock Manager: Lock Not Found")
#define ERR_LKMGR_DEADLOCK				PKG_NSF8+66
	errortext(ERR_LKMGR_DEADLOCK,		"Lock Manager: Deadlock detected, retry transaction latter")
#define ERR_FALSE_DEADLOCK				PKG_NSF8+67
	internaltext(ERR_FALSE_DEADLOCK,	"Lock Manager: False Deadlock detected")
#define ERR_LKMGR_POOL_FAILURE			PKG_NSF8+68
	errortext(ERR_LKMGR_POOL_FAILURE,	"Lock Manager: Insufficient memory - Lock Manager Pool is full")
#define ERR_KEY_DELETED					PKG_NSF8+69
	internaltext(ERR_KEY_DELETED,		"Found a Pseudo Deleted Key")
#define ERR_FOLDERSLIST_TOO_BIG			PKG_NSF8+70
	errortext(ERR_FOLDERSLIST_TOO_BIG,			"NSF Folders list for IMAP is too big")
#define ERR_NSFXACL_INHERITED				PKG_NSF8+71
	errortext(ERR_NSFXACL_INHERITED,		"Access Control Information was inherited from other object")
#define ERR_ARCHIVE_NO_MATCH_FOR_SERVER		PKG_NSF8+72
	errortext(ERR_ARCHIVE_NO_MATCH_FOR_SERVER, "No match for server found")
#define ERR_NOTE_DELETED_BY_RULE			PKG_NSF8+73
	errortext(ERR_NOTE_DELETED_BY_RULE,			"Document has been rejected by mail rule")
#define ERR_ARCHIVE_ON_SERVER_ENABLED 	PKG_NSF8+74
	errortext(ERR_ARCHIVE_ON_SERVER_ENABLED,	"Server based archiving enabled - unable to initiate archiving from local workstation")
#define STR_LK_REPORTING_THREAD			PKG_NSF8+75
	internaltext(STR_LK_REPORTING_THREAD, "Lock Manager Reporting Thread")
#define ERR_SCOS_DIR_NOT_CONFIGURED			PKG_NSF8+76
	errortext(ERR_SCOS_DIR_NOT_CONFIGURED, 		"SCOS_DIR_NOT_CONFIGURED_%d")
#define ERR_DBG_CLOSED_CONFIG				PKG_NSF8+77
	errortext(ERR_DBG_CLOSED_CONFIG,    "not configured     ")
#define ERR_DBG_SUMMARY_HDR				PKG_NSF8+78
	stringtext(ERR_DBG_SUMMARY_HDR,	"\n\nDirectory\tAvailability\tRequested\t   Actual\tMax Size\n\n")
#define ERR_DBG_SUMMARY_INFO				PKG_NSF8+79
	stringtext(ERR_DBG_SUMMARY_INFO,	"%s\n\t\t%s\t%d\t\t%d\t%d\n")
#define ERR_DBG_SUMMARY_TAIL				PKG_NSF8+80
	stringtext(ERR_DBG_SUMMARY_TAIL,	"\nTotals\t\t\t\t\t%d\t\t%d\t%d\n")
#define ERR_SCOS_DIRLINK_CONFLICT			PKG_NSF8+81
	errortext(ERR_SCOS_DIRLINK_CONFLICT, "\nDirectory %s conflicts with SCOS directory link %s%s to directory %s.\n")
#define ERR_DBG_DISABLED_DB				PKG_NSF8+82
	errortext(ERR_DBG_DISABLED_DB ,  "The object store database is disabled.  ")
#define ERR_SCOS_NUM_FILES_DIR	PKG_NSF8+83
	errortext(ERR_SCOS_NUM_FILES_DIR, "SCOS directory %s - the total number of specified files (%d) exceeds the maximum (%d).\n")
#define ERR_ARCH_CANT_MOD_EFFECTIVE_POLICY 	PKG_NSF8+84
	errortext(ERR_ARCH_CANT_MOD_EFFECTIVE_POLICY,  "Cannot modify settings in the effective archive policy")
#define ERR_NOT_AN_ADMINISTRATOR 			PKG_NSF8+85
	errortext(ERR_NOT_AN_ADMINISTRATOR,	   "You must be a server administrator to perform this action")
/* Logging for Roaming Users (NameLookupUserID) */
#define STR_ROAMING_USER_ACCESS				PKG_NSF8+86
	errortext(STR_ROAMING_USER_ACCESS,		"Roaming user %s password failed")
#define ERR_RM_TRAN_NESTING					PKG_NSF8+87
	errortext(ERR_RM_TRAN_NESTING,			"Recovery Manager: Transaction nesting exceeded supported maximum or no transaction in progress")
#define ERR_RM_LONG_TRANSACTION_ABORT		PKG_NSF8+88
	errortext(ERR_RM_LONG_TRANSACTION_ABORT, "Recovery Manager: Transaction ran too long, must abort")
#define ERR_ONLY_LOGGED_DATABASES			PKG_NSF8+89
	errortext(ERR_ONLY_LOGGED_DATABASES, 	"Function only supported for transaction logged databases")
#define ERR_REPLACE							PKG_NSF8+90
	errortext(ERR_REPLACE,"Replace existing %p?")
#define STR_SCOS_ONLINE_CFG					PKG_NSF8+91
	stringtext(STR_SCOS_ONLINE_CFG,		"SCOS_ONLINE_%d")
#define STR_SCOS_ACTIVE_CFG					PKG_NSF8+92
	stringtext(STR_SCOS_ACTIVE_CFG,		"SCOS_ACTIVE_%d")
#define STR_SCOS_OFFLINE					PKG_NSF8+93
	stringtext(STR_SCOS_OFFLINE,			"offline             ")
#define ERR_DBG_DB_INACTIVE				PKG_NSF8+94
	errortext(ERR_DBG_DB_INACTIVE, 	"Object store database Inactive.")
#define ERR_DBG_DIR_INACTIVE				PKG_NSF8+95
	errortext(ERR_DBG_DIR_INACTIVE, 	"Object store directory Inactive.")
#define ERR_DBG_DIR_OFFLINE				PKG_NSF8+96
	errortext(ERR_DBG_DIR_OFFLINE, 	"Object store directory offline.")
#define ERR_SOFTDELETE_EXPIRE_TIME			PKG_NSF8+97
	errortext(ERR_SOFTDELETE_EXPIRE_TIME,	"Soft delete expire time must be a whole number greater than zero.")
#define	ERR_NOTE_INVSIG11					PKG_NSF8+98
	errortext(ERR_NOTE_INVSIG11,	"Data signature passed; object signature invalid")
#define ERR_SWITCH_TO_COPYSTYLE			PKG_NSF8+99
	errortext(ERR_SWITCH_TO_COPYSTYLE,		"Switching to copy-style compaction for %p due to unique fields names table size")
#define ERR_USE_LARGEUNK_OR_COPYSTYLE	PKG_NSF8+100
	errortext(ERR_USE_LARGEUNK_OR_COPYSTYLE, 	"Unique field names table filling up for %p: Set 'Allow more fields in database' option or use copy-style compaction to reduce")
#define ERR_USE_COPYSTYLE				PKG_NSF8+101
	errortext(ERR_USE_COPYSTYLE, 				"Unique field names table filling up for %p: Use copy-style compaction to reduce")
#define	ERR_IMAP_KEYWORD_TOO_LONG			PKG_NSF8+102
	errortext(ERR_IMAP_KEYWORD_TOO_LONG, "IMAP Keyword flag names cannot be longer than 31 bytes")
#define ERR_DOCCACHE_POOLFULL				PKG_NSF8+103
	errortext(ERR_DOCCACHE_POOLFULL, 		"Insufficient memory - Document cache pool is full.")
#define ERR_CANT_INIT_ASP_CONTEXT           PKG_NSF8+104
	errortext(ERR_CANT_INIT_ASP_CONTEXT,    "Unable to initialize parse the DN %s")
#define ERR_INVALID_ASP_USER_NAME           PKG_NSF8+105
	errortext(ERR_INVALID_ASP_USER_NAME,    "%s is an invalid user name in an ASP configuration")
#define ERR_EM_CONTINUE_VNOTE				PKG_NSF8+106
	internaltext(ERR_EM_CONTINUE_VNOTE,		"Continue Virtual Note processing.")
#define ERR_INVALID_VNOTE					PKG_NSF8+107
	errortext(ERR_INVALID_VNOTE,			"Virtual Note has an invalid format.")
#define ERR_COLLREBUILDEXTRACOL 			PKG_NSF8+108
	errortext(ERR_COLLREBUILDEXTRACOL,	"Informational, rebuilding view - external format note has been modified since last rebuild. (reading %s %s note Title:'%s')")
#define	ERR_TARGET_DB_CLOSED 				PKG_NSF8+109
	errortext(ERR_TARGET_DB_CLOSED,			"Cannot access the document's target database to copy attachments")
#define ERR_NO_NOTE_UPDATE					PKG_NSF8+110
	errortext(ERR_NO_NOTE_UPDATE,			"Cannot update note due to NOTE_FLAG2_NO_UPDATE being set")
#define ERR_NO_NOTE_CHANGE					PKG_NSF8+111
	errortext(ERR_NO_NOTE_CHANGE,			"Cannot modify note due to NOTE_FLAG2_NO_CHANGE being set")

#define ERR_SIGNER_MUST_BE_POL_CREATOR_MOD 	(PKG_NSF8+112)
	errortext(ERR_SIGNER_MUST_BE_POL_CREATOR_MOD,	"The Policy and/or Settings Document assigned to you has been edited by an unauthorized person. Please notify your Administrator that you cannot proceed with the client setup.")
#define ERR_POLICY_SIGNER_NOT_POL_CREATOR_MOD 	(PKG_NSF8+113)
	errortext(ERR_POLICY_SIGNER_NOT_POL_CREATOR_MOD,	"%s, the signer of policy document %s, does not have the required PolicyCreator or PolicyModifier role to the Domino Directory.")
#define ERR_POLICY_SIGNATURE_MSG 			(PKG_NSF8+114)
	errortext(ERR_POLICY_SIGNATURE_MSG,	"The effective policy could not be derived - the following error was encountered while processing policy document %s: %e.")

#define	ERR_CANNOT_DELEGATE PKG_NSF8+115
	errortext(ERR_CANNOT_DELEGATE,"You may not delegate to this person, as he/she has already been invited to this meeting. Please choose another delegee for this meeting.")

#define	ERR_MESSAGE_DO_NOT_DELIVER PKG_NSF8+116
	errortext(ERR_MESSAGE_DO_NOT_DELIVER,"This message will not be delivered by the router because an error occurred in processing.")
#define ERR_EH_PSEUDO_DELETED				PKG_NSF8+117
	internaltext(ERR_EH_PSEUDO_DELETED,		"Key in Extensible hash exists but is flag deleted")

#define ERR_NOTE_LOCKED_BYSOMEONE		PKG_NSF8+118
	errortext(ERR_NOTE_LOCKED_BYSOMEONE,	"The document is already locked by someone else.")

/* Logging for Roaming Users (NameLookupUserID) */
#define STR_ROAMING_USER_ID_VIOLATION		PKG_NSF8+119
	errortext(STR_ROAMING_USER_ID_VIOLATION,	"You are trying to access a roaming user id which has not been properly roamed. Please re-roam.")

#define ERR_DBIMAP_MUST_BE_REENABLED			PKG_NSF8+120
	errortext(ERR_DBIMAP_MUST_BE_REENABLED,	"Database %p must be re-enabled for IMAP support")

#define ERR_ARC_REQ_SRC_USER_ACCESS			PKG_NSF8+121
	errortext(ERR_ARC_REQ_SRC_USER_ACCESS,	"You do not have the required access rights to the source database.")
#define ERR_ARC_REQ_ARC_USER_ACCESS			PKG_NSF8+122
	errortext(ERR_ARC_REQ_ARC_USER_ACCESS,	"You do not have the required access rights to the archive database.")
#define ERR_ARC_SETTINGS_SIGNATURE			PKG_NSF8+123
	errortext(ERR_ARC_SETTINGS_SIGNATURE, 	"Signature on archive settings is invalid")
#define ERR_ARCHIVE_POLICY_PROHIBITED			PKG_NSF8+124
	errortext(ERR_ARCHIVE_POLICY_PROHIBITED	, "Archiving is prohibited by a policy set by an administrator")

#define	ERR_IMAP_KEYWORDS_TOO_LARGE			PKG_NSF8+125
	errortext(ERR_IMAP_KEYWORDS_TOO_LARGE, "IMAP Keyword flags length cannot exceed 32 KB")

#define ERR_ARC_RETRIEVE_SETTINGS			PKG_NSF8+126
	errortext(ERR_ARC_RETRIEVE_SETTINGS, 	"Unable to retrieve archiving settings")
#define ERR_ARC_SELECTING_DOCS				PKG_NSF8+127
	errortext(ERR_ARC_SELECTING_DOCS,		"Unable to select documents for archiving")

/* PKG_NSF8 codes limited to 0-127 */

#define ERR_ARC_COPY_DOCUMENTS				PKG_NSF9+0
	errortext(ERR_ARC_COPY_DOCUMENTS,		"Unable to copy documents to archive database")
#define ERR_ARC_DELETE_DOCUMENTS			PKG_NSF9+1
	errortext(ERR_ARC_DELETE_DOCUMENTS,		"Unable to delete documents from source database")
#define ERR_ARC_TRUNCATE_DOCUMENTS			PKG_NSF9+2
	errortext(ERR_ARC_TRUNCATE_DOCUMENTS,   "Unable to truncate documents from source database")
#define STR_ARC_ARCHIVING					PKG_NSF9+3
	stringtext(STR_ARC_ARCHIVING,			"Archiving|Calendar Cleanup|Done.|done. %d docs deleted")
#define IDX_ARCHIVING	1
#define IDX_CAL_CLEANUP 2		   		 
#define IDX_DONE		3
#define IDX_DOCS_DEL	4
#define STR_ARC_PROGRESS_STATES				PKG_NSF9+4
	stringtext(STR_ARC_PROGRESS_STATES, 	"%s|Retrieving settings|Creating Archive DB|Creating Log DB|Updating Log Design|Selecting Documents|Copying Documents|Deleting Documents|Truncating Documents|done. %d docs archived, %d docs deleted, %d docs truncated|Processing Documents for Preview")
#define ERR_LOCATING_SCT					PKG_NSF9+5
	errortext(ERR_LOCATING_SCT,				"The single copy template associated with this database cannot be located.")
#define ERR_OPENING_SCT						PKG_NSF9+6
	errortext(ERR_OPENING_SCT,				"Unable to open the single copy template associated with this database.")
#define STR_ADDIN_NSF_DONTTRANS_MSG_1		PKG_NSF9+7
	donottranslatetext(STR_ADDIN_NSF_DONTTRANS_MSG_1,	"Administration Requests")
#define ERR_EM_INVALID_CALLBACKPTR			PKG_NSF9+8
	errortext(ERR_EM_INVALID_CALLBACKPTR,	"Extension registration contains NULL pointer to callback routine")
#define IDX_GEN_ADMIN4_TITLE		1
#define	ERR_PAINDEX_CORRUPT					PKG_NSF9+9
	errortext(ERR_PAINDEX_CORRUPT,			"Protected Attachments Index corrupt.")
#define	ERR_ARCHIVE_DATABASE_INVALID		PKG_NSF9+10
	errortext(ERR_ARCHIVE_DATABASE_INVALID, "Archive Database format is invalid")
#define	ERR_ARCHIVE_DATABASE_NO_MATCH_SRCREPID  PKG_NSF9+11
	errortext(ERR_ARCHIVE_DATABASE_NO_MATCH_SRCREPID, "Archive database specifies a different source database")
#define ERR_OPT_UNREAD_SYNC				PKG_NSF9+12
	internaltext(ERR_OPT_UNREAD_SYNC,		"Optimized unread synchronization performed.")
#define	ERR_NO_PROVISIONAL_ON_MASTERDB			  PKG_NSF9+13
	errortext(ERR_NO_PROVISIONAL_ON_MASTERDB, "Provisional locks on the master lock database are not allowed")
#define ERR_FILEEXT_FAILURE					PKG_NSF9+14
	errortext(ERR_FILEEXT_FAILURE, "Extendable File object: failure")
#define ERR_URL_TOO_LONG					PKG_NSF9+15
	errortext(ERR_URL_TOO_LONG,		"URL is too long")
#define ERR_BACKUP_ABORTED			PKG_NSF9+16
	errortext(ERR_BACKUP_ABORTED,		"Backup terminated due to timeout")
#define ERR_PURGED_NOTES_MISSING			PKG_NSF9+17
	internaltext(ERR_PURGED_NOTES_MISSING,		"Unable to determine what notes may have been purged before the search date")
#define	ERR_COMPACT_INTERRUPTED_MSG			PKG_NSF9+18
	errortext(ERR_COMPACT_INTERRUPTED_MSG,	"Compaction of the database was stopped prematurely because another user modified it while it was being compacted (%s).")
#define	ERR_INSERT_DATANOTEID_TBL			PKG_NSF9+19
	errortext(ERR_INSERT_DATANOTEID_TBL,	"Error inserting Note ID %X into data note ID table in %s: %e.")
#define ERR_UNABLE_TO_BRING_ONLINE			PKG_NSF9+20
	errortext(ERR_UNABLE_TO_BRING_ONLINE,	"Compaction failed: Unable to bring %p back online.")
#define ERR_NSFIMAP_HAS_SUBFOLDERS			PKG_NSF9+21
	errortext(ERR_NSFIMAP_HAS_SUBFOLDERS,	"One or more child mailbox(es) exists")
#define ERR_CANT_CREATE_TEMPLATE			PKG_NSF9+22
	errortext(ERR_CANT_CREATE_TEMPLATE,		"You do not have sufficient access to create master templates on this server.")
#define ERR_DBU_WRONG_PROCESS				PKG_NSF9+23
	errortext(ERR_DBU_WRONG_PROCESS,		"DBUs cannot be shared between Processes.")
#define ERR_NSF_ARCH_SELDOCS_NOT_DELETED	PKG_NSF9+24
	errortext(ERR_NSF_ARCH_SELDOCS_NOT_DELETED,	"Some of the selected documents were not deleted because they are still required in this database.")
#define LOG_CREATING_DBDIRMAN_CACHE			PKG_NSF9+25
	errortext(LOG_CREATING_DBDIRMAN_CACHE,	"Creating Database Directory Cache database (%s)...")
#define ERR_TOO_MANY_RARELY_USED_ITEMS 	PKG_NSF9+26
	errortext(ERR_TOO_MANY_RARELY_USED_ITEMS, 	"Cannot store document because it has too many unique field names.")
#define ERR_NSF_UNABLE_TO_CHANGE_DBOPTION	PKG_NSF9+27
	errortext(ERR_NSF_UNABLE_TO_CHANGE_DBOPTION,"Cannot disable consistent access without turning off extended access control first.")
#define ERR_SIGNDB_NOTE_CLASS_NOT_FOUND			PKG_NSF9+28
	errortext(ERR_SIGNDB_NOTE_CLASS_NOT_FOUND,	"Specified note does not match note class requested.")
#define ERR_DUPLICATE_TEMPLATE				PKG_NSF9+29
	errortext(ERR_DUPLICATE_TEMPLATE,		"Cannot create template with duplicate name.")
#define ERR_CACHEFAILURE					PKG_NSF9+30
	errortext(ERR_CACHEFAILURE,				"An error has occurred which will affect performance.  Please make a copy of the file %s and then delete the original which will correct the problem.  Please report the problem to IBM.")
#define ERR_DESIGN_REFRESH_NOT_ADMIN		PKG_NSF9+32
	errortext(ERR_DESIGN_REFRESH_NOT_ADMIN,	"Design Refresh can only be done on the Administration server of this database.")
#define ERR_ARCHIVING_NOT_ENABLED_ON_SVR 	PKG_NSF9+33
	errortext(ERR_ARCHIVING_NOT_ENABLED_ON_SVR,	"Server-based archiving not enabled on database.")
#define	ERR_BKTDSC_MODIFIED_BACKWARDS		PKG_NSF9+34
	errortext(ERR_BKTDSC_MODIFIED_BACKWARDS,"Bucket descriptor modified time is going backwards from %z to %z in %s.")
#define ERR_CURRENT_UNIQUE_TIME				PKG_NSF9+35
	errortext(ERR_CURRENT_UNIQUE_TIME,		"Database (%s) time is too far in the future.")
#define ERR_FILEREAD						PKG_NSF9+36
	errortext(ERR_FILEREAD,					"Hardware/OS error (%s) reading from database (%s), Length=%X (PID=%0X/TID=%0X)\n")
#define ERR_FILEWRITE						PKG_NSF9+37
	errortext(ERR_FILEWRITE,				"Hardware/OS error (%s) writing to database (%s), Length=%X (PID=%0X/TID=%0X)\n")
#define ERR_NTUPDATE_MONITOR_ERROR			PKG_NSF9+38
	errortext(ERR_NTUPDATE_MONITOR_ERROR,	"Monitor Error in Database %s:")
#define ERR_NTUPDATE_FOLDER_ERROR			PKG_NSF9+39
	errortext(ERR_NTUPDATE_FOLDER_ERROR,	"Unable to add note %x to folder in Database %s:")
#define ERR_CMD_SHOW_MONITORPOOL			PKG_NSF9+40
	stringtext(ERR_CMD_SHOW_MONITORPOOL,	 "NSFMONITORPOOL")
#define ERR_HELP_SHOW_MONITORPOOL			PKG_NSF9+41
	stringtext(ERR_HELP_SHOW_MONITORPOOL, 	"* Show the contents of the Server's MONITOR pool.")
#define ERR_XACL_NOTSUPPORTED				PKG_NSF9+42
	errortext(ERR_XACL_NOTSUPPORTED,		"The extended access control feature is not supported for the database.")
#define ERR_DESIGN_IN_NAMOBJ				PKG_NSF9+43
	errortext(ERR_DESIGN_IN_NAMOBJ,			"Design collection not in named object table.")
#define ERR_DISK_SCOS						PKG_NSF9+44
	errortext(ERR_DISK_SCOS,				"Insufficient disk space in shared mail directory.")
#define ERR_FORCE_COMPACT					PKG_NSF9+45
	internaltext(ERR_FORCE_COMPACT,			"Force compaction.")
#define ERR_SUPERBLOCK_TOO_LARGE			PKG_NSF9+46
	internaltext(ERR_SUPERBLOCK_TOO_LARGE,	"Superblock has exceeded its maximum allowable size.")
#define ERR_CONFLICT_SCOS					PKG_NSF9+47
	errortext(ERR_CONFLICT_SCOS,			"Shared mail note access conflict, retry in progress.")
#define ERR_NOTE_MUST_BE_LOCKED_FOR_DELETE	  	PKG_NSF9+48
	errortext(ERR_NOTE_MUST_BE_LOCKED_FOR_DELETE,	"Document locking is enabled. You must lock the document before deleting.")
#define ERR_ARCHIVE_DEFAULT_PRIVATE_NAME_MOD  PKG_NSF9+49
	stringtext(ERR_ARCHIVE_DEFAULT_PRIVATE_NAME_MOD, "Default for Last Modified")
#define ERR_ARCHIVE_DEFAULT_PRIVATE_NAME_LA  PKG_NSF9+50
	stringtext(ERR_ARCHIVE_DEFAULT_PRIVATE_NAME_LA, "Default for Last Accessed")
#define ERR_ARCHIVE_DEFAULT_PRIVATE_NAME_EXP  PKG_NSF9+51
	stringtext(ERR_ARCHIVE_DEFAULT_PRIVATE_NAME_EXP, "Default for Expired")
#define	ERR_OVERWARNQUOTA					PKG_NSF9+52
	errortext(ERR_OVERWARNQUOTA,			"Warning: Operation would put database over quota warning threshold.")
#define	LOG_CANT_CREATE_TEMPLATE			PKG_NSF9+53
	errortext(LOG_CANT_CREATE_TEMPLATE,		"Attempt by %s to define database %s as template %s rejected.")
#define	LOG_DUPLICATE_TEMPLATE				PKG_NSF9+54
	errortext(LOG_DUPLICATE_TEMPLATE,		"Attempt by %s to create duplicate template %s in database %s - rejected.")
#define	LOG_ALREADY_IS_A_CHILD				PKG_NSF9+55
	errortext(LOG_ALREADY_IS_A_CHILD,		"Note %d already is a child of note %d in database %s.")
#define ERR_NSF_MONITOR_EVENTSFULL			PKG_NSF9+56
	errortext(ERR_NSF_MONITOR_EVENTSFULL, 	"Insufficient memory - NSF monitor pool event space is full.")
#define ERR_DBIMAP_FOLDER_UPDATE_FAILED		PKG_NSF9+57
	errortext(ERR_DBIMAP_FOLDER_UPDATE_FAILED,	"Note NT%08lx was not updated in the IMAP btree for folder %s (NT%08lx) in database %s.  Database should be re-enabled for IMAP support.")
#define ERR_ARCHIVING_POLICY_EXISTS			PKG_NSF9+58
	errortext(ERR_ARCHIVING_POLICY_EXISTS,	"An archive policy has been set by the administrator, cannot get archive settings.")
#define STR_NSF_XACL_ENABLED					PKG_NSF9+59
	errortext(STR_NSF_XACL_ENABLED, "Extended access control feature is enabled for the database %s")
#define STR_NSF_XACL_DISABLED					PKG_NSF9+60
	errortext(STR_NSF_XACL_DISABLED, "Extended access control feature is disabled for the database %s")
#define ERR_NONSUMMARY_READAUTH			PKG_NSF9+61
	errortext(ERR_NONSUMMARY_READAUTH,			"Update not allowed due to nonsummary author/reader names field.  Saving may restrict access to this document.")

#define ERR_BAD_IDTABLE_LENGTH				PKG_NSF9+62
	errortext(ERR_BAD_IDTABLE_LENGTH,		"Bad idtable length detected in a super block.")
#define ERR_DB_DELETE_IN_PROGRESS			PKG_NSF9+63
	errortext(ERR_DB_DELETE_IN_PROGRESS, 	"Database is being deleted.")

#define ERR_ARCHIVE_LOG_TEMPLATE_NOT_FOUND	PKG_NSF9+64
	errortext(ERR_ARCHIVE_LOG_TEMPLATE_NOT_FOUND,	"Archive log template (archlg50.ntf) not found, unable to create archive log database.")
#define ERR_ARCHIVE_LOG_CREATE				PKG_NSF9+65
	errortext(ERR_ARCHIVE_LOG_CREATE,		"Error creating archive log database")
#define ERR_RM_LOGGER_RC					PKG_NSF9+66
	errortext(ERR_RM_LOGGER_RC, "Logger Failure: Func=%s RC=0x%08x File=%s\n")
#define ERR_RM_UNDO_REDO_RC					PKG_NSF9+67
	errortext(ERR_RM_UNDO_REDO_RC, "Error from undo/redo LSN=%02x%02x%02x%02x-%02x%02x%02x%02x rc: %d %e Type=%02x/%02x DB=%s\n")
#define ERR_INVALID_CONTAINER_OBJECT		PKG_NSF9+68
	errortext(ERR_INVALID_CONTAINER_OBJECT,	"Container object is corrupt.  Use compact -d to discard view indexes.")
#define ERR_CMD_SHOW_DBG					PKG_NSF9+69
	stringtext(ERR_CMD_SHOW_DBG,"DBG")
#define	ERR_HELP_SHOW_DBG	 				PKG_NSF9+70
	stringtext(ERR_HELP_SHOW_DBG,"Database group information")
#define ERR_LK_PER_TXN_LOCKS				PKG_NSF9+71
	errortext(ERR_LK_PER_TXN_LOCKS, "Maximum locks per explicit txn exceeded")
#define ERR_LK_MAX_TXN_LOCKS				PKG_NSF9+72
	errortext(ERR_LK_MAX_TXN_LOCKS, "Maximum locks for explicit txns exceeded")
#define	ERR_RESOLVING_REF_NOTE				PKG_NSF9+73
	errortext(ERR_RESOLVING_REF_NOTE,	"Error opening note unable to resolve the referenced note")
#define ERR_CANNOT_SELECT_PROFILE_DOCS		PKG_NSF9+74
	stringtext(ERR_CANNOT_SELECT_PROFILE_DOCS,	"Using a selection formula to filter profile documents is not supported by version 5 (or older) Domino servers.")
#define ERR_NULL_PNOVEC_VARRAY				PKG_NSF9+75
	stringtext(ERR_NULL_PNOVEC_VARRAY,		"Page vector array is NULL")
#define ERR_CONT_REOPENED					PKG_NSF9+76
	stringtext(ERR_CONT_REOPENED,			"Attempt to reopen an open container")
#define ERR_UNREAD_LOG_CORRUPT				PKG_NSF9+77
	errortext(ERR_UNREAD_LOG_CORRUPT, "Unread Log is corrupt")
#define ERR_FIXUP_FOUND_UNREADLOG			PKG_NSF9+78
	errortext(ERR_FIXUP_FOUND_UNREADLOG,	"Unread activity log for user %a in database %p is damaged: %e")
#define ERR_AUTHOR_UPDATED_ONLY_PROTECTED	PKG_NSF9+79
	errortext(ERR_AUTHOR_UPDATED_ONLY_PROTECTED, "NoteUpdate:  ID[%s] with Author access tried to update *ONLY* protected fields, nothing updated; db=%s, NoteID=0x%lx\n")
#define ERR_ARCHIVE_DEFAULT_PRIVATE_NAME_SELECTED  PKG_NSF9+80
	stringtext(ERR_ARCHIVE_DEFAULT_PRIVATE_NAME_SELECTED, "Default for Selected (no aging)")
#define LOG_REALLOC_UNREAD_LOG				PKG_NSF9+81
	errortext(LOG_REALLOC_UNREAD_LOG, 		"Unread log for user %a in database %p is being reallocated. ReasonCode:%d")
#define LOG_REALLOC_UNREAD_ALLUSER			PKG_NSF9+82
	errortext(LOG_REALLOC_UNREAD_ALLUSER, 	"Unread all-user for database %p is being reallocated. ReasonCode:%d")
#define LOG_UNREAD_CHUNK_BAD				PKG_NSF9+83
	errortext(LOG_UNREAD_CHUNK_BAD, 		"Unread chunk %d for user %a in database %p is bad. Object: %x. ReasonCode:%d")
#define LOG_UNREAD_REPLICATION_OPTION_CHG	PKG_NSF9+84
	errortext(LOG_UNREAD_REPLICATION_OPTION_CHG, "%s Database Option: %s for database %s")
#define LOG_ALLUSER_BAD_LOG					PKG_NSF9+85
	errortext(LOG_ALLUSER_BAD_LOG, 			"Invalid Object in Unread All-User table for database %p. Removing reference for index %d. %e")
#define LOG_DDM_LOOK_AT_UPDATE					PKG_NSF9+86
	errortext(LOG_DDM_LOOK_AT_UPDATE, 			"NSFNoteUpdate may be working on an interesting note ... %s, %a")
#define ERR_RM_XA_ERROR						PKG_NSF9+87
	errortext(ERR_RM_XA_ERROR, "An error occurred accessing the db2 datasource, xa error = %d")
#define ERR_DB2_ADMIN_CATALOG_NODE				PKG_NSF9+89
	errortext(ERR_DB2_ADMIN_CATALOG_NODE, 		"Error attempting to catalog node.")
#define ERR_DB2_ADMIN_CATALOG_DBALIAS			PKG_NSF9+90
	errortext(ERR_DB2_ADMIN_CATALOG_DBALIAS, 	"Error attempting to catalog db alias.")
#define ERR_BKTDESC_CORRUPT						PKG_NSF9+91
	errortext(ERR_BKTDESC_CORRUPT,			 	"Bucket descriptor corrupt.")
#define ERR_DB2_ADMIN_LIB_NOT_AVAILABLE			PKG_NSF9+92
	errortext(ERR_DB2_ADMIN_LIB_NOT_AVAILABLE, 	"The DB2 library is not available.")
#define ERR_NSF_ALREADY_DB2NSF					PKG_NSF9+93
	errortext(ERR_NSF_ALREADY_DB2NSF, 			"The database is already stored in DB2.")
/* Available PKG_NSF9+94 - 99 */
#define NSF_DB2_CLP_SQL_NO_DATA					PKG_NSF9+100
	errortext(NSF_DB2_CLP_SQL_NO_DATA, 			"Data not found.")
#define ERR_FIXING_WAIT							PKG_NSF9+101
	errortext(ERR_FIXING_WAIT, 					"Performing consistency check. Please wait...")
#define ERR_DB2_ADMIN_CREATE_DB2_GROUP			PKG_NSF9+102
	errortext(ERR_DB2_ADMIN_CREATE_DB2_GROUP,	"Error attempting to add group to SYSCTRL_GROUP.")

#define LOG_UNREAD_ROLLBACK					PKG_NSF9+103
	errortext(LOG_UNREAD_ROLLBACK, 			"Rolling back Unread Log for user %a in database %p.")
#define STR_RM_INDOUBTPROC_THREAD			PKG_NSF9+104
	stringtext(STR_RM_INDOUBTPROC_THREAD,	"Recovery Manager Indoubt resolution Thread")

#define NSF_DB2_LOCAL_ENCRYPTION_UNAVAILABLE	PKG_NSF9+105
    errortext(NSF_DB2_LOCAL_ENCRYPTION_UNAVAILABLE,	"Local encryption is not supported for a DB2 backed Notes database.  %s will not be locally encrypted by Domino in DB2.")
#define ERR_DB2NSF_FEDERATION					PKG_NSF9+106
			errortext(ERR_DB2NSF_FEDERATION,        	"Unable to set up federation to alternate recovery database")
			
#define ERR_MONITOR_TOO_MUCH_SUMMARY			PKG_NSF9+108
	errortext(ERR_MONITOR_TOO_MUCH_SUMMARY,		"Too much summary data returned for monitor in %p")

#define NSF_DB2BACKUP_SERVERRUNNING             PKG_NSF9+109
	errortext(NSF_DB2BACKUP_SERVERRUNNING,      "This API function cannot be called if Domino is running")
#define CONTEXT_DB2UPDATE_FILE_HEADER			PKG_NSF9+110
	errortext(CONTEXT_DB2UPDATE_FILE_HEADER,   "Failed to update system Catalog table")
#define CONTEXT_DB2READ_CATALOG  				PKG_NSF9+111
	errortext(CONTEXT_DB2READ_CATALOG,			"Failed to read system Catalog table")
#define CONTEXT_DB2READ_SCHEMATABLES  				PKG_NSF9+112
	errortext(CONTEXT_DB2READ_SCHEMATABLES,		"Failed to query DB2 system catalog for tables belonging to schema")
#define ERR_DB2NSF_NOTDB2BACKED					PKG_NSF9+113
	errortext(ERR_DB2NSF_NOTDB2BACKED,          "Invalid NSFDB2 database.")
#define ERR_DB2NSF_SRC_AND_DESTINATION			PKG_NSF9+114
	errortext(ERR_DB2NSF_SRC_AND_DESTINATION,  "The source and destination file specification cannot be the same")
#define CONTEXT_DB2NSF_SQL_COPY_FAILURE             PKG_NSF9+115
	errortext(CONTEXT_DB2NSF_SQL_COPY_FAILURE,     "Unable to perform a fast copy/move to target")
#define ERR_DB2NSF_TABLESPACE_INFO              PKG_NSF9+116
	errortext(ERR_DB2NSF_TABLESPACE_INFO,      "Failed to obtain tablespace information.")
#define ERR_DB2NSF_ODS_NOT_SUPPORTED		PKG_NSF9+117
	errortext(ERR_DB2NSF_ODS_NOT_SUPPORTED,	    "NSFDB2 Databases require Notes/Domino 6.0 or later database versions.")

/* Available PKG_NSF9+118 - 123 */

#define ERR_DB2_ADMIN_EN_ACCOUNT_YES_NO			PKG_NSF9+124
	errortext(ERR_DB2_ADMIN_EN_ACCOUNT_YES_NO,	"Server could not update user accounts. If you have already set up the user accounts, press 'Continue' to continue. Otherwise press 'Stop' to stop.\n\nContinue?\n")

#define ERR_DB2_UDF_ADMIN_ERROR_RET				PKG_NSF9+125
	errortext(ERR_DB2_UDF_ADMIN_ERROR_RET,		"DB2 Access admin returned error.")
#define ERR_DB2_UDF_ADMIN_DLL_RET				PKG_NSF9+126
	stringtext(ERR_DB2_UDF_ADMIN_DLL_RET,		"%d UDF Admin DLL: %s.|Update connection completed|Update connection returned error|Read connection returned error")
#define IDX_DB2_UDF_DLL_PREF		1
#define IDX_DB2_UDF_DLL_UPDT_OK		2
#define IDX_DB2_UDF_DLL_UPDT_ERR	3
#define IDX_DB2_UDF_DLL_READ_ERR	4

#define ERR_DB2_ADMIN_EN_USER_ERROR				PKG_NSF9+127
	stringtext(ERR_DB2_ADMIN_EN_USER_ERROR,		"-  DB2 Server Enable could not set up user account.\n")

#define ERR_DB2_ADMIN_EN_GROUP_EXISTS			PKG_NSF9+128
	stringtext(ERR_DB2_ADMIN_EN_GROUP_EXISTS,	"-  The SYSCTRL_GROUP, '%s' already exists.\n")

#define ERR_DB2_ADMIN_EN_GROUP_GROUP			PKG_NSF9+129
	stringtext(ERR_DB2_ADMIN_EN_GROUP_GROUP,		"-  The user must be added to the SYSCTRL_GROUP '%s'.\n")

#define DB2_ENABLE_TOOL_RAN				PKG_NSF9+130
	stringtext(DB2_ENABLE_TOOL_RAN,		"This server is already running on DB2. Please make any changes to the DB2 configuration in the server document.")

#define LOG_IGNORE_OLD_UNREAD_REPL_OPS		PKG_NSF9+131
	errortext(LOG_IGNORE_OLD_UNREAD_REPL_OPS, "Ignoring Pre-Cutoff Unread Replication Activities for user %a in database %p.")
#define LOG_IGNORE_FUTURE_UNREAD_REPL_OPS	PKG_NSF9+132
	errortext(LOG_IGNORE_FUTURE_UNREAD_REPL_OPS, "Ignoring Unread Replication Activities dated in the future for user %a in database %p.")
			
#define ERR_DB2NSF_IDENTIFIER_INVALID			PKG_NSF9+133
    errortext(ERR_DB2NSF_IDENTIFIER_INVALID,    "The identifier specified does not exist in DB2 or does not contain the requested NSFDB2 data.")
			
#define ERR_DB2_ADMIN_EN_ACCOUNT_RIGHTS_ERR2		PKG_NSF9+134
	stringtext(ERR_DB2_ADMIN_EN_ACCOUNT_RIGHTS_ERR2,	"-  You must create the user account.\n-  You also need to create the group 'DB2DOM', if it doesn't exist, \n-  and add the DB2 user to it.\n")

#define ERR_DB2NSF_SCHEMA_INUSE				PKG_NSF9+135
	errortext(ERR_DB2NSF_SCHEMA_INUSE,      "The schema specified is in use by another NSFDB2 database.")
#define ERR_DB2NSF_CATALOG_UPDATE_FAILURE   PKG_NSF9+136
	errortext(ERR_DB2NSF_CATALOG_UPDATE_FAILURE, "Failed to update row in system CATALOG for database %s in schema %s.")

#define ERR_DB2NSF_NONXA_CONNECT            PKG_NSF9+137
	errortext(ERR_DB2NSF_NONXA_CONNECT,     "Failed to create a non-XA connection to DB2 database")
#define ERR_DB2NSF_CATALOG_REMOVE_ROW           PKG_NSF9+138
	errortext(ERR_DB2NSF_CATALOG_REMOVE_ROW,    "Row in system CATALOG table for database %s in schema %s is invalid and was removed")
#define CONTEXT_DB2DAVPOP						PKG_NSF9+139
	errortext(CONTEXT_DB2DAVPOP,				"Error encountered in the DB2 Access View Populator.")
#define CONTEXT_DB2NSF_UAV_CLEAR                PKG_NSF9+140
    errortext(CONTEXT_DB2NSF_UAV_CLEAR,         "Error clearing UAVROOT from related NSFDB2 databases.")
#define LOG_UNREAD_WRITE_ERROR					PKG_NSF9+141
	errortext(LOG_UNREAD_WRITE_ERROR,			"Error writing unread log for database %p. User %a. Error: %e.")
#define CONTEXT_DB2NSF_UAV_EXEMPT               PKG_NSF9+142
	errortext(CONTEXT_DB2NSF_UAV_EXEMPT,		"Error flagging NSFDB2 database as exempt from the UAV.")
#define ERR_DB2NSF_ACCESS_DELETE				PKG_NSF9+143
	errortext(ERR_DB2NSF_ACCESS_DELETE,			"The exported NSFDB2 database access table entities could not be dropped.")
#define ERR_INT_CONSTRAINT						PKG_NSF9+144
	errortext(ERR_INT_CONSTRAINT,				"Error saving non integral value to a DB2 Access View INTEGER column.")
#define ERR_BAD_DAV								PKG_NSF9+145
	errortext(ERR_BAD_DAV,						"Invalid Access View definition.")
#define ERR_NO_DAV_FORMS						PKG_NSF9+146
	errortext(ERR_NO_DAV_FORMS,					"Access View definition must be associated with one or more forms on the infobox for the DB2 Access View.")
#define CONTEXT_UPDATE_NSFID					PKG_NSF9+147
	errortext(CONTEXT_UPDATE_NSFID,				"Error encountered modifying NSFID value for table after a conflict was detected updating system CATALOG table")		
#define ERR_NSF_DBCREATE_DDM				PKG_NSF9+148		
	errortext(ERR_NSF_DBCREATE_DDM,			"Database creation error: %s")
#define ERR_NSF_DBDELETE_DDM				PKG_NSF9+149
	errortext(ERR_NSF_DBDELETE_DDM,			"Database deletion error: %s")
#define ERR_NSF_DBOPEN_DDM					PKG_NSF9+150
	errortext(ERR_NSF_DBOPEN_DDM,			"Database open error: %s")
#define ERR_NSF_NOTEOPEN_DDM				PKG_NSF9+151
	errortext(ERR_NSF_NOTEOPEN_DDM,			"Database note open error: %s")
#define ERR_NSF_NOTEUPDATE_DDM				PKG_NSF9+152
	errortext(ERR_NSF_NOTEUPDATE_DDM,		"Database note update error: %s")
#define ERR_NSF_DBFILEREAD_DDM				PKG_NSF9+153
	errortext(ERR_NSF_DBFILEREAD_DDM,		"Database file read error: %s")
#define ERR_NSF_DBFILEWRITE_DDM				PKG_NSF9+154
	errortext(ERR_NSF_DBFILEWRITE_DDM,		"Database file write error: %s")
#define ERR_NSF_DBCORRUPT_DDM				PKG_NSF9+155
	errortext(ERR_NSF_DBCORRUPT_DDM,		"Database marked corrupt: %s")
#define ERR_NSF_DBOBJREAD_DDM				PKG_NSF9+156
	errortext(ERR_NSF_DBOBJREAD_DDM,		"Database object read error: %s")
#define ERR_NSF_DBOBJWRITE_DDM				PKG_NSF9+157
	errortext(ERR_NSF_DBOBJWRITE_DDM,		"Database object write error: %s")
#define ERR_DB2_ADMIN_EN_SRV_RESTART		PKG_NSF9+158
	errortext(ERR_DB2_ADMIN_EN_SRV_RESTART,	"The Domino server is being restarted.\n")
#define ERR_DB2_ADMIN_EN_USER_SYSCTRL		PKG_NSF9+159
	stringtext(ERR_DB2_ADMIN_EN_USER_SYSCTRL,	"-  Could not add user, '%s' to DB2 SYSCTRL group, '%s'.\n You must manually add user to DB2 SYSCTRL group\nor add DB2DOM group to DB2 SYSCTRL group.\n")
#define ERR_DB2_USER_MAPPING				PKG_NSF9+160
	errortext(ERR_DB2_USER_MAPPING,			"Missing Domino to DB2 user mapping.")
#define CONTEXT_DB2NSF_CLEAN_CONN			PKG_NSF9+161
	errortext(CONTEXT_DB2NSF_CLEAN_CONN,	"Error encountered while cleaning up DB2 connection." )
#define ERR_NOT_A_DB2NSF	                PKG_NSF9+162
	errortext(ERR_NOT_A_DB2NSF,				"This action only available with DB2NSF databases.")
#define ERR_DAV_COL_NAME_LEN                PKG_NSF9+163
	errortext(ERR_DAV_COL_NAME_LEN,			"Field name length exceeds maximum column name length (30 bytes), Fieldname: ")
#define ERR_DAV_NOGEN						PKG_NSF9+164
	errortext(ERR_DAV_NOGEN,				"Current DB2 table is compatible with the Access View definition.  No changes needed to be made in DB2.")
#define ERR_DAV_DUPCOL						PKG_NSF9+165
	errortext(ERR_DAV_DUPCOL,				"Access View definition contains duplicate field names.")
#define ERR_DAV_NEEDSPOP					PKG_NSF9+166
	errortext(ERR_DAV_NEEDSPOP,				"Access Definition created successfully. You must populate the view before the view will be available in DB2.")
#define ERR_DAV_LENGTH_REQD					PKG_NSF9+167
	errortext(ERR_DAV_LENGTH_REQD,			"Text fields cannot have DB2 length of 0." )
#define ERR_DAV_NODROP						PKG_NSF9+168
	errortext(ERR_DAV_NODROP,				"This Access View definition is not active in DB2.  No changes needed to be made in DB2.")
#define CONTEXT_DB2ADMIN_CLP				PKG_NSF9+169
	errortext(CONTEXT_DB2ADMIN_CLP,   		"Failure processing admin SQL command line request")
#define ERR_DB2_QV_NO_RESULT_SET			PKG_NSF9+170
	errortext(ERR_DB2_QV_NO_RESULT_SET,		"The given query cannot generate a result set and therefore has not been executed.")
#define ERR_DB2ADMIN_NO_UDF_INFO		 	PKG_NSF9+171
	errortext(ERR_DB2ADMIN_NO_UDF_INFO,  	"'DB2 ACCESS SET' failed. Please make sure that the DB2 Access information in the DB2 tab of the Server Document is up to date.")
#define ERR_DB2_EN_DUP_DB		 			PKG_NSF9+172
	errortext(ERR_DB2_EN_DUP_DB,  			"The database, %s, exists in the DB2 context you are using. You must select another DB2 database name.")
#define CONTEXT_DAV_ACTION					PKG_NSF9+173
	errortext(CONTEXT_DAV_ACTION,			"DAV Action SQL error." )
#define ERR_DAV_TOOBIG						PKG_NSF9+174
	errortext(ERR_DAV_TOOBIG,				"DB2 Access View definition exceeds maximum supported length." )
#define ERR_SERVER_NOT_UDFENABLED			PKG_NSF9+175
	errortext(ERR_SERVER_NOT_UDFENABLED,	"Domino server is not DB2 Access View enabled." )
#define ERR_NO_DAV_ANON_ACCESS				PKG_NSF9+176
	errortext(ERR_NO_DAV_ANON_ACCESS,		"No matching Notes user found for DB2 user and anonymous access is disabled." )
#define ERR_DB2_BACKLEVEL				    PKG_NSF9+177
	errortext(ERR_DB2_BACKLEVEL,		"WARNING: Domino minimum DB2 installation requirement not met.  Upgrade your DB2 installation.")
#define LOG_DB2NSF_POLL_TASK_PURGE			PKG_NSF9+178
	stringtext(LOG_DB2NSF_POLL_TASK_PURGE,  "Domino is now purging the DB2 schema %s and table space %s")
			
#define STR_UDF_SETUP_START					PKG_NSF9+179
	errortext(STR_UDF_SETUP_START,			"Please wait while the server sets up the information for the DB2 Access DLL (%a)." )
#define STR_UDF_SET_USER_DOC				PKG_NSF9+180
	errortext(STR_UDF_SET_USER_DOC,			"Create user document for the DB2 Access DLL (%a)." )
#define STR_UDF_SETUP_COMPLETE				PKG_NSF9+181
	errortext(STR_UDF_SETUP_COMPLETE,		"DB2 Access DLL setup complete.\n" )
#define ERR_DB2ADMIN_UDF_PROP		 		PKG_NSF9+182
	errortext(ERR_DB2ADMIN_UDF_PROP,  		"'DB2 ACCESS SET' failed while attempting to set the following property: \n%s")
#define ERR_DB2ADMIN_UDF_DB2_NOT		 	PKG_NSF9+183
	errortext(ERR_DB2ADMIN_UDF_DB2_NOT,  	"This server does not have DB2 Access set up.")
#define ERR_DB2ADMIN_UDF_INFO		 		PKG_NSF9+184
	errortext(ERR_DB2ADMIN_UDF_INFO,  		"'DB2 ACCESS SET' failed while attempting to execute the following command: \n%s")
#define ERR_DB2ADMIN_UDF_NO_PATH		 	PKG_NSF9+185
	errortext(ERR_DB2ADMIN_UDF_NO_PATH,  	"'DB2 ACCESS SET' failed. 'DB2UDFPATH' could not be found in notes.ini")
#define ERR_DB2ADMIN_UDF_NO_SERVER		 	PKG_NSF9+186
	errortext(ERR_DB2ADMIN_UDF_NO_SERVER,  	"'DB2 ACCESS SET' failed. 'DB2UDFSERVER' could not be found in notes.ini")

#define ERR_RRM_NOTSET						PKG_NSF9+187
	errortext(ERR_RRM_NOTSET,				"RRM Context not set for this database")
#define ERR_RRM_ALREADYSET 					PKG_NSF9+188
	errortext(ERR_RRM_ALREADYSET,			"RRM Context already set for this database")
#define NSFSTR_DB2CREATE_MESSAGE		    PKG_NSF9+189
	stringtext(NSFSTR_DB2CREATE_MESSAGE,    "Creating DB2 database '%s' in DB2 Instance '%s' with username '%s'...")
#define NSFSTR_DB2CONNECT_MESSAGE		    PKG_NSF9+190
	stringtext(NSFSTR_DB2CONNECT_MESSAGE,   "Connecting to DB2 database '%s' in DB2 Instance '%s' with username '%s'...")
#define ERR_SINGLEVAL_CONSTRAINT			PKG_NSF9+191
	errortext(ERR_SINGLEVAL_CONSTRAINT,		"Cannot save a multi-value item to a DB2 Access View defined as 'single value only'.")
#define CONTEXT_DB2NSF_USER_ACTIVITY_LOG	PKG_NSF9+192
	errortext(CONTEXT_DB2NSF_USER_ACTIVITY_LOG,	"Error encountered in the DB2 User Activity Log.")
#define CONTEXT_DB2NSF_DELETE_PROP			PKG_NSF9+193
	errortext(CONTEXT_DB2NSF_DELETE_PROP,	"Failed when DB2 was deleting properties.")
#define ERR_DAV_BAD_COLNAME					PKG_NSF9+194	
	errortext(ERR_DAV_BAD_COLNAME,			"Access View definition has an invalid name.  The DB2 Access View name and the DB2 column names must begin with a letter.")
#define ERR_DAV_NOFIELDS					PKG_NSF9+195
	errortext(ERR_DAV_NOFIELDS,				"Access View Definition does not contain any fields.")
#define NSFSTR_DB2_NO_USERNAME_PASSWORD		PKG_NSF9+196
	stringtext(NSFSTR_DB2_NO_USERNAME_PASSWORD,"WARNING: Could not retrieve DB2 username and password from the server ID file.")
#define ERR_NO_POP_REQUEST					PKG_NSF9+197
	errortext(ERR_NO_POP_REQUEST,			"DB2 Access View population is not currently required." )
#define MSG_DAVPOP_SUCCESS					PKG_NSF9+198
	stringtext(MSG_DAVPOP_SUCCESS,			"DB2 Access View population started successfully." )
#define LOG_DAVPOP_INFO						PKG_NSF9+199
	stringtext(LOG_DAVPOP_INFO,"Failed populating Access Table - NoteID: %lu, Field Name: %s." )

#define ERR_DB2_ADMIN_EN_ACCOUNT_RIGHTS_ERR3 PKG_NSF9+200
	errortext(ERR_DB2_ADMIN_EN_ACCOUNT_RIGHTS_ERR3,	"-  You must create the user account to access DB2.\n You may have already done this. If so ignore this and continue.\n")

#define ERR_DB2_ADMIN_EN_INVALID_INSTANCE   PKG_NSF9+201
	errortext(ERR_DB2_ADMIN_EN_INVALID_INSTANCE, "-  DB2 enablement could not attach to the specified instance. You may have entered an invalid instance name or are setting up for remote and not entering the host information.\n")

#define ERR_DB2ADMIN_UDF_NO_ACCESS          PKG_NSF9+202
	errortext(ERR_DB2ADMIN_UDF_NO_ACCESS,   "'DB2 ACCESS SET' failed. DB2 Access could not access server %a. Server returned (%e).")
#define ERR_DB2_NULL_LOAD_COMPUTED_COLUMN   PKG_NSF9+203
	errortext(ERR_DB2_NULL_LOAD_COMPUTED_COLUMN, "NULL CLI Load performed on table with computed column")
#define CONTEXT_DB2_NULLLOAD                PKG_NSF9+204
	errortext(CONTEXT_DB2_NULLLOAD,         "NULL CLI Load having problems bulk erasing table data")
#define CONTEXT_DB2_RECLAIM_SPACE           PKG_NSF9+205
	errortext(CONTEXT_DB2_RECLAIM_SPACE,    "Problems reclaiming space in a tablespace container")
#define NSFSTR_DB2ATTACH_MESSAGE		    PKG_NSF9+206
	stringtext(NSFSTR_DB2ATTACH_MESSAGE,    "Attaching to DB2 Instance '%s' with username '%s'...")
#define ERR_DAV_RECREATE_NEEDED				PKG_NSF9+207
	errortext(ERR_DAV_RECREATE_NEEDED,      "DB2 Access View needs to be recreated before population." )
			
#define STR_UDF_SETUP_TEST_OK				PKG_NSF9+208
	errortext(STR_UDF_SETUP_TEST_OK,		"DB2 Access successfully connected to %a." )

#define STR_UDF_SETUP_TEST_START			PKG_NSF9+209
	errortext(STR_UDF_SETUP_TEST_START,		"DB2 Access testing connection and access to %a." )

#define STR_UDF_SETUP_TEST_CONN_FAIL		PKG_NSF9+210
	errortext(STR_UDF_SETUP_TEST_CONN_FAIL,	"DB2 Access connection failed with error -> (%e)" )

#define STR_UDF_SETUP_CONN_NO_CONFIG		PKG_NSF9+211
	errortext(STR_UDF_SETUP_CONN_NO_CONFIG,	"DB2 Access is not configured on this server." )

#define STR_UDF_SETUP_ITEM_LIST				PKG_NSF9+212
	stringtext(STR_UDF_SETUP_ITEM_LIST,		"server document|NOTES.INI" )

#define STR_UDF_SETUP_MISSING_ITEM			PKG_NSF9+213
	errortext(STR_UDF_SETUP_MISSING_ITEM,	"DB2 Access is missing item '%s' from %s." )

#define STR_UDF_SETUP_FOUND_ITEM			PKG_NSF9+214
	errortext(STR_UDF_SETUP_FOUND_ITEM,		"DB2 Access successfully found item '%s'('%s') in %s." )

#define ERR_UDF_SETUP_CONFIG_ERROR			PKG_NSF9+215
	errortext(ERR_UDF_SETUP_CONFIG_ERROR,	"DB2 Access configuration errors. Please check the notes.ini and server document." )

#define DB2_ENABLE_TOOL_SRV_DB2_AND_UDF			PKG_NSF9+216
	errortext(DB2_ENABLE_TOOL_SRV_DB2_AND_UDF,	"DB2 and DB2 Access are set up." )

#define STR_UDF_SETUP_MISSING_PER_DOC			PKG_NSF9+217
	errortext(STR_UDF_SETUP_MISSING_PER_DOC,	"DB2 Access is missing person document." )
	
#define ERR_DAV_INVALID_READER_LIST			PKG_NSF9+218
	errortext(ERR_DAV_INVALID_READER_LIST,  "The reader list is corrupt.")
#define DB2_ENABLE_TOOL_SRV_DB2_NOT_UDF			PKG_NSF9+219
	errortext(DB2_ENABLE_TOOL_SRV_DB2_NOT_UDF,	"This server has DB2 but DB2 Access not set up.")
#define ERR_DAV_MAXFIELDS			 PKG_NSF9+220
	errortext(ERR_DAV_MAXFIELDS,             "The maximum number of DB2 Access fields has been exceeded.")
/* 221 IS FREE! */

#define CONTEXT_TABLE_QUERY                 PKG_NSF9+222
	errortext(CONTEXT_TABLE_QUERY,           "Failed to query tables for schema")
#define ERR_DAV_INVALID_DELIMITER			PKG_NSF9+223
    errortext(ERR_DAV_INVALID_DELIMITER,    "Invalid multivalue delimiter.")
#define STR_UDF_DROPFAILURE			        PKG_NSF9+224
	stringtext(STR_UDF_DROPFAILURE,          "Unable to remove some DB2 Access entities for schema %s.  These will need to be manually dropped. ")
#define STR_UDF_TYPEKEY				        PKG_NSF9+225
	stringtext(STR_UDF_TYPEKEY,				"Types are: 1=Table, 2=View, 3=Trigger, 4=Function, 5=Index\nThe items to be dropped can be obtained with the following query:")			
#define ERR_MONITORS_CLIENT_EVENTS			PKG_NSF9+226
	errortext(ERR_MONITORS_CLIENT_EVENTS,	"Monitor client has exceeded maximum number of events")
#define ERR_ARCHIVE_PRUNE_FLAG_NOT_SET		PKG_NSF9+231
	errortext(ERR_ARCHIVE_PRUNE_FLAG_NOT_SET,		"Archive doc link detected in a document that does not have the pruned or abstracted flag set.  NoteID=%x.\n")
#define ERR_DESK_ARCHIVE_PRN_DOCS		(PKG_NSF9+232)
	errortext(ERR_DESK_ARCHIVE_PRN_DOCS,	"Some documents were not archived because they had already been pruned. See the archive log for additional details.")
#define ERR_DESK_ARCHIVE_NUM_PRN_DOCS		(PKG_NSF9+233)
	errortext(ERR_DESK_ARCHIVE_NUM_PRN_DOCS, "%d documents were not archived because they had already been pruned.")
#define CONTEXT_DB2LOAD_RESTORE_DEF			PKG_NSF9+234
	errortext(CONTEXT_DB2LOAD_RESTORE_DEF,	"Failed when restoring access definition.")	
#define ERR_TASK_DB2DEFERRED_QCREATEFAIL	PKG_NSF9+235
	stringtext(ERR_TASK_DB2DEFERRED_QCREATEFAIL, "Failed to create the DB2 deferred processing message queue")
#define ERR_TASK_DB2DEFERRED_DB2ERR			PKG_NSF9+236
	errortext(ERR_TASK_DB2DEFERRED_DB2ERR,  "DB2 deferred processing thread delayed due to DB2 errors")
#define STR_UDF_REMOVE_WARNING			PKG_NSF9+237
	errortext(STR_UDF_REMOVE_WARNING,		"Warning: %d Domino Access Views could not be removed.\n" )

#define STR_UDF_REMOVE_COMPLETE			PKG_NSF9+238
	errortext(STR_UDF_REMOVE_COMPLETE,		"DB2 Access successfully removed from server.\n" )

#define ERR_DB2NSF_SCHEMAUPGRADEBLOCKED			PKG_NSF9+239
	errortext(ERR_DB2NSF_SCHEMAUPGRADEBLOCKED,	"Upgrade to schema version %d disabled, set DEBUG_DB2_ALLOW_SCHEMA_UPGRADE=%d in notes.ini to enable." )

#define ERR_ARCHIVE_BEHALFOF_NOT_SET             PKG_NSF9+240
	errortext(ERR_ARCHIVE_BEHALFOF_NOT_SET,      "Error opening archive settings because item $OnBehalfOf exists but is not set")
#define ERR_ARC_REQ_SRC_ACCESS_BEHALF			PKG_NSF9+241
	errortext(ERR_ARC_REQ_SRC_ACCESS_BEHALF,	"OnBehalfOf user does not have the required access rights to the source database.")
#define ERR_ARC_REQ_ARC_ACCESS_BEHALF			PKG_NSF9+242
	errortext(ERR_ARC_REQ_ARC_ACCESS_BEHALF,	"OnBehalfOf user does not have the required access rights to the archive database.")
#define NSFSTR_DB2DROP_MESSAGE				PKG_NSF9+243
	stringtext(NSFSTR_DB2DROP_MESSAGE,		"Dropping DB2 database '%s' in DB2 Instance '%s' with username '%s'...")			
#define ERR_DB2_INVALID_LIB_PATH			PKG_NSF9+244
	errortext(ERR_DB2_INVALID_LIB_PATH,		"\nThe DB2 library path may be invalid. \nPlease make sure you have entered the correct path to the DB2 libraries." )

#define DB2_ENABLE_TOOL_INVALID_OS			PKG_NSF9+245
	errortext(DB2_ENABLE_TOOL_INVALID_OS,		"This server is not on a supported platform. Only servers on Windows or AIX platforms support update to DB2.")
#define ERR_RM_XA_ERROR_NOCODE				PKG_NSF9+246
	errortext(ERR_RM_XA_ERROR_NOCODE,		"An error occurred accessing the db2 datasource.")			
#define ERR_DB2_LOG_FULL				PKG_NSF9+247
	errortext(ERR_DB2_LOG_FULL,		"DB2 log full.  Make sure your DB2 'LOGFILSIZ', 'LOGPRIMARY' and 'LOGSECOND' are large enough.")
#define ERR_TASK_DESC_SCR					PKG_NSF9+248
	errortext(ERR_TASK_DESC_SCR,			"Cluster Replication Cleanup Thread")			
#define ERR_DAV_BAD_UNK				PKG_NSF9+249
	errortext(ERR_DAV_BAD_UNK,		"Access View Definition references an unknown form : ")
#define ERR_IMAP_CONV_UNREAD_ERR			PKG_NSF9+250
	errortext(ERR_IMAP_CONV_UNREAD_ERR,		"Cannot migrate R5 IMAP Seen flags to Unread marks.")

#define ERR_DB2_ACCESS_CONN_NET_ADDR			PKG_NSF9+251
	errortext(ERR_DB2_ACCESS_CONN_NET_ADDR,		"The optional network address (%s) for the connection record may be invalid. Please check using the 'Edit DB2 Access Connection' tool in the Domino Administration clients.")

#define ERR_ARC_PREVIEW_DOCS				PKG_NSF9+252
	errortext(ERR_ARC_PREVIEW_DOCS,			"Unable to process documents for archive preview")

#define CONTEXT_GROUP_MGMT							PKG_NSF9+253
	errortext(CONTEXT_GROUP_MGMT,	"Error managing DB2 groups")

#define ERR_DB2_GROUP_ENTRIES				PKG_NSF9+254
	errortext(ERR_DB2_GROUP_ENTRIES,		"There are no DB2 based Domino databases on this server.")

#define DB2_ENABLE_TOOL_SRV_NOT_DB2				PKG_NSF9+255
	stringtext(DB2_ENABLE_TOOL_SRV_NOT_DB2,		"This server is not DB2 enabled.")
/* PKG_NSF9 codes limited to 0-255 */

/*	End of Error Codes */

/* PKG_DB2_STR codes limited to 0-127. */ 
/* STRINGS ONLY *** These can not be sued for error codes *** STRINGS ONLY */

#define STR_DB2_GROUP_SETSTATE				PKG_DB2_STR+0
	stringtext(STR_DB2_GROUP_SETSTATE,		"opened|locked|%s has been %s.|%s could not be %s.|Group %s does not exist.|Group %s is already %s.")

#define STR_DB2_GROUP_SETCLASS				PKG_DB2_STR+1
	stringtext(STR_DB2_GROUP_SETCLASS,		"The class for %s has been set to '%s'.|The class for %s could not be set to '%s'.|This group is already named %s.")

#define STR_DB2_GROUP_RENCLASS				PKG_DB2_STR+2
	stringtext(STR_DB2_GROUP_RENCLASS,		"The class '%s' has been renamed to '%s'.|The class '%s' could not be renamed to '%s'.|Class %s does not exist.|The old and new class names are identical.")

#define STR_DB2_GROUP_MOVE					PKG_DB2_STR+3
	stringtext(STR_DB2_GROUP_MOVE,			"Database %s is being moved to group %s.|Database %s is being moved to a new group.|Database %s could not be moved to group %s: %e |Please try again later. ")

#define STR_DB2_GROUP_LOCKED				PKG_DB2_STR+4
	stringtext(STR_DB2_GROUP_LOCKED,		"Open|Locked|Full")

#define ADMIN_DB2_ENABLE_EFFECT_RESTART				PKG_DB2_STR+5
	stringtext(ADMIN_DB2_ENABLE_EFFECT_RESTART,		"\nDB2 enablement will not take effect until the server has been restarted.\n")

#define ADMIN_DB2_ENABLE_EFFECT_RESTART1				PKG_DB2_STR+6
	stringtext(ADMIN_DB2_ENABLE_EFFECT_RESTART1,		"You may now restart the Domino server.\n")

#define ADMIN_DB2_ENABLE_EFFECT_RESTART2				PKG_DB2_STR+7
	stringtext(ADMIN_DB2_ENABLE_EFFECT_RESTART2,		"You must wait for all the appropriated DB2 information to be updated in the server document and then restart the Domino server.")

#define STR_DB2_GROUP_MOVE2					PKG_DB2_STR+8
	stringtext(STR_DB2_GROUP_MOVE2,			"Group %s does not exist.|Database %s does not exist.|Database %s is already in group %s.|Group %s is locked.|Group %s is full.")

#define DB2STR_XA_RBROLLBACK			PKG_DB2_STR+9
	stringtext(DB2STR_XA_RBROLLBACK,	"Rollback caused by unspecified reason.")
#define DB2STR_XA_RBCOMMFAIL			PKG_DB2_STR+10
	stringtext(DB2STR_XA_RBCOMMFAIL,	"Rollback caused by communication failure.")
#define DB2STR_XA_RBDEADLOCK			PKG_DB2_STR+11
	stringtext(DB2STR_XA_RBDEADLOCK,	"Deadlock detected.")
#define DB2STR_XA_RBINTEGRITY			PKG_DB2_STR+12
	stringtext(DB2STR_XA_RBINTEGRITY,	"An integrity error was detected.")
#define DB2STR_XA_RBOTHER			    PKG_DB2_STR+13
	stringtext(DB2STR_XA_RBOTHER,	    "The RM rolled back for unspecified reason.")
#define DB2STR_XA_RBPROTO			    PKG_DB2_STR+14
	stringtext(DB2STR_XA_RBPROTO,	    "A protocol error occurred in the RM.")
#define DB2STR_XA_RBTIMEOUT			    PKG_DB2_STR+15
	stringtext(DB2STR_XA_RBTIMEOUT,	    "The transaction took too long.")
#define DB2STR_XA_RBTRANSIENT			PKG_DB2_STR+16
	stringtext(DB2STR_XA_RBTRANSIENT,	"Retry the transaction.")
#define DB2STR_XA_NOMIGRATE			    PKG_DB2_STR+17
	stringtext(DB2STR_XA_NOMIGRATE,	    "Resumed in wrong thread.")
#define DB2STR_XA_HEURHAZ			    PKG_DB2_STR+18
	stringtext(DB2STR_XA_HEURHAZ,	    "The transaction branch may have been heuristically completed.")
#define DB2STR_XA_HEURCOM			    PKG_DB2_STR+19
	stringtext(DB2STR_XA_HEURCOM,	    "The transaction branch has been heuristically committed.")
#define DB2STR_XA_HEURRB			    PKG_DB2_STR+20
	stringtext(DB2STR_XA_HEURRB,	    "The transaction branch has been heuristically rolled back.")
#define DB2STR_XA_HEURMIX			    PKG_DB2_STR+21
	stringtext(DB2STR_XA_HEURMIX,	    "The transaction branch has been heuristically committed & rolled back.")
#define DB2STR_XA_RETRY			        PKG_DB2_STR+22
	stringtext(DB2STR_XA_RETRY,	        "Routine returned with no effect & may be reissued.")
#define DB2STR_XA_RDONLY			    PKG_DB2_STR+23
	stringtext(DB2STR_XA_RDONLY,	    "The transaction branch was read-only and has been committed.")
#define DB2STR_XAER_ASYNC			    PKG_DB2_STR+24
	stringtext(DB2STR_XAER_ASYNC,	    "An asynchronous operation already outstanding.")
#define DB2STR_XAER_RMERR			    PKG_DB2_STR+25
	stringtext(DB2STR_XAER_RMERR,	    "A RM error occurred.")
#define DB2STR_XAER_NOTA			    PKG_DB2_STR+26
	stringtext(DB2STR_XAER_NOTA,	    "The specified XID is not valid.")
#define DB2STR_XAER_INVAL			    PKG_DB2_STR+27
	stringtext(DB2STR_XAER_INVAL,	    "Invalid arguments were given.")
#define DB2STR_XAER_PROTO			    PKG_DB2_STR+28
	stringtext(DB2STR_XAER_PROTO,	    "Routine invoked in improper context.")
#define DB2STR_XAER_RMFAIL			    PKG_DB2_STR+29
	stringtext(DB2STR_XAER_RMFAIL,	    "RM unavailable.")
#define DB2STR_XAER_DUPID			    PKG_DB2_STR+30
	stringtext(DB2STR_XAER_DUPID,	    "The XID already exists.")
#define DB2STR_XAER_OUTSIDE			    PKG_DB2_STR+31
	stringtext(DB2STR_XAER_OUTSIDE,	    "The RM doing work outside of global transaction.")
#define DB2STR_CREATE_DB_SUCCESS	    PKG_DB2_STR+32
	stringtext(DB2STR_CREATE_DB_SUCCESS,"DB2 database %s successfully created with codepage/territory (%s/%s).")
#define DB2STR_CFG_SET_ERR			    PKG_DB2_STR+33
	stringtext(DB2STR_CFG_SET_ERR,	    "DB2NSFInit Error setting configuration parms for DB2 database %s sqlcode=%d.")
#define DB2STR_CREATE_DB_ERR		    PKG_DB2_STR+34
	stringtext(DB2STR_CREATE_DB_ERR,    "DB2NSFInit Error creating DB2 database %s sqlcode=%d.")
#define DB2STR_CREATE_SRV_TBLS_ERR	    PKG_DB2_STR+35
	stringtext(DB2STR_CREATE_SRV_TBLS_ERR,    "DB2NSFInit Error creating server tables for server %s.")
#define DB2STR_SESS_ALLOC_ERR		    PKG_DB2_STR+36
	stringtext(DB2STR_SESS_ALLOC_ERR,   "DB2NSFInit Error allocating DB2 session.")

#define STR_DB2_GROUP_DEFAULT				PKG_DB2_STR+37
	stringtext(STR_DB2_GROUP_DEFAULT,		"Default")

#define STR_DB2_GROUP_INPUT_INVALID				PKG_DB2_STR+38
	stringtext(STR_DB2_GROUP_INPUT_INVALID,		"The group you have requested does not exist.")

#define STR_DB2_DEF_GROUP_MOVE					PKG_DB2_STR+39
	stringtext(STR_DB2_DEF_GROUP_MOVE,			"DB2 group move request for %s submitted to server %a.|DB2 group move of %s request failed: %e|DB2 group move of %s completed.")

#define STR_DB2_COMPACT_DEFER			PKG_DB2_STR+40
	stringtext(STR_DB2_COMPACT_DEFER,   "Skipping NSFDB2 database %s.   DB2 reorg will be performed on tables in schemas %s and %s after NSF processing")
#define STR_DB2_TABLESPACE_REDUCE		PKG_DB2_STR+41
	stringtext(STR_DB2_TABLESPACE_REDUCE, "The DB2 tablespace %s is being reduced by %d pages.")
#define STR_DB2_TABLE_REORG_ALL			PKG_DB2_STR+42
	stringtext(STR_DB2_TABLE_REORG_ALL ,"Performing DB2 table and index reorgs for all NSFDB2 databases")
#define STR_DB2_TABLE_REORG_DIR			PKG_DB2_STR+43
	stringtext(STR_DB2_TABLE_REORG_DIR ,"Performing DB2 table and index reorgs for all NSFDB2 databases contained in %s")
#define STR_DB2_TABLE_REORG_TABLE1		PKG_DB2_STR+44
	stringtext(STR_DB2_TABLE_REORG_TABLE1 ,"Performing DB2 table and index reorgs for schemas %s and %s")
#define STR_DB2_TABLE_REORG_TABLE2		PKG_DB2_STR+45
	stringtext(STR_DB2_TABLE_REORG_TABLE2 ,"Continuing DB2 reorgs on tables in user schema %s, if applicable")
			
#define STR_DB2_DEF_GROUP_MOVE_GRP				PKG_DB2_STR+46
	stringtext(STR_DB2_DEF_GROUP_MOVE_GRP,		"DB2 group move request for %s to group %s submitted to server %a.|DB2 group move of %s to group %s request failed: %e|DB2 group move of %s to group %s completed.")
#define STR_DB2_STATS_RESET                     PKG_DB2_STR+47
	stringtext(STR_DB2_STATS_RESET,             "DB2NSF statistics were set to zero.")
#define STR_DB2_LOBLOCATOR_EXTRA_FETCH			PKG_DB2_STR+48
	stringtext(STR_DB2_LOBLOCATOR_EXTRA_FETCH,  "Extra DB2SQLFetch to free LOB locators->0x%x=%e")
#define STR_DB2NSF_DBCFG_WARNING		PKG_DB2_STR+49
    stringtext(STR_DB2NSF_DBCFG_WARNING,"Warning: Modifying DB2 database configuration parameters for optimization...")
#define STR_DB2NSF_DBMCFG_WARNING		PKG_DB2_STR+50
    stringtext(STR_DB2NSF_DBMCFG_WARNING,"Warning: Modifying DB2 database manager configuration parameters for optimization...")
#define STR_DB2_KEY_WARNING		PKG_DB2_STR+51
    stringtext(STR_DB2_KEY_WARNING,"Warning: DB2 will not be accessible by Domino until you copy the DB2 Key file, %s, to the Domino binaries directory.")
#endif


#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif


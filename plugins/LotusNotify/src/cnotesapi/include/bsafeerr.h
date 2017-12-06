
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



#ifndef BSAFE_ERR_DEFS
#define	BSAFE_ERR_DEFS


/* BSAFE (Security package) Error Code Definitions			*/

#define	ERR_BSAFE_PSW_PROMPT				(PKG_BSAFE+0)
#ifdef OS400
	errortext(ERR_BSAFE_PSW_PROMPT,			"Enter password (press the F3 key to abort): ")
#else
	errortext(ERR_BSAFE_PSW_PROMPT,			"Enter password (press the Esc key to abort): ")
#endif
#define	ERR_BSAFE_FALSE						(PKG_BSAFE+1)
	internaltext(ERR_BSAFE_FALSE,			"Do not suspend operation")
#define ERR_BSAFE_ALLOCATE					(PKG_BSAFE+2)
	internaltext(ERR_BSAFE_ALLOCATE,		"Error allocating security block")
#define ERR_BSAFE_FREE						(PKG_BSAFE+3)
	internaltext(ERR_BSAFE_FREE,			"Error freeing security block")
#define ERR_BSAFE_ENTRY						(PKG_BSAFE+4)
	internaltext(ERR_BSAFE_ENTRY,			"Error initializing security block")
#define ERR_BSAFE_EXIT						(PKG_BSAFE+5)
	internaltext(ERR_BSAFE_EXIT,			"Error cleaning up security block")
#define ERR_BSAFE_PAUSE						(PKG_BSAFE+6)
	internaltext(ERR_BSAFE_PAUSE,			"Security operation temporarily suspended")
#define ERR_BSAFE_BADKEY					(PKG_BSAFE+7)
	errortext(ERR_BSAFE_BADKEY,				"The encrypted data has been modified or the wrong key was used to decrypt it")
#define ERR_BSAFE_BADCTX					(PKG_BSAFE+8)
	errortext(ERR_BSAFE_BADCTX,				"Bad Security Context information")
#define ERR_BSAFE_BADOPCODE					(PKG_BSAFE+9)
	errortext(ERR_BSAFE_BADOPCODE,			"Illegal Security function code")
#define	ERR_BSAFE_BADCHECKSUM				(PKG_BSAFE+10)
	errortext(ERR_BSAFE_BADCHECKSUM,		"The encrypted data has been modified or the wrong key was used to decrypt it")
#define	ERR_BSAFE_BADDATA					(PKG_BSAFE+11)
	errortext(ERR_BSAFE_BADDATA,			"The encrypted data has been modified or the wrong key was used to decrypt it")

#define ERR_BSAFE_BUG_1						(PKG_BSAFE+12)
	errortext(ERR_BSAFE_BUG_1,				"BSAFE package software error #1")
#define ERR_BSAFE_POOLFULL					(PKG_BSAFE+13)
	errortext(ERR_BSAFE_POOLFULL,			"Insufficient memory - BSAFE pool is full")

#define ERR_BSAFE_CACHEFULL					(PKG_BSAFE+14)
	errortext(ERR_BSAFE_CACHEFULL,			"Insufficient memory - BSAFE Name lookup cache is full")

#define ERR_BSAFE_TOOSMALL					(PKG_BSAFE+15)
	errortext(ERR_BSAFE_TOOSMALL,			"Buffer used to receive cryptographic output was too small")
#define ERR_BSAFE_BAD_ATTRIBUTES			(PKG_BSAFE+16)
	errortext(ERR_BSAFE_BAD_ATTRIBUTES,		"Inconsistent name attributes")
#define ERR_BSAFE_MDLENGTH					(PKG_BSAFE+17)
	errortext(ERR_BSAFE_MDLENGTH,			"Bad Message Digest length")
#define ERR_BSAFE_NOT_ALLOWED				(PKG_BSAFE+18)
	errortext(ERR_BSAFE_NOT_ALLOWED,		"Attempted encryption operation is not supported by this version of Notes.")
#define	ERR_BSAFE_ILLEGAL_IDFILE			(PKG_BSAFE+19)
	errortext(ERR_BSAFE_ILLEGAL_IDFILE,		"The specified ID file may only be used inside of North America.")
#define	ERR_BSAFE_BAD_PI_VERSION			(PKG_BSAFE+20)
	errortext(ERR_BSAFE_BAD_PI_VERSION,		"The public key or certificate version stored in the Address Book is not supported.")
#define	ERR_BSAFE_BAD_PI_CHECKSUM			(PKG_BSAFE+21)
	errortext(ERR_BSAFE_BAD_PI_CHECKSUM,	"The public key or certificate stored in the Address Book has been corrupted.")
#define ERR_BSAFE_MISSING_PI				(PKG_BSAFE+22)
	errortext(ERR_BSAFE_MISSING_PI,			"There is no certificate in the Address Book.")
#define ERR_BSAFE_TABLE_MUST_BE_HI			(PKG_BSAFE+23)
	errortext(ERR_BSAFE_TABLE_MUST_BE_HI, 	"The supplied certificate table does not contain any hierarchical certificates")
#define	ERR_BSAFE_NO_COMMON_CERT			(PKG_BSAFE+24)
	errortext(ERR_BSAFE_NO_COMMON_CERT,		"No certificates in common")
#define	ERR_BSAFE_CERTTABLE_VERSION			(PKG_BSAFE+25)
	errortext(ERR_BSAFE_CERTTABLE_VERSION,	"Unrecognized certificate table version")
#define ERR_BSAFE_ILLUSE_GENID				(PKG_BSAFE+26)
	errortext(ERR_BSAFE_ILLUSE_GENID,		"Illegal use of an ID generator file")
#define	ERR_BSAFE_NOT_GENID					(PKG_BSAFE+27)
	errortext(ERR_BSAFE_NOT_GENID,			"Specified file is not an ID generator file")
#define	ERR_BSAFE_NOSUCH_PRVKEY				(PKG_BSAFE+28)
	errortext(ERR_BSAFE_NOSUCH_PRVKEY,		"Specified private key does not exist")
#define ERR_BSAFE_NOSUCH_ESCROW				(PKG_BSAFE+29)
	errortext(ERR_BSAFE_NOSUCH_ESCROW,		"This version of the software does not know the key of a required escrow authority")
#define	ERR_BSAFE_NETFILE					(PKG_BSAFE+30)
	errortext(ERR_BSAFE_NETFILE,			"WARNING: Network-based ID files should be password protected!")
#define	ERR_BSAFE_BAD_IDFILE_VERSION 		(PKG_BSAFE+31)
	errortext(ERR_BSAFE_BAD_IDFILE_VERSION,	"Unsupported ID file version.")
#define	ERR_BSAFE_NOSUCH_PUBKEY				(PKG_BSAFE+32)
	errortext(ERR_BSAFE_NOSUCH_PUBKEY,		"Specified public key does not exist")
#define ERR_BSAFE_PROC_NOT_INITED			(PKG_BSAFE+33)
	errortext(ERR_BSAFE_PROC_NOT_INITED,	"Process has not initialized with the security package")
#define ERR_BSAFE_SUBPROCESS				(PKG_BSAFE+34)
	errortext(ERR_BSAFE_SUBPROCESS,			"A sub-process cannot change to a new ID file or prompt for passwords.")
#define	ERR_BSAFE_NOMEMORY					(PKG_BSAFE+35)
	errortext(ERR_BSAFE_NOMEMORY,			"Insufficient memory for attempted security operation.")
#define	ERR_BSAFE_ILLFUNC					(PKG_BSAFE+36)
	debugtext(ERR_BSAFE_ILLFUNC,			"Unknown security function code.")
#define ERR_BSAFE_NULLPARAM					(PKG_BSAFE+37)
	debugtext(ERR_BSAFE_NULLPARAM,			"Illegal null parameter passed to security function.")
#define ERR_BSAFE_INVALID_XCERT				(PKG_BSAFE+38)
	errortext(ERR_BSAFE_INVALID_XCERT,		"Invalid Cross Certificate was found for %a. This could be a possible attack, please notify your administrator. Select 'Yes' to keep the current existing Cross Certificate")
#define ERR_BSAFE_ILLUSE_SAFECOPY			(PKG_BSAFE+39)
	errortext(ERR_BSAFE_ILLUSE_SAFECOPY,	"The ID file is a safe copy and cannot be used for that purpose.")
#define ERR_BSAFE_USER_ABORT				(PKG_BSAFE+40)
	errortext(ERR_BSAFE_USER_ABORT,			"The prompt for password was aborted by user")
#define ERR_BSAFE_ID_PROTECTED				(PKG_BSAFE+41)
	errortext(ERR_BSAFE_ID_PROTECTED,		"Cannot access or create the ID file")
#define ERR_BSAFE_ID_CREATE					(PKG_BSAFE+42)
	errortext(ERR_BSAFE_ID_CREATE,			"ID file cannot be created")

#define ERR_BSAFE_ID_TRUNC					(PKG_BSAFE+43)
	errortext(ERR_BSAFE_ID_TRUNC,			"The specified ID file has been corrupted, or is not an ID file")
#define ERR_BSAFE_CORRUPTIDFILE				ERR_BSAFE_ID_TRUNC

#define ERR_BSAFE_ID_INVFILENAME			(PKG_BSAFE+44)
	errortext(ERR_BSAFE_ID_INVFILENAME,		"Illegal ID file name: too long or uses invalid syntax")
#define ERR_BSAFE_ID_WRITETRUNC				(PKG_BSAFE+45)
	errortext(ERR_BSAFE_ID_WRITETRUNC,		"Cannot write to ID file; either it is READ-ONLY or the disk is out of space")
#define ERR_BSAFE_ID_INVDRIVE				(PKG_BSAFE+46)
	errortext(ERR_BSAFE_ID_INVDRIVE,		"Invalid drive specified for ID file")
#define ERR_BSAFE_NON_EXISTENT				(PKG_BSAFE+47)
	errortext(ERR_BSAFE_NON_EXISTENT,		"The requested item does not exist")
#define ERR_BSAFE_NEK_NOT_FOUND				(PKG_BSAFE+48)
	errortext(ERR_BSAFE_NEK_NOT_FOUND,		"You don't have any of the specified encryption keys")
#define ERR_BSAFE_KEY_NOT_FOUND				(PKG_BSAFE+49)
	errortext(ERR_BSAFE_KEY_NOT_FOUND,		"The cryptographic key was not found")
#define ERR_BSAFE_KEY_INV_FORMAT			(PKG_BSAFE+50)
	errortext(ERR_BSAFE_KEY_INV_FORMAT,		"The cryptographic key's format is invalid")
#define ERR_BSAFE_NO_KEYS					(PKG_BSAFE+51)
	errortext(ERR_BSAFE_NO_KEYS, 			"Your ID file does not contain any encryption keys")
#define ERR_BSAFE_UNKNOWN_CERTIFICATE		(PKG_BSAFE+52)
	errortext(ERR_BSAFE_UNKNOWN_CERTIFICATE,"Unrecognized or unsupported certificate type")
#define ERR_BSAFE_UNKNOWN_IDFILE			(PKG_BSAFE+53)
	errortext(ERR_BSAFE_UNKNOWN_IDFILE, 	"Unrecognized or unsupported ID File type")
#define ERR_BSAFE_UNKNOWN_DOCKEY			(PKG_BSAFE+54)
	errortext(ERR_BSAFE_UNKNOWN_DOCKEY, 	"Unrecognized or unsupported document encryption key type stored in the ID file")
#define ERR_BSAFE_UNKNOWN_USERINFO			(PKG_BSAFE+55)
	errortext(ERR_BSAFE_UNKNOWN_USERINFO, 	"Unrecognized or unsupported user-information structure")
#define ERR_BSAFE_UNKNOWN_ITEM				(PKG_BSAFE+56)
	debugtext(ERR_BSAFE_UNKNOWN_ITEM,		"Internal error: unknown BSAFE-item code or version")
#define ERR_BSAFE_INV_DNAME					(PKG_BSAFE+57)
	debugtext(ERR_BSAFE_INV_DNAME,			"Invalid name syntax")

/* Note: next error message name is misleading; the error was reused for
*		exceeding a 64K buffer length in a variety of cases.
*/

#define ERR_BSAFE_ODSCERT_TOOBIG			(PKG_BSAFE+58)
	errortext(ERR_BSAFE_ODSCERT_TOOBIG,		"Certificate object requires more than 64K bytes")
#define ERR_BSAFE_UNKNOWN_UDO				(PKG_BSAFE+59)
	errortext(ERR_BSAFE_UNKNOWN_UDO, 		"Unrecognized or unsupported user-descriptor object found in the ID file")
#define ERR_BSAFE_FILE_NOT_OPENED			(PKG_BSAFE+60)
	errortext(ERR_BSAFE_FILE_NOT_OPENED,	"You must first open the ID file before attempting that operation")

#define	ERR_BSAFE_CERTIFICATE_REVOKED		(PKG_BSAFE + 61)
	errortext (ERR_BSAFE_CERTIFICATE_REVOKED,	"The certificate was revoked")
#define	ERR_BSAFE_CERT_VALIDITY				(PKG_BSAFE + 62)
	errortext (ERR_BSAFE_CERT_VALIDITY,		"Certificate is expired or not yet valid")
#define	ERR_BSAFE_CRL_VALIDITY				(PKG_BSAFE + 63)
	errortext (ERR_BSAFE_CRL_VALIDITY,		"The CRL is expired or not yet valid")
#define	ERR_BSAFE_CRL_NOT_FOUND				(PKG_BSAFE + 64)
	errortext (ERR_BSAFE_CRL_NOT_FOUND,		"A CRL was required, but not found for the certificate")
#define	ERR_BSAFE_KEY_USAGE					(PKG_BSAFE + 65)
	errortext (ERR_BSAFE_KEY_USAGE,			"The certificate or chain is invalid due to a key usage violation")
#define	ERR_BSAFE_BASIC_CONSTRAINTS			(PKG_BSAFE + 66)
	errortext (ERR_BSAFE_BASIC_CONSTRAINTS,	"The certificate or chain is invalid due to a basic constraints violation")
#define STR_BSAFE_IDC_SUBJECT				(PKG_BSAFE+67)
	stringtext(STR_BSAFE_IDC_SUBJECT, "%A has certified the ID belonging to %A.  The certificate can be merged into your ID file by using the Actions menu 'Accept Certificate...' option.")
#define ERR_BSAFE_SENDER_SIGNING_CERT_IS_EXPIRED		(PKG_BSAFE+68)
	errortext(ERR_BSAFE_SENDER_SIGNING_CERT_IS_EXPIRED,	"Senders' signing certificate is expired")
#define ERR_BSAFE_SENDER_SIGNING_CERT_HAS_SINCE_EXPIRED	(PKG_BSAFE+69)
	errortext(ERR_BSAFE_SENDER_SIGNING_CERT_HAS_SINCE_EXPIRED,	"Senders' signing certificate has expired after this mail was signed")
#define ERR_BSAFE_INVALID_XCERT_NO_ARG		(PKG_BSAFE+70)
	errortext(ERR_BSAFE_INVALID_XCERT_NO_ARG,	"An invalid Cross Certificate was found. It may be a corrupted Cross Certificate, an attack or a new public key issued. Please notify administrator of the problem")	
#define ERR_BSAFE_CANT_CREATE_XCERT			(PKG_BSAFE+71)
	errortext(ERR_BSAFE_CANT_CREATE_XCERT,		"The requested Cross Certificate will not be issued because the subject is already trusted.")
#define ERR_BSAFE_WRONG_SUBJECT_KEY_IN_SIGNATURE	(PKG_BSAFE+72)
	errortext (ERR_BSAFE_WRONG_SUBJECT_KEY_IN_SIGNATURE, "The signer's public key found in the signature does not match the one stored in the directory")
#define ERR_BSAFE_NO_VALID_PW				(PKG_BSAFE+73)
	errortext (ERR_BSAFE_NO_VALID_PW,		"Password has not yet been validated")

#define ERR_BSAFE_ILL_PW_ALG				(PKG_BSAFE+74)
	errortext (ERR_BSAFE_ILL_PW_ALG,		"Unrecognized password object or algorithm")

/* available PKG_BSAFE+75 */
#define STR_BSAFE_SMIME_UNABLE_TO_DECODE_STREAM	(PKG_BSAFE+76)
	stringtext(STR_BSAFE_SMIME_UNABLE_TO_DECODE_STREAM,	"Unable to decode S/MIME stream")
#define	ERR_BSAFE_IDFILE_LOCKED				(PKG_BSAFE+77)
	errortext(ERR_BSAFE_IDFILE_LOCKED,		"The ID File is in use elsewhere and cannot be modified")
#define ERR_BSAFE_NEK_EXISTS				(PKG_BSAFE+78)
	errortext(ERR_BSAFE_NEK_EXISTS,			"Cannot add the encryption key to your ID file.  A key with that name already exists.")
#define ERR_BSAFE_KFM_INVREADOPT			(PKG_BSAFE+79)
	debugtext(ERR_BSAFE_KFM_INVREADOPT,		"Software error: conflicting read ID File options")
#define ERR_BSAFE_MERGE_NONCOPY				(PKG_BSAFE+80)
	errortext(ERR_BSAFE_MERGE_NONCOPY,		"The certificate(s) have not been issued to you and cannot be merged into your ID file.  They have been issued to")
#define ERR_BSAFE_MERGE_NOPRVKEY			(PKG_BSAFE+81)
	errortext(ERR_BSAFE_MERGE_NOPRVKEY,		"The Public Key cannot be accepted since there is no matching Private Key found in the ID file.")
#define	ERR_BSAFE_NONAMECHANGE				(PKG_BSAFE+82)
	errortext(ERR_BSAFE_NONAMECHANGE,		"If you change the user name, this ID will lose all of its certificates.  When the name change is complete, this ID will need to be certified.  Do you want to continue?")
#define ERR_BSAFE_NOMERGECERT				(PKG_BSAFE+83)
	errortext(ERR_BSAFE_NOMERGECERT,		"There are no certificates to be merged into your ID file")
#define	ERR_BSAFE_INVUSERIDFILE				(PKG_BSAFE+84)
	errortext(ERR_BSAFE_INVUSERIDFILE,		"Your ID file has been corrupted, or is not an ID file.")
#define ERR_BSAFE_CORRUPTFILE				(PKG_BSAFE+85)
	errortext(ERR_BSAFE_CORRUPTFILE,		"The specified ID file has been corrupted, or is not an ID file.")
#define ERR_BSAFE_REQUIRES_NONFLATID 		(PKG_BSAFE+86)
	errortext(ERR_BSAFE_REQUIRES_NONFLATID, "This request is not supported with the version of your ID file.  An ID file containing a hierarchical name is required.")
#define ERR_BSAFE_NO_CROSS_CERT		 		(PKG_BSAFE+87)
	errortext(ERR_BSAFE_NO_CROSS_CERT,		"The Address Book does not contain a cross certificate capable of validating the public key.")
#define ERR_BSAFE_INCOMPLETE_CERTTABLE	 	(PKG_BSAFE+88)
	errortext(ERR_BSAFE_INCOMPLETE_CERTTABLE, "The certificate table does not contain enough valid certificates to verify the public key of its owner.")
#define ERR_BSAFE_ILLUSE_CERTIFIER			(PKG_BSAFE+89)
	errortext(ERR_BSAFE_ILLUSE_CERTIFIER,	"Illegal use of a Certifier ID file")
#define ERR_BSAFE_MUSTBE_CERTIFIER			(PKG_BSAFE+90)
	errortext(ERR_BSAFE_MUSTBE_CERTIFIER,	"The ID file is not a Certifier ID file and cannot be used to issue certificates")
#define ERR_BSAFE_WRITEPROTECTED			(PKG_BSAFE+91)
	errortext(ERR_BSAFE_WRITEPROTECTED,		"The ID file is write protected")
#define ERR_BSAFE_NOPSW_ON_SAFECOPY			(PKG_BSAFE+92)
	errortext(ERR_BSAFE_NOPSW_ON_SAFECOPY,	"This is a safe-copy version of an ID file and cannot be password protected")
#define ERR_BSAFE_YOU_MUST_BE_HI			(PKG_BSAFE+93)
	errortext(ERR_BSAFE_YOU_MUST_BE_HI,		"The local system cannot complete the operation because it is not running with a hierarchical name")
#define ERR_BSAFE_WRONG_SUBJECT_KEY			(PKG_BSAFE+94)
	errortext(ERR_BSAFE_WRONG_SUBJECT_KEY,	"The subject's public key found in the certificate is not the one stored in our ID file for that entity.")
#define ERR_BSAFE_ILL_CERTTABLE				(PKG_BSAFE+95)
	errortext(ERR_BSAFE_ILL_CERTTABLE,		"The supplied certificate table used to validate the signer's public key is improperly formed")
#define ERR_BSAFE_CERTIFIER_MUST_BE_HI		(PKG_BSAFE+96)
	errortext(ERR_BSAFE_CERTIFIER_MUST_BE_HI, "The certifier has not been assigned a hierarchical name")
#define ERR_BSAFE_EXTERNAL_PASSWORD			(PKG_BSAFE+97)
	errortext(ERR_BSAFE_EXTERNAL_PASSWORD,	"Password Externally Supplied")
#define ERR_BSAFE_EXTERNAL_PWD_AND_DATA		(PKG_BSAFE+98)
	errortext(ERR_BSAFE_EXTERNAL_PWD_AND_DATA, "Password and Data Externally Supplied")
#define ERR_BSAFE_FLAT_KEY_TOO_BIG			(PKG_BSAFE+99)
	errortext(ERR_BSAFE_FLAT_KEY_TOO_BIG,	"RSA key size limit for a flat ID exceeded")
#define ERR_BSAFE_CERTREQ_SUBMITTED			(PKG_BSAFE+100)
	errortext(ERR_BSAFE_CERTREQ_SUBMITTED,	"A certificate request has been submitted")

#define ERR_BSAFE_ID_EXTRA_UPDATED			(PKG_BSAFE+101)
	errortext(ERR_BSAFE_ID_EXTRA_UPDATED,	"%s username and password were updated in the ID file.")

#define ERR_BSAFE_ID_EXTRA_UPDATE_FAIL			(PKG_BSAFE+102)
	errortext(ERR_BSAFE_ID_EXTRA_UPDATE_FAIL,	"%s username and password could not be updated in the ID file.")

#define ERR_BSAFE_CANCELED					(PKG_BSAFE+103)
	errortext(ERR_BSAFE_CANCELED,			"Operation canceled")

#define ERR_BSAFE_ID_EXTRA_DELETED			(PKG_BSAFE+104)
	errortext(ERR_BSAFE_ID_EXTRA_DELETED,	"%s username and password were cleared from the ID file.")

#define ERR_BSAFE_ASSIGNED_TO_DIFF_LANG		(PKG_BSAFE+105)
	errortext(ERR_BSAFE_ASSIGNED_TO_DIFF_LANG,	"That name has already been associated with another language specifier.")

#define ERR_BSAFE_ID_EXTRA_DELETE_FAIL			(PKG_BSAFE+106)
	errortext(ERR_BSAFE_ID_EXTRA_DELETE_FAIL,	"%s username and password could not be cleared from the ID file.")


#define ERR_BSAFE_CERT_HAS_ALTNAME			(PKG_BSAFE+108)
	errortext(ERR_BSAFE_CERT_HAS_ALTNAME,	"The operation cannot be completed because the certificate contains an alternate name")

#define ERR_BSAFE_ID_EXTRA_TYPE			(PKG_BSAFE+109)
	errortext(ERR_BSAFE_ID_EXTRA_TYPE,	"Domino Controller|DB2")
/* available PKG_BSAFE + 110 */

#define	ERR_BSAFE_USA_KEY_ONLY				(PKG_BSAFE+111)
	errortext(ERR_BSAFE_USA_KEY_ONLY,		"This key may only be stored in ID files with North American licenses")
#define ERR_BSAFE_ID_READ_ONLY				(PKG_BSAFE+112)
	errortext(ERR_BSAFE_ID_READ_ONLY,		"The ID file was not opened for write access")
#define ERR_BSAFE_NO_PUBLIC_INFO			(PKG_BSAFE+113)
	errortext(ERR_BSAFE_NO_PUBLIC_INFO,		"Your public key was not found in the Name and Address Book")
#define ERR_BSAFE_TOOMANY_ORG_UNITS			(PKG_BSAFE+114)
	errortext(ERR_BSAFE_TOOMANY_ORG_UNITS,	"You can create only as many as four Organizational Unit levels")
#define ERR_BSAFE_MUSTBE_HIERARCHICAL		(PKG_BSAFE+115)
	errortext(ERR_BSAFE_MUSTBE_HIERARCHICAL, "This request is not supported for the specified ID file.  An ID file containing a hierarchical name is required.")
#define ERR_BSAFE_ID_DRIVE_NOT_READY		(PKG_BSAFE+116)
	errortext(ERR_BSAFE_ID_DRIVE_NOT_READY,	"The disk drive specified for the ID file is not ready")
#define ERR_BSAFE_ID_DISK					(PKG_BSAFE+117)
	errortext(ERR_BSAFE_ID_DISK,			"Cannot update the ID file; insufficient disk space")
#define ERR_BSAFE_ID_IOERROR	  			(PKG_BSAFE+118)
	errortext(ERR_BSAFE_ID_IOERROR,			"I/O error experienced while accessing the ID file")
#define ERR_BSAFE_PASSWORD_REQUIRED			(PKG_BSAFE+119)
	errortext(ERR_BSAFE_PASSWORD_REQUIRED,	"A password is required for this ID file")
#define ERR_BSAFE_BAD_AUTH_CODE				(PKG_BSAFE+120)
	errortext(ERR_BSAFE_BAD_AUTH_CODE,		"The authentication code has failed the consistency check")
#define ERR_BSAFE_ILL_AUTH_CODE_VERSION		(PKG_BSAFE+121)
	errortext(ERR_BSAFE_ILL_AUTH_CODE_VERSION, "The authentication code version number is not supported")
#define ERR_BSAFE_NEWNAME_TOO_LONG			(PKG_BSAFE+122)
	errortext(ERR_BSAFE_NEWNAME_TOO_LONG,	"Cannot create certificate, resultant subject name would be too long")
#define ERR_BSAFE_CERT_NOT_ANCESTOR			(PKG_BSAFE+123)
	errortext(ERR_BSAFE_CERT_NOT_ANCESTOR,	"The certifier is not an ancestor of the subject in the certificate.")
#define ERR_BSAFE_ILL_ATFUNC				(PKG_BSAFE+124)
	errortext(ERR_BSAFE_ILL_ATFUNC,			"Unrecognized or unsupported @CERTIFICATE code")
#define ERR_BSAFE_CERTIFIER_NOT_IN_NAB		(PKG_BSAFE+125)
	errortext(ERR_BSAFE_CERTIFIER_NOT_IN_NAB, "A required certifier entry was not found in the Name and Address Book; consult the Notes Log for details on the specific entry")
#define ERR_BSAFE_ILL_DISTINFO				(PKG_BSAFE+126)
	errortext(ERR_BSAFE_ILL_DISTINFO,		"Unrecognized or unsupported Distinguished Name structure")
#define ERR_BSAFE_INSUF_INPUT_ARGS			(PKG_BSAFE+127)
	errortext(ERR_BSAFE_INSUF_INPUT_ARGS,	"Internal error: too few parameters passed in subroutine call")
#define ERR_BSAFE_CERT_MISMATCH				(PKG_BSAFE+128)
	errortext(ERR_BSAFE_CERT_MISMATCH,		"Either the certificates' issuers or the certificates' subjects don't match")
#define ERR_BSAFE_ILLUSE_XCERT				(PKG_BSAFE+129)
	errortext(ERR_BSAFE_ILLUSE_XCERT,		"A cross-certificate was illegally used as a substitute for an ordinary certificate")
#define ERR_BSAFE_TOO_MANY_CERTS			(PKG_BSAFE+130)
	errortext(ERR_BSAFE_TOO_MANY_CERTS,		"Certificate table could not be constructed because there are too many certificates")
#define ERR_BSAFE_TOO_FEW_CERTS				(PKG_BSAFE+131)
	errortext(ERR_BSAFE_TOO_FEW_CERTS,		"The supplied Certificate Table is missing a required certificate to complete the operation")
#define ERR_BSAFE_ID_MUST_BE_HI				(PKG_BSAFE+132)
	errortext(ERR_BSAFE_ID_MUST_BE_HI,		"This operation is only supported for ID file's containing hierarchical names")
#define ERR_BSAFE_INV_KFHANDLE				(PKG_BSAFE+133)
	errortext(ERR_BSAFE_INV_KFHANDLE,		"Invalid KFHANDLE")
#define ERR_BSAFE_FILE_LOCKED				(PKG_BSAFE+134)
	errortext(ERR_BSAFE_FILE_LOCKED,		"The ID file is locked by another process. Try again later")
#define ERR_BSAFE_MUST_BE_SERVER			(PKG_BSAFE+135)
	errortext(ERR_BSAFE_MUST_BE_SERVER,		"Only the server process is allowed to attempt that operation")
#define ERR_BSAFE_INAPPROPRIATE_ORGUNIT		(PKG_BSAFE+136)
	errortext(ERR_BSAFE_INAPPROPRIATE_ORGUNIT, "Org Unit may only be supplied when upgrading flat to hierarchical")

/* available PKG_BSAFE+137  */

#define ERR_BSAFE_WRONG_SUBJECT_KEY_X		(PKG_BSAFE+138)
	errortext(ERR_BSAFE_WRONG_SUBJECT_KEY_X, "The subject's public key found in the cross certificate does not match the one found in the certificate table.")
#define ERR_BSAFE_INVCERT_DATA				(PKG_BSAFE+139)
	errortext(ERR_BSAFE_INVCERT_DATA,		"Error processing certificate created by %A for %A")
#define ERR_BSAFE_INVCROSSCERT_SIG			(PKG_BSAFE+140)
	errortext(ERR_BSAFE_INVCROSSCERT_SIG,	"The signature on the cross certificate was found to be invalid.  Check the log file for details.")
#define ERR_BSAFE_WRONG_CERT_SUBJECT		(PKG_BSAFE+141)
	errortext(ERR_BSAFE_WRONG_CERT_SUBJECT,	"The certificate contains the wrong subject name")
#define ERR_BSAFE_WRONG_CERT_ISSUER			(PKG_BSAFE+142)
	errortext(ERR_BSAFE_WRONG_CERT_ISSUER,	"The certificate contains the wrong issuer name")

#define STR_BSAFE_SECPANEL_TRUST_CONSTRAINT_PEOPLE	(PKG_BSAFE+143)
	stringtext(STR_BSAFE_SECPANEL_TRUST_CONSTRAINT_PEOPLE,	"Mail encryption certificate not found|Usable for signing only (no encryption)|Usable for encryption only (no signing)|May not pass verification checks")
#define IDX_STR_BSAFE_SECPANEL_CERT_NOTFOUND			1
#define IDX_STR_BSAFE_SECPANEL_CERT_NOENCRYPTION		2
#define IDX_STR_BSAFE_SECPANEL_CERT_ENCRYPTION			3
#define IDX_STR_BSAFE_SECPANEL_CERT_NOTVERIFIED			4

#define STR_BSAFE_SECPANEL_TRUST_CONSTRAINT			(PKG_BSAFE+144)
	stringtext(STR_BSAFE_SECPANEL_TRUST_CONSTRAINT,	"<All Names>|<All Internet Names>|*%s|<All Flat Names>")
#define IDX_STR_BSAFE_SECPANEL_TRUST_CONSTRAINT_ALL			1
#define IDX_STR_BSAFE_SECPANEL_TRUST_CONSTRAINT_ALLINET		2
#define IDX_STR_BSAFE_SECPANEL_TRUST_CONSTRAINT_STARSTR		3
#define IDX_STR_BSAFE_SECPANEL_TRUST_CONSTRAINT_ALLFLAT		4

#define ERR_BSAFE_INVCERT_SIG				(PKG_BSAFE+145)
	errortext(ERR_BSAFE_INVCERT_SIG,		"The signature on the certificate was found to be invalid.  Check the log file for details.")

#define ERR_BSAFE_SECPANEL_NO_HOME_SERVER		(PKG_BSAFE+146)
	errortext(ERR_BSAFE_SECPANEL_NO_HOME_SERVER,	"Your home server could not be contacted or is not configured.")

#define ERR_BSAFE_WRONG_SUBJECT_KEY_SRV			(PKG_BSAFE+147)
	errortext(ERR_BSAFE_WRONG_SUBJECT_KEY_SRV,	"The subject's public key found in the certificate is not the one stored in the server's ID file for that entity.  Check the server's log file for details.")
#define ERR_BSAFE_WRONG_SUBJECT_KEY_LOC			(PKG_BSAFE+148)
	errortext(ERR_BSAFE_WRONG_SUBJECT_KEY_LOC,	"The subject's public key found in the certificate is not the one stored in our ID file for that entity.    Check the local log file for details.")
#define ERR_BSAFE_CERT_NOT_ANCESTOR_SRV			(PKG_BSAFE+149)
	errortext(ERR_BSAFE_CERT_NOT_ANCESTOR_SRV,	"The certifier is not an ancestor of the subject in the certificate.  Check the server's log file for details.")
#define ERR_BSAFE_CERT_NOT_ANCESTOR_LOC			(PKG_BSAFE+150)
	errortext(ERR_BSAFE_CERT_NOT_ANCESTOR_LOC,	"The certifier is not an ancestor of the subject in the certificate.  Check the local log file for details.")
#define ERR_BSAFE_ADDBOOK_CERT_ENTRY			(PKG_BSAFE+151)
	errortext(ERR_BSAFE_ADDBOOK_CERT_ENTRY,		"Error locating a Domino Directory entry for certifier %s")


/* (PKG_BSAFE ends at 151 */


#define	ERR_BSAFE2_ATTRIBUTE_UNKNOWN			(PKG_BSAFE2+0)
	errortext(ERR_BSAFE2_ATTRIBUTE_UNKNOWN,		"Attribute type is unrecognized ")

#define	ERR_BSAFE2_UNABLE_TO_PARSE_CERT 		(PKG_BSAFE2+1)
	errortext(ERR_BSAFE2_UNABLE_TO_PARSE_CERT,	"Unable to parse certificate ")

#define	ERR_BSAFE2_BAD_RECOVERY_INFO 	    	(PKG_BSAFE2+2)
	errortext(ERR_BSAFE2_BAD_RECOVERY_INFO,		"Nonexistent or invalid recovery information")

#define	ERR_BSAFE2_KEY_USAGE_RESTRICTION 		(PKG_BSAFE2+3)
	errortext(ERR_BSAFE2_KEY_USAGE_RESTRICTION,	"A cross certificate will not be made due to key usage restrictions in the input certificate.")

#define ERR_BSAFE_BIG_CERT_ELEMENT				(PKG_BSAFE2+4)
	errortext (ERR_BSAFE_BIG_CERT_ELEMENT,		"Certificate element too large for processing.")
#define ERR_BSAFE_NAMELIST_VERSION				(PKG_BSAFE2+5)
	errortext (ERR_BSAFE_NAMELIST_VERSION,		"Unrecognized name list version.")
#define ERR_BSAFE_NAMELIST_MALFORMED			(PKG_BSAFE2+6)
	errortext (ERR_BSAFE_NAMELIST_MALFORMED,	 "The name list is improperly formed and may have been corrupted.")
#define ERR_BSAFE_NAMELIST_NOSUCH_INDEX			(PKG_BSAFE2+7)
	errortext (ERR_BSAFE_NAMELIST_NOSUCH_INDEX,	"The requested name list element does not exist.")
#define ERR_BSAFE_TOO_MANY_ALTNAMES				(PKG_BSAFE2+8)
	errortext (ERR_BSAFE_TOO_MANY_ALTNAMES,		"Too many alternate names in the name list.")
#define ERR_BSAFE_LANGTAG_EXISTS				(PKG_BSAFE2+9)
	errortext (ERR_BSAFE_LANGTAG_EXISTS,		"A name with the specified language already exits.")
#define ERR_BSAFE_LANGTAG_UNKNOWN				(PKG_BSAFE2+10)
	errortext (ERR_BSAFE_LANGTAG_UNKNOWN,		"The specified language is unknown")
#define ERR_BSAFE_NAME_MALFORMED				(PKG_BSAFE2+11)
	errortext (ERR_BSAFE_NAME_MALFORMED,		"An improperly formed name was encountered")
#define ERR_BSAFE_INV_CERT						(PKG_BSAFE2+12)
	errortext (ERR_BSAFE_INV_CERT,				"Invalid certificate contents or format")
#define ERR_ASN1_ILL_OID						(PKG_BSAFE2+13)
	errortext (ERR_ASN1_ILL_OID,				"Illegal ASN.1 Object ID field")
#define ERR_ASN1_ILL_DATATYPE					(PKG_BSAFE2+14)
	errortext (ERR_ASN1_ILL_DATATYPE,			"Illegal or unexpected ASN.1 data type")
#define ERR_ASN1_ILL_DATALENGTH					(PKG_BSAFE2+15)
	errortext (ERR_ASN1_ILL_DATALENGTH,			"Illegal or unsupported ASN.1 data length")

/* 		(PKG_BSAFE2:  limited to 0-15 */
/* TIPEM errors */

#define ERR_BSAFE3_TE_UNKNOWN					(PKG_BSAFE3+0)
    errortext (ERR_BSAFE3_TE_UNKNOWN,			"Unknown S/MIME error")
#define ERR_BSAFE3_TE_ASN_SIGNATURE				(PKG_BSAFE3+1)
    errortext (ERR_BSAFE3_TE_ASN_SIGNATURE,		"Invalid signature on certificate or CRL")
#define ERR_BSAFE3_TE_ATTRIBUTES_OBJ			(PKG_BSAFE3+2)
    errortext (ERR_BSAFE3_TE_ATTRIBUTES_OBJ,	 "Invalid cryptographic attributes object")
#define ERR_BSAFE3_TE_EOS						(PKG_BSAFE3+3)
    errortext (ERR_BSAFE3_TE_EOS, 				"End of cryptographic data stream")

#define ERR_BSAFE4_TE_MEMORY_OBJ				(PKG_BSAFE3+4)
    errortext (ERR_BSAFE4_TE_MEMORY_OBJ,		"Invalid cryptographic memory object")
#define ERR_BSAFE4_TE_OVER_32K					(PKG_BSAFE3+5)
    errortext (ERR_BSAFE4_TE_OVER_32K,			"Cryptographic data block is too big to be processed")
#define ERR_BSAFE4_TE_PARAMETER					(PKG_BSAFE3+6)
    errortext (ERR_BSAFE4_TE_PARAMETER,			"Invalid parameter passed to cryptographic function")
#define ERR_BSAFE5_TE_VERSION					(PKG_BSAFE3+7)
    errortext (ERR_BSAFE5_TE_VERSION,			"S/MIME version not supported")

#define ERR_BSAFE5_ERROR						(PKG_BSAFE3+8)
    errortext (ERR_BSAFE5_ERROR,				"Error -- ")
#define ERR_BSAFE5_WARNING						(PKG_BSAFE3+9)
    errortext (ERR_BSAFE5_WARNING,				"Warning -- ")
#define ERR_BSAFE5_LOG							(PKG_BSAFE3+10)
	errortext(ERR_BSAFE5_LOG,					"%s %s")
#define ERR_BSAFE5_TE_UNTRUSTED_CERTS_CRLS		(PKG_BSAFE3+11)
	errortext (ERR_BSAFE5_TE_UNTRUSTED_CERTS_CRLS,	"Cannot establish trust in a certificate or CRL.")

#define ERR_NOCA_KEYMATCH						(PKG_BSAFE3+12)
	errortext (ERR_NOCA_KEYMATCH,				"The certifier key in the certificate does not match the key stored in the directory.")
#define ERR_ILL_RECERT_FCT						(PKG_BSAFE3+13)
	errortext (ERR_ILL_RECERT_FCT,				"The requested rename or recertify function is unknown or unsupported.")
#define ERR_NO_ANCESTOR_LANGTAG					(PKG_BSAFE3+14)
	errortext (ERR_NO_ANCESTOR_LANGTAG,			"The certifier has not yet been assigned a name associated with the proposed language.")
#define ERR_NO_INET_KEY							(PKG_BSAFE3+15)
	errortext (ERR_NO_INET_KEY,					"This entry has not been assigned a public key suitable for use on the internet.")

#define ERR_BSAFE_NAME_EXISTS               	(PKG_BSAFE3+16)
	errortext (ERR_BSAFE_NAME_EXISTS,      		"This name already exists.")

#define ERR_BSAFE_INVALID_BB                	(PKG_BSAFE3+17)
    errortext (ERR_BSAFE_INVALID_BB,       		"You are not authorized to recover this ID file.")

	
	
#define ERR_ASN1_INV_SYNTAX						(PKG_BSAFE3+18)
	errortext (ERR_ASN1_INV_SYNTAX,				"Invalid ASN.1 syntax")
#define ERR_BSAFE_INV_CRL						(PKG_BSAFE3+19)
	errortext (ERR_BSAFE_INV_CRL,				"Invalid CRL contents or format")
#define ERR_BSAFE_INV_EXTENSIONS_OBJ 			(PKG_BSAFE3+20)
	errortext (ERR_BSAFE_INV_EXTENSIONS_OBJ, 	"Invalid certificate extensions object")
#define ERR_BSAFE_INV_MSG_FORMAT				(PKG_BSAFE3+21)
	errortext (ERR_BSAFE_INV_MSG_FORMAT,		"Cryptographic error: invalid message format")
#define ERR_BSAFE_BAD_KEY_LENGTH				(PKG_BSAFE3+22)
	errortext (ERR_BSAFE_BAD_KEY_LENGTH,		"Invalid cryptographic key length")
#define ERR_BSAFE_BAD_SIGNATURE					(PKG_BSAFE3+23)
	errortext (ERR_BSAFE_BAD_SIGNATURE,			"Could not verify cryptographic signature")
#define ERR_BSAFE_BAD_RANDOM_STATE				(PKG_BSAFE3+24)
	errortext (ERR_BSAFE_BAD_RANDOM_STATE,		"Bad random number generator state")
#define ERR_BSAFE_CERTCRL_NOT_FOUND				(PKG_BSAFE3+25)
	errortext (ERR_BSAFE_CERTCRL_NOT_FOUND,		"Certificate, private key or CRL was not found")
#define ERR_BSAFE_MAKING_CERT_CHAIN				(PKG_BSAFE3+26)
	errortext (ERR_BSAFE_MAKING_CERT_CHAIN,		"A certificate chain could not be constructed")
#define ERR_BSAFE_CRYPTO_UPDATE_COUNT			(PKG_BSAFE3+27)
	errortext (ERR_BSAFE_CRYPTO_UPDATE_COUNT,	"Cryptographic error: update operation called an inappropriate number of times")
#define ERR_BSAFE_DATA_STREAM					(PKG_BSAFE3+28)
	errortext (ERR_BSAFE_DATA_STREAM,			"Data streaming error encountered during cryptographic operation")
#define ERR_BSAFE_CERT_EXT_EXISTS				(PKG_BSAFE3+29)
	errortext (ERR_BSAFE_CERT_EXT_EXISTS,		"Certificate extension already exists")
#define ERR_BSAFE_GENERIC_DATA					(PKG_BSAFE3+30)
	errortext (ERR_BSAFE_GENERIC_DATA,			"Unknown data error occurred during cryptographic processing")
#define ERR_BSAFE_HARDWARE		 				(PKG_BSAFE3+31)
	errortext (ERR_BSAFE_HARDWARE,		 		"Cryptographic hardware error")

/* 		(PKG_BSAFE3:  Limited to 0-31 */


#define ERR_BSAFE_UNSUPPORTED_CERTCRLSIG		(PKG_BSAFE4+0)
	errortext (ERR_BSAFE_UNSUPPORTED_CERTCRLSIG,"Unsupported certificate or CRL signature algorithm")
#define ERR_BSAFE_UNSUPPORTED_CRYPTO_OP			(PKG_BSAFE4+1)
	errortext (ERR_BSAFE_UNSUPPORTED_CRYPTO_OP,	"An unsupported cryptographic operation was requested")
#define ERR_BSAFE_WEAK_KEY						(PKG_BSAFE4+2)
	errortext (ERR_BSAFE_WEAK_KEY,				"The data supplied would generate a known weak cryptographic key")

/*		+3 thru +6 are available  */

#define ERR_BSAFE_INVALID_PASSWORD              (PKG_BSAFE4+7)
	errortext (ERR_BSAFE_INVALID_PASSWORD,      "Password does not meet the requirement")
#define ERR_BSAFE_INSERT_SMARTCARD              (PKG_BSAFE4+8)
	errortext (ERR_BSAFE_INSERT_SMARTCARD,      "Please insert the smartcard")
#define ERR_BSAFE_SC_PIN_INVALID                (PKG_BSAFE4+9)
	errortext (ERR_BSAFE_SC_PIN_INVALID,        "Incorrect PIN")
#define ERR_BSAFE_SC_RESET                      (PKG_BSAFE4+10)
	errortext (ERR_BSAFE_SC_RESET,              "A smartcard device error has occurred. Please eject and re-insert your smartcard.")
#define ERR_BSAFE_SC_UNKNOWN                    (PKG_BSAFE4+11)
	errortext (ERR_BSAFE_SC_UNKNOWN,            "An unknown smartcard error has occurred.")
#define ERR_BSAFE_SC_INVALID_CONFIG             (PKG_BSAFE4+12)
	errortext (ERR_BSAFE_SC_INVALID_CONFIG,     "Incomplete or incorrect smartcard configuration.")
#define ERR_BSAFE_SC_UNSUPPORTED_FUNC           (PKG_BSAFE4+13)
	errortext (ERR_BSAFE_SC_UNSUPPORTED_FUNC,   "This feature is not supported by your smartcard.")
#define ERR_BSAFE_SC_DEVICE_MEMORY              (PKG_BSAFE4+14)
	errortext (ERR_BSAFE_SC_DEVICE_MEMORY,      "Insufficient free space on smartcard. Please contact your administrator.")
#define ERR_BSAFE_SC_PIN_LOCKED                 (PKG_BSAFE4+15)
	errortext (ERR_BSAFE_SC_PIN_LOCKED,         "Your smartcard or cryptographic device is locked. Please contact your administrator.")
#define ERR_BSAFE_RECOVERY_INFO_REMOVED			(PKG_BSAFE4+16)
	errortext (ERR_BSAFE_RECOVERY_INFO_REMOVED, "Recovery information has been removed")
#define ERR_BSAFE_SC_NOT_RECOGNIZED             (PKG_BSAFE4+17)
	errortext (ERR_BSAFE_SC_NOT_RECOGNIZED,     "This smartcard has not been initialized or is incompatible with your smartcard reader.")
#define ERR_BSAFE_SC_KEY_CREATE                 (PKG_BSAFE4+18)
	errortext (ERR_BSAFE_SC_KEY_CREATE,         "This key could not be written to your smartcard.")
#define ERR_SMIME_RECIPIENT_BAD_CERT            (PKG_BSAFE4+19)
    errortext (ERR_SMIME_RECIPIENT_BAD_CERT,	" The certificate is not usable.")
#define ERR_SMIME_RECIPIENT_CERT_CHAIN          (PKG_BSAFE4+20)
    errortext (ERR_SMIME_RECIPIENT_CERT_CHAIN,	" The certificate chain is not trusted.")
#define ERR_SMIME_RECIPIENT_ILLEGAL         	(PKG_BSAFE4+21)
    errortext (ERR_SMIME_RECIPIENT_ILLEGAL,		" You are not allowed to encrypt for this recipient.")
#define ERR_SMIME_RECIPIENT_ADD_FAILURE         (PKG_BSAFE4+22)
    errortext (ERR_SMIME_RECIPIENT_ADD_FAILURE,	" You cannot encrypt a message to this recipient.")
#define ERR_SMIME_RECIPIENT_BAD_EMAIL           (PKG_BSAFE4+23)
    errortext (ERR_SMIME_RECIPIENT_BAD_EMAIL,	" This recipient's email address was not found in the certificate.")
#define ERR_SMIME_RECIPIENT_CERT_REVOKED        (PKG_BSAFE4+24)
    errortext (ERR_SMIME_RECIPIENT_CERT_REVOKED,"This recipient's certificate had been revoked.")
#define ERR_SMIME_RECIPIENT_EMAIL_MISMATCH      (PKG_BSAFE4+25)
    errortext (ERR_SMIME_RECIPIENT_EMAIL_MISMATCH," This recipient's email address in the directory did not match what was found in the certificate.")
#define ERR_SMIME_RECIPIENT_CERT_EXPIRED        (PKG_BSAFE4+26)
    errortext (ERR_SMIME_RECIPIENT_CERT_EXPIRED," This recipient's certificate is expired.")
#define ERR_SMIME_SENDER_SIGNING_CERT_EXPIRED   (PKG_BSAFE4+27)
    errortext (ERR_SMIME_SENDER_SIGNING_CERT_EXPIRED," The certificate used to send signed mail is expired.")
#define ERR_LTPA_TOKEN_SHOULD_RENEW				(PKG_BSAFE4+28)
	errortext (ERR_LTPA_TOKEN_SHOULD_RENEW,		"Single Sign-On token should be renewed.")
#define ERR_BSAFE_CERT_EXPIRES_SERVER				(PKG_BSAFE4+29)
	errortext(ERR_BSAFE_CERT_EXPIRES_SERVER,		"WARNING:  Server certificate issued to %s by %s will expire on %s. Contact your Domino administrator.  %s")
#define ERR_BSAFE_CERT_EXPIRED_SERVER				(PKG_BSAFE4+30)
	errortext(ERR_BSAFE_CERT_EXPIRED_SERVER,		"WARNING:  Server certificate issued to %s by %s expired on %s and can no longer be used.  Contact your Domino administrator.")

/* 		PKG_BSAFE4:  Limited to 0-31 */


#define ERR_BSAFE_RECOVERY_INFO_TOO_OLD         (PKG_BSAFE5+0)
    errortext (ERR_BSAFE_RECOVERY_INFO_TOO_OLD, "The recovery information was not accepted because it is the same or older than your current recovery information")
#define ERR_BSAFE_NOT_LEAF_CERT                 (PKG_BSAFE5+1)
    errortext (ERR_BSAFE_NOT_LEAF_CERT,         "Only leaf certificates can be deleted from the ID file")
#define ERR_BSAFE_NO_REPOSITORY_NAME            (PKG_BSAFE5+2)
    errortext (ERR_BSAFE_NO_REPOSITORY_NAME,    "You must specify a name for the backup repository address")
#define ERR_BSAFE_PKCS12_IMPORT_ADD_KEY  		(PKG_BSAFE5+3)
	errortext(ERR_BSAFE_PKCS12_IMPORT_ADD_KEY, "Cannot add key from the import file.")
#define ERR_BSAFE_PKCS12_IMPORT_ADD_CERT  		(PKG_BSAFE5+4)
	errortext(ERR_BSAFE_PKCS12_IMPORT_ADD_CERT, "Cannot add certificate from the import file.")
#define ERR_BSAFE_SC_NO_PASSWORD                (PKG_BSAFE5+5)
	errortext(ERR_BSAFE_SC_NO_PASSWORD,         "The password for this ID file is not stored on a smartcard.")
#define ERR_BSAFE_SC_NO_DLL_FOUND                (PKG_BSAFE5+6)
	errortext(ERR_BSAFE_SC_NO_DLL_FOUND,         "The path or file selected for the smartcard driver is invalid.")
#define ERR_BSAFE_ILL_CERTTAB_ITEM				(PKG_BSAFE5+7)
	errortext(ERR_BSAFE_ILL_CERTTAB_ITEM,		"A certificate table item was unrecognized")
#define ERR_BSAFE_CANT_MODIFY_ACTIVE_ID			(PKG_BSAFE5+8)
	errortext(ERR_BSAFE_CANT_MODIFY_ACTIVE_ID,	"You cannot modify the active ID file")
#define ERR_BSAFE_TOO_MANY_RO_CERTIFICATES		(PKG_BSAFE5+9)
	errortext(ERR_BSAFE_TOO_MANY_RO_CERTIFICATES, "The certificate table contains too many key rollover certificates")
#define ERR_BSAFE_KEYGEN_WRONG_STATE			(PKG_BSAFE5+10)
	errortext(ERR_BSAFE_KEYGEN_WRONG_STATE,		"Pending public keys are in the wrong state for the requested operation")
#define ERR_BSAFE_ROLLOVER_TOO_MANY				(PKG_BSAFE5+12)
	errortext(ERR_BSAFE_ROLLOVER_TOO_MANY,		"Too many key rollover certificates")
#define ERR_BSAFE_RECOVERY_CERTIFIER_NOT_ANCESTOR (PKG_BSAFE5+13)
	errortext (ERR_BSAFE_RECOVERY_CERTIFIER_NOT_ANCESTOR,	"Recovery information is from a certifier that is not an ancestor of this user")

#define ERR_BSAFE_ROLLOVER_UNKNOWN_TYPE			(PKG_BSAFE5+15)
	errortext(ERR_BSAFE_ROLLOVER_UNKNOWN_TYPE,	"Unknown key rollover object type")

/* 		PKG_BSAFE5:  Available, limited to 0-15 */

#define ERR_BSAFE_TE_UNTRUSTED_SIGNER			(PKG_BSAFE6+0)
	errortext (ERR_BSAFE_TE_UNTRUSTED_SIGNER, 	"The signer's certificate is not trusted.")
#define ERR_BSAFE_INCOMPLETE_CERT_CHAIN			(PKG_BSAFE6+1)
	errortext (ERR_BSAFE_INCOMPLETE_CERT_CHAIN,	"Cannot accept internet certificate because the certificate authority certificate is unavailable.")
#define ERR_BSAFE_MISSING_PRIVATE_KEY			(PKG_BSAFE6+2)
	errortext (ERR_BSAFE_MISSING_PRIVATE_KEY,	"Cannot accept internet certificate because the current ID file was not used to create the original request.")
#define ERR_BSAFE_CERT_ALREADY_IN_ID_FILE		(PKG_BSAFE6+3)
	errortext (ERR_BSAFE_CERT_ALREADY_IN_ID_FILE,	"Cannot accept internet certificate because the certificate is already in the ID file.")
		

/*	Note: Put here, because there was space, but these really belong
* 	elsewhere
*/

/* LATER:
*	In some cases, copies of ERR_DESK codes moved here because they can
*	be returned by the SDK. Corresponding ERR_DESK codes should be eliminated
*/

#define ERR_BSAFE_ILL_IDCHAR			(PKG_BSAFE6+4)
	errortext(ERR_BSAFE_ILL_IDCHAR,		"The only allowed characters for a person, server, or certifier name are letters, numbers, ampersand, apostrophe, hyphen, period, space, and underscore")
#define ERR_BSAFE_ILL_DOMAIN			(PKG_BSAFE6+5)
	errortext(ERR_BSAFE_ILL_DOMAIN,		"The only allowed characters for a domain name are letters, numbers, ampersand, apostrophe, hyphen, space, and underscore")
#define ERR_BSAFE_NA_ENTRY_NOT_FOUND	(PKG_BSAFE6+6)
	stringtext(ERR_BSAFE_NA_ENTRY_NOT_FOUND,	"The ID was certified but there was no corresponding entry in the Address Book to be updated")
#define ERR_BSAFE_OLDNSF				(PKG_BSAFE6+7)
	errortext(ERR_BSAFE_OLDNSF,			"Unable to perform this operation; options were selected that cannot be used with this older format Notes database.")
#define ERR_BSAFE_INV_RECERT_FORM		(PKG_BSAFE6+8)
	errortext(ERR_BSAFE_INV_RECERT_FORM,"Cannot certify this entry because the form used to create it is not supported for this operation.")
#define ERR_BSAFE_NOT_IMPLEMENTED		(PKG_BSAFE6+9)
	errortext(ERR_BSAFE_NOT_IMPLEMENTED,"Operation is not yet implemented")
#define ERR_BSAFE_DUP_NAME				(PKG_BSAFE6+10)
	errortext(ERR_BSAFE_DUP_NAME,		"Duplicate name found")
#define ERR_BSAFE_ILL_CERTIFIER_NAME	(PKG_BSAFE6+11)
	errortext(ERR_BSAFE_ILL_CERTIFIER_NAME, "You specified an illegal certifier name")
#define ERR_BSAFE_NO_INET_CERTS			(PKG_BSAFE6+12)
	errortext(ERR_BSAFE_NO_INET_CERTS,	"Could not locate or process internet certificates for this ID.")
#define ERR_BSAFE_PKCS12_IMPORT_BAD_FILE_READ  	(PKG_BSAFE6+13)
	errortext(ERR_BSAFE_PKCS12_IMPORT_BAD_FILE_READ, "Import file could not be read.  Check file permissions.")
#define ERR_BSAFE_PKCS12_IMPORT_BAD_INFO  	(PKG_BSAFE6+14)
	errortext(ERR_BSAFE_PKCS12_IMPORT_BAD_INFO, "Unsupported PKCS12 version or content.  Check the import file.")
#define ERR_BSAFE_PKCS12_IMPORT_BAD_UNKNOWN  	(PKG_BSAFE6+15)
	errortext(ERR_BSAFE_PKCS12_IMPORT_BAD_UNKNOWN, "Cannot handle the import file.  Check file name and file password.")

/* 		PKG_BSAFE6:  Available, limited to 0-15 */


/*PKG_BSAFE_STR bsafe strings 0 - 127 */
#define ERR_BSAFE_DISPLAY_IDFILE			(PKG_BSAFE_STR)
	stringtext(ERR_BSAFE_DISPLAY_IDFILE,	"The ID file being used is: ")
#define STR_BSAFE_RSA_PRIMARY				(PKG_BSAFE_STR + 2)
	stringtext(STR_BSAFE_RSA_PRIMARY,		"Primary RSA")
#define STR_BSAFE_RSA_SECONDARY				(PKG_BSAFE_STR + 3)
	stringtext(STR_BSAFE_RSA_SECONDARY,		"Secondary RSA")
#define STR_BSAFE_RC2						(PKG_BSAFE_STR + 4)
	stringtext(STR_BSAFE_RC2,				"RC2")
#define STR_BSAFE_RC4						(PKG_BSAFE_STR + 5)
	stringtext(STR_BSAFE_RC4,				"RC4")
#define STR_BSAFE_USA						(PKG_BSAFE_STR + 6)
	stringtext(STR_BSAFE_USA,				"North American")
#define	STR_BSAFE_NONUSA					(PKG_BSAFE_STR + 7)
	stringtext(STR_BSAFE_NONUSA,			"International")
#define STR_BSAFE_SIGNED_MAIL				(PKG_BSAFE_STR + 8)
	stringtext(STR_BSAFE_SIGNED_MAIL,		"Signed mail using %d bit %e key")
#define STR_BSAFE_SEALED_DOC				(PKG_BSAFE_STR + 9)
	stringtext(STR_BSAFE_SEALED_DOC,		"Encrypted document using %d/%d bit %e/%e keys")
#define STR_BSAFE_CERTIFY					(PKG_BSAFE_STR + 10)
	stringtext(STR_BSAFE_CERTIFY, 			"Created certificate using %d bit %e key")
#define STR_BSAFE_IDFILE_ENCRYPTED			(PKG_BSAFE_STR + 11)
	stringtext(STR_BSAFE_IDFILE_ENCRYPTED,	"ID file is encrypted with %d bit %e key")
#define STR_BSAFE_FORCED_AUTH				(PKG_BSAFE_STR + 12)
	stringtext(STR_BSAFE_FORCED_AUTH,		"Authentication is being forced due to secure channel setting.")
#define STR_BSAFE_CERT_EXPIRES				(PKG_BSAFE_STR + 13)
	stringtext(STR_BSAFE_CERT_EXPIRES,		"WARNING:  Your certificate issued to %s by %s will expire on %s. To request a new certificate, you should start User Security (see File - Security - User Security) and click Renew.  %s")
#define STR_BSAFE_CERT_EXPIRED				(PKG_BSAFE_STR + 14)
	stringtext(STR_BSAFE_CERT_EXPIRED,		"WARNING:  Your certificate issued to %s by %s expired on %s and can no longer be used.  Contact your Domino administrator.")
#define STR_BSAFE_INVALID					(PKG_BSAFE_STR + 15)
	stringtext(STR_BSAFE_INVALID,			"Invalid")
#define STR_BSAFE_UNKNOWN					(PKG_BSAFE_STR + 16)
	stringtext(STR_BSAFE_UNKNOWN,			"Unknown")
#define STR_BSAFE_CERTIFYING				(PKG_BSAFE_STR + 17)
	stringtext(STR_BSAFE_CERTIFYING,		"Certifying %A")
#define	STR_BSAFE_CERTIFIED					(PKG_BSAFE_STR + 18)
	stringtext(STR_BSAFE_CERTIFIED,			"%A successfully certified")
#define STR_BSAFE_REPLACE_ANCESTRY			(PKG_BSAFE_STR + 19)
	stringtext(STR_BSAFE_REPLACE_ANCESTRY,	"A certificate hierarchy has already been assigned to this ID.  Do you wish to continue and replace it?")
#define STR_BSAFE_UPGRADETOHI				(PKG_BSAFE_STR + 20)
	stringtext(STR_BSAFE_UPGRADETOHI,		"The ID file will be upgraded to hierarchical format.  Do you wish to continue?")
#define STR_BSAFE_AUTHENTICATED				(PKG_BSAFE_STR + 21)
	stringtext(STR_BSAFE_AUTHENTICATED,		"Authenticated: %d bit Ticket, %d bit %e session key, %e on RC4 escrow.")
#define STR_BSAFE_HICERTIFIERID				(PKG_BSAFE_STR + 22)
	stringtext(STR_BSAFE_HICERTIFIERID,		"Hierarchical Certifier")
#define STR_BSAFE_UNKNOWNID					(PKG_BSAFE_STR + 23)
	stringtext(STR_BSAFE_UNKNOWNID,			"Unknown ID type")
#define STR_BSAFE_ID_FULL					(PKG_BSAFE_STR + 24)
     stringtext(STR_BSAFE_ID_FULL,			"Notes")
#define STR_BSAFE_ID_DESKTOP				(PKG_BSAFE_STR + 25)
     stringtext(STR_BSAFE_ID_DESKTOP,		"Notes Desktop")

/* Use OSLoadSubString to load substrings - english total length<80! 
*/
#define STR_BSAFE_LIST_IDFILE				(PKG_BSAFE_STR + 26)
	stringtext(STR_BSAFE_LIST_IDFILE,		"Non-Hierarchical ID|Hierarchical User or Server|Hierarchical Certifier")

#define IDX_BSAFE_IDFILE_FLAT		1
#define IDX_BSAFE_IDFILE_HIUSER		2
#define IDX_BSAFE_IDFILE_HICA		3

#define STR_BSAFE_LIST_MISCID				(PKG_BSAFE_STR + 27)
	stringtext(STR_BSAFE_LIST_MISCID,		"Safe Copy|Unknown ID type|Internet Certifier| ")

#define IDX_BSAFE_MISCID_SAFE		1
#define IDX_BSAFE_MISCID_UNKNOWN	2
#define IDX_BSAFE_MISCID_INETCA		3
#define IDX_BSAFE_MISCID_BLANK		4

#define STR_BSAFE_CROSS_CERTIFYING			(PKG_BSAFE_STR + 28)
	stringtext(STR_BSAFE_CROSS_CERTIFYING,	"Cross certifying %A")
#define	STR_BSAFE_CROSS_CERTIFIED			(PKG_BSAFE_STR + 29)
	stringtext(STR_BSAFE_CROSS_CERTIFIED,	"%A successfully cross certified")
#define STR_BSAFE_MAX_IBULKDATAKEY			(PKG_BSAFE_STR + 30)
#ifdef FRANCE_ENGLISH
	stringtext(STR_BSAFE_MAX_IBULKDATAKEY,	"40")
#else
	stringtext(STR_BSAFE_MAX_IBULKDATAKEY,	"64")
#endif
#define STR_BSAFE_DUP_NAMES_IN_DIRECTORY	(PKG_BSAFE_STR + 31)
	stringtext (STR_BSAFE_DUP_NAMES_IN_DIRECTORY, "%A and %A both already exist in the directory but in different entries")
#define	STR_BSAFE_SC_PIN_PROMPT				(PKG_BSAFE_STR + 32)
#ifdef OS400
	stringtext(STR_BSAFE_SC_PIN_PROMPT,		"Enter PIN (press the F3 key to abort): ")
#else
	stringtext(STR_BSAFE_SC_PIN_PROMPT,		"Enter PIN (press the Esc key to abort): ")
#endif
#define STR_BSAFE_DISPLAY_SMARTCARD			(PKG_BSAFE_STR + 33)
	stringtext(STR_BSAFE_DISPLAY_SMARTCARD,	"The smartcard being used is: ")

/* available	PKG_BSAFE_STR + 34 */

#define STR_BSAFE_PRESS_ENTER				(PKG_BSAFE_STR + 35)
	stringtext(STR_BSAFE_PRESS_ENTER,		"Press ENTER to continue")

/* available PKG_BSAFE_STR + 36, 37 */

/* Following are reserved for Password Policy	*/
#define STR_BSAFE_PW_LEN_FAIL				 	(PKG_BSAFE_STR + 38)
	stringtext(STR_BSAFE_PW_LEN_FAIL,			"Password does not meet length requirement.")
#define STR_BSAFE_PW_QTY_FAIL					(PKG_BSAFE_STR + 39)
	stringtext(STR_BSAFE_PW_QTY_FAIL,			"Password does not meet quality requirement.")
#define STR_BSAFE_PW_ALPHA_FAIL					(PKG_BSAFE_STR + 40)
	stringtext(STR_BSAFE_PW_ALPHA_FAIL,			"Password does not contain minimum required number of alpha characters.")
#define STR_BSAFE_PW_NUM_FAIL  					(PKG_BSAFE_STR + 41)
	stringtext(STR_BSAFE_PW_NUM_FAIL,			"Password does not contain minimum required number of numeric characters.")
#define STR_BSAFE_PW_PUNC_FAIL					(PKG_BSAFE_STR + 42)
	stringtext(STR_BSAFE_PW_PUNC_FAIL,			"Password does not contain minimum required number of punctuation characters.")
#define STR_BSAFE_PW_UPP_FAIL  					(PKG_BSAFE_STR + 43)
	stringtext(STR_BSAFE_PW_UPP_FAIL,			"Password does not contain minimum required number of upper case characters.")
#define STR_BSAFE_PW_LOW_FAIL					(PKG_BSAFE_STR + 44)
	stringtext(STR_BSAFE_PW_LOW_FAIL,			"Password does not contain minimum required number of lower case characters.")
#define STR_BSAFE_PW_UNIQUE_FAIL  				(PKG_BSAFE_STR + 45)
	stringtext(STR_BSAFE_PW_UNIQUE_FAIL,		"Password does not contain enough number of unique characters.")
#define STR_BSAFE_PW_REPEAT_FAIL  				(PKG_BSAFE_STR + 46)
	stringtext(STR_BSAFE_PW_REPEAT_FAIL,		"Password fails repeat characters check.  There are too many of the same character.")
#define STR_BSAFE_PW_START_FAIL  				(PKG_BSAFE_STR + 47)
	stringtext(STR_BSAFE_PW_START_FAIL,			"Password fails the check 'must not start with'.")
#define STR_BSAFE_PW_END_FAIL  					(PKG_BSAFE_STR + 48)
	stringtext(STR_BSAFE_PW_END_FAIL,			"Password fails the check 'must not end with'.")
#define STR_BSAFE_PW_CNNAME_FAIL				(PKG_BSAFE_STR + 49)
	stringtext(STR_BSAFE_PW_CNNAME_FAIL,		"Password contains some aspect of the user name.")
#define STR_BSAFE_PW_FIRST_CNG	 				(PKG_BSAFE_STR + 50)
	stringtext(STR_BSAFE_PW_FIRST_CNG,			"User must change password on first use.")

															 
/*	The following are currently used to log to DDM with better context during authentication
*/

#define STR_BSAFE_REMOTE_HOST					(PKG_BSAFE_STR + 51)
	stringtext(STR_BSAFE_REMOTE_HOST,			"from host")

#define STR_BSAFE_AUTH_FAILED_WITHNET			(PKG_BSAFE_STR + 52)
	stringtext(STR_BSAFE_AUTH_FAILED_WITHNET,	"%a %e [%s] failed to authenticate")
#define STR_BSAFE_AUTH_FAILED_NONET				(PKG_BSAFE_STR + 53)
	stringtext(STR_BSAFE_AUTH_FAILED_NONET,		"%a failed to authenticate")

#define STR_BSAFE_AUTH_PROBLEM_WITHNET			(PKG_BSAFE_STR + 54)
	stringtext(STR_BSAFE_AUTH_PROBLEM_WITHNET,	"%a %e [%s] encountered non-fatal problem during authentication")
#define STR_BSAFE_AUTH_PROBLEM_NONET			(PKG_BSAFE_STR + 55)
	stringtext(STR_BSAFE_AUTH_PROBLEM_NONET,	"%a encountered non-fatal problem during authentication")

	/* Id recovery string for better logging*/

#define STR_BSAFE_BACKUP_ID_NAME_CHANGE			(PKG_BSAFE_STR + 56)
	stringtext (STR_BSAFE_BACKUP_ID_NAME_CHANGE, "ID containing name change for %A marked for backup")
#define STR_BSAFE_BACKUP_ID_NEW_NEK				(PKG_BSAFE_STR + 57)
	stringtext (STR_BSAFE_BACKUP_ID_NEW_NEK,		"ID containing new NEK for %A marked for backup")
#define STR_BSAFE_BACKUP_ID_NEWKEY_CERTS		(PKG_BSAFE_STR + 58)
	stringtext (STR_BSAFE_BACKUP_ID_NEWKEY_CERTS, "ID containing certificates for new keys for %A marked for backup")
#define STR_BSAFE_BACKUP_ID_NEW_RECOVERY_INFO	(PKG_BSAFE_STR + 59)
	stringtext (STR_BSAFE_BACKUP_ID_NEW_RECOVERY_INFO, "ID containing new recovery information for %A marked for backup")
#define STR_BSAFE_RECOVERY_INFO_CREATION_DATE	(PKG_BSAFE_STR + 60)
	stringtext (STR_BSAFE_RECOVERY_INFO_CREATION_DATE, "Creation date of recovery info to be imported is %s")
#define STR_BSAFE_RECOVERY_INFO_SIGNATURE_FAILURE	(PKG_BSAFE_STR + 61)
	stringtext (STR_BSAFE_RECOVERY_INFO_SIGNATURE_FAILURE, "Signature on recovery information failed to verify")
#define STR_BSAFE_RECOVERY_OF_ID_FILE			(PKG_BSAFE_STR + 62)
	stringtext (STR_BSAFE_RECOVERY_OF_ID_FILE,	"ID file for %A recovered successfully")
#define STR_BSAFE_RECOVERY_OF_ID_FILE_FAIL		(PKG_BSAFE_STR + 63)
	stringtext (STR_BSAFE_RECOVERY_OF_ID_FILE_FAIL,	"ID file for %A recovery failure")
#define STR_BSAFE_XCERT_LOOKUP_ERROR			(PKG_BSAFE_STR + 64)
	stringtext (STR_BSAFE_XCERT_LOOKUP_ERROR,	"Error during Directory lookup for cross certificates issued to %A or one of its ancestors")
#define STR_BSAFE_RECOVERY_ID_INFO_CREATION_DATE	(PKG_BSAFE_STR + 65)
	stringtext (STR_BSAFE_RECOVERY_ID_INFO_CREATION_DATE, "Creation date of recovery info in ID file is %s")
#define STR_BSAFE_RECOVERY_ID_INFO_UPDATED		(PKG_BSAFE_STR + 66)
	stringtext (STR_BSAFE_RECOVERY_ID_INFO_UPDATED, "New recovery info successfully updated in ID file for %A")
#define STR_BSAFE_BACKUP_ON_DEMAND				(PKG_BSAFE_STR+67)
	stringtext (STR_BSAFE_BACKUP_ON_DEMAND,		"Requested backup of ID file for %A")
#define STR_BSAFE_RECOVERY_COOKIES_CREATION_FAILURE 		(PKG_BSAFE_STR+68)
	stringtext (STR_BSAFE_RECOVERY_COOKIES_CREATION_FAILURE,	"Recovery passwords generation failure")
#define STR_BSAFE_RECOVERY_MULTI_PASSWORD_FAILURE	(PKG_BSAFE_STR+69)
	stringtext (STR_BSAFE_RECOVERY_MULTI_PASSWORD_FAILURE,		"Unable to set multi recovery passwords on ID file")
#define STR_BSAFE_RECOVERY_PASSWORD_EXTRA_FAILURE	(PKG_BSAFE_STR+70)
	stringtext (STR_BSAFE_RECOVERY_PASSWORD_EXTRA_FAILURE,		"Unable to generate password recovery extra in ID file")
#define STR_BSAFE_RECOVERY_KEYS_CREATION_FAILURE	(PKG_BSAFE_STR+71)
	stringtext (STR_BSAFE_RECOVERY_KEYS_CREATION_FAILURE,		"Unable to create keys recovery extra in ID file")
#define STR_BSAFE_RECOVERY_MAIL_BACKUP_FAIL			(PKG_BSAFE_STR+72)
	stringtext (STR_BSAFE_RECOVERY_MAIL_BACKUP_FAIL,		"Failure to mail backup recovery ID file: ")
#define STR_BSAFE_RECOVERY_MAIL_BACKUP_FAIL_NAMED	(PKG_BSAFE_STR+73)
	stringtext (STR_BSAFE_RECOVERY_MAIL_BACKUP_FAIL_NAMED,		"Failure to mail backup recovery ID file for %s to %s")
#define STR_BSAFE_BACKUP_ID_KEY_CHANGE				(PKG_BSAFE_STR + 74)
	stringtext (STR_BSAFE_BACKUP_ID_KEY_CHANGE, "ID containing new public keys for %A marked for backup")


#define STR_BSAFE_AUTH_LOCPROB_ON_CLIENT	   		(PKG_BSAFE_STR + 75)
	stringtext(STR_BSAFE_AUTH_LOCPROB_ON_CLIENT,	"Non-fatal problem encountered during authentication with server %A")
#define STR_BSAFE_AUTH_LOCFAIL_ON_CLIENT	   		(PKG_BSAFE_STR + 76)
	stringtext(STR_BSAFE_AUTH_LOCFAIL_ON_CLIENT,	"Failed to authenticate with server %A")
#define STR_BSAFE_AUTH_REMPROB_ON_CLIENT			(PKG_BSAFE_STR + 77)
	stringtext(STR_BSAFE_AUTH_REMPROB_ON_CLIENT,	"Server %A reported the following non-fatal problem during authentication")
#define STR_BSAFE_AUTH_REMFAIL_ON_CLIENT			(PKG_BSAFE_STR + 78)
	stringtext(STR_BSAFE_AUTH_REMFAIL_ON_CLIENT,	"Server %A reported the following problem causing authentication to fail")
#define STR_BSAFE_EXTRA_DATA_FUNCTIONS		(PKG_BSAFE_STR+79)
	stringtext (STR_BSAFE_EXTRA_DATA_FUNCTIONS,		"retrieval|deletion|update")
#define IDX_EXTRA_FUNC_OFFSET		1

#define STR_BSAFE_EXTRA_DATA_TYPES		(PKG_BSAFE_STR+80)
	stringtext (STR_BSAFE_EXTRA_DATA_TYPES,		"DB2 user name|DB2 password|Server Controller user name|Server Controller password")
#define IDX_EXTRA_TYPE_OFFSET		29

#define STR_BSAFE_EXTRA_DATA_LOGSTRING		(PKG_BSAFE_STR+81)
	stringtext (STR_BSAFE_EXTRA_DATA_LOGSTRING,		"%s %s by %a completed.|%s %s by %a failed.")
#define IDX_EXTRA_LOG_PASSED		1
#define IDX_EXTRA_LOG_FAILED		2

#define STR_BSAFE_BACKUP_ON_REG_SBJ             (PKG_BSAFE_STR+82)
	stringtext (STR_BSAFE_BACKUP_ON_REG_SBJ,     "Backup of newly registered ID file for %A")
#define STR_BSAFE_BACKUP_ON_ACCEPT_SBJ          (PKG_BSAFE_STR+83)
	stringtext (STR_BSAFE_BACKUP_ON_ACCEPT_SBJ,  "Backup of newly changed recovery information for %A")
#define STR_BSAFE_BACKUP_ON_MODIFY_SBJ          (PKG_BSAFE_STR+84)
	stringtext (STR_BSAFE_BACKUP_ON_MODIFY_SBJ,  "Backup of recent changes to ID file for %A")
#define STR_BSAFE_NEW_REC_INFO_SBJ              (PKG_BSAFE_STR+85)
	stringtext (STR_BSAFE_NEW_REC_INFO_SBJ,      "New ID file recovery information is attached.  Please add it to your ID file by using the Actions menu %s option.")


/*PKG_BSAFE_STR bsafe strings up to 127 */

#define ERR_BSAFE3_TE_ALLOC						ERR_BSAFE_NOMEMORY
#define ERR_BSAFE3_TE_ATTRIBUTE_COUNT			ERR_ASN1_INV_SYNTAX
#define ERR_BSAFE3_TE_ATTRIBUTE_NOT_FOUND		ERR_ASN1_INV_SYNTAX
#define ERR_BSAFE3_TE_ATTRIBUTE_TAG				ERR_ASN1_INV_SYNTAX
#define ERR_BSAFE3_TE_ATTRIBUTE_TYPE			ERR_BSAFE2_ATTRIBUTE_UNKNOWN
#define ERR_BSAFE3_TE_ATTRIBUTE_VALUE			ERR_ASN1_ILL_DATATYPE
#define ERR_BSAFE3_TE_ATTRIBUTE_VALUE_LEN		ERR_ASN1_ILL_DATALENGTH

#define ERR_BSAFE3_TE_BEGIN_PEM					ERR_BSAFE_INV_MSG_FORMAT
#define ERR_BSAFE3_TE_BER_ENCODING				ERR_ASN1_INV_SYNTAX

#define ERR_BSAFE3_TE_CANCEL					ERR_BSAFE_CANCELED
#define ERR_BSAFE3_TE_CERT_ENCODING				ERR_BSAFE_INV_CERT
#define ERR_BSAFE3_TE_CERT_OBJ					ERR_BSAFE_INV_CERT

#define ERR_BSAFE3_TE_CO_SET					ERR_BSAFE_BADCTX

#define ERR_BSAFE3_TE_CRL_ENCODING				ERR_BSAFE_INV_CRL
#define ERR_BSAFE3_TE_CRL_OBJ					ERR_BSAFE_INV_CRL

#define ERR_BSAFE3_TE_DATA						ERR_BSAFE_GENERIC_DATA
#define ERR_BSAFE3_TE_DBASE						ERR_BSAFE_BADCTX
#define ERR_BSAFE3_TE_DEK_ALG_NOT_SUPPORTED		ERR_BSAFE_UNSUPPORTED_CRYPTO_OP
#define ERR_BSAFE3_TE_DEK_ALG_UNKNOWN			ERR_BSAFE_UNSUPPORTED_CRYPTO_OP
#define ERR_BSAFE3_TE_DIGEST_OBJ				ERR_BSAFE_BADCTX
#define ERR_BSAFE3_TE_ENHANCED_TEXT_STREAM		ERR_BSAFE_DATA_STREAM

#define ERR_BSAFE3_TE_EXPONENT_EVEN				ERR_BSAFE_KEY_INV_FORMAT
#define ERR_BSAFE3_TE_EXPONENT_LEN				ERR_BSAFE_KEY_INV_FORMAT
#define ERR_BSAFE3_TE_EXTENSIONS_OBJ			ERR_BSAFE_INV_EXTENSIONS_OBJ
#define ERR_BSAFE3_TE_EXTENSION_ALREADY_EXISTS	ERR_BSAFE_CERT_EXT_EXISTS
#define ERR_BSAFE3_TE_HARDWARE					ERR_BSAFE_HARDWARE
#define ERR_BSAFE3_TE_HEADER					ERR_BSAFE_INV_MSG_FORMAT
#define ERR_BSAFE3_TE_INDEX						ERR_BSAFE4_TE_PARAMETER

#define ERR_BSAFE4_TE_CERT_CHAIN				ERR_BSAFE_MAKING_CERT_CHAIN
#define ERR_BSAFE4_TE_INPUT_LEN					ERR_BSAFE_GENERIC_DATA
#define ERR_BSAFE4_TE_INPUT_STREAM				ERR_BSAFE_DATA_STREAM

#define ERR_BSAFE4_TE_IO						ERR_BSAFE_DATA_STREAM
#define ERR_BSAFE4_TE_LIST_OBJ					ERR_BSAFE_BADCTX
#define ERR_BSAFE4_TE_ME_SET					ERR_BSAFE_BADCTX
#define ERR_BSAFE4_TE_MESSAGE_SIGNATURE			ERR_BSAFE_BAD_SIGNATURE
#define ERR_BSAFE4_TE_MIC_ALG_NOT_SUPPORTED		ERR_BSAFE_UNSUPPORTED_CRYPTO_OP
#define ERR_BSAFE4_TE_MIC_ALG_UNKNOWN			ERR_BSAFE_UNSUPPORTED_CRYPTO_OP
#define ERR_BSAFE4_TE_MODULUS_LEN				ERR_BSAFE_BAD_KEY_LENGTH
#define ERR_BSAFE4_TE_NAME_OBJ					ERR_BSAFE_BADCTX
#define ERR_BSAFE4_TE_NEED_RANDOM				ERR_BSAFE_BAD_RANDOM_STATE
#define ERR_BSAFE4_TE_NOT_FOUND					ERR_BSAFE_CERTCRL_NOT_FOUND
#define ERR_BSAFE4_TE_NOT_ME					(PKG_NSF+101)			/* ERR_NOT_SEALED_FOR_YOU	nsferr.h */
#define ERR_BSAFE4_TE_NOT_SUPPORTED				ERR_BSAFE_UNSUPPORTED_CRYPTO_OP
#define ERR_BSAFE4_TE_OUTPUT_LEN				ERR_BSAFE_TOOSMALL
#define ERR_BSAFE4_TE_OUTPUT_STREAM				ERR_BSAFE_DATA_STREAM

#define ERR_BSAFE4_TE_PASSWORD					(PKG_SECURE+8)			/* ERR_SECURE_BADPASSWORD	secerr.h */
#define ERR_BSAFE4_TE_PBE_ALG_NOT_SUPPORTED		ERR_BSAFE_UNSUPPORTED_CRYPTO_OP
#define ERR_BSAFE4_TE_PBE_ALG_UNKNOWN			ERR_BSAFE_UNSUPPORTED_CRYPTO_OP
#define ERR_BSAFE4_TE_PKCS_INPUT_STREAM			ERR_BSAFE_DATA_STREAM
#define ERR_BSAFE4_TE_PKCS_OUTPUT_STREAM		ERR_BSAFE_DATA_STREAM
#define ERR_BSAFE4_TE_PKCS_STREAM				ERR_BSAFE_DATA_STREAM

#define ERR_BSAFE4_TE_PRIVATE_KEY				ERR_BSAFE_KEY_INV_FORMAT
#define ERR_BSAFE4_TE_PROCESS					ERR_BSAFE4_TE_PARAMETER
#define ERR_BSAFE4_TE_PROTECTED_DATA			ERR_BSAFE_BADDATA
#define ERR_BSAFE4_TE_PUBLIC_KEY				ERR_BSAFE_KEY_INV_FORMAT
#define ERR_BSAFE4_TE_RANDOM_OBJ				ERR_BSAFE_BAD_RANDOM_STATE
#define ERR_BSAFE4_TE_SIGNATURE_ALG_NOT_SUPPORTED	ERR_BSAFE_UNSUPPORTED_CERTCRLSIG
#define ERR_BSAFE5_TE_SIGNATURE_ALG_UNKNOWN			ERR_BSAFE_UNSUPPORTED_CERTCRLSIG

#define ERR_BSAFE5_TE_TEXT_ENCODING				ERR_BSAFE_INV_MSG_FORMAT
#define ERR_BSAFE5_TE_TEXT_STREAM				ERR_BSAFE_DATA_STREAM
#define ERR_BSAFE5_TE_TOKEN						ERR_BSAFE4_TE_PARAMETER
#define ERR_BSAFE5_TE_VALIDITY					(PKG_SECURE+28)			/* ERR_SECURE_EXPIRED_CERT	secerr.h */
#define ERR_BSAFE5_TE_YOU_SET					ERR_BSAFE_BADCTX

#define ERR_CERT_MALFORMED						ERR_BSAFE_INV_CERT
#define ERR_KEYRING_MUSTBE_CA					ERR_BSAFE_MUSTBE_CERTIFIER


#endif

#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif


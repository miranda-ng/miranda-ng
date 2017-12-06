
#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(P128)
#endif

/*********************************************************************/
/*                                                                   */
/* Licensed Materials - Property of IBM                              */
/*                                                                   */
/* L-GHUS-5VMPGW, L-GHUS-5S3PEE                                      */
/* (C) Copyright IBM Corp. 1995, 2005  All Rights Reserved           */
/*                                                                   */
/* US Government Users Restricted Rights - Use, duplication or       */
/* disclosure restricted by GSA ADP Schedule Contract with           */
/* IBM Corp.                                                         */
/*                                                                   */
/*********************************************************************/



/* Extension Manager for Notes */

#ifndef EXMGR_DEFS
#define EXMGR_DEFS

#ifndef NSF_DATA_DEFS
#include "nsfdata.h"					/* We need DBHANDLE */
#endif

#ifndef NIF_DEFS
#include "nif.h"						/* We need HCOLLECTION */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Extension identifier */

typedef WORD EID;

/* handle passed back to identify registration */

typedef DWORD HEMREGISTRATION;

/* extension record; this data structure is passed to the callback routine */

typedef struct
	{
	EID			EId;					/* identifier */
	WORD		NotificationType;		/* EM_BEFORE or EM_AFTER */
	STATUS		Status;					/* core error code */
	VARARG_PTR	Ap;						/* ptr to args */
	} EMRECORD; 

/* the callback; takes one argument */

typedef STATUS (LNCALLBACKPTR EMHANDLER)(EMRECORD far *);

/*	Constants used in NotificationType */

#define EM_BEFORE	0
#define EM_AFTER	1

/*	Flags which can be passed to EMRegister */

#define EM_REG_BEFORE		0x0001
#define EM_REG_AFTER		0x0002

/* prototypes */

STATUS LNPUBLIC	EMRegister(EID EmID, DWORD Flags, EMHANDLER Proc, WORD RecursionID, HEMREGISTRATION far *rethRegistration);
STATUS LNPUBLIC	EMDeregister(HEMREGISTRATION hRegistration);
STATUS LNPUBLIC	EMCreateRecursionID(WORD far *retRecursionID);


#define EM_NSFDBCLOSESESSION				1
#define EM_NSFDBCLOSE						2
#define EM_NSFDBCREATE						3
#define EM_NSFDBDELETE						4
#define EM_NSFNOTEOPEN						5
#define EM_NSFNOTECLOSE						6
#define EM_NSFNOTECREATE					7
#define EM_NSFNOTEDELETE					8
#define EM_NSFNOTEOPENBYUNID				10
#define EM_FTGETLASTINDEXTIME				11
#define EM_FTINDEX							12
#define EM_FTSEARCH							13
#define EM_NIFFINDBYKEY						14
#define EM_NIFFINDBYNAME					15
#define EM_NIFOPENNOTE						17
#define EM_NIFREADENTRIES					18
#define EM_NIFUPDATECOLLECTION				20
#define EM_NSFDBALLOCOBJECT					22
#define EM_NSFDBCOMPACT						23
#define EM_NSFDBDELETENOTES					24
#define EM_NSFDBFREEOBJECT					25
#define EM_NSFDBGETMODIFIEDNOTETABLE		26
#define EM_NSFDBGETNOTEINFO					29
#define EM_NSFDBGETNOTEINFOBYUNID			30
#define EM_NSFDBGETOBJECTSIZE				31
#define EM_NSFDBGETSPECIALNOTEID			32
#define EM_NSFDBINFOGET						33
#define EM_NSFDBINFOSET						34
#define EM_NSFDBLOCATEBYREPLICAID			35
#define EM_NSFDBMODIFIEDTIME				36
#define EM_NSFDBREADOBJECT					37
#define EM_NSFDBREALLOCOBJECT				39
#define EM_NSFDBREPLICAINFOGET				40
#define EM_NSFDBREPLICAINFOSET				41
#define EM_NSFDBSPACEUSAGE					42
#define EM_NSFDBSTAMPNOTES					43
#define EM_NSFDBWRITEOBJECT					45
#define EM_NSFNOTEUPDATE					47
#define EM_NIFOPENCOLLECTION				50
#define EM_NIFCLOSECOLLECTION				51
#define EM_NSFDBGETBUILDVERSION				52
#define EM_NSFDBRENAME						54
#define EM_NSFDBITEMDEFTABLE				56
#define EM_NSFDBREOPEN						59
#define EM_NSFDBOPENEXTENDED				63	
#define EM_NSFNOTEOPENEXTENDED				64
#define EM_TERMINATENSF						69
#define EM_NSFNOTEDECRYPT					70
#define EM_GETPASSWORD						73
#define EM_SETPASSWORD						74
#define EM_NSFCONFLICTHANDLER				75	
#define EM_MAILSENDNOTE						83
#define EM_CLEARPASSWORD					90
#define EM_NSFNOTEUPDATEXTENDED				102
#define EM_SCHFREETIMESEARCH				105
#define EM_SCHRETRIEVE						106
#define EM_SCHSRVRETRIEVE					107
#define EM_NSFDBCOMPACTEXTENDED				121
#define EM_ADMINPPROCESSREQUEST				124
#define EM_NIFGETCOLLECTIONDATA				126
#define EM_NSFDBCOPYNOTE					127
#define EM_NSFNOTECOPY						128
#define EM_NSFNOTEATTACHFILE				129
#define EM_NSFNOTEDETACHFILE				130
#define EM_NSFNOTEEXTRACTFILE				131
#define EM_NSFNOTEATTACHOLE2OBJECT			132
#define EM_NSFNOTEDELETEOLE2OBJECT			133
#define EM_NSFNOTEEXTRACTOLE2OBJECT			134
#define EM_NSGETSERVERLIST					135
#define EM_NSFDBCOPY						136
#define EM_NSFDBCREATEANDCOPY				137
#define EM_NSFDBCOPYACL						138
#define EM_NSFDBCOPYTEMPLATEACL				139
#define EM_NSFDBCREATEACLFROMTEMPLATE		140
#define EM_NSFDBREADACL						141
#define EM_NSFDBSTOREACL					142
#define EM_NSFDBFILTER						143
#define EM_FTDELETEINDEX					144
#define EM_NSFNOTEGETINFO					145
#define EM_NSFNOTESETINFO					146
#define EM_NSFNOTECOMPUTEWITHFORM			147
#define EM_NIFFINDDESIGNNOTE				148
#define EM_NIFFINDPRIVATEDESIGNNOTE			149
#define EM_NIFGETLASTMODIFIEDTIME			150
#define EM_FTSEARCHEXT						160
#define EM_NAMELOOKUP						161
#define EM_NSFNOTEUPDATEMAILBOX				164
#define EM_NIFFINDDESIGNNOTEEXT         167
#define EM_AGENTOPEN                    170
#define EM_AGENTRUN                     171
#define EM_AGENTCLOSE                   172
#define EM_AGENTISENABLED               173
#define EM_AGENTCREATERUNCONTEXT        175
#define EM_AGENTDESTROYRUNCONTEXT       176
#define EM_AGENTSETDOCUMENTCONTEXT      177
#define EM_AGENTSETTIMEEXECUTIONLIMIT   178
#define EM_AGENTQUERYSTDOUTBUFFER       179
#define EM_AGENTREDIRECTSTDOUT          180
#define EM_SECAUTHENTICATION				184
#define EM_NAMELOOKUP2						185
#define EM_NSFDBHASPROFILENOTECHANGED		198
#define EM_NSFMARKREAD						208
#define EM_NSFADDTOFOLDER					209
#define	EM_NSFDBSPACEUSAGESCALED			210		/* V6 */
#define EM_NSFDBGETMAJMINVERSION			222		/* V5.09 */
#define EM_ROUTERJOURNALMESSAGE				223     /* V6 */

/* V6 SMTP hooks */
#define EM_SMTPCONNECT					224		
#define EM_SMTPCOMMAND					225
#define EM_SMTPMESSAGEACCEPT				226
#define EM_SMTPDISCONNECT				227
#define	EM_NSFARCHIVECOPYNOTES				228
#define	EM_NSFARCHIVEDELETENOTES		 	229
#define EM_NSFNOTEEXTRACTWITHCALLBACK		235
#define EM_NSFDBSTAMPNOTESMULTIITEM 		239
#define	EM_MEDIARECOVERY_NOTE				244

/*	These functions are not actually defined (thus their declarations are only
	in comments) but illustrate the arguments for the corresponding extension
	manager callouts.

	EM_ROUTERJOURNALMESSAGE occurs when the router has received a message that has been 
	marked to be journalled.

	STATUS LNPUBLIC JournalMessage(DBHANDLE hMailBoxHandle,
								   NOTEID   NoteID);

	EM_SETPASSWORD occurs when an ID file password is being set, either by
	a user or by administrator action.

	STATUS LNPUBLIC SetPassword (
						DWORD MaxPwdLen,	 Longest password you may supply 
						DWORD far *retLength, Return the length of the password
						char far *retPassword, Return the password here 

						char far *FileName,	 The name of the ID file 
						char far *OwnerName, The name of the owner of the ID file 

						DWORD DataLen,		 The old length of the extra ID info 
						BYTE far *Data,		 The old value of the extra ID info 

						DWORD MaxNewData,	 The max amount of extra ID info you may supply 
						DWORD far *retNewDataLen, Return the length of the new ID info 
						BYTE far *retNewData); Return the new ID info 

	EM_GETPASSWORD occurs when a user is about to be prompted for a password
	to decrypt and ID file.

	STATUS LNPUBLIC GetPassword (
						DWORD MaxPwdLen,	 Longest password you may supply 
						DWORD far *retLength, Return the length of the password here 
						char far *retPassword, Return the password here 

						char far *FileName,	 The name of the ID file 
						char far *OwnerName, The name of the owner of the ID file 

						DWORD DataLen,		 The length of the extra ID info 
						BYTE far *Data);	 The extra ID info 

	EM_CLEARPASSWORD occurs when a password is to be "cleared" either due to
	a timeout or because the user has pressed F5.

	STATUS LNPUBLIC ClearPassword ();


	STATUS LNPUBLIC ConflictHandler ( 
						HANDLE hDb,			Database Handle
						HANDLE hOldNote,	Original Note Handle
						HANDLE hNewNote,	New Note Handle
						DWORD  *pAction);	Conflict Action to take	

			*pAction returns:
				CONFLICT_ACTION_MERGE	- Have Notes try to merge
				CONFLICT_ACTION_HANDLE	- User handled the conflict	
				(zero)					- Proceed with Conflict in normal manner


	EM_ADMINPPROCESSREQUEST occurs prior to and after the Administration Process has
	processed a request on a server.

	STATUS LNPUBLIC ProcessRequest (
						NOTEHANDLE nhRequest, The handle of the Admin Request note 
						NOTEHANDLE nhResponse); The handle of the Admin Log note  


	EM_TERMINATENSF occurs when NSF service terminates for the process.

	void LNPUBLIC TerminateNSF (void *unused_params);


	EM_NSFNOTEUPDATEMAILBOX occurs when a NSFNoteUpdate is performed on 
	any and all mailbox databases (e.g. mail.box).  This is true even if
	multiple mailboxes are enabled in the server configuration document.
	The arguments are identical to those used for EM_NSFNOTEUPDATE.
	

	EM_NIFOPENNOTE opens a note by index position and optionally navigates.
	It is used by the Notes Editor.

	STATUS LNPUBLIC NIFOpenNote (
						HCOLLECTION hCollection,
						COLLECTIONPOSITION far *IndexPos,
						WORD Navigator,
						WORD FallbackNavigator,
						NOTEID NoteID,
						DWORD OpenFlags,
						INDEXSPECIALINFO far *retIndexInfo,
						HANDLE far *rethNote);


	EM_NSFNOTEOPENEXTENDED is a Notes internal, extended form of NSFNoteOpen.

	STATUS LNPUBLIC NSFNoteOpenExtended (
						DBHANDLE hDB,
						NOTEID NoteID,
						DWORD flags, 
						DWORD SinceSeqNum,
						BYTE *pKey,
						HANDLE *rtn);


	EM_MAILSENDNOTE is called when the Mailer sends an open note to recipients
	listed in the note's header items.

	STATUS LNPUBLIC MailSendNote (
						HANDLE			hNote,
						void			*internalViewDesc, 
						WORD			Flags, 
						BOOL			*Modified,
						void			*SendNoteCtx);


	EM_NSFMARKREAD is called when a note is opened by the Notes client and marked READ.
	Note that this does not include other mechanisms that could result in a note being
	marked read, such as being read by the client on another replica.

	STATUS LNPUBLIC NSFMarkRead(
							DBHANDLE	hDB,			- Handle of database containing note being marked
							HANDLE		hNote,			- Handle to note being marked read
							NOTEID		NoteID);		- NoteID of the note being marked read


	EM_NSFADDTOFOLDER is called when a note is being added or removed 
	from a folder.  The IsAddOperation flag should be checked to determine
	if this note is being added or removed from the folder.

	STATUS LNPUBLIC NSFAddToFolder( 
							DBHANDLE	hViewDB,		- Handle of database containing folder
							DBHANDLE	hDataDB,		- Handle of database containing notes being added to folder
							NOTEID		FolderNoteID,	- NoteID of the folder note
							NOTEID		NoteID,			- NoteID of the note being added to (removed from) the folder
							UNID		*NoteUNID,		- UNID of the note being added to (removed from) the folder
							BOOL		IsAddOperation,	- TRUE if note being added to the folder, FALSE if note being removed
							TIMEDATE	*RevisionTime);	- Time of original folder addition (OPTIONAL - may be NULL)

	EM_SMTPCONNECT is called when an inbound SMTP connection has been detected.
	
	The Extension Manager EM_BEFORE notification type for the EM_SMTPCONNECT event occurs when an 
	inbound SMTP connection has been detected and prior to the execution of the internal Domino SMTP 
	restriction controls.  Callback routine can implement their own anti-relay checks and bypass 
	Domino related checks through the use of PossibleRelay BOOL and return status of value NOERROR.  
	Return STATUS other than ERR_EM_CONTINUE or NOERROR sets AccessDenied flag which causes subsequent 
	commands to be rejected.  

	The Extension Manager EM_AFTER notification type for the EM_SMTPCONNECT event occurs after the SMTP 
	listener task has accepted the connection but prior to sending the SMTP greeting to the connecting host.

	STATUS LNPUBLIC SMTPConnect(
							DWORD	SessionID,			- Unique session identifier
							char	*RemoteIP,			- NULL terminated string containing IP address of connecting host
							char	*RemoteHost,		- NULL terminated string containing host name of connecting host if reverse DNS 
														lookup was successful.  If lookup was unsuccessful, the string length will be zero.
							BOOL	&PossibleRelay, 	- Indicator whether connecting host should be treated as possible relay or not
							char	*Greeting,			- Greeting that will be returned to the connecting host
														- Greeting is NULL during EM_BEFORE notification
							DWORD	MaxGreetingLength);	- Size of buffer allocated to modify Greeting

	EM_SMTPCOMMAND is called whenever a SMTP command has been received by the SMTP task.
	
	The Extension Manager EM_BEFORE notification type for the EM_SMTPCOMMAND event occurs whenever a SMTP 
	command has been received by the SMTP listener task but prior to the parsing of the command. 
	
	Domino allocates a buffer that can be used by callback routines for the EM_BEFORE notification to modify 
	the command and thus change internal Domino processing.  NOERROR return status indicates to skip parsing 
	and execution of command.  The default reply when STATUS is NOERROR is "250 OK".  Return STATUS values 
	other than ERR_EM_CONTINUE and NOERROR from the EM_BEFORE notification results in the command being 
	rejected.  A default error message will be generated by the Domino SMTP server, which can be modified by 
	the callback routine for the EM_AFTER notification.  STATUS of ERR_EM_CONTINUE will continue normal Domino 
	processing.
	
	SMTP response to the command entered can be modified during the callback routine of EM_AFTER notification.  
	Care must be taken not to change the reply code from success to failure or vice-versa as this will cause 
	the sender-SMTP and receiver-SMTP servers to be out of synch.  Domino supplies a buffer that can 
	be used by the callback routine to change the SMTP response.

	STATUS LNPUBLIC SMTPCommand(
							DWORD	SessionID,			- Unique session identifier
							char	*Command,			- NULL terminated string containing SMTP command and arguments received
							DWORD	MaxCommandLength,	- Size of buffer allocated to modify Command
							char	*SMTPReply,			- SMTP response that will be returned to the connecting host
														- SMTPReply is NULL during EM_BEFORE notification
							DWORD 	SMTPReplyLength);	- Size of buffer allocated to modify SMTPReply

	EM_SMTPMESSAGEACCEPT is called following the receipt of "end of mail data indicator", a line containing a single
	period, and itemization of the MIME stream into an in-memory note.

	Following the receipt of the "end of mail data indicator", the resulting stream is itemized to an 
	in-memory note.  The Extension Manager EM_BEFORE notification type for the EM_SMTPMESSAGEACCEPT event 
	occurs following itemization but prior to adding the note to the mailbox.  If the callback routine 
	returns STATUS of NOERROR, the Domino SMTP server will stop further processing of the message.  By default
	a success response is generated by core Domino code when NOERROR STATUS has been returned. It is also 
	possible for the callback routine to intercept the message and deposit to database where scans can be 
	performed.  The Callback routine can make changes to the note in the EM_BEFORE event but should not 
	attempt to close the note as this is done by the core code.  

	The Extension Manager EM_AFTER notification type for the EM_SMTPMESSAGEACCEPT event occurs after the SMTP 
	listener task attempted to submit the message to the mailbox but prior to sending a reply.  The Callback 
	routines for the EM_AFTER notification can change the reply returned to the connecting host however, care 
	must be taken not to change the reply code from success to error or vice-versa as this would cause the 
	sender-SMTP and receiver-SMTP servers to be out of synch.

	STATUS LNPUBLIC SMTPMessageAccept(
							DWORD	SessionID,			- Unique session identifier
							NOTEHANDLE	Note,			- Note containing the itemized message
							char	*SMTPReply,			- SMTP Response that will be returned to the connecting host
														- SMTPReply is NULL during EM_BEFORE notification
							DWORD	SMTPReplyLength);	- Size of buffer allocated to modify SMTPReply

	EM_SMTPDISCONNECT is called when a SMTP connection is being torn down.  This includes normal and abnormal disconnects,
	such as when the QUIT command is issued or when a session times out.

	STATUS LNPUBLIC SMTPDisconnect(
							DWORD	SessionID);			- Unique session identifier


	The following 2 archive calls are NOT defined

	EM_NSFARCHIVEDELETENOTES is called after documents have been selected and copied to
	the destination db if desired. It is time for archiving to delete the archived notes
	that qualify from the source database 

   	STATUS LNPUBLIC NSFArchiveDeleteNotes( 
			DBHANDLE 	hDB, 			- handle of the source db where notes will be deleted from
			HANDLE 		hTempIDTable, 	- handle of the id table containing the notes ids to be deleted
			DWORD 		dwFlags ); 		- none at this time
			

	EM_NSFARCHIVECOPYNOTES is called after documents have been selected  for archiving and
	copies them from the source db to the specified destination db  

 	STATUS LNPUBLIC NSFArchiveCopyNotes(
 		DBHANDLE hSrcDB,  		  - dbhandle of the source db
 		DBHANDLE hDestDB, 		  - dbhandle of the destination db
 		HANDLE   hIdTable, 		  - handle to the id table containing the note ids to be copied to the dest
 		DWORD    dwFlags, 		  - none at this time
 		TIMEDATE *ptdSrcMod, 	  - pointer to a timedate for last modified time of source db
 		REPLFILESTATS *pStats);   - pointer to a REPLFILESTATS strcuture containing the stats of the copy


	The following functions ARE defined, but their extension manager callouts
	have slightly different signatures than the standard mechanism because of
	the fact that the function returns something other than STATUS.  The
	extension manager callout has an added argument which allows the callout
	to write the return value of the API routine.  For purposes of the callout,
	it is as if the function signatures were as follows.

	STATUS LNPUBLIC AgentIsEnabled(HAGENT hAgent, BOOL *return_value);
  
	EM_SECAUTHENTICATION is called on the server after the Notes Authentication
	succeeds but before the session is opened. 

	STATUS LNPUBLIC Authentication  (WORD		wEvent
									,SESSIONID	SessionId
									,char		far * pRemoteName
									,DWORD		dwFlags
									,WORD		wNetProtocol
									,char		far * NetAddress
									,void 		far * vpNull
									)



	This function is called every time a note is encountered during database media recovery.  The database being recovered can
		not be changed in any way but the note can be, for example, copied off to a different database.  This can be used, for example,
		for salavaging the content of notes which are going to be deleted at a later point in the media recovery process.

	STATUS LNPUBLIC NSFDbMediaRecoveryNote(DBHANDLE hDB, HANDLE hNote, BOOL IsInsert, BOOL IsUpdate, BOOL IsDelete)

*/
/* Authentication Extension Manager flags
*/
#define fAuthRoleServer					((DWORD) 0x00000001)
#define fAuthRolePassthruServer			((DWORD) 0x00000002)
#define fAuthClientViaPassthruServer	((DWORD) 0x00000004)

/* Authentication Extension Manager events
*/
#define AUTHEM_StartAuthentication		((WORD) 0x0000)
#define AUTHEM_Poll						((WORD) 0x0001)
#define AUTHEM_Identify					((WORD) 0x0002)
#define AUTHEM_Terminate				((WORD) 0x0003)

#define MAXAUTHEMNAMELENGTH	1024

#ifdef __cplusplus
}
#endif

#endif /* EXMGR_DEFS */


#if defined(OS400) && (__OS400_TGTVRM__ >= 510)
#pragma datamodel(pop)
#endif


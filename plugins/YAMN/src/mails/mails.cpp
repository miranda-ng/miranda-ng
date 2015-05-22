/*
 * This code implements retrieving info from MIME header 
 *
 * (c) majvan 2002-2004
 */

#include "../yamn.h"

//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

// SMALL INTRO
// Mails are queued in a queue (chained list). Pointer to first mail is pointed from Account structure
// member called Mails.
// Mail queue is ended with NULL- pointered mail (NULL handle)

//Creates new mail for plugin (calling plugin's constructor, when plugin imported to YAMN)
INT_PTR CreateAccountMailSvc(WPARAM wParam,LPARAM lParam);

//Deletes mail for plugin (calling plugin's destructor, when plugin imported to YAMN)
INT_PTR DeleteAccountMailSvc(WPARAM wParam,LPARAM lParam);

//Loads mail data from standard storage to memory
INT_PTR LoadMailDataSvc(WPARAM wParam,LPARAM lParam);

//Deletes mail data from memory
INT_PTR UnloadMailDataSvc(WPARAM wParam,LPARAM);

//Saves mail data from memory to standard storage
INT_PTR SaveMailDataSvc(WPARAM wParam,LPARAM lParam);

//Appends second MIME mail queue to the first one
//Only finds the end of first queue and its Next memember repoints to second one
void WINAPI AppendQueueFcn(HYAMNMAIL first,HYAMNMAIL second);

//Synchronizes two accounts
//Function finds, if there were some mails deleted from mailbox and deletes (depends on RemovedOld param) them from OldQueue
//Next finds, if there are new mails. Mails that are still on mailbox are deleted (depends on RemovedNew param) from NewQueue
//After this, OldQueue is pointer to mails that are on mailbox, but not new mails
//and NewQueue contains new mails in account
//New accounts can be then appended to account mails queue, but they have set the New flag
//
//Two mails equals if they have the same ID
//
// hPlugin- handle of plugin going to delete mails
// OldQueue- queue of mails that we found on mailbox last time, after function finishes queue contains all mails except new ones
// RemovedOld- queue of mails where to store removed mails from OldQueue, if NULL deletes mails from OldQueue
// NewQueue- queue of mails that we found on mailbox (all mails), after function finishes queue contains only new mails
// RemovedNew- queue of mails where to store removed mails from NewQueue, if NULL deletes mails from NewQueue
//So function works like:
//1. delete (or move to RemovedOld queue if RemovedOld is not NULL) all mails from OldQueue not found in NewQueue
//2. delete (or move to RemovedNew queue if RemovedNew is not NULL) all mails from NewQueue found in OldQueue
void WINAPI SynchroMessagesFcn(HACCOUNT Account,HYAMNMAIL *OldQueue,HYAMNMAIL *RemovedOld,HYAMNMAIL *NewQueue,HYAMNMAIL *RemovedNew);

//Deletes messages from mail From to the end
// Account- account who owns mails
// From- first mail in queue, which is going to delete
void WINAPI DeleteMessagesToEndFcn(HACCOUNT Account,HYAMNMAIL From);

//Removes message from queue, does not delete from memory
// From- queue pointer
// Which- mail to delete
// mode- nonzero if you want to decrement numbers in messages that are bigger than the one in Which mail, 0 if not
void WINAPI DeleteMessageFromQueueFcn(HYAMNMAIL *From,HYAMNMAIL Which,int mode);

//Finds message in queue that has the same ID number
// From- message queue
// ID- pointer to ID
// returns pointer to found message, NULL if not found
HYAMNMAIL WINAPI FindMessageByIDFcn(HYAMNMAIL From,char *ID);

//Translate header from text to queue of CMimeItem structures
//This means that new queue will contain all info about headers
// stream- pointer to text containing header (can be ended with zero)
// len- length of stream
// head- function fills this pointer to first header item in queue
void WINAPI TranslateHeaderFcn(char *stream,int len,struct CMimeItem **head);

//Creates new mail queue, copying only these mails, that have set flag for deleting
// From- message queue, whose mail with given flag are duplicated
// returns new mail queue (or NULL when no mail with flag is in From queue)
//Function does not copy the whole mails, it copies only ID string. And ID is copied as string, so
//you can use this fcn only if you have your ID as pointer to char string ended with zero character
HYAMNMAIL WINAPI CreateNewDeleteQueueFcn(HYAMNMAIL From);

//Sets/removes flags from specific mails
// From- pointer to first message
// FlagsSet- mail must have set these flags...
// FlagsNotSet- ...and must not have set these flags...
// FlagsToSetRemove- ...to set/remove these flags (see mode)
// mode- nonzero to set, else remove
void WINAPI SetRemoveFlagsInQueueFcn(HYAMNMAIL From,DWORD FlagsSet,DWORD FlagsNotSet,DWORD FlagsToSetRemove,int mode);

struct CExportedFunctions MailExportedFcn[]=
{
	{YAMN_SYNCHROMIMEMSGSID,(void *)SynchroMessagesFcn},
	{YAMN_TRANSLATEHEADERID,(void *)TranslateHeaderFcn},
	{YAMN_APPENDQUEUEID,(void *)AppendQueueFcn},
	{YAMN_DELETEMIMEQUEUEID,(void *)DeleteMessagesToEndFcn},
	{YAMN_DELETEMIMEMESSAGEID,(void *)DeleteMessageFromQueueFcn},
	{YAMN_FINDMIMEMESSAGEID,(void *)FindMessageByIDFcn},
	{YAMN_CREATENEWDELETEQUEUEID,(void *)CreateNewDeleteQueueFcn},
	{YAMN_SETREMOVEQUEUEFLAGSID,(void *)SetRemoveFlagsInQueueFcn},
};

struct CExportedServices MailExportedSvc[]=
{
	{MS_YAMN_CREATEACCOUNTMAIL,CreateAccountMailSvc},
	{MS_YAMN_DELETEACCOUNTMAIL,DeleteAccountMailSvc},
	{MS_YAMN_LOADMAILDATA,LoadMailDataSvc},
	{MS_YAMN_UNLOADMAILDATA,UnloadMailDataSvc},
	{MS_YAMN_SAVEMAILDATA,SaveMailDataSvc},
};


//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------

INT_PTR CreateAccountMailSvc(WPARAM wParam,LPARAM lParam)
{
	HACCOUNT Account=(HACCOUNT)wParam;
	DWORD MailVersion=(DWORD)lParam;
	HYAMNMAIL NewMail;

//test if we are going to initialize members of suitable structure (structures of plugin and YAMN must match)
	if (MailVersion != YAMN_MAILVERSION)
		return NULL;

	if (Account->Plugin != NULL)
	{
		if (Account->Plugin->MailFcn->NewMailFcnPtr != NULL)
		{
//Let plugin create its own structure, which can be derived from CAccount structure
			if (NULL==(NewMail=Account->Plugin->MailFcn->NewMailFcnPtr(Account,YAMN_MAILVERSION)))
				return NULL;
		}
		else
		{
//We suggest plugin uses standard CAccount structure, so we create it
			if (NULL==(NewMail=new YAMNMAIL))
//If not created successfully
				return NULL;
			NewMail->MailData=NULL;
		}
//Init every members of structure, used by YAMN
		return (INT_PTR)NewMail;
	}
	return NULL;
}

INT_PTR DeleteAccountMailSvc(WPARAM wParam,LPARAM lParam)
{
	HYAMNPROTOPLUGIN Plugin=(HYAMNPROTOPLUGIN)wParam;
	HYAMNMAIL OldMail=(HYAMNMAIL)lParam;
	struct CMimeItem *TH;

	if (Plugin->MailFcn != NULL) {
		if (Plugin->MailFcn->DeleteMailFcnPtr != NULL) {
			//Let plugin delete its own CMimeMsgQueue derived structure
			Plugin->MailFcn->DeleteMailFcnPtr(OldMail);
			return 1;
		}
	}
	if (OldMail->MailData != NULL) {
		if (OldMail->MailData->Body != NULL)
			delete[] OldMail->MailData->Body;
		if ((TH=OldMail->MailData->TranslatedHeader) != NULL)
			for (;OldMail->MailData->TranslatedHeader != NULL;) {
				TH=TH->Next;
				if (OldMail->MailData->TranslatedHeader->name != NULL)
					delete[] OldMail->MailData->TranslatedHeader->name;
				if (OldMail->MailData->TranslatedHeader->value != NULL)
					delete[] OldMail->MailData->TranslatedHeader->value;
				delete OldMail->MailData->TranslatedHeader;
				OldMail->MailData->TranslatedHeader=TH;
			}
		delete OldMail->MailData;
	}
	if (OldMail->ID != NULL)
		delete[] OldMail->ID;

	delete OldMail;				//consider mail as standard HYAMNMAIL, not initialized before and use its own destructor
	return 1;
}


void WINAPI AppendQueueFcn(HYAMNMAIL first,HYAMNMAIL second)
{
	HYAMNMAIL Finder=first;
	while(Finder->Next != NULL) Finder=Finder->Next;
	Finder->Next=second;
}

INT_PTR LoadMailDataSvc(WPARAM wParam,LPARAM lParam)
{
	HYAMNMAIL Mail=(HYAMNMAIL)wParam;
	DWORD MailVersion=(DWORD)lParam;

	if (MailVersion != YAMN_MAILDATAVERSION)
		return NULL;

//now we have all data to memory persisting, so no loading is needed
	return (INT_PTR)Mail->MailData;
}

INT_PTR UnloadMailDataSvc(WPARAM wParam,LPARAM)
{
	HYAMNMAIL Mail=(HYAMNMAIL)wParam;

//now we should delete structure from memory, but it will be made in future YAMN version
	return 1;
}

INT_PTR SaveMailDataSvc(WPARAM wParam,LPARAM lParam)
{
	HYAMNMAIL Mail=(HYAMNMAIL)wParam;
	DWORD MailVersion=(DWORD)lParam;

	if (MailVersion != YAMN_MAILDATAVERSION)
		return (INT_PTR)-1;

//now we have all data to memory persisting, so no saving is needed
	return (INT_PTR)0;
}

void WINAPI SynchroMessagesFcn(HACCOUNT Account,HYAMNMAIL *OldQueue,HYAMNMAIL *RemovedOld,HYAMNMAIL *NewQueue,HYAMNMAIL *RemovedNew)
//deletes messages from new queue, if they are old
//it also deletes messages from old queue, if they are not in mailbox anymore
//"YAMN_MSG_DELETED" messages in old queue remain in old queue (are never removed, although they are not in new queue)
//"YAMN_MSG_DELETED" messages in new queue remain in new queue (are never removed, although they can be in old queue)
{
	HYAMNMAIL Finder,FinderPrev;
	HYAMNMAIL Parser,ParserPrev;
	HYAMNMAIL RemovedOldParser =NULL;
	HYAMNMAIL RemovedNewParser =NULL;
	if (RemovedOld != NULL) *RemovedOld=NULL;
	if (RemovedNew != NULL) *RemovedNew=NULL;

	for (FinderPrev=NULL,Finder=*OldQueue;Finder != NULL;)
	{
		if (Finder->Flags & YAMN_MSG_DELETED)			//if old queue contains deleted mail
		{
			FinderPrev=Finder;
			Finder=Finder->Next;						//get next message in old queue for testing
			continue;
		}
		for (ParserPrev=NULL,Parser=*NewQueue;Parser != NULL;ParserPrev=Parser,Parser=Parser->Next)
		{
			if (Parser->Flags & YAMN_MSG_DELETED)
				continue;

			if (Parser->ID==NULL)						//simply ignore the message, that has not filled its ID
				continue;

			if (0==mir_strcmp(Parser->ID,Finder->ID))		//search for equal message in new queue
				break;
		}
		if (Parser != NULL)								//found equal message in new queue
		{
			if (Parser==*NewQueue)
				*NewQueue=(*NewQueue)->Next;
			else
				ParserPrev->Next=Parser->Next;
			Finder->Number=Parser->Number;				//rewrite the number of current message in old queue

			if (RemovedNew==NULL)						//delete from new queue
				DeleteAccountMailSvc((WPARAM)Account->Plugin,(LPARAM)Parser);
			else										//or move to RemovedNew
			{
				if (RemovedNewParser==NULL)				//if it is first mail removed from NewQueue
					*RemovedNew=Parser;					//set RemovedNew queue to point to first message in removed queue
				else
					RemovedNewParser->Next=Parser;		//else don't forget to show to next message in RemovedNew queue
				RemovedNewParser=Parser;				//follow RemovedNew queue
				RemovedNewParser->Next=NULL;
			}
			FinderPrev=Finder;
			Finder=Finder->Next;						//get next message in old queue for testing
		}
		else											//a message was already deleted from mailbox
		{
			if (Finder==*OldQueue)						//if we are at the first item in OldQueue
			{
				*OldQueue=(*OldQueue)->Next;			//set OldQueue to next item
				if (RemovedOld==NULL)					//delete from old queue
					DeleteAccountMailSvc((WPARAM)Account->Plugin,(LPARAM)Finder);
				else									//or move to RemovedOld
				{
					if (RemovedOldParser==NULL)			//if it is first mail removed from OldQueue
						*RemovedOld=Finder;				//set RemovedOld queue to point to first message in removed queue
					else
						RemovedOldParser->Next=Finder;	//else don't forget to show to next message in RemovedNew queue
					RemovedOldParser=Finder;			//follow RemovedOld queue
					RemovedOldParser->Next=NULL;
				}
				Finder=*OldQueue;
			}
			else
			{
				FinderPrev->Next=Finder->Next;
				if (RemovedOld==NULL)					//delete from old queue
					DeleteAccountMailSvc((WPARAM)Account->Plugin,(LPARAM)Finder);
				else									//or move to RemovedOld
				{
					if (RemovedOldParser==NULL)			//if it is first mail removed from OldQueue
						*RemovedOld=Finder;				//set RemovedOld queue to point to first message in removed queue
					else
						RemovedOldParser->Next=Finder;	//else don't forget to show to next message in RemovedNew queue
					RemovedOldParser=Finder;			//follow RemovedOld queue
					RemovedOldParser->Next=NULL;
				}
				Finder=FinderPrev->Next;
			}
		}
	}
}

void WINAPI DeleteMessagesToEndFcn(HACCOUNT Account,HYAMNMAIL From)
{
	HYAMNMAIL Temp;
	while(From != NULL)
	{
		Temp=From;
		From=From->Next;
		DeleteAccountMailSvc((WPARAM)Account->Plugin,(LPARAM)Temp);
	}
}

void WINAPI DeleteMessageFromQueueFcn(HYAMNMAIL *From,HYAMNMAIL Which,int mode=0)
{
	DWORD Number=Which->Number;
	HYAMNMAIL Parser;

	if (*From==Which)
	{
		Parser=Which->Next;
		*From=Parser;
	}
	else 
	{
		for (Parser=*From;Which != Parser->Next;Parser=Parser->Next)
			if (mode && (Parser->Number>Number)) Parser->Number--;
		if (mode && (Parser->Number>Number)) Parser->Number--;
		Parser->Next=Parser->Next->Next;
		Parser=Which->Next;
	}	
	if (mode)
		for (;Parser != NULL;Parser=Parser->Next)
			if (Parser->Number>Number) Parser->Number--;
}

void DeleteMessagesFromQueue(HYAMNMAIL *From,HYAMNMAIL Which,int mode=0)
{
	HYAMNMAIL Parser;

	for (Parser=Which;Parser != NULL;Parser=Parser->Next)
		DeleteMessageFromQueueFcn(From,Parser,mode);
}

HYAMNMAIL WINAPI FindMessageByIDFcn(HYAMNMAIL From,char *ID)
{
	HYAMNMAIL Browser;

	for (Browser=From;Browser != NULL;Browser=Browser->Next)
		if (0==mir_strcmp(Browser->ID,ID))
			break;
	return Browser;
}

void WINAPI TranslateHeaderFcn(char *stream,int len,struct CMimeItem **head)
{
	try
	{
		char *finder=stream;
		char *prev1,*prev2,*prev3;
		struct CMimeItem *Item=NULL;

		while(finder<=(stream+len))
		{
			while(ENDLINEWS(finder)) finder++;

			//at the start of line
			if (DOTLINE(finder+1))					//at the end of stream
				break;

			prev1=finder;

			while(*finder != ':' && !EOS(finder)) finder++;
			if (!EOS(finder))
				prev2=finder++;
			else
				break;

			while(WS(finder) && !EOS(finder)) finder++;
			if (!EOS(finder))
				prev3=finder;
			else
				break;

			do
			{
				if (ENDLINEWS(finder)) finder+=2;						//after endline information continues
				while(!ENDLINE(finder) && !EOS(finder)) finder++;
			}while(ENDLINEWS(finder));

			if (Item != NULL)
			{
				if (NULL==(Item->Next=new struct CMimeItem))
					break;
				Item=Item->Next;
			}
			else
			{
				Item = new CMimeItem;
				*head = Item;
			}

			Item->Next=NULL;
			Item->name=new char [prev2-prev1+1];
			mir_strncpy(Item->name,prev1,prev2-prev1+1);
			Item->value=new char [finder-prev3+1];
			mir_strncpy(Item->value,prev3,finder-prev3+1);

			if (EOS(finder))
				break;
			finder++;
			if (ENDLINE(finder)) {
				finder++;
				if (ENDLINE(finder)) {
					// end of headers. message body begins
					finder++;
					if (ENDLINE(finder))finder++;
					prev1 = finder;
					while (!DOTLINE(finder+1))finder++;
					if (ENDLINE(finder))finder--;
					prev2 = finder;
					if (prev2>prev1) { // yes, we have body
						if (NULL==(Item->Next=new struct CMimeItem))	break; // Cant create new item?!
						Item=Item->Next;
						Item->Next=NULL;//just in case;
						Item->name=new char[5]; strncpy(Item->name,"Body",5);
						Item->value=new char [prev2-prev1];
						mir_strncpy(Item->value,prev1,prev2-prev1-1);
					}
					break; // there is nothing else
				}
			}
		}
	}
	catch(...)
	{
		MessageBoxA(NULL,"Translate header error","",0);
	}
}

HYAMNMAIL WINAPI CreateNewDeleteQueueFcn(HYAMNMAIL From)
{
	HYAMNMAIL FirstMail,Browser;

	for (FirstMail=NULL;From != NULL;From=From->Next)
	{
		if ((From->Flags & (YAMN_MSG_USERDELETE | YAMN_MSG_AUTODELETE)) && !(From->Flags & YAMN_MSG_DELETED))
		{
			if (FirstMail==NULL)
			{
				FirstMail=Browser=new YAMNMAIL;
				if (FirstMail==NULL)
					break;
			}
			else
			{
				Browser->Next=new YAMNMAIL;
				Browser=Browser->Next;
			}
			Browser->ID=new char[mir_strlen(From->ID)+1];
			strcpy(Browser->ID,From->ID);
			Browser->Number=From->Number;
			Browser->Flags=From->Flags;
		}
	}
	return FirstMail;
}

void WINAPI SetRemoveFlagsInQueueFcn(HYAMNMAIL From,DWORD FlagsSet,DWORD FlagsNotSet,DWORD FlagsToSetRemove,int mode)
{
	HYAMNMAIL msgq;

	for (msgq=(HYAMNMAIL)From;msgq != NULL;msgq=msgq->Next)
	{
		if ((FlagsSet==(msgq->Flags & FlagsSet)) && (0==(msgq->Flags & FlagsNotSet)))
		{
			if (mode)
				msgq->Flags=msgq->Flags | FlagsToSetRemove;
			else
				msgq->Flags=msgq->Flags & ~FlagsToSetRemove;
		}
	}
}

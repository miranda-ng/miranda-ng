==================================================================================
= YAMN plugin for Miranda (short readme for developers)                          =
==================================================================================
Hello developer! :)
I hope YAMN will give you what you find, but you can also improve YAMN.

This readme gives you some info about YAMN. Please read it first before you are 
going to look at YAMN sources.

YAMN provides two types of plugins now: protocol plugins and filter plugins.


1.	What do you need to make your protocol plugin cooperating with YAMN
	-------------------------------------------------------------------

	If you want to cooperate with YAMN, you have to do some things. YAMN offers you some services,
	so your work is easier, but YAMN needs some things to be done for proper work. These limits go
	according thread synchronization and memory mutual exclusion.

	YAMN  offers  you  two  types  of services. Exported functions and Miranda services. Miranda
	services  are  described in header files, exported functions are described in cpp files. All
	exported  functions  in  YAMN  have  the  suffix Fcn, so you can easy get if the function is
	exported.  Using exported functions is more difficult than using miranda services, but after
	solving some definitions, the work with exported functions is more clear and easier. Miranda
	services  from  YAMN  are for miscellaneus functions. The fact Miranda service uses only two
	parameters  and  therefore is sometimes very unsuitable leads us to make exported functions.
	Exported  functions  are  divided in several parts: synchronizing functions (used for thread
	and account synchronization) and MIME functions (used to work with MIME 
	messages).

	Miranda services are used through Miranda CallService function. YAMN exported functions are avialable
	when registering plugin. Then YAMN gives you its table of exported functions.

	How  to  write  write  your  protocol  plugin  for  YAMN? The best way for you is to look at
	internal POP3 protocol, where all info about this is written. At start, you need to register
	plugin  (it  is  done in two steps- registering and inserting to YAMN), then get pointers to
	YAMN's  exported  functions  (using  Miranda's service MS_YAMN_GETFCN) you will need in your
	protocol  plugin.  These  are the first steps you should do when implementing some plugin to
	YAMN.  Next,  you  should  know  how  YAMN is stuctured. Structures of YAMN are described in
	chapter  2.  And, at the end, you should know something about account synchronizing and some
	limitations you have to achieve, if you want your plugin works well.


2.	YAMN structures and memory organization
	---------------------------------------

	YAMN  uses  its  own  structures, that can change in the future. The problem with change is,
	that  there  can occur some incomapatibilities between YAMN and plugins written for old YAMN
	versions.  To  avoid  problems,  YAMN  defines  versions  for  services or exported/imported
	functions, where strucutre version information is passed to/from plugins.


2.1.	Structures of protcol plugin queue

        (PYAMN_PROTOPLUGINQUEUE)FirstPlugin---> =(HYAMNPROTOPLUGIN)=  ---> =(HYAMNPROTOPLUGIN)=  ---> =(HYAMNPROTOPLUGIN)=  ---> NULL
                                                |                  |  |    |                  |  |    |                  |  |
                                                |         .        |  |    |         .        |  |    |         .        |  |
                                                |         .        |  |    |         .        |  |    |         .        |  |
                                                |         .        |  |    |         .        |  |    |         .        |  |
                                                --------------------  |    |------------------|  |    |------------------|  |
                                                |             Next |--|    |             Next |--|    |             Next |--|
                                                ====================       ====================       ====================

        This  structure  is not needed if you only create protocol plugin for YAMN. YAMN plugin does
        not see and it is not important for it how YAMN works with plugins and how it stores plugins
        data.   For   plugin   is   important   only  handle  for  its  own  plugin,  returned  from
        MS_YAMN_REGISTERPLUGIN service.


2.2.	Structure of accounts

	Every  account  in  YAMN  belongs  to  exact  plugin  and  its  members  are  allocated with
	MS_YAMN_CREATEPLUGINACCOUNT  service.  This  service cooperates with your function, which is
	defined in your function import table. In your function (if you have defined it), you should
	create  the whole account. It is because YAMN cannot know which members in structure did you
	add.  So  you  create  the  whole derived structure. If your fcn is not implemented (NULL in
	import table), YAMN creates standard account structure.

	This  structure contains information (members) related to YAMN, to plugin and members shared
	between both (plugin and YAMN). Therefore it is needed to synchronize access to members (see
	Ch.  3).  Standard  YAMN  account  is  defined  in  m_account.h  header  file.  There's also
	description  for every member how it is synchronised. YAMN creates two synchronizing objects
	(SO)  to  synchronise access to members. In m_synchro.h file, there are definitions for easy
	work with these SO.

	Accounts are queued in plugin:

        =(HYAMNPLUGIN)=  ---> ===(HACCOUNT)===  ---> ===(HACCOUNT)===  ---> ===(HACCOUNT)===  ---> NULL
        |             |  |    |              |  |    |              |  |    |              |  |
        |             |  |    |              |  |    |              |  |    |              |  |
        |      .      |  |    |              |  |    |              |  |    |              |  |
        |      .      |  |    |              |  |    |              |  |    |              |  |
        |      .      |  |    |              |  |    |              |  |    |              |  |
        |             |  |    |--------------|  |    |--------------|  |    |--------------|  |
        | (HACCOUNT)  |  |    |         Next |--|    |         Next |--|    |         Next |--|
        | FirstAccount|--|    ================       ================       ================
        |-------------|
        |             |
        ===============

        Every  account  has  its own back pointer to (HYAMNPLUGIN) in Plugin member, so you can easy
        look at first account, when you have any other account (see m_account.h).


2.3.	Structure of mails

	Account  has a pointer to mails. Account's pointer to mails is pointer to first mail in fact
	and mails are queued too:
        
        ==(HACCOUNT)==  ---> ==(HYAMNMAIL)==  ---> ==(HYAMNMAIL)==  ---> ==(HYAMNMAIL)==  ---> NULL
        |            |  |    |             |  |    |             |  |    |             |  |
        |      .     |  |    |             |  |    |             |  |    |             |  |
        |      .     |  |    |             |  |    |             |  |    |             |  |
        |      .     |  |    |             |  |    |             |  |    |             |  |
        |            |  |    |-------------|  |    |-------------|  |    |-------------|  |
        | (HYAMNMAIL)|  |    |        Next |--|    |        Next |--|    |        Next |--|
        |       Mails|--|    ===============       ===============       ===============
        |------------|
        |            |
	==============

	Standard MIME mail is defined in mails/m_mails.h file. 

	Plugin  can  work with accounts in its own way, but note it is needed to synchronize access.
	For  better  work,  YAMN  offers  you  some  services  and exports functions. Description of
	exported functions is in its declartation; for accounts functions see account.cpp, for mails
	functions see mails/mails.cpp and so on.


3.	YAMN thread synchronization
	---------------------------

	Because  YAMN  is  multithreaded,  more  than one thread can access to any member of account
	structure.  Therefore  access to these members should be synchronised. YAMN offers two types
	of   synchronization  objects  (SO):  SCOUNTER  (Synchronized  Counter)  and  SWMRG  (Single
	Writer/Multiple Readers Guard). To use these objects, you can use exported functions:

	SWMRG: WaitToWriteSO, WaitToWriteSOEx, WriteDoneSO, WaitToReadSO, WaitToReadSOEx, ReadDoneSO
	SCOUNTER: SCGetNumber, SCInc, SCDec

	To  see  description  for these functions, see m_synchro.h header file and synchro.cpp. Note
	that  in HACCOUNT structure, there are 3 synchronizing members, which you have to use if you
	want to access to any member of account structure. All access techniques (writing to members
	and  read  from members) are used in POP3 protocol plugin. Now, it is important what we have
	to  do  when  we  want  to make our plugin be synchronized with YAMN (in POP3 protocol it is
	described too).

	1. We have to use ThreadRunningEV event when YAMN calls our checking/deleting function. This
	parameter  is to stop YAMN called thread until we do not have copied datas from stack. After
	that, we SetEvent(ThreadRunningEvent) to unblock YAMN to continue in its work.

	2.  We  have  to  use  UsingThreads account's member. This is only for YAMN account deleting
	prevention. We use this counter to set number of threads using account. If no thread is just
	using  account,  account  is  signaled, that it can be deleted (and is deleted when needed).
	This leads us to do some things: We use SCInc(UsingThreads) as the first thing we can do. We
	cannot  omit,  that called thread finished before we call this function. UsingThreads should
	have  "continuous"  value  greater than zero when using account. E.g. if YAMN creates thread
	for  plugin  that  checks account for new mail, YAMN waits until we set ThreadRunningEV (see
	point  1).  After  setting  this  event  to  signal,  that YAMN can continue in its work, we
	increase  SCInc(UsingThreads),  so  we  ensure  that another thread uses account before YAMN
	thread,  that uses this account ends. And SCDec(UsingThreads) should be the last thing we do
	in  our  thread.  If  we  run another thread in our thread, we should wait until it does not
	SCInc(UsingThreads)  and after that we should continue (just like YAMN creates and calls our
	thread).

	3.  If  we  use account's SWMRG (AccountAccessSO, MessagesAccessSO), we should test what our
	function  returned.  Use  the same methods as POP3 protocol does while testing and accessing
	critical  section. Note that we cannot use WaitToWriteSO(MyAccount->AccountAccessSO), but in
	easy       way      we      can      WaitToWrite(AccountAccess)      and      for      mails
	WaitToWriteSO(MyAccount->MessagesAccessSO) use MsgsWaitToWrite(AccountAccess) and so on. See
	export.h file for these definitions.

	4. Deleting account is quite easy, but in YAMN, it is very problematic operation. If you use
	MS_YAMN_DELETEACCOUNT service, it is the best way to avoid any problem. These problems raise
	from the facts desribed in the point 2.

	5. You should use ctritical sections only for short time not to block other threads. You can
	imagine that users can't browse through mails, because account is blocked by your thread...

	All  needed  infos  in  POP3  internal  protocol  plugin  (see proto/pop3/pop3comm.cpp), are
	described.


4.	What do you need to make your filter plugin cooperating with YAMN
	-----------------------------------------------------------------

	Filter  plugins  are  very  easy  to write in its own way, it much more easier than protocol
	plugin.  But  some  things  are  common: you have to register your plugin and insert to YAMN
	(these  are  2  steps,  see  sources of some filter plugin), You have to import to YAMN your
	filter function. Filter function can do anything with mails, but the most important is, that
	it  can set Flags member of mail (see mails/m_mails.h file) to one of YAMN_MSG_SPAMLx.
	Note  Mail  is  in  write-access,  so  your  plugin  can do anything with mail and avoid the
	synchronization problem.

	Now YAMN recognizes 4 spam levels:
	1. Notifies about this mail, but shows it in mailbrowser with other color than normally
	2. Does not notify about this mail, shows it in mailbrowser with other color than normally
        3. Deletes mail from server (depends on protocol),  does not notify and shows "this spam was
        deleted"
        4. Deletes  mail  from  server  (depends  on  protocol),  does not notify,  does not show in
        mailbrowser

	Your  plugin  can  set  data for mail in the TranslatedHeader structure, inserting it to the
	queue.  This  information  is  stored, so it is reloaded after protocol read mails from book
	file.

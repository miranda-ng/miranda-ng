{
 This file is a description of the interface between the Miranda Scripting 
 Plugin (MSP) and IRC. It is mainly an internal draft.
 
 The MSP is a powerful PHP based scripting engine 
 which can automate many tasks in Miranda IM. With the implementation of 
 this interface IRC can also benefit from the advantages MSP brings. The
 interface has been constructed by the respective authors of IRC and MSP.

 The idea is that MSP and IRC should communicate via the interface outlined herein, 
 but MSP should "package" the data in attractive functions that can be easily 
 used by eager scripters.

 Note that m_chat.h should be #included as the events defined therein is used in this interface
 
 The IRC plugin communicates with MSP mainly through 8 (2 x 4) services which are 
 illustrated by the following matrix (MSP has 4 services and IRC 4 services):

 --------------------------------------------------
 |                      |                         |
 |  Incoming raw data   |      Data going         |
 |     from server      |        to GUI           |
 | (before processing)  |    (after processing)   |
 |                      |                         |
 --------------------------------------------------
 |                      |                         |
 |  Outgoing raw data   |       Data going        |
 |     to server        |        from GUI         |
 | (after processing)   |    (before processing)  |
 |                      |                         |
 --------------------------------------------------
   
 (processing refers to the IRC plugin reacting to different commands)
}

{$IFNDEF M_IRCSCRIPT}
{$DEFINE M_IRCSCRIPT}

//------------------------- HANDLING RAW MESSAGES ------------------------
{
 The two leftmost boxes are related to data sent to and from the server in RAW form.
 (a typical RAW message looks like this: ":nick!user@host COMMAND PARAMETERS")

 It is very useful for scripters to be able to manipulate and add RAW streams of data.
 
 The below structure is used when RAW data is generated and IRC must trigger an 
 event in MSP so the scripts can do their magic.

 If the data is modified make sure to use MS_SYSTEM_GET_MMI and then mmi_realloc() to
 allocate from the same heap. Do not free the data.

  wParam=  Module name
 lParam=  (LPARAM)(AnsiChar *)pszRaw
 returns nonzero if the message should be dropped
}

{
 NOTE! IRC will implement two services which also uses the same parameters:

 "<Module Name>/InsertRawIn"    //used for inserting a "fake" incoming RAW event
 "<Module Name>/InsertRawOut"   //used for inserting a "fake" outgoing RAW event
}

const
  MS_MBOT_IRC_RAW_IN  = 'MBot/IrcRawIn';
  MS_MBOT_IRC_RAW_OUT = 'MBot/IrcRawOut';

//------------------------- HANDLING GUI MESSAGES 1 ------------------------
{
 The top rightmost box is related to data sent to the gui 
 (currently chat.dll) in the form of GCEVENT's. (see m_chat.h)

 While maybe not as useful to the scripter, it can be useful to be able to 
 "tap into" messages to the GUI from the IRC plugin. 

 If the data is modified make sure to realloc() or similar.

   wParam=  (WPARAM) (WPARAM_GUI_IN) &wgi
 lParam=  (LPARAM)(GCEVENT *)&gce     // see m_chat.h
 returns nonzero if the message should be dropped
}

{
 NOTE! IRC will implement a service which also uses the same parameters:

 "<Module Name>/InsertGuiIn"  //used for inserting a "fake" event to the GUI
}
type
  PWPARAM_GUI_IN = ^WPARAM_GUI_IN;
  WPARAM_GUI_IN = record
    pszModule:PAnsiChar;
    wParam:WPARAM;
  end;
  PMBOT_GUI_WPARAM_IN_STRUCT = PWPARAM_GUI_IN;
  TMBOT_GUI_WPARAM_IN_STRUCT = WPARAM_GUI_IN;

const
  MS_MBOT_IRC_GUI_IN = 'MBot/IrcGuiIn';

//------------------------- HANDLING GUI MESSAGES 2 ------------------------
{
 The bottom rightmost box is related to data sent from the gui 
 (currently chat.dll) in the form of GCHOOKS's. Originally this is carried out in a hook,
 but for MSP this is done via a service instead.

 It can be useful for scripters to simulate user actions, like entering text, closing the window etc

 If the data is modified make sure to realloc() or similar.

   wParam=  Module name
 lParam=  (LPARAM)(GCHOOK *)gch     // see m_chat.h
 returns nonzero if the message should be dropped
}

{
 NOTE! IRC will implement a service which also uses the same parameters:

 "<Module Name>/InsertGuiOut"  //used for inserting a "fake" outgoing event to the GUI
}

  MS_MBOT_IRC_GUI_OUT = 'MBot/IrcGuiOut';

  MS_MBOT_REGISTERIRC = 'MBot/RegisterIRCModule';

{$ENDIF}
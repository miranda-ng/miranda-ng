Chess4Net
(c) 2007-2011 No rights reserved
E-Mail: packpaul@mail.ru, packpaul1@gmail.com
Skype:  packpaul1
URL:    http://chess4net.ru
==============================================

Opening
--------

Chess4Net is a program for playing chess via Internet. It can be used as a standalone application (Socket version for Microsoft Windows or Linux) or as plug-in for such instant messengers as Skype, Miranda, QIP Infium, Trillian Pro and &RQ. Two modes of chess game are supported: standart chess and its random counterpart - PP Random Chess. Chess4Net supports sudden death and incremental time controls as well as possibility to give your opponent a time-handicap. If you want to improve your chess skills there is also a support for a training mode. You can also invite other contacts to watch the games you're playing in real-time.

Middle game
------------

Installation:

Socket version:
  Extract the archive to desired folder on your computer.

Skype plug-in:
  * Windows:
  There're two options. You can either install from installation package or download an archive and extract it to a desired folder on your computer (Windows version).

  * Linux:
  Extract downloaded archive to a folder. If you're root then make the user to have full access to the installation folder.

Miranda plug-in:
  Extract archive to the Miranda Plugins directory and re/start Miranda.

QIP Infium plug-in:
  Extract archive to the QIP Infium Plugins directory and re/start QIP Infium

Trillian Pro plug-in:
  Extract archive to the Trillian plugins directory; go to Trillian->Trillian Preferences... choose Plugins and check Chess4Net_trillian.dll box.

&RQ plug-in:
  Extract archive folder Plugins\ to the installation directory of &RQ (the one where &RQ.exe is located).

How to use:

Socket version:
  1) Start Chess4Net.exe
  2) Choose 'Connect...'
  3) Type in-your name into 'Your Nick' edit-box.
    a) Choose Server; provide your partner with IP (type 'ipconfig' in Start->Run... or command line to find it out in Windows; run ./get_ip.pl from installation in terminal on Linux); enter Port# (agree upon it with your partner).
    b) Choose Client; Enter IP or domain Name that your partner told you; Enter Port# (for each instance of Chess4Net it should be unique).
  4) Click OK and wait until the connection is completed.

Skype:
  * Windows:
  1) Run Chess4Net_Skype.exe. If you don't have Skype running it will start automatically. Click 'Allow access' when dialog 'Chess4Net_Skype.exe wants to use Skype' pops-up.
  2) When a window with Skype contacts appears choose a contact you want to play chess with.
  3) Wait until connection is completed.
  
  * Linux
  1) You should have Skype running and be logged in to it.
  2) In terminal go to the installation folder for example:
    cd ~/Chess4Net_Skype
  3) Run Chess4Net:
    ./Chess4Net_Skype

Miranda:
  1) Right-click the contact you want to play chess with and select 'Chess4Net' from contact pop-up menu.
  2) Wait until connection is completed.

QIP Infium:
  1) Open a chat window for the contact you want to play chess with.
  2) Press 'Chess4Net' message button (below the upper frame on the right where the incomming and outgoing messages are shown).

Trillian Pro:
  1) Right-click the contact you want to play chess with and select 'Chess4Net' from pop-up menu or open message window for that contact and type "/CHESS4NET" without double quotes.
  2) Wait until connection is completed.
  NB: Chess4Net uses instant messages to transmit moves and other information. In Trillian Pro there is no way to suppress messages unlike in Miranda, so 
you may want to switch off the sounds for incomming messages while playing.

&RQ:
  1) Open a chat window for the contact you want to play chess with.
  2) Press 'Chess4Net' chat button (below the upper frame where the incomming and outgoing messages are shown).
  3) Wait until connection is completed.

Chess4Net uses context-menu so if you find yourself in trouble don't hesitate and press the right button of your mouse.
After several runs in installation folder (socket, Skype) or \Chess4Net folder (messengers other than Skype) you will find Chess4Net_GAMELOG.txt with all games you have played with Chess4Net so far, user data base (Chess4Net.pos and Chess4Net.mov) and external data bases (other files *.pos and *.mov) which are used in training mode.

Training mode:

The training mode can be switched on/off in Game Options menu. The user data base and external data bases are used in training mode only. The User DB will contain all games played by the user in the training mode, and data from it is used to provide prioritized move hints.
User DB can learn from your games in order to give you best choices. You can also select an external DB to use together with user DB to improve the quality of move hints.

Game transmitting mode:

While you're playing some games you can invite other contacts to follow up these games in real-time. Therefore you must

Miranda:
  start a new session of Chess4Net for a contact you want the game(s) be transmitted to. A mode selection dialog appears asking if game broadcasting has to be started. You must select 'Yes' (if you select 'No' an ordinary game session is created). After that you should select a game for transmition (only if there're several games in progress).

Skype:
  select Broadcast... from pop-up menu. A list of contacts appears. Select a contact whom you want to transmit the game played to.
  
After your contact connects he/she'll be able to watch the game.


Endgame
--------

Chess4Net is constantly improving and it is up to you what features will be included in the next release. Please send all your wishes and/or complaints to E-mail: packpaul@mail.ru or ICQ: 98750806 or Skype: packpaul1
Chess4Net is a software and, like it is peculiar to software, it will never be bug free. Please report all bugs you may find to packpaul@mail.ru but see change log before reporting.

How you can help?
-----------------
If you like Chess4Net you can give your credits at http://chess4net.ru or make some reasonable donations to Yandex-Money: 41001124111397 or Web Money: 774788633995. Also you can provide localization into new languages (see Lang.ini), and, of course, your fresh ideas won't be unnoticed!

Congratulations you won!
------------------------

"I want this feature and I want it right now!!!"
You can have this tuning. Drop me an E-mail we will discuss it.

"I want more external data bases!"
Yes, you can have them. E-mail me and we will discuss it too.

"I want an external DB with the games of the champion from our town and my games played in the Icelandic National Championship!"
Why not? You can have them. Send me an E-mail and we will discuss this.

ENJOY!

PP

==============================

Change log
-----------

Chess4Net 2011.1 (Skype)
[2011-08-06] Localization for German, French, Italian and Estonian
[2011-08-01] Game resurection after application failure improved
[2011-07-24] Keeping Skype connection alive feature
[2011-05-23] Game transmition feature
[2011-05-30] Incorrect chess clock timing on different locales fix
[2011-06-01] Stay on top enabled


Chess4Net 2010.0 (MI)
[2010-05-28] Game retransmition feature added. Dialog handling improved (stay on top, out of screen etc). Majority of crashes fixed. Games numbering if several ones are played.


Chess4Net 2010.1 (Skype)
[2010-02-07] Released with Credits reminder.
[2010-03-06] Skype accept help image added.
[2010-11-13] Linux version released


Chess4net 2009.1 (MI)
[2009-01-01] Localization for English and Russian and possibility to add new languages. Chess board resizing. Possibility to play more than one game simultaneously (multi-sessionality). Flashing window on incomming move.


Chess4net 2008.1 (&RQ)
[2008-06-28] Game adjourn feature added.


Chess4net 2008.1 (Trillian)
[2008-06-28] About dialog and game adjourn feature added.


Chess4net 2008.1 (QIP Infium)
[2008-06-28] Game adjourn feature added.


Chess4net 2008.1 (MI)
[2008-04-19] About dialog and game adjourn feature added.


Chess4net 2008.0 (&RQ)
[2008-02-12] New API version of 9.7.4 is utilized: no history entries are done anymore when plugin messages are transmited. Compatibility with 9.7.3 version is preserved.


Chess4net 2008.0 (QIP Infium)
[2007-12-31] Released; This version has some important restrictions: you will NOT be able to play against Chess4Net clients of other IMs due to the restriction imposed on QIP Infium SDK ver. 1.3.0 (waiting for that to be fixed)


Chess4net 2007.6 (Socket - Linux)
[2007-12-8] Released; code and functionality branched from Chess4net 2007.6 (Socket - Windows)


Chess4net 2007.6 (Socket - Windows)
[2007-10-09] Dialogs' caption changed to 'Chess4Net'. Casual color desynchronization on game start fixed.
[2007-10-16] Game pause functionality introduced; Flag UI introduced; spin boxes added to Game Options dialog.
[2007-10-17] Connector module slightly improved.
[2007-11-25] Bugs in game options dialog fixed; auto-flag bugs fixed
[2007-12-05] Critical bug in logic after en passant capture with double pawn in a position fixed. [!!!PLEASE UPDATE TO CURRENT VERSION!!!]
[2007-12-07] Switching color disabled after the game has started


Chess4net 2007.6 (MI)
[2007-10-10] Dialogs handling improved.
[2007-10-11] Casual ñolor desynchronization on game start fixed; Game pause functionality introduced.
[2007-10-12] Writing empty games to game log canceled.
[2007-10-16] Flag UI introduced; spin boxes added to Game Options dialog.
[2007-11-25] Bugs in game options dialog fixed; auto-flag bugs fixed
[2007-12-05] Critical bug in logic after en passant capture with double pawn in a position fixed. [!!!PLEASE UPDATE TO CURRENT VERSION!!!]
[2007-12-07] Switching color disabled after the game has started


Chess4net 2007.6 (Trillian)
[2007-10-10] Dialogs handling improved.
[2007-10-11] Casual ñolor desynchronization on game start fixed.
[2007-10-16] Flag UI introduced; spin boxes added to Game Options dialog.
[2007-10-17] Game pause functionality introduced; 
[2007-11-25] Bugs in game options dialog fixed; auto-flag bugs fixed
[2007-12-05] Critical bug in logic after en passant capture with double pawn in a position fixed. [!!!PLEASE UPDATE TO CURRENT VERSION!!!]
[2007-12-07] Switching color disabled after the game has started


Chess4net 2007.6 (&RQ)
[2007-10-10] Dialogs handling improved.
[2007-10-11] Casual ñolor desynchronization on game start fixed.
[2007-10-16] Flag UI introduced; spin boxes added to Game Options dialog.
[2007-10-17] Game pause functionality introduced; 
[2007-11-25] Bugs in game options dialog fixed; auto-flag bugs fixed
[2007-12-05] Critical bug in logic after en passant capture with double pawn in a position fixed. [!!!PLEASE UPDATE TO CURRENT VERSION!!!]
[2007-12-07] Switching color disabled after the game has started


Chess4net 2007.5 (Socket)
[2007-08-29] Protocol changed; Takebacks on first move fixed; New behaviour to Game Options dialog introduced; Saving private and public settings introduced (also IP+port, socket state and nick); Display of hint moves after takeback and on start fixed; Amount of entries of training DB restricted up to 30 per game; 'Disconnect' item moved to lower part of pop-up menu.


Chess4net 2007.5 (MI)
[2007-08-22] Protocol changed; Takebacks on first move fixed; New behaviour to Game Options dialog introduced.
[2007-08-24] Crash on opening pawn promotion window fixed.
[2007-08-25] Saving private and public settings introduced.
[2007-08-26] Creation of training DBs with empty name fixed.
[2007-08-27] Display of hint moves after takeback and on start fixed.
[2007-08-29] Bug with resetting of exteral training base for standard and PP mode fixed; Amount of entries of training DB restricted up to 30 per game.
[2007-09-03] Hiding of Old version warning dialog in Always On Top mode fixed.
[2007-09-12] Objectionable 'ext' command message suppreced.
[2007-09-13] Wrong clock settings saving when client was forcibly closed.
[2007-09-26] Modal dialogs with 'Stay On Top' option switched on caused AV error -> fixed.


Chess4net 2007.5 (Trillian)
[2007-09-13] A port of Chess4Net for Trillian has appeared. Some issues are still there: no icon for client menu; it was impossible to suppress Ch4N messages.
[2007-09-15] HTML untagging for incomming plugin messages done.


Chess4net 2007.5 (&RQ)
[2007-09-17] A port of Chess4Net for &RQ has appeared. There are some issues though: it was impossible to force the outgoing messages be hidden from chat window (lack of API -> PE_MSG_SENT should be fired after the message actually has gone).


Chess4Net 2007.4 (Socket)
[2007-05-23] Training mode added.
[2007-05-25] Tending to backward compatibility.
[2007-05-31] Wrong pop-up menu by disconnection fixed.
[2007-06-21] Clock setting to 0.0 after time control is out fixed.
[2007-07-17] Extra Exit on ESCAPE added; Background of coordinates improved.


Chess4Net 2007.4 (MI)
[2007-05-21] New wellcome message.
[2007-05-23] Training mode added.
[2007-05-25] Tending to backward compatibility.
[2007-07-17] Extra Exit on ESCAPE added; Background of coordinates improved; Click-click synchronization fixed; AV by pawn promotion fixed


Chess4Net 2007.3 (Socket)
[2007-03-31] time display refresh fixed when changing colors.
[2007-04-01] game log added; 'Equal time for both players' checkbox initialization fixed.


Chess4Net 2007.3 (MI)
[2007-02-15] time incrementation fixed.
[2007-03-31] game log added; time display refresh fixed when changing colors; ver. info changed.
[2007-04-01] 'Equal time for both players' checkbox initialization fixed.
[2007-04-02] 'Chess4Net' title addedd to pop-up windows; Chess4Net icon added to plugin window and contact menu.
[2007-04-03] Saving to game log on exit and on connection error fixed.


Chess4Net 2007.2 (Socket)
[2007-02-15] time incrementation fixed.
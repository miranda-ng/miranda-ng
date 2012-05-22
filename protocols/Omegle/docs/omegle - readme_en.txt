---------------------------
| Omegle Protocol 0.1.0.0 |
|        (26.4.2012)      |
---------------------------

Autor: Robyer
  E-mail: robyer@seznam.cz
  Jabber: robyer@jabbim.cz
  ICQ: 372317536
  Web: http://robyer.info
 
SVN: http://code.google.com/p/robyer/

!!!! Required Miranda 0.9.43 or newer or Miranda 0.10.2 and newer !!!!

--------------------------------
			 HIDDEN SETTINGS
--------------------------------
TimeoutsLimit (byte) - number of allowed timeouts before disconnection protocol

--------------------------------
       Version history
--------------------------------
0.1.0.0 - 26.4.2012
 First addons release
 
0.0.2.0 - 23.4.2012
 ! Commited some fixes from Facebook protocol (patch by borkra)
 * Reworked commands
 + Remember last question in Question mode

0.0.1.1 - 13.4.2012
 ! Fixed saving text values in options

0.0.1.0 - 13.4.2012
 + Added settings and account dialogs
 + Getting number of users connected to server
 + Support for Question mode and Spy mode
 + Support for searching strangers with same interests
 + Added sounds for stranger typing notify
 ! Code cleanup
 ! Many fixes and improvements

0.0.0.6 - 26.3.2012
 ! Fixed connection
 * When connecting to stranger, chose random Omegle server

0.0.0.5 - 27.9.2011
 * Some internal changes (this brings some bugs - you will see :))
 + Added support for send message after stranger connects (see hidden settings)
 + Added support for automatic reconnect to another stranger (see hidden settings)
 + Added support for not erasing content of chat window (see hidden settings)

0.0.0.4 - 18.4.2011
 * (Dis)connecting strangers in other thread
 * Added admin role to own contact
 ! Fixed <undef> in Tabsrmm
 ! Fixed not informing about disconnecting stranger
 ! Fixes to own nickname (instead of "You")
 ! Fixed not supporting smileys
 ! Another small fixes
 * Changed name of hidden key "ChatName" to "Nick"
 * You can enable logging through hidden key "EnableLogging" (byte) = 1

0.0.0.3 - 16.4.2011
 + Added protocol icons (until someone creates better)
 + You can change your name in chat through hidden key "ChatName" (unicode string)
 * Changed commands: Command "/new" for connect to new stranger, "/quit" for end actual conversation
 ! Fixed duplicying outgoing events after reconnect
 ! Fix for not showing messages, which were received with another event

0.0.0.2 - 13.4.2011
 ! Replacing special chars in incomming messages (\r,\n,\,")
 + Hidden key "MessageAsl" (Unicode string) with text, which is send when you write "/asl"
 * Used persistent http connection
 * Small changes/fixes

0.0.0.1 - 10.4.2011
 * First public version
 
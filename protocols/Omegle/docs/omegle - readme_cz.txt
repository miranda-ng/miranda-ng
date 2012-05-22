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

!!!! Vyžadována Miranda 0.9.43 a novìjší nebo 0.10.2 a novìjší !!!!!

--------------------------------
			 SKRYTÉ NASTAVENÍ
--------------------------------
TimeoutsLimit (byte) - poèet povolených timeoutù pøed odpojením protokolu

--------------------------------
       Historie verzí
--------------------------------
0.1.0.0 - 26.4.2012
 První vydání na addons
 
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
 ! Opraveno pøipojení
 * Pøi pøipojování k cizinci se volí náhodný Omegle server

0.0.0.5 - 27.9.2011
 * Interní zmìny ohlednì stavù komunikace
 + Added support for send message after stranger connects (see hidden settings)
 + Added support for automatic reconnect to another stranger (see hidden settings)
 + Added support for not erasing content of chat window (see hidden settings) 

0.0.0.4 - 18.4.2011
 * Pøipojování/odpojování cizincù v jiném vláknì
 * Úprava rolí kontaktù v chatu
 ! Opraveno <undef> v Tabsrmm
 ! Opraveno neinformování o odpojení cizince
 ! Opravy k zobrazení vlastní pøezdívky místo "You"
 ! Oprava pro použití smajlíkù
 ! Spousta rùzných oprav
 * Zmìna názvu skrytého klíèe "ChatName" na "Nick"
 * Možnost zapnutí logování skrz skrytý klíè "EnableLogging" (byte) = 1

0.0.0.3 - 16.4.2011
 + Pøidány ikony protokolu (dokud nìkdo nevytvoøí lepší)
 + Možnost pøejmenování vlastního jména v chatu skrz Unicode klíè "ChatName".
 * Zmìna "ovládání": Pøíkaz "/new" pro pøipojení k nové konverzaci a "/quit" pro ukonèení stávající
 ! Opraveno duplikování odchozích událostí po pøepøipojení
 ! Oprava nezobrazení zpráv, které byly pøijaty s jinou událostí

0.0.0.2 - 13.4.2011
 ! Nahrazování speciálních znakù v pøíchozích zprávách (\r,\n,\,")
 + Skrytý klíè "MessageAsl" (Unicode string), kde lze zapsat text zprávy, která se odešle pøi odeslání "/asl"
 * Použití persistent http pøipojení
 * Drobné zmìny èi opravy

0.0.0.1 - 10.4.2011
 + První veøejná verze
 
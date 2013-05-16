---------------------------
| Omegle Protocol 0.1.2.0 |
|       pro Miranda NG    |
|         (16.5.2013)     |
---------------------------

Autor: Robyer
  E-mail: robyer@seznam.cz
  Jabber: robyer@jabbim.cz
  ICQ: 372317536
  Web: http://www.robyer.cz

Info:
 - Tento plugin používá èást kódu z Facebook Protokolu (autor jarvis) verze 0.1.3.3 (open source).
 - Originální zdrojové kódy jsou k nalezení na http://code.google.com/p/eternityplugins/

--------------------------------
			 SKRYTÉ NASTAVENÍ
--------------------------------
TimeoutsLimit (byte) - poèet povolených timeoutù pøed odpojením protokolu

--------------------------------
       Historie verzí
--------------------------------
0.1.2.0 - 16.5.2013
 + Zobrazení upozornìní na psaní cizince ve stavovém øádku
 + Pøidána možnost pro volbu jazyka cizince
 ! Drobné opravy.

0.1.1.0 - 8.3.2013
 + Možnost automaticky se pøipojit pøi pøepnutí protokolu do online
 * Aktualizace (tohoto) readme
 
=== STARÉ ZMÌNY (MIRANDA IM) ===

0.1.0.1 - 24.5.2012
 * Stejné GUID pro 32bit a 64bit verzi pluginu 
 * Aktualizace pro novou verzi smileyadd pluginu
 ! Opravena funkènost pluginu kvùli zmìnám na Omegle
 ! Opraveno nastavení (ukládání textových hodnot, TAB pøepínání) 
 ! Zobrazení vlastního jména v tooltipu v kontaktlistu (díky FREAK_THEMIGHTY) 
 ! Interní opravy 

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
 
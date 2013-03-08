--------------------------------
| Facebook Protocol RM 0.0.9.5 |
|        pro Miranda NG        |
|          (2.3.2013)          |
--------------------------------

Autor: Robyer
  E-mail: robyer@seznam.cz
  Jabber: robyer@jabbim.cz
  ICQ: 372317536
  Web: http://www.robyer.cz
 
Info: 
 - Tento plugin je zaloen na Facebook Protokolu (autor jarvis) verze 0.1.3.3 (open source).
 - Jeho verze je k nalezení na: http://code.google.com/p/eternityplugins/

--------------------------------
      Informace o stavech
--------------------------------
 - Online = pøipojen k fb, chat je online
 - Neviditelnı = pøipojen k fb, ale pouze pro získávání novinek a novıch oznámení - CHAT je OFFLINE
 - Offline = odpojeno

--------------------------------
       Skryté nastavení
--------------------------------
"TimeoutsLimit" (Byte) - Errors limit (default 3) after which fb disconnects
"DisableLogout" (Byte) - Disables logout procedure, default 0
"PollRate" (Byte) - Waiting time between buddy list and newsfeed parsing.

--------------------------------
       Historie verzí
--------------------------------

... TODO ...

=== OLD CHANGES (MIRANDA IM) ===

0.0.9.3 - 12.12.2012
 ! Opraven pád kdy nebyl na stránce nalezen vlastní avatar
 * Plugin linkován staticky

0.0.9.2 - 23.10.2012
 ! Opraveny zmìny avatarù kontaktù
 ! Opraveno pøipojení s jedním typem vyádání nastavení názvu poèítaèe
 ! Pøepracováno poadavkù pro odesílání zpráv (mìlo by zamezit vzácnému banu od FB :))
 ! Vıchozí skupina pro nové kontakty se vytvoøí pøi pøihlášení, pokud ještì neexistuje
 ! Oprava zmìn viditelnosti z jiného klienta
 * Plugin linkován znovu proti C++ 2008 knihovnám

0.0.9.1 - 16.10.2012
 ! Opraveno nefunkèní pøihlášení kvùli zmìnì na Facebooku
 * Plugin linkován proti C++ 2010 knihovnám

0.0.9.0 - 11.6.2012 
 + Pøijímání poadavkù o pøátelství (kontrola kadıch cca. 20 minut)
 + Pøepracovány autorizace - ádání, potvrzování, rušení pøátelství
 + Podpora vyhledávání a pøidávání lidí
 * Zmìna nìkterıch øetìzcù
 * Pouití stejného GUID pro 32bit a 64bit verzi
 ! Pøijímání zpráv s originálním èasovım razítkem
 ! Opraveno mazání avatarù kontaktù se stavem "Na mobilu"
 ! Uvolòování OnModulesLoaded hooku (díky Awkward)
 ! Neodesílat upozornìní na psaní kontaktùm, kteøí jsou offline 
 ! Oprava práce s avatary (díky borkra)
 ! SetWindowLong -> SetWindowLongPtr (díky ghazan) 
 ! Funkèní pøihlášení s pozvrzením posledního pokusu o pøihlášení z neznámého zaøízení
 ! Opraveno odesílání zpráv do skupin
 ! Rùzné další interní opravy

0.0.8.1 - 26.4.2012
 ! Opraveno pøijímání oznámení po pøipojení
 ! Opraveno pøijímání nepøeètenıch zpráv po pøipojení
 ! Pøijímání nepøeètenıch zpráv po pøipojení se správnım èasovım razítkem
 ! Opraveno oznamování novinek ze zdi
 ! Oprava související s mazáním kontaktù/rušením pøátelství
 + Nová volba novinek ze zdi "Aplikace a hry"
 + Kontakty mají nyní MirVer "Facebook" 
 + U offline zpráv se pøijímají i pøípadné pøílohy
 ! Opraveno fungování avatarù v Mirandì 0.10.0#3 a vyšší (díky borkra)
 ! Nìjaké drobné opravy (díky borkra)
 
 x Nefunguje oznámení o poadavcích o pøátelství

0.0.8.0 - 14.3.2012
 # Pro spuštìní pluginu je vyadována Miranda verze 0.9.43 a novìjší
 # Plugin je kompilován skrz VS2005 (Fb x86) a VS2010 (Fb x64)
 + Pøidány 2 typy pøíspìvkù k oznamování ze zdi: Fotky a Odkazy 
 * Pøepracováno nastavení
 ! Opraveno nastavení oznamování jiného typu novinek ze zdi
 ! Opraveno a vylepšeno parsování novinek ze zdi
 ! Opraveno pøijímání zpráv v chatu, ve kterıch je %
 ! Opraveno nefunkèní pøihlášení 
 ! Vylepšeno mazání kontaktù  
 + Podpora EV_PROTO_ONCONTACTDELETED událostí 
 + Pøidáno chybìjící GUID pro x64 verzi a aktualizován user-agent  
 ! Nìjaké další malé opravy a vylepšení
 ! Opravena poloka 'Visit Profile', pokud je v hlavním menu
 * Aktualizován language pack soubor (pro pøekladatele)
 - Vypnuta monost pro zavírání oken na facebooku (doèasnì nefunguje)

0.0.7.0 - 19.1.2012
 + Podpora skupinovıch chatù (EXPERIMENTÁLNÍ!) - zapnìte v nastavení
 ! Opraveno naèítání seznamu kontaktù
 ! Oprava moného zamrznutí Mirandy

0.0.6.1 - 6.1.2012
 + Vrácena monost v nastavení, pro zavírání oken chatu (na webu)
 + Monost mapovat nestandardní stavy na stav Neviditelnı (místo na Online)
 + Monost zobrazovat lidi, kteøí mají stav "na mobilu"
 ! Opravena zmìna viditelnosti v chatu
 ! Velmi dlouhé zprávy u se nepøijímají duplicitnì
 ! Úpravy ohlednì skupinovıch zpráv a pøíjmu zpráv od lidí, kteøí nejsou mezi pøáteli

0.0.6.0 - 18.11.2011
 * Pøepracováno nastavení
  + Monost pouít https pøipojení i pro "Channel" poadavky
  + Monost pouívat vìtší avatary kontaktù
	+ Monost pøijímat nepøeètené zprávy pøi pøihlášení (EXPERIMENTÁLNÍ!)
	+ Monost pøi odpojení protokolu odpojit také chat.
  - Zrušena monost nastavení User-Agenta
  - Zrušena monost zobrazení Cookies
 * Do databáze se ukládá datum a èas zjištìní odstranìní kontaktu ze serveru (hodnota "Deleted" typu DWORD)
 + Monost v menu kontaktu pro smazání ze serveru.
 + Monost v menu kontaktu pro poádání o pøidání mezi pøátele.
 + Pøi smazání kontaktu z Mirandy se zobrazí dialog s moností smazat i ze serveru 
 ! Opraveno nefunkèní pøihlášení

0.0.5.5 - 16.11.2011
 ! Opraveno neodpojování kontaktù

0.0.5.4 - 16.11.2011 
 ! Opraveno nìkolik problémù s pøipojením
 ! Rùzné opravy pádù, únikù pamìti a komunikace (díky borkra)
 @ Pokud se vám Facebook odpojuje a máte zapnuté HTTPS pøipojení, vypnìte si volbu "Ovìøit platnost certifikátù" pro Facebook v Monosti/Sítì.

0.0.5.3 - 31.10.2011
 ! Opraven problém s pøihlášením pro nìkteré lidi.
 ! Opraveno nepøijímání nìkterıch zpráv
 ! Opraveno chybné naèítání vlastního jména
 * Rychlejší odesílání zpráv

0.0.5.2 - 31.10.2011
 ! Opraven problém s pøipojením a pádem.

0.0.5.1 - 30.10.2011
 ! Work-around pro posílání zpráv obsahující odkazy.

0.0.5.0 - 29.10.2011
 + Oznámení o pøátelích, kteøí se znovu objevili na serveru.
 * Kompletnì pøepracována podpora avatarù.
 * Ve vıchozím nastavení se pouívají malé ètvercové avatary (lze zmìnit skrytım nastavením "UseBigAvatars")
 ! Opraveno pouití skrytého nastavení "UseBigAvatars" 
 ! Opraveno nastavování stavové zprávy.
 ! Opraven pád pøi pouití MetaContacts.
 ! Opraveno pøihlášení pro nìkteré lidi.
 ! Opravy ohlednì oznámení událostí pøi pøipojení.
 ! Opraven únik pamìti související s oznámením.
 ! Opraveno zjišování nepøeètenıch zpráv pøi pøihlášení (pokud je pouito skryté nastavení "ParseUnreadMessages")
 ! Opraveno pøipojení s nastavením Názvu zaøízení. 
 ! Rùzné opravy, vylepšení a oèista kódu. 
 - Odstranìn skrytı klíè "OldFeedsTimestamp" 
 @ Díky borkra pro pomoc se spoustou vìcí. 

0.0.4.3 - 22.9.2011
 ! Oprava nového layoutu facebooku
 ! Oprava naèítání jména kontaktu pro nové kontakty

0.0.4.2 - 15.9.2011
 ! Opraveno neaktivování poloek stavového menu
 ! Nenastavování automaticky kontaktu do "Online" stavu, kdy od nìj pøijde zpráva
 ! Oprava kódování chybovıch hlášek pøi odesílání zprávy
 + Podpora odesílání zpráv ve stavu "Neviditelnı" 
 + Stav protokolu respektuje zmìny stavu chatu na stránce
 + Pøi pøihlášení oznámení konkrétních upozornìní namísto jejich pouhého poètu
 + Pøidán skrytı klíè (ParseUnreadMessages) pro pøijímání nepøeètenıch zpráv pøi pøihlášení namísto oznámení jejich poètu << POZOR: není plnì funkèní!!!

0.0.4.1 - 13.9.2011
 ! Vráceny zmìny zpùsobující neodpojování kontaktù

0.0.4.0 - 12.9.2011
 * Interní zmìny ohlednì zmìny stavu
 - Odstranìna podpora stavu "Na chvíli pryè"
 ! Opraveno parsování novinek ze zdi
 - Zrušeno oznámení o poètu nepøeètenıch zpráv ve stavu "Neviditelnı"
 + Ve stavu "Neviditelnı" pøijímání zpráv pøímo
 + Naèítání pohlaví kontaktù
 + Naèítání všech kontaktù ze serveru (tzn. nejen ty, co jsou právì online)
 + Informování o tom, e nìkdo zrušil pøátelství s vámi (= nebo si jednoduše deaktivoval facebook úèet)

0.0.3.3 - 17.6.2011
 ! Oprava správné komunikace (zjišování seq number)
 ! Oprava oznámení s nepøeètenımi událostmi pøi pøipojení

0.0.3.2 - 8.6.2011
 ! Oznaèování zpráv na facebooku jako pøeètenıch.
 * Vypnuto oznamování o channel refresh

0.0.3.1 - 23.5.2011
 ! Opraveno upozornìní pøi neodeslání zprávy kvùli vypršení èasového limitu

0.0.3.0 - 22.5.2011
 ! Opraveno naèítání avatarù
 ! Opraveno zobrazování avatarù v SRMM
 ! Opraveno nenaètení pluginu pro nìkteré lidi s Mirandou 0.9.17
 ! Opraveny pády a zamrzávání pøi mazání úètu
 ! Nìkolik oprav a vylepšení spojenıch s pøihlašováním
 ! Oprava obèasného chybného parsování novinek ze zdi 
 ! Oprava pádu pøi pøipojení pokud jsou nepøeètené události 
 ! Oprava nìkterıch dalších pádù
 * Vylepšeno získávání avatarù kontaktù (2x rychlejší)
 * Optimalizace nastavení kompilace -> 2x menší vıslednı soubor (díky borkra)
 + Pouito persistentní http pøipojení (díky borkra)
 + Nièení slueb a hookù pøi ukonèení (díky FREAK_THEMIGHTY)
 + Podpora pro per-plugin-translation (pro MIM 0.10#2) (díky FREAK_THEMIGHTY)
 + Podpora pro EV_PROTO_ONCONTACTDELETED (pro MIM 0.10#2) (díky FREAK_THEMIGHTY)
 - Nepøekládání názvu protokolu v nastavení
 - Zrušena "optimalizace" pro zavírání oken. 
 ! Opraveno odesílání informace o psaní
 ! Opraveno parsování odkazù u novinek na zdi
 * Povoleno odesílání offline zpráv
 ! Opraveno oznamování chyb pøi odesílání zpráv (+ zobrazení konkrétní chyby)
 + 5 pokusù o odeslání zprávy pøed zobrazením chybové hlášky.

0.0.2.4 - 6.3.2011
 ! Opraveny duplicitní zprávy a oznámení
 * Limit pro oznamování zpráv skupinovıch rozhovorù - max. jednou za 15 sekund
 * Optimalizace pro odesílání informace o psaní
 * Optimalizace pro zavírání oken se zprávami na webu
 + Upozornìní, pokud je moné, e jsme nepøijali nìkterou zprávu
 + Pøi odeslání offline zprávy zobrazení oznámení s moností otevøít odeslání zprávy skrz web

0.0.2.3 - 5.3.2011
 ! Oprava nenaèítání jmen kontaktù
 ! Oprava definic (x64 verze) pro Updater
 + Pøidány velké (32px) stavové ikony
 + Volba typu pøíspìvkù, které se budou oznamovat
 * Zmìna názvù User-Agentù na lidštìjší
 * Vylepšeno parsování novinek ze zdi
 
0.0.2.2 - 2.3.2011
 + Podpora pro Updater
 + Poloka v menu kontaktu a stavu pro otevøení profilové stránky (+ ukládání v db jako poloka Homepage)
 + Oznamování o novì pøijatıch soukromıch zprávách (pouze ve stavu Neviditelnı)
 + Automatické nastavení https pøipojení pøi pøihlášení, pokud je vyadováno
 * Optimalizace stahování avatarù kontaktù
 ! Oprava pro obèasné \n v oznámeních+ 5 pokusù o odeslání zprávy pøed zobrazením chybové hlášky.
 ! Oprava pro html tagy v chybové hlášce pøi pøipojení
 ! Oprava nemonosti odesílat zprávy

0.0.2.1 - 21.2.2011
 ! Opravy dalších stavù (nemonost pøepnutí do Na chvíli pryè, pøepínání v jinıch vláknech,...)
 ! Oprava naètení vlastního avataru pøi zmìnì
 * Nastavení zvukù pouívá konkrétní název úètu (díky FREAK_THEMIGHTY)
 ! Opravy pro x64 verzi (díky FREAK_THEMIGHTY)
 ! Oprava synchronizace vláken 
 ! Opraveno poøadí odesílanıch zpráv a oznamování o jejích chybách
 ! Oprava pár chyb zpùsobujících nedoruèování všech zpráv
 ! Opravy interní implementace seznamu
 * Refactoring a zjednodušení nìkterıch vìcí
 + Pøidána 64 bitová verze pluginu
 + Pøi pøihlášení oznámení o poètu novıch notifikací
 ! Oprava parsování nìkterıch typù novinek
 + 1. fáze podpory skupinovıch chatù - oznamování pøíchozích zpráv

0.0.2.0 - 13.2.2011
 * Vedeno jako novı plugin Facebook RM + nové readme a adresáøová struktura
 x Doèasnì vypnuta podpora pro updater a informování o "fb api"
 ! Oprava naètení vlastního avataru
 ! Oprava zobrazování novinek na zdi a jejich lepší parsování
 + Pøidány stavy Na chvíli pryè a Neviditelnı
 * Stav Na chvíli pryè se pouívá stejnì jako idle pøíznak
 ! Opravena správa neèinnosti - facebook padá do neèinnosti jen ve stavu Na chvíli pryè

--------------------------------
      Stará historie verzí
 (integrováno v oficiální verzi)
--------------------------------
0.0.2.0 (0.1.3.0) - 20.12.2010 (nevydáno)
 ! Oprava zobrazování bublinového oznámení
 ! Oprava odhlašovací procedury (nekompletní)
 ! Oprava zobrazení oznámení s poètem novıch zpráv
 x Zrušeno naèítání stavovıch zpráv (facebook je pøestal zobrazovat)
 * Zmìna naèítání avatarù z hovercardu místo mobilního facebooku (zrychlení stahování a sníení velikosti dat)
 + Kontrola úspìšného odeslání zprávy
 * Aktualizována modifikovaná miranda32.exe na nejnovìjší verzi
 * Rozdìleno readme na èeské a anglické a aktualizováno
1.21 - 27.11.2010
 + Oznamování novì pøijatıch "soukromıch" (ne chatovıch) zpráv
 + Pseudo správa neèinnosti (pøi neèinnosti v mirandì nechá facebook upadnout do jeho neèinnosti)
 + Skrytá poloka v databázi, pro ignorování timeoutù kanálu zpráv (vloit klíè "DisableChannelTimeouts" (byte) s hodnotou 1)
 * Pøepsány a upraveny nìkteré vìci (moná opraveny duplicitní zprávy, moná pøidány další chyby, apod.)
 ! Opravena zmìna neèinnosti kontaktù
 ! Oprava vıpisu èasu v logu
1.20 - 22.11.2010
 + Monost pouití bublinového oznámení v lištì místo popupù
 + Monost pouití https protokolu (pomùe pøipojení pøi nìkterıch firewallech)
 + Upravenı miranda32.exe je pøidán pro správnou funkènost bez odpojování fb (dokud nìkdo nepøepíše http komunikaci :)
1.19 - 20.11.2010
 ! Oprava stahování a aktualizace avatarù
 ! Nenastavování "Vlastního jména" kontaktù (tzn. zobrazení nové pøezdívky pøi zmìnì)
 ! Oprava html tagù ve stavovıch zprávách
 ! Zobrazení chyby pøi pokusu o odeslání zprávy v offline/offline kontaktu
 ! Pokus o opravu pádù pøi moc dlouhém textu pøíspìvku ze zdi
 ! Doèasné ignorování timeout chyb (dokud neopraví v jádøe mirandy)
1.18 - 28.9.2010
 ! Oprava pøi vynuceném pøepojení.
 ! Nerozbrazování odpojení kontaktù pøi vlastním odpojení
1.17 - 23.9.2010
 ! Pokus o vyøešení duplicitních zpráv
1.16 - 10.9.2010
 * Po pøihlášení se nezobrazují staré novinky ze zdi (pro staré chování vytvoøit BYTE klíè OldFeedsTimestamp s hodnotou 1)
 ! Oprava pro prázdné novinky na zdi (prázdné se nezobrazí)
1.15 - 9.9.2010
 ! Oprava pro nastavení -1 (nekoneèného) èasu oznámení
1.14 - 9.9.2010
 ! Opravena v oznámeních /span>
1.13 - 9.9.2010
 ! Zavírání fb okna se zprávou (pøi pøijímání zprávy) je zpracováno v jiném threadu.
1.12 - 8.9.2010
 ! Oprava chyby pøi odpojení posledního kontaktu.
1.11 - 7.9.2010
 ! Oprava pro duplicitní oznámení novinek, které nemají odkaz (zmìna fotky, atd.)
1.10 - 7.9.2010
 + Základní otvírání facebook stránky po kliknutí levého tlaèítka myši
 + Pøidáno nastavení oznámení (barvy, èasy,...)
 + Informování o KONKRÉTNÍ chybì pøi neúspìšném pokusu o pøihlášení.
1.9 - 4.9.2010
 + Informování o psaní kontaktu
1.8 - 4.9.2010
 + Monost automaticky zavírat okna se zprávou (na webu)
1.7 (0.1.2.0) - 7.8.2010
 ! Nezobrazování Mirandího dialogu pro zadání stavové zprávy, pokud není aktivována volba "Set Facebook 'Whats on my mind' status through Miranda status.
 ! Opraveno nìkolik únikù pamìti, ale nìkteré tu ještì mùou bıt...
 + Pøidán èas do debug logu.
1.6 (0.1.1.0) - 15.6.2010
 ! Oprava znovu nefunkèního pøipojení pøi "Oznámení pøi pøipojení z jiného zaøízení".
 x Nefunguje nastavování/mazání vlastní "Co se vám honí hlavou?" zprávy
1.5 - 10.6.2010
 ! Oprava oznámení o novém Facebook API
 ! Opraven bug s kódováním zpráv pøi pouití metakontaktù
 ! Oprava jedné malé chyby
1.4 - 7.6.2010 14:45
 ! Oprava získání stavové zprávy (GetMyAwayMsg) pro ansi pluginy (napø. mydetails)
 + Pøidán zvuk a základní ikona FB k oznámení
1.3 - 7.6.2010 01:00
 ! Oprava nekonèící "aktualizace informací kontaktu"
 ! Oprava kdy se pøi odpojení pluginu nastavila offline globální ikona
 ! Oprava naèítání feedù z facebook zdi
1.2
 ! Funguje pøihlašování i se zapnutım "Oznánemím pøi pøipojení z jiného zaøízení.
1.1
 ! Oprava nefunkèního pøihlášení
1.0
 + Pøidán skrytı klíè v databázi: "Sloka" Facebook, klíè DisableStatusNotify (typ Byte) s hodnotou 1. (Novì pøidané kontakty budou mít automaticky nastaven pøíznak "Ignorovat oznámení o zmìnì stavu" (tzn. to co se nastauvje v Monosti / Události / Filtrování)
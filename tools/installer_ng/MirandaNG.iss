#ifdef ptx86
   #define MirName "Miranda32.exe"
   #define MirGroupName "Miranda NG"
   #define MirOutName "miranda-ng-alpha-latest"
   #define ArcAllow ""
   #define VcRedistName "vcredist_x86.exe"
   #define Ptf "x86"
   #define RedistRegChk "(HKEY_LOCAL_MACHINE, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{F0C3E5D1-1ADE-321E-8167-68EF0DE699A5}')"
   #define MirPfInstDir "ExpandConstant('{pf32}')"
#else
   #define MirName "Miranda64.exe"
   #define MirGroupName "Miranda NG x64"
   #define MirOutName "miranda-ng-alpha-latest_x64"
   #define ArcAllow "x64"
   #define VcRedistName "vcredist_x64.exe"
   #define Ptf "x64"
   #define RedistRegChk "(HKLM64, 'SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{1D8E6291-B0D5-35EC-8441-6616F567A0F7}')"
   #define MirPfInstDir "ExpandConstant('{pf64}')"
#endif

[Setup]
AppVersion={#AppVer}
AppName=Miranda NG
AppVerName=Miranda NG {#SetupSetting("AppVersion")}
AppPublisher=Miranda NG Team
AppCopyRight=2013 © Miranda NG Team
VersionInfoVersion={#SetupSetting("AppVersion")}
MinVersion=5.0
ArchitecturesAllowed={#ArcAllow}
DefaultDirName={pf}\Miranda NG
DefaultGroupName={#MirGroupName}
LicenseFile=Files\Docs\license.txt
UninstallFilesDir={app}\Uninstall
UninstallDisplayIcon={#MirName}
UninstallDisplayName=Miranda NG
UninstallLogMode=overwrite 
Uninstallable=IsDefault()
CreateUninstallRegKey=IsDefault()
Compression=lzma2/max
SolidCompression=yes
PrivilegesRequired=poweruser
OutputDir=..\
OutputBaseFilename={#MirOutName}
WizardImageFile=Installer\WizModernImage-IS.bmp
WizardSmallImageFile=Installer\SetupMNGSmall.bmp
SetupIconFile=Installer\mng_installer.ico
DirExistsWarning=no
UsePreviousAppDir=no
UsePreviousGroup=no
UsePreviousSetupType=no
UsePreviousTasks=no


[Files]
; Folders
Source: "Files\Docs\*"; DestDir: "{app}\Docs"; Components: program; Flags: ignoreversion recursesubdirs createallsubdirs; AfterInstall: ShowPercent() 
Source: "Files\Skins\Sounds\*"; DestDir: "{app}\Skins\Sounds"; Components: sounds; Flags: ignoreversion recursesubdirs createallsubdirs; AfterInstall: ShowPercent() 

; Icons
Source: "Files\Icons\Proto_AIM.dll"; DestDir: "{app}\Icons"; Components: protocols\aim; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\Proto_Facebook.dll"; DestDir: "{app}\Icons"; Components: protocols\facebook; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\Proto_GG.dll"; DestDir: "{app}\Icons"; Components: protocols\gg; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\Proto_ICQ.dll"; DestDir: "{app}\Icons"; Components: protocols\icq; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\Proto_IRC.dll"; DestDir: "{app}\Icons"; Components: protocols\irc; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\Proto_Jabber.dll"; DestDir: "{app}\Icons"; Components: protocols\jabber; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\Proto_MetaContacts.dll"; DestDir: "{app}\Icons"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\Proto_Yahoo.dll"; DestDir: "{app}\Icons"; Components: protocols\yahoo; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\xStatus_ICQ.dll"; DestDir: "{app}\Icons"; Components: protocols\icq; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\xStatus_Jabber.dll"; DestDir: "{app}\Icons"; Components: protocols\jabber; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\TabSRMM_icons.dll"; DestDir: "{app}\Icons"; Components: messagewin\tabsrmm; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\Toolbar_icons.dll"; DestDir: "{app}\Icons"; Components: clicts\modern; Flags: ignoreversion; AfterInstall: ShowPercent() 

; Core and core modules
Source: "Files\{#MirName}"; DestDir: "{app}"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\mir_core.dll"; DestDir: "{app}"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Pcre16.dll"; DestDir: "{app}"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Zlib.dll"; DestDir: "{app}"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\DbChecker.bat"; DestDir: "{app}"; Components: program; Check: IsPortable(); Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\msvcp100.dll"; DestDir: "{app}"; Components: program; Check: IsPortable(); Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\msvcr100.dll"; DestDir: "{app}"; Components: program; Check: IsPortable(); Flags: ignoreversion; AfterInstall: ShowPercent()
Source: "Files\Core\StdAuth.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdAutoAway.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdAway.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdChat.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdClist.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdCrypt.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdEmail.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdFile.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdHelp.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdIdle.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdMsg.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdUIHist.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdUrl.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdUserInfo.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\StdUserOnline.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 

; LangPacks
Source: "Files\Languages\langpack_czech.txt"; DestDir: "{app}\Languages"; Components: lang\czech; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Languages\langpack_german.txt"; DestDir: "{app}\Languages"; Components: lang\german; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Languages\langpack_polish.txt"; DestDir: "{app}\Languages"; Components: lang\polish; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Languages\langpack_russian.txt"; DestDir: "{app}\Languages"; Components: lang\russian; Flags: ignoreversion; AfterInstall: ShowPercent() 

;Plugins - not selectable
Source: "Files\Plugins\AdvaImg.dll"; DestDir: "{app}\Plugins"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\AVS.dll"; DestDir: "{app}\Plugins"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\DbChecker.dll"; DestDir: "{app}\Plugins"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\Import.dll"; DestDir: "{app}\Plugins"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\PluginUpdater.dll"; DestDir: "{app}\Plugins"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\pu_stub.exe"; DestDir: "{app}"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 

; Protocols
Source: "Files\Plugins\AIM.dll"; DestDir: "{app}\Plugins"; Components: protocols\aim; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\Facebook.dll"; DestDir: "{app}\Plugins"; Components: protocols\facebook; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\GG.dll"; DestDir: "{app}\Plugins"; Components: protocols\gg; Flags: ignoreversion; AfterInstall: ShowPercent()  
Source: "Files\Plugins\ICQ.dll"; DestDir: "{app}\Plugins"; Components: protocols\icq; Flags: ignoreversion; AfterInstall: ShowPercent()  
Source: "Files\Plugins\IRC.dll"; DestDir: "{app}\Plugins"; Components: protocols\irc; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\Jabber.dll"; DestDir: "{app}\Plugins"; Components: protocols\jabber; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\Yahoo.dll"; DestDir: "{app}\Plugins"; Components: protocols\yahoo; Flags: ignoreversion; AfterInstall: ShowPercent() 

; Clists
Source: "Files\Plugins\Clist_blind.dll"; DestDir: "{app}\Plugins"; Components: clicts\blind; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\Clist_modern.dll"; DestDir: "{app}\Plugins"; Components: clicts\modern; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\Clist_nicer.dll"; DestDir: "{app}\Plugins"; Components: clicts\nicer; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\Cln_skinedit.dll"; DestDir: "{app}\Plugins"; Components: clicts\nicer; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\TopToolBar.dll"; DestDir: "{app}\Plugins"; Components: clicts\modern or clicts\nicer; Flags: ignoreversion; AfterInstall: ShowPercent() 

; Message sessions
Source: "Files\Plugins\TabSRMM.dll"; DestDir: "{app}\Plugins"; Components: messagewin\tabsrmm; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\Scriver.dll"; DestDir: "{app}\Plugins"; Components: messagewin\scriver; Flags: ignoreversion; AfterInstall: ShowPercent() 

; DB drivers
Source: "Files\Plugins\Dbx_mmap.dll"; DestDir: "{app}\Plugins"; Components: dbx_mmap; Flags: ignoreversion; AfterInstall: ShowPercent() 

; Config files
Source: "Files\Settings\mirandaboot_default.ini"; DestDir: "{app}"; Components: program; DestName: "mirandaboot.ini"; Check: IsDefault(); Flags: ignoreversion onlyifdoesntexist; AfterInstall: ShowPercent() 
Source: "Files\Settings\mirandaboot_portable.ini"; DestDir: "{app}"; Components: program; DestName: "mirandaboot.ini"; Check: IsPortable(); Flags: ignoreversion onlyifdoesntexist; AfterInstall: ShowPercent() 
Source: "Files\Settings\autoexec_sounds.ini"; DestDir: "{app}"; Components: sounds; Flags: ignoreversion; AfterInstall: ShowPercent() 

; Installer add-ons
Source: "Installer\ISWin7.dll"; Flags: dontcopy 
Source: "Installer\descctrl.dll"; Flags: dontcopy
Source: "Installer\{#VcRedistName}"; DestDir: {tmp}; Flags: deleteafterinstall; Check: RedistIsNotInstalled

[Components]
Name: "program"; Description: "{cm:Program}"; Types: extended regular minimal custom; Flags: fixed 
Name: "sounds"; Description: "{cm:Sounds}"; Types: extended custom 
Name: "protocols"; Description: "{cm:Protocols}"; Types: extended regular minimal 
Name: "protocols\aim"; Description: "{cm:Aim}"; Types: extended regular minimal 
Name: "protocols\facebook"; Description: "{cm:Facebook}"; Types: extended regular minimal 
Name: "protocols\gg"; Description: "{cm:Gg}"; Types: extended regular minimal 
Name: "protocols\icq"; Description: "{cm:Icq}"; Types: extended regular minimal 
Name: "protocols\irc"; Description: "{cm:Irc}"; Types: extended regular minimal 
Name: "protocols\jabber"; Description: "{cm:Jabber}"; Types: extended regular minimal 
Name: "protocols\yahoo"; Description: "{cm:Yahoo}"; Types: extended regular minimal 
Name: "clists"; Description: "{cm:Clists}"; Types: extended regular minimal custom; Flags: fixed 
Name: "clicts\stdclist"; Description: "{cm:StdClist}"; Types: minimal; Flags: exclusive 
Name: "clicts\blind"; Description: "{cm:BlindClist}"; Types: custom; Flags: exclusive 
Name: "clicts\nicer"; Description: "{cm:NicerClist}"; Types: custom; Flags: exclusive 
Name: "clicts\modern"; Description: "{cm:ModernClist}"; Types: extended regular; Flags: exclusive 
Name: "messagewin"; Description: "{cm:MessageWin}"; Types: extended regular minimal custom; Flags: fixed 
Name: "messagewin\stdmsg"; Description: "{cm:StdMsg}"; Types: minimal; Flags: exclusive 
Name: "messagewin\scriver"; Description: "{cm:Scriver}"; Types: custom; Flags: exclusive 
Name: "messagewin\tabsrmm"; Description: "{cm:Tabsrmm}"; Types: extended regular; Flags: exclusive 
Name: "dbx_mmap"; Description: "{cm:Dbx_Mmap}"; Types: extended regular minimal
Name: "lang"; Description: "{cm:Lang}"; Types: extended regular custom minimal; Flags: fixed 
Name: "lang\russian"; Description: "{cm:LangRussian}"; Flags: exclusive 
Name: "lang\czech"; Description: "{cm:LangCzech}"; Flags: exclusive 
Name: "lang\polish"; Description: "{cm:LangPolish}"; Flags: exclusive 
Name: "lang\german"; Description: "{cm:LangGerman}"; Flags: exclusive 
Name: "lang\english"; Description: "{cm:LangEnglish}"; Flags: exclusive 

[Types]
Name: "regular"; Description: "{cm:TypeRegular}" 
Name: "minimal"; Description: "{cm:TypeMinimal}" 
Name: "extended"; Description: "{cm:TypeExtended}" 
Name: "custom"; Description: "{cm:TypeCustom}"; Flags: iscustom 

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked 
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked 

[Icons]
Name: "{app}\DbChecker"; Filename: "{app}\{#MirName}"; Parameters: "/svc:dbchecker"; Check: IsDefault() 
Name: "{group}\{#MirGroupName}"; Filename: "{app}\{#MirName}" 
Name: "{group}\{cm:UninstallProgram,Miranda NG}"; Filename: {app}\Uninstall\Unins000.exe 
Name: "{userdesktop}\{#MirGroupName}"; Filename: "{app}\{#MirName}"; WorkingDir: {app}; Tasks: desktopicon 
Name: "{userappdata}\Microsoft\Internet Explorer\Quick Launch\{#MirGroupName}"; Filename: "{app}\{#MirName}"; WorkingDir: {app}; Tasks: quicklaunchicon 

[Run]
Filename: "{tmp}\{#VcRedistName}"; Parameters: "/passive /Q:a /c:""msiexec /qb /i vcredist.msi"" "; Check: RedistIsNotInstalled(); StatusMsg: Installing Microsoft Visual C++ 2010 SP1 Redistributable Package ({#Ptf})
Filename: "{app}\{#MirName}"; Description: "{cm:LaunchProgram,Miranda NG}"; Flags: nowait postinstall skipifsilent 

[UninstallDelete]
Type: filesandordirs; Name: {app} 

[CustomMessages]
; English
en.InstTypeHeader=Installation Mode
en.InstTypeText=Select installation type
en.DefTypeInstRadio=Normal Installation (recommended)
en.PortTypeInstRadio=Portable Installation
en.DefTypeInstLabel=Profiles and user data files are stored in Application Data.%nUAC compatible (Windows Vista/7/8)
en.PortTypeInstLabel=Profiles and user data files are stored in program folder.%nVersion for installation on removable storage devices (such as USB flash drive)%n(Do not install to Program Files with enabled UAC!!!)

en.ProfileUninst=WARNING!!!%nYou are about to delete profile folder containing all your settings, contacts and message history!%nAre you sure you want to remove profile?%n(You may need it later)

en.FolderExists1=Folder
en.FolderExists2=already exists. This installer is for a first time install only! Please select another folder for a new installation.%nIf you wish to upgrade your current program, please use PluginUpdater ("Check for plugin updates" item in Miranda's main menu).

en.PercentDone=% done

en.TypeRegular=Regular installation
en.TypeMinimal=Minimal installation
en.TypeExtended=Extended installation
en.TypeCustom=Custom installation 

en.Program=Core components
en.Sounds=Sound files
en.Protocols=Protocols
en.Aim=AIM protocol
en.Facebook=Facebook protocol
en.GG=GG protocol
en.Icq=ICQ protocol
en.Irc=IRC protocol
en.Jabber=Jabber protocol
en.Yahoo=Yahoo protocol
en.Clists=Contact lists
en.StdClist=StdClist contact list
en.BlindClist=Contact list for blind folks
en.NicerClist=Nicer contact list
en.ModernClist=Modern contact list
en.MessageWin=Message session
en.StdMsg=StdMsg and StdChat plugins
en.Scriver=Scriver message plugin
en.Tabsrmm=TabSRMM message plugin
en.Dbx_Mmap=Dbx_mmap database driver
en.Lang=Localization files
en.LangRussian=Russian langpack
en.LangCzech=Czech langpack
en.LangPolish=Polish langpack
en.LangGerman=German langpack
en.LangEnglish=English interface

en.DescriptionTitle=Description
en.InfoCaption=Hover over the component to get additional information
en.CoreFilesDescription=The minimum set of files needed to run the program.
en.SoundsDescription=Sound notification support for various events.
en.ProtocolsDescription=Various protocols for instant messaging and more.
en.ProtocolsAIMDescription=AOL Instant Messenger (AIM) protocol support for Miranda NG.
en.ProtocolsFbDescription=Facebook protocol support for Miranda NG. More advanced than XMPP facebook chat, supporting posting statuses, authorizations, searching contacts, and much more.
en.ProtocolsGGDescription=Gadu-Gadu protocol support for Miranda NG.
en.ProtocolsICQDescription=ICQ (OSCAR) protocol support for Miranda NG.
en.ProtocolsIRCDescription=Internet Relay Chat (IRC) protocol support for Miranda NG.
en.ProtocolsJabberDescription=Open-standard communications protocol (XMPP) for message-oriented middleware based on XML.
en.ProtocolsYahooDescription=Yahoo protocol support for Miranda NG.
en.ClistsDescription=Contact list plugins.
en.StdClistDescription=Basic contact list core module.
en.BlindClistDescription=This is a contact list for blind folks. It uses a list control to show all contacts, so screen readers can "read" the clist to the user.
en.ModernClistDescription=Supports skins, displays contacts, event notifications, protocol status with advantage visual modifications. Supports MW modifications, enhanced metacontact cooperation.
en.NicerClistDescription=Displays contacts, event notifications, protocol status.
en.MessageWinDescription=Message session plugins.
en.StdMsgDescription=Basic messaging and group chats core modules. No tabs support.
en.ScriverDescription=Easy and lightweight messaging plugin.
en.TabsrmmDescription=Advanced IM and group chat module for Miranda NG. Supports skins.
en.MMAPDescription=Provides Miranda database support: global settings, contacts, history, settings per contact.(Recommended)
en.LangDescription=Localization files for Miranda NG.
en.LangRussianDescription=Translation of Miranda NG user interface to Russian.
en.LangCzechDescription=Translation of Miranda NG user interface to Czech.
en.LangPolishDescription=Translation of Miranda NG user interface to Polish.
en.LangGermanDescription=Translation of Miranda NG user interface to German.
en.LangEnglishDescription=Original Miranda NG user interface in English.

; Russian
ru.InstTypeHeader=Тип установки
ru.InstTypeText=Выберите тип установки программы.
ru.DefTypeInstRadio=Обычная установка (рекомендуется)
ru.PortTypeInstRadio=Портативная установка
ru.DefTypeInstLabel=Профиль и все необходимые файлы хранятся в папке пользователя%n(Application Data).%nСовместимо с UAC (Windows Vista/7/8)
ru.PortTypeInstLabel=Профиль и файлы пользователя находятся в папке с программой.%nПодходит для установки на Flash-накопители, не оставляет следов в ОС.%n(Не устанавливать в Program Files при включенном UAC!!!)

ru.ProfileUninst=ПРЕДУПРЕЖДЕНИЕ!!!%nВы можете удалить папку профиля, включающую в себя настройки, контакты и историю переписки!%nВы действительно хотите удалить файлы профиля?%n(Может понадобиться в дальнейшем)

ru.FolderExists1=Выбранная папка
ru.FolderExists2=включает в себя предыдущую установку! Установщик не предназначен для обновления. Пожалуйста, выберите иную папку для новой установки.%nДля обновления уже установленной программы воспользуйтесь плагином PluginUpdater (Пункт "Проверить обновления плагинов" в главном меню).

ru.PercentDone=% выполнено

ru.TypeRegular=Обычная установка
ru.TypeMinimal=Минимальная установка
ru.TypeExtended=Расширенная установка
ru.TypeCustom=Выборочная установка

ru.Program=Основные файлы
ru.Sounds=Звуки
ru.Protocols=Протоколы
ru.Aim=Протокол AIM
ru.Facebook=Протокол Facebook
ru.GG=Протокол GG
ru.Icq=Протокол ICQ
ru.Irc=Протокол IRC
ru.Jabber=Протокол Jabber
ru.Yahoo=Протокол Yahoo
ru.Clists=Списки контактов
ru.StdClist=Список контактов StdClist
ru.BlindClist=Список контактов для слабовидящих
ru.NicerClist=Список контактов Nicer
ru.ModernClist=Список контактов Modern
ru.MessageWin=Диалоговые окна
ru.StdMsg=Плагины StdMsg и StdChat
ru.Scriver=Диалоговое окно Scriver
ru.Tabsrmm=Диалоговое окно TabSRMM
ru.Dbx_Mmap=Драйвер Dbx_mmap
ru.Lang=Языковые пакеты
ru.LangRussian=Русский
ru.LangCzech=Чешский
ru.LangPolish=Польский
ru.LangGerman=Немецкий
ru.LangEnglish=Английский

ru.DescriptionTitle=Описание
ru.InfoCaption=Наведите на компонент, чтобы получить дополнительную информацию
ru.CoreFilesDescription=Минимальный набор файлов, необходимый для работы программы.
ru.SoundsDescription=Звуковые эффекты, проигрываемые при различных событиях в программе.
ru.ProtocolsDescription=Различные протоколы для обмена сообщениями и не только.
ru.ProtocolsAIMDescription=Поддержка протокола AOL Instant Messenger (AIM) в Miranda NG.
ru.ProtocolsFbDescription=Поддержка протокола Facebook в Miranda NG. Поддержка статусов, поиск контактов, авторизация и не только.
ru.ProtocolsGGDescription=Поддержка протокола Gadu-Gadu в Miranda NG.
ru.ProtocolsICQDescription=Поддержка протокола ICQ в Miranda NG.
ru.ProtocolsIRCDescription=Поддержка протокола Internet Relay Chat (IRC) в Miranda NG.
ru.ProtocolsJabberDescription=Поддержка протокола Jabber (XMPP) в Miranda NG. Протокол основан на открытых стандартах, базирующихся на XML.
ru.ProtocolsYahooDescription=Поддержка протокола Yahoo в Miranda NG.
ru.ClistsDescription=Плагины списков контактов.
ru.StdClistDescription=Список контактов, являющийся модулем ядра, обеспечивает базовый функционал.
ru.BlindClistDescription=Список контактов для слабовидящих. Вспомогательные программы для людей с проблемами зрения могут легко "считать" информацию пользователю.
ru.ModernClistDescription=Показывает контакты, события, статусы протоколов с расширенными функциями оформления (скины). Мультиоконная модификация, расширенная поддержка метаконтактов.
ru.NicerClistDescription=Показывает контакты, события, статусы протоколов.
ru.MessageWinDescription=Плагины диалоговых окон.
ru.StdMsgDescription=Модули ядра, обеспечивающие базовый функционал переписки и чатов. Нет поддержки табов.
ru.ScriverDescription=Простой плагин диалоговых окон с поддержкой вкладок. Не поддерживает скины.
ru.TabsrmmDescription=Продвинутый плагин диалоговых окон с поддержкой вкладок и скинов.
ru.MMAPDescription=Поддержка базы Miranda NG: глобальные настройки, контакты, история, настройки контактов и т. п. Отличается меньшим количеством обращений к базе данных в процессе работы.(Рекомендуется).
ru.LangDescription=Файлы перевода для Miranda NG.
ru.LangRussianDescription=Перевод интерфейса Miranda NG на русский язык.
ru.LangCzechDescription=Перевод интерфейса Miranda NG на чешский язык.
ru.LangPolishDescription=Перевод интерфейса Miranda NG на польский язык.
ru.LangGermanDescription=Перевод интерфейса Miranda NG на немецкий язык.
ru.LangEnglishDescription=Интерфейс Miranda NG по умолчанию — английский.

; Czech
cz.InstTypeHeader=Režim instalace
cz.InstTypeText=Zvolte režim instalace
cz.DefTypeInstRadio=Běžná instalace (doporučeno)
cz.PortTypeInstRadio=Přenositelná verze
cz.DefTypeInstLabel=Profily a uživatelská data jsou uložena ve složce Data aplikací.%nKompatibilní se zapnutým Řízením uživatelských účtů (UAC) (Windows Vista/7/8)
cz.PortTypeInstLabel=Profily a uživatelská data jsou uložena ve složce spolu s programem.%nTento způsob je vhodný pro přenosné disky (jako USB flash disky)%n(Neinstalujte do Program Files, pokud máte zapnuté UAC!!!)

cz.ProfileUninst=VAROVÁNÍ!!!%nChystáte se odstranit složku s profily, která obsahuje všechno vaše nastavení, kontakty a historii komunikace!%nOpravdu si přejete odstranit vaše data?%n(V budoucnu se vám ještě mohou hodit.)

cz.FolderExists1=Složka
cz.FolderExists2=již existuje. Tento instalátor slouží pouze pro prvotní instalaci! Zvolte prosím jinou složku, pokud si přejete provést novou instalaci.%nPokud si přejete aktualizovat vaši stávající verzi, použijte doplněk PluginUpdater (položka "Zkontrolovat aktualizace" v hlavní nabídce Mirandy).

cz.PercentDone=% dokončeno

cz.TypeRegular=Běžná instalace
cz.TypeMinimal=Minimální instalace
cz.TypeExtended=Rozšířená instalace
cz.TypeCustom=Vlastní instalace

cz.Program=Základní komponenty
cz.Sounds=Zvukové soubory
cz.protocols=Protokoly
cz.Aim=AIM protokol
cz.Facebook=Facebook protokol
cz.GG=GG protokol
cz.Icq=ICQ protokol
cz.Irc=IRC protokol
cz.Jabber=Jabber protokol
cz.Yahoo=Yahoo protokol
cz.Clists=Seznamy kontaktů
cz.StdClist=StdClist
cz.BlindClist=Clist_blind (pro nevidomé)
cz.NicerClist=Clist_nicer
cz.ModernClist=Clist_modern
cz.MessageWin=Komunikační okna
cz.StdMsg=StdMsg a StdChat
cz.Scriver=Scriver
cz.Tabsrmm=TabSRMM
cz.Dbx_Mmap=Dbx_mmap
cz.Lang=Jazykové rozhraní
cz.LangRussian=Ruština
cz.LangCzech=Čeština
cz.LangPolish=Polština
cz.LangGerman=Němčina
cz.LangEnglish=Angličtina

cz.DescriptionTitle=Popis
cz.InfoCaption=Najeďte myší na název komponenty pro zobrazení více informací
cz.CoreFilesDescription=Balíček základních komponent vyžadovaných pro spuštění programu.
cz.SoundsDescription=Podpora zvukových efektů pro různé události.
cz.protocolsDescription=Různé protokoly pro rychlou komunikaci a další funkce.
cz.protocolsAIMDescription=AOL Instant Messenger (AIM) protokol pro Mirandu NG. 
cz.protocolsFbDescription=Facebook protokol pro Mirandu NG. Pokročilejší verze než Facebook chat přes Jabber (XMPP). Podporuje sdílení stavů, neviditelnost, autorizace, vyhledávání kontaktů a mnohem více.
cz.protocolsGGDescription=Gadu-Gadu protokol pro Mirandu NG. 
cz.protocolsICQDescription=ICQ (OSCAR) protokol pro Mirandu NG. 
cz.protocolsIRCDescription=Internet Relay Chat (IRC) protokol pro Mirandu NG.
cz.protocolsJabberDescription=Otevřený komunikační protokol Jabber (XMPP) pro Mirandu NG.
cz.protocolsYahooDescription=Yahoo protokol pro Mirandu NG.
cz.ClistsDescription=Doplňky pro seznamy kontaktů.
cz.StdClistDescription=Základní integrovaný modul pro seznam kontaktů.
cz.BlindClistDescription=Seznam kontaktů pro nevidomé uživatele. Pro zobrazení kontaktů využívá jednoduchý seznam, který může být jednoduše přečten.
cz.ModernClistDescription=Podporuje skiny, zobrazuje kontakty, upozornění na události, stavy protokolů a umožňuje pokročilé úpravy vzhledu. Podporuje rámečky a rozšířenou práci s metakontakty.
cz.NicerClistDescription=Zobrazuje kontakty, upozornění na události, stavy protokolů a podporuje rámečky.
cz.MessageWinDescription=Doplňky pro komunikační okna.
cz.StdMsgDescription=Základní integrovaný modul pro komunikaci a skupinové chaty. Bez podpory více záložek v jednom okně.
cz.ScriverDescription=Jednoduchý a rychlý modul pro komunikaci a skupinové chaty. S podporou více záložek v jednom okně.
cz.TabsrmmDescription=Pokročilý modul pro komunikaci a skupinové chaty. Podporuje skiny a více záložek v jednom okně.
cz.MMAPDescription=Podpora pro databáze Mirandy: globální nastavení, kontakty, historie, nastavení pro jednotlivé kontakty.
cz.LangDescription=Jazykové balíčky pro Mirandu NG.
cz.LangRussianDescription=Překlad rozhraní Mirandy NG do ruštiny.
cz.LangCzechDescription=Překlad rozhraní Mirandy NG do češtiny.
cz.LangPolishDescription=Překlad rozhraní Mirandy NG do polštiny.
cz.LangGermanDescription=Překlad rozhraní Mirandy NG do němčiny.
cz.LangEnglishDescription=Výchozí rozhraní Mirandy NG v angličtině.

; Deutsch
de.InstTypeHeader=Installationsmodus
de.InstTypeText=Wählen Sie die Art der Installation aus
de.DefTypeInstRadio=Normale Installation (empfohlen)
de.PortTypeInstRadio=Portable Installation
de.DefTypeInstLabel=Profil- und Benutzerdaten werden in Anwendungsdaten gespeichert.%nUAC-kompatibel (Windows Vista/7/8).
de.PortTypeInstLabel=Profil- und Benutzerdaten werden im Programmverzeichnis gespeichert.%nDies ist die Version zur Verwendung auf portablen Laufwerken (wie USB-Sticks).%n(Installieren Sie Miranda NG mit aktivierter UAC nicht im Programme-Ordner!)

de.ProfileUninst=WARNUNG!%nSie sind dabei, Ihr Profil mitsamt all Ihren Einstellungen, Kontakten und Nachrichten zu löschen!%nSind Sie sich sicher, dass Sie das Profil löschen möchten?%n(Sie könnten es später brauchen.)

de.FolderExists1=Verzeichnis
de.FolderExists2=ist bereits vorhanden. Dieses Installationsprogramm ist nur für die erstmalige Installation geeignet! Bitte wählen Sie ein anderes Verzeichnis für eine Neuinstallation aus.%nFalls Sie Ihre aktuelle Installation aktualisieren möchten, verwenden Sie bitte den Plugin-Updater ("Auf Plugin-Aktualisierungen prüfen" im Miranda-Hauptmenü).

de.PercentDone=% erledigt

de.TypeRegular=Übliche Installation
de.TypeMinimal=Minimale Installation
de.TypeExtended=Erweiterte Installation
de.TypeCustom=Benutzerdefinierte Installation 

de.Program=Kernkomponenten
de.Sounds=Klangdateien
de.Protocols=Protokolle
de.Aim=AIM-Protokoll
de.Facebook=Facebook-Protokoll
de.GG=GG-Protokoll
de.Icq=ICQ-Protokoll
de.Irc=IRC-Protokoll
de.Jabber=Jabber-Protokoll
de.Yahoo=Yahoo-Protokoll
de.Clists=Kontaktlisten
de.StdClist=Klassische Kontaktliste
de.BlindClist=Kontaktliste für Blinde
de.NicerClist=Nicer-Kontaktliste
de.ModernClist=Moderne Kontaktliste
de.MessageWin=Nachrichtensitzungen
de.StdMsg=StdMsg- und StdChat-Plugins
de.Scriver=Scriver-Nachrichtenplugin
de.Tabsrmm=TabSRMM-Nachrichtenplugin
de.Dbx_Mmap=Dbx_mmap-Datenbanktreiber
de.Lang=Sprachdateien
de.LangRussian=Russische Sprachdatei
de.LangCzech=Tschechische Sprachdatei
de.LangPolish=Polnische Sprachdatei
de.LangGerman=Deutsche Sprachdatei
de.LangEnglish=Englischsprachige Oberfläche

de.DescriptionTitle=Beschreibung
de.InfoCaption=Fahren Sie mit der Maus über eine Komponente, um ihre Beschreibung anzeigen zu lassen.
de.CoreFilesDescription=Die für den Betrieb von Miranda NG notwendigen Dateien.
de.SoundsDescription=Klangbenachrichtigungen für eine Vielzahl an Ereignissen.
de.ProtocolsDescription=Verschiedene Protokolle für Sofortnachrichten und mehr.
de.ProtocolsAIMDescription=AOL-Instant-Messenger- (AIM-)Unterstützung für Miranda NG.
de.ProtocolsFbDescription=Facebook-Protokollunterstützung für Miranda NG. Funktionsreicher als der XMPP-Facebook-Chat, unterstützt das Veröffentlichen neuer Statusnachrichren, Autorisierung, Kontaktsuche und vieles mehr.
de.ProtocolsGGDescription=Gadu-Gadu-Protokollunterstützung für Miranda NG.
de.ProtocolsICQDescription=ICQ- (OSCAR-)Protokollunterstützung für Miranda NG.
de.ProtocolsIRCDescription=Internet-Relay-Chat- (IRC-)Protokollunterstützung für Miranda NG.
de.ProtocolsJabberDescription=Unterstützung des Extensible Messaging and Presence Protocols (XMPP) für Miranda NG.
de.ProtocolsYahooDescription=Yahoo-Protokollunterstützung für Miranda NG.
de.ClistsDescription=Kontaktlistenplugins.
de.StdClistDescription=Grundlegendes Kontaktlisten-Kernmodul.
de.BlindClistDescription=Dies ist eine Kontaktliste für Blinde. Sie benutzt eine Liste, um alle Kontakte anzuzeigen, so dass Bildschirmleser die Kontaktliste dem Benutzer "vorlesen" können.
de.ModernClistDescription=Unterstützt Skins, zeigt Kontakte, Ereignisbenachrichtigungen und Protokollstatus mit visuellen Verbesserungen an. Unterstützt MW-Modifikationen, erweiterte Zusammenarbeit mit Metakontakten.
de.NicerClistDescription=Zeigt Kontakte, Ereignisbenachrichtigungen und Protokollstatus an.
de.MessageWinDescription=Nachrichtensitzungsplugins.
de.StdMsgDescription=Grundlegende Nachrichten- und Gruppenchat-Kernmodule. Keine Tabunterstützung.
de.ScriverDescription=Einfaches und leichtgewichtiges Nachrichtenplugin.
de.TabsrmmDescription=Erweitertes Nachrichten- und Gruppenchatmodul für Miranda NG. Unterstützt Skins.
de.MMAPDescription=Stellt Miranda-Datenbankunterstützung bereit: globale Einstellungen, Kontakte, Verlauf, Per-Kontakt-Einstellungen.
de.LangDescription=Sprachdateien für Miranda NG.
de.LangRussianDescription=Übersetzung der Miranda-NG-Benutzeroberfläche ins Russische.
de.LangCzechDescription=Übersetzung der Miranda-NG-Benutzeroberfläche ins Tschechische.
de.LangPolishDescription=Übersetzung der Miranda-NG-Benutzeroberfläche ins Polnische.
de.LangGermanDescription=Übersetzung der Miranda-NG-Benutzeroberfläche ins Deutsche.
de.LangEnglishDescription=Original-Miranda-NG-Benutzeroberfläche auf Englisch.

; Polish
pl.InstTypeHeader=Typ instalacji
pl.InstTypeText=Wybierz typ instalacji
pl.DefTypeInstRadio=Normalna (zalecana)
pl.PortTypeInstRadio=Przenośna
pl.DefTypeInstLabel=Profile i pliki danych użytkownika są przechowywane w folderze Dane aplikacji.%nZgodne z Kontrolą konta uzytkownika (Windows Vista/7/8)
pl.PortTypeInstLabel=Profile i pliki danych użytkownika są przechowywane w folderze programu.%nWersja do instalowania na urządzeniach przenośnych (takich jak pamięć przenośna)%n(Nie instaluj w Program files/Pliki programów z włączoną Kontrolą konta użytkownika!!!)

pl.ProfileUninst=UWAGA!!!%nZamierzasz usunąć folder z profilami, zawierający wszystkie Twoje ustawienia, kontakty i historię rozmów.%nCzy na pewno chcesz go usunąć?%n(Możesz go później potrzebować)

pl.FolderExists1=Folder
pl.FolderExists2=już istnieje. Ten instalator służy tylko do czystej instalacji! Wybierz inny folder aby zainstalować program.%nJeżeli chcesz zaktualizować Mirandę, użyj do tego celu wtyczki PluginUpdater (wybierając "Sprawdź aktualizacje" z menu głównego Mirandy).

pl.PercentDone=% ukończono

pl.TypeRegular=Typowa instalacja
pl.TypeMinimal=Minimalna instalacja
pl.TypeExtended=Rozszerzona instalacja
pl.TypeCustom=Własna instalacja

pl.Program=Pliki rdzenia
pl.Sounds=Dźwięki
pl.Protocols=Protokoły
pl.Aim=AIM
pl.Facebook=Facebook
pl.GG=GG
pl.Icq=ICQ
pl.Irc=IRC
pl.Jabber=Jabber
pl.Yahoo=Yahoo
pl.Clists=Lista kontaktów
pl.StdClist=Lista klasyczna (StdClist)
pl.BlindClist=Dla niewidomych (Blind)
pl.NicerClist=Lista Nicer
pl.ModernClist=Lista Modern
pl.MessageWin=Okno wiadomości
pl.StdMsg=StdMsg i StdChat
pl.Scriver=Scriver
pl.Tabsrmm=TabSRMM
pl.Dbx_Mmap=Dbx_mmap
pl.Lang=Język interfejsu
pl.LangRussian=rosyjski
pl.LangCzech=czeski
pl.LangPolish=polski
pl.LangGerman=niemiecki
pl.LangEnglish=angielski

pl.DescriptionTitle=Opis
pl.InfoCaption=Najedź na element w celu dodatkowych informacji
pl.CoreFilesDescription=Pliki konieczne do uruchomienia Mirandy.
pl.SoundsDescription=Paczka predefiniowanych dźwięków dla różnych zdarzeń.
pl.ProtocolsDescription=Wtyczki zapewniające wspracie dla różnych protokołów.
pl.ProtocolsAIMDescription=Zapewnia obsługę protokołu AOL Instant Messenger (AIM).
pl.ProtocolsFbDescription=Zapewnia obsługę protokołu Facebook. Dostarcza więcej niż czat facebooka przez XMPP. Wspiera ustawianie statusu, zatwierdzanie znajomości, wyszukiwanie kontaktów i wiele inncyh.
pl.ProtocolsGGDescription=Zapewnia obsługę protokołu Gadu-Gadu.
pl.ProtocolsICQDescription=Zapewnia obsługę protokołu ICQ (OSCAR).
pl.ProtocolsIRCDescription=Zapewnia obsługę protokołu Internet Relay Chat (IRC).
pl.ProtocolsJabberDescription=Zapewnia obsługę protokołu Jabber/XMPP.
pl.ProtocolsYahooDescription=Zapewnia obsługę protokołu Yahoo.
pl.ClistsDescription=Wtyczki listy kontaktów.
pl.StdClistDescription=Podstawowy moduł listy kontaktów zawarty w rdzeniu.
pl.BlindClistDescription=Jest to lista kontaktów dla ludzi niewidomych. Używa kontrolki listy, aby pokazać wszystkie kontakty, dzięki temu czytnik ekranu może "czytać" listę użtkownikowi.
pl.ModernClistDescription=Wspiera skórki. Wyświetla kontakty, powiadomienia zdarzeń, statusy protokołów z dobrymi modyfikacjami wizualnymi. Wspierane zmiany z MW, rozszerzona współpraca z metakontaktami.
pl.NicerClistDescription=Wyświetla kontakty, powiadomienia zdarzeń, statusy protokołów.
pl.MessageWinDescription=Wtyczki okna wiadomości.
pl.StdMsgDescription=Podstawowe moduły rdzenia do wiadomości i czatu. Brak wsparcia zakładek.
pl.ScriverDescription=Łatwa i lekka wtyczka rozmów.
pl.TabsrmmDescription=Zaawansowana wtyczka rozmów z modułem czatu. Wspiera skórki.
pl.MMAPDescription=Zapewnia wsparcie profilu Mirandy: ustawień globalnych, kontaktów, historii, ustawień dla każdego kontaktu z osobna.
pl.LangDescription=Pakiety językowe dla Mirandy NG.
pl.LangRussianDescription=Tłumaczy interfejs Mirandy na język rosyjski.
pl.LangCzechDescription=Tłumaczy interfejs Mirandy na język czeski.
pl.LangPolishDescription=Tłumaczy interfejs Mirandy na język polski.
pl.LangGermanDescription=Tłumaczy interfejs Mirandy na język niemiecki.
pl.LangEnglishDescription=Domyślny (angielski) język interfejsu.

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "cz"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"

[Code]
// Global variables
var
  DefTypeInstLabel, PortTypeInstLabel, ProgressLabel: TLabel;
  DefTypeInstRadio, PortTypeInstRadio: TRadioButton;
  InstallTypePage: TWizardPage;
  ComponentInfo: TNewStaticText;
  ComponentList: TStringList;

//--Functions called from external DLLs--

// Aero effects
procedure iswin7_add_glass(Handle:HWND; Left, Top, Right, Bottom : Integer; GDIPLoadMode: boolean);
external 'iswin7_add_glass@files:iswin7.dll stdcall';
procedure iswin7_add_button(Handle:HWND);
external 'iswin7_add_button@files:iswin7.dll stdcall';
procedure iswin7_free;
external 'iswin7_free@files:iswin7.dll stdcall';

// Components description
function EnableDescription(ComponentsListHandle: HWND; DescLabelHandle: HWND; DescStrings: PAnsiChar): BOOL;
external 'enabledesc@files:descctrl.dll stdcall';
function DisableDescription: BOOL;
external 'disabledesc@files:descctrl.dll stdcall';

// Getting a list of all logical disk drives
function GetLogicalDriveStrings(nLenDrives: LongInt; lpDrives: ansistring): integer;
external 'GetLogicalDriveStringsA@kernel32.dll stdcall';

// Determine disk drive type (removable, fixed, CD-ROM, RAM disk, or network)
function GetDriveType(lpDisk: ansistring): integer;
external 'GetDriveTypeA@kernel32.dll stdcall';

// Drive type constants
const
  DRIVE_UNKNOWN = 0;     // The drive type cannot be determined.
  DRIVE_NO_ROOT_DIR = 1; // The root path is invalid. For example, no volume is mounted at the path.
  DRIVE_REMOVABLE = 2;   // The disk can be removed from the drive.
  DRIVE_FIXED = 3;       // The disk cannot be removed from the drive.
  DRIVE_REMOTE = 4;      // The drive is a remote (network) drive.
  DRIVE_CDROM = 5;       // The drive is a CD-ROM drive.
  DRIVE_RAMDISK = 6;     // The drive is a RAM disk.

// Default installation check
function IsDefault: Boolean;
begin
  if (DefTypeInstRadio.Checked) then
    Result:= True;
end;

// Portable installation check
function IsPortable: Boolean;
begin
  if (PortTypeInstRadio.Checked) then
    Result:= True;
end;

// Visual C++ redistributable package installation check
function RedistIsNotInstalled: Boolean;
begin
   Result := not RegKeyExists{#RedistRegChk};
end;

// Installation type page creation (default or portable)
procedure CreateInstallTypePage();
begin
  InstallTypePage:=CreateCustomPage(wpLicense, ExpandConstant('{cm:InstTypeHeader}'), ExpandConstant('{cm:InstTypeText}'));
  DefTypeInstRadio:=TRadioButton.Create(InstallTypePage);
  with DefTypeInstRadio do
  begin
    Parent:=InstallTypePage.Surface;
    Checked:=True;
    Top:=35;
    Left:=20;
    Width:=210;
    Caption:=ExpandConstant('{cm:DefTypeInstRadio}');
  end;
  DefTypeInstLabel:=TLabel.Create(InstallTypePage);
  with DefTypeInstLabel do
  begin
    Parent:=InstallTypePage.Surface;
    Top:=DefTypeInstRadio.Top+DefTypeInstRadio.Height+2;
    Left:=DefTypeInstRadio.Left+15;
    Caption:=ExpandConstant('{cm:DefTypeInstLabel}');
  end;
  PortTypeInstRadio:=TRadioButton.Create(InstallTypePage);
  with PortTypeInstRadio do
  begin
    Parent:=InstallTypePage.Surface;
    Checked:=False;
    Top:=DefTypeInstLabel.Top+DefTypeInstLabel.Height+20;
    Left:=20;
    Width:=150;
    Caption:=ExpandConstant('{cm:PortTypeInstRadio}');
  end;
  PortTypeInstLabel:=TLabel.Create(InstallTypePage);
  with PortTypeInstLabel do
  begin
    Parent:=InstallTypePage.Surface;
    Top:=PortTypeInstRadio.Top+PortTypeInstRadio.Height+2;
    Left:=PortTypeInstRadio.Left+15;
    Caption:=ExpandConstant('{cm:PortTypeInstLabel}');
  end;
end;

// Components info-panel 
procedure CreateComponentsInfoPanel();
var
  InfoCaption: TNewStaticText;
  InfoPanel: TPanel;
begin
  WizardForm.TypesCombo.Width := ScaleX(234);
  WizardForm.ComponentsList.Width := ScaleX(234);

  InfoPanel := TPanel.Create(WizardForm);
  InfoPanel.Parent := WizardForm.SelectComponentsPage;
  InfoPanel.Caption := '';
  InfoPanel.Top := WizardForm.TypesCombo.Top;
  InfoPanel.Left := ScaleX(250);
  InfoPanel.Width := ScaleX(166);
  InfoPanel.Height := WizardForm.ComponentsList.Height + WizardForm.TypesCombo.Height + 5;
  InfoPanel.BevelInner := bvRaised;
  InfoPanel.BevelOuter := bvLowered;
  InfoCaption := TNewStaticText.Create(WizardForm);
  InfoCaption.Parent := WizardForm.SelectComponentsPage;
  InfoCaption.Caption := ExpandConstant('{cm:DescriptionTitle}');
  InfoCaption.Left := ScaleX(258);
  InfoCaption.Top := InfoPanel.Top - ScaleY(5);
  //InfoCaption.Font.Color := clActiveCaption;

  ComponentInfo := TNewStaticText.Create(WizardForm);
  ComponentInfo.Parent := InfoPanel;
  ComponentInfo.AutoSize := False;
  ComponentInfo.Left := ScaleX(6);
  ComponentInfo.Width := ScaleX(154);
  ComponentInfo.Top := ScaleY(12);
  ComponentInfo.Height := WizardForm.ComponentsList.Height - ScaleY(17);
  ComponentInfo.Caption := ExpandConstant('{cm:InfoCaption}');
  ComponentInfo.WordWrap := true;
end;

// Choosing installation folder
procedure CheckFolder();
var
  drivesletters, drive: ansistring;
  lenletters, disktype, posnull, n: integer;
  FreeMB, TotalMB: Cardinal;
begin
  if (DefTypeInstRadio.Checked) then
    WizardForm.DirEdit.Text := {#MirPfInstDir} + '\Miranda NG';

  if (PortTypeInstRadio.Checked) then
  begin
    // Getting all the disk drives
    drivesletters := StringOfChar(' ', 64);
    lenletters := GetLogicalDriveStrings(63, drivesletters);
    SetLength(drivesletters , lenletters);
    drive := '';
    n := 0;
    // Checking all disks in cycle
    while ((Length(drivesletters) > 0)) do
    begin
      posnull := Pos(#0, drivesletters);
        if posnull > 0 then
        begin
        drive:= UpperCase(Copy(drivesletters, 1, posnull - 1));
        // Getting disk drive type
        disktype := GetDriveType(drive);
        // If removable, checking available space and suggesting to install on it
        if  (disktype = DRIVE_REMOVABLE) then
        begin
          GetSpaceOnDisk(drive, True, FreeMB, TotalMB);
          if (FreeMB > 26) then
          begin
            WizardForm.DirEdit.Text:=drive+'Miranda NG';
            break;
          end;
        end;
        Delete(drivesletters, 1, posnull);
        posnull := Pos(#0, drivesletters);
      end;
    end;
    if (drivesletters='') and (CompareStr(WizardForm.DirEdit.Text, drive+'Miranda NG') <> 0) then
      WizardForm.DirEdit.Text := ExpandConstant('{sd}')+'\Miranda NG';
  end;
end;

// Start Menu group
procedure CheckGroup();
begin
  if DefTypeInstRadio.Checked then
    WizardForm.NoIconsCheck.Checked := False;

  if PortTypeInstRadio.Checked then
    WizardForm.NoIconsCheck.Checked := True;
end;

// Checking if at least one protocol selected to be installed
procedure ComponentOnClick(Sender: TObject);
begin
  if (Pos(ExpandConstant('{cm:Protocols}'), ' ' + WizardSelectedComponents(True)) = 0) then
    begin
      WizardForm.NextButton.Enabled := False;
    end
  else
    WizardForm.NextButton.Enabled := True;
end;

// Checking if at least one protocol selected to be installed
procedure ComponentOnKeyPress(Sender: TObject; var Key: Char);
begin
  if Key = ' ' then
    if (Pos(ExpandConstant('{cm:Protocols}'), ' ' + WizardSelectedComponents(True)) = 0) then
      begin
        WizardForm.NextButton.Enabled := False;
      end
    else
      WizardForm.NextButton.Enabled := True;
end;

// Calculation and output of installation process in percents to progress label
procedure ShowPercent();
begin
  with WizardForm.ProgressGauge do
    ProgressLabel.Caption:= IntToStr((Position-Min)/((Max - Min)/100)) + ExpandConstant('{cm:PercentDone}');
end;

// Initialization beginning
procedure InitializeWizard();
var
  Version: TWindowsVersion;
begin
  // Shrinking bottom border (looks nicer this way)
  WizardForm.Bevel.Height := 1;

  GetWindowsVersionEx(Version);
  if Version.NTPlatform and
     (Version.Major <= 6) and
     (Version.Minor < 2) then
  begin
    // Initializing library
    iswin7_add_button(WizardForm.BackButton.Handle);
    iswin7_add_button(WizardForm.NextButton.Handle);
    iswin7_add_button(WizardForm.CancelButton.Handle);
    //Do not touch 'True' parameter, it's for htuos ))
    iswin7_add_glass(WizardForm.Handle, 0, 0, 0, 47, True);
  end;

  // Component list
  ComponentList := TStringList.Create();
  with ComponentList do
  begin
    ComponentList.Add('program');
    ComponentList.Add('sounds');
    ComponentList.Add('protocols');
    ComponentList.Add('protocols\aim');
    ComponentList.Add('protocols\facebook');
    ComponentList.Add('protocols\gg');
    ComponentList.Add('protocols\icq');
    ComponentList.Add('protocols\irc');
    ComponentList.Add('protocols\jabber');
    ComponentList.Add('protocols\yahoo');
    ComponentList.Add('clists');
    ComponentList.Add('clists\stdclist');
    ComponentList.Add('clists\blind');
    ComponentList.Add('ckists\nicer');
    ComponentList.Add('ckists\modern');
    ComponentList.Add('messagewin');
    ComponentList.Add('messagewin\stdmsg');
    ComponentList.Add('messagewin\scriver');
    ComponentList.Add('messagewin\tabsrmm');
    ComponentList.Add('dbx_mmap');
    ComponentList.Add('lang');
    ComponentList.Add('lang\russian');
    ComponentList.Add('lang\czech');
    ComponentList.Add('lang\polish');
    ComponentList.Add('lang\german');
    ComponentList.Add('lang\english');
  end;

  WizardForm.NoIconsCheck.Visible:=True;
  // Pages creation
  CreateInstallTypePage(); //страница выбора типа установки (обычная или портативная)

  // Adding component descriptions
  CreateComponentsInfoPanel();

  // 'At least 1 protocol selected' event handlers
  WizardForm.ComponentsList.OnClick := @ComponentOnClick;
  WizardForm.ComponentsList.OnKeyPress := @ComponentOnKeyPress;

  // Creating label for displaying percents of installation progress
  ProgressLabel := TLabel.Create(WizardForm);
  with WizardForm.ProgressGauge do
  begin
    ProgressLabel.Top := Top + Height + ScaleY(8);
    ProgressLabel.Left:= Left;
    ProgressLabel.Caption := '0' + ExpandConstant('{cm:PercentDone}');
    ProgressLabel.AutoSize := True;
    ProgressLabel.Parent := WizardForm.InstallingPage;
  end;
end;

// Skipping unused pages
function ShouldSkipPage(PageID: Integer): Boolean;
begin
  if ((PageID = wpSelectProgramGroup) or (PageID = wpSelectTasks)) and (PortTypeInstRadio.Checked) then
    Result := True;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  // Tasks creation preventing clutch
  if CurPageID = wpSelectTasks then
    WizardForm.TasksList.CheckItem(1, coCheck);
  // Langpack selection
  if CurPageID = wpSelectComponents then
  begin
    if ActiveLanguage = 'en' then
      WizardForm.ComponentsList.Checked[26]:= True
    else if ActiveLanguage = 'ru' then
      WizardForm.ComponentsList.Checked[22]:= True
    else if ActiveLanguage = 'cz' then
      WizardForm.ComponentsList.Checked[23]:= True
    else if ActiveLanguage = 'pl' then
      WizardForm.ComponentsList.Checked[24]:= True
    else if ActiveLanguage = 'de' then
      WizardForm.ComponentsList.Checked[25]:= True;
  end;
end;

// Next button click handler
function NextButtonClick(CurPage: Integer): Boolean;
begin
  Result:=true;
  if (CurPage=InstallTypePage.ID) then
    CheckFolder();

  if (CurPage = wpSelectDir) then
  begin
    EnableDescription(WizardForm.ComponentsList.Handle, ComponentInfo.Handle,
    ExpandConstant('{cm:CoreFilesDescription}') + ';' +
    ExpandConstant('{cm:SoundsDescription}') + ';' +
    ExpandConstant('{cm:ProtocolsDescription}') + ';' +
    ExpandConstant('{cm:ProtocolsAIMDescription}') + ';' +
    ExpandConstant('{cm:ProtocolsFbDescription}') + ';' +
    ExpandConstant('{cm:ProtocolsGGDescription}') + ';' +
    ExpandConstant('{cm:ProtocolsICQDescription}') + ';' +
    ExpandConstant('{cm:ProtocolsIRCDescription}') + ';' +
    ExpandConstant('{cm:ProtocolsJabberDescription}') + ';' +
    ExpandConstant('{cm:ProtocolsYahooDescription}') + ';' +
    ExpandConstant('{cm:ClistsDescription}') + ';' +
    ExpandConstant('{cm:StdClistDescription}') + ';' +
    ExpandConstant('{cm:BlindClistDescription}') + ';' +
    ExpandConstant('{cm:NicerClistDescription}') + ';' +
    ExpandConstant('{cm:ModernClistDescription}') + ';' +
    ExpandConstant('{cm:MessageWinDescription}') + ';' +
    ExpandConstant('{cm:StdMsgDescription}') + ';' +
    ExpandConstant('{cm:ScriverDescription}') + ';' +
    ExpandConstant('{cm:TabsrmmDescription}') + ';' +
    ExpandConstant('{cm:MMAPDescription}') + ';' +
    ExpandConstant('{cm:LangDescription}') + ';' +
    ExpandConstant('{cm:LangRussianDescription}') + ';' +
    ExpandConstant('{cm:LangCzechDescription}') + ';' +
    ExpandConstant('{cm:LangPolishDescription}') + ';' +
    ExpandConstant('{cm:LangGermanDescription}') + ';' +
    ExpandConstant('{cm:LangEnglishDescription}') + ';' +
    '');

    if DirExists(WizardForm.DirEdit.Text) and (FileExists(WizardForm.DirEdit.Text+'\{#MirName}') or FileExists(WizardForm.DirEdit.Text+'\mir_core.dll'))  then
    begin
      MsgBox(ExpandConstant('{cm:FolderExists1}')+#10+#10+WizardForm.DirEdit.Text+#10+#10+ExpandConstant('{cm:FolderExists2}'), mbError, MB_OK);
      Result := False;
      exit;
    end;
  end;

  if CurPage = wpSelectComponents then
    CheckGroup();

  if CurPage=wpFinished then
  begin
    if DirExists(AddBackslash(ExpandConstant('{app}\Icons\Fp_ClientIcons'))) then
    begin
      DelTree(AddBackslash(ExpandConstant('{app}\Icons\Fp_ClientIcons')), True, True, True);
    end;
  end;
end;

// Uninstall
procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep=usPostUninstall then
  begin
    if DirExists(AddBackslash(ExpandConstant('{userappdata}\Miranda NG'))) then
      if MsgBox(ExpandConstant('{cm:ProfileUninst}'), mbError, MB_YESNO or MB_DEFBUTTON2) = IDYES then
      begin
        DelTree(AddBackslash(ExpandConstant('{userappdata}\Miranda NG')), True, True, True);
      end;
  end;
end;

// Setup deinitialization
procedure DeinitializeSetup();
var
  Version: TWindowsVersion;
begin
  GetWindowsVersionEx(Version);
  if Version.NTPlatform and
     (Version.Major <= 6) and
     (Version.Minor < 2) then
  begin
    // Deinitializing library
    iswin7_free;
  end;
  DisableDescription();
end;

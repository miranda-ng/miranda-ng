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
Source: "Files\Icons\Proto_MSN.dll"; DestDir: "{app}\Icons"; Components: protocols\msn; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\Proto_Yahoo.dll"; DestDir: "{app}\Icons"; Components: protocols\yahoo; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\xstatus_icq.dll"; DestDir: "{app}\Icons"; Components: protocols\icq; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\xstatus_jabber.dll"; DestDir: "{app}\Icons"; Components: protocols\jabber; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\tabsrmm_icons.dll"; DestDir: "{app}\Icons"; Components: messagewin\tabsrmm; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Icons\toolbar_icons.dll"; DestDir: "{app}\Icons"; Components: clicts\modern; Flags: ignoreversion; AfterInstall: ShowPercent()

; Core and core modules
Source: "Files\{#MirName}"; DestDir: "{app}"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\mir_core.dll"; DestDir: "{app}"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\zlib.dll"; DestDir: "{app}"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\DbChecker.bat"; DestDir: "{app}"; Components: program; Check: IsPortable(); Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\msvcp100.dll"; DestDir: "{app}"; Components: program; Check: IsPortable(); Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\msvcr100.dll"; DestDir: "{app}"; Components: program; Check: IsPortable(); Flags: ignoreversion; AfterInstall: ShowPercent()
Source: "Files\Core\stdauth.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdautoaway.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdaway.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdchat.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdclist.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdcrypt.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdemail.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdfile.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdhelp.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdidle.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdmsg.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stduihist.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stdurl.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stduserinfo.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Core\stduseronline.dll"; DestDir: "{app}\Core"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 

; LangPacks
Source: "Files\langpack_czech.txt"; DestDir: "{app}"; Components: lang\czech; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\langpack_german.txt"; DestDir: "{app}"; Components: lang\german; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\langpack_polish.txt"; DestDir: "{app}"; Components: lang\polish; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\langpack_russian.txt"; DestDir: "{app}"; Components: lang\russian; Flags: ignoreversion; AfterInstall: ShowPercent() 

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
Source: "Files\Plugins\MSN.dll"; DestDir: "{app}\Plugins"; Components: protocols\msn; Flags: ignoreversion; AfterInstall: ShowPercent() 
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
Source: "Files\Plugins\Dbx_mmap.dll"; DestDir: "{app}\Plugins"; Components: dbx\mmap; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\Plugins\Dbx_3x.dll"; DestDir: "{app}\Plugins"; Components: dbx\3x; Flags: ignoreversion; AfterInstall: ShowPercent() 

; Config files
Source: "Files\Settings\mirandaboot_default.ini"; DestDir: "{app}"; Components: program; DestName: "mirandaboot.ini"; Check: IsDefault(); Flags: ignoreversion onlyifdoesntexist; AfterInstall: ShowPercent() 
Source: "Files\Settings\mirandaboot_portable.ini"; DestDir: "{app}"; Components: program; DestName: "mirandaboot.ini"; Check: IsPortable(); Flags: ignoreversion onlyifdoesntexist; AfterInstall: ShowPercent() 
Source: "Files\Settings\autoexec_sounds.ini"; DestDir: "{app}"; Components: sounds; Flags: ignoreversion; AfterInstall: ShowPercent() 
Source: "Files\autoexec_nightly_pu.ini"; DestDir: "{app}"; Components: program; Flags: ignoreversion; AfterInstall: ShowPercent() 

; Installer add-ons
Source: "Installer\ISWin7.dll"; Flags: dontcopy 
Source: "Installer\descctrl.dll"; Flags: dontcopy
Source: "Installer\{#VcRedistName}"; DestDir: {tmp}; Flags: deleteafterinstall; Check: RedistIsNotInstalled

[Components]
Name: "program"; Description: "{cm:Program}"; Types: extended regular minimal custom; Flags: fixed 
Name: "sounds"; Description: "{cm:Sounds}"; Types: extended custom 
Name: "protocols"; Description: "{cm:Protocols}"; Types: extended regular minimal custom; Flags: fixed 
Name: "protocols\aim"; Description: "{cm:Aim}"; Types: extended regular minimal 
Name: "protocols\facebook"; Description: "{cm:Facebook}"; Types: extended regular minimal 
Name: "protocols\gg"; Description: "{cm:Gg}"; Types: extended regular minimal 
Name: "protocols\icq"; Description: "{cm:Icq}"; Types: extended regular minimal 
Name: "protocols\irc"; Description: "{cm:Irc}"; Types: extended regular minimal 
Name: "protocols\jabber"; Description: "{cm:Jabber}"; Types: extended regular minimal 
Name: "protocols\msn"; Description: "{cm:Msn}"; Types: extended regular minimal 
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
Name: "dbx"; Description: "{cm:Dbx}"; Types: extended regular minimal custom ; Flags: fixed 
Name: "dbx\3x"; Description: "{cm:Dbx_3x}"; Types: custom; Flags: exclusive 
Name: "dbx\mmap"; Description: "{cm:Mmap}"; Types: extended regular minimal; Flags: exclusive 
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
en.FolderExists2=already exists. Continue installation?

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
en.Msn=MSN protocol
en.Yahoo=Yahoo protocol
en.Clists=Contact lists
en.StdClist=Classic contact list
en.BlindClist=Contact list for blind folks
en.NicerClist=Nicer contact list
en.ModernClist=Modern contact list
en.MessageWin=Message session
en.StdMsg=SRMM and Chat plugins
en.Scriver=Scriver message plugin
en.Tabsrmm=TabSRMM message plugin
en.Dbx=Database drivers
en.Mmap=Dbx_mmap database driver (recommended)
en.Dbx_3x=Dbx_3x database driver
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
en.ProtocolsMSNDescription=Microsoft Network (MSN) protocol support for Miranda NG.
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
en.DBXDescription=Various database drivers.
en.MMAPDescription=Provides Miranda database support: global settings, contacts, history, settings per contact.(Recommended)
en.DBX_3xDescription=Provides Miranda database support: global settings, contacts, history, settings per contact.
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

ru.FolderExists1=Папка
ru.FolderExists2=уже существует. Продолжить установку?

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
ru.Msn=Протокол MSN
ru.Yahoo=Протокол Yahoo
ru.Clists=Списки контактов
ru.StdClist=Список контактов Classic
ru.BlindClist=Список контактов для слабовидящих
ru.NicerClist=Список контактов Nicer
ru.ModernClist=Список контактов Modern
ru.MessageWin=Диалоговые окна
ru.StdMsg=Плагины SRMM и Chat
ru.Scriver=Диалоговое окно Scriver
ru.Tabsrmm=Диалоговое окно TabSRMM
ru.Dbx=Драйверы базы данных
ru.Mmap=Драйвер Dbx_mmap (рекомендуется)
ru.Dbx_3x=Драйвер Dbx_3x
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
ru.ProtocolsMSNDescription=Поддержка протокола Microsoft Network (MSN) в Miranda NG.
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
ru.DBXDescription=Выбор драйвера базы данных Miranda NG.
ru.MMAPDescription=Поддержка базы Miranda NG: глобальные настройки, контакты, история, настройки контактов и т. п. Отличается меньшим количеством обращений к базе данных в процессе работы.(Рекомендуется).
ru.DBX_3xDescription=Поддержка базы Miranda NG: глобальные настройки, контакты, история, настройки контактов и т. п.
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
cz.FolderExists2=již existuje. Pokračovat v instalaci?

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
cz.Msn=MSN protokol
cz.Yahoo=Yahoo protokol
cz.Clists=Seznamy kontaktů
cz.StdClist=Clist_classic
cz.BlindClist=Clist_blind (pro nevidomé)
cz.NicerClist=Clist_nicer
cz.ModernClist=Clist_modern
cz.MessageWin=Komunikační okna
cz.StdMsg=SRMM a Chat
cz.Scriver=Scriver
cz.Tabsrmm=TabSRMM
cz.Dbx=Ovladače databáze
cz.Mmap=Dbx_mmap (doporučeno)
cz.Dbx_3x=Dbx_3x
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
cz.protocolsMSNDescription=Microsoft Network (MSN) protokol pro Mirandu NG.
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
cz.DBXDescription=Různé ovladače databáze.
cz.MMAPDescription=Podpora pro databáze Mirandy: globální nastavení, kontakty, historie, nastavení pro jednotlivé kontakty. (doporučeno)
cz.DBX_3xDescription=Podpora pro databáze Mirandy: globální nastavení, kontakty, historie, nastavení pro jednotlivé kontakty.
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
de.FolderExists2=existiert bereits. Installation fortsetzen?

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
de.Msn=MSN-Protokoll
de.Yahoo=Yahoo-Protokoll
de.Clists=Kontaktlisten
de.StdClist=Klassische Kontaktliste
de.BlindClist=Kontaktliste für Blinde
de.NicerClist=Nicer-Kontaktliste
de.ModernClist=Moderne Kontaktliste
de.MessageWin=Nachrichtensitzungen
de.StdMsg=SRMM- und Chat-Plugins
de.Scriver=Scriver-Nachrichtenplugin
de.Tabsrmm=TabSRMM-Nachrichtenplugin
de.Dbx=Datenbanktreiber
de.Mmap=Dbx_mmap-Datenbanktreiber (empfohlen)
de.Dbx_3x=Dbx_3x-Datenbanktreiber
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
de.ProtocolsMSNDescription=Microsoft-Network- (MSN)-Protokollunterstützung für Miranda NG.
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
de.DBXDescription=Verschiedene Datenbanktreiber.
de.MMAPDescription=Stellt Miranda-Datenbankunterstützung bereit: globale Einstellungen, Kontakte, Verlauf, Per-Kontakt-Einstellungen. (Empfohlen.)
de.DBX_3xDescription=Stellt Miranda-Datenbankunterstützung bereit: globale Einstellungen, Kontakte, Verlauf, Per-Kontakt-Einstellungen.
de.LangDescription=Sprachdateien für Miranda NG.
de.LangRussianDescription=Übersetzung der Miranda-NG-Benutzeroberfläche ins Russische.
de.LangCzechDescription=Übersetzung der Miranda-NG-Benutzeroberfläche ins Tschechische.
de.LangPolishDescription=Übersetzung der Miranda-NG-Benutzeroberfläche ins Polnische.
de.LangGermanDescription=Übersetzung der Miranda-NG-Benutzeroberfläche ins Deutsche.
de.LangEnglishDescription=Original-Miranda-NG-Benutzeroberfläche auf Englisch.

[Languages]
Name: "en"; MessagesFile: "compiler:Default.isl"
Name: "ru"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "cz"; MessagesFile: "compiler:Languages\Czech.isl"
Name: "pl"; MessagesFile: "compiler:Languages\Polish.isl"
Name: "de"; MessagesFile: "compiler:Languages\German.isl"

[Code]
//глобальные переменные
var
  DefTypeInstLabel, PortTypeInstLabel, ProgressLabel: TLabel;
  DefTypeInstRadio, PortTypeInstRadio: TRadioButton;
  InstallTypePage: TWizardPage;
  ComponentInfo: TNewStaticText;
  ComponentList: TStringList;

//--Функции из внешних длл--

//Aero эффект
procedure iswin7_add_glass(Handle:HWND; Left, Top, Right, Bottom : Integer; GDIPLoadMode: boolean);
external 'iswin7_add_glass@files:iswin7.dll stdcall';
procedure iswin7_add_button(Handle:HWND);
external 'iswin7_add_button@files:iswin7.dll stdcall';
procedure iswin7_free;
external 'iswin7_free@files:iswin7.dll stdcall';

//Описание компонентов
function EnableDescription(ComponentsListHandle: HWND; DescLabelHandle: HWND; DescStrings: PAnsiChar): BOOL;
external 'enabledesc@files:descctrl.dll stdcall';
function DisableDescription: BOOL;
external 'disabledesc@files:descctrl.dll stdcall';

//Получение всех логических дисков
function GetLogicalDriveStrings(nLenDrives: LongInt; lpDrives: ansistring): integer;
external 'GetLogicalDriveStringsA@kernel32.dll stdcall';

//Получение типа диска
function GetDriveType(lpDisk: ansistring): integer;
external 'GetDriveTypeA@kernel32.dll stdcall';

//константы типов дисков
const
  DRIVE_UNKNOWN = 0;     // The drive type cannot be determined.
  DRIVE_NO_ROOT_DIR = 1; // The root path is invalid. For example, no volume is mounted at the path.
  DRIVE_REMOVABLE = 2;   // The disk can be removed from the drive.
  DRIVE_FIXED = 3;       // The disk cannot be removed from the drive.
  DRIVE_REMOTE = 4;      // The drive is a remote (network) drive.
  DRIVE_CDROM = 5;       // The drive is a CD-ROM drive.
  DRIVE_RAMDISK = 6;     // The drive is a RAM disk.

//проверка версии установки по дефолту
function IsDefault: Boolean;
begin
  if (DefTypeInstRadio.Checked) then
    Result:= True;
end;

//проверка версии установки портативной
function IsPortable: Boolean;
begin
  if (PortTypeInstRadio.Checked) then
    Result:= True;
end;

//vc redist installation check
function RedistIsNotInstalled: Boolean;
begin
   Result := not RegKeyExists{#RedistRegChk};
end;

//создание страницы установки с типами установки (обычная или портативная)
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

// Панель информации о компонентах
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

//выбор папки установки
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
    //получаем все диски системы
    drivesletters := StringOfChar(' ', 64);
    lenletters := GetLogicalDriveStrings(63, drivesletters);
    SetLength(drivesletters , lenletters);
    drive := '';
    n := 0;
    //перебираем все диски в цикле
    while ((Length(drivesletters) > 0)) do
    begin
      posnull := Pos(#0, drivesletters);
        if posnull > 0 then
        begin
        drive:= UpperCase(Copy(drivesletters, 1, posnull - 1));
        // получаем тип диска
        disktype := GetDriveType(drive);
        //если съемный, то проверяем совбодное место и предлагаем поставить на него
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

//выбор группы меню
procedure CheckGroup();
begin
  if DefTypeInstRadio.Checked then
    WizardForm.NoIconsCheck.Checked := False;

  if PortTypeInstRadio.Checked then
    WizardForm.NoIconsCheck.Checked := True;
end;

// Проверка выбора протоколов
procedure ComponentOnClick(Sender: TObject);
begin
  if (Pos(ExpandConstant('{cm:Protocols}'), ' ' + WizardSelectedComponents(True)) = 0) then
    begin
      WizardForm.NextButton.Enabled := False;
    end
  else
    WizardForm.NextButton.Enabled := True;
end;

// Проверка выбора протоколов
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

//рассчет и вывод процентов установки в лейбл
procedure ShowPercent();
begin
  with WizardForm.ProgressGauge do
    ProgressLabel.Caption:= IntToStr((Position-Min)/((Max - Min)/100)) + ExpandConstant('{cm:PercentDone}');
end;

//начальная инициализация
procedure InitializeWizard();
var
  Version: TWindowsVersion;
begin
  // Для более красивого отображения уменьшаем нижнюю границу
  WizardForm.Bevel.Height := 1;

  GetWindowsVersionEx(Version);
  if Version.NTPlatform and
     (Version.Major <= 6) and
     (Version.Minor < 2) then
  begin
    // Инициализируем библиотеку
    iswin7_add_button(WizardForm.BackButton.Handle);
    iswin7_add_button(WizardForm.NextButton.Handle);
    iswin7_add_button(WizardForm.CancelButton.Handle);
    // Параметр True не трогать он для htuos ))
    iswin7_add_glass(WizardForm.Handle, 0, 0, 0, 47, True);
  end;

  // Компоненты
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
    ComponentList.Add('protocols\msn');
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
    ComponentList.Add('dbx');
    ComponentList.Add('dbx\3x');
    ComponentList.Add('dbx\mmap');
    ComponentList.Add('lang');
    ComponentList.Add('lang\russian');
    ComponentList.Add('lang\czech');
    ComponentList.Add('lang\polish');
    ComponentList.Add('lang\german');
    ComponentList.Add('lang\english');
  end;

  WizardForm.NoIconsCheck.Visible:=True;
  // Создание страниц
  CreateInstallTypePage(); //страница выбора типа установки (обычная или портативная)

  //добавление описаний к компонентам установки
  CreateComponentsInfoPanel();

  //обработчики проверки выбора хотя бы 1 протокола
  WizardForm.ComponentsList.OnClick := @ComponentOnClick;
  WizardForm.ComponentsList.OnKeyPress := @ComponentOnKeyPress;

  //создание лейбла для вывода процентов установки
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

//пропуск неиспользуемых страниц
function ShouldSkipPage(PageID: Integer): Boolean;
begin
  if ((PageID = wpSelectProgramGroup) or (PageID = wpSelectTasks)) and (PortTypeInstRadio.Checked) then
    Result := True;
end;

procedure CurPageChanged(CurPageID: Integer);
begin
  //кастыль, чтобы обойти создание тасков
  if CurPageID = wpSelectTasks then
    WizardForm.TasksList.CheckItem(1, coCheck);
  //langpack selection
  if CurPageID = wpSelectComponents then
  begin
    if ActiveLanguage = 'en' then
      WizardForm.ComponentsList.Checked[28]:= True
    else if ActiveLanguage = 'ru' then
      WizardForm.ComponentsList.Checked[24]:= True
    else if ActiveLanguage = 'cz' then
      WizardForm.ComponentsList.Checked[25]:= True
    else if ActiveLanguage = 'pl' then
      WizardForm.ComponentsList.Checked[26]:= True
    else if ActiveLanguage = 'de' then
      WizardForm.ComponentsList.Checked[27]:= True;
  end;
end;

//обработчик нажатия кнопки Далее
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
    ExpandConstant('{cm:ProtocolsMSNDescription}') + ';' +
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
    ExpandConstant('{cm:DBXDescription}') + ';' +
    ExpandConstant('{cm:DBX_3xDescription}') + ';' +
    ExpandConstant('{cm:MMAPDescription}') + ';' +
    ExpandConstant('{cm:LangDescription}') + ';' +
    ExpandConstant('{cm:LangRussianDescription}') + ';' +
    ExpandConstant('{cm:LangCzechDescription}') + ';' +
    ExpandConstant('{cm:LangPolishDescription}') + ';' +
    ExpandConstant('{cm:LangGermanDescription}') + ';' +
    ExpandConstant('{cm:LangEnglishDescription}') + ';' +
    '');

    if DirExists(WizardForm.DirEdit.Text) then
    begin
      case MsgBox(ExpandConstant('{cm:FolderExists1}')+#10+#10+WizardForm.DirEdit.Text+#10+#10+ExpandConstant('{cm:FolderExists2}'), mbconfirmation, mb_YesNo) of
      IDYES:;
      IDNO:
        Result := False;
      end;
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

//Деинсталляция
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

//Деинициализация установки
procedure DeinitializeSetup();
var
  Version: TWindowsVersion;
begin
  GetWindowsVersionEx(Version);
  if Version.NTPlatform and
     (Version.Major <= 6) and
     (Version.Minor < 2) then
  begin
    // Отключаем библиотеку
    iswin7_free;
  end;
  DisableDescription();
end;

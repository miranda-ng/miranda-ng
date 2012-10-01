==================
Pack Updater 0.1.0
==================

Miranda IM plugin that do automatic or manual Miranda packs update.

Plugin maybe interesting for packs author, which wants do update of their pack automatically.

0. Changelog
1. How to use?
2. How it works?
3. Copyrights and other


-----------------
0. Changelog
-----------------

0.1.0
+ now not installed files are not updated
+ added an additional parameter "Force" in the description of the file. With it you an update not installed files
* Changed FileType parameter type

0.0.9
* Fixed order in update list

0.0.8
+ Added Icolib support

0.0.7
+ disabled or not existed plugins not updated and not downloaded
+ added FileType 5. It ia as type 4 but without restart

0.0.6
* All updates shows in one list
* Fixed bug with no updating files
* Changeg version comparing algorithm 

0.0.5
+ Added x64 version
+ Added Netlib support
+ Added ability to update plugins, icons and other miranda files
+ Added ability to clear folder with downloaded updates
+ Added hotkeys support (Customize-Hotkeys-Pack Updater) as default set Ctrl+F10
+ Added Popup plus actions support
! Fixed frizing when downloaded files
* If extension of downloaded file is ".html" it will be opened in browser
* Fixed activation Apply button even you not change anything
* Changed name of plugin. now it us PackUpdater
* Plugin is Unicode now
* Changed storing format on server - all information about all files stored in one file
* In Pack for all files - one url for chexking
* Fixed logic in popups
- ZeroUpdate.ini not needed anymore, all settings storing in profile

0.0.4
+ Added partly support of popups

0.0.3
+ Added support of Folders plugin
+ Added ability to translate description of update from langpack
* Changed new version detection

0.0.2
+ Added support of path for dowloaded updates
+ Added ability don't start update automatically

0.0.1
+ Pre-release

--------------------
1. How to use?
--------------------

Import settings from Autoexec_PackUpdater.ini to profile and for server (file1.ini) are in Tamplates folder. Probably full langpack in this folder too.

1. Add in your langpack_*.txt strings needed for plugin.

Variables in Autoexec_PackUpdater.ini:

	FileCount - number of files for witch needs to check updates. Plugin supports checking for several files, but most time is only one (Pack setup file)
	
	AutoUpdate - Check updates automatically with Miranda startup. configured in Events->Pack updates

	File_VersionURL - URL where saved file with infirmation about versions of all files (file1.ini)

[File_x_] (where x - serial number file, begining from 1)
		
	CurrentVersion - sets 1.0.0 (or another what you wants) and don't change any more
	
	LastVersion - the same


3. Create file with infirmation about versions (file1.ini). One for all checked files


Options of file1.ini:

Handle [FileInfo_õ], where õ - serial number file, begining from 1 (must be equal with number in profile)


FileVersion - Current file version (Version of new pack file)

Message - Description that user see when plugin found new version

DownloadURL - URL of Update file

DiskFileName - Name of file saved on user computer.

FileType - type of file. Defines final destination of file. (1 - pack, 2 - plugin (will be installed into Plugins), 3 - icon (will be installed into Icons), 4 - files in miranda root (e.g. langpack, dbtool))

Descr - description witch shows in update list.

InfoURL - Optional. If it writed, message box that user see have button "See information", click on it open this URL.

AdvFolder - Optional. If it writed, set subfolder in folder defined FileType.

Force - Optional. If it writed, the file will be offered for renewal, even if you do not installed.

4. Download file with infirmation and updates on server.


--------------------
2. How it works?
--------------------

Plugin has two modes: silent and normal. silent is used when checking for updates on startup, normal - click menu item.
First, plugin downloads file version information and compares the "local" (File_x_CurrentVersion from profile) version with the "remote" 
(FileVersion). If they differ, that:
a). When you work in silent mode plugin checks the value LastVersion and if it differs from the FileVersion, shows the message about the update.
if not - there are no messages.
b). When working in normal mode plugin displays a message in any case.
If a user downloaded the update, but decided it just did not install, update is still considered to be established.


---------------------
3. Copyrights and other
---------------------

Plugin written by ZERO_BiT, continued development by Mataes.
The author is not liable for any damages, the explicit or implied, caused by this plugin. Also wanted to ask not to delete the original copyright.

For questions contacts:
	e-mail: mataes2007@gmail.com
	icq: 4618898
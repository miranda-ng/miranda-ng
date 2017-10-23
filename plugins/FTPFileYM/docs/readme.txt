***********
FTP File YM
***********

Description
===========
Plugin uploads files to the FTP server.

Main features
=============
- FTPS (FTP+SSL) and SFTP (FTP via SSH) support
- support for zipping files before upload
- support for sending folders
- tabs support in upload dialog
- possibility to set 5 different FTP servers
- simple file manager for deleting uploaded files from FTP
- added Pause/Resume and Cancel buttons in upload dialog
- customizable menu entries

!!! Requirements !!!
====================
Libcurl.dll has to be in Miranda root folder.


Changelog
=========

--- 0.4.0.3 ---
* x64 portability changes
! fixed multiple same files in file manager

--- 0.4.0.2 ---
+ possibility to enter archive file name
+ check if file exists on server

--- 0.4.0.1 ---
! restored backward compatibility with Send Screenshot

--- 0.4.0.0 ---
+ added zipping support
+ added support for sending folders
* many other changes and improvements
* plugin completely rewritted to C++ 

--- 0.3.0.4 ---
* redesigned buttons in file manager
! fixed multiple same files in file manager
! another fixes for SFTP

--- 0.3.0.3 ---
+ added hidden option ('LocalIP', string format) for set IP address used in PORT command
! fixed file deleting via SFTP
! fixed upload dialog refreshing

--- 0.3.0.2 ---
! some FTP connection fixes

--- 0.3.0.1 ---
! fixed 100% CPU load in file manager
! some other minor fixes

--- 0.3.0.0 ---
+ implemented libcurl library
+ added FTPS (FTP+SSL) and SFTP (FTP via SSH) support

--- 0.2.2.1 ---
+ auto deletion function can be adjusted in options menu in upload dialog
! some fixes for file manager

--- 0.2.2.0 ---
+ automatic deletion of file after defined time
+ added send button to tabsrmm button bar
+ added server info to upload dialog
! fixed 'Command after upload' function

--- 0.2.1.0 ---
+ simple file manager for deleting uploaded files from FTP

--- 0.2.0.1 ---
+ ability to close tab by double click or middle button click
* changed upload dialog layout
! fixed selection of new tab after tab is closed 

--- 0.2.0.0 ---
* plugin was completely rewritten
+ added tabs support

--- 0.1.0.3 ---
! disabled menu entries for non IM contacts

--- 0.1.0.2 ---
! fixed possible crash in options

--- 0.1.0.1 ---
+ added options for better customize of contact menu items (original patch by jarvis)
+ added check of minimal Miranda version (0.7+)
! fixed percentual indicator for large files

--- 0.1.0.0 ---
+ first FL version

--- 0.0.2.5 ---
+ option for remove national characters from filename
+ updater support
* increased number of FTP servers to 5
* visual changes in upload dialog
! fixed 100% CPU load when upload process is paused
! fixed compatibility with clist_classic and clist_mw
! fixed compatibility with scriver and srmm

--- 0.0.2.4 ---
+ added Cancel All and Stop/Resume buttons
+ restored service function for other plugins (sendSS) - see m_ftpfile.h
! fixed some minor bugs

--- 0.0.2.3 ---
+ added Ansi version
! fixed crash on sending multiple files  

--- 0.0.2.2 ---
+ added possibility to set 3 different FTP server
+ full Unicode support
* menu entry redesigned

Symbols used in changelog: + = new / * = changed / ! = fixed


Author
======
original plugin by Joel Jonsson (thx!)
mod by yaho -> YM 

ICQ:	356344824
email:	yaho@miranda-easy.net
www:	www.miranda-easy.net
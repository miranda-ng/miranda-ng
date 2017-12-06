**********
StopSpam+
**********

Description
===========
This plugin is further development of StopSpam plugin v0.0.1.0 from Roman Miklashevsky and Andrey Petkevich.

Plugin works in pretty simple way:
While messages from users in your contact-list will be shown as if there were no antispam plugin, messages from unknown users will be hidden from you until user gives predefined answer to your question. You can set this question and expected answer in plugin options. After receiving correct answer all messages sent to you from the user will be shown (even those before correct answer was given) and if 'Add contact permanently' option is enabled user will be added to local contact-list. 'StopSpam-StopSpam infinite talk protection' option prevents endless crossmessaging between two StopSpam plugins. 
Plugin supports unicode for question and answer texts. 

New features
************
- Unicode aware.

Requirements
************
- Miranda v0.7.13.0 or higher.

Official plugin website 
***********************
http://koshechka.miranda.im/category/stopspam/.

Source Code
***********
http://svn.miranda.im/mainrepo/stopspam/.

Changelog
=========

----0.0.1.2---
+ Added detection of outgoing events. Now if we will send any event to user then that user will be automatically accepted as passed through StopSpam.
+ Users from 'Not In List' ICQ group are considered as not yet passed spam filtering.
* Minor fixes and changes.

----0.0.1.1---
* Renamed plugin to StopSpam+.
* Slightly reorganized options pages.
+ Unicode aware.

Symbols used in changelog
--------------
+ : new feature
* : changed
! : bugfix
- : feature removed or disabled because of pending bugs
--------------

Author
======
Created by Roman Miklashevsky.
Further development by Kosh&chka and persei.


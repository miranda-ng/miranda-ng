0. Introduction
===============

 * Purpose of this document

   This is a collection of important things you might want to know to get the
   most out of HistoryStats plugin for Miranda IM.

 * Outline

   1. Installation
   2. Plugin support
   3. Documentation
   4. Credits


1. Installation
===============

 * Download

   The most recent version of this plugin should always be available at [1].
   In case you want to switch back to an older version of HistoryStats (might
   be problematic and in general not advisable) this is the place to search at.
   Additionaly, you can find it on the official Miranda IM File Listing under
   [2] (non-Unicode) and [3] (Unicode).

   [1] http://miranda.dark-passage.de/
   [2] http://addons.miranda-im.org/details.php?action=viewfile&id=2534
   [3] http://addons.miranda-im.org/details.php?action=viewfile&id=2535

 * Requirements

   For the ANSI/Unicode build: Miranda IM 0.6.7 or above.

 * PNG format support

   Please use the "png2dib.dll" plugin from Miranda IM 0.6.7 or any newer
   version. Note that there might be problems with transparency for older
   versions of "png2dib.dll". Note that "advaimg.dll" replaces "png2dib.dll"
   in Miranda IM 0.7.0 #27 or above.
   
 * Stripping raw RTF control sequences from messages

   In order for this option to be available the library "rtfconv.dll" needs
   to be in the Miranda IM folder or in the "plugins" subfolder. You can
   download "rtfconv.dll" from [1] (just extract that one file and ignore the
   rest) or you should be able to find an alternative link to it in [2].

   [1] http://www5b.biglobe.ne.jp/~harigaya/e/rtfcnv.html
   [2] http://addons.miranda-im.org/details.php?action=viewfile&id=2445


2. Plugin support
=================

 * Supported plugins

   HistoryStats integrates with MetaContacts, Updater and IcoLib. Just go to
   the plugin's options page to find out more about this.


3. Documentation
================

 * Supported variables in output file/folder

   If enabled you can use several variables in your output file and your folder
   for additional files. Variables referencing date/time values are taken from
   the date/time you generate the statistics. These variables are supported:

   %profile_path% - where your profile is stored (including trailing backslash)
   %profile_name% - the name of your profile
   %miranda_path% - location of miranda32.exe (including trailing backslash)
   %nick%         - your nick (see corresponding option in the output settings)
   %h%            - hour in 12h format without leading zero (1-12)
   %hh%           - hour in 12h format with leading zero (01-12)
   %H%            - hour in 24h format without leading zero (0-23)
   %HH%           - hour in 24h format with leading zero (00-23)
   %tt%           - am/pm symbol in lower case (am/pm)
   %TT%           - am/pm symbol in upper case (AM/PM)
   %m%            - minute without leading zero (0-59)
   %mm%           - minute with leading zero (00-59)
   %s%            - second without leading zero (0-59)
   %ss%           - second with leading zero (00-59)
   %yy%           - two-digit year without century (e.g. 06)
   %yyyy%         - four-digit year including century (e.g. 2006)
   %M%            - month without leading zero (1-12)
   %MM%           - month with leading zero (01-12)
   %MMM%          - abbreviated month name (Jan, Feb, ...)
   %MMMM%         - full month name (January, February, ...)
   %d%            - day without leading zero (1-31)
   %dd%           - day with leading zero (01-31)
   %ww%           - abbreviated day of week (Mo, Tu, We, ...)
   %www%          - abbreviated day of week (Mon, Tue, Wed, ...)
   %wwww%         - full day of week (Monday, Tuesday, Wednesday, ...)
   %%             - a single percent sign (%)

   If you enter a relative path it will be autoamtically prefixed with
   %miranda_path% which is what previous versions of this plugin did.

 * Other resources

   There is no official documentation but maybe the FAQ [1] or the Miranda IM
   Forums thread [2] can help you. Feel free to contact me or to ask questions
   on the forums.

   [1] http://miranda.dark-passage.de/historystats/faq/
   [2] http://forums.miranda-im.org/showthread.php?t=1281


4. Credits
==========

 * Most icons were contributed by a0x (more info at http://www.a0x.info/)

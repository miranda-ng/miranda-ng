#!/usr/bin/perl
#
# Script generating translation templates from sources
#
# Miranda IM: the free IM client for Microsoft* Windows*
# 
# Copyright 2000-2008 Miranda ICQ/IM project,
# all portions of this codebase are copyrighted to the people
# listed in contributors.txt.
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

use POSIX;
use File::Find;
use strict;

my $rootdir = '';
my %hash = ();
my $clines = 0;

my $version = '0.7svn';
my $time = localtime();

if (@ARGV && $ARGV[0] eq "version") {
	shift @ARGV;
	# 1st arg version major i.e. 07 for "0.7"
	if (@ARGV) {
		my $vermaj = shift @ARGV;
		if ($vermaj =~ /^([0-9]*)([0-9])$/) {
			if ($1 eq "") {
				$version = $2;
			} else {
				$version = "$1.$2";
			}
		} else {
			$version = $vermaj;
		}
	}
	# 2nd arg version minor i.e. 1 for "0.7.1"
	if (@ARGV) {
		my $vermin = shift @ARGV;
		if ($vermin =~ /^[0-9]+$/) {
			$version .= ".$vermin";
		} else {
			$version .= $vermin;
		}
	}
	# 3rd arg build number i.e. 36 for "0.7.1 build 36"
	if (@ARGV) {
		my $build = shift @ARGV;
		if ($build =~ /^[0-9]+$/) {
			$version .= " build $build";
		} else {
			$version .= $build;
		}
	}
}

#Language Files
#if (!@ARGV) {
	#create_langfile(
		#'../..',
		#'../../i18n/langpack_english.txt',
		#'English (US)', '0809',
		#'Miranda IM Development Team',
		#'project-info@miranda-im.org',
		#'Scriver,avs,chat,clist,clist_nicer,db3x,db3x_mmap,dbrw,advaimg,import,modernb,mwclist,png2dib,srmm,tabsrmm,AimOscar,Gadu-Gadu,IRC,IcqOscarJ,JabberG,MSN,Yahoo');
#}
if (!@ARGV) {
	create_langfile('../../src',
		'../../src/!core-translation.txt',
		'English (US)', '0809',
		'Miranda IM Development Team',
		'project-info@miranda-im.org');
	create_langfile('../../plugins/AddContactPlus/',
		'../../plugins/AddContactPlus/addcontactplus-translation.txt');
	create_langfile('../../plugins/Alarms/',
		'../../plugins/Alarms/alarms-translation.txt');
	create_langfile('../../plugins/AssocMgr/',
		'../../plugins/AssocMgr/assocmgr-translation.txt');
	create_langfile('../../plugins/AuthState/',
		'../../plugins/AuthState/authstate-translation.txt');
	create_langfile('../../plugins/AutoRun/',
		'../../plugins/AutoRun/autorun-translation.txt');
	create_langfile('../../plugins/AVS/',
		'../../plugins/AVS/avs-translation.txt');
	create_langfile('../../plugins/BasicHistory/',
		'../../plugins/BasicHistory/basichistory-translation.txt');
	create_langfile('../../plugins/BASS_interface/',
		'../../plugins/BASS_interface/bass_interface-translation.txt');
	create_langfile('../../plugins/Boltun/',
		'../../plugins/Boltun/boltun-translation.txt');
	create_langfile('../../plugins/AddContactPlus/',
		'../../plugins/AddContactPlus/addcontactplus-translation.txt');
	create_langfile('../../plugins/BossKeyPlus/',
		'../../plugins/BossKeyPlus/bosskeyplus-translation.txt');
	create_langfile('../../plugins/BuddyExpectator/',
		'../../plugins/BuddyExpectator/buddyexpectator-translation.txt');
	create_langfile('../../plugins/ChangeKeyboardLayout/',
		'../../plugins/ChangeKeyboardLayout/changekeyboardlayout-translation.txt');
	create_langfile('../../plugins/Chat/',
		'../../plugins/Chat/chat-translation.txt');
	create_langfile('../../plugins/ClientChangeNotify/',
		'../../plugins/ClientChangeNotify/clientchangenotify-translation.txt');
	create_langfile('../../plugins/Clist_classic/',
		'../../plugins/Clist_classic/clist_classic-translation.txt');
	create_langfile('../../plugins/Clist_modern/',
		'../../plugins/Clist_modern/clist_modern-translation.txt');
	create_langfile('../../plugins/Clist_mw/',
		'../../plugins/Clist_mw/clist_mw-translation.txt');
	create_langfile('../../plugins/Clist_nicer/',
		'../../plugins/Clist_nicer/clist_nicer-translation.txt');
	create_langfile('../../plugins/Console/',
		'../../plugins/Console/console-translation.txt');
	create_langfile('../../plugins/ContactsPlus/',
		'../../plugins/ContactsPlus/contactsplus-translation.txt');
	create_langfile('../../plugins/CountryFlags/',
		'../../plugins/CountryFlags/countryflags-translation.txt');
	create_langfile('../../plugins/CSList/',
		'../../plugins/CSList/cslist-translation.txt');
	create_langfile('../../plugins/CyrTranslit/',
		'../../plugins/CyrTranslit/cyrtranslit-translation.txt');
	create_langfile('../../plugins/Db3x/',
		'../../plugins/Db3x/db3x-translation.txt');
	create_langfile('../../plugins/Db3x_mmap/',
		'../../plugins/Db3x_mmap/db3x_mmap-translation.txt');
	create_langfile('../../plugins/Dbx_mmap_SA/',
		'../../plugins/Dbx_mmap_SA/dbx_mmap_sa-translation.txt');
	create_langfile('../../plugins/Dbx_tree/',
		'../../plugins/Dbx_tree/dbx_tree-translation.txt');
	create_langfile('../../plugins/Db_autobackups/',
		'../../plugins/Db_autobackups/db_autobackups-translation.txt');
	create_langfile('../../plugins/ExtraIcons/',
		'../../plugins/ExtraIcons/extraicons-translation.txt');
	create_langfile('../../plugins/FavContacts/',
		'../../plugins/FavContacts/favcontacts-translation.txt');
	create_langfile('../../plugins/FingerPrintModPlus/',
		'../../plugins/FingerPrintModPlus/fingerprintmodplus-translation.txt');
	create_langfile('../../plugins/FirstRun/',
		'../../plugins/FirstRun/firstrun-translation.txt');
	create_langfile('../../plugins/FlashAvatars/',
		'../../plugins/FlashAvatars/flashavatars-translation.txt');
	create_langfile('../../plugins/FloatingContacts/',
		'../../plugins/FloatingContacts/floatingcontacts-translation.txt');
	create_langfile('../../plugins/Folders/',
		'../../plugins/Folders/folders-translation.txt');
	create_langfile('../../plugins/FreeImage/',
		'../../plugins/FreeImage/freeimage-translation.txt');
	create_langfile('../../plugins/Gender/',
		'../../plugins/Gender/gender-translation.txt');
	create_langfile('../../plugins/HistoryLinkListPlus/',
		'../../plugins/HistoryLinkListPlus/historylinklistplus-translation.txt');
	create_langfile('../../plugins/HistorySweeperLight/',
		'../../plugins/HistorySweeperLight/historysweeperlight-translation.txt');
	create_langfile('../../plugins/IEView/',
		'../../plugins/IEView/ieview-translation.txt');
	create_langfile('../../plugins/IgnoreState/',
		'../../plugins/IgnoreState/ignorestate-translation.txt');
	create_langfile('../../plugins/Import/',
		'../../plugins/Import/import-translation.txt');
	create_langfile('../../plugins/ImportTXT/',
		'../../plugins/ImportTXT/importtxt-translation.txt');
	create_langfile('../../plugins/KeyboardNotify/',
		'../../plugins/KeyboardNotify/keyboardnotify-translation.txt');
	create_langfile('../../plugins/LangMan/',
		'../../plugins/LangMan/langman-translation.txt');
	create_langfile('../../plugins/ListeningTo/',
		'../../plugins/ListeningTo/listeningto-translation.txt');
	create_langfile('../../plugins/MenuItemEx/',
		'../../plugins/MenuItemEx/menuitemex-translation.txt');
	create_langfile('../../plugins/MetaContacts/',
		'../../plugins/MetaContacts/metacontacts-translation.txt');
	create_langfile('../../plugins/MirOTR/',
		'../../plugins/MirOTR/mirotr-translation.txt');
	create_langfile('../../plugins/ModernOpt/',
		'../../plugins/ModernOpt/modernopt-translation.txt');
	create_langfile('../../plugins/mTextControl/',
		'../../plugins/mTextControl/mtextcontrol-translation.txt');
	create_langfile('../../plugins/NewAwaySysMod/',
		'../../plugins/NewAwaySysMod/newawaysysmod-translation.txt');
	create_langfile('../../plugins/NewEventNotify/',
		'../../plugins/NewEventNotify/neweventnotify-translation.txt');
	create_langfile('../../plugins/NewXstatusNotify/',
		'../../plugins/NewXstatusNotify/newxstatusnotify-translation.txt');
	create_langfile('../../plugins/Nudge/',
		'../../plugins/Nudge/nudge-translation.txt');
	create_langfile('../../plugins/PackUpdater/',
		'../../plugins/PackUpdater/packupdater-translation.txt');
	create_langfile('../../plugins/Popup/',
		'../../plugins/Popup/popup-translation.txt');
	create_langfile('../../plugins/ProfileManager/',
		'../../plugins/ProfileManager/profilemanager-translation.txt');
	create_langfile('../../plugins/QuickMessages/',
		'../../plugins/QuickMessages/quickmessages-translation.txt');
	create_langfile('../../plugins/QuickReplies/',
		'../../plugins/QuickReplies/quickreplies-translation.txt');
	create_langfile('../../plugins/Rate/',
		'../../plugins/Rate/rate-translation.txt');
	create_langfile('../../plugins/RecentContacts/',
		'../../plugins/RecentContacts/recentcontacts-translation.txt');
	create_langfile('../../plugins/Scriver/',
		'../../plugins/Scriver/scriver-translation.txt');
	create_langfile('../../plugins/SecureIM/',
		'../../plugins/SecureIM/secureim-translation.txt');
	create_langfile('../../plugins/SeenPlugin/',
		'../../plugins/SeenPlugin/seenplugin-translation.txt');
	create_langfile('../../plugins/Sessions/',
		'../../plugins/Sessions/sessions-translation.txt');
	create_langfile('../../plugins/SimpleAR/',
		'../../plugins/SimpleAR/simplear-translation.txt');
	create_langfile('../../plugins/SimpleStatusMsg/',
		'../../plugins/SimpleStatusMsg/simplestatusmsg-translation.txt');
	create_langfile('../../plugins/SmileyAdd/',
		'../../plugins/SmileyAdd/smileyadd-translation.txt');
	create_langfile('../../plugins/SpellChecker/',
		'../../plugins/SpellChecker/spellchecker-translation.txt');
	create_langfile('../../plugins/SplashScreen/',
		'../../plugins/SplashScreen/splashscreen-translation.txt');
	create_langfile('../../plugins/SRMM/',
		'../../plugins/SRMM/srmm-translation.txt');
	create_langfile('../../plugins/StatusPlugins/',
		'../../plugins/StatusPlugins/statusplugins-translation.txt');
	create_langfile('../../plugins/StopSpamPlus/',
		'../../plugins/StopSpamPlus/stopspamplus-translation.txt');
	create_langfile('../../plugins/Svc_crshdmp/',
		'../../plugins/Svc_crshdmp/svc_crshdmp-translation.txt');
	create_langfile('../../plugins/Svc_dbepp/',
		'../../plugins/Svc_dbepp/svc_dbepp-translation.txt');
	create_langfile('../../plugins/Svc_vi/',
		'../../plugins/Svc_vi/svc_vi-translation.txt');
	create_langfile('../../plugins/TabSRMM/',
		'../../plugins/TabSRMM/tabsrmm-translation.txt');
	create_langfile('../../plugins/TipperYM/',
		'../../plugins/TipperYM/tipperym-translation.txt');
	create_langfile('../../plugins/TooltipNotify/',
		'../../plugins/TooltipNotify/tooltipnotify-translation.txt');
	create_langfile('../../plugins/TopToolBar/',
		'../../plugins/TopToolBar/toptoolbar-translation.txt');
	create_langfile('../../plugins/TranslitSwitcher/',
		'../../plugins/TranslitSwitcher/translitswitcher-translation.txt');
	create_langfile('../../plugins/Updater/',
		'../../plugins/Updater/updater-translation.txt');
	create_langfile('../../plugins/UserGuide/',
		'../../plugins/UserGuide/userguide-translation.txt');
	create_langfile('../../plugins/UserInfoEx/',
		'../../plugins/UserInfoEx/userinfoex-translation.txt');
	create_langfile('../../plugins/Variables/',
		'../../plugins/Variables/variables-translation.txt');
	create_langfile('../../plugins/W7UI/',
		'../../plugins/W7UI/w7ui-translation.txt');
	create_langfile('../../plugins/WhenWasIt/',
		'../../plugins/WhenWasIt/whenwasit-translation.txt');
	create_langfile('../../plugins/YAPP/',
		'../../plugins/YAPP/yapp-translation.txt');
	create_langfile('../../plugins/ZeroNotification/',
		'../../plugins/ZeroNotification/zeronotification-translation.txt');
	create_langfile('../../protocols/AimOscar/',
		'../../protocols/AimOscar/aimoscar-translation.txt');
	create_langfile('../../protocols/FacebookRM/',
		'../../protocols/FacebookRM/facebookrm-translation.txt');
	create_langfile('../../protocols/Gadu-Gadu/',
		'../../protocols/Gadu-Gadu/gadu-gadu-translation.txt');
	create_langfile('../../protocols/GTalkExt/',
		'../../protocols/GTalkExt/gtalkext-translation.txt');
	create_langfile('../../protocols/IcqOscarJ/',
		'../../protocols/IcqOscarJ/icqoscarj-translation.txt');
	create_langfile('../../protocols/IRCG/',
		'../../protocols/IRCG/ircg-translation.txt');
	create_langfile('../../protocols/JabberG/',
		'../../protocols/JabberG/jabberg-translation.txt');
	create_langfile('../../protocols/MRA/',
		'../../protocols/MRA/mra-translation.txt');
	create_langfile('../../protocols/MSN/',
		'../../protocols/MSN/msn-translation.txt');
	create_langfile('../../protocols/NewsAggregator/',
		'../../protocols/NewsAggregator/newsaggregator-translation.txt');
	create_langfile('../../protocols/Omegle/',
		'../../protocols/Omegle/omegle-translation.txt');
	create_langfile('../../protocols/Quotes/',
		'../../protocols/Quotes/quotes-translation.txt');
	create_langfile('../../protocols/Twitter/',
		'../../protocols/Twitter/twitter-translation.txt');
	create_langfile('../../protocols/Weather/',
		'../../protocols/Weather/weather-translation.txt');
	create_langfile('../../protocols/Yahoo/',
		'../../protocols/Yahoo/yahoo-translation.txt');
	create_langfile('../../protocols/YAMN/',
		'../../protocols/YAMN/yamn-translation.txt');
	create_langfile('../dbtool/',
		'../dbtool/dbtool-translation.txt');
	}
elsif ($ARGV[0] eq "core") {
	create_langfile('../../src',
		'../../i18n/core-translation.txt',
		'English (US)', '0809',
		'Miranda IM Development Team',
		'project-info@miranda-im.org');
}
elsif ($ARGV[0] eq "srmm") {
	create_langfile('../../plugins/SRMM/',
		'../../plugins/SRMM/Docs/srmm-translation.txt',
		'English (US)', '0809',
		'Miranda IM Development Team',
		'project-info@miranda-im.org');
}
elsif ($ARGV[0] eq "import") {
	create_langfile('../../plugins/Import/',
		'../../plugins/Import/docs/import-translation.txt',
		'English (US)', '0809',
		'Miranda IM Development Team',
		'project-info@miranda-im.org');
}
else {
	print "Error: Unknown module $ARGV[0]\n";
	exit 1;
}

sub create_langfile {
	$rootdir = shift(@_);
	my $outfile = shift(@_);
	my $lang = shift(@_);
	my $locale = shift(@_);
	my $author = shift(@_);
	my $email = shift(@_);
	my $plugins = @_ ? shift(@_) : '';
	%hash = ();
	my %common = ();
	$clines = 0;
	print "Building language file for $rootdir:\n";
	find({ wanted => \&csearch, preprocess => \&pre_dir }, $rootdir);
	find({ wanted => \&rcsearch, preprocess => \&pre_dir }, $rootdir);
	open(WRITE, "> $outfile") or die;
	if ($outfile eq "../../src/!core-translation.txt") {
	print WRITE <<HEADER; }
Miranda Language Pack Version 1
Locale: $locale
Authors: $author
Author-email: $email
Last-Modified-Using: Miranda IM $version
Plugins-included: $plugins
; Generated by lpgen on $time
; Translations: $clines

HEADER
	foreach my $trans (keys %hash) {
		if ($hash{$trans} =~ /^\d+$/) {
			$common{$trans} = $hash{$trans};
			delete $hash{$trans};
		}
	}

	my @sorted = sort { (floor($common{$b}/20) == floor($common{$a}/20)) ? ($a cmp $b) : ($common{$b} <=> $common{$a}) } keys %common;
	print WRITE "; Common strings that belong to many files\n;[";
	print WRITE join "]\n;[", @sorted;
	print WRITE "]\n";

	my $file = '';
	foreach my $trans (sort { ($hash{$a} eq $hash{$b}) ? ($a cmp $b) : ($hash{$a} cmp $hash{$b}) } keys %hash) {
		if ($hash{$trans} ne $file) {
			$file = $hash{$trans};
			print WRITE "\n; $file\n";
		}
		print WRITE ";[$trans]\n";
	}

	close(WRITE);
	print "  $outfile is complete ($clines)\n\n";
}

sub pre_dir {
	# avoid parsing some ext SDKs
	my @files = ();
	return @files if (
		$File::Find::name =~/..\/extplugins/ or
		$File::Find::name =~/freeimage\/Source/ or
		$File::Find::name =~/dbrw\/sqlite3/);
	@files = grep { not /^\.\.?$/ } @_;
	return sort @files;
}

sub append_str {
	my $str = shift(@_);
	my $found = shift(@_);
	$str = substr($str, 1, length($str) - 2);
	if (length($str) gt 0 and $str ne "List1" and $str ne "Tree1" and $str =~ /[a-zA-Z]+/g) {
		my $path = $File::Find::name;
		$path =~ s/(\.\.\/)+miranda\///;
		if (!$hash{$str}) {
			$hash{$str} = $path;
			$clines ++;
			return 1;
		} elsif ($hash{$str} ne $path) {
			if ($hash{$str} =~ /^\d+$/) {
				$hash{$str} ++;
			} else {
				$hash{$str} = 1;
			}
		}
	}
	return 0;
}

sub csearch {
	if (-f $_ and ($_ =~ m/\.c(pp)?$/i or $_ =~ m/\.h(pp)?$/i)) {
		my $found = 0;
		my $file = $_;
		print "  Processing $_ ";
		open(READ, "< $_") or return;
		my $all = '';
		while (my $lines = <READ>) {
			$all = $all.$lines;
		}
		close(READ);
		$_ = $all;
		while (/(?:Button_SetIcon_IcoLib|Translate[A-Z]{0,2}|LPGENT?|ICQTranslateUtfStatic)\s*\(\s*(\"[^\\]*?(\\.[^\\]*?)*\")\s*[,\)]/g) {
			$found += append_str($1, $found);
		}
		print "($found)\n";
	}
}

sub rcsearch {
	if ( -f $_ and $_ =~ m/\.rc$/i) {
		my $found = 0;
		my $file = $_;
		print "  Processing $_ ";
		open(READ, "< $_") or return;
		my $all = '';
		while (my $lines = <READ>) {
			$all = $all.$lines;
		}
		close(READ);
		$_ = $all;
		s/\"\"/\\\"/g;
		while (/\s*(?:CONTROL|(?:DEF)?PUSHBUTTON|[LRC]TEXT|GROUPBOX|CAPTION|MENUITEM|POPUP)\s*(\"[^\\]*?(\\.[^\\]*?)*\")/g) {
			$found += append_str($1, $found);
		}
		print "($found)\n";
	}
}

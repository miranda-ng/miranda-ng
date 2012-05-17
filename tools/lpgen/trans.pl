#!/usr/bin/perl
#
# Script generating new translation from template, using old translation
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

if(@ARGV < 2)
{
	print "trans <to translate> <old translation> [<next old translation> ...]\n";
	exit;
}

my $key;
my %trans;
my $totrans = shift @ARGV;
while(@ARGV)
{
	my $oldtrans = shift @ARGV;
	open OT, "<$oldtrans" or die "Error: Cannot open translation";
	while(<OT>)
	{
		chop;
		if(m/^\[(.*)\]/)
		{ $key = $1; next; }
		elsif($key and !$trans{$key} and !m/^;/)
		{ $trans{$key} = $_; $key = 0; }
	}
}

open TT, "<$totrans" or die "Error: Cannot open file to translate";
while(<TT>)
{
	if(m/^;\[(.*)\]/ and $trans{$1})
	{
		print "[$1]\n";
		print $trans{$1};
		print "\n";
	}
	else
	{ print; }
}

close TT;
close OT;

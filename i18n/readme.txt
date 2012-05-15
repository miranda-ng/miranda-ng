The official, most up-to-date version is always available from
http://cvs.sourceforge.net/viewcvs.py/~checkout~/miranda-icq/Miranda-IM/docs/translations/langpack_english.txt?rev=HEAD&content-type=text/plain
It is *strongly recommended* that you check there for the latest version
before you begin to translate.

If you maintain a translation and want to find out what new strings have been
added and when, that and more can be done from
http://cvs.sourceforge.net/viewcvs.py/miranda-icq/Miranda-IM/docs/translations/langpack_english.txt

Blank lines and lines starting with are ignored.
You cannot use "[English]   ;this is a comment"
The first line must be maintained as-is, or the file will not be recognised
Subsequent lines are in HTTP header format. Do not translate anything before
the :

'Language' header
A full description of the language in this file, in the local language, eg
"English (UK)", "English (US) humorous" or "Deutsch"

'Locale'
The Windows language code. A complete list is available at
http://msdn.microsoft.com/library/default.asp?URL=/library/psdk/winbase/nls_8xo3.htm

'Last-Modified-Using'
Please keep this line up-to-date, containing the last version of Miranda IM that
you made changes with, so people can compare files containing the same
language to see which is the most up-to-date. The formatting of the value is
unimportant, as long as it's human-readable.

'Authors'
A list of people who have worked on this file. When you do something, add (or
move) your name to the front, and put a comma between each name, eg
"Hari Seldon, Gaal Dornick"

'Author-email'
Should contain the e-mail of the person to last change the file only, on the
assumption that that person is qualified to manage the file.

'Plugins-included'
Contains a list of the plugins that are also translated in this file. It
should be a comma-delimited list of the plugin filenames, eg:
Plugins-included: splitmsgdialogs,import,historyp
It is *very* strongly recommended that you put a clearly identifiable
separator between each set of strings, so that people can mix-and-match
different people's translations to get strings for all the plugins they use.

To use the file, place it in the same directory as miranda32.exe, and call it
langpack_*.txt where * can be anything, however it is recommended that it
be the language.
Miranda IM will eventually have an interface for switching languages, but
currently if there is more than one then which one is used is undefined.
Only left-to-right languages are supported currently. This will change if I
ever get the time.
If a string does not translate, it's likely that I forgot to mark it
translatable. Please file a bug.
If a translated string will not fit in its allocated space, try to re-phrase
it to fit. If you really can't get it to fit, file a bug.
Provided a plugin was written to be translation-aware, you can put strings
from that in here too.
I haven't translated splitmsgdialogs or import yet, because by the time I'd
finished this file, I was *so* mindlessly bored with it.
There will shortly be a section on the website where you can submit your
language packs. Please do so, so that other people can benefit from your
goodness.

You must restart Miranda IM for changes in this file to take effect.

All lines from now on contain text in Miranda IM, and its translation.
The text in Miranda IM *is* case sensitive.
The \t and \n glyphs are recognised to mean tab and newline respectively.
Text containing printf-style format specifiers (eg %s, %d) must be translated
into text containing the same specifiers in the same order. No more, no less.
Spaces are trimmed from the beginning and end of all strings before parsing.
Strings that cannot be found in this file will remain untranslated.
Some strings will have accelerator keys prefixed by an ampersand in the
normal Windows way. The accelerator key can be re-assigned, but be careful of
overlaps.
If an English string is duplicated, which of the translated strings that will
be used is undefined.
New lines in translated strings are treated as new lines, but two or more
newlines will be converted to one, because it involves a blank line.
You can't change hotkeys just by changing their strings in here.

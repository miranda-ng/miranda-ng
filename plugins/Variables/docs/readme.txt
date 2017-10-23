Variables plugin for Miranda IM v0.2.2.0
unregistered@users.sourceforge.net

--- About ---

This plugin doesn't add any functionality on itself. But other plugins
can use it to format a string which contains variables. Since this
plugin can  be quite complicated to use, please read this document
carefully before reporting any problems.

Many thanks to HeikoH and DeathDemon for their suggestions and help.

--- Usage ---

Variables requires Windows 2000 or higher, and the unicode version
requires the unicode version of Miranda.

The Variables plugin allows you to use special formatting of strings
within plugins that process their strings using this plugin. Examples
of such plugins are NewAwaySys, Tipper, SimpleAway and
StartupStatus. Please refer to the documentation of the plugin whether
or not Variables is supported. Usually, there is no special action
needed to use a formatting string instead of a normal string.

A formatting string consists of 'tokens', special keywords which are
translated by the Variables plugin into some text. Popular use of
Variables is to show the currently playing song in your away
message. If your away message module (like NewAwaySys) supports the
Variables plugin, you can enter a text like the following to show your
current Winamp song in your away message: "Hi, I'm listening to
%winampsong%.". In this example, the token '%winampsong%' will be
translated by the Variables plugin into the current song. Please see
the notes for more details.

Such special keywords, or tokens, are there in two flavors. First,
there are fields, the syntax for a field is %fieldname%, an example is
the already shown %winampsong% field. A field takes no arguments. A
but more advanced is the second flavor, functions. Functions are
usually accessed by "?functionname(argument1,argument2,...)", a
function can take zero or more arguments. An example of a function is
"?add(1,1)", this function will, as you can guess, be replaced by the
number "2".

A complete list of tokens can be found by pressing "Help..." on the
Plugins->Variables options screen.

Next to the % and ? character, there are a few others which have a
special meaning, these are:

!function

This will parse the function given the arguments, but the result will
not be parsed again. Example: "Message waiting:
!message(,first,rcvd,unread)". In case you use
"?message(,first,rcvd,unread)" and the message would be "You should
use the string %winampsong% to get the song.", the result could be
"Message waiting: You should use the string DJ Mike Llama - Llama
Whippin' Intro to get the song.".

`string`
This will not parse the given string, any function, field or special
character in the string will shown in the result without being
translated. Example: "Use `%mirandapath%` to show the installation
path." It's usually a good idea to put any non-special keyword between
` characters.

#comment
This will add a comment in the formatting string. Everything from the
# character to the end of the line will be removed. Example:
"%dbprofile% #this is a useless comment."

The Variables plugin allows you to build much more complex formatting
strings than the one shown above. Please refer to the "Additional
Help" and "Notes" sections at the end of this document for more
information.

There are a few settings in the options dialog:

[] Automatically strip "end of line" characters

This will remove any CRLF or LF characters. You can insert them using
the crlf function.

[] Automatically strip white characters

This will remove any white space. You can add white spaces by placing
` characters around them (see above).

[] Automatically strip all non-parsing characters

This will remove any characters that don't have a special meaning
(all characters except those shown above). Basically this means you
have to put all text besides tokens and comments between ` characters
(see above).

[] Parse the following string at startup

The input box below this option is not just for testing, you can
choose to parse the string at startup to set your global variables
using the put and puts functions (which aren't different in this
case).  Also, you can created aliases using the alias function.

[] Auto parse

Normally the string is only parsed when you press the "Parse" button,
check this to keep the parsed string updated every second.

[Help...]

Pressing this button shows up a list of available functions and
fields.

--- Additional Help ---

As stated before, this plugin can be quite complicated to use, here
follows some addition help for some of the complex functions.

---
if(x,y,z)
---
The Variables has several functions based on logics. The if function
will show string y in case x evaluates to true, and z otherwise. The
return values of the several logic functions can be found in the help
dialog. Also, a string x is false in case an error occurs while
parsing it.

Example:
!if(%winampsong%,`winamp is running`,`winamp is not running`)

---
cinfo(x,y)
---
x specifies the contact (see notes).

y specifies the info you want to retrieve, the following values are
possible:

first, last, nick, cnick, email, city, state, country, phone,
homepage, about, gender, age, firstlast, id, display, protocol,
status, intip, extip, protoid

From Miranda IM v0.4.3.0 Test Build #55, the following are also
available:

fax, cellular, timezone, mynotes, bday, bmonth, byear, street, zip,
lang1, lang2, lang3, coname, codept, copos, costreet, cocity, costate,
cozip, cocountry, cohomepage

Example:
!cinfo(<ICQ:12345678>,nick)

---
lsdate(x,y)
lstime(x,y)
lsstatus(x)
---
These functions require LastSeen or ContactsEx to be installed.

---
lsdate(x,y)
cdate(x)
---
The format argument can be formatted using the following:

d	Day of month as digits with no leading zero for single-digit
        days.

dd 	Day of month as digits with leading zero for single-digit
        days.

ddd 	Day of week as a three-letter abbreviation.

dddd 	Day of week as its full name.

M 	Month as digits with no leading zero for single-digit months.

MM 	Month as digits with leading zero for single-digit months.

MMM 	Month as a three-letter abbreviation.

MMMM 	Month as its full name.

y 	Year as last two digits, but with no leading zero for years
        less than 10.

yy 	Year as last two digits, but with leading zero for years less
        than 10.

yyyy 	Year represented by full four digits.

gg 	Period/era string. This element is ignored if the date to be
        formatted does not have an associated era or period string.

---
lstime(x,y)
ctime(x)
---
The format parameter can be formatted using the following:

h 	Hours with no leading zero for single-digit hours; 12-hour
        clock.

hh 	Hours with leading zero for single-digit hours; 12-hour
        clock.

H 	Hours with no leading zero for single-digit hours; 24-hour
        clock.

HH 	Hours with leading zero for single-digit hours; 24-hour
        clock.

m 	Minutes with no leading zero for single-digit minutes.

mm 	Minutes with leading zero for single-digit minutes.

s 	Seconds with no leading zero for single-digit seconds.

ss 	Seconds with leading zero for single-digit seconds.

t 	One character time-marker string, such as A or P.

tt 	Multicharacter time-marker string, such as AM or PM.

---
subject
---
This field returns a string in the format <PROTOCOL:UNQIUEID>, the
contact associated with this token depends on the plugin which uses it.

---
extratext
---
This field returns a string depending on the plugin which uses it.

---
get(x)
put(x,y)
puts(x,y)
---
Put and puts will store string y under name x. Puts will return
nothing, while put returns the string stored. Get will retrieve the
value stored earlier with put or puts.

NOTE: The stored string is first parsed, this means calling for
example !puts(time,!ctime()) and later !get(time) will always return
the time at which puts was called. Use !alias(x,y) to store 'unparsed
strings'.

NOTE2: Strings stored using put and puts can always be accessed (they
are shared amongst instances). So beware you don't modify a variable
in two places (unless you want that to happen).

---
alias(x,y)
---
Stores an alias named x with the value y. The alias x can be used as a
regular token. You can also override existing tokens using the alias
function.

Example:
!alias(song(pl,st,pa,nr),!switch(?if(!and(%winampstate%,%winampsong%),%winampstate%,`Not Running`),`Playing`,pl,`Paused`,pa,`Stopped`,st,`Not Running`,nr))
This line above will add a token "song" which takes 4 arguments. The
first one is the string when Winamp is playing a song, the second when
stopped, the third when paused and the fourth argument specifies the
string when Winamp is not running. You can use it like this:

?song(%winampsong%` is playing`,`just stopped listening to `%winampsong%,%winampsong%` <- paused`,`Winamp is not running`)

---
message(x,y,z,w)
---
Retrieves a message for contact x according to y,z and w.

y = either "first" to retrieve the first message or "last" (default)
to get the last one.

z = either "sent" to retrieve a sent message or "rcvd" to get a
received one.

w = either "read" to get a message you already read, or "unread" to
get one from the message queue. Only used when z = "rcvd".

If an argument is not given, this function will retrieve the message
according to the rest of the arguments. Some examples:

!message(,`first`,`rcvd`,`unread`)
  This is 'normal' operation, to get the first message in the message queue.

!message(%subject%,`last`,`rcvd`,`unread`)
  Get the last message you received from the contact %subject%, which
  is in the message queue.

!message(%subject%,`last`,,)
  Get the last message from contact %subject%. This one could be sent,
  received, read or unread.

!message(,,,)
  Get the last message Miranda processed.

---
for(w,x,y,z)
---

Generates a for-loop; w will be initialized, x specifies the condition
on which y will be executed and z will be shown.

Example:
!for(!puts(i,0),!not(!strcmp(!get(i),3)),!puts(i,!add(!get(i),1)),i=!get(i)!crlf())

--- Notes ---

1)
Whenever a functions requires a contact as an argument, you can
specify it in two ways:

-  Using a unique id (UIN for ICQ, email for MSN) or, a protocol id
   followed by a unique id in the form <PROTOID:UNIQUEID>, for example
   <MSN:miranda@hotmail.com> or <ICQ:123456789>.

-  Using the contact function:
---
contact(x,y)
---
A contact will be searched which will have value x for its property y,
y can be one of the following:

first, last, nick, email, id, display

Example:
?contact(miranda@hotmail.com,email) or
?contact(Miranda,nick).

Contact will return either a unique contact according to the arguments
or nothing if none or multiple contacts exists with the given
property.  If a property holds for multiple contacts, you can select
one of those  contacts by adding an extra argument z. This indicates
which contact to use. The ?ccount function can be used to find out how
many contacts  hold for a certain property. The following formatting
string  demonstrates the use of ccount and contact with three
arguments.

Example:
# get the number of contacts with the display name "Marley"
!puts(count,!ccount(`Marley`,display))
# loop through these contacts, for each of them, show the e-mail address
!for(!puts(n,0),!not(!strcmp(!get(n),!get(count))),!puts(n,!add(!get(n),1)),!cinfo(!contact(`Marley`,`display`,!add(!get(n),1)),email))

In case the z argument is used, y can be any property of the cinfo
function. Z can be 'r' to get a random contact.

2)
The %winampsong% field will only retrieve a song for Winamp, for
Foobar2000, I recommend using foo_text
(http://members.lycos.co.uk/musicf/) and the txtfile function.

3)
The variables regexp_check and regexp_substr are only available if you
have pcre.dll or pcre3.dll in your miranda directory (or system32).
PCRE is available from the "external" folder in this package and/or
from http://www.pcre.org/

4)
Variables v0.2.0.0 syntax is inspired by the TAGZ system used in
Foobar2000, http://www.foobar2000.org/.


--- Changelog ---

0.2.3.4: - added ?cinfo(<>,account) -> gets the account name under 0.8.x and later

0.2.3.3: - memory leak fix

0.2.3.2: - adaptation for Miranda 0.8

0.2.3.0: - temporary variables were added by Ricardo Pescuma Domenecci
         - fixes for memory leaks & crashes

0.2.2.0: - Added service MS_VARS_GETSKINITEM (V icon by Faith Healer)
         - Added option to remove white spaces and other non-parsing characters
         - Change in parsing routine: fields (%field%) are not parsed by default, use ?noop(%field%) instead
         - Change in parsing routine: fields won't be parsed with function chars anymore and vise versa
         - Added function strmcmp, amipvar, amipformat, noop, protoname, ls
         - %subject% supports contacts without uniqueid (please test)
         - txtfile now supports unicode text files
         - Performance gain (only if you use HUGE strings or rapid parsing) by using (Bob Jenkins') hash function
         - Lots of bug fixes

0.2.1.0: - Lot of bug fixes, unicode version, extended API, much more

0.2.0.0: - Build from scratch

0.1.0.0: - First release

--- Disclaimer ---

If something terrible happens, don't blame me.
